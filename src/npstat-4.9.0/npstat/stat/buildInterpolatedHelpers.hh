#ifndef NPSTAT_BUILDINTERPOLATEDHELPERS_HH_
#define NPSTAT_BUILDINTERPOLATEDHELPERS_HH_

//======================================================================
// buildInterpolatedHelpers.hh
//
// This is an internal header which is subject to change without notice.
// Application code should never directly use classes and functions 
// declared or defined in this header.
//
// Author: I. Volobouev
//
// July 2015
//======================================================================

#include <vector>
#include <utility>
#include <climits>
#include <iostream>
#include <ctime>
#include <cstdio>
#include <cassert>

#include "geners/CPP11_auto_ptr.hh"

#include "npstat/nm/AbsVisitor.hh"
#include "npstat/nm/BoxND.hh"
#include "npstat/nm/KDTree.hh"
#include "npstat/nm/SimpleFunctors.hh"
#include "npstat/nm/areAllElementsUnique.hh"
#include "npstat/nm/PointDimensionality.hh"
#include "npstat/nm/BoxNDScanner.hh"
#include "npstat/nm/findRootInLogSpace.hh"
#include "npstat/nm/GridAxis.hh"

#include "npstat/stat/Distributions1D.hh"
#include "npstat/stat/SampleAccumulator.hh"
#include "npstat/stat/AbsCompositeDistroBuilder.hh"
#include "npstat/stat/AbsDistro1DBuilder.hh"

namespace npstat {
    namespace Private {
        // Number of bandwidth values needed so that the weight of
        // the distribution outside of this number of bandwidth values
        // can be neglected
        double symbetaNumSigmas(int symbetaPower, unsigned dim);

        double symbetaBandwidthInsideUnitBox(
            int symbetaPower, double nEffToNRatio,
            const double* coords, unsigned dim);

        double symbetaEffRatioInsideUnitBox(
            int symbetaPower, double bandwidth,
            const double* coords, unsigned dim);

        template <class Point>
        class CdfWeightAssigner : public AbsVisitor<Point,double>
        {
        public:
            typedef typename Point::value_type Real;
            typedef std::pair<const Point*, double> WeightedPointPtr;
            typedef std::vector<WeightedPointPtr> WeightedPtrVec;

            inline CdfWeightAssigner(
                const std::vector<SampleAccumulator<Real> >& predSamples,
                const unsigned* dimPredictors,
                const double* coords, const unsigned nCoords,
                const int symbetaPow)
                : predSamples_(predSamples),
                  dimPredictors_(dimPredictors),
                  coords_(coords),
                  symbetaPow_(symbetaPow),
                  gauss_(0.0, 1.0),
                  symbeta_(0.0, 1.0, symbetaPow > 0 ? symbetaPow : 0),
                  h_(0.0),
                  wsum_(0.0L),
                  wsumsq_(0.0L),
                  nVizited_(0),
                  nZeroWeights_(0)
            {
                assert(nCoords == predSamples_.size());
            }

            inline void setBandwidth(const double h)
            {
                assert(h > 0.0);
                h_ = h;
            }

            inline void clear()
            {
                 ptrVec_.clear();
                 h_ = 0.0;
                 wsum_ = 0.0L;
                 wsumsq_ = 0.0L;
                 nVizited_ = 0;
                 nZeroWeights_ = 0;
            }

            inline void process(const Point& point)
            {
                assert(h_ > 0.0);
                const npstat::AbsDistribution1D& g = symbetaPow_ < 0 ?
                    dynamic_cast<const npstat::AbsDistribution1D&>(gauss_) :
                    dynamic_cast<const npstat::AbsDistribution1D&>(symbeta_);
                const unsigned ndim = predSamples_.size();
                double wprod = 1.0;
                for (unsigned ipred=0; ipred<ndim; ++ipred)
                {
                    const unsigned idim = dimPredictors_[ipred];
                    const double cdf = predSamples_[ipred].cdf(point[idim]);
                    const double w = g.density((cdf - coords_[ipred])/h_)/h_;
                    wprod *= w;
                }
                if (wprod > 0.0)
                {
                    ptrVec_.push_back(WeightedPointPtr(&point, wprod));
                    wsum_ += wprod;
                    wsumsq_ += wprod*wprod;
                }
                else
                    ++nZeroWeights_;
                ++nVizited_;
            }

            inline double result()
            {
                if (wsumsq_ > 0.0L)
                    return wsum_*wsum_/wsumsq_;
                else
                    return 0.0;
            }

            inline const WeightedPtrVec& getPoints() const {return ptrVec_;}
            inline unsigned getVisitCount() const {return nVizited_;}
            inline unsigned getNZeroWeights() const {return nZeroWeights_;}

        private:
            const std::vector<SampleAccumulator<Real> >& predSamples_;
            const unsigned* dimPredictors_;
            const double* coords_;
            WeightedPtrVec ptrVec_;
            int symbetaPow_;
            Gauss1D gauss_;
            SymmetricBeta1D symbeta_;
            double h_;
            long double wsum_;
            long double wsumsq_;
            unsigned nVizited_;
            unsigned nZeroWeights_;
        };

        template <typename Real>
        inline void fillCoordBoxFromBandwidth(
            const std::vector<SampleAccumulator<Real> >& predSamples,
            const double* cdfValues, const unsigned nCdfValues,
            const double h, BoxND<double>* box)
        {
            assert(h > 0.0);

            box->clear();
            box->reserve(nCdfValues);
            for (unsigned i=0; i<nCdfValues; ++i)
            {
                double cdfmin = cdfValues[i] - h;
                if (cdfmin < 0.0)
                    cdfmin = 0.0;
                double cdfmax = cdfValues[i] + h;
                if (cdfmax > 1.0)
                    cdfmax = 1.0;
                double xmin = predSamples[i].quantile(cdfmin);
                double xmax = predSamples[i].quantile(cdfmax);
                const double range = xmax - xmin;
                if (cdfValues[i] - h < 0.0)
                    xmin -= 0.01*range;
                if (cdfValues[i] + h > 1.0)
                    xmax += 0.01*range;
                box->push_back(Interval<double>(xmin, xmax));
            }
        }

        template <class Point>
        class PointCountByCdfBandwidth : public Functor1<double,double>
        {
        public:
            typedef typename Point::value_type Real;

            inline PointCountByCdfBandwidth(
                const KDTree<Point>& kdtree,
                const std::vector<SampleAccumulator<Real> >& predSamples,
                const double* cdfValues,
                const unsigned nCdfValues)
                : kdtree_(kdtree),
                  predSamples_(predSamples),
                  cdfValues_(cdfValues),
                  nCdfValues_(nCdfValues)
            {
                assert(cdfValues_);
                assert(nCdfValues_);
                assert(predSamples_.size() == nCdfValues_);
            }

            inline double operator()(const double& h) const
            {
                fillCoordBoxFromBandwidth(predSamples_, cdfValues_,
                                          nCdfValues_, h, &box_);
                return kdtree_.nInBox(box_);
            }

        private:
            const KDTree<Point>& kdtree_;
            const std::vector<SampleAccumulator<Real> >& predSamples_;
            const double* cdfValues_;
            mutable BoxND<double> box_;
            unsigned nCdfValues_;
        };

        template <
            class Point,
            class ResultDistro,
            template <class> class BuilderBase = AbsCompositeDistroBuilder
        >
        struct DistroBuilderAdapter
        {
            typedef typename BuilderBase<Point>::result_type CompDistro;
            typedef typename BuilderBase<Point>::WeightedPtrVec WeightedPtrVec;

            inline CPP11_auto_ptr<ResultDistro> construct(
                const std::vector<GridAxis>& predAxes,
                const unsigned nResponseVars, const bool mode) const
            {
                return CPP11_auto_ptr<ResultDistro>(
                    new ResultDistro(predAxes, nResponseVars, mode));
            }

            inline CompDistro* callBuilder(
                const BuilderBase<Point>& builder, const unsigned long uniqueId,
                const double* predictorCoords, const unsigned nPredictors,
                const BoxND<double>& predictorBox, const WeightedPtrVec& data,
                const unsigned* dimsToUse, const unsigned nDimsToUse) const
            {
                return builder.buildWeighted(
                    uniqueId, predictorCoords, nPredictors,
                    predictorBox, data, dimsToUse, nDimsToUse);
            }
        };

        template <class Point, class ResultDistro>
        struct DistroBuilderAdapter<Point, ResultDistro, AbsDistro1DBuilder>
        {
            typedef typename AbsDistro1DBuilder<Point>::result_type CompDistro;
            typedef typename AbsDistro1DBuilder<Point>::WeightedPtrVec WeightedPtrVec;

            inline CPP11_auto_ptr<ResultDistro> construct(
                const std::vector<GridAxis>& predAxes,
                const unsigned /* nResponseVars */, const bool mode) const
            {
                return CPP11_auto_ptr<ResultDistro>(new ResultDistro(predAxes, mode));
            }

            inline CompDistro* callBuilder(
                const AbsDistro1DBuilder<Point>& builder, const unsigned long uniqueId,
                const double* predictorCoords, const unsigned nPredictors,
                const BoxND<double>& predictorBox, const WeightedPtrVec& data,
                const unsigned* dimsToUse, const unsigned /* nDimsToUse */) const
            {
                return builder.buildWeighted(
                    uniqueId, predictorCoords, nPredictors,
                    predictorBox, data, dimsToUse[0]);
            }
        };

        template <
            class Point,
            class ResultDistro,
            template <class> class BuilderBase
        >
        CPP11_auto_ptr<ResultDistro> buildInterpolatedHelper(
            const std::vector<Point>& data,
            const unsigned* dimPredictors, const unsigned nPredictors,
            const std::string* predictorNames,
            const unsigned* predictorNumBins, const int predictorSymbetaPower,
            const double effectiveEventsPerBin, const bool stretchPredKernels,
            const unsigned* dimResponses, const unsigned nResponseVars,
            const BuilderBase<Point>& builder,
            const bool interpolateSwitch, const unsigned reportFrequency)
        {
            typedef typename Point::value_type Real;
            typedef typename BuilderBase<Point>::result_type CompDistro;
            const unsigned dim_size = PointDimensionality<Point>::dim_size;

            // Some hardwired parameters
            const unsigned maxdim = 32;
            const double bandwidthTol = 1.0e-7;
            const double initScaleStep = 0.02;

            // Check that the input arguments are reasonable
            const unsigned long dataLen = data.size();
            if (!dataLen) throw std::invalid_argument(
                "In npstat::Private::buildInterpolatedHelper: no data points");
            if (!nPredictors) throw std::invalid_argument(
                "In npstat::Private::buildInterpolatedHelper: no predictors");
            if (nPredictors >= maxdim) throw std::invalid_argument(
                "In npstat::Private::buildInterpolatedHelper: "
                "too many predictors");
            if (nResponseVars >= maxdim) throw std::invalid_argument(
                "In npstat::Private::buildInterpolatedHelper: "
                "too many response variables");
            if (effectiveEventsPerBin >= static_cast<double>(dataLen))
                throw std::invalid_argument(
                    "In npstat::Private::buildInterpolatedHelper: "
                    "too many effective events requested per bin");

            assert(dimPredictors);
            assert(predictorNumBins);
            assert(dimResponses);

            for (unsigned i=0; i<nPredictors; ++i)
                if (!predictorNumBins[i]) throw std::invalid_argument(
                    "In npstat::Private::buildInterpolatedHelper: "
                    "zero predictor bins in one of the dimensions");

            // Check correctness of the dimension numbers
            {
                unsigned dims[2*maxdim];
                for (unsigned i=0; i<nPredictors; ++i)
                    dims[i] = dimPredictors[i];
                for (unsigned i=0; i<nResponseVars; ++i)
                    dims[i+nPredictors] = dimResponses[i];
                unsigned dmax = 0;
                const unsigned ndims = nPredictors + nResponseVars;
                for (unsigned i=0; i<ndims; ++i)
                    if (dims[i] > dmax)
                        dmax = dims[i];
                if (dmax >= dim_size) throw std::invalid_argument(
                    "In npstat::Private::buildInterpolatedHelper: "
                    "dimension number out of range");
                if (!areAllElementsUnique(&dims[0], &dims[0]+ndims))
                    throw std::invalid_argument(
                        "In npstat::Private::buildInterpolatedHelper: "
                        "duplicate dimensions detected");
            }

            // We will need empirical quantile values for all predictors
            std::vector<SampleAccumulator<Real> > predSamples(nPredictors);
            for (unsigned ipred=0; ipred<nPredictors; ++ipred)
            {
                SampleAccumulator<Real>& acc(predSamples[ipred]);
                acc.reserve(dataLen);
                const unsigned idim = dimPredictors[ipred];
                for (unsigned long ipt=0; ipt<dataLen; ++ipt)
                    acc.accumulate(data[ipt][idim]);
            }

            // Grid axes for the GridInterpolatedDistribution
            std::vector<GridAxis> predAxes;
            predAxes.reserve(nPredictors);
            {
                std::vector<double> gridCoords;
                for (unsigned ipred=0; ipred<nPredictors; ++ipred)
                {
                    const unsigned nbins = predictorNumBins[ipred];
                    gridCoords.clear();
                    gridCoords.reserve(nbins);
                    for (unsigned ibin=0; ibin<nbins; ++ibin)
                    {
                        const double q = (ibin + 0.5)/nbins;
                        const double x = predSamples[ipred].quantile(q);
                        gridCoords.push_back(x);
                    }
                    if (predictorNames)
                        predAxes.push_back(GridAxis(
                            gridCoords, predictorNames[ipred].c_str()));
                    else
                        predAxes.push_back(GridAxis(gridCoords));
                }
            }

            // We can now initialize the resulting distribution
            DistroBuilderAdapter<Point, ResultDistro, BuilderBase> adap;
            CPP11_auto_ptr<ResultDistro> distro = adap.construct(
                predAxes, nResponseVars, interpolateSwitch);

            // To get the requested number of effective points we will need
            // the tree of data points
            const KDTree<Point> kdtree(data, dimPredictors, nPredictors);

            // See what kind of cdf bandwidth one would need to use in order
            // to obtain the requested effective number of points in the center
            // of the predictor distribution (in the assumption of uniform
            // copula)
            double predictorCdfs[maxdim];
            for (unsigned idim=0; idim<nPredictors; ++idim)
                predictorCdfs[idim] = 0.5;
            const double centralBw = symbetaBandwidthInsideUnitBox(
                predictorSymbetaPower, effectiveEventsPerBin/dataLen,
                predictorCdfs, nPredictors);

            // Objects that will be needed inside the cycle over
            // the predictor grid
            PointCountByCdfBandwidth<Point> pointCounter(
                kdtree, predSamples, predictorCdfs, nPredictors);
            BoxND<double> pBox;
            CdfWeightAssigner<Point> wAssign(predSamples, dimPredictors,
                                             predictorCdfs, nPredictors,
                                             predictorSymbetaPower);
            unsigned gridCell[maxdim];
            double predCoords[maxdim];

            // Scan over the points in the predictor space
            for (BoxNDScanner<double> scan(BoxND<double>::unitBox(nPredictors),
                                           predictorNumBins, nPredictors);
                 scan.isValid(); ++scan)
            {
                scan.getCoords(predictorCdfs, nPredictors);

                // Find the bandwidth to use for weighting the points
                double bwGuess = centralBw, realBw = 0.0;
                if (stretchPredKernels)
                {
                    // Figure out the bandwidth needed for the requested
                    // symbeta in the assumption of uniform copula
                    bwGuess = symbetaBandwidthInsideUnitBox(
                        predictorSymbetaPower, effectiveEventsPerBin/dataLen,
                        predictorCdfs, nPredictors);

                    // Figure out the bandwidth needed for the flat kernel
                    // in the assumption of uniform copula
                    const double bw0 = symbetaBandwidthInsideUnitBox(
                        0, effectiveEventsPerBin/dataLen,
                        predictorCdfs, nPredictors);

                    // Figure out the actual bandwidth needed for the flat
                    // kernel to get the requested effective number of events
                    double realBw0 = 0.0;
                    const bool status = findRootInLogSpace(
                        pointCounter, effectiveEventsPerBin, bw0, bandwidthTol,
                        &realBw0, initScaleStep);
                    assert(status);

                    // Scale the bandwidth to get the "real" bandwidth
                    realBw = bwGuess*realBw0/bw0;
                }
                else
                {
                    // Figure out the effective number of events we would
                    // get with this bandwidth for this cdf point in the
                    // assumption of uniform copula
                    const double ratioExp = symbetaEffRatioInsideUnitBox(
                        predictorSymbetaPower, bwGuess,
                        predictorCdfs, nPredictors);

                    // Figure out the bandwidth needed for the flat kernel
                    // to get that number of events in the assumption of
                    // uniform copula
                    const double bw0 = symbetaBandwidthInsideUnitBox(
                        0, ratioExp, predictorCdfs, nPredictors);

                    // Figure out the actual bandwidth needed for the flat
                    // kernel to get this effective number of events
                    double realBw0 = 0.0;
                    const bool status = findRootInLogSpace(
                        pointCounter, ratioExp*dataLen, bw0, bandwidthTol,
                        &realBw0, initScaleStep);
                    assert(status);

                    // Scale the bandwidth to get the "real" bandwidth
                    realBw = bwGuess*realBw0/bw0;
                }

                // Now, weight the points according to the bandwidth found
                wAssign.clear();
                wAssign.setBandwidth(realBw);
                const double nBw = symbetaNumSigmas(predictorSymbetaPower,
                                                    nPredictors);
                fillCoordBoxFromBandwidth(predSamples, predictorCdfs,
                                          nPredictors, nBw*realBw, &pBox);
                kdtree.visitInBox(wAssign, pBox);
                const double nEff = wAssign.result();
                assert(nEff > 0.0);

                // Get coordinates of the cell center in the predictor space
                scan.getIndex(gridCell, nPredictors);
                for (unsigned ipred=0; ipred<nPredictors; ++ipred)
                    predCoords[ipred] = predAxes[ipred].coordinate(gridCell[ipred]);

                // Build the distribution for this grid point
                const unsigned long scanCycle = scan.state();
                CompDistro* cdnd = adap.callBuilder(
                    builder, scanCycle, predCoords, nPredictors, pBox,
                    wAssign.getPoints(), dimResponses, nResponseVars);
                distro->setGridDistro(gridCell, nPredictors, cdnd);

                // Print some info about the processed point
                if (reportFrequency)
                    if (scanCycle % reportFrequency == 0)
                    {
                        // Generate a time stamp
                        struct tm *current;
                        time_t now;
                        time(&now);
                        current = localtime(&now);
                        char timestamp[10];
                        sprintf(timestamp, "%02i:%02i:%02i", current->tm_hour,
                                current->tm_min, current->tm_sec);

                        // Other things we want to print
                        const unsigned long nCycles = scan.maxState();

                        // Print a message
                        std::cout << timestamp << "  "
                                  << "In npstat::Private::buildInterpolatedHelper:"
                                  << "\n  Processed cell " << scanCycle+1
                                  << '/' << nCycles
                                  << ", bwGuess = " << bwGuess
                                  << ", bw = " << realBw
                                  << ", Neff = " << nEff << ".\n  Visited "
                                  << wAssign.getVisitCount() << " points, "
                                  << wAssign.getNZeroWeights() << " unused."
                                  << std::endl;
                    }
            }

            return distro;
        }
    }
}

#endif // NPSTAT_BUILDINTERPOLATEDHELPERS_HH_
