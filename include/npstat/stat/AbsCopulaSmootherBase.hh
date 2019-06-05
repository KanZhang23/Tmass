#ifndef NPSTAT_ABSCOPULASMOOTHERBASE_HH_
#define NPSTAT_ABSCOPULASMOOTHERBASE_HH_

/*!
// \file AbsCopulaSmootherBase.hh
//
// \brief Interface definition for classes which build discrete copulas
//
// Author: I. Volobouev
//
// September 2010
*/

#include <vector>
#include <utility>

#include "geners/AbsArchive.hh"

#include "npstat/stat/HistoND.hh"
#include "npstat/stat/OrderedPointND.hh"

namespace npstat{
    /**
    // Interface definition for classes which build discrete copulas out of data
    // samples (typically, by LOrPE, KDE, or some other nonparametric method).
    // The derived classes are supposed to override the "smoothHistogram"
    // member function.
    */
    class AbsCopulaSmootherBase
    {
    public:
        /**
        // Parameters "tolerance" and "maxNormaCycles" define how many
        // copula normalization cycles to perform after smoothing.
        // These cycles make sure that the resulting distribution has
        // uniform marginal densities. The cycles are needed because,
        // in general, marginals are not preserved by smoothing procedures.
        */
        AbsCopulaSmootherBase(const unsigned* nBinsInEachDim, unsigned dim,
                              double tolerance, unsigned maxNormCycles);

        inline virtual ~AbsCopulaSmootherBase() {delete h_;}

        /** Return the shape of the copula array */
        inline ArrayShape copulaShape() const {return makeShape(shape_, dim_);}

        /** Set the archive for storing the histograms */
        void setArchive(gs::AbsArchive* ar, const char* category = 0);

        /** Smoothing function for unweighted samples */
        template <class Point>
        const HistoND<double>& smooth(
            unsigned long uniqueId,
            std::vector<OrderedPointND<Point> >& in,
            double* bandwidthUsed = 0);

        /** Smoothing function for weighted samples */
        template <class Point>
        const HistoND<double>& weightedSmooth(
            unsigned long uniqueId,
            const std::vector<std::pair<const Point*, double> >& in,
            const unsigned* dimsToUse, unsigned nDimsToUse,
            double* bandwidthUsed = 0);

    private:
        AbsCopulaSmootherBase();
        AbsCopulaSmootherBase(const AbsCopulaSmootherBase&);
        AbsCopulaSmootherBase& operator=(const AbsCopulaSmootherBase&);

        HistoND<double>& clearHisto();
        void makeMarginalsUniform();
        void storeHisto(unsigned long uniqueId, double bw) const;

        // Method to implement in derived classes
        virtual void smoothHisto(HistoND<double>& histo,
                                 double effectiveSampleSize,
                                 double* bandwidthUsed,
                                 bool isSampleWeighted) = 0;
        HistoND<double>* h_;
        double tol_;
        unsigned shape_[CHAR_BIT*sizeof(unsigned long)];
        unsigned dim_;
        unsigned nCycles_;
        gs::AbsArchive* ar_;
        std::string category_;
    };
}

#include "npstat/stat/AbsCopulaSmootherBase.icc"

#endif // NPSTAT_ABSCOPULASMOOTHERBASE_HH_
