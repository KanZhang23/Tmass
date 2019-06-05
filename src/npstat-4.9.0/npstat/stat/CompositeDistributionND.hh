#ifndef NPSTAT_COMPOSITEDISTRIBUTIONND_HH_
#define NPSTAT_COMPOSITEDISTRIBUTIONND_HH_

/*!
// \file CompositeDistributionND.hh
//
// \brief Multivariate statistical distributions decomposed into copula
//        and marginals
//
// Author: I. Volobouev
//
// March 2010
*/

#include <vector>

#include "npstat/nm/ArrayND.hh"

#include "npstat/stat/AbsDistributionND.hh"
#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    /**
    // This class represents multivariate statistical distributions
    // decomposed into copula and marginals
    */
    class CompositeDistributionND : public AbsDistributionND
    {
    public:
        /**
        // Constructor from pre-calculated copula and the marginals.
        // If "assumePointerOwnership" is true then all argument
        // distributions will be deleted in the destructor. If
        // "assumePointerOwnership" is false then the constructor
        // will build copies of all distributions (which will be
        // later deleted in the destructor).
        */
        CompositeDistributionND(
            const AbsDistributionND* copula,
            const std::vector<const AbsDistribution1D*> marginals,
            bool assumePointerOwnership=false);

        /**
        // Constructor from a histogram. The copula and the marginals
        // are generated internally. "interpolationDegree" should be
        // either 0 or 1 (limited by the abilities of BinnedDensityND class).
        */
        template <typename Num1, unsigned Len1, unsigned Dim1>
        CompositeDistributionND(
            const double* location, const double* scale,
            unsigned locationAndScaleLength,
            const ArrayND<Num1,Len1,Dim1>& histogram,
            const ArrayShape& copulaShape,
            const unsigned interpolationDegree);

        CompositeDistributionND(const CompositeDistributionND&);
        CompositeDistributionND& operator=(const CompositeDistributionND&);

        virtual ~CompositeDistributionND();

        inline virtual CompositeDistributionND* clone() const
            {return new CompositeDistributionND(*this);}

        inline bool mappedByQuantiles() const {return true;}

        /** Return the copula */
        inline const AbsDistributionND* copula() const
            {return copula_;}

        /** Return the marginal distribution for the given dimension */
        inline const AbsDistribution1D* marginal(unsigned index) const
            {return marginals_.at(index);}

        double density(const double* x, unsigned dim) const;
        double copulaDensity(const double* x, unsigned dim) const;
        double productOfTheMarginals(const double* x, unsigned dim) const;

        void unitMap(const double* rnd, unsigned dim, double* x) const;

        //@{
        /** Method needed for "geners" I/O */
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& of) const;
        //@}

        static inline const char* classname()
            {return "npstat::CompositeDistributionND";}
        static inline unsigned version() {return 1;}

        static CompositeDistributionND* read(
            const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistributionND&) const;

    private:
        void cleanup();

        const AbsDistributionND* copula_;
        std::vector<const AbsDistribution1D*> marginals_;
        mutable std::vector<double> work_;
    };
}

#include "npstat/stat/CompositeDistributionND.icc"

#endif // NPSTAT_COMPOSITEDISTRIBUTIONND_HH_
