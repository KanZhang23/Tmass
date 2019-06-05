#include <cmath>
#include <cfloat>
#include <cassert>
#include <stdexcept>

#include "geners/IOException.hh"

#include "npstat/nm/SimpleFunctors.hh"
#include "npstat/nm/MathUtils.hh"
#include "npstat/nm/SpecialFunctions.hh"

#include "npstat/rng/convertToSphericalRandom.hh"

#include "npstat/stat/Copulas.hh"
#include "npstat/stat/NMCombinationSequencer.hh"

#define SQRT2 1.41421356237309505
#define FGM_MAX_DIM 31U

namespace npstat {
    GaussianCopula::GaussianCopula(const Matrix<double>& covmat)
        : AbsDistributionND(covmat.nRows()),
          form_(covmat.covarToCorr().symPDInv()),
          sqrCov_(covmat.covarToCorr().symFcn(
                      FcnFunctor1<double,double>(sqrt))),
          norm_(sqrt(form_.det())),
          buf_(covmat.nRows())
    {
        if (norm_ <= 0.0) throw std::invalid_argument(
            "In npstat::GaussianCopula constructor: "
            "invalid or degenerate covariance matrix");
        form_ -= Matrix<double>(covmat.nRows(), covmat.nRows(), 1);   
    }

    double GaussianCopula::density(const double* x, const unsigned dim) const
    {
        if (dim != dim_) throw std::invalid_argument(
            "In npstat::GaussianCopula::density: "
            "incompatible input point dimensionality");
        assert(x);
        double* buf = &buf_[0];
        for (unsigned i=0; i<dim; ++i)
        {
            if (x[i] < 0.0 || x[i] > 1.0)
                return 0.0;
            buf[i] = inverseGaussCdf(x[i]);
        }
        return norm_*exp(-form_.bilinear(buf, dim)/2.0);
    }

    void GaussianCopula::unitMap(const double* rnd, const unsigned bufLen,
                                 double* x) const
    {
        if (bufLen != dim_) throw std::invalid_argument(
            "In npstat::GaussianCopula::unitMap: "
            "incompatible dimensionality of the random vector");
        assert(rnd);
        assert(x);

        double* buf = &buf_[0];
        for (unsigned i=0; i<bufLen; ++i)
            buf[i] = inverseGaussCdf(rnd[i]);
        const Matrix<double>& m(sqrCov_.timesVector(buf, bufLen));
        const double* d = m.data();
        for (unsigned i=0; i<bufLen; ++i)
            x[i] = (1.0 + erf(d[i]/SQRT2))/2.0;
    }

    bool GaussianCopula::write(std::ostream& os) const
    {
        gs::write_pod(os, dim_);
        form_.classId().write(os);
        form_.write(os);
        sqrCov_.write(os);
        gs::write_pod(os, norm_);
        return !os.fail();
    }

    GaussianCopula* GaussianCopula::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<GaussianCopula>());
        current.ensureSameId(id);

        unsigned dim;
        gs::read_pod(in, &dim);
        gs::ClassId mid(in, 1);
        if (in.fail())
            throw gs::IOReadFailure("In npstat::GaussianCopula::read: "
                                    "input stream failure");
        GaussianCopula* g = new GaussianCopula(dim);
        try 
        {
            g->form_.restore(mid, in, &g->form_);
            g->sqrCov_.restore(mid, in, &g->sqrCov_);
            gs::read_pod(in, &g->norm_);
            g->buf_.resize(dim);
            if (in.fail())
                throw gs::IOReadFailure("In npstat::GaussianCopula::read: "
                                        "input stream failure");
        }
        catch (...)
        {
            delete g;
            throw;
        }
        return g;
    }

    TCopula::TCopula(const Matrix<double>& covmat,
                     const double nDegreesOfFreedom)
        : AbsDistributionND(covmat.nRows()),
          form_(covmat.covarToCorr().symPDInv()),
          sqrCov_(covmat.covarToCorr().symFcn(
                      FcnFunctor1<double,double>(sqrt))),
          nDoF_(nDegreesOfFreedom),
          norm_(sqrt(form_.det())),
          t_(0.0, 1.0, nDegreesOfFreedom),
          buf_(covmat.nRows())
    {
        if (norm_ <= 0.0) throw std::invalid_argument(
            "In npstat::TCopula constructor: "
            "invalid or degenerate covariance matrix");
        power_ = (nDoF_ + dim_)/2.0;
        norm_ *= (Gamma(power_)/Gamma(nDoF_/2.0)/pow(M_PI*nDoF_, dim_/2.0));
    }

    double TCopula::density(const double* x, const unsigned dim) const
    {
        if (dim != dim_) throw std::invalid_argument(
            "In npstat::TCopula::density: "
            "incompatible input point dimensionality");
        assert(x);
        double* buf = &buf_[0];
        long double dens = norm_;
        for (unsigned i=0; i<dim; ++i)
        {
            if (x[i] <= 0.0 || x[i] >= 1.0)
                return 0.0;
            buf[i] = t_.quantile(x[i]);
            dens /= t_.density(buf[i]);
        }
        return dens*pow(1.0 + form_.bilinear(buf, dim)/nDoF_, -power_);
    }

    void TCopula::unitMap(const double* rnd, const unsigned bufLen,
                          double* x) const
    {
        if (bufLen != dim_) throw std::invalid_argument(
            "In npstat::TCopula::unitMap: "
            "incompatible dimensionality of the random vector");
        assert(rnd);
        assert(x);

        // Map the point according to multivariate Student's t distribution
        double* buf = &buf_[0];
        const double rrnd = convertToSphericalRandom(rnd, dim_, buf);
        const double xr = inverseIncompleteBeta(dim_/2.0, nDoF_/2.0, rrnd);
        double denom = 1.0 - xr;
        if (denom <= 0.0)
            denom = DBL_EPSILON;
        const double t = xr/denom;
        const double r = sqrt(nDoF_*t);
        for (unsigned i=0; i<bufLen; ++i)
            buf[i] *= r;

        // Rotate and convert into individual variables
        const Matrix<double>& m(sqrCov_.timesVector(buf, bufLen));
        const double* d = m.data();
        for (unsigned i=0; i<bufLen; ++i)
            x[i] = t_.cdf(d[i]);
    }

    FGMCopula::FGMCopula(const unsigned dim, const double* params,
                         const unsigned nParams)
        : AbsDistributionND(dim),
          cornerValues_(1U << dim, 0.0)
    {
        if (!(dim && dim <= FGM_MAX_DIM)) throw std::invalid_argument(
            "In npstat::FGMCopula constructor: insupported dimensionality");
        if (nParams != (1U << dim) - dim - 1U) throw std::invalid_argument(
            "In npstat::FGMCopula constructor: wrong # of input parameters");
        if (nParams)
        {
            // Trivial check that parameter values are allowed
            assert(params);
            for (unsigned i=0; i<nParams; ++i)
                if (fabs(params[i]) > 1.0)
                    throw std::invalid_argument(
                        "In npstat::FGMCopula constructor: "
                        "invalid input parameters");
        }
        else
        {
            // This means dim == 1
            cornerValues_[0] = 1.0;
            cornerValues_[1] = 1.0;
            return;
        }

        // Prepare sequencers for density calculations
        std::vector<NMCombinationSequencer> seq;
        seq.reserve(dim - 2U);
        for (unsigned i=2; i<dim; ++i)
            seq.push_back(NMCombinationSequencer(i, dim));
        NMCombinationSequencer* sequencers = dim > 2U ? &seq[0] : 0;

        // Calculate corner values
        bool valid = true;
        double corner[FGM_MAX_DIM];
        const unsigned maxcycle = 1U << dim;
        for (unsigned icycle=0; icycle<maxcycle && valid; ++icycle)
        {
            for (unsigned i=0; i<dim; ++i)
            {
                if (icycle & (1UL << i))
                    corner[i] = 1.0;
                else
                    corner[i] = 0.0;
            }
            const double value = density0(sequencers, params, corner, dim);
            if (value < 0.0)
                valid = false;
            else
                cornerValues_[icycle] = value;
        }

        if (!valid)
            throw std::invalid_argument(
                "In npstat::FGMCopula constructor: "
                "invalid input parameters");
    }

    bool TCopula::write(std::ostream& os) const
    {
        gs::write_pod(os, dim_);
        gs::write_pod(os, nDoF_);
        form_.classId().write(os);
        form_.write(os);
        sqrCov_.write(os);
        gs::write_pod(os, norm_);
        gs::write_pod(os, power_);
        return !os.fail();
    }

    TCopula* TCopula::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<TCopula>());
        current.ensureSameId(id);

        unsigned dim;
        gs::read_pod(in, &dim);
        double ndof;
        gs::read_pod(in, &ndof);
        gs::ClassId mid(in, 1);
        if (in.fail())
            throw gs::IOReadFailure("In npstat::TCopula::read: "
                                    "input stream failure");
        TCopula* g = new TCopula(dim, ndof);
        try
        {
            g->form_.restore(mid, in, &g->form_);
            g->sqrCov_.restore(mid, in, &g->sqrCov_);
            gs::read_pod(in, &g->norm_);
            gs::read_pod(in, &g->power_);
            g->buf_.resize(dim);
            if (in.fail())
                throw gs::IOReadFailure("In npstat::TCopula::read: "
                                        "input stream failure");
        }
        catch (...)
        {
            delete g;
            throw;
        }
        return g;
    }

    bool FGMCopula::write(std::ostream& os) const
    {
        gs::write_pod(os, dim_);
        gs::write_pod_vector(os, cornerValues_);
        return !os.fail();
    }

    FGMCopula* FGMCopula::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<FGMCopula>());
        current.ensureSameId(id);

        unsigned dim;
        gs::read_pod(in, &dim);
        if (in.fail())
            throw gs::IOReadFailure("In npstat::FGMCopula::read: "
                                    "input stream failure");
        FGMCopula* g = new FGMCopula(dim);
        try
        {
            gs::read_pod_vector(in, &g->cornerValues_);
            if (in.fail())
                throw gs::IOReadFailure("In npstat::FGMCopula::read: "
                                        "input stream failure");
        }
        catch (...)
        {
            delete g;
            throw;
        }
        return g;
    }

    double FGMCopula::density(const double* x, const unsigned dim) const
    {
        if (dim != dim_) throw std::invalid_argument(
            "In npstat::FGMCopula::density: "
            "incompatible input point dimensionality");
        assert(x);

        for (unsigned i=0; i<dim; ++i)
            if (x[i] < 0.0 || x[i] > 1.0)
                return 0.0;

        return interpolate(&cornerValues_[0], x, dim);
    }

    double FGMCopula::interpolate(const double* corners, const double* x,
                                  const unsigned dim)
    {
        long double sum = 0.0L;
        const unsigned maxcycle = 1U << dim;
        for (unsigned icycle=0; icycle<maxcycle; ++icycle)
        {
            double w = 1.0;
            for (unsigned i=0; i<dim; ++i)
            {
                if (icycle & (1UL << i))
                    w *= x[i];
                else
                    w *= (1.0 - x[i]);
            }
            sum += corners[icycle]*w;
        }
        return sum;
    }

    double FGMCopula::density0(NMCombinationSequencer* sequencers,
                               const double* par, const double* x,
                               const unsigned dim)
    {
        long double dens = 0.0L;
        unsigned parNum = 0;
        for (unsigned i=2; i<dim; ++i)
        {
            NMCombinationSequencer& s(sequencers[i - 2U]);
            for (s.reset(); s.isValid(); ++s, ++parNum)
            {
                long double prod = par[parNum];
                const unsigned m = s.M();
                const unsigned* idx = s.combination();
                for (unsigned i=0; i<m; ++i)
                    prod *= (1.0L - 2.0L*x[idx[i]]);
                dens += prod;
            }
        }

        if (parNum != (1U << dim) - dim - 2U) 
            throw std::invalid_argument(
                "In npstat::FGMCopula::density0: "
                "incompatible input point dimensionality");
        long double lastProd = par[parNum];
        for (unsigned i=0; i<dim; ++i)
            lastProd *= (1.0L - 2.0L*x[i]);
        dens += lastProd;
        dens += 1.0L;

        return dens;
    }

    void FGMCopula::unitMap(const double* rnd, const unsigned bufLen,
                            double* x) const
    {
        if (bufLen != dim_) throw std::invalid_argument(
            "In npstat::FGMCopula::unitMap: "
            "incompatible dimensionality of the random vector");
        assert(rnd);
        assert(x);
        for (unsigned i=0; i<dim_; ++i)
            if (!(rnd[i] >= 0.0 && rnd[i] <= 1.0))
                throw std::domain_error(
                    "In npstat::FGMCopula::unitMap: a value "
                    "in the random vector is outside of [0, 1] interval");

        // Marginal density in the first variable is a constant
        x[0] = rnd[0];

        // Density is linear in any variable (for all other
        // variables fixed). Because of this, integration over
        // any variable is equivalent to calculating the density
        // with that variable set to 0.5.
        for (unsigned i=2; i<dim_; ++i)
            x[i] = 0.5;

        const double* corners = &cornerValues_[0];
        for (unsigned i=1; i<dim_; ++i)
        {
            x[i] = 0.0;
            double lo = interpolate(corners, x, dim_);
            x[i] = 1.0;
            double hi = interpolate(corners, x, dim_);
            const double norm = (lo + hi)/2.0;
            if (norm <= 0.0)
            {
                x[i] = rnd[i];
                continue;
            }
            lo /= norm;
            hi /= norm;
            const double delta = (hi - lo)/2.0;
            if (fabs(delta) < 1.e-11)
            {
                x[i] = rnd[i];
                continue;
            }
            double x1, x2;
            if (!solveQuadratic(lo/delta, -rnd[i]/delta, &x1, &x2))
                throw std::runtime_error("In npstat::FGMCopula::unitMap: "
                                         "no solutions");
            if (fabs(x1 - 0.5) < fabs(x2 - 0.5))
                x[i] = x1;
            else
                x[i] = x2;
            if (x[i] < 0.0)
                x[i] = 0.0;
            if (x[i] > 1.0)
                x[i] = 1.0;
        }
    }
}
