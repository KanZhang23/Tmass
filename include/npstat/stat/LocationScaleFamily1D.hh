#ifndef NPSTAT_LOCATIONSCALEFAMILY1D_HH_
#define NPSTAT_LOCATIONSCALEFAMILY1D_HH_

/*!
// \file LocationScaleFamily1D.hh
//
// \brief Create a location-scale family from a non-scalable 1-d distribution
//
// Author: I. Volobouev
//
// September 2017
*/

#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    class LocationScaleFamily1D : public AbsScalableDistribution1D
    {
    public:
        inline LocationScaleFamily1D(const AbsDistribution1D& distro,
                                     const double location, const double scale)
            : AbsScalableDistribution1D(location, scale), d_(distro.clone()) {}

        LocationScaleFamily1D(const LocationScaleFamily1D&);
        LocationScaleFamily1D& operator=(const LocationScaleFamily1D&);

        inline virtual LocationScaleFamily1D* clone() const
            {return new LocationScaleFamily1D(*this);}

        inline virtual ~LocationScaleFamily1D() {delete d_;}

        inline AbsDistribution1D& getUnderlyingDistro() {return *d_;}
        inline const AbsDistribution1D& getUnderlyingDistro() const
            {return *d_;}

        //@{
        /** Prototype needed for I/O */
        inline virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream&) const;
        //@}

        static inline const char* classname()
            {return "npstat::LocationScaleFamily1D";}
        static inline unsigned version() {return 1;}
        static LocationScaleFamily1D* read(
            const gs::ClassId& id, std::istream& is);

    protected:
        inline virtual bool isEqual(const AbsDistribution1D& other) const
        {
            const LocationScaleFamily1D& r = 
                static_cast<const LocationScaleFamily1D&>(other);
            return AbsScalableDistribution1D::isEqual(r) && *d_ == *r.d_;
        }

    private:
        inline LocationScaleFamily1D()
            : AbsScalableDistribution1D(0.0, 1.0), d_(0) {}

        inline virtual double unscaledDensity(const double x) const
            {return d_->density(x);}
        inline virtual double unscaledCdf(const double x) const
            {return d_->cdf(x);}
        inline virtual double unscaledExceedance(const double x) const
            {return d_->exceedance(x);}
        inline virtual double unscaledQuantile(const double x) const
            {return d_->quantile(x);}

        AbsDistribution1D* d_;
    };
}

#endif // NPSTAT_LOCATIONSCALEFAMILY1D_HH_
