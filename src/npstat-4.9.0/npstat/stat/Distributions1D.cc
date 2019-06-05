#include <cmath>
#include <cfloat>
#include <algorithm>
#include <stdexcept>

#include "geners/binaryIO.hh"

#include "npstat/nm/MathUtils.hh"
#include "npstat/nm/SpecialFunctions.hh"
#include "npstat/nm/interpolate.hh"

#include "npstat/stat/Distributions1D.hh"
#include "npstat/stat/StatUtils.hh"
#include "npstat/stat/distributionReadError.hh"

#define SQR2PI 2.5066282746310005
#define SQRT2  1.41421356237309505
#define SQRPI  1.77245385090551603

#define SQRT2L 1.414213562373095048801689L
#define SQRPIL 1.77245385090551602729816748L
#define TWOPIL 6.28318530717958647692528676656L

static long double inverseErf(const long double fval)
{
    long double x = npstat::inverseGaussCdf((fval + 1.0L)/2.0L)/SQRT2L;
    for (unsigned i=0; i<2; ++i)
    {
        const long double guessed = erfl(x);
        const long double deri = 2.0L/SQRPIL*expl(-x*x);
        x += (fval - guessed)/deri;
    }
    return x;
}

static unsigned improved_random(npstat::AbsRandomGenerator& g,
                                long double* generatedRandom)
{
    const long double extra = sqrt(DBL_EPSILON);

    long double u = 0.0L;
    unsigned callcount = 0;
    while (u <= 0.0L || u >= 1.0L)
    {
        u = g()*(1.0L + extra) - extra/2.0L;
        u += (g() - 0.5L)*extra;
        callcount += 2U;
    }
    *generatedRandom = u;
    return callcount;
}

static unsigned gauss_random(const double mean, const double sigma,
                             npstat::AbsRandomGenerator& g,
                             double* generatedRandom)
{
    assert(generatedRandom);
    long double r1 = 0.0L, r2 = 0.0L;
    const unsigned calls = improved_random(g, &r1) + improved_random(g, &r2);
    *generatedRandom = mean + sigma*sqrtl(-2.0L*logl(r1))*sinl(TWOPIL*(r2-0.5L));
    return calls;
}

// static unsigned gauss_random(const double mean, const double sigma,
//                              npstat::AbsRandomGenerator& g,
//                              double* generatedRandom)
// {
//     assert(generatedRandom);
//     long double r1 = 0.0L;
//     const unsigned count = improved_random(g, &r1);
//     *generatedRandom = mean + sigma*SQRT2*inverseErf(2.0L*r1 - 1.0L);
//     return count;
// }

namespace npstat {
    bool SymmetricBeta1D::write(std::ostream& os) const
    {
        AbsScalableDistribution1D::write(os);
        gs::write_pod(os, n_);
        return !os.fail();
    }

    SymmetricBeta1D* SymmetricBeta1D::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<SymmetricBeta1D>());
        current.ensureSameId(id);

        double location, scale;
        if (AbsScalableDistribution1D::read(in, &location, &scale))
        {
            double n;
            gs::read_pod(in, &n);
            if (!in.fail())
                return new SymmetricBeta1D(location, scale, n);
        }
        distributionReadError(in, classname());
        return 0;
    }

    bool SymmetricBeta1D::isEqual(const AbsDistribution1D& otherBase) const
    {
        const SymmetricBeta1D& r = 
            static_cast<const SymmetricBeta1D&>(otherBase);
        return AbsScalableDistribution1D::isEqual(r) && n_ == r.n_;
    }

    SymmetricBeta1D::SymmetricBeta1D(const double location,
                                     const double scale,
                                     const double power)
        : AbsScalableDistribution1D(location, scale),
          n_(power)
    {
        norm_ = calculateNorm();
    }

    SymmetricBeta1D::SymmetricBeta1D(const double location,
                                     const double scale,
                                     const std::vector<double>& params)
        : AbsScalableDistribution1D(location, scale),
          n_(params[0])
    {
        norm_ = calculateNorm();
    }

    double SymmetricBeta1D::calculateNorm() const
    {
        static const double normcoeffs[11] = {
            0.5, 0.75, 0.9375, 1.09375, 1.23046875, 1.353515625,
            1.46630859375, 1.571044921875, 1.6692352294921875,
            1.76197052001953125, 1.85006904602050781};

        if (n_ <= -1.0) throw std::invalid_argument(
            "In npstat::SymmetricBeta1D::calculateNorm: "
            "invalid power parameter");

        const int intpow = static_cast<int>(floor(n_));
        if (static_cast<double>(intpow) == n_ &&
            intpow >= 0 && intpow <= 10)
            return normcoeffs[intpow];
        else
            return Gamma(1.5 + n_)/sqrt(M_PI)/Gamma(1.0 + n_);
    }

    double SymmetricBeta1D::unscaledDensity(const double x) const
    {
        const double oneminusrsq = 1.0 - x*x;
        if (oneminusrsq <= 0.0)
            return 0.0;
        else
            return norm_*pow(oneminusrsq, n_);
    }

    double SymmetricBeta1D::unscaledCdf(const double x) const
    {
        if (x >= 1.0)
            return 1.0;
        else if (x <= -1.0)
            return 0.0;
        else if (n_ == 0.0)
            return (x + 1.0)/2.0;
        else
            return incompleteBeta(n_+1.0, n_+1.0, (x + 1.0)/2.0);
    }

    double SymmetricBeta1D::unscaledQuantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::SymmetricBeta1D::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        if (r1 == 0.0)
            return -1.0;
        else if (r1 == 1.0)
            return 1.0;
        else
        {
            double r;
            if (n_ == 0.0)
                r = r1*2.0 - 1.0;
            else
                r = 2.0*inverseIncompleteBeta(n_+1.0, n_+1.0, r1) - 1.0;
            if (r < -1.0)
                r = -1.0;
            else if (r > 1.0)
                r = 1.0;
            return r;
        }
    }

    bool Beta1D::write(std::ostream& os) const
    {
        AbsScalableDistribution1D::write(os);
        gs::write_pod(os, alpha_);
        gs::write_pod(os, beta_);
        return !os.fail();
    }

    Beta1D* Beta1D::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<Beta1D>());
        current.ensureSameId(id);

        double location, scale, a, b;
        if (AbsScalableDistribution1D::read(in, &location, &scale))
        {
            gs::read_pod(in, &a);
            gs::read_pod(in, &b);
            if (!in.fail())
                return new Beta1D(location, scale, a, b);
        }
        distributionReadError(in, classname());
        return 0;
    }

    bool Beta1D::isEqual(const AbsDistribution1D& otherBase) const
    {
        const Beta1D& r = static_cast<const Beta1D&>(otherBase);
        return AbsScalableDistribution1D::isEqual(r) && 
               alpha_ == r.alpha_ && beta_ == r.beta_;
    }

    Beta1D::Beta1D(const double location, const double scale,
                   const double pa, const double pb)
        : AbsScalableDistribution1D(location, scale),
          alpha_(pa),
          beta_(pb)
    {
        norm_ = calculateNorm();
    }

    Beta1D::Beta1D(const double location,
                   const double scale,
                   const std::vector<double>& params)
        : AbsScalableDistribution1D(location, scale),
          alpha_(params[0]),
          beta_(params[1])
    {
        norm_ = calculateNorm();
    }

    double Beta1D::calculateNorm() const
    {
        if (!(alpha_ > 0.0 && beta_ > 0.0)) throw std::invalid_argument(
            "In npstat::Beta1D::calculateNorm: invalid power parameters");
        return Gamma(alpha_ + beta_)/Gamma(alpha_)/Gamma(beta_);
    }

    double Beta1D::unscaledDensity(const double x) const
    {
        if (x <= 0.0 || x >= 1.0)
            return 0.0;
        else if (alpha_ == 1.0 && beta_ == 1.0)
            return 1.0;
        else
            return norm_*pow(x, alpha_-1.0)*pow(1.0-x, beta_-1.0);
    }

    double Beta1D::unscaledCdf(const double x) const
    {
        if (x >= 1.0)
            return 1.0;
        else if (x <= 0.0)
            return 0.0;
        else if (alpha_ == 1.0 && beta_ == 1.0)
            return x;
        else
            return incompleteBeta(alpha_, beta_, x);
    }

    double Beta1D::unscaledExceedance(const double x) const
    {
        return 1.0 - unscaledCdf(x);
    }

    double Beta1D::unscaledQuantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::Beta1D::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        if (r1 == 0.0)
            return 0.0;
        else if (r1 == 1.0)
            return 1.0;
        else if (alpha_ == 1.0 && beta_ == 1.0)
            return r1;
        else
            return inverseIncompleteBeta(alpha_, beta_, r1);
    }

    Gamma1D::Gamma1D(const double location, const double scale,
                     const double a)
        : AbsScalableDistribution1D(location, scale),
          alpha_(a)
    {
        initialize();
    }

    Gamma1D::Gamma1D(double location, double scale,
                     const std::vector<double>& params)
        : AbsScalableDistribution1D(location, scale),
          alpha_(params[0])
    {
        initialize();
    }

    void Gamma1D::initialize()
    {
        if (!(alpha_ > 0.0)) throw std::invalid_argument(
            "In npstat::Gamma1D::initialize: invalid power parameter");
        norm_ = 1.0/Gamma(alpha_);
    }

    bool Gamma1D::isEqual(const AbsDistribution1D& otherBase) const
    {
        const Gamma1D& r = static_cast<const Gamma1D&>(otherBase);
        return AbsScalableDistribution1D::isEqual(r) && alpha_ == r.alpha_;
    }

    double Gamma1D::unscaledDensity(const double x) const
    {
        if (x > 0.0)
            return norm_*pow(x, alpha_-1.0)*exp(-x);
        else
            return 0.0;
    }

    double Gamma1D::unscaledCdf(const double x) const
    {
        if (x > 0.0)
            return incompleteGamma(alpha_, x);
        else
            return 0.0;
    }

    double Gamma1D::unscaledExceedance(const double x) const
    {
        if (x > 0.0)
            return incompleteGammaC(alpha_, x);
        else
            return 1.0;
    }

    double Gamma1D::unscaledQuantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::Gamma1D::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        return inverseIncompleteGamma(alpha_, r1);
    }

    bool Gamma1D::write(std::ostream& os) const
    {
        AbsScalableDistribution1D::write(os);
        gs::write_pod(os, alpha_);
        return !os.fail();
    }

    Gamma1D* Gamma1D::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<Gamma1D>());
        current.ensureSameId(id);

        double location, scale, a;
        if (AbsScalableDistribution1D::read(in, &location, &scale))
        {
            gs::read_pod(in, &a);
            if (!in.fail())
                return new Gamma1D(location, scale, a);
        }
        distributionReadError(in, classname());
        return 0;
    }

    double Gauss1D::unscaledDensity(const double x) const
    {
        return exp(-x*x/2.0)/SQR2PI;
    }

    unsigned Gauss1D::random(AbsRandomGenerator& g,
                             double* generatedRandom) const
    {
        return gauss_random(location(), scale(), g, generatedRandom);
    }

    Gauss1D* Gauss1D::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<Gauss1D>());
        current.ensureSameId(id);

        double location, scale;
        if (!AbsScalableDistribution1D::read(in, &location, &scale))
        {
            distributionReadError(in, classname());
            return 0;
        }
        return new Gauss1D(location, scale);
    }

    double Gauss1D::unscaledCdf(const double x) const
    {
        if (x < 0.0)
            return erfc(-x/SQRT2)/2.0;
        else
            return (1.0 + erf(x/SQRT2))/2.0;
    }

    double Gauss1D::unscaledExceedance(const double x) const
    {
        if (x > 0.0)
            return erfc(x/SQRT2)/2.0;
        else
            return (1.0 - erf(x/SQRT2))/2.0;
    }

    double Gauss1D::unscaledQuantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::Gauss1D::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        return inverseGaussCdf(r1);
    }

    double Uniform1D::unscaledDensity(const double x) const
    {
        if (x >= 0.0 && x <= 1.0)
            return 1.0;
        else
            return 0.0;
    }

    Uniform1D* Uniform1D::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<Uniform1D>());
        current.ensureSameId(id);

        double location, scale;
        if (!AbsScalableDistribution1D::read(in, &location, &scale))
        {
            distributionReadError(in, classname());
            return 0;
        }
        return new Uniform1D(location, scale);
    }

    double Uniform1D::unscaledCdf(const double x) const
    {
        if (x <= 0.0)
            return 0.0;
        else if (x >= 1.0)
            return 1.0;
        else
            return x;
    }

    double Uniform1D::unscaledQuantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::Uniform1D::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        return r1;
    }

    double IsoscelesTriangle1D::unscaledDensity(const double x) const
    {
        if (x > -1.0 && x < 1.0)
            return 1.0 - fabs(x);
        else
            return 0.0;
    }

    IsoscelesTriangle1D* IsoscelesTriangle1D::read(
        const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<IsoscelesTriangle1D>());
        current.ensureSameId(id);

        double location, scale;
        if (!AbsScalableDistribution1D::read(in, &location, &scale))
        {
            distributionReadError(in, classname());
            return 0;
        }
        return new IsoscelesTriangle1D(location, scale);
    }

    double IsoscelesTriangle1D::unscaledCdf(const double x) const
    {
        if (x <= -1.0)
            return 0.0;
        else if (x >= 1.0)
            return 1.0;
        else if (x <= 0.0)
        {
            const double tmp = 1.0 + x;
            return 0.5*tmp*tmp;
        }
        else
        {
            const double tmp = 1.0 - x;
            return 1.0 - 0.5*tmp*tmp;
        }
    }

    double IsoscelesTriangle1D::unscaledQuantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::IsoscelesTriangle1D::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        if (r1 == 0.0)
            return -1.0;
        else if (r1 == 1.0)
            return 1.0;
        else if (r1 <= 0.5)
            return sqrt(2.0*r1) - 1.0;
        else
            return 1.0 - sqrt((1.0 - r1)*2.0);
    }

    Exponential1D* Exponential1D::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<Exponential1D>());
        current.ensureSameId(id);

        double location, scale;
        if (!AbsScalableDistribution1D::read(in, &location, &scale))
        {
            distributionReadError(in, classname());
            return 0;
        }
        return new Exponential1D(location, scale);
    }

    double Exponential1D::unscaledDensity(const double x) const
    {
        if (x < 0.0)
            return 0.0;
        const double eval = exp(-x);
        return eval < DBL_MIN ? 0.0 : eval;
    }

    double Exponential1D::unscaledCdf(const double x) const
    {
        return x > 0.0 ? 1.0 - exp(-x) : 0.0;
    }

    double Exponential1D::unscaledQuantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::Exponential1D::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        if (r1 == 1.0)
            return -log(DBL_MIN);
        else
            return -log(1.0 - r1);
    }

    double Exponential1D::unscaledExceedance(const double x) const
    {
        if (x < 0.0)
            return 1.0;
        const double eval = exp(-x);
        return eval < DBL_MIN ? 0.0 : eval;
    }

    Logistic1D* Logistic1D::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<Logistic1D>());
        current.ensureSameId(id);

        double location, scale;
        if (!AbsScalableDistribution1D::read(in, &location, &scale))
        {
            distributionReadError(in, classname());
            return 0;
        }
        return new Logistic1D(location, scale);
    }

    double Logistic1D::unscaledDensity(const double x) const
    {
        const double eval = exp(-x);
        if (eval < DBL_MIN)
            return 0.0;
        else
        {
            const double tmp = 1.0 + eval;
            return eval/tmp/tmp;
        }
    }

    double Logistic1D::unscaledCdf(const double x) const
    {
        const double lmax = -log(DBL_MIN);
        if (x <= -lmax)
            return 0.0;
        else if (x >= lmax)
            return 1.0;
        else
            return 1.0/(1.0 + exp(-x));
    }

    double Logistic1D::unscaledQuantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::Logistic1D::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        const double lmax = -log(DBL_MIN);
        if (r1 == 0.0)
            return -lmax;
        else if (r1 == 1.0)
            return lmax;
        else
            return log(r1/(1.0 - r1));
    }

    double Logistic1D::unscaledExceedance(const double x) const
    {
        const double lmax = -log(DBL_MIN);
        if (x >= lmax)
            return 0.0;
        else if (x <= -lmax)
            return 1.0;
        else
        {
            const double eval = exp(-x);
            return eval/(1.0 + eval);
        }
    }

    bool Quadratic1D::write(std::ostream& os) const
    {
        AbsScalableDistribution1D::write(os);
        gs::write_pod(os, a_);
        gs::write_pod(os, b_);
        return !os.fail();
    }

    Quadratic1D* Quadratic1D::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<Quadratic1D>());
        current.ensureSameId(id);

        double location, scale;
        if (AbsScalableDistribution1D::read(in, &location, &scale))
        {
            double a, b;
            gs::read_pod(in, &a);
            gs::read_pod(in, &b);
            if (!in.fail())
                return new Quadratic1D(location, scale, a, b);
        }
        distributionReadError(in, classname());
        return 0;
    }

    bool Quadratic1D::isEqual(const AbsDistribution1D& otherBase) const
    {
        const Quadratic1D& r = static_cast<const Quadratic1D&>(otherBase);
        return AbsScalableDistribution1D::isEqual(r) &&
               a_ == r.a_ && b_ == r.b_;
    }

    Quadratic1D::Quadratic1D(const double location, const double scale,
                             const double a, const double b)
        : AbsScalableDistribution1D(location, scale), a_(a), b_(b)
    {
        verifyNonNegative();
    }

    Quadratic1D::Quadratic1D(const double location, const double scale,
                             const std::vector<double>& params)
        : AbsScalableDistribution1D(location, scale),
          a_(params[0]),
          b_(params[1])
    {
        verifyNonNegative();
    }

    void Quadratic1D::verifyNonNegative()
    {
        const double a = 2.0*a_;
        const double b = 2.0*b_;
        if (b == 0.0)
        {
            if (fabs(a) > 1.0)
                throw std::invalid_argument(
                    "In npstat::Quadratic1D::verifyNonNegative:"
                    " invalid distribution parameters");
        }
        else
        {
            double x1 = 0.0, x2 = 0.0;
            const double sixb = 6*b;
            if (solveQuadratic((2*a-sixb)/sixb, (1-a+b)/sixb, &x1, &x2))
            {
                if (!(fabs(x1 - 0.5) >= 0.5 && fabs(x2 - 0.5) >= 0.5))
                    throw std::invalid_argument(
                        "In npstat::Quadratic1D::verifyNonNegative:"
                        " invalid distribution parameters");
            }
        }
    }

    double Quadratic1D::unscaledDensity(const double x) const
    {
        if (x < 0.0 || x > 1.0)
            return 0.0;
        else
            return 1.0 + 2.0*(b_ - a_ + x*(2.0*a_ + 6.0*b_*(x - 1.0)));
    }

    double Quadratic1D::unscaledCdf(const double x) const
    {
        if (x <= 0.0)
            return 0.0;
        else if (x >= 1.0)
            return 1.0;
        else
            return x*(1.0 + 2.0*(b_ - a_ + x*(a_ - 3.0*b_ + 2.0*b_*x)));
    }

    double Quadratic1D::unscaledQuantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::Quadratic1D::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        if (r1 == 0.0)
            return 0.0;
        else if (r1 == 1.0)
            return 1.0;
        else
        {
            const double a = 2.0*a_;
            const double b = 2.0*b_;
            if (b == 0.0)
            {
                if (a == 0.0)
                    return r1;
                else
                {
                    double x0 = 0.0, x1 = 0.0;
                    const unsigned n = solveQuadratic(
                        (1.0 - a)/a, -r1/a, &x0, &x1);
                    if (!n) throw std::runtime_error(
                        "In npstat::Quadratic1D::unscaledQuantile: "
                        "no solutions");
                    if (fabs(x0 - 0.5) < fabs(x1 - 0.5))
                        return x0;
                    else
                        return x1;
                }
            }
            else
            {
                const double twob = 2*b;
                double x[3] = {0.0};
                const unsigned n = solveCubic(
                    (a - 3*b)/twob, (1 - a + b)/twob, -r1/twob, x);
                if (n == 1U)
                    return x[0];
                else
                {
                    unsigned ibest = 0;
                    double dbest = fabs(x[0] - 0.5);
                    for (unsigned i=1; i<n; ++i)
                        if (fabs(x[i] - 0.5) < dbest)
                        {
                            ibest = i;
                            dbest = fabs(x[i] - 0.5);
                        }
                    return x[ibest];
                }
            }
        }
    }

    bool LogQuadratic1D::write(std::ostream& os) const
    {
        AbsScalableDistribution1D::write(os);
        gs::write_pod(os, a_);
        gs::write_pod(os, b_);
        return !os.fail();
    }

    LogQuadratic1D* LogQuadratic1D::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<LogQuadratic1D>());
        current.ensureSameId(id);

        double location, scale, a, b;
        if (AbsScalableDistribution1D::read(in, &location, &scale))
        {
            gs::read_pod(in, &a);
            gs::read_pod(in, &b);
            if (!in.fail())
                return new LogQuadratic1D(location, scale, a, b);
        }
        distributionReadError(in, classname());
        return 0;
    }

    bool LogQuadratic1D::isEqual(const AbsDistribution1D& otherBase) const
    {
        const LogQuadratic1D& r = static_cast<const LogQuadratic1D&>(
            otherBase);
        return AbsScalableDistribution1D::isEqual(r) &&
               a_ == r.a_ && b_ == r.b_;
    }

    LogQuadratic1D::LogQuadratic1D(const double location, const double scale,
                                   const double a, const double b)
        : AbsScalableDistribution1D(location, scale), a_(a), b_(b)
    {
        normalize();
    }

    LogQuadratic1D::LogQuadratic1D(const double location, const double scale,
                                   const std::vector<double>& params)
        : AbsScalableDistribution1D(location, scale),
          a_(params[0]),
          b_(params[1])
    {
        normalize();
    }

    inline long double LogQuadratic1D::quadInteg(const long double x) const
    {
        return dawsonIntegral(x)*expl(x*x);
    }

    void LogQuadratic1D::normalize()
    {
        ref_ = 0.0L;
        range_ = 1.0L;
        k_ = 0.0;
        s_ = 0.0;
        norm_ = 1.0;

        const double b = 2.0*b_;
        const double a = 2.0*a_;

        if (b > DBL_EPSILON)
        {
            k_ = sqrt(6.0*b);
            s_ = 0.5 - a/(6.0*b);
            ref_ = quadInteg(k_*s_);
            range_ = ref_ - quadInteg(k_*(s_ - 1.0));
            norm_ = k_/range_;
        }
        else if (b < -DBL_EPSILON)
        {
            k_ = sqrt(-6.0*b);
            s_ = 0.5 - a/(6.0*b);
            ref_ = erfl(k_*s_);
            range_ = ref_ - erfl(k_*(s_ - 1.0));
            norm_ = 2.0*k_/SQRPI/range_;
        }
        else if (fabs(a) > DBL_EPSILON)
        {
            range_ = expl(2.0L*a) - 1.0L;
            if (fabs(a) > 1.e-10)
                norm_ = a/sinh(a);
        }
    }

    double LogQuadratic1D::unscaledDensity(const double x) const
    {
        if (x < 0.0 || x > 1.0)
            return 0.0;

        const double b = 2.0*b_;
        if (fabs(b) > DBL_EPSILON)
        {
            const double delta = x - s_;
            return norm_*exp(6.0*b*delta*delta);
        }

        const double a = 2.0*a_;
        if (fabs(a) > DBL_EPSILON)
            return norm_*exp((2.0*x - 1.0)*a);
        else
            return 1.0;
    }

    double LogQuadratic1D::unscaledCdf(const double x) const
    {
        if (x <= 0.0)
            return 0.0;
        else if (x >= 1.0)
            return 1.0;
        else
        {
            const double b = 2.0*b_;
            const double a = 2.0*a_;
            if (b > DBL_EPSILON)
                return (ref_ - quadInteg(k_*(s_ - x)))/range_;
            else if (b < -DBL_EPSILON)
                return (ref_ - erfl(k_*(s_ - x)))/range_;
            else if (fabs(a) > DBL_EPSILON)
                return (expl(2.0L*a*x) - 1.0L)/range_;
            else
                return x;
        }
    }

    double LogQuadratic1D::unscaledQuantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::LogQuadratic1D::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        if (r1 == 0.0)
            return 0.0;
        else if (r1 == 1.0)
            return 1.0;
        else
        {
            const double b = 2.0*b_;
            const double a = 2.0*a_;
            double q = 0.0;
            if (b > DBL_EPSILON)
                q = s_ - inverseExpsqIntegral(ref_ - r1*range_)/k_;
            else if (b < -DBL_EPSILON)
                q = s_ - inverseErf(ref_ - r1*range_)/k_;
            else if (fabs(a) > DBL_EPSILON)
                q = logl(r1*range_ + 1.0L)/2.0/a;
            else
                q = r1;

            if (q < 0.0)
                q = 0.0;
            else if (q > 1.0)
                q = 1.0;
            return q;
        }
    }

    bool TruncatedGauss1D::write(std::ostream& os) const
    {
        AbsScalableDistribution1D::write(os);
        gs::write_pod(os, nsigma_);
        return !os.fail();
    }

    TruncatedGauss1D* TruncatedGauss1D::read(
        const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<TruncatedGauss1D>());
        current.ensureSameId(id);

        double location, scale, nsig;
        if (AbsScalableDistribution1D::read(in, &location, &scale))
        {
            gs::read_pod(in, &nsig);
            if (!in.fail())
                return new TruncatedGauss1D(location, scale, nsig);
        }
        distributionReadError(in, classname());
        return 0;
    }

    bool TruncatedGauss1D::isEqual(const AbsDistribution1D& otherBase) const
    {
        const TruncatedGauss1D& r = 
            static_cast<const TruncatedGauss1D&>(otherBase);
        return AbsScalableDistribution1D::isEqual(r) && nsigma_ == r.nsigma_;
    }

    void TruncatedGauss1D::initialize()
    {
        if (nsigma_ <= 0.0) throw std::invalid_argument(
            "In npstat::TruncatedGauss1D::initialize: "
            "invalid truncation parameter");
        cdf0_ = erfc(nsigma_/SQRT2)/2.0;
        const double u = (1.0 + erf(nsigma_/SQRT2))/2.0;
        norm_ = 1.0/(u - cdf0_);
    }

    TruncatedGauss1D::TruncatedGauss1D(const double location,
                                       const double scale,
                                       const double i_nsigma)
        : AbsScalableDistribution1D(location, scale),
          nsigma_(i_nsigma)
    {
        initialize();
    }

    TruncatedGauss1D::TruncatedGauss1D(const double location,
                                       const double scale,
                                       const std::vector<double>& params)
        : AbsScalableDistribution1D(location, scale),
          nsigma_(params[0])
    {
        initialize();
    }

    double TruncatedGauss1D::unscaledDensity(const double x) const
    {
        if (fabs(x) > nsigma_)
            return 0.0;
        else
            return norm_*exp(-x*x/2.0)/SQR2PI;
    }

    unsigned TruncatedGauss1D::random(AbsRandomGenerator& g,
                                      double* generatedRandom) const
    {
        const double m = location();
        const double s = scale();
        unsigned cnt = gauss_random(m, s, g, generatedRandom);
        while (fabs(*generatedRandom - m) > nsigma_*s)
            cnt += gauss_random(m, s, g, generatedRandom);
        return cnt;
    }

    double TruncatedGauss1D::unscaledCdf(const double x) const
    {
        if (x <= -nsigma_)
            return 0.0;
        else if (x >= nsigma_)
            return 1.0;
        else if (x < 0.0)
            return (erfc(-x/SQRT2)/2.0 - cdf0_)*norm_;
        else
            return ((1.0 + erf(x/SQRT2))/2.0 - cdf0_)*norm_;
    }

    double TruncatedGauss1D::unscaledQuantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::TruncatedGauss1D::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        if (r1 == 0.0)
            return -nsigma_;
        else if (r1 == 1.0)
            return nsigma_;
        else
            return inverseGaussCdf(r1/norm_ + cdf0_);
    }

    bool MirroredGauss1D::isEqual(const AbsDistribution1D& otherBase) const
    {
        const MirroredGauss1D& r = 
            static_cast<const MirroredGauss1D&>(otherBase);
        return AbsScalableDistribution1D::isEqual(r) && 
            mu0_ == r.mu0_ && sigma0_ == r.sigma0_;
    }

    MirroredGauss1D::MirroredGauss1D(const double location, const double scale,
                                     const double mean, double const sigma)
        : AbsScalableDistribution1D(location, scale),
          mu0_(mean),
          sigma0_(sigma)
    {
        validate();
    }

    MirroredGauss1D::MirroredGauss1D(const double location,
                                     const double scale,
                                     const std::vector<double>& params)
        : AbsScalableDistribution1D(location, scale),
          mu0_(params[0]),
          sigma0_(params[1])
    {
        validate();
    }

    bool MirroredGauss1D::write(std::ostream& os) const
    {
        AbsScalableDistribution1D::write(os);
        gs::write_pod(os, mu0_);
        gs::write_pod(os, sigma0_);
        return !os.fail();
    }

    MirroredGauss1D* MirroredGauss1D::read(
        const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<MirroredGauss1D>());
        current.ensureSameId(id);

        double location, scale, mu, sig;
        if (AbsScalableDistribution1D::read(in, &location, &scale))
        {
            gs::read_pod(in, &mu);
            gs::read_pod(in, &sig);
            if (!in.fail())
                return new MirroredGauss1D(location, scale, mu, sig);
        }
        distributionReadError(in, classname());
        return 0;
    }

    double MirroredGauss1D::unscaledDensity(const double x) const
    {
        if (x < 0.0 || x > 1.0)
            return 0.0;
        Gauss1D g(x, sigma0_);
        long double acc = g.density(mu0_)*1.0L + g.density(-mu0_);
        for (unsigned k=1; ; ++k)
        {
            const long double old = acc;
            acc += g.density(2.0*k + mu0_);
            acc += g.density(2.0*k - mu0_);
            acc += g.density(-2.0*k + mu0_);
            acc += g.density(-2.0*k - mu0_);
            if (old == acc)
                break;
        }
        return acc;
    }

    long double MirroredGauss1D::ldCdf(const double x) const
    {
        if (x <= 0.0)
            return 0.0L;
        else if (x >= 1.0)
            return 1.0L;
        else
        {
            long double acc = 0.0L;
            {
                Gauss1D g(mu0_, sigma0_);
                acc += (g.cdf(x) - g.cdf(0.0));
            }
            {
                Gauss1D g(-mu0_, sigma0_);
                acc += (g.cdf(x) - g.cdf(0.0));
            }
            for (unsigned k=1; ; ++k)
            {
                const long double old = acc;
                {
                    Gauss1D g(2.0*k + mu0_, sigma0_);
                    acc += (g.cdf(x) - g.cdf(0.0));
                }
                {
                    Gauss1D g(2.0*k - mu0_, sigma0_);
                    acc += (g.cdf(x) - g.cdf(0.0));
                }
                {
                    Gauss1D g(-2.0*k + mu0_, sigma0_);
                    acc -= (g.exceedance(x) - g.exceedance(0.0));
                }
                {
                    Gauss1D g(-2.0*k - mu0_, sigma0_);
                    acc -= (g.exceedance(x) - g.exceedance(0.0));
                }
                if (old == acc)
                    break;
            }
            return acc;
        }
    }

    double MirroredGauss1D::unscaledCdf(const double x) const
    {
        return ldCdf(x);
    }

    double MirroredGauss1D::unscaledExceedance(const double x) const
    {
        return 1.0L - ldCdf(x);
    }

    double MirroredGauss1D::unscaledQuantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::MirroredGauss1D::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        if (r1 == 0.0)
            return 0.0;
        else if (r1 == 1.0)
            return 1.0;
        else
        {
            const long double ldr1 = r1;
            double xmin = 0.0;
            double xmax = 1.0;
            for (unsigned i=0; i<1000; ++i)
            {
                if ((xmax - xmin)/xmax <= 2.0*DBL_EPSILON)
                    break;
                const double xtry = (xmin + xmax)/2.0;
                const long double ld = ldCdf(xtry);
                if (ld == ldr1)
                    return xtry;
                else if (ld > ldr1)
                    xmax = xtry;
                else
                    xmin = xtry;
            }
            return (xmin + xmax)/2.0;
        }
    }

    void MirroredGauss1D::validate()
    {
        if (mu0_ < 0.0 || mu0_ > 1.0) throw std::invalid_argument(
            "In MirroredGauss1D::validate: interval mean must be within [0, 1]");
        if (sigma0_ <= 0.0) throw std::invalid_argument(
            "In MirroredGauss1D::validate: interval sigma must be positive");
    }

    BifurcatedGauss1D::BifurcatedGauss1D(
        const double location, const double scale,
        const double i_leftSigmaFraction,
        const double i_nSigmasLeft, const double i_nSigmasRight)
        : AbsScalableDistribution1D(location, scale),
          leftSigma_(i_leftSigmaFraction*2.0),
          rightSigma_(2.0 - leftSigma_),
          nSigmasLeft_(i_nSigmasLeft),
          nSigmasRight_(i_nSigmasRight)
    {
        initialize();
    }

    BifurcatedGauss1D::BifurcatedGauss1D(const double location,
                                         const double scale,
                                         const std::vector<double>& params)
        : AbsScalableDistribution1D(location, scale),
          leftSigma_(params[0]*2.0),
          rightSigma_(2.0 - leftSigma_),
          nSigmasLeft_(params[1]),
          nSigmasRight_(params[2])
    {
        initialize();
    }

    void BifurcatedGauss1D::initialize()
    {
        if (leftSigma_ < 0.0 || rightSigma_ < 0.0)
            throw std::invalid_argument(
                "In npstat::BifurcatedGauss1D::initialize: "
                "invalid left sigma fraction");
        if (nSigmasLeft_ < 0.0) throw std::invalid_argument(
            "In npstat::BifurcatedGauss1D::initialize: "
            "invalid left truncation parameter");
        if (nSigmasRight_ < 0.0) throw std::invalid_argument(
            "In npstat::BifurcatedGauss1D::initialize: "
            "invalid right truncation parameter");
        if (nSigmasLeft_ + nSigmasRight_ == 0.0) throw std::invalid_argument(
            "In npstat::BifurcatedGauss1D::initialize: "
            "both truncation parameters can not be 0");

        const double maxNSigma = inverseGaussCdf(1.0);
        if (nSigmasRight_ > maxNSigma)
            nSigmasRight_ = maxNSigma;
        if (nSigmasLeft_ > maxNSigma)
            nSigmasLeft_ = maxNSigma;

        cdf0Left_ = erfc(nSigmasLeft_/SQRT2)/2.0;
        cdf0Right_ = (1.0 + erf(nSigmasRight_/SQRT2))/2.0;
        assert(cdf0Right_ > cdf0Left_);

        const double leftArea = (0.5 - cdf0Left_)*leftSigma_;
        const double rightArea = (cdf0Right_ - 0.5)*rightSigma_;
        norm_ = 1.0/(leftArea + rightArea);
        leftCdfFrac_ = leftArea/(leftArea + rightArea);
    }

    double BifurcatedGauss1D::unscaledDensity(const double x) const
    {
        if (x == 0.0)
            return norm_/SQR2PI;
        else if (x > 0.0)
        {
            if (x > rightSigma_*nSigmasRight_)
                return 0.0;
            else
            {
                const double dx = x/rightSigma_;
                return norm_*exp(-dx*dx/2.0)/SQR2PI;
            }
        }
        else
        {
            if (x < -leftSigma_*nSigmasLeft_)
                return 0.0;
            else
            {
                const double dx = x/leftSigma_;
                return norm_*exp(-dx*dx/2.0)/SQR2PI;
            }
        }
    }

    double BifurcatedGauss1D::unscaledCdf(const double x) const
    {
        if (x == 0.0)
            return leftCdfFrac_;
        else if (x > 0.0)
        {
            if (x > rightSigma_*nSigmasRight_)
                return 1.0;
            else
            {
                const double dx = x/rightSigma_;
                const double cdfDelta = (1.0 + erf(dx/SQRT2))/2.0 - cdf0Right_;
                return 1.0 + cdfDelta*(1.0 - leftCdfFrac_)/(cdf0Right_ - 0.5);
            }
        }
        else
        {
            if (x < -leftSigma_*nSigmasLeft_)
                return 0.0;
            else
            {
                const double dx = x/leftSigma_;
                const double cdfDelta = erfc(-dx/SQRT2)/2.0 - cdf0Left_;
                return cdfDelta*leftCdfFrac_/(0.5 - cdf0Left_);
            }
        }
    }

    double BifurcatedGauss1D::unscaledExceedance(const double x) const
    {
        return 1.0 - unscaledCdf(x);
    }

    double BifurcatedGauss1D::unscaledQuantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::BifurcatedGauss1D::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        if (r1 == 0.0)
            return -nSigmasLeft_*leftSigma_;
        else if (r1 == 1.0)
            return nSigmasRight_*rightSigma_;
        else
        {
            if (r1 == leftCdfFrac_)
                return 0.0;
            else if (r1 < leftCdfFrac_)
            {
                // Map 0 into cdf0Left_ and leftCdfFrac_ into 0.5
                const double arg = r1/leftCdfFrac_*(0.5-cdf0Left_) + cdf0Left_;
                return leftSigma_*inverseGaussCdf(arg);
            }
            else
            {
                // Map leftCdfFrac_ into 0.5 and 1.0 into cdf0Right_
                const double d = (r1 - leftCdfFrac_)/(1.0 - leftCdfFrac_);
                const double arg = 0.5 + d*(cdf0Right_ - 0.5);
                return rightSigma_*inverseGaussCdf(arg);
            }
        }
    }

    bool BifurcatedGauss1D::isEqual(const AbsDistribution1D& otherBase) const
    {
        const BifurcatedGauss1D& r =
            static_cast<const BifurcatedGauss1D&>(otherBase);
        return AbsScalableDistribution1D::isEqual(r) &&
            leftSigma_ == r.leftSigma_ &&
            rightSigma_ == r.rightSigma_ &&
            nSigmasLeft_ == r.nSigmasLeft_ &&
            nSigmasRight_ == r.nSigmasRight_ &&
            norm_ == r.norm_ &&
            leftCdfFrac_ == r.leftCdfFrac_ &&
            cdf0Left_ == r.cdf0Left_ &&
            cdf0Right_ == r.cdf0Right_;
    }

    bool BifurcatedGauss1D::write(std::ostream& os) const
    {
        AbsScalableDistribution1D::write(os);
        gs::write_pod(os, leftSigma_);
        gs::write_pod(os, rightSigma_);
        gs::write_pod(os, nSigmasLeft_);
        gs::write_pod(os, nSigmasRight_);
        gs::write_pod(os, norm_);
        gs::write_pod(os, leftCdfFrac_);
        gs::write_pod(os, cdf0Left_);
        gs::write_pod(os, cdf0Right_);
        return !os.fail();
    }

    BifurcatedGauss1D::BifurcatedGauss1D(const double location,
                                         const double scale)
        : AbsScalableDistribution1D(location, scale)
    {
    }

    BifurcatedGauss1D* BifurcatedGauss1D::read(
        const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<BifurcatedGauss1D>());
        current.ensureSameId(id);

        double location, scale;
        if (AbsScalableDistribution1D::read(in, &location, &scale))
        {
            BifurcatedGauss1D* ptr = new BifurcatedGauss1D(location, scale);
            gs::read_pod(in, &ptr->leftSigma_);
            gs::read_pod(in, &ptr->rightSigma_);
            gs::read_pod(in, &ptr->nSigmasLeft_);
            gs::read_pod(in, &ptr->nSigmasRight_);
            gs::read_pod(in, &ptr->norm_);
            gs::read_pod(in, &ptr->leftCdfFrac_);
            gs::read_pod(in, &ptr->cdf0Left_);
            gs::read_pod(in, &ptr->cdf0Right_);
            if (!in.fail() &&
                ptr->leftSigma_ >= 0.0 &&
                ptr->rightSigma_ >= 0.0 &&
                ptr->leftSigma_ + ptr->rightSigma_ > 0.0 &&
                ptr->nSigmasLeft_ >= 0.0 &&
                ptr->nSigmasRight_ >= 0.0 &&
                ptr->nSigmasLeft_ + ptr->nSigmasRight_ > 0.0 &&
                ptr->norm_ > 0.0 &&
                ptr->leftCdfFrac_ >= 0.0 &&
                ptr->cdf0Left_ >= 0.0 &&
                ptr->cdf0Right_ > ptr->cdf0Left_)
                return ptr;
            else
                delete ptr;
        }
        distributionReadError(in, classname());
        return 0;
    }

    Cauchy1D::Cauchy1D(const double location, const double scale,
                       const std::vector<double>& /* params */)
        : AbsScalableDistribution1D(location, scale),
          support_(sqrt(DBL_MAX/M_PI))
    {
    }

    Cauchy1D::Cauchy1D(const double location, const double scale)
        : AbsScalableDistribution1D(location, scale),
          support_(sqrt(DBL_MAX/M_PI))
    {
    }

    Cauchy1D* Cauchy1D::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<Cauchy1D>());
        current.ensureSameId(id);

        double location, scale;
        if (!AbsScalableDistribution1D::read(in, &location, &scale))
        {
            distributionReadError(in, classname());
            return 0;
        }
        return new Cauchy1D(location, scale);
    }

    double Cauchy1D::unscaledDensity(const double x) const
    {
        if (fabs(x) < support_)
            return 1.0/M_PI/(1.0 + x*x);
        else
            return 0.0;
    }

    double Cauchy1D::unscaledCdf(const double x) const
    {
        if (x < -support_)
            return 0.0;
        else if (x > support_)
            return 1.0;
        else
            return atan(x)/M_PI + 0.5;
    }

    double Cauchy1D::unscaledQuantile(const double x) const
    {
        if (!(x >= 0.0 && x <= 1.0)) throw std::domain_error(
            "In npstat::Cauchy1D::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        if (x == 0.0)
            return -support_;
        else if (x == 1.0)
            return support_;
        else
            return tan(M_PI*(x - 0.5));
    }

    bool LogNormal::isEqual(const AbsDistribution1D& otherBase) const
    {
        const LogNormal& r = static_cast<const LogNormal&>(otherBase);
        return AbsScalableDistribution1D::isEqual(r) && 
               skew_ == r.skew_;
    }

    bool LogNormal::write(std::ostream& os) const
    {
        AbsScalableDistribution1D::write(os);
        gs::write_pod(os, skew_);
        return !os.fail();
    }

    LogNormal* LogNormal::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<LogNormal>());
        current.ensureSameId(id);

        double location, scale, skew;
        if (AbsScalableDistribution1D::read(in, &location, &scale))
        {
            gs::read_pod(in, &skew);
            if (!in.fail())
                return new LogNormal(location, scale, skew);
        }
        distributionReadError(in, classname());
        return 0;
    }

    void LogNormal::initialize()
    {
        logw_ = 0.0;
        s_ = 0.0;
        xi_ = 0.0;
        emgamovd_ = 0.0;

        if (skew_)
        {
            const double b1 = skew_*skew_;
            const double tmp = pow((2.0+b1+sqrt(b1*(4.0+b1)))/2.0, 1.0/3.0);
            const double w = tmp + 1.0/tmp - 1.0;
            logw_ = log(w);
            if (logw_ > 0.0)
            {
                s_ = sqrt(logw_);
                emgamovd_ = 1.0/sqrt(w*(w-1.0));
                xi_ = -emgamovd_*sqrt(w);
            }
            else
            {
                // This is not different from a Gaussian within
                // the numerical precision of our calculations
                logw_ = 0.0;
                skew_ = 0.0;
            }
        }
    }

    LogNormal::LogNormal(const double mean, const double stdev,
                         const double skewness)
        : AbsScalableDistribution1D(mean, stdev),
          skew_(skewness)
    {
        initialize();
    }

    LogNormal::LogNormal(const double mean, const double stdev,
                         const std::vector<double>& params)
        : AbsScalableDistribution1D(mean, stdev),
          skew_(params[0])
    {
        initialize();
    }

    double LogNormal::unscaledDensity(const double x) const
    {
        if (skew_)
        {
            const double diff = skew_ > 0.0 ? x - xi_ : -x - xi_;
            if (diff <= 0.0)
                return 0.0;
            else
            {
                const double lg = log(diff/emgamovd_);
                return exp(-lg*lg/2.0/logw_)/s_/SQR2PI/diff;
            }
        }
        else
        {
            // This is a Gaussian
            return exp(-x*x/2.0)/SQR2PI;
        }
    }

    double LogNormal::unscaledCdf(const double x) const
    {
        if (skew_)
        {
            const double diff = skew_ > 0.0 ? x - xi_ : -x - xi_;
            double posCdf = 0.0;
            if (diff > 0.0)
                posCdf = (1.0 + erf(log(diff/emgamovd_)/s_/SQRT2))/2.0;
            return skew_ > 0.0 ? posCdf : 1.0 - posCdf;
        }
        else
            return (1.0 + erf(x/SQRT2))/2.0;
    }

    double LogNormal::unscaledExceedance(const double x) const
    {
        return 1.0 - unscaledCdf(x);
    }

    double LogNormal::unscaledQuantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::LogNormal::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        const double g = inverseGaussCdf(skew_ >= 0.0 ? r1 : 1.0 - r1);
        if (skew_)
        {
            const double v = emgamovd_*exp(s_*g) + xi_;
            return skew_ > 0.0 ? v : -v;
        }
        else
            return g;
    }

    Moyal1D::Moyal1D(const double location, const double scale)
        : AbsScalableDistribution1D(location, scale),
          xmax_(-2.0*log(DBL_MIN*SQR2PI)),
          xmin_(-log(xmax_))
    {
    }

    Moyal1D::Moyal1D(const double location, const double scale,
                     const std::vector<double>& /* params */)
        : AbsScalableDistribution1D(location, scale),
          xmax_(-2.0*log(DBL_MIN*SQR2PI)),
          xmin_(-log(xmax_))
    {
    }

    double Moyal1D::unscaledDensity(const double x) const
    {
        if (x <= xmin_ || x >= xmax_)
            return 0.0;
        else
            return exp(-0.5*(x + exp(-x)))/SQR2PI;
    }

    double Moyal1D::unscaledCdf(const double x) const
    {
        if (x <= xmin_)
            return 0.0;
        else if (x >= xmax_)
            return 1.0;
        else
            return incompleteGammaC(0.5, 0.5*exp(-x));
    }

    double Moyal1D::unscaledExceedance(const double x) const
    {
        if (x <= xmin_)
            return 1.0;
        else if (x >= xmax_)
            return 0.0;
        else
            return incompleteGamma(0.5, 0.5*exp(-x));
    }

    double Moyal1D::unscaledQuantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::Moyal1D::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        if (r1 == 0.0)
            return xmin_;
        else if (r1 == 1.0)
            return xmax_;
        else
        {
            const double d = inverseIncompleteGammaC(0.5, r1);
            return -log(2.0*d);
        }
    }

    Moyal1D* Moyal1D::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<Moyal1D>());
        current.ensureSameId(id);

        double location, scale;
        if (!AbsScalableDistribution1D::read(in, &location, &scale))
        {
            distributionReadError(in, classname());
            return 0;
        }
        return new Moyal1D(location, scale);
    }

    bool Pareto1D::write(std::ostream& os) const
    {
        AbsScalableDistribution1D::write(os);
        gs::write_pod(os, c_);
        return !os.fail();
    }

    Pareto1D* Pareto1D::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<Pareto1D>());
        current.ensureSameId(id);

        double location, scale, c;
        if (AbsScalableDistribution1D::read(in, &location, &scale))
        {
            gs::read_pod(in, &c);
            if (!in.fail())
                return new Pareto1D(location, scale, c);
        }
        distributionReadError(in, classname());
        return 0;
    }

    bool Pareto1D::isEqual(const AbsDistribution1D& otherBase) const
    {
        const Pareto1D& r = static_cast<const Pareto1D&>(otherBase);
        return AbsScalableDistribution1D::isEqual(r) && 
               c_ == r.c_;
    }

    void Pareto1D::initialize()
    {
        if (c_ <= 0.0) throw std::invalid_argument(
            "In npstat::Pareto1D::initialize: power parameter must be positive");
        if (c_ > 1.0)
            support_ = pow(1.0/DBL_MIN, 1.0/c_);
        else
            support_ = 1.0/DBL_MIN;
    }

    Pareto1D::Pareto1D(const double location, const double scale,
                       const double powerParam)
        : AbsScalableDistribution1D(location, scale),
          c_(powerParam)
    {
        initialize();
    }

    Pareto1D::Pareto1D(const double location, const double scale,
                       const std::vector<double>& params)
        : AbsScalableDistribution1D(location, scale),
          c_(params[0])
    {
        initialize();
    }

    double Pareto1D::unscaledDensity(const double x) const
    {
        if (x < 1.0 || x > support_)
            return 0.0;
        else
            return c_/pow(x, c_ + 1.0);
    }

    double Pareto1D::unscaledCdf(const double x) const
    {
        if (x <= 1.0)
            return 0.0;
        else if (x >= support_)
            return 1.0;
        else
            return 1.0 - 1.0/pow(x, c_);
    }

    double Pareto1D::unscaledQuantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::Pareto1D::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        if (r1 == 0.0)
            return 1.0;
        else if (r1 == 1.0)
            return support_;
        else
            return pow(1.0 - r1, -1.0/c_);
    }

    double Pareto1D::unscaledExceedance(const double x) const
    {
        if (x <= 1.0)
            return 1.0;
        else if (x >= support_)
            return 0.0;
        else
            return 1.0/pow(x, c_);
    }

    bool UniPareto1D::write(std::ostream& os) const
    {
        AbsScalableDistribution1D::write(os);
        gs::write_pod(os, c_);
        return !os.fail();
    }

    UniPareto1D* UniPareto1D::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<UniPareto1D>());
        current.ensureSameId(id);

        double location, scale, c;
        if (AbsScalableDistribution1D::read(in, &location, &scale))
        {
            gs::read_pod(in, &c);
            if (!in.fail())
                return new UniPareto1D(location, scale, c);
        }
        distributionReadError(in, classname());
        return 0;
    }

    bool UniPareto1D::isEqual(const AbsDistribution1D& otherBase) const
    {
        const UniPareto1D& r = static_cast<const UniPareto1D&>(otherBase);
        return AbsScalableDistribution1D::isEqual(r) && 
               c_ == r.c_;
    }

    void UniPareto1D::initialize()
    {
        if (c_ <= 0.0) throw std::invalid_argument(
            "In npstat::UniPareto1D::initialize: power parameter must be positive");
        if (c_ > 1.0)
            support_ = pow(1.0/DBL_MIN, 1.0/c_);
        else
            support_ = 1.0/DBL_MIN;
        amplitude_ = c_/(c_ + 1.0);
    }

    UniPareto1D::UniPareto1D(const double location, const double scale,
                             const double powerParam)
        : AbsScalableDistribution1D(location, scale),
          c_(powerParam)
    {
        initialize();
    }

    UniPareto1D::UniPareto1D(const double location, const double scale,
                             const std::vector<double>& params)
        : AbsScalableDistribution1D(location, scale),
          c_(params[0])
    {
        initialize();
    }

    double UniPareto1D::unscaledDensity(const double x) const
    {
        if (x < 0.0 || x > support_)
            return 0.0;
        else if (x <= 1.0)
            return amplitude_;
        else
            return amplitude_/pow(x, c_ + 1.0);
    }

    double UniPareto1D::unscaledCdf(const double x) const
    {
        if (x <= 0.0)
            return 0.0;
        else if (x >= support_)
            return 1.0;
        else if (x <= 1.0)
            return x*amplitude_;
        else
            return amplitude_ + (1.0 - amplitude_)*(1.0 - 1.0/pow(x, c_));
    }

    double UniPareto1D::unscaledQuantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::UniPareto1D::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        if (r1 <= amplitude_)
            return r1/amplitude_;
        else if (r1 == 1.0)
            return support_;
        else
            return pow(1.0 - (r1 - amplitude_)/(1.0 - amplitude_), -1.0/c_);
    }

    double UniPareto1D::unscaledExceedance(const double x) const
    {
        if (x > 1.0)
            return (1.0 - amplitude_)/pow(x, c_);
        else
            return 1.0 - unscaledCdf(x);
    }

    bool Huber1D::write(std::ostream& os) const
    {
        AbsScalableDistribution1D::write(os);
        gs::write_pod(os, tailWeight_);
        return !os.fail();
    }

    Huber1D* Huber1D::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<Huber1D>());
        current.ensureSameId(id);

        double location, scale, tailWeight;
        if (AbsScalableDistribution1D::read(in, &location, &scale))
        {
            gs::read_pod(in, &tailWeight);
            if (!in.fail())
                return new Huber1D(location, scale, tailWeight);
        }
        distributionReadError(in, classname());
        return 0;
    }

    bool Huber1D::isEqual(const AbsDistribution1D& otherBase) const
    {
        const Huber1D& r = static_cast<const Huber1D&>(otherBase);
        return AbsScalableDistribution1D::isEqual(r) && 
               tailWeight_ == r.tailWeight_;
    }

    void Huber1D::initialize()
    {
        if (!(tailWeight_ >= 0.0 && tailWeight_ < 1.0))
            throw std::invalid_argument(
                "In npstat::Huber1D::initialize: "
                "tail weight not inside [0, 1) interval");

        if (tailWeight_ == 0.0)
        {
            // Pure Gaussian
            a_ = DBL_MAX;
            normfactor_ = 1.0/sqrt(2.0*M_PI);
            support_ = inverseGaussCdf(1.0);
            cdf0_ = 0.0;
        }
        else
        {
            // Solve the equation for "a" by bisection
            const double eps = 2.0*DBL_EPSILON;
            double c = -2.0*log(tailWeight_);
            assert(c > 0.0);
            assert(weight(c) <= tailWeight_);
            double b = 0.0;
            while ((c - b)/(c + b) > eps)
            {
                const double half = (c + b)/2.0;
                if (weight(half) >= tailWeight_)
                    b = half;
                else
                    c = half;
            }
            a_ = (c + b)/2.0;
            normfactor_ = 0.5/(exp(-a_*a_/2.0)/a_ + 
                               sqrt(M_PI/2.0)*erf(a_/SQRT2));
            support_ = a_/2.0 - log(DBL_MIN)/a_;
            cdf0_ = (1.0 + erf(-a_/SQRT2))/2.0;
        }
    }

    Huber1D::Huber1D(const double location, const double scale,
                     const double tailWeight)
        : AbsScalableDistribution1D(location, scale),
          tailWeight_(tailWeight)
    {
        initialize();
    }

    Huber1D::Huber1D(const double location, const double scale,
                     const std::vector<double>& params)
        : AbsScalableDistribution1D(location, scale),
          tailWeight_(params[0])
    {
        initialize();
    }

    double Huber1D::unscaledDensity(const double x) const
    {
        const double absx = fabs(x);
        if (absx <= a_)
            return normfactor_*exp(-x*x/2.0);
        else
            return normfactor_*exp(a_*(a_/2.0 - absx));
    }

    double Huber1D::unscaledCdf(const double x) const
    {
        if (tailWeight_ == 0.0)
            return (1.0 + erf(x/SQRT2))/2.0;
        if (x < -a_)
            return normfactor_*exp((a_*(a_ + 2*x))/2.0)/a_;
        else if (x <= a_)
        {
            static const double sq1 = sqrt(M_PI/2.0);
            static const double sq2 = sqrt(2.0);
            return normfactor_*sq1*(erf(a_/sq2) + erf(x/sq2)) + tailWeight_/2;
        }
        else
            return 1.0 - normfactor_*exp((a_*(a_ - 2*x))/2.0)/a_;
    }

    double Huber1D::unscaledQuantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::Huber1D::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        if (tailWeight_ == 0.0)
            return inverseGaussCdf(r1);
        if (r1 == 0.0)
            return -support_;
        else if (r1 == 1.0)
            return support_;
        else if (r1 <= tailWeight_/2.0)
            return log(r1*a_/normfactor_)/a_ - 0.5*a_;
        else if (r1 < 1.0 - tailWeight_/2.0)
        {
            const double t = (r1 - tailWeight_/2.0)/normfactor_/SQR2PI + cdf0_;
            return inverseGaussCdf(t);
        }
        else
            return 0.5*a_ - log((1.0 - r1)*a_/normfactor_)/a_;
    }

    double Huber1D::weight(const double a) const
    {
        static const double sq1 = sqrt(M_PI/2.0);
        return 1.0/(1.0 + a*exp(a*a/2.0)*sq1*erf(a/SQRT2));
    }

    bool Tabulated1D::write(std::ostream& os) const
    {
        AbsScalableDistribution1D::write(os);
        gs::write_pod(os, deg_);
        gs::write_pod_vector(os, table_);
        return !os.fail();
    }

    Tabulated1D* Tabulated1D::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<Tabulated1D>());
        current.ensureSameId(id);

        double location, scale;
        if (AbsScalableDistribution1D::read(in, &location, &scale))
        {
            unsigned deg = 0;
            gs::read_pod(in, &deg);
            std::vector<double> table;
            gs::read_pod_vector(in, &table);
            if (!in.fail() && table.size())
                return new Tabulated1D(location, scale,
                                       &table[0], table.size(), deg);
        }
        distributionReadError(in, classname());
        return 0;
    }

    bool Tabulated1D::isEqual(const AbsDistribution1D& otherBase) const
    {
        const Tabulated1D& r = static_cast<const Tabulated1D&>(otherBase);
        return AbsScalableDistribution1D::isEqual(r) && 
               table_ == r.table_ && deg_ == r.deg_;
    }

    Tabulated1D::Tabulated1D(const double location, const double scale,
                             const std::vector<double>& params)
        : AbsScalableDistribution1D(location, scale)
    {
        const unsigned npara = params.size();
        if (npara)
            initialize(&params[0], npara, std::min(3U, npara-1U));
        else
            initialize(static_cast<double*>(0), 0U, 0U);
    }

    void Tabulated1D::normalize()
    {
        cdf_.reserve(len_);
        cdf_.push_back(0.0);
        long double sum = 0.0L;
        for (unsigned i=0; i<len_-1; ++i)
        {
            sum += intervalInteg(i);
            cdf_.push_back(static_cast<double>(sum));
        }
        const double norm = cdf_[len_ - 1];
        assert(norm > 0.0);
        for (unsigned i=0; i<len_; ++i)
        {
            table_[i] /= norm;
            cdf_[i] /= norm;
        }
    }

    double Tabulated1D::intervalInteg(const unsigned i) const
    {
        // The formula used here is exact for cubic polynomials
        static const double legendreRootOver2 = 0.5/sqrt(3.0);
        const double x0 = step_*(i + 0.5);
        const double v0 = unscaledDensity(x0 - legendreRootOver2*step_);
        const double v1 = unscaledDensity(x0 + legendreRootOver2*step_);
        return step_*(v0 + v1)/2.0;
    }

    double Tabulated1D::interpolate(const double x) const
    {
        if (x < 0.0 || x > 1.0)
            return 0.0;
        if (x == 0.0)
            return table_[0];
        if (x == 1.0)
            return table_[len_ - 1];

        unsigned idx = static_cast<unsigned>(x/step_);
        if (idx >= len_ - 1)
            idx = len_ - 2;
        const double dx = x/step_ - idx;

        switch (deg_)
        {
        case 0:
            if (dx < 0.5)
                return table_[idx];
            else
                return table_[idx + 1];

        case 1:
            return interpolate_linear(dx, table_[idx], table_[idx + 1]);

        case 2:
            if (idx == 0)
                return interpolate_quadratic(dx, table_[idx], table_[idx + 1],
                                          table_[idx + 2]);
            else if (idx == len_ - 2)
                return interpolate_quadratic(dx+1.0, table_[idx - 1],
                                          table_[idx], table_[idx + 1]);
            else
            {
                const double v0 = interpolate_quadratic(
                    dx, table_[idx], table_[idx + 1], table_[idx + 2]);
                const double v1 = interpolate_quadratic(
                    dx+1.0, table_[idx - 1], table_[idx], table_[idx + 1]);
                return (v0 + v1)/2.0;
            }

        case 3:
            if (idx == 0)
                return interpolate_cubic(dx, table_[idx], table_[idx + 1],
                                         table_[idx + 2], table_[idx + 3]);
            else if (idx == len_ - 2)
                return interpolate_cubic(dx+2.0, table_[idx-2], table_[idx-1],
                                         table_[idx], table_[idx + 1]);
            else
                return interpolate_cubic(dx+1.0, table_[idx - 1], table_[idx],
                                         table_[idx + 1], table_[idx + 2]);

        default:
            assert(0);
            return 0.0;
        }
    }

    double Tabulated1D::unscaledDensity(const double x) const
    {
        const double v = this->interpolate(x);
        if (v >= 0.0)
            return v;
        else
            return 0.0;
    }

    double Tabulated1D::unscaledCdf(double x) const
    {
        if (x <= 0.0)
            return 0.0;
        if (x >= 1.0)
            return 1.0;

        unsigned idx = static_cast<unsigned>(x/step_);
        if (idx >= len_ - 1)
            idx = len_ - 2;

        double v;
        switch (deg_)
        {
        case 0:
        {
            const double dx = x/step_ - idx;
            if (dx < 0.5)
                v = table_[idx]*dx*step_;
            else
                v = (table_[idx]*0.5 + table_[idx + 1]*(dx - 0.5))*step_;
        }
        break;

        default:
            v = interpIntegral(step_*idx, x);
        }

        return cdf_[idx] + v;
    }

    double Tabulated1D::interpIntegral(const double a, const double b) const
    {
        static const double legendreRootOver2 = 0.5/sqrt(3.0);
        const double x0 = (b + a)/2.0;
        const double step = b - a;
        const double v0 = unscaledDensity(x0 - legendreRootOver2*step);
        const double v1 = unscaledDensity(x0 + legendreRootOver2*step);
        return step*(v0 + v1)/2.0;
    }

    double Tabulated1D::unscaledQuantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::Tabulated1D::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        if (r1 == 0.0)
            return 0.0;
        if (r1 == 1.0)
            return 1.0;

        unsigned idx = std::lower_bound(cdf_.begin(), cdf_.end(), r1) -
                                        cdf_.begin() - 1U;
        double xlo = step_*idx;
        const double dcdf = r1 - cdf_[idx];
        assert(dcdf > 0.0);

        switch (deg_)
        {
        case 0:
        {
            const double c1 = table_[idx]*0.5*step_;
            if (dcdf <= c1)
            {
                assert(table_[idx] > 0.0);
                return xlo + dcdf/table_[idx];
            }
            else
            {
                assert(table_[idx+1] > 0.0);
                return xlo + 0.5*step_ + (dcdf - c1)/table_[idx+1];
            }
        }

        case 1:
        {
            const double a = (table_[idx+1] - table_[idx])/step_/2.0;
            if (a == 0.0)
            {
                assert(table_[idx] > 0.0);
                return xlo + dcdf/table_[idx];
            }
            else
            {
                double x1, x2;
                const unsigned nroots = solveQuadratic(
                    table_[idx]/a, -dcdf/a, &x1, &x2);
                if (nroots != 2U) throw std::runtime_error(
                    "In npstat::Tabulated1D::unscaledQuantile: "
                    "unexpected number of solutions");
                if (fabs(x1 - 0.5*step_) < fabs(x2 - 0.5*step_))
                    return xlo + x1;
                else
                    return xlo + x2;
            }
        }

        default:
        {
            double xhi = xlo + step_;
            const double eps = 2.0*DBL_EPSILON;
            while ((xhi - xlo)/(xhi + xlo) > eps)
            {
                const double med = (xhi + xlo)/2.0;
                if (unscaledCdf(med) >= r1)
                    xhi = med;
                else
                    xlo = med;
            }
            return (xhi + xlo)/2.0;
        }
        }
    }

    bool BinnedDensity1D::isEqual(const AbsDistribution1D& otherBase) const
    {
        const double eps = 1.0e-12;

        const BinnedDensity1D& r = 
            static_cast<const BinnedDensity1D&>(otherBase);
        if (!AbsScalableDistribution1D::isEqual(r))
            return false;
        if (!(deg_ == r.deg_))
            return false;
        const unsigned long n = table_.size();
        if (!(n == r.table_.size()))
            return false;
        for (unsigned long i=0; i<n; ++i)
            if (fabs(table_[i] - r.table_[i])/
                ((fabs(table_[i]) + fabs(r.table_[i]))/2.0 + 1.0) > eps)
                return false;
        return true;
    }

    BinnedDensity1D::BinnedDensity1D(const double location, const double scale,
                                     const std::vector<double>& params)
        : AbsScalableDistribution1D(location, scale)
    {
        const unsigned npara = params.size();
        if (npara)
            initialize(&params[0], npara, std::min(1U, npara-1U));
        else
            initialize(static_cast<double*>(0), 0U, 0U);
    }

    void BinnedDensity1D::normalize()
    {
        cdf_.reserve(len_);
        long double sum = 0.0L;

        switch (deg_)
        {
        case 0U:
            for (unsigned i=0; i<len_; ++i)
            {
                sum += table_[i];
                cdf_.push_back(static_cast<double>(sum));
            }
            break;

        case 1U:
        {
            double oldval = 0.0;
            double* data = &table_[0];
            for (unsigned i=0; i<len_; ++i)
            {
                sum += (data[i] + oldval)*0.5;
                oldval = data[i];
                cdf_.push_back(static_cast<double>(sum));
            }
            sum += oldval*0.5;
        }
        break;

        default:
            assert(0);
        }
        
        const double norm = static_cast<double>(sum);
        assert(norm > 0.0);
        const double integ = norm*step_;
        for (unsigned i=0; i<len_; ++i)
        {
            table_[i] /= integ;
            cdf_[i] /= norm;
        }
    }

    double BinnedDensity1D::unscaledDensity(const double x) const
    {
        double v = interpolate(x);
        if (v < 0.0)
            v = 0.0;
        return v;
    }

    double BinnedDensity1D::interpolate(const double x) const
    {
        if (x < 0.0 || x > 1.0)
            return 0.0;

        switch (deg_)
        {
        case 0:
        {
            unsigned idx = static_cast<unsigned>(x/step_);
            if (idx > len_ - 1)
                idx = len_ - 1;
            return table_[idx];
        }

        case 1:
        {
            const double xs = x - step_/2.0;
            if (xs <= 0.0)
                return table_[0];
            const unsigned idx = static_cast<unsigned>(xs/step_);
            if (idx > len_ - 2)
                return table_[len_ - 1];
            const double dx = xs/step_ - idx;
            return interpolate_linear(dx, table_[idx], table_[idx + 1]);
        }

        default:
            assert(0);
            return 0.0;
        }
    }

    double BinnedDensity1D::unscaledCdf(double x) const
    {
        if (x <= 0.0)
            return 0.0;
        if (x >= 1.0)
            return 1.0;

        double v = 0.0;
        switch (deg_)
        {
        case 0:
        {            
            unsigned idx = static_cast<unsigned>(x/step_);
            if (idx > len_ - 1)
                idx = len_ - 1;
            v = (idx ? cdf_[idx - 1] : 0.0) + table_[idx]*(x - idx*step_);
        }
        break;

        case 1:
        {
            const double xs = x - step_/2.0;
            if (xs <= 0.0)
                v = table_[0]*x;
            else 
            {
                const unsigned idx = static_cast<unsigned>(xs/step_);
                if (idx > len_ - 2)
                    v = 1.0 - table_[len_ - 1]*(1.0 - x);
                else
                {
                    const double dx = xs - idx*step_;
                    const double slope = (table_[idx+1] - table_[idx])/step_;
                    v = cdf_[idx] + table_[idx]*dx + slope*dx*dx/2.0;
                }
            }
        }
        break;

        default:
            assert(0);
            break;
        }
        if (v < 0.0)
            v = 0.0;
        else if (v > 1.0)
            v = 1.0;
        return v;
    }

    double BinnedDensity1D::unscaledQuantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::BinnedDensity1D::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        if (r1 == 0.0 && deg_)
            return 0.0;
        if (r1 == 1.0 && deg_)
            return 1.0;

        double v = 0.0;
        switch (deg_)
        {
        case 0:
        {
            if (r1 == 0)
                v = firstNonZeroBin_;
            else if (r1 == 1.0)
                v = lastNonZeroBin_ + 1U;
            else if (r1 <= cdf_[0])
                v = r1/cdf_[0];
            else
            {
                double rem;
                unsigned bin = quantileBinFromCdf(&cdf_[0],len_,r1,&rem) + 1U;
                assert(bin < len_);
                v = bin + rem;
            }
        }
        break;

        case 1:
        {
            if (r1 <= cdf_[0])
                v = 0.5*r1/cdf_[0];
            else if (r1 >= cdf_[len_ - 1])
                v = (len_-0.5+0.5*(r1-cdf_[len_-1])/(1.0-cdf_[len_-1]));
            else
            {
                const unsigned idx = std::lower_bound(cdf_.begin(),cdf_.end(),r1) -
                                     cdf_.begin() - 1U;
                assert(idx < len_ - 1);
                const double k = (table_[idx+1] - table_[idx])/step_;
                const double y = r1 - cdf_[idx];
                double x;
                if (fabs(k) < 1.e-10*table_[idx])
                    x = y/table_[idx];
                else
                {
                    const double b = 2.0*table_[idx]/k;
                    const double c = -2.0*y/k;
                    double x1, x2;
                    if (solveQuadratic(b, c, &x1, &x2))
                    {
                        if (fabs(x1 - step_*0.5) < fabs(x2 - step_*0.5))
                            x = x1;
                        else
                            x = x2;
                    }
                    else
                    {
                        // This can happen due to various round-off problems.
                        // Assume that the quadratic equation determinant
                        // should have been 0 instead of negative.
                        x = -b/2.0;
                    }
                }
                if (x < 0.0)
                    x = 0.0;
                else if (x > step_)
                    x = step_;
                v = x/step_ + idx + 0.5;
            }
        }
        break;

        default:
            assert(0);
            return 0.0;
        }
        v *= step_;
        if (v < 0.0)
            v = 0.0;
        else if (v > 1.0)
            v = 1.0;
        return v;
    }

    bool BinnedDensity1D::write(std::ostream& os) const
    {
        AbsScalableDistribution1D::write(os);
        gs::write_pod(os, deg_);
        gs::write_pod_vector(os, table_);
        return !os.fail();
    }

    BinnedDensity1D* BinnedDensity1D::read(const gs::ClassId& id,
                                           std::istream& in)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<BinnedDensity1D>());
        current.ensureSameId(id);

        double location, scale;
        if (AbsScalableDistribution1D::read(in, &location, &scale))
        {
            unsigned deg = 0;
            gs::read_pod(in, &deg);
            std::vector<double> table;
            gs::read_pod_vector(in, &table);
            if (!in.fail() && table.size())
                return new BinnedDensity1D(location, scale,
                                           &table[0], table.size(), deg);
        }
        distributionReadError(in, classname());
        return 0;
    }

    bool StudentsT1D::write(std::ostream& os) const
    {
        AbsScalableDistribution1D::write(os);
        gs::write_pod(os, nDoF_);
        return !os.fail();
    }

    StudentsT1D* StudentsT1D::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<StudentsT1D>());
        current.ensureSameId(id);

        double location, scale;
        if (AbsScalableDistribution1D::read(in, &location, &scale))
        {
            double nDoF;
            gs::read_pod(in, &nDoF);
            if (!in.fail())
                return new StudentsT1D(location, scale, nDoF);
        }
        distributionReadError(in, classname());
        return 0;
    }

    bool StudentsT1D::isEqual(const AbsDistribution1D& otherBase) const
    {
        const StudentsT1D& r = static_cast<const StudentsT1D&>(otherBase);
        return AbsScalableDistribution1D::isEqual(r) && 
            nDoF_ == r.nDoF_;
    }

    StudentsT1D::StudentsT1D(const double location, const double scale,
                             const double degreesOfFreedom)
        : AbsScalableDistribution1D(location, scale),
          nDoF_(degreesOfFreedom)
    {
        initialize();
    }
    
    StudentsT1D::StudentsT1D(const double location, const double scale,
                             const std::vector<double>& params)
        : AbsScalableDistribution1D(location, scale),
          nDoF_(params[0])
    {
        initialize();
    }

    void StudentsT1D::initialize()
    {
        if (nDoF_ <= 0.0) throw std::invalid_argument(
            "In npstat::StudentsT1D::initialize: invalid number "
            "of degrees of freedom");
        power_ = (nDoF_ + 1.0)/2.0;
        bignum_ = 0.0;
        normfactor_ = Gamma(power_)/Gamma(nDoF_/2.0)/sqrt(nDoF_)/SQRPI;
    }

    double StudentsT1D::unscaledDensity(const double x) const
    {
        return normfactor_*pow(1.0 + x*x/nDoF_, -power_);
    }

    double StudentsT1D::unscaledCdf(const double t) const
    {
        const double s = sqrt(t*t + nDoF_);
        return incompleteBeta(nDoF_/2.0, nDoF_/2.0, (t + s)/(2.0*s));
    }

    double StudentsT1D::unscaledExceedance(const double x) const
    {
        return 1.0 - unscaledCdf(x);
    }

    double StudentsT1D::unscaledQuantile(const double r1) const
    {
        if (r1 == 0.5)
            return 0.0;
        const double c = inverseIncompleteBeta(nDoF_/2.0, nDoF_/2.0, r1);
        const double tmp = 2.0*c - 1.0;
        const double a = tmp*tmp;
        const double denom = 1.0 - a;
        if (denom > 0.0)
        {
            const double sqroot = sqrt(nDoF_*a/denom);
            return r1 > 0.5 ? sqroot : -sqroot;
        }
        else
        {
            if (bignum_ == 0.0)
                (const_cast<StudentsT1D*>(this))->bignum_ = effectiveSupport();
            return r1 > 0.5 ? bignum_ : -bignum_;
        }
    }

    double StudentsT1D::effectiveSupport() const
    {
        // Figure out at which (positive) values of the argument
        // the density becomes effectively indistinguishable from 0
        const double biglog = (log(normfactor_) - log(DBL_MIN) + 
                               power_*log(nDoF_))/(2.0*power_);
        if (biglog >= log(DBL_MAX))
            return DBL_MAX;
        else
            return exp(biglog);
    }
}
