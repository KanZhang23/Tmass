#include <cmath>
#include <cfloat>

#include "geners/GenericIO.hh"

#include "npstat/nm/interpolate.hh"
#include "npstat/nm/SpecialFunctions.hh"

#include "npstat/rng/convertToSphericalRandom.hh"

#include "npstat/stat/DistributionsND.hh"
#include "npstat/stat/StatUtils.hh"
#include "npstat/stat/distributionReadError.hh"

namespace npstat {
    bool ProductDistributionND::isEqual(const AbsDistributionND& other) const
    {
        const ProductDistributionND& r = 
            static_cast<const ProductDistributionND&>(other);
        if (dim_ != r.dim_)
            return false;
        for (unsigned i=0; i<dim_; ++i)
            if (*marginals_[i] != *r.marginals_[i])
                return false;
        return true;
    }

    bool ProductDistributionND::write(std::ostream& os) const
    {
        return gs::write_item(os, marginals_);
    }

    ProductDistributionND* ProductDistributionND::read(const gs::ClassId& id,
                                                       std::istream& in)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<ProductDistributionND>());
        current.ensureSameId(id);

        std::vector<AbsDistribution1D*> marg;
        gs::restore_item(in, &marg);
        ProductDistributionND* p = new ProductDistributionND(marg.size());
        p->marginals_ = marg;
        return p;
    }

    bool ProductDistributionND::isScalable() const
    {
        for (unsigned i=0; i<dim_; ++i)
        {
            AbsScalableDistribution1D* d = 
                dynamic_cast<AbsScalableDistribution1D*>(marginals_[i]);
            if (!d)
                return false;
        }
        return true;
    }

    ProductDistributionND::ProductDistributionND(
        const AbsDistribution1D** marginals, const unsigned nMarginals)
        : AbsDistributionND(nMarginals)
    {
        if (!nMarginals) throw std::invalid_argument(
            "In npstat::ProductDistributionND constructor: "
            "no marginals provided");
        assert(marginals);
        marginals_.reserve(nMarginals);
        for (unsigned i=0; i<nMarginals; ++i)
        {
            assert(marginals[i]);
            marginals_.push_back(marginals[i]->clone());
        }
    }

    ProductDistributionND::ProductDistributionND(
        const ProductDistributionND& r)
        : AbsDistributionND(r.dim_)
    {
        marginals_.reserve(r.dim_);
        for (unsigned i=0; i<r.dim_; ++i)
            marginals_.push_back(r.marginals_[i]->clone());
    }

    ProductDistributionND& ProductDistributionND::operator=(
        const ProductDistributionND& r)
    {
        if (this != &r)
        {
            AbsDistributionND::operator=(r);
            cleanup();
            marginals_.resize(r.dim_);
            for (unsigned i=0; i<r.dim_; ++i)
                marginals_[i] = r.marginals_[i]->clone();
        }
        return *this;
    }

    void ProductDistributionND::cleanup()
    {
        for (int i=marginals_.size()-1; i>=0; --i)
        {
            delete marginals_[i];
            marginals_[i] = 0;
        }
    }

    ProductDistributionND::~ProductDistributionND()
    {
        cleanup();
    }

    double ProductDistributionND::density(
        const double* x, const unsigned dim) const
    {
        if (dim != dim_) throw std::invalid_argument(
            "In npstat::ProductDistributionND::density: "
            "incompatible input point dimensionality");
        assert(x);
        double prod = 1.0;
        for (unsigned i=0; i<dim; ++i)
            prod *= marginals_[i]->density(x[i]);
        return prod;
    }

    void ProductDistributionND::unitMap(
        const double* rnd, const unsigned bufLen, double* x) const
    {
        if (bufLen)
        {
            assert(rnd);
            assert(x);
            for (unsigned i=0; i<dim_ && i<bufLen; ++i)
                x[i] = marginals_[i]->quantile(rnd[i]);
        }
    }

    double UniformND::unscaledDensity(const double* x) const
    {
        for (unsigned i=0; i<dim_; ++i)
            if (x[i] < 0.0 || x[i] > 1.0)
                return 0.0;
        return 1.0;
    }

    void UniformND::unscaledUnitMap(
        const double* rnd, const unsigned bufLen, double* x) const
    {
        for (unsigned i=0; i<bufLen; ++i)
            x[i] = rnd[i];
    }

    bool UniformND::write(std::ostream& of) const
    {
        return AbsScalableDistributionND::write(of);
    }

    UniformND* UniformND::read(const gs::ClassId& id, std::istream& is)
    {
        static const gs::ClassId current(gs::ClassId::makeId<UniformND>());
        current.ensureSameId(id);

        unsigned dim = 0;
        std::vector<double> locations, scales;
        if (AbsScalableDistributionND::read(is, &dim, &locations, &scales))
            if (dim && locations.size() == dim && scales.size() == dim)
                return new UniformND(&locations[0], &scales[0], dim);
        distributionReadError(is, classname());
        return 0;
    }

    bool ScalableSymmetricBetaND::isEqual(const AbsDistributionND& ri) const
    {
        const ScalableSymmetricBetaND& o = 
            static_cast<const ScalableSymmetricBetaND&>(ri);
        return AbsScalableDistributionND::isEqual(o) &&
               power_ == o.power_;
    }

    ScalableSymmetricBetaND::ScalableSymmetricBetaND(
        const double* location, const double* scale,
        const unsigned dim, const double power)
        : AbsScalableDistributionND(location, scale, dim),
          power_(power)
    {
        if (power < 0.0) throw std::invalid_argument(
            "In npstat::ScalableSymmetricBetaND constructor: "
            "invalid power parameter");
        norm_ = Gamma(1 + power + dim/2.0)/pow(M_PI,dim/2.0)/Gamma(1 + power);
    }

    double ScalableSymmetricBetaND::unscaledDensity(const double* x) const
    {
        double rsquare = 0.0;
        for (unsigned i=0; i<dim_; ++i)
            rsquare += x[i]*x[i];
        if (rsquare >= 1.0)
            return 0.0;
        else
            return norm_*pow(1.0-rsquare, power_);
    }

    double ScalableSymmetricBetaND::radialQuantile(const double cdf) const
    {
        const double asq = inverseIncompleteBeta(dim_/2.0, power_+1.0, cdf);
        return sqrt(asq);
    }

    void ScalableSymmetricBetaND::unscaledUnitMap(
        const double* rnd, const unsigned bufLen, double* x) const
    {
        if (bufLen != dim_) throw std::invalid_argument(
            "In npstat::ScalableSymmetricBetaND::unscaledUnitMap: "
            "incompatible dimensionality of the random vector");
        const double cdf = convertToSphericalRandom(rnd, dim_, x);
        const double r = radialQuantile(cdf);
        for (unsigned i=0; i<dim_; ++i)
            x[i] *= r;
    }

    bool ScalableHuberND::isEqual(const AbsDistributionND& ri) const
    {
        const ScalableHuberND& o = static_cast<const ScalableHuberND&>(ri);
        return AbsScalableDistributionND::isEqual(o) &&
               tWeight_ == o.tWeight_;
    }

    ScalableHuberND::ScalableHuberND(
        const double* location, const double* scales,
        const unsigned dim, const double alpha)
        : AbsScalableDistributionND(location, scales, dim),
          tWeight_(alpha)
    {
        if (!(tWeight_ >= 0.0 && tWeight_ < 1.0)) throw std::invalid_argument(
            "In npstat::ScalableHuberND constructor: "
            "tail weight not inside [0, 1) interval");

        const1_ = pow(2.0*M_PI, dim_/2.0);
        const2_ = 2.0*pow(M_PI, dim_/2.0)*Gamma(dim_)/Gamma(dim_/2.0);

        if (tWeight_ == 0.0)
        {
            a_ = DBL_MAX;
            normfactor_ = 1.0/const1_;
        }
        else
        {
            const double relative_eps = 5.0e-13;
            unsigned i = 0;
            double a, bf, n1, n2, amin, amax = 0.0;

            const double scale = sqrt(static_cast<double>(dim));
            do {
                amin = amax;
                amax = scale*pow(2.0, i++);
                n1 = norm1(amax);
                n2 = norm2(amax);
                bf = n2/(n1+n2);
            } while (bf > alpha);
            do {
                a = (amax + amin)/2.0;
                n1 = norm1(a);
                n2 = norm2(a);
                bf = n2/(n1+n2);
                if (bf > alpha)
                    amin = a;
                else
                    amax = a;
            } while ((amax - amin)/a > relative_eps);
            a_ = a;
            normfactor_ = 1.0/(n1+n2);
        }
    }

    double ScalableHuberND::norm1(const double a) const
    {
        return const1_*incompleteGamma(dim_/2.0, a*a/2.0);
    }

    double ScalableHuberND::norm2(const double a) const
    {
        const double d = dim_;
        return const2_*pow(a, -d)*exp(a*a/2.0)*incompleteGammaC(d, a*a);
    }

    double ScalableHuberND::unscaledDensity(const double* x) const
    {
        double rsquare = 0.0;
        for (unsigned i=0; i<dim_; ++i)
            rsquare += x[i]*x[i];
        const double r = sqrt(rsquare);
        if (r <= a_)
            return normfactor_*exp(-rsquare/2.0);
        else
            return normfactor_*exp(a_*(a_/2.0-r));
    }

    double ScalableHuberND::radialQuantile(const double cdf) const
    {
        if (cdf <= (1.0 - tWeight_))
        {
            const double halfdim = dim_/2.0;
            const double arg = cdf/normfactor_/pow(2.0*M_PI, halfdim);
            const double x = inverseIncompleteGamma(halfdim, arg);
            return sqrt(2.0*x);
        }
        else
        {
            const double target = (1.0 - cdf)/normfactor_*pow(a_, dim_)*
                                  exp(-a_*a_/2.0)/const2_;
            if (target <= 0.0)
                return 800.0/a_ + a_/2.0;
            else
                return inverseIncompleteGammaC(dim_, target)/a_;
        }
    }

    void ScalableHuberND::unscaledUnitMap(
        const double* rnd, const unsigned bufLen, double* x) const
    {
        if (bufLen != dim_) throw std::invalid_argument(
            "In npstat::ScalableHuberND::unscaledUnitMap: "
            "incompatible dimensionality of the random vector");
        const double cdf = convertToSphericalRandom(rnd, dim_, x);
        const double r = radialQuantile(cdf);
        for (unsigned i=0; i<dim_; ++i)
            x[i] *= r;
    }

    ProductSymmetricBetaND::ProductSymmetricBetaND(
        const double* location, const double* scale,
        const unsigned dim, const double power)
        : HomogeneousProductDistroND<SymmetricBeta1D>(dim)
    {
        assert(location);
        assert(scale);
        for (unsigned i=0; i<dim; ++i)
            marginals_.push_back(SymmetricBeta1D(location[i],scale[i],power));
    }

    bool RadialProfileND::isEqual(const AbsDistributionND& ri) const
    {
        const RadialProfileND& o = static_cast<const RadialProfileND&>(ri);

        const unsigned sz1 = sizeof(xg_)/sizeof(xg_[0]);
        for (unsigned i=0; i<sz1; ++i)
            if (xg_[i] != o.xg_[i])
                return false;

        const unsigned sz2 = sizeof(wg_)/sizeof(wg_[0]);
        for (unsigned i=0; i<sz2; ++i)
            if (wg_[i] != o.wg_[i])
                return false;
        
        return AbsScalableDistributionND::isEqual(o) &&
            profile_ == o.profile_ &&
            cdf_ == o.cdf_ &&
            ndSphereVol_ == o.ndSphereVol_ &&
            step_ == o.step_ &&
            len_ == o.len_ &&
            deg_ == o.deg_;
    }

    RadialProfileND* RadialProfileND::read(const gs::ClassId& id,
                                           std::istream& is)
    {
        static const gs::ClassId curr(gs::ClassId::makeId<RadialProfileND>());
        curr.ensureSameId(id);

        unsigned dim = 0;
        std::vector<double> locations, scales;
        if (AbsScalableDistributionND::read(is, &dim, &locations, &scales))
            if (dim && locations.size() == dim && scales.size() == dim)
            {
                CPP11_auto_ptr<RadialProfileND> rp(
                    new RadialProfileND(&locations[0], &scales[0], dim));
                gs::read_pod_vector(is, &rp->profile_);
                gs::read_pod_vector(is, &rp->cdf_);
                gs::read_pod(is, &rp->ndSphereVol_);
                gs::read_pod(is, &rp->step_);
                gs::read_pod(is, &rp->len_);
                gs::read_pod(is, &rp->deg_);
                gs::read_pod_array(is, &rp->xg_[0], sizeof(rp->xg_)/sizeof(rp->xg_[0]));
                gs::read_pod_array(is, &rp->wg_[0], sizeof(rp->wg_)/sizeof(rp->wg_[0]));
                if (!is.fail())
                    return rp.release();
            }
        distributionReadError(is, classname());
        return 0;
    }

    bool RadialProfileND::write(std::ostream& of) const
    {
        const bool ok = AbsScalableDistributionND::write(of);
        if (ok)
        {
            gs::write_pod_vector(of, profile_);
            gs::write_pod_vector(of, cdf_);
            gs::write_pod(of, ndSphereVol_);
            gs::write_pod(of, step_);
            gs::write_pod(of, len_);
            gs::write_pod(of, deg_);
            gs::write_pod_array(of, &xg_[0], sizeof(xg_)/sizeof(xg_[0]));
            gs::write_pod_array(of, &wg_[0], sizeof(wg_)/sizeof(wg_[0]));
        }
        return ok && !of.fail();
    }

    // The following function calculates the interval integral
    // exactly (up to rounding errors) in arbitrary dimension
    double RadialProfileND::intervalInteg(const unsigned i,
                                          const double stepFraction) const
    {
        double coeffs[4];
        unsigned ncoeffs = 0;

        switch (deg_)
        {
        case 0:
        {
            const double r0 = step_*i;
            const double rmax = r0 + step_*stepFraction;
            const double rmed = r0 + step_/2.0;
            const double powr0 = pow(r0, dim_);
            const double powrmax = pow(rmax, dim_);

            if (rmax <= rmed)
                return ndSphereVol_*profile_[i]*(powrmax - powr0);
            else
            {
                const double powrmed = pow(rmed, dim_);
                return ndSphereVol_*(profile_[i]*(powrmed - powr0) +
                                     profile_[i+1]*(powrmax - powrmed));
            }
        }
        case 1:
            ncoeffs = interpolation_coefficients(
                coeffs, sizeof(coeffs)/sizeof(coeffs[0]),
                profile_[i], profile_[i+1]);
            coeffs[0] -= coeffs[1]*i;
            break;

        case 2:
            if (i == 0)
            {
                ncoeffs = interpolation_coefficients(
                    coeffs, sizeof(coeffs)/sizeof(coeffs[0]),
                    profile_[i], profile_[i+1], profile_[i+2]);
                coeffs[1] -= 2*coeffs[2]*i;
                coeffs[0] -= (coeffs[2]*i + coeffs[1])*i;
            }
            else if (i == len_-2)
            {
                ncoeffs = interpolation_coefficients(
                    coeffs, sizeof(coeffs)/sizeof(coeffs[0]),
                    profile_[i-1], profile_[i], profile_[i+1]);
                const double rbase = i - 1.0;
                coeffs[1] -= 2*coeffs[2]*rbase;
                coeffs[0] -= (coeffs[2]*rbase + coeffs[1])*rbase;
            }
            else
            {
                double cset1[3], cset2[3];
                ncoeffs = interpolation_coefficients(
                    cset1, 3,
                    profile_[i], profile_[i+1], profile_[i+2]);
                cset1[1] -= 2*cset1[2]*i;
                cset1[0] -= (cset1[2]*i + cset1[1])*i;

                ncoeffs = interpolation_coefficients(
                    cset2, 3,
                    profile_[i-1], profile_[i], profile_[i+1]);
                const double rbase = i - 1.0;
                cset2[1] -= 2*cset2[2]*rbase;
                cset2[0] -= (cset2[2]*rbase + cset2[1])*rbase;

                for (unsigned j=0; j<3; ++j)
                    coeffs[j] = (cset1[j] + cset2[j])/2.0;
            }
            break;

        case 3:
        {
            double rb;
            if (i == 0)
            {
                ncoeffs = interpolation_coefficients(
                    coeffs, sizeof(coeffs)/sizeof(coeffs[0]),
                    profile_[i], profile_[i+1], profile_[i+2], profile_[i+3]);
                rb = i;
            }
            else if (i == len_-2)
            {
                ncoeffs = interpolation_coefficients(
                    coeffs, sizeof(coeffs)/sizeof(coeffs[0]),
                    profile_[i-2], profile_[i-1], profile_[i], profile_[i+1]);
                rb = i - 2.0;
            }
            else
            {
                ncoeffs = interpolation_coefficients(
                    coeffs, sizeof(coeffs)/sizeof(coeffs[0]),
                    profile_[i-1], profile_[i], profile_[i+1], profile_[i+2]);
                rb = i - 1.0;
            }
            coeffs[2] -= 3*coeffs[3]*rb;
            coeffs[1] -= rb*(2*coeffs[2] + 3*coeffs[3]*rb);
            coeffs[0] -= rb*(coeffs[1] + rb*(coeffs[2] + coeffs[3]*rb));
        }
        break;

        default:
            assert(0);
            return 0.0;
        }

        assert(ncoeffs == deg_ + 1U);
        long double sum = 0.0L;
        for (unsigned j=0; j<ncoeffs; ++j)
            sum += coeffs[j]/(j+dim_)*(pow(i+stepFraction, j+dim_) -
                                       pow(i, j+dim_));
        return static_cast<double>(sum)*dim_*ndSphereVol_*pow(step_, dim_);
    }

    void RadialProfileND::calculateQuadratureCoeffs()
    {
        xg_[0] = sqrt((3.0 - 2.0*sqrt(6.0/5.0))/7.0);
        xg_[1] = -xg_[0];
        xg_[2] = sqrt((3.0 + 2.0*sqrt(6.0/5.0))/7.0);
        xg_[3] = -xg_[2];

        wg_[0] = (18.0 + sqrt(30.0))/36.0;
        wg_[1] = wg_[0];
        wg_[2] = (18.0 - sqrt(30.0))/36.0;
        wg_[3] = wg_[2];
    }

    double RadialProfileND::intervalIntegLowD(const unsigned i,
                                              const double stepFraction) const
    {
        const double halfstep = step_*stepFraction/2.0;
        const double midpoint = i*step_ + halfstep;
        long double sum = 0.0L;
        for (unsigned j=0; j<sizeof(xg_)/sizeof(xg_[0]); ++j)
        {
            const double r = midpoint + xg_[j]*halfstep;
            sum += wg_[j]*pow(r,dim_-1)*radialPofile(r);
        }
        return static_cast<double>(sum)*dim_*halfstep*ndSphereVol_;
    }

    void RadialProfileND::normalize()
    {
        step_ = 1.0/(len_ - 1);
        cdf_.reserve(len_);
        cdf_.push_back(0.0);
        long double sum = 0.0L;
        const bool useGaussRule = deg_ && 
            (dim_ + deg_ <= 2*sizeof(xg_)/sizeof(xg_[0]));
        for (unsigned i=0; i<len_-1; ++i)
        {
            if (useGaussRule)
                sum += intervalIntegLowD(i);
            else
                sum += intervalInteg(i);
            cdf_.push_back(static_cast<double>(sum));
        }
        const double norm = cdf_[len_ - 1];
        assert(norm > 0.0);
        for (unsigned i=0; i<len_; ++i)
        {
            profile_[i] /= norm;
            cdf_[i] /= norm;
        }
    }

    double RadialProfileND::radialPofile(const double x) const
    {
        if (x < 0.0) throw std::domain_error(
            "In npstat::RadialProfileND::radialPofile: "
            "input argument must be non-negative");
        if (x > 1.0)
            return 0.0;
        if (x == 0.0)
            return profile_[0];
        if (x == 1.0)
            return profile_[len_ - 1];

        unsigned idx = static_cast<unsigned>(x/step_);
        if (idx >= len_ - 1)
            idx = len_ - 2;
        const double dx = x/step_ - idx;

        switch (deg_)
        {
        case 0:
            if (dx < 0.5)
                return profile_[idx];
            else
                return profile_[idx + 1];

        case 1:
            return interpolate_linear(dx, profile_[idx], profile_[idx + 1]);

        case 2:
            if (idx == 0)
                return interpolate_quadratic(dx, profile_[idx],
                                             profile_[idx + 1],
                                             profile_[idx + 2]);
            else if (idx == len_ - 2)
                return interpolate_quadratic(dx+1.0, profile_[idx - 1],
                                             profile_[idx], profile_[idx + 1]);
            else
            {
                const double v0 = interpolate_quadratic(
                    dx, profile_[idx], profile_[idx + 1], profile_[idx + 2]);
                const double v1 = interpolate_quadratic(
                    dx+1.0, profile_[idx - 1],
                    profile_[idx], profile_[idx + 1]);
                return (v0 + v1)/2.0;
            }

        case 3:
            if (idx == 0)
                return interpolate_cubic(dx, profile_[idx], profile_[idx + 1],
                                         profile_[idx + 2], profile_[idx + 3]);
            else if (idx == len_ - 2)
                return interpolate_cubic(dx+2.0, profile_[idx-2],
                                         profile_[idx-1],
                                         profile_[idx], profile_[idx + 1]);
            else
                return interpolate_cubic(dx+1.0, profile_[idx - 1],
                                         profile_[idx],
                                         profile_[idx + 1], profile_[idx + 2]);
        default:
            assert(0);
            return 0.0;
        }
    }

    double RadialProfileND::unscaledDensity(const double* x) const
    {
        double sum = 0.0;
        for (unsigned i=0; i<dim_; ++i)
            sum += x[i]*x[i];
        return radialPofile(sqrt(sum));
    }

    double RadialProfileND::radialQuantile(const double rnd) const
    {
        if (rnd <= 0.0)
            return 0.0;
        if (rnd >= 1.0)
            return 1.0;

        const unsigned ibin = quantileBinFromCdf(&cdf_[0], len_, rnd);
        const double base = step_*ibin;
        const double ydelta = rnd - cdf_[ibin];
        assert(ydelta >= 0.0);
        if (ydelta == 0.0)
            return base;

        const double eps = 8.0*DBL_EPSILON;
        const bool useGaussRule = deg_ && 
            (dim_ + deg_ <= 2*sizeof(xg_)/sizeof(xg_[0]));

        double xlow = base;
        double xhi = base + step_;
        double half = (xlow + xhi)/2.0;
        double integ;
        while ((xhi - xlow)/(half + step_) > eps)
        {
            const double ds = (half - base)/step_;
            if (useGaussRule)
                integ = intervalIntegLowD(ibin, ds);
            else
                integ = intervalInteg(ibin, ds);
            if (integ > ydelta)
                xhi = half;
            else
                xlow = half;
            half = (xlow + xhi)/2.0;
        }
        return half;
    }

    void RadialProfileND::unscaledUnitMap(
        const double* rnd, const unsigned bufLen, double* x) const
    {
        if (bufLen != dim_) throw std::invalid_argument(
            "In npstat::RadialProfileND::unscaledUnitMap: "
            "incompatible dimensionality of the random vector");
        const double cdf = convertToSphericalRandom(rnd, dim_, x);
        const double r = radialQuantile(cdf);
        for (unsigned i=0; i<dim_; ++i)
            x[i] *= r;
    }

    bool BinnedDensityND::write(std::ostream& os) const
    {
        AbsScalableDistributionND::write(os);
        gs::write_pod(os, randomizer_.interpolationDegree());
        gs::ClassId(randomizer_.gridData()).write(os);
        randomizer_.gridData().write(os);
        return !os.fail();
    }

    BinnedDensityND* BinnedDensityND::read(const gs::ClassId& id,
                                           std::istream& in)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<BinnedDensityND>());
        current.ensureSameId(id);

        unsigned dim = 0, interpolationDeg = 0;
        std::vector<double> locations, scales;
        if (AbsScalableDistributionND::read(in, &dim, &locations, &scales))
            if (dim && locations.size() == dim && scales.size() == dim)
            {
                gs::read_pod(in, &interpolationDeg);
                gs::ClassId arrid(in, 1);
                ArrayND<double> gridData;
                ArrayND<double>::restore(arrid, in, &gridData);
                return new BinnedDensityND(
                    &locations[0], &scales[0], dim, gridData, interpolationDeg);
            }
        distributionReadError(in, classname());
        return 0;
    }

    bool BinnedDensityND::isEqual(const AbsDistributionND& ri) const
    {
        const BinnedDensityND& o = static_cast<const BinnedDensityND&>(ri);
        return AbsScalableDistributionND::isEqual(o) &&
               randomizer_ == o.randomizer_;
    }
}
