#ifndef NPSTAT_INTERPOLATEDDISTRO1D1P_HH_
#define NPSTAT_INTERPOLATEDDISTRO1D1P_HH_

/*!
// \file InterpolatedDistro1D1P.hh
//
// \brief Interpolation of 1-d distributions as a function of one parameter
//
// Author: I. Volobouev
//
// December 2014
*/

#include "geners/CPP11_shared_ptr.hh"

#include "npstat/nm/GridAxis.hh"
#include "npstat/stat/AbsInterpolatedDistribution1D.hh"

namespace npstat {
    class InterpolatedDistro1D1P : public AbsDistribution1D
    {
    public:
        InterpolatedDistro1D1P(
            const GridAxis& axis,
            const std::vector<CPP11_shared_ptr<const AbsDistribution1D> >& distros,
            double initialParameterValue = 0.0, bool interpolateVertically = false);

        inline virtual ~InterpolatedDistro1D1P() {delete interpolator_;}

        InterpolatedDistro1D1P(const InterpolatedDistro1D1P&);
        InterpolatedDistro1D1P& operator=(const InterpolatedDistro1D1P&);

        /** Set the value of the parameter */
        void setParameter(double value);

        /** Get the value of the parameter */
        inline double getParameter() const {return param_;}

        void interpolateVertically(bool b);
        inline bool interpolatingVertically() const {return vertical_;}

        /** "Virtual copy constructor" */
        inline virtual InterpolatedDistro1D1P* clone() const
            {return new InterpolatedDistro1D1P(*this);}

        inline double density(const double x) const
            {return interpolator_->density(x);}
        inline double cdf(const double x) const
            {return interpolator_->cdf(x);}
        inline double exceedance(const double x) const
            {return interpolator_->exceedance(x);}
        inline double quantile(const double x) const
            {return interpolator_->quantile(x);}

        //@{
        /** Prototype needed for I/O */
        virtual inline gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream&) const;
        //@}

        static inline const char* classname()
            {return "npstat::InterpolatedDistro1D1P";}
        static inline unsigned version() {return 2;}
        static InterpolatedDistro1D1P* read(const gs::ClassId& id,
                                            std::istream& in);
    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        InterpolatedDistro1D1P();

        GridAxis axis_;
        std::vector<CPP11_shared_ptr<const AbsDistribution1D> > distros_;
        AbsInterpolatedDistribution1D* interpolator_;
        double param_;
        bool vertical_;
    };
}

#endif // NPSTAT_INTERPOLATEDDISTRO1D1P_HH_
