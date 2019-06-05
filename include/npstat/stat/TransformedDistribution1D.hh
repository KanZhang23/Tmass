#ifndef NPSTAT_TRANSFORMEDDISTRIBUTION1D_HH_
#define NPSTAT_TRANSFORMEDDISTRIBUTION1D_HH_

/*!
// \file TransformedDistribution1D.hh
//
// \brief Distribution in x for the given transform y(x) and distribution in y
//
// Author: I. Volobouev
//
// April 2015
*/

#include "npstat/stat/AbsDistribution1D.hh"
#include "npstat/stat/AbsDistributionTransform1D.hh"

namespace npstat {
    class TransformedDistribution1D : public AbsDistribution1D
    {
    public:
        TransformedDistribution1D(const AbsDistributionTransform1D& transform,
                                  const AbsDistribution1D& distro);
        TransformedDistribution1D(const TransformedDistribution1D&);
        TransformedDistribution1D& operator=(const TransformedDistribution1D&);

        inline virtual TransformedDistribution1D* clone() const
            {return new TransformedDistribution1D(*this);}

        inline virtual ~TransformedDistribution1D() {cleanup();}

        inline AbsDistributionTransform1D& getTransform() {return *t_;}
        inline const AbsDistributionTransform1D& getTransform() const
            {return *t_;}

        inline AbsDistribution1D& getUnderlyingDistro() {return *d_;}
        inline const AbsDistribution1D& getUnderlyingDistro() const
            {return *d_;}

        /** Probability density */
        virtual double density(double x) const;

        /** Cumulative distribution function */
        virtual double cdf(double x) const;

        /** 1 - cdf, implementations should avoid subtractive cancellation */
        virtual double exceedance(double x) const;

        /** The quantile function */
        virtual double quantile(double x) const;

        //@{
        /** Prototype needed for I/O */
        inline virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream&) const;
        //@}

        static inline const char* classname()
            {return "npstat::TransformedDistribution1D";}
        static inline unsigned version() {return 1;}
        static TransformedDistribution1D* read(
            const gs::ClassId& id, std::istream& is);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        TransformedDistribution1D();
        void cleanup();

        AbsDistributionTransform1D* t_;
        AbsDistribution1D* d_;
    };
}

#endif // NPSTAT_TRANSFORMEDDISTRIBUTION1D_HH_
