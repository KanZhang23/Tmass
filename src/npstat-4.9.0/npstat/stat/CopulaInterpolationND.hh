#ifndef NPSTAT_COPULAINTERPOLATIONND_HH_
#define NPSTAT_COPULAINTERPOLATIONND_HH_

/*!
// \file CopulaInterpolationND.hh
//
// \brief Interpolation of multivariate statistical distributions decomposed
//        into copula and marginals
//
// Author: I. Volobouev
//
// March 2010
*/

#include "npstat/stat/AbsInterpolationAlgoND.hh"
#include "npstat/stat/InterpolatedDistribution1D.hh"

namespace npstat {
    /**
    // This class interpolates multivariate statistical distributions
    // decomposed into copula and marginals. Interpolation weights are
    // to be calculated outside of this class.
    */
    class CopulaInterpolationND : public AbsInterpolationAlgoND
    {
    public:
        /**
        // Constructor arguments are the dimensionality of the density
        // and the number of terms in the weighted sum which represents
        // the interpolated density. The terms are added to the sum
        // later, by calling "add" or "replace" methods. At least
        // "nInterpolated" distributions must be added by the "add"
        // method before calling the "density" method.
        */
        CopulaInterpolationND(unsigned dim, unsigned nInterpolated);

        inline virtual ~CopulaInterpolationND() {}

        inline virtual CopulaInterpolationND* clone() const
            {return new CopulaInterpolationND(*this);}

        inline bool mappedByQuantiles() const {return false;}

        //@{
        /**
        // In this method, "d" must refer to a CompositeDistributionND object
        */
        void add(const AbsDistributionND& d, double weight);
        void replace(unsigned i, const AbsDistributionND& d, double weight);
        //@}

        /** Set the weight for the given term in the weighted sum */
        void setWeight(unsigned i, double weight);

        /** Clear all the terms in the weighted sum */
        void clear();

        /**
        // This method should be called to disable
        // (and later enable) automatic weight normalization
        // if you want to use the "setWeight" or "replace" methods
        // many times and, especially, if at some point in this process
        // the sum of the weights becomes zero. The "density" and
        // "unitMap" methods can not be called if normalization
        // is not enabled.
        */
        void normalizeAutomatically(bool allow);

        /** The number of terms in the weighted sum */
        inline unsigned size() const {return copulas_.size();}

        //@{
        /** Interpolate the complete multivariate distribution */
        double density(const double* x, unsigned dim) const;
        void unitMap(const double* rnd, unsigned dim, double* x) const;
        //@}

        /** Interpolate the copula density */
        double copulaDensity(const double* x, unsigned dim) const;

        /** Interpolate the product of the marginals */
        double productOfTheMarginals(const double* x, unsigned dim) const;

        //@{
        /** Interpolate the marginal distribution for dimension "idim" */
        double marginalDensity(unsigned idim, double x) const;
        double marginalCdf(unsigned idim, double x) const;
        double marginalExceedance(unsigned idim, double x) const;
        double marginalQuantile(unsigned idim, double x) const;
        //@}

        /** Method needed for "geners" I/O */
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}

        // Class name for the I/O
        static inline const char* classname()
            {return "npstat::CopulaInterpolationND";}

        // Version number for the I/O
        static inline unsigned version() {return 1;}

    protected:
        virtual bool isEqual(const AbsDistributionND&) const;

    private:
        void normalize();

        std::vector<InterpolatedDistribution1D> marginInterpolators_;
        std::vector<WeightedND> copulas_;
        std::vector<double> wCdf_;
        mutable std::vector<double> work_;
        double wsum_;
        unsigned nInterpolated_;
        bool autoNormalize_;
    };
}

#endif // NPSTAT_COPULAINTERPOLATIONND_HH_
