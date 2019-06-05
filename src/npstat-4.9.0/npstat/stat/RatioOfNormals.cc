#include <cmath>
#include <cfloat>
#include <stdexcept>

#include "geners/binaryIO.hh"

#include "npstat/nm/SpecialFunctions.hh"

#include "npstat/stat/RatioOfNormals.hh"
#include "npstat/stat/Distributions1D.hh"
#include "npstat/stat/distributionReadError.hh"

#define SQR2PI 2.5066282746310005
#define SQRT2  1.41421356237309505

namespace npstat {
    RatioOfNormals::RatioOfNormals(const double mu1, const double s1,
                                   const double mu2, const double s2,
                                   const double rho)
        : mu1_(mu1), s1_(s1), mu2_(mu2), s2_(s2), rho_(rho)
    {
        if (s1 <= 0.0)
            throw std::invalid_argument("In npstat::RatioOfNormals constructor:"
                                        " s1 argument is out of range");
        if (s2 <= 0.0)
            throw std::invalid_argument("In npstat::RatioOfNormals constructor:"
                                        " s2 argument is out of range");
        if (1.0 - rho_*rho_ <= 0.0)
            throw std::invalid_argument("In npstat::RatioOfNormals constructor:"
                                        " rho argument is out of range");
        support_ = sqrt(DBL_MAX);
    }

    double RatioOfNormals::density(const double w) const
    {
        if (fabs(w) > support_)
            return 0.0;

        const double oneminusrhosq = 1.0 - rho_*rho_;
        const double sqrtoneminusrhosq = sqrt(oneminusrhosq);

        double awsq = w*w/s1_/s1_ - 2.0*rho_*w/s1_/s2_ + 1.0/s2_/s2_;
        if (awsq <= 0.0)
            awsq = DBL_MIN;
        const double aw = sqrt(awsq);

        const double bw = mu1_*w/s1_/s1_ - 
            rho_*(mu1_ + mu2_*w)/s1_/s2_ + mu2_/s2_/s2_;

        const double c = mu1_*mu1_/s1_/s1_ - 2.0*rho_*mu1_*mu2_/s1_/s2_ +
                         mu2_*mu2_/s2_/s2_;

        const double dw = exp((bw*bw - c*aw*aw)/2.0/oneminusrhosq/aw/aw);

        const double arg = bw/sqrtoneminusrhosq/aw;
        const double phiterm = 1.0 - 2.0*erfc(arg/SQRT2)/2.0;

        return bw*dw/SQR2PI/s1_/s2_/aw/aw/aw*phiterm + 
            sqrtoneminusrhosq/M_PI/s1_/s2_/awsq*exp(-c/2.0/oneminusrhosq);
    }

    double RatioOfNormals::cdf(const double w) const
    {
        if (w >= support_)
            return 1.0;
        if (w <= -support_)
            return 0.0;

        double awsq = w*w/s1_/s1_ - 2.0*rho_*w/s1_/s2_ + 1.0/s2_/s2_;
        if (awsq <= 0.0)
            awsq = DBL_MIN;
        const double aw = sqrt(awsq);
        const double s1s2aw = s1_*s2_*aw;
        const double gamma = (s2_*w - rho_*s1_)/s1s2aw;
        const double mu2rat = mu2_/s2_;
        const double x = (mu1_ - mu2_*w)/s1s2aw;

        return bivariateNormalIntegral(gamma, x, -mu2rat) +
               bivariateNormalIntegral(gamma, -x, mu2rat);
    }

    double RatioOfNormals::exceedance(const double x) const
    {
        return 1.0 - cdf(x);
    }

    double RatioOfNormals::quantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::RatioOfNormals::quantile: "
            "cdf argument outside of [0, 1] interval");
        if (r1 == 0.0)
            return -support_;
        else if (r1 == 1.0)
            return support_;
        else
        {
            double qmin = -1.0;
            double qmax = 1.0;
            double fmin = cdf(qmin);
            double fmax = cdf(qmax);
            assert(fmin <= fmax);
            while (!(fmin <= r1 && r1 <= fmax))
            {
                if (r1 < fmin)
                {
                    const double newqmin = qmin - 2.0*(qmax - qmin);
                    qmax = qmin;
                    fmax = fmin;
                    qmin = newqmin;
                    fmin = cdf(qmin);
                }
                else
                {
                    const double newqmax = qmax + 2.0*(qmax - qmin);
                    qmin = qmax;
                    fmin = fmax;
                    qmax = newqmax;
                    fmax = cdf(qmax);
                }
                assert(fmin <= fmax);
            }
            for (unsigned i=0; i<1000; ++i)
            {
                const double x = (qmin + qmax)/2.0;
                if (fabs(qmax - qmin)/std::max(fabs(qmin), fabs(qmax)) <
                    2.0*DBL_EPSILON)
                    return x;
                const double fval = cdf(x);
                if (fval == r1)
                    return x;
                else if (fval > r1)
                    qmax = x;
                else
                    qmin = x;
                if (qmax == qmin)
                    return qmin;
            }
            return (qmin + qmax)/2.0;
        }
    }

    unsigned RatioOfNormals::random(AbsRandomGenerator& g,
                                    double* generatedRandom) const
    {
        assert(generatedRandom);
        Gauss1D gauss(0.0, 1.0);
        unsigned count = 0;
        double z1, z2;
        count += gauss.random(g, &z1);
        count += gauss.random(g, &z2);
        const double x = s1_*z1 + mu1_;
        const double y = s2_*(rho_*z1 + sqrt(1.0 - rho_*rho_)*z2) + mu2_;
        if (y)
        {
            double r = x/y;
            if (r > support_)
                r = support_;
            else if (r < -support_)
                r = -support_;
            *generatedRandom = r;
        }
        else
        {
            if (g() > 0.5)
                *generatedRandom = support_;
            else
                *generatedRandom = -support_;
            ++count;
        }
        return count;
    }

    bool RatioOfNormals::write(std::ostream& os) const
    {
        gs::write_pod(os, mu1_);
        gs::write_pod(os, s1_);
        gs::write_pod(os, mu2_);
        gs::write_pod(os, s2_);
        gs::write_pod(os, rho_);
        return !os.fail();
    }

    RatioOfNormals* RatioOfNormals::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<RatioOfNormals>());
        current.ensureSameId(id);

        double mu1, s1, mu2, s2, rho;
        gs::read_pod(in, &mu1);
        gs::read_pod(in, &s1);
        gs::read_pod(in, &mu2);
        gs::read_pod(in, &s2);
        gs::read_pod(in, &rho);
        if (in.fail())
            distributionReadError(in, classname());
        return new RatioOfNormals(mu1, s1, mu2, s2, rho);
    }

    bool RatioOfNormals::isEqual(const AbsDistribution1D& other) const
    {
        const RatioOfNormals& r = static_cast<const RatioOfNormals&>(other);
        return mu1_ == r.mu1_ &&
            s1_ == r.s1_ &&
            mu2_ == r.mu2_ &&
            s2_ == r.s2_ &&
            rho_ == r.rho_;
    }
}
