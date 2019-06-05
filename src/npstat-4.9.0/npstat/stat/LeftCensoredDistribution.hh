#ifndef NPSTAT_LEFTCENSOREDDISTRIBUTION_HH_
#define NPSTAT_LEFTCENSOREDDISTRIBUTION_HH_

/*!
// \file LeftCensoredDistribution.hh
//
// \brief Distribution to represent left-censored data
//
// Author: I. Volobouev
//
// March 2013
*/

#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    class LeftCensoredDistribution : public AbsDistribution1D
    {
    public:
        /**
        // Constructor arguments are as follows:
        //
        //  visiblePart        -- the visible right tail of the distribution
        //
        //  visibleFraction    -- fraction of the overall distribution
        //                        represented by the right tail
        //
        //  effectiveNegativeInfinity -- where to place the Dirac delta
        //                        function representing the invisible part
        */
        LeftCensoredDistribution(const AbsDistribution1D& visiblePart,
                                 double visibleFraction,
                                 double effectiveNegativeInfinity);

        LeftCensoredDistribution(const LeftCensoredDistribution&);
        LeftCensoredDistribution& operator=(const LeftCensoredDistribution&);

        inline virtual LeftCensoredDistribution* clone() const
            {return new LeftCensoredDistribution(*this);}

        virtual ~LeftCensoredDistribution();

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
            {return "npstat::LeftCensoredDistribution";}
        static inline unsigned version() {return 1;}
        static LeftCensoredDistribution* read(
            const gs::ClassId& id, std::istream&);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        LeftCensoredDistribution();

        AbsDistribution1D* distro_;
        double frac_;
        double infty_;
    };
}

#endif // NPSTAT_LEFTCENSOREDDISTRIBUTION_HH_
