#include <cmath>

#include "npstat/nm/SpecialFunctions.hh"
#include "npstat/stat/ScalableGaussND.hh"
#include "npstat/stat/distributionReadError.hh"

namespace npstat {
    ScalableGaussND::ScalableGaussND(const double* location,
                                     const double* scale,
                                     const unsigned dim)
        : AbsScalableDistributionND(location, scale, dim)
    {
        norm_ = pow(2.0*M_PI, -0.5*dim);
    }

    double ScalableGaussND::unscaledDensity(const double* x) const
    {
        double sumsq = x[0]*x[0];
        for (unsigned i=1; i<dim_; ++i)
            sumsq += x[i]*x[i];
        return norm_*exp(-sumsq/2.0);
    }

    void ScalableGaussND::unscaledUnitMap(
        const double* rnd, const unsigned bufLen, double* x) const
    {
        for (unsigned i=0; i<bufLen; ++i)
            x[i] = inverseGaussCdf(rnd[i]);
    }

    bool ScalableGaussND::write(std::ostream& of) const
    {
        return AbsScalableDistributionND::write(of);
    }

    ScalableGaussND* ScalableGaussND::read(const gs::ClassId& id,
                                           std::istream& is)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<ScalableGaussND>());
        current.ensureSameId(id);

        unsigned dim = 0;
        std::vector<double> locations, scales;
        
        if (AbsScalableDistributionND::read(is, &dim, &locations, &scales))
            if (dim && locations.size() == dim && scales.size() == dim)
                return new ScalableGaussND(&locations[0], &scales[0], dim);
        distributionReadError(is, classname());        
        return 0;
    }
}
