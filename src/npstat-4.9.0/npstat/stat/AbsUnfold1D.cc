#include <cmath>
#include <cassert>
#include <stdexcept>
#include <numeric>
#include <sstream>

#include "npstat/stat/AbsUnfold1D.hh"
#include "npstat/stat/LocalPolyFilter1D.hh"

namespace npstat {
    AbsUnfold1D::AbsUnfold1D(const Matrix<double>& responseMat)
        : responseMatrix_(responseMat),
          responseMatrixT_(responseMat.T()),
          efficiency_(responseMatrix_.nColumns()),
          filt_(0),
          useConvolutions_(false)
    {
        const unsigned ncols = responseMatrix_.nColumns();
        for (unsigned i=0; i<ncols; ++i)
        {
            efficiency_[i] = responseMatrix_.columnSum(i);
            if (efficiency_[i] <= 0.0)
            {
                std::ostringstream os;
                os << "In npstat::AbsUnfold1D constructor: "
                   << "efficiency for column " << i
                   << " is not positive. Please remove this column "
                   << "from the response matrix.";
                throw std::invalid_argument(os.str());
            }
        }
    }

    void AbsUnfold1D::validateDimensions(const unsigned lenObserved,
                                         const unsigned lenUnfolded) const
    {
        if (lenObserved != responseMatrix_.nRows())
            throw std::invalid_argument(
                "In npstat::AbsUnfold1D::validateDimensions: "
                "incompatible discretization of the observation space");
        if (lenUnfolded != responseMatrix_.nColumns())
            throw std::invalid_argument(
                "In npstat::AbsUnfold1D::validateDimensions: "
                "incompatible discretization of the unfolded space");
    }

    void AbsUnfold1D::validateDensity(const double* observ,
                                      const unsigned lenObserved)
    {
        assert(lenObserved);
        assert(observ);

        double s = 0.0;
        for (unsigned i=0; i<lenObserved; ++i)
        {
            if (observ[i] < 0.0)
                throw std::invalid_argument(
                    "In npstat::AbsUnfold1D::validateDensity: "
                    "argument counts must not be negative");
            s += observ[i];
        }
        if (s == 0.0)
            throw std::invalid_argument(
                "In npstat::AbsUnfold1D::validateDensity: "
                "argument array is empty");
    }

    void AbsUnfold1D::setInitialApproximation(const double* approx,
                                              const unsigned lenApprox)
    {
        validateDimensions(responseMatrix_.nRows(), lenApprox);
        validateDensity(approx, lenApprox);

        if (initialApproximation_.size() != lenApprox)
            initialApproximation_.resize(lenApprox);
        for (unsigned i=0; i<lenApprox; ++i)
            initialApproximation_[i] = approx[i];
    }

    void AbsUnfold1D::clearInitialApproximation()
    {
        initialApproximation_.clear();
    }

    const std::vector<double>& AbsUnfold1D::getInitialApproximation() const
    {
        return initialApproximation_;
    }

    void AbsUnfold1D::buildUniformInitialApproximation(
        const double* observed,
        const unsigned lenObserved,
        std::vector<double>* result) const
    {
        const unsigned ncols = responseMatrix_.nColumns();
        validateDimensions(lenObserved, ncols);
        assert(observed);
        assert(result);

        const long double observedSum = std::accumulate(
            observed, observed+lenObserved, 0.0L);
        const long double effSum = std::accumulate(
            efficiency_.begin(), efficiency_.end(), 0.0L);
        const long double eff = effSum/efficiency_.size();
        const long double unfoldedSum = observedSum/eff;
        const double u = unfoldedSum/ncols;

        if (result->size() != ncols)
            result->resize(ncols);
        double *data = &(*result)[0];
        for (unsigned i=0; i<ncols; ++i)
            data[i] = u;
    }

    void AbsUnfold1D::setFilter(const LocalPolyFilter1D* f)
    {
        if (f)
            validateDimensions(responseMatrix_.nRows(), f->dataLen());
        filt_ = f;
    }

    const LocalPolyFilter1D* AbsUnfold1D::getFilter(const bool throwIfNull) const
    {
        if (!filt_ && throwIfNull) throw std::runtime_error(
            "In npstat::AbsUnfold1D::getFilter: filter has not been set");
        return filt_;
    }

    std::pair<double, double> AbsUnfold1D::smoothingNDoF() const
    {
        std::pair<double, double> n;
        const Matrix<double>& fmat = getFilter(true)->getFilterMatrix();
        const Matrix<double>& cov = useConvolutions_ ? 
            fmat.TtimesThis() : fmat.timesT();
        n.first = cov.symPSDefEffectiveRank(0.0, EIGEN_D_AND_C, &n.second);
        return n;
    }

    std::pair<double, double> AbsUnfold1D::responseNDoF() const
    {
        std::pair<double, double> n;
        const Matrix<double>& cov = responseMatrix_ * responseMatrixT_;
        n.first = cov.symPSDefEffectiveRank(0.0, EIGEN_D_AND_C, &n.second);
        return n;
    }

    std::pair<double, double> AbsUnfold1D::smoothedResponseNDoF() const
    {
        std::pair<double, double> n;
        const Matrix<double>& fmat = getFilter(true)->getFilterMatrix();
        const Matrix<double>& transfer = useConvolutions_ ? 
            responseMatrix_ * fmat.T() : responseMatrix_ * fmat;
        const Matrix<double>& cov = transfer.timesT();
        n.first = cov.symPSDefEffectiveRank(0.0, EIGEN_D_AND_C, &n.second);
        return n;
    }

    Matrix<double> AbsUnfold1D::observationCovariance(
        const double* yhat, const unsigned lenObserved, const bool isMulti)
    {
        assert(lenObserved);
        assert(yhat);

        Matrix<double> mcov(lenObserved, lenObserved, 0);
        if (isMulti)
        {
            const double N = std::accumulate(yhat, yhat+lenObserved, 0.0L);
            for (unsigned i=0; i<lenObserved; ++i)
            {
                const double pi = yhat[i]/N;
                double* mat = mcov[i];
                mat[i] = N*pi*(1.0 - pi);
                for (unsigned j=0; j<i; ++j)
                {
                    const double c = -pi*yhat[j];
                    mat[j] = c;
                    mcov[j][i] = c;
                }
            }
        }
        else
        {
            // Assume the Poisson distribution
            for (unsigned i=0; i<lenObserved; ++i)
                mcov[i][i] = yhat[i];
            mcov.tagAsDiagonal();
        }
        return mcov;
    }

    double AbsUnfold1D::probDelta(const double* prev, const double* next,
                                  const unsigned len)
    {
        assert(len);
        assert(prev);
        assert(next);

        long double del = 0.0L, sum = 0.0L;
        for (unsigned i=0; i<len; ++i)
        {
            del += fabs(*prev - *next);
            sum += fabs(*prev++);
            sum += fabs(*next++);
        }
        sum /= 2.0L;
        double ratio = 0.0;
        if (sum)
            ratio = del/sum;
        return ratio;
    }
}
