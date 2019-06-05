#include <cassert>
#include <stdexcept>

#include "npstat/nm/ClassicalOrthoPolys1D.hh"
#include "npstat/nm/SpecialFunctions.hh"

namespace npstat {
    JacobiOrthoPoly1D::JacobiOrthoPoly1D(const double a, const double b)
        : alpha_(a),
          beta_(b)
    {
        if (!(alpha_ > -1.0 && beta_ > -1.0)) throw std::invalid_argument(
            "In npstat::JacobiOrthoPoly1D constructor: invalid arguments");
        norm_ = powl(2.0L, alpha_+beta_+1)*Gamma(alpha_+1)*Gamma(beta_+1)/
            Gamma(alpha_+beta_+2);
    }

    long double JacobiOrthoPoly1D::weight(const long double x) const
    {
        if (x < -1.0L || x > 1.0L)
            return 0.0L;
        else
        {
            if (x == 1.0L && alpha_ < 0.0L) throw std::invalid_argument(
                "In npstat::JacobiOrthoPoly1D::weight: invalid x=1 argument");
            if (x == -1.0L && beta_ < 0.0L) throw std::invalid_argument(
                "In npstat::JacobiOrthoPoly1D::weight: invalid x=-1 argument");
            return powl(1 - x, alpha_)*powl(1 + x, beta_)/norm_;
        }
    }

    std::pair<long double,long double>
    JacobiOrthoPoly1D::recurrenceCoeffs(const unsigned k) const
    {
        long double a, b = 1.0L;
        if (k)
        {
            const long double tmp = 2*k + alpha_ + beta_;
            a = (beta_ - alpha_)*(beta_ + alpha_)/tmp/(tmp + 2);

            if (k == 1U)
                b = 4*k*(k+alpha_)*(k+beta_)/tmp/tmp/(tmp + 1);
            else
                b = 4*k*(k+alpha_)*(k+beta_)*(k+alpha_+beta_)/
                    tmp/tmp/(tmp + 1)/(tmp - 1);
        }
        else
            a = (beta_ - alpha_)/(beta_ + alpha_ + 2);
        return std::pair<long double,long double>(a, sqrtl(b));
    }

    long double HermiteProbOrthoPoly1D::weight(const long double x) const
    {
        const long double norm = 0.39894228040143267793994606L; // 1/sqrt(2 Pi)
        return norm*expl(-0.5*x*x);
    }

    std::pair<long double,long double>
    HermiteProbOrthoPoly1D::recurrenceCoeffs(const unsigned k) const
    {
        return std::pair<long double,long double>(0.0L, sqrtl(k));
    }
}
