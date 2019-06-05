#ifndef NPSTAT_RIGHTCENSOREDDISTRIBUTION_HH_
#define NPSTAT_RIGHTCENSOREDDISTRIBUTION_HH_

/*!
// \file RightCensoredDistribution.hh
//
// \brief Distribution to represent right-censored data
//
// Author: I. Volobouev
//
// March 2013
*/

#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    class RightCensoredDistribution : public AbsDistribution1D
    {
    public:
        /**
        // Constructor arguments are as follows:
        //
        //  visiblePart        -- the visible left tail of the distribution
        //
        //  visibleFraction    -- fraction of the overall distribution
        //                        represented by the left tail
        //
        //  effectivePositiveInfinity -- where to place the Dirac delta
        //                        function representing the invisible part
        */
        RightCensoredDistribution(const AbsDistribution1D& visiblePart,
                                  double visibleFraction,
                                  double effectivePositiveInfinity);

        RightCensoredDistribution(const RightCensoredDistribution&);
        RightCensoredDistribution& operator=(const RightCensoredDistribution&);

        inline virtual RightCensoredDistribution* clone() const
            {return new RightCensoredDistribution(*this);}

        virtual ~RightCensoredDistribution();

        //@{
        /** Simple inspector of the object properties */
        inline const AbsDistribution1D& visible() const {return *distro_;}
        inline double visibleFraction() const {return frac_;}
        inline double effectiveInfinity() const {return infty_;}
        //@}

        /** Distribution density */
        virtual double density(double x) const;

        /** Cumulative distribution function */
        virtual double cdf(double x) const;

        /** 1 - cdf, avoiding subtractive cancellation */
        virtual double exceedance(double x) const;

        /** The quantile function */
        virtual double quantile(double x) const;

        //@{
        /** Method needed for I/O */
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;
        //@}

        static inline const char* classname()
            {return "npstat::RightCensoredDistribution";}
        static inline unsigned version() {return 1;}
        static RightCensoredDistribution* read(
            const gs::ClassId& id, std::istream&);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        RightCensoredDistribution();

        AbsDistribution1D* distro_;
        double frac_;
        double infty_;
    };
}

#endif // NPSTAT_RIGHTCENSOREDDISTRIBUTION_HH_
