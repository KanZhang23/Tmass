#include <cassert>
#include <stdexcept>

#include "npstat/nm/FejerQuadrature.hh"

namespace npstat {
    FejerQuadrature::FejerQuadrature(const unsigned npoints)
        : npoints_(npoints)
    {
        const long double pi = 3.14159265358979323846264338L;

        if (!npoints) throw std::invalid_argument(
            "In npstat::FejerQuadrature constructor:"
            " number of points must be positive");
        a_.reserve(npoints);
        w_.reserve(npoints);
        const unsigned mmax = npoints/2U;
        for (unsigned i=npoints; i; --i)
        {
            const long double theta = (2*i - 1U)*pi/(2U*npoints);
            a_.push_back(cosl(theta));
            long double sum = 0.0L;
            for (unsigned m=1; m<=mmax; ++m)
                sum += cosl(2U*m*theta)/(4U*static_cast<unsigned long>(m)*m - 1U);
            w_.push_back(2*(1.0L - 2.0L*sum)/npoints);
        }
    }

    unsigned FejerQuadrature::minimalExactRule(const unsigned polyDegree)
    {
        return polyDegree  + 1U;
    }

    void FejerQuadrature::getAbscissae(
        long double* abscissae, const unsigned len) const
    {
        if (len < npoints_) throw std::invalid_argument(
            "In npstat::FejerQuadrature::getAbscissae: "
            "unsifficient length of the output buffer");
        assert(abscissae);
        for (unsigned i=0; i<npoints_; ++i)
            abscissae[i] = a_[i];
    }

    void FejerQuadrature::getWeights(
        long double* weights, const unsigned len) const
    {
        if (len < npoints_) throw std::invalid_argument(
            "In npstat::FejerQuadrature::getWeights: "
            "unsifficient length of the output buffer");
        assert(weights);
        for (unsigned i=0; i<npoints_; ++i)
            weights[i] = w_[i];
    }
}
