#ifndef NPSTAT_TRUNCATEDDISTRIBUTION1D_HH_
#define NPSTAT_TRUNCATEDDISTRIBUTION1D_HH_

/*!
// \file TruncatedDistribution1D.hh
//
// \brief 1-d continuous statistical distributions with truncated support
//
// Author: I. Volobouev
//
// December 2012
*/

#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    class TruncatedDistribution1D : public AbsDistribution1D
    {
    public:
        /**
        // Constructor arguments are as follows:
        //
        //  distro             -- distribution whose support we want to truncate
        //
        //  xmin, xmax         -- new limits for the support
        //
        //  minAndMaxAreForCDF -- if this argument is "true" then the limits
        //                        are given for the cumulative distribution
        //                        values and must both be between 0 and 1.
        //                        They will be internally converted into the
        //                        new limits of the support region.
        */
        TruncatedDistribution1D(const AbsDistribution1D& distro,
                                double xmin, double xmax,
                                bool minAndMaxAreForCDF = false);

        TruncatedDistribution1D(const TruncatedDistribution1D&);
        TruncatedDistribution1D& operator=(const TruncatedDistribution1D&);

        inline virtual TruncatedDistribution1D* clone() const
            {return new TruncatedDistribution1D(*this);}

        virtual ~TruncatedDistribution1D();

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
            {return "npstat::TruncatedDistribution1D";}
        static inline unsigned version() {return 1;}
        static TruncatedDistribution1D* read(
            const gs::ClassId& id, std::istream&);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        TruncatedDistribution1D();

        AbsDistribution1D* distro_;
        double xmin_;
        double xmax_;
        double cdfmin_;
        double cdfmax_;
        double exmin_;
    };
}

#endif // NPSTAT_TRUNCATEDDISTRIBUTION1D_HH_
