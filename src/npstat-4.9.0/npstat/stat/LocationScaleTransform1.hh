#ifndef NPSTAT_LOCATIONSCALETRANSFORM1_HH_
#define NPSTAT_LOCATIONSCALETRANSFORM1_HH_

/*!
// \file LocationScaleTransform1.hh
//
// \brief Transform in which density location and scale depend on one parameter
//
// Author: I. Volobouev
//
// April 2015
*/

#include <cfloat>

#include "npstat/stat/AbsDistributionTransform1D.hh"

namespace npstat {
    template<class LocationFunctor, class ScaleFunctor>
    class LocationScaleTransform1 : public AbsDistributionTransform1D
    {
    public:
        inline LocationScaleTransform1(const LocationFunctor& location,
                                       const ScaleFunctor& scale,
                                       const double paramMin = -DBL_MAX,
                                       const double paramMax = DBL_MAX)
            : AbsDistributionTransform1D(1U),
              loc_(location),
              scale_(scale),
              param_(paramMin),
              paramMin_(paramMin),
              paramMax_(paramMax),
              changed_(true)
        {
        }

        inline virtual ~LocationScaleTransform1() {}

        inline virtual LocationScaleTransform1* clone() const 
            {return new LocationScaleTransform1(*this);}

        inline double getParamMin() const {return paramMin_;}
        inline double getParamMax() const {return paramMax_;}
        inline const LocationFunctor& getLocationFcn() const {return loc_;}
        inline const ScaleFunctor& getScaleFcn() const {return scale_;}
        inline double getLocation() const
            {if (changed_) recalculate(); return m_;}
        inline double getScale() const
            {if (changed_) recalculate(); return s_;}

        inline double transformForward(const double x, double* dydx) const
        {
            if (changed_) recalculate();
            if (dydx) *dydx = 1.0/s_;
            return (x - m_)/s_;
        }

        inline double transformBack(const double y) const
        {
            if (changed_) recalculate();
            return y*s_ + m_;
        }

        inline bool isIncreasing() const {return true;}

        //@{
        /** Prototype needed for I/O */
        inline virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream&) const;
        //@}

        static const char* classname()
        {
            static const std::string myClassName(
                gs::template_class_name<LocationFunctor,ScaleFunctor>(
                    "npstat::LocationScaleTransform1"));
            return myClassName.c_str();
        }
        static inline unsigned version() {return 1;}
        static LocationScaleTransform1* read(
            const gs::ClassId& id, std::istream&);

    protected:
        inline virtual bool isEqual(const AbsDistributionTransform1D& oth) const
        {
            const LocationScaleTransform1& r = 
                static_cast<const LocationScaleTransform1&>(oth);
            return loc_ == r.loc_ && scale_ == r.scale_ && param_ == r.param_ &&
                   paramMin_ == r.paramMin_ && paramMax_ == r.paramMax_;
        }

    private:
        inline void recalculate() const
        {
            const double s = scale_(param_);
            if (s <= 0.0) throw std::invalid_argument(
                "In npstat::LocationScaleTransform1::recalculate:"
                " obtained scale is not positive");
            m_ = loc_(param_);
            s_ = s;
            changed_ = false;
        }

        inline void setParameterChecked(unsigned, const double value)
        {
            if (value < paramMin_ || value > paramMax_)
                throw std::invalid_argument(
                    "In npstat::LocationScaleTransform1::setParameterChecked:"
                    " parameter value out of range");
            param_ = value;
            changed_ = true;
        }

        inline void setAllParametersChecked(const double* p)
        {
            if (p[0] < paramMin_ || p[0] > paramMax_)
                throw std::invalid_argument(
                    "In npstat::LocationScaleTransform1::setAllParametersChecked:"
                    " parameter value out of range");
            param_ = p[0];
            changed_ = true;
        }

        inline double getParameterChecked(unsigned) const {return param_;}

        LocationFunctor loc_;
        ScaleFunctor scale_;
        double param_;
        double paramMin_;
        double paramMax_;

        mutable double m_;
        mutable double s_;
        mutable bool changed_;
    };
}

#include "npstat/stat/LocationScaleTransform1.icc"

#endif // NPSTAT_LOCATIONSCALETRANSFORM1_HH_
