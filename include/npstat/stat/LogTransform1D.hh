#ifndef NPSTAT_LOGTRANSFORM1D_HH_
#define NPSTAT_LOGTRANSFORM1D_HH_

/*!
// \file LogTransform1D.hh
//
// \brief Transform y = log(1 + x). Useful for long-tailed distributions
//        supported on x >= 0.
//
// Author: I. Volobouev
//
// April 2015
*/

#include <cmath>
#include "npstat/stat/AbsDistributionTransform1D.hh"

namespace npstat {
    class LogTransform1D : public AbsDistributionTransform1D
    {
    public:
        inline LogTransform1D() : AbsDistributionTransform1D(0U) {}
        inline virtual ~LogTransform1D() {}

        inline virtual LogTransform1D* clone() const
            {return new LogTransform1D(*this);}

        inline double transformForward(const double x, double* dydx) const
            {if (dydx) *dydx = 1.0/(1.0 + x); return log(1.0 + x);}
        inline double transformBack(const double y) const {return exp(y)-1.0;}
        inline bool isIncreasing() const {return true;}

        //@{
        /** Prototype needed for I/O */
        inline virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        inline virtual bool write(std::ostream&) const {return true;}
        //@}

        static inline const char* classname()
            {return "npstat::LogTransform1D";}
        static inline unsigned version() {return 1;}
        static LogTransform1D* read(const gs::ClassId& id, std::istream&);

    protected:
        inline bool isEqual(const AbsDistributionTransform1D&) const
            {return true;}

    private:
        inline void setParameterChecked(unsigned, double) {}
        inline void setAllParametersChecked(const double*) {}
        inline double getParameterChecked(unsigned) const {return 0.0;}
    };
}

#endif // NPSTAT_LOGTRANSFORM1D_HH_
