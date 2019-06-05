#ifndef NPSTAT_DISTRIBUTIONMIX1D_HH_
#define NPSTAT_DISTRIBUTIONMIX1D_HH_

/*!
// \file DistributionMix1D.hh
//
// \brief A mixture of one-dimensional statistical distributions
//
// Author: I. Volobouev
//
// June 2014
*/

#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    /** One-dimensional mixture of statistical distributions */
    class DistributionMix1D : public AbsDistribution1D
    {
    public:
        DistributionMix1D();

        DistributionMix1D(const DistributionMix1D&);
        DistributionMix1D& operator=(const DistributionMix1D&);

        /** "Virtual copy constructor" */
        inline virtual DistributionMix1D* clone() const
            {return new DistributionMix1D(*this);}

        virtual ~DistributionMix1D();

        /** 
        // Add a component to the mixture. Weight must be positive.
        // All weights will be normalized internally so that their
        // sum is 1.
        */
        DistributionMix1D& add(const AbsDistribution1D& distro, double weight);

        /**
        // Set all weights. Number of weights provided should be equal
        // to the number of components. Weights will be normalized internally.
        */
        void setWeights(const double* weights, unsigned nWeights);

        /** Number of mixture components */
        inline unsigned nComponents() const {return entries_.size();}

        /** Get the mixture component with the given number */
        inline const AbsDistribution1D& getComponent(const unsigned n) const
            {return *entries_.at(n);}

        /** Get the component weight */
        double getWeight(unsigned n) const;

        /** Probability density */
        double density(double x) const;

        /** Cumulative distribution function */
        double cdf(double x) const;

        /** 1 - cdf, implementations should avoid subtractive cancellation */
        double exceedance(double x) const;

        /** The quantile function */
        double quantile(double x) const;

        /** Random number generator according to this distribution */
        unsigned random(AbsRandomGenerator& g, double* r) const;

        //@{
        /** Prototype needed for I/O */
        virtual inline gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream&) const;
        //@}

        static inline const char* classname()
            {return "npstat::DistributionMix1D";}
        static inline unsigned version() {return 1;}
        static DistributionMix1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        void normalize();

        std::vector<const AbsDistribution1D*> entries_;
        std::vector<double> weights_;
        std::vector<double> weightCdf_;
        long double wsum_;
        bool isNormalized_;
    };
}

#endif // NPSTAT_DISTRIBUTIONMIX1D_HH_
