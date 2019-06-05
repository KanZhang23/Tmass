#ifndef NPSTAT_SCALABLEGAUSSND_HH_
#define NPSTAT_SCALABLEGAUSSND_HH_

/*!
// \file ScalableGaussND.hh
//
// \brief Multivariate Gaussian distribution
//
// Author: I. Volobouev
//
// March 2010
*/

#include <cmath>

#include "npstat/stat/AbsDistributionND.hh"

namespace npstat {
    /**
    // Multivariate Gaussian distribution. It can be shifted and scaled
    // but not rotated (its arguments of course, can be rotated independently).
    */
    class ScalableGaussND : public AbsScalableDistributionND
    {
    public:
        ScalableGaussND(const double* location,
                        const double* scale, unsigned dim);
        virtual ~ScalableGaussND() {}

        inline virtual ScalableGaussND* clone() const
            {return new ScalableGaussND(*this);}
        inline bool mappedByQuantiles() const {return true;}

        //@{
        /** Method needed for "geners" I/O */
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& of) const;
        //@}

        static inline const char* classname() {return "npstat::ScalableGaussND";}
        static inline unsigned version() {return 1;}
        static ScalableGaussND* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistributionND& r) const
            {return AbsScalableDistributionND::isEqual(r);}

    private:
        double unscaledDensity(const double* x) const;
        void unscaledUnitMap(const double* rnd, unsigned bufLen,
                             double* x) const;
        double norm_;

#ifdef SWIG
    public:
        inline ScalableGaussND(const double* ilocation, unsigned lenLocation,
                               const double* iscale, unsigned lenScale)
            : AbsScalableDistributionND(ilocation,lenLocation,iscale,lenScale)
              {norm_ = pow(2.0*M_PI, -0.5*lenScale);}
#endif
    };
}

#endif // NPSTAT_SCALABLEGAUSSND_HH_
