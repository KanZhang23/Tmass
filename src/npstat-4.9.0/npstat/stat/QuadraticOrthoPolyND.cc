#include <cmath>

#include "npstat/stat/QuadraticOrthoPolyND.hh"

static void subtract_in_place(double *d1, const double *d2,
                              const unsigned n, const double d2_factor)
{
    for (unsigned i=0; i<n; ++i)
        d1[i] -= d2_factor*d2[i];
}

static double scalar_product(const double *v1, const double *v2,
                             const long double *gramMat, const unsigned n)
{
    long double sum = 0.0L;
    for (unsigned i=0; i<n; ++i)
    {
        const long double *g = gramMat + i*n;
        long double local = 0.0L;
        for (unsigned j=0; j<n; ++j)
            local += g[j]*v2[j];
        sum += v1[i]*local;
    }
    return sum;
}

static void mult_lower_diag_by_vector(const double *m, const double *v,
                                      const unsigned ndim, double *result)
{
    for (unsigned idim=0; idim<ndim; ++idim)
    {
        const double *row = m + idim*ndim;
        long double r = 0.0L;
        for (unsigned idim2=0; idim2<=idim; ++idim2)
            r += row[idim2]*v[idim2];
        result[idim] = static_cast<double>(r);
    }
}

namespace npstat {
    QuadraticOrthoPolyND::QuadraticOrthoPolyND(
        const AbsDistributionND& weightI,
        const BoxND<double>& boundingBoxI,
        const unsigned* nStepsI, const unsigned nStepsDim)
        : weight_(weightI.clone()),
          box_(boundingBoxI),
          cellSize_(1.0L),
          dim_(weightI.dim())
    {
        // Check that the inputs make sense
        if (dim_ != boundingBoxI.dim())
            throw std::invalid_argument(
                "In npstat::QuadraticOrthoPolyND constructor: "
                "incompatible bounding box dimensionality");
        if (dim_ != nStepsDim)
            throw std::invalid_argument(
                "In npstat::QuadraticOrthoPolyND constructor: incompatible "
                "dimensionality for the numbers of integration intervals");
        assert(nStepsI);

        numSteps_.reserve(dim_);
        unsigned long npoints = 1UL;
        for (unsigned i=0; i<dim_; ++i)
        {
            const double len = boundingBoxI[i].length();
            if (len <= 0.0)
                throw std::invalid_argument(
                    "In npstat::QuadraticOrthoPolyND constructor: "
                    "bounding box must have positive length in each dimension");
            if (!nStepsI[i])
                throw std::invalid_argument(
                    "In npstat::QuadraticOrthoPolyND constructor: number of "
                    "integration intervals must be positive in each dimension");
            cellSize_ *= (len/nStepsI[i]);
            numSteps_.push_back(nStepsI[i]);
            npoints *= nStepsI[i];
        }

        // The number of monomials should be smaller than the number
        // of integration points. Otherwise we will get into trouble
        // during the orthogonalization process.
        const unsigned nmono = ((dim_+1)*(dim_+2))/2;
        if (nmono >= npoints)
            throw std::invalid_argument(
                "In npstat::QuadraticOrthoPolyND constructor: "
                "insufficient number of integration intervals");

        // Calculate the Gram matrix for the monomials
        buf_.resize(nmono);
        work_.resize(nmono);
        work_[0] = 1.0;
        sprod_.resize(nmono);
        const unsigned nmonosq = nmono*nmono;
        std::vector<long double> gram(nmonosq, 0.0L);
        integrationLoop(0U, &gram[0]);

        // The weights must sum up to a positive number
        assert(gram[0] > 0.0L);

        // Correct the Gram matrix values for the cell size
        for (unsigned i=0; i<nmonosq; ++i)
            gram[i] *= cellSize_;

        // Setup initial values of the monomial coefficients
        coeffM_.resize(nmonosq);
        double* coeffM = &coeffM_[0];
        for (unsigned i=0; i<nmonosq; ++i)
            coeffM[i] = 0.0;
        for (unsigned idim=1; idim<nmono; ++idim)
            coeffM[idim+idim*nmono] = 1.0;
        coeffM[0] = 1.0L/sqrtl(gram[0]);

        // Run the Gram-Schmidt procedure
        gramSchmidt(&gram[0], 1U);
    }

    void QuadraticOrthoPolyND::gramSchmidt(const long double *gram,
                                           const unsigned startingMono)
    {
        const unsigned nmono = ((dim_+1)*(dim_+2))/2;
        for (unsigned imono=startingMono; imono<nmono; ++imono)
        {
            double *polyCoeffs = &coeffM_[0] + imono*nmono;
            for (unsigned i=0; i<imono; ++i)
            {
                const double *coeffsI = &coeffM_[0] + i*nmono;
                double norm = scalar_product(polyCoeffs, coeffsI, gram, nmono);
                subtract_in_place(polyCoeffs, coeffsI, i+1, norm);
            }
            double norm = scalar_product(polyCoeffs, polyCoeffs, gram, nmono);
            assert(norm > 0.0);
            norm = sqrt(norm);
            for (unsigned i=0; i<=imono; ++i)
                polyCoeffs[i] /= norm;
        }
    }

    void QuadraticOrthoPolyND::integrationLoop(
        const unsigned level, long double *gram)
    {
        const double xmin = box_[level].min();
        const unsigned nIntervals = numSteps_[level];
        const double step = box_[level].length()/nIntervals;
        double* mono = &work_[0];
        double* monop1 = mono + 1;

        if (level == dim_ - 1U)
        {
            // This is the last level
            const unsigned nmono = ((dim_+1)*(dim_+2))/2;
            for (unsigned i=0; i<nIntervals; ++i)
            {
                monop1[level] = xmin + (i + 0.5)*step;
                const double w = weight_->density(monop1, dim_);

                // The weight can not be negative
                if (w < 0.0)
                    throw std::invalid_argument(
                        "In npstat::QuadraticOrthoPolyND::integrationLoop: "
                        "the weight function must be non-negative");

                // Generate quadratic monomials
                unsigned imono = dim_+1;
                for (unsigned idim = 0; idim < dim_; ++idim)
                    for (unsigned idim2 = idim; idim2 < dim_; ++idim2)
                        mono[imono++] = monop1[idim]*monop1[idim2];

                // Add this point to the gram matrix accumulator
                for (unsigned idim = 0; idim < nmono; ++idim)
                {
                    long double *g = gram + idim*nmono;
                    for (unsigned idim2 = 0; idim2 < nmono; ++idim2)
                        g[idim2] += mono[idim]*mono[idim2]*w;
                }
            }
        }
        else
        {
            for (unsigned i=0; i<nIntervals; ++i)
            {
                monop1[level] = xmin + (i + 0.5)*step;
                integrationLoop(level+1U, gram);
            }
        }
    }

    QuadraticOrthoPolyND::~QuadraticOrthoPolyND()
    {
        delete weight_;
    }

    QuadraticOrthoPolyND::QuadraticOrthoPolyND(const QuadraticOrthoPolyND& r)
        : weight_(r.weight_->clone()),
          box_(r.box_),
          numSteps_(r.numSteps_),
          coeffM_(r.coeffM_),
          work_(r.work_),
          buf_(r.buf_),
          sprod_(r.sprod_),
          cellSize_(r.cellSize_),
          dim_(r.dim_)
    {
    }

    QuadraticOrthoPolyND& QuadraticOrthoPolyND::operator=(
        const QuadraticOrthoPolyND& r)
    {
        if (this != &r)
        {
            delete weight_;
            weight_ = r.weight_->clone();
            box_ = r.box_;
            numSteps_ = r.numSteps_;
            coeffM_ = r.coeffM_;
            work_ = r.work_;
            buf_ = r.buf_;
            sprod_ = r.sprod_;
            cellSize_ = r.cellSize_;
            dim_ = r.dim_;
        }
        return *this;
    }

    double QuadraticOrthoPolyND::value(
        const unsigned polyNumber, const double* x, const unsigned lenX) const
    {
        const unsigned nmono = ((dim_+1)*(dim_+2))/2;
        if (polyNumber >= nmono)
            throw std::out_of_range(
                "In npstat::QuadraticOrthoPolyND::value: "
                "polynomial number is out of range");
        if (lenX != dim_)
            throw std::invalid_argument(
                "In npstat::QuadraticOrthoPolyND::value: "
                "incompatible point dimensionality");
        assert(x);

        const double *coeffs = &coeffM_[0] + polyNumber*nmono;
        if (polyNumber == 0U)
            return coeffs[0];

        // Make up the linear monomials
        double *mono = &work_[0];
        unsigned imono = 0U;
        mono[imono++] = 1.0;
        for (unsigned idim = 0; idim < dim_; ++idim)
            mono[imono++] = x[idim];

        long double sum = 0.0L;
        if (polyNumber <= dim_)
        {
            // Linear term is requested
            for (imono = 0U; imono <= polyNumber; ++imono)
                sum += coeffs[imono]*mono[imono];
            return static_cast<double>(sum);
        }

        // Make up the quadratic monomials
        double *monop1 = mono + 1;
        for (unsigned idim = 0; idim < dim_; ++idim)
            for (unsigned idim2 = idim; idim2 < dim_; ++idim2)
                mono[imono++] = monop1[idim]*monop1[idim2];
        
        /* Make the polynomial */
        for (imono = 0U; imono <= polyNumber; ++imono)
            sum += coeffs[imono]*mono[imono];
        return static_cast<double>(sum);
    }

    double QuadraticOrthoPolyND::series(
        const double* x, const unsigned lenX,
        const double* coeffs, const unsigned nCoeffs,
        double* individualPolynomials) const
    {
        const unsigned nmono = ((dim_+1)*(dim_+2))/2;
        if (lenX != dim_)
            throw std::invalid_argument(
                "In npstat::QuadraticOrthoPolyND::series: "
                "incompatible point dimensionality");
        if (!(nCoeffs == 1U || nCoeffs == dim_ + 1U || nCoeffs == nmono))
            throw std::invalid_argument(
                "In npstat::QuadraticOrthoPolyND::series: "
                "incompatible number of polynomial coefficients");
        assert(x);
        assert(coeffs);

        if (nCoeffs == 1U)
        {
            if (individualPolynomials)
                individualPolynomials[0] = coeffM_[0];
            return coeffM_[0]*coeffs[0];
        }

        // Generate linear monomials
        double *mono = &work_[0];
        unsigned imono = 0U;
        mono[imono++] = 1.0;
        for (unsigned idim = 0; idim < dim_; ++idim)
            mono[imono++] = x[idim];
        
        if (nCoeffs > dim_ + 1U)
        {
            // Generate quadratic monomials
            double *monop1 = mono + 1;
            for (unsigned idim = 0; idim < dim_; ++idim)
                for (unsigned idim2 = idim; idim2 < dim_; ++idim2)
                    mono[imono++] = monop1[idim]*monop1[idim2];
        }

        // Calculate the total result
        long double totalSum = 0.0L;
        for (unsigned polyNumber=0; polyNumber<nCoeffs; ++polyNumber)
        {
            const double *coeffm = &coeffM_[0] + polyNumber*nmono;
            long double sum = 0.0L;
            for (unsigned imono = 0U; imono <= polyNumber; ++imono)
                sum += coeffm[imono]*mono[imono];
            totalSum += coeffs[polyNumber]*sum;
            if (individualPolynomials)
                individualPolynomials[polyNumber] = static_cast<double>(sum);
        }
        return static_cast<double>(totalSum);
    }

    void QuadraticOrthoPolyND::gradient(
        const double* x, const unsigned lenX,
        const double* coeffs, const unsigned nCoeffs,
        double* result, const unsigned lenResult) const
    {
        const unsigned nmono = ((dim_+1)*(dim_+2))/2;
        if (lenX != dim_)
            throw std::invalid_argument(
                "In npstat::QuadraticOrthoPolyND::gradient: "
                "incompatible point dimensionality");
        if (lenResult < dim_)
            throw std::invalid_argument(
                "In npstat::QuadraticOrthoPolyND::gradient: "
                "insufficient length of the result buffer");
        assert(x);
        assert(coeffs);
        assert(result);

        if (nCoeffs == 1U)
        {
            for (unsigned i=0; i<dim_; ++i)
                result[i] = 0.0;
        }
        else if (nCoeffs == dim_ + 1U)
        {
            for (unsigned graddim=0; graddim<dim_; ++graddim)
            {
                long double totalSum = 0.0L;
                for (unsigned polyNumber=1; polyNumber<nCoeffs; ++polyNumber)
                    totalSum += coeffs[polyNumber]*
                        coeffM_[polyNumber*nmono + graddim + 1U];
                result[graddim] = static_cast<double>(totalSum);
            }
        }
        else if (nCoeffs == nmono)
        {        
            double *mono = &work_[0];
            double *monop1 = mono + 1;
            double* pval = &buf_[0];

            for (unsigned graddim = 0; graddim < dim_; ++graddim)
            {
                long double sum = 0.0L;
                unsigned imono = 0U;

                /* Make up the monomial derivatives */
                mono[imono++] = 1.0;
                for (unsigned idim = 0; idim < dim_; ++idim)
                    mono[imono++] = x[idim];
                for (unsigned idim = 0; idim < dim_; ++idim)
                    for (unsigned idim2 = idim; idim2 < dim_; ++idim2)
                    {   
                        if (idim == graddim && idim2 == graddim)
                            mono[imono++] = 2.0*monop1[idim];
                        else if (idim == graddim)
                            mono[imono++] = monop1[idim2];
                        else if (idim2 == graddim)
                            mono[imono++] = monop1[idim];
                        else
                            mono[imono++] = 0.0;
                    }

                for (unsigned idim = 0; idim <= dim_; ++idim)
                    mono[idim] = 0.0;
                monop1[graddim] = 1.0;

                /* Make up the polynomials */
                mult_lower_diag_by_vector(&coeffM_[0], mono, nmono, pval);

                /* Calculate the series */
                for (unsigned imono = 0; imono < nmono; ++imono)
                    sum += coeffs[imono]*pval[imono];

                result[graddim] = static_cast<double>(sum);
            }
        }
        else
        {
            throw std::invalid_argument(
                "In npstat::QuadraticOrthoPolyND::gradient: "
                "incompatible number of polynomial coefficients");
        }
    }

    void QuadraticOrthoPolyND::hessian(
        const double* coeffs, const unsigned nCoeffs,
        double* result, const unsigned resultBufferLen) const
    {
        const unsigned nmono = ((dim_+1)*(dim_+2))/2;
        if (!(nCoeffs == 1U || nCoeffs == dim_ + 1U || nCoeffs == nmono))
            throw std::invalid_argument(
                "In npstat::QuadraticOrthoPolyND::hessian: "
                "incompatible number of polynomial coefficients");
        assert(coeffs);
        const unsigned lenResult = (dim_*(dim_+1U))/2U;
        if (resultBufferLen < lenResult)
            throw std::invalid_argument(
                "In npstat::QuadraticOrthoPolyND::hessian: "
                "insufficient length of the result buffer");
        assert(result);

        if (nCoeffs < nmono)
        {
            for (unsigned i=0; i<lenResult; ++i)
                result[i] = 0.0;
            return;
        }

        double *mono = &work_[0];
        for (unsigned imono=0; imono<nmono; ++imono)
            mono[imono] = 0.0;

        int ihess = 0; // Should not be unsigned -- we subtract 1 in the cycle
        double *monopn1 = mono + (1U + dim_);
        double* pval = &buf_[0];
        for (unsigned idim = 0; idim < dim_; ++idim)
            for (unsigned idim2 = idim; idim2 < dim_; ++idim2)
            {
                long double sum = 0.0L;

                monopn1[ihess - 1] = 0.0;
                if (idim2 == idim)
                    monopn1[ihess] = 2.0;
                else
                    monopn1[ihess] = 1.0;

                mult_lower_diag_by_vector(&coeffM_[0], mono, nmono, pval);
                for (unsigned imono = 0; imono < nmono; ++imono)
                    sum += coeffs[imono]*pval[imono];

                result[ihess++] = static_cast<double>(sum);
            }
    }

    double QuadraticOrthoPolyND::monomialCoefficient(
        const unsigned iPoly, const unsigned iMono) const
    {
        const unsigned nmono = ((dim_+1)*(dim_+2))/2;
        if (!(iPoly < nmono && iMono < nmono))
            throw std::out_of_range(
                "In npstat::QuadraticOrthoPolyND::monomialCoefficient: "
                "index out of range");
        return coeffM_[iPoly*nmono + iMono];
    }
}
