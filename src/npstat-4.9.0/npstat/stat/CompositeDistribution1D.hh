#ifndef NPSTAT_COMPOSITEDISTRIBUTION1D_HH_
#define NPSTAT_COMPOSITEDISTRIBUTION1D_HH_

/*!
// \file CompositeDistribution1D.hh
//
// \brief Composite distributions in one dimension
//
// Author: I. Volobouev
//
// April 2010
*/

#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    /**
    // This class represents univariate statistical distributions whose
    // cumulative distribution functions F(x) can be built by composition
    // of two other cumulative distribution functions: F(x) = G(H(x)).
    // Naturally, G must correspond to a density supported on the interval
    // [0, 1]. This density is often easier to fit (or to represent
    // nonparametrically) than the original one in case H(x) is well chosen.
    //
    // The corresponding technique is known in the KDE-related literature
    // as the "nonparametric transformations" method. See, for example,
    // "A Comparison of Higher-Order Bias Kernel Density Estimators" by
    // M.C. Jones and D.F. Signorini, JASA, Vol. 92, No. 439, p. 1063 (1997).
    */
    class CompositeDistribution1D : public AbsDistribution1D
    {
    public:
        /**
        // Constructor arguments are the distributions used to build this
        // composite. The support of pG must be the [0, 1] interval.
        */
        CompositeDistribution1D(const AbsDistribution1D& pG,
                                const AbsDistribution1D& pH);

        CompositeDistribution1D(const CompositeDistribution1D&);
        CompositeDistribution1D& operator=(const CompositeDistribution1D&);

        inline virtual ~CompositeDistribution1D() {cleanup();}

        /** Probability density */
        inline double density(const double x) const
            {return pH_->density(x)*pG_->density(pH_->cdf(x));}

        /** Cumulative distribution function */
        inline double cdf(const double x) const
            {return pG_->cdf(pH_->cdf(x));}

        /** Exceedance (i.e., 1 - cdf) */
        inline double exceedance(const double x) const
            {return pG_->exceedance(pH_->cdf(x));}

        /** Quantile function */
        inline double quantile(const double x) const
            {return pH_->quantile(pG_->quantile(x));}

        /** "Virtual copy constructor" */
        inline virtual CompositeDistribution1D* clone() const
            {return new CompositeDistribution1D(*this);}

        /** Fetch the distribution with support on [0, 1] */
        inline const AbsDistribution1D& G() const {return *pG_;}

        /** Fetch the distribution with arbitrary support */
        inline const AbsDistribution1D& H() const {return *pH_;}

        //@{
        /** Method needed for "geners" I/O */
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;
        //@}

        static inline const char* classname()
            {return "npstat::CompositeDistribution1D";}
        static inline unsigned version() {return 1;}
        static CompositeDistribution1D* read(
            const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

        CompositeDistribution1D();

        const AbsDistribution1D* pG_;
        const AbsDistribution1D* pH_;

    private:
        void cleanup();
    };
}

#endif // NPSTAT_COMPOSITEDISTRIBUTION1D_HH_
