#ifndef NPSTAT_LOGRATIOTRANSFORM1D_HH_
#define NPSTAT_LOGRATIOTRANSFORM1D_HH_

/*!
// \file LogRatioTransform1D.hh
//
// \brief Log of the ratio 1-d coordinate transform, as in Johnson's S_B curve
//
// Author: I. Volobouev
//
// April 2015
*/

#include "npstat/stat/AbsDistributionTransform1D.hh"

namespace npstat {
    class LogRatioTransform1D : public AbsDistributionTransform1D
    {
    public:
        LogRatioTransform1D(double delta, double lambda, double gamma, double xi);
        inline virtual ~LogRatioTransform1D() {}

        inline virtual LogRatioTransform1D* clone() const
            {return new LogRatioTransform1D(*this);}

        double transformForward(double x, double* dydx) const;
        double transformBack(double y) const;
        inline bool isIncreasing() const {return true;}

        //@{
        /** Prototype needed for I/O */
        inline virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream&) const;
        //@}

        static inline const char* classname()
            {return "npstat::LogRatioTransform1D";}
        static inline unsigned version() {return 1;}
        static LogRatioTransform1D* read(const gs::ClassId& id, std::istream& is);

    protected:
        virtual bool isEqual(const AbsDistributionTransform1D&) const;

    private:
        inline void setParameterChecked(const unsigned which, const double value)
            {params_[which] = value;}
        inline void setAllParametersChecked(const double* p)
            {for (unsigned i=0; i<4U; ++i) {params_[i] = p[i];}}
        inline double getParameterChecked(const unsigned which) const
            {return params_[which];}

        double params_[4];
    };
}

#endif // NPSTAT_LOGRATIOTRANSFORM1D_HH_
