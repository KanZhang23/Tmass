#include <climits>
#include <stdexcept>
#include <algorithm>
#include <numeric>

#include "npstat/stat/SmoothedEMUnfoldND.hh"
#include "npstat/stat/StatUtils.hh"
#include "npstat/stat/AbsUnfold1D.hh"

static npstat::Matrix<double> unfoldingMatrix0(
    const npstat::Matrix<double>& K, const double* unfolded,
    const double* yhat, const double* eff)
{
    assert(unfolded);
    assert(yhat);
    assert(eff);

    npstat::Matrix<double> umat(K.T());
    const unsigned nRows = umat.nRows();
    const unsigned nCols = umat.nColumns();
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

namespace npstat {
    SmoothedEMUnfoldND::SmoothedEMUnfoldND(
        const ResponseMatrix& i_responseMatrix,
        const AbsUnfoldingFilterND& f,
        const bool i_useConvolutions,
        const bool i_useMultinomialCovariance,
        const bool i_smoothLastIteration,
        const double i_convergenceEpsilon,
        const unsigned i_maxIterations)
        : AbsUnfoldND(i_responseMatrix),
          convergenceEpsilon_(0.0),
          lastNormfactor_(1.0),
          maxIterations_(i_maxIterations),
          lastIterations_(0),
          lastEPIterations_(0),
          useMultinomialCovariance_(i_useMultinomialCovariance),
          smoothLast_(i_smoothLastIteration),
          v1_(i_responseMatrix.shapeData(), i_responseMatrix.rank())
    {
        setConvergenceEpsilon(i_convergenceEpsilon);
        setFilter(&f);
        useConvolutions(i_useConvolutions);
    }

    void SmoothedEMUnfoldND::setConvergenceEpsilon(const double eps)
    {
        convergenceEpsilon_ = eps;
        if (convergenceEpsilon_ < 0.0)
            convergenceEpsilon_ = 0.0;
    }

    bool SmoothedEMUnfoldND::unfold(const ArrayND<double>& observed,
                                    const Matrix<double>* observationCovMat,
                                    ArrayND<double>* unfolded,
                                    Matrix<double>* unfoldedCovMat)
    {
        validateObservedShape(observed);
        if (!observed.isDensity()) throw std::invalid_argument(
            "In npstat::SmoothedEMUnfoldND::unfold: "
            "array of observations is not a valid probability density");

        const unsigned long lenObserved = observed.length();
        if (observationCovMat)
        {
            const unsigned nrows = observationCovMat->nRows();
            const unsigned ncols = observationCovMat->nColumns();
            if (nrows != lenObserved || ncols != lenObserved)
                throw std::invalid_argument(
                    "In npstat::SmoothedEMUnfoldND::unfold: "
                    "incompatible dimensions for the covariance "
                    "matrix of observations");
            if (nrows*1UL*ncols > UINT_MAX) throw std::invalid_argument(
                "In npstat::SmoothedEMUnfoldND::unfold: "
                "covariance matrix of observations is too large");
        }

        assert(unfolded);
        const ResponseMatrix& K = responseMatrix();
        unfolded->reshape(K.shapeData(), K.rank());

        lastNormfactor_ = 1.0;

        // Set up the "previous" approximation
        ArrayND<double>* prev = &v1_;

        // Set the "next" approximation to the initial one
        if (getInitialApproximation().isShapeKnown())
            v2_ = getInitialApproximation();
        else
            buildUniformInitialApproximation(observed, &v2_);
        ArrayND<double>* next = &v2_;

        // Perform expectation-maximization (a.k.a. D'Agostini) iterations
        // until convergence
        lastIterations_ = 0U;
        bool converged = false;
        for (; lastIterations_<maxIterations_ && !converged; ++lastIterations_)
        {
            std::swap(prev, next);
            update(observed, prev, next, true);
            converged = probDelta(*prev, *next) <= convergenceEpsilon_;
        }

        // One more cycle in case we do not want to smooth the last iteration
        if (!smoothLast_)
        {
            std::swap(prev, next);
            update(observed, prev, next, false);
        }

        // Fill out the unfolded result
        *unfolded = *next;

        if (unfoldedCovMat)
        {
            // We need to calculate the covariance matrix
            // of the unfolded result
            const unsigned long lenUnfolded = unfolded->length();
            if (unfoldedCovMat->nRows() != lenUnfolded ||
                unfoldedCovMat->nColumns() != lenUnfolded)
                unfoldedCovMat->uninitialize();

            // yhat for the smoothed spectrum
            const ArrayND<double>& smoothed = smoothLast_ ? *next : *prev;
            K.timesVector(smoothed, &yhatBuf_);
            assert(yhatBuf_.length() == lenObserved);

            // Calculate the error propagation matrix
            // using the smoothed spectrum yhat
            bool conv = false;
            const Matrix<double>& J = errorPropagationMatrix(
                observed.data(), lenObserved, smoothed.data(), lenUnfolded,
                yhatBuf_.data(), lastNormfactor_, smoothLast_, maxIterations_,
                convergenceEpsilon_, &lastEPIterations_, &conv);
            converged = converged && conv;

            if (observationCovMat)
                *unfoldedCovMat = J*(*observationCovMat)*J.T();
            else
            {
                // To construct the covariance matrix for the
                // observations, we need the final yhat. If we
                // had an "unsmoothed" last iteration, this yhat
                // is different from the one calculated previously.
                if (!smoothLast_)
                    K.timesVector(*next, &yhatBuf_);
                const Matrix<double>& mcov = AbsUnfold1D::observationCovariance(
                    yhatBuf_.data(), lenObserved, useMultinomialCovariance_);
                *unfoldedCovMat = J*mcov*J.T();
            }
        }

        return converged;
    }

    void SmoothedEMUnfoldND::update(
        const ArrayND<double>& observedArr,
        const ArrayND<double>* preva, ArrayND<double>* next,
        const bool performSmoothing) const
    {
        // Perform one expectation-maximization iteration
        const ResponseMatrix& K = responseMatrix();
        K.timesVector(*preva, &yhatBuf_);
        double* yhat = const_cast<double*>(yhatBuf_.data());

        const unsigned long lenObserved = observedArr.length();
        const double* observed = observedArr.data();
        for (unsigned long iobs=0; iobs<lenObserved; ++iobs)
        {
            if (observed[iobs] > 0.0)
            {
                if (yhat[iobs] <= 0.0)
                {
                    std::ostringstream os;
                    os << "In npstat::SmoothedEMUnfoldND::update: "
                       << yhat[iobs] << " entries predicted, "
                       << observed[iobs] << " observed for linear bin " << iobs
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

        const unsigned long lenUnfolded = preva->length();
        const double* eff = efficiency().data();
        const double* prev = preva->data();
        ArrayND<double>* ubuf = performSmoothing ? &unfoldedBuf_ : next;
        K.rowMultiply(yhatBuf_, ubuf);
        double* udata = const_cast<double*>(ubuf->data());
        for (unsigned long iunf=0; iunf<lenUnfolded; ++iunf)
            udata[iunf] *= (prev[iunf]/eff[iunf]);

        if (performSmoothing)
        {
            // Apply the smoothing procedure
            if (usingConvolutions())
                getFilter(true)->convolve(unfoldedBuf_, next);
            else
                getFilter(true)->filter(unfoldedBuf_, next);

            // Make sure that the smoothing result is non-negative
            // and restore normalization
            const long double sum = unfoldedBuf_.sum<long double>();
            normalizeArrayAsDensity(const_cast<double*>(next->data()),
                                    lenUnfolded, 1.0L/sum, &lastNormfactor_);
        }
    }

    Matrix<double> SmoothedEMUnfoldND::errorPropagationMatrix(
        const double* observed, const unsigned lenObserved,
        const double* unfolded, const unsigned lenUnfolded,
        const double* yHat, const double norm, const bool smoothLast,
        const unsigned maxiter, const double convergenceEps,
        unsigned* itersMade, bool* convergedPtr) const
    {
        typedef Matrix<double> DMat;

        assert(observed);
        assert(unfolded);
        assert(yHat);
        assert(itersMade);
        assert(convergedPtr);

        const DMat& K = responseMatrix().denseMatrix();
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

        const double* eff = efficiency().data();
        const DMat& m0 = unfoldingMatrix0(K, unfolded, yHat, eff);

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
