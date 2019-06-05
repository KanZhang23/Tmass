#ifndef NPSI_MINUITDENSITYFITFCN1D_HH_
#define NPSI_MINUITDENSITYFITFCN1D_HH_

/*!
// \file MinuitDensityFitFcn1D.hh
//
// \brief Binned fit of a parametric density by maximum likelihood
//
// Author: I. Volobouev
//
// September 2010
*/

#include <cfloat>

#include "npstat/stat/HistoND.hh"
#include "npstat/stat/FitUtils.hh"

#include "Minuit2/FCNBase.h"

namespace npsi {
    /**
    // Target minimization function adapter class for running maximum
    // likelihood density fits to histogrammed data by Minuit2.
    //
    // DensityConstructor is a functor which creates the necessary
    // density function on the stack out of a vector of parameters.
    // Must have "operator()(const std::vector<double>&) const"
    // which returns an object (or a reference) of some class which
    // was derived from AbsDistribution1D.
    */
    template
    <
        class Numeric,
        class DensityConstructor,
        class Axis = npstat::HistoAxis
    >
    class MinuitDensityFitFcn1D : public ROOT::Minuit2::FCNBase
    {
    public:
        /**
        // This class will not assume ownership of any pointers or references.
        //
        // Elements of "binMask" should be set to 1 for bins used in the fit
        // and to 0 for bins that are to be ignored.
        //
        // "nQuad" is the number of quadrature points to use for calculating
        // the density integral inside each bin (should be supported by
        // GaussLegendreQuadrature class). If this parameter is set to 0,
        // cumulative density function will be used.
        */
        MinuitDensityFitFcn1D(const npstat::HistoND<Numeric,Axis>& histo,
                              const unsigned char* binMask,
                              const unsigned maskLength,
                              const DensityConstructor& densityMaker,
                              const double minlog=log(DBL_MIN),
                              const double up=0.05,
                              const unsigned nQuad=6U)
            : histo_(histo),
              densityMaker_(densityMaker),
              binMask_(binMask),
              workspace_(histo_.nBins()),
              minlog_(minlog),
              up_(up),
              densityArea_(0.0),
              enabledBinCount_(0.0),
              nquad_(nQuad)
        {
            if (histo_.dim() != 1U) throw std::invalid_argument(
                "In npsi::MinuitDensityFitFcn1D constructor : "
                "only 1-d histograms can be used with this class");

            assert(binMask);

            if (histo_.nBins() != maskLength) throw std::invalid_argument(
                "In npsi::MinuitDensityFitFcn1D constructor : "
                "bin mask is not compatible with histogram binning");
        }

        inline virtual ~MinuitDensityFitFcn1D() {}

        /** This method returns negative log likelihood */
        inline virtual double operator()(const std::vector<double>& x) const
        {
            return -npstat::densityFitLogLikelihood1D(
                histo_, binMask_, histo_.nBins(), densityMaker_(x),
                minlog_, &workspace_[0], workspace_.size(),
                nquad_, &densityArea_, &enabledBinCount_);
        }

        inline double Up() const {return up_;}

        /** Area of the density inside enabled bins */
        inline double enabledArea() const {return densityArea_;}

        /** Event count for enabled bins */
        inline double enabledBinCount() const {return enabledBinCount_;}

    private:
        const npstat::HistoND<Numeric,Axis>& histo_;
        const DensityConstructor& densityMaker_;
        const unsigned char* binMask_;
        mutable std::vector<double> workspace_;
        double minlog_;
        double up_;
        mutable double densityArea_;
        mutable double enabledBinCount_;
        unsigned nquad_;
    };
}

#endif // NPSI_MINUITDENSITYFITFCN1D_HH_
