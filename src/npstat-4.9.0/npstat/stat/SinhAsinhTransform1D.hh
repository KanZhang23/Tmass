#ifndef NPSTAT_SINHASINHTRANSFORM1D_HH_
#define NPSTAT_SINHASINHTRANSFORM1D_HH_

/*!
// \file SinhAsinhTransform1D.hh
//
// \brief Sinh-arcsinh 1-d coordinate transform, y = sinh(a + b*asinh(x))
//
// Author: I. Volobouev
//
// September 2017
*/

#include "npstat/stat/AbsDistributionTransform1D.hh"

namespace npstat {
    class SinhAsinhTransform1D : public AbsDistributionTransform1D
    {
    public:
        SinhAsinhTransform1D(double a, double b);
        inline virtual ~SinhAsinhTransform1D() {}

        inline virtual SinhAsinhTransform1D* clone() const
            {return new SinhAsinhTransform1D(*this);}

        double transformForward(double x, double* dydx) const;
        double transformBack(double y) const;
        inline bool isIncreasing() const {return params_[1] > 0.0;}

        //@{
        /** Prototype needed for I/O */
        inline virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream&) const;
        //@}

        static inline const char* classname()
            {return "npstat::SinhAsinhTransform1D";}
        static inline unsigned version() {return 1;}
        static SinhAsinhTransform1D* read(const gs::ClassId& id, std::istream& is);

    protected:
        virtual bool isEqual(const AbsDistributionTransform1D&) const;

    private:
        inline void setParameterChecked(const unsigned which, const double value)
            {params_[which] = value;}
        inline void setAllParametersChecked(const double* p)
            {for (unsigned i=0; i<2U; ++i) {params_[i] = p[i];}}
        inline double getParameterChecked(const unsigned which) const
            {return params_[which];}

        double params_[2];
    };
}

#endif // NPSTAT_SINHASINHTRANSFORM1D_HH_
