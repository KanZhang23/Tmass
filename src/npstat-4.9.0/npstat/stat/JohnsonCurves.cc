#include <cmath>
#include <cfloat>
#include <cassert>
#include <stdexcept>

#include "geners/binaryIO.hh"
#include "geners/CPP11_auto_ptr.hh"

#include "npstat/nm/SpecialFunctions.hh"

#include "npstat/stat/JohnsonCurves.hh"
#include "npstat/stat/Distributions1D.hh"
#include "npstat/stat/distributionReadError.hh"

#define ONE   1.0
#define TWO   2.0
#define THREE 3.0
#define FOUR  4.0
#define DSQRT sqrt
#define SQR2PI 2.5066282746310005
#define SQRT2 1.41421356237309505

// The following function calculates correct values of beta_1 and D[beta_1,W]
// for given values of W ( = exp(delta^-2)) and beta_2 for Johnson's S_u type
// curve.
//
// Input:
//      W  - Johnson's small omega
//      B2 - beta_2
// Output:
//      B1 - beta_1
//      DB1DW - D[beta_1,w]
//      pM - Johnson's m
//
// Translated from Fortran
//
static void beta1(const long double W, const long double B2,
                  double* B1, double* DB1DW, double* pM)
{
    typedef long double Real;

    const Real WP1=W+1.0L;
    const Real WP2=W+2.0L;
    const Real WM1=W-1.0L;
    const Real B2M3=B2-3.0L;
    const Real A2 = 8.0L*(6.0L+W*(6.0L+W*(3.0L+W)));
    const Real DA2DW = 24.0L*(2.0L+W*WP2);
    const Real A1 = A2*W+8.0L*(W+3.0L);
    const Real DA1DW = DA2DW*W+A2+8.0L;
    const Real A0 = WP1*WP1*WP1*(W*W+3.0L);
    const Real DA0DW = WP1*WP1*(9.0L+W*(2.0L+5.0L*W));
    const Real A = A2*WM1-8.0L*B2M3;
    assert(A);
    const Real DADW = DA1DW-DA2DW-8.0L;
    const Real B = A1*WM1-8.0L*WP1*B2M3;
    const Real DBDW = DA1DW*WM1+A1-8.0L*B2M3;
    const Real C = A0*WM1-2.0L*B2M3*WP1*WP1;
    const Real DCDW = DA0DW*WM1+A0-4.0L*B2M3*WP1;
    const Real D = B*B-4.0L*A*C;
    assert(D > 0.0L);
    const Real DDDW = 2.0L*B*DBDW-4.0L*(A*DCDW+C*DADW);
    Real TMP = std::sqrt(D);
    Real DTMPDW = 0.5/TMP*DDDW;
    const Real M = B > 0.0L ? -2.0L*C/(B+TMP) : (TMP-B)/2.0L/A;
    const Real DMDW = (DTMPDW - DBDW - DADW*M*2.0L)/A/2.0L;

    TMP = 4.0L*WP2*M+3.0L*WP1*WP1;
    DTMPDW = 4.0L*(M+DMDW*WP2)+6.0L*WP1;
    const Real NUM = M*WM1*TMP*TMP;
    const Real DNUMDW = TMP*(WM1*(DMDW*TMP+2.0L*DTMPDW*M)+M*TMP);
    const Real loctmp = 2.0L*M+WP1;
    const Real DEN = 2.0L*loctmp*loctmp*loctmp;
    const Real DDENDW = 6.0L*loctmp*loctmp*(2.0L*DMDW+1.0L);

    *B1 = static_cast<double>(NUM/DEN);
    *DB1DW = static_cast<double>((DNUMDW*DEN-DDENDW*NUM)/(DEN*DEN));
    *pM = static_cast<double>(M);
}


namespace npstat {
    bool JohnsonSu::isEqual(const AbsDistribution1D& otherBase) const
    {
        const JohnsonSu& r = static_cast<const JohnsonSu&>(otherBase);
        return AbsScalableDistribution1D::isEqual(r) && 
            isValid_ && r.isValid_ && skew_ == r.skew_ && kurt_ == r.kurt_ &&
            delta_ == r.delta_ && lambda_ == r.lambda_ &&
            gamma_ == r.gamma_ && xi_ == r.xi_;
    }

    // The code below looks funny because this is a translation from Fortran
    void JohnsonSu::initialize()
    {
        const double eps = 2.0e-13;
        const unsigned maxiter = 100000U;

        delta_ = 0.0;
        lambda_ = 0.0;
        gamma_ = 0.0;
        xi_ = 0.0;

        const double B1 = skew_*skew_;
        double TMP = pow((TWO+B1+DSQRT(B1*(FOUR+B1)))/TWO, ONE/THREE);
        double W = TMP+ONE/TMP-ONE;
        TMP = W*W*(W*(W+TWO)+THREE)-THREE;
        isValid_ = kurt_ > TMP;
        if (isValid_)
        {
            // Make a guess for the value of W
            W = DSQRT(DSQRT(TWO*kurt_-TWO)-ONE)-B1/kurt_;

            // Iterations to get the correct W
            double B1TMP, DB1DW, M;
            beta1(W, kurt_, &B1TMP, &DB1DW, &M);
            unsigned count = 0U;
            while (fabs(B1-B1TMP)/(fabs(B1)+ONE) > eps && count < maxiter)
            {
                W += (B1-B1TMP)/DB1DW;
                beta1(W, kurt_, &B1TMP, &DB1DW, &M);
                ++count;
            }
            if (count >= maxiter)
            {
                // Newton-Raphson convergence is supposed to be much faster.
                // If we are here, it means we have entered an infinite loop.
                throw std::runtime_error("In npstat::JohnsonSu::initialize: "
                                         "infinite loop detected");
            }
            if (M < 0.0)
                M = 0.0;
            delta_ = DSQRT(ONE/log(W));
            lambda_ = 1.0/DSQRT((W-ONE)*(TWO*M+W+ONE)/TWO);
            if (skew_)
            {
                const double sgn = skew_ > 0.0 ? 1.0 : -1.0;
                TMP = DSQRT(M/W);
                gamma_ = -sgn*fabs(delta_*log(TMP+DSQRT(TMP*TMP+ONE)));
                xi_ = -DSQRT(W)*lambda_*sgn*fabs(TMP);
            }
        }
    }

    JohnsonSu::JohnsonSu(const double location, const double scale,
                         const double skewness, const double kurtosis)
        : AbsScalableDistribution1D(location, scale),
          skew_(skewness),
          kurt_(kurtosis)
    {
        initialize();
    }

    JohnsonSu::JohnsonSu(const double location, const double scale,
                         const std::vector<double>& params)
        : AbsScalableDistribution1D(location, scale),
          skew_(params[0]),
          kurt_(params[1])
    {
        initialize();
    }

    double JohnsonSu::unscaledDensity(const double x) const
    {
        if (isValid_)
        {
            const double TMP = (x-xi_)/lambda_;
            const double y = gamma_ + delta_*log(TMP+DSQRT(1.0+TMP*TMP));
            return delta_/lambda_/SQR2PI/DSQRT(1.0+TMP*TMP)*exp(-y*y/2.0);
        }
        else
            return -1.0;
    }

    double JohnsonSu::unscaledCdf(const double x) const
    {
        if (isValid_)
        {
            const double diff = delta_*asinh((x - xi_)/lambda_) + gamma_;
            if (diff < 0.0)
                return erfc(-diff/SQRT2)/2.0;
            else
                return (1.0 + erf(diff/SQRT2))/2.0;
        }
        else
            return -1.0;
    }

    double JohnsonSu::unscaledExceedance(const double x) const
    {
        if (isValid_)
        {
            const double diff = delta_*asinh((x - xi_)/lambda_) + gamma_;
            if (diff > 0.0)
                return erfc(diff/SQRT2)/2.0;
            else
                return (1.0 - erf(diff/SQRT2))/2.0;
        }
        else
            return -1.0;
    }

    double JohnsonSu::unscaledQuantile(const double x) const
    {
        if (!(x >= 0.0 && x <= 1.0)) throw std::domain_error(
            "In npstat::JohnsonSu::unscaledQuantile: cdf"
            " argument outside of [0, 1] interval");
        if (isValid_)
            return lambda_*sinh((inverseGaussCdf(x) - gamma_)/delta_) + xi_;
        else
            return 0.0;
    }

    bool JohnsonSu::write(std::ostream& os) const
    {
        AbsScalableDistribution1D::write(os);
        gs::write_pod(os, skew_);
        gs::write_pod(os, kurt_);
        gs::write_pod(os, delta_);
        gs::write_pod(os, lambda_);
        gs::write_pod(os, gamma_);
        gs::write_pod(os, xi_);
        gs::write_pod(os, isValid_);
        return !os.fail();
    }

    JohnsonSu* JohnsonSu::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<JohnsonSu>());
        current.ensureSameId(id);

        double location, scale;
        JohnsonSu* j = 0;
        if (AbsScalableDistribution1D::read(in, &location, &scale))
        {
            j = new JohnsonSu(location, scale);
            gs::read_pod(in, &j->skew_);
            gs::read_pod(in, &j->kurt_);
            gs::read_pod(in, &j->delta_);
            gs::read_pod(in, &j->lambda_);
            gs::read_pod(in, &j->gamma_);
            gs::read_pod(in, &j->xi_);
            gs::read_pod(in, &j->isValid_);
            if (!in.fail())
                return j;
        }
        delete j;
        distributionReadError(in, classname());
        return 0;
    }

    bool JohnsonSb::isEqual(const AbsDistribution1D& otherBase) const
    {
        const JohnsonSb& r = static_cast<const JohnsonSb&>(otherBase);
        return AbsScalableDistribution1D::isEqual(r) && 
            isValid_ && r.isValid_ && skew_ == r.skew_ && kurt_ == r.kurt_ &&
            delta_ == r.delta_ && lambda_ == r.lambda_ &&
            gamma_ == r.gamma_ && xi_ == r.xi_;
    }

    bool JohnsonSb::write(std::ostream& os) const
    {
        AbsScalableDistribution1D::write(os);
        gs::write_pod(os, skew_);
        gs::write_pod(os, kurt_);
        gs::write_pod(os, delta_);
        gs::write_pod(os, lambda_);
        gs::write_pod(os, gamma_);
        gs::write_pod(os, xi_);
        gs::write_pod(os, isValid_);
        return !os.fail();
    }

    JohnsonSb* JohnsonSb::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<JohnsonSb>());
        current.ensureSameId(id);

        double location, scale;
        JohnsonSb* j = 0;
        if (AbsScalableDistribution1D::read(in, &location, &scale))
        {
            j = new JohnsonSb(location, scale);
            gs::read_pod(in, &j->skew_);
            gs::read_pod(in, &j->kurt_);
            gs::read_pod(in, &j->delta_);
            gs::read_pod(in, &j->lambda_);
            gs::read_pod(in, &j->gamma_);
            gs::read_pod(in, &j->xi_);
            gs::read_pod(in, &j->isValid_);
            if (!in.fail())
                return j;
        }
        delete j;
        distributionReadError(in, classname());
        return 0;
    }

    JohnsonSb::JohnsonSb(const double location, const double scale,
                         const double skewness, const double kurtosis)
        : AbsScalableDistribution1D(location, scale),
          skew_(skewness),
          kurt_(kurtosis)
    {
        isValid_ = fitParameters(skew_,kurt_,&gamma_,&delta_,&lambda_,&xi_);
    }

    JohnsonSb::JohnsonSb(const double location, const double scale,
                         const std::vector<double>& params)
        : AbsScalableDistribution1D(location, scale),
          skew_(params[0]),
          kurt_(params[1])
    {
        isValid_ = fitParameters(skew_,kurt_,&gamma_,&delta_,&lambda_,&xi_);
    }

    double JohnsonSb::unscaledDensity(const double x) const
    {
        if (isValid_)
        {
            if (x <= xi_ || x >= xi_ + lambda_)
                return 0.0;
            else
            {
                const double y = gamma_+delta_*log((x-xi_)/(lambda_-x+xi_));
                return (delta_*lambda_)/
                    (exp(y*y/2.0)*SQR2PI*(x-xi_)*(lambda_-x+xi_));
            }
        }
        else
            return -1.0;
    }

    double JohnsonSb::unscaledCdf(const double x) const
    {
        if (isValid_)
        {
            if (x <= xi_)
                return 0.0;
            else if (x >= xi_ + lambda_)
                return 1.0;
            else
            {
                double diff = (x - xi_)/lambda_;
                const double tmp = diff/(1.0 - diff);
                diff = delta_*log(tmp) + gamma_;
                if (diff < 0.0)
                    return erfc(-diff/SQRT2)/2.0;
                else
                    return (1.0 + erf(diff/SQRT2))/2.0;
            }
        }
        else
            return -1.0;
    }

    double JohnsonSb::unscaledExceedance(const double x) const
    {
        if (isValid_)
        {
            if (x <= xi_)
                return 0.0;
            else if (x >= xi_ + lambda_)
                return 1.0;
            else
            {
                double diff = (x - xi_)/lambda_;
                const double tmp = diff/(1.0 - diff);
                diff = delta_*log(tmp) + gamma_;
                if (diff > 0.0)
                    return erfc(diff/SQRT2)/2.0;
                else
                    return (1.0 - erf(diff/SQRT2))/2.0;
            }
        }
        else
            return -1.0;
    }

    double JohnsonSb::unscaledQuantile(const double x) const
    {
        if (!(x >= 0.0 && x <= 1.0)) throw std::domain_error(
            "In npstat::JohnsonSb::unscaledQuantile: cdf"
            " argument outside of [0, 1] interval");
        if (isValid_)
        {
            if (x == 0.0)
                return xi_;
            else if (x == 1.0)
                return xi_ + lambda_;
            else
            {
                const double tmp = exp((inverseGaussCdf(x) - gamma_)/delta_);
                return lambda_*tmp/(tmp + 1.0) + xi_;
            }
        }
        else
            return 0.0;
    }

    bool JohnsonSystem::isEqual(const AbsDistribution1D& otherBase) const
    {
        const JohnsonSystem& r = static_cast<const JohnsonSystem&>(otherBase);
        return AbsScalableDistribution1D::isEqual(r) && 
               curveType_ == r.curveType_ && curveType_ != INVALID &&
               skew_ == r.skew_ && kurt_ == r.kurt_;
    }

    JohnsonSystem::JohnsonSystem(const double location, const double scale,
                                 const double skewness, const double kurtosis)
        : AbsScalableDistribution1D(location, scale),
          fcn_(0),
          skew_(skewness),
          kurt_(kurtosis)
    {
        initialize();
    }

    void JohnsonSystem::initialize()
    {
        curveType_ = JohnsonSystem::select(skew_, kurt_);
        switch (curveType_)
        {
        case GAUSSIAN:
            fcn_ = new Gauss1D(0.0, 1.0);
            break;
        case LOGNORMAL:
            fcn_ = new LogNormal(0.0, 1.0, skew_);
            break;
        case SU:
            fcn_ = new JohnsonSu(0.0, 1.0, skew_, kurt_);
            break;
        case SB:
            fcn_ = new JohnsonSb(0.0, 1.0, skew_, kurt_);
            break;
        case INVALID:
            break;
        default:
            assert(0);
        }
    }

    JohnsonSystem::JohnsonSystem(const double location, const double scale,
                                 const std::vector<double>& params)
        : AbsScalableDistribution1D(location, scale),
          fcn_(0),
          skew_(params[0]),
          kurt_(params[1])
    {
        initialize();        
    }

    JohnsonSystem::JohnsonSystem(const JohnsonSystem& r)
        : AbsScalableDistribution1D(r),
          fcn_(0),
          skew_(r.skew_),
          kurt_(r.kurt_),
          curveType_(r.curveType_)
    {
        if (r.fcn_)
            fcn_ = r.fcn_->clone();
    }

    JohnsonSystem& JohnsonSystem::operator=(const JohnsonSystem& r)
    {
        if (this != &r)
        {
            AbsScalableDistribution1D* newfcn = 0;
            if (r.fcn_)
                newfcn = r.fcn_->clone();
            AbsScalableDistribution1D::operator=(r);
            skew_ = r.skew_;
            kurt_ = r.kurt_;
            curveType_ = r.curveType_;
            delete fcn_;
            fcn_ = newfcn;
        }
        return *this;
    }

    JohnsonSystem::~JohnsonSystem()
    {
        delete fcn_;
    }

    JohnsonSystem::CurveType JohnsonSystem::select(const double skew,
                                                   const double kurt)
    {
        // The tolerance here has to be made sufficiently large
        // so that the choice of Su or Sb would be correct even
        // if the calculations of the lognormal boundary were
        // performed with long double precision. This is because
        // Sb parameters are fitted using long double precision.
        static const double tol = 64.0*DBL_EPSILON;

        // Check for impossible combination of skewness and kurtosis
        const double b1 = skew*skew;
        if (kurt <= b1 + 1.0 + tol)
            return INVALID;

        // Check for Gaussian
        if (std::abs(skew) < tol && std::abs((kurt - 3.0)/3.0) < tol)
            return GAUSSIAN;

        // Check for lognormal
        const double B1 = skew*skew;
        double TMP = pow((TWO+B1+DSQRT(B1*(FOUR+B1)))/TWO, ONE/THREE);
        double W = TMP+ONE/TMP-ONE;
        TMP = W*W*(W*(W+TWO)+THREE)-THREE;
        if (std::abs((kurt - TMP)/kurt) < tol)
            return LOGNORMAL;

        // The only remaining choice is Su or Sb
        return kurt < TMP ? SB : SU;
    }

    bool JohnsonSystem::write(std::ostream& os) const
    {
        AbsScalableDistribution1D::write(os);

        gs::write_pod(os, skew_);
        gs::write_pod(os, kurt_);
        int curveType = static_cast<int>(curveType_);
        gs::write_pod(os, curveType);
        if (curveType_ != INVALID)
        {
            fcn_->classId().write(os);
            fcn_->write(os);
        }
        return !os.fail();
    }

    JohnsonSystem* JohnsonSystem::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<JohnsonSystem>());
        current.ensureSameId(id);

        double location, scale;
        JohnsonSystem* j = 0;
        if (AbsScalableDistribution1D::read(in, &location, &scale))
        {
            j = new JohnsonSystem(location, scale);
            gs::read_pod(in, &j->skew_);
            gs::read_pod(in, &j->kurt_);
            int curveType;
            gs::read_pod(in, &curveType);
            j->curveType_ = static_cast<CurveType>(curveType);
            if (j->curveType_ != INVALID)
            {
                gs::ClassId id(in, 1);
                j->fcn_ = dynamic_cast<AbsScalableDistribution1D*>(
                    AbsDistribution1D::read(id, in));
                if (j->fcn_)
                    return j;
            }
            else if (!in.fail())
                return j;
        }
        delete j;
        distributionReadError(in, classname());
        return 0;
    }
}
