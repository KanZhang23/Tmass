#include <cassert>
#include <utility>
#include <algorithm>
#include <numeric>

#include "npstat/nm/allocators.hh"

#include "npstat/stat/SmoothedEMUnfold1D.hh"
#include "npstat/stat/StatUtils.hh"
#include "npstat/stat/LocalPolyFilter1D.hh"

typedef npstat::Matrix<double> DMat;

namespace npstat {
    SmoothedEMUnfold1D::SmoothedEMUnfold1D(
        const DMat& responseMatrix,
        const LocalPolyFilter1D& f,
        const bool i_useConvolutions,
        const bool i_useMultinomialCovariance,
        const bool i_smoothLastIteration,
        const double i_convergenceEpsilon,
        const unsigned i_maxIterations)
        : AbsUnfold1D(responseMatrix),
          convergenceEpsilon_(0.0),
          lastNormfactor_(1.0),
          maxIterations_(i_maxIterations),
          lastIterations_(0),
          lastEPIterations_(0),
          useMultinomialCovariance_(i_useMultinomialCovariance),
          smoothLast_(i_smoothLastIteration),
          v1_(responseMatrix.nColumns()),
          v3_(v1_.size()),
          yhatBuf_(std::max(responseMatrix.nRows(), responseMatrix.nColumns()))
    {
        setConvergenceEpsilon(i_convergenceEpsilon);
        setFilter(&f);
        useConvolutions(i_useConvolutions);
    }

    void SmoothedEMUnfold1D::setConvergenceEpsilon(const double eps)
    {
        convergenceEpsilon_ = eps;
        if (convergenceEpsilon_ < 0.0)
            convergenceEpsilon_ = 0.0;
    }

    bool SmoothedEMUnfold1D::unfold(
        const double* observed, const unsigned lenObserved,
        const Matrix<double>* observationCovarianceMatrix,
        double* unfolded, const unsigned lenUnfolded,
        DMat* unfoldedCovarianceFunction)
    {
        validateDimensions(lenObserved, lenUnfolded);
        validateDensity(observed, lenObserved);
        if (observationCovarianceMatrix)
        {
            if (observationCovarianceMatrix->nRows() != lenObserved ||
                observationCovarianceMatrix->nColumns() != lenObserved)
                throw std::invalid_argument(
                    "In npstat::SmoothedEMUnfold1D::unfold: "
                    "incompatible dimensions for the covariance "
                    "matrix of observations");
        }
        assert(unfolded);

        lastNormfactor_ = 1.0;

        // Set up the "previous" approximation
        double* prev = &v1_[0];

        // Set the "next" approximation to the initial one
        if (getInitialApproximation().empty())
            buildUniformInitialApproximation(observed, lenObserved, &v2_);
        else
            v2_ = getInitialApproximation();
        double* next = &v2_[0];

        // Run advanced iterations (e.g., with sharpening filter bandwidth)
        lastIterations_ = preIterate(observed, lenObserved,
                                     &prev, &next, lenUnfolded);

        // Perform expectation-maximization (a.k.a. D'Agostini) iterations
        // until convergence
        bool converged = false;
        for (; lastIterations_<maxIterations_ && !converged; ++lastIterations_)
        {
            std::swap(prev, next);
            update(observed, lenObserved, prev, next, lenUnfolded, true);
            converged = probDelta(prev, next, lenUnfolded)<=convergenceEpsilon_;
        }

        // One more cycle in case we do not want to smooth the last iteration
        if (!smoothLast_)
        {
            std::swap(prev, next);
            update(observed, lenObserved, prev, next, lenUnfolded, false);
        }

        // Fill out the unfolded result
        for (unsigned i=0; i<lenUnfolded; ++i)
            unfolded[i] = next[i];

        if (unfoldedCovarianceFunction)
        {
            // We will also need to calculate the result covariance matrix
            if (unfoldedCovarianceFunction->nRows() != lenUnfolded ||
                unfoldedCovarianceFunction->nColumns() != lenUnfolded)
                unfoldedCovarianceFunction->uninitialize();

            // Estimate yhat for the smoothed spectrum. This yhat
            // has to be used to get the the error propagation matrix.
            const double *smoothed = smoothLast_ ? next : prev;
            const DMat& K = responseMatrix();
            double* yhat = &yhatBuf_[0];
            K.timesVector(smoothed, lenUnfolded, yhat, lenObserved);

            // Calculate the error propagation matrix
            bool conv = false;
            const DMat& J = errorPropagationMatrix(
                observed, lenObserved, smoothed, lenUnfolded, yhat,
                lastNormfactor_, smoothLast_, maxIterations_,
                convergenceEpsilon_, &lastEPIterations_, &conv);
            converged = converged && conv;

            if (observationCovarianceMatrix)
            {
                // Assume that the user knows what (s)he is doing...
                *unfoldedCovarianceFunction = 
                    J*(*observationCovarianceMatrix)*J.T();
            }
            else
            {
                // To construct the covariance matrix for the
                // observations, we need the final yhat. If we
                // had an "unsmoothed" last iteration, this yhat
                // is different from the one calculated previously.
                if (!smoothLast_)
                    K.timesVector(next, lenUnfolded, yhat, lenObserved);
                const DMat& mcov = observationCovariance(
                    yhat, lenObserved, useMultinomialCovariance_);
                *unfoldedCovarianceFunction = J*mcov*J.T();
            }
        }

        return converged;
    }

    void SmoothedEMUnfold1D::update(
        const double* observed, const unsigned lenObserved,
        const double* prev, double* next, const unsigned lenUnfolded,
        const bool performSmoothing) const
    {
        double* yhat = &yhatBuf_[0];

        // Perform one expectation-maximization iteration
        const DMat& K = responseMatrix();
        K.timesVector(prev, lenUnfolded, yhat, lenObserved);

        for (unsigned iobs=0; iobs<lenObserved; ++iobs)
        {
            if (observed[iobs] > 0.0)
            {
                if (yhat[iobs] <= 0.0)
                {
                    std::ostringstream os;
                    os << "In npstat::SmoothedEMUnfold1D::update: "
                       << yhat[iobs] << " entries predicted, "
                       << observed[iobs] << " observed for bin " << iobs
                       << ". You need to change something (the response matrix,"
                       << " the initial approximation, or the filter).";
                    throw std::runtime_error(os.str());
                }
                else
                    yhat[iobs] = observed[iobs]/yhat[iobs];
            }
            else
                yhat[iobs] = 0.0;
        }

        double* num = &v3_[0];
        K.rowMultiply(yhat, lenObserved, num, lenUnfolded);

        const double* eff = &efficiency()[0];
        double* out = performSmoothing ? yhat : next;
        for (unsigned iunf=0; iunf<lenUnfolded; ++iunf)
            out[iunf] = prev[iunf]*num[iunf]/eff[iunf];

        if (performSmoothing)
        {
            // Apply the smoothing procedure
            if (usingConvolutions())
                getFilter(true)->convolve(out, lenUnfolded, next);
            else
                getFilter(true)->filter(out, lenUnfolded, next);

            // Make sure that the smoothing result is non-negative
            // and restore normalization
            const double sum = std::accumulate(out, out+lenUnfolded, 0.0L);
            normalizeArrayAsDensity(next, lenUnfolded,
                                    1.0/sum, &lastNormfactor_);
        }
    }

    DMat SmoothedEMUnfold1D::unfoldingMatrix0(const double* unfolded,
                                              const double* yhat) const
    {
        assert(unfolded);
        assert(yhat);

        DMat umat(transposedResponseMatrix());
        const unsigned nRows = umat.nRows();
        const unsigned nCols = umat.nColumns();
        const double* eff = &(efficiency()[0]);
        for (unsigned row=0; row<nRows; ++row)
        {
            double* rowData = umat[row];
            const double factor = unfolded[row]/eff[row];
            if (factor > 0.0)
                for (unsigned col=0; col<nCols; ++col)
                    rowData[col] *= factor/yhat[col];
            else
                for (unsigned col=0; col<nCols; ++col)
                    rowData[col] = 0.0;
        }
        return umat;
    }

    DMat SmoothedEMUnfold1D::errorPropagationMatrix(
        const double* observed, const unsigned lenObserved,
        const double* unfolded, const unsigned lenUnfolded,
        const double* yHat, const double norm, const bool smoothLast,
        const unsigned maxiter, const double convergenceEps,
        unsigned* itersMade, bool* convergedPtr) const
    {
        assert(observed);
        assert(unfolded);
        assert(yHat);
        assert(itersMade);
        assert(convergedPtr);

        const DMat& K = responseMatrix();
        assert(K.nColumns() == lenUnfolded);
        assert(K.nRows() == lenObserved);

        DMat S(usingConvolutions() ?
               getFilter(true)->getFilterMatrix().T() :
               getFilter(true)->getFilterMatrix());
        assert(S.isSquare());
        assert(S.nColumns() == lenUnfolded);

        S *= norm;

        // The following code block takes into account dependence
        // of the smoothing normalization factor on yhat
        {
            // Note that column sums are calculated for S
            // that is already multiplied by the normfactor
            std::vector<double> csums(lenUnfolded);
            for (unsigned i=0; i<lenUnfolded; ++i)
                csums[i] = S.columnSum(i);
            const double uSum = std::accumulate(
                unfolded, unfolded+lenUnfolded, 0.0L);
            for (unsigned irow=0; irow<lenUnfolded; ++irow)
            {
                const double frac = unfolded[irow]/uSum;
                double* s = S[irow];
                for (unsigned col=0; col<lenUnfolded; ++col)
                    s[col] += (1.0 - csums[col])*frac;
            }
        }

        const DMat& m0 = unfoldingMatrix0(unfolded, yHat);
        const double* eff = &(efficiency()[0]);

        DMat diagm(lenUnfolded, lenUnfolded, 0);
        std::vector<double> obsBuf(lenObserved);
        double* tmp = &obsBuf[0];
        for (unsigned iobs=0; iobs<lenObserved; ++iobs)
        {
            if (observed[iobs] > 0.0)
                tmp[iobs] = observed[iobs]/yHat[iobs];
            else
                tmp[iobs] = 0.0;
        }

        {
            const DMat& numMat = K.rowMultiply(tmp, lenObserved);
            const double* num = numMat.data();
            for (unsigned i=0; i<lenUnfolded; ++i)
                diagm[i][i] = num[i]/eff[i];
        }

        for (unsigned iobs=0; iobs<lenObserved; ++iobs)
            if (tmp[iobs] > 0.0)
                tmp[iobs] /= yHat[iobs];

        DMat mat(lenUnfolded, lenUnfolded);
        for (unsigned m=0; m<lenUnfolded; ++m)
        {
            const double factor = unfolded[m]/eff[m];
            for (unsigned r=0; r<lenUnfolded; ++r)
            {
                long double sum = 0.0L;
                for (unsigned i=0; i<lenObserved; ++i)
                    sum += K[i][m]*tmp[i]*K[i][r];
                mat[m][r] = factor*sum;
            }
        }

        const DMat& SM = S*m0;
        const DMat& SB = S*(diagm - mat);

        // Come up with an initial approximation to the solution
        DMat buf2;
        {
            const DMat& IMSB = DMat(lenUnfolded, lenUnfolded, 1) - SB;
            IMSB.solveLinearSystems(SM, &buf2);
        }

        // Prepare to refine the solution iteratively (iterations are
        // also needed in case "solveLinearSystems" call above fails)
        DMat* next = &buf2;
        double oldNorm = next->frobeniusNorm();
        DMat buf1(lenUnfolded, lenObserved);
        DMat* prev = &buf1;
        bool converged = false;

        // Refine the solution iteratively at least once
        double oldRatio = 0.0;
        unsigned iterCount = 0;
        for (; iterCount < maxiter && !converged; ++iterCount)
        {
            std::swap(prev, next);

            // *next = SM + SB * *prev;
            SB.times(*prev, next);
            *next += SM;

            // const double del = (*next - *prev).frobeniusNorm();
            double del = 0.0;
            {
                long double sum = 0.0L;
                const unsigned len = next->length();
                const double* nd = next->data();
                const double* pd = prev->data();
                for (unsigned il=0; il<len; ++il)
                {
                    const double d = *nd++ - *pd++;
                    sum += d*d;
                }
                del = sqrt(static_cast<double>(sum));
            }

            const double nn = next->frobeniusNorm();
            const double pn = oldNorm;
            oldNorm = nn;
            const double rat = del*2.0/(nn + pn);
            converged = rat <= convergenceEps;

            // Terminate the iterations if the convergence
            // does not seem to improve
            if (iterCount > 4U && !converged && rat >= oldRatio)
                break;
            oldRatio = rat;
        }

        *itersMade = iterCount;
        *convergedPtr = converged;

        if (smoothLast)
            return *next;
        else
            return m0 + (diagm - mat) * *next;
    }
}
