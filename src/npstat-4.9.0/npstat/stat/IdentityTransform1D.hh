#ifndef NPSTAT_IDENTITYTRANSFORM1D_HH_
#define NPSTAT_IDENTITYTRANSFORM1D_HH_

/*!
// \file IdentityTransform1D.hh
//
// \brief Identity 1-d coordinate transform, for use where a transform
//        is expected but not needed
//
// Author: I. Volobouev
//
// April 2015
*/

#include "npstat/stat/AbsDistributionTransform1D.hh"

namespace npstat {
    class IdentityTransform1D : public AbsDistributionTransform1D
    {
    public:
        /**
        // This transform can be used with one dummy parameter
        // which is stored internally and can be retrieved
        */
        inline explicit IdentityTransform1D(double p0=0.0)
            : AbsDistributionTransform1D(1U), p_(p0) {}

        inline virtual ~IdentityTransform1D() {}

        inline virtual IdentityTransform1D* clone() const
            {return new IdentityTransform1D(*this);}

        inline double transformForward(const double x, double* dydx) const
            {if (dydx) *dydx = 1.0; return x;}
        inline double transformBack(const double y) const {return y;}
        inline bool isIncreasing() const {return true;}

        //@{
        /** Prototype needed for I/O */
        inline virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream&) const;
        //@}

        static inline const char* classname()
            {return "npstat::IdentityTransform1D";}
        static inline unsigned version() {return 1;}
        static IdentityTransform1D* read(const gs::ClassId& id, std::istream&);

    protected:
        inline bool isEqual(const AbsDistributionTransform1D& o) const
        {
            const IdentityTransform1D& r = 
                static_cast<const IdentityTransform1D&>(o);
            return p_ == r.p_;
        }

    private:
        inline void setParameterChecked(unsigned, double v) {p_ = v;}
        inline void setAllParametersChecked(const double* p) {p_ = p[0];}
        inline double getParameterChecked(unsigned) const {return p_;}

        double p_;
    };
}

#endif // NPSTAT_IDENTITYTRANSFORM1D_HH_
