#include <cmath>
#include <sstream>

#include "npstat/stat/AbsUnfoldND.hh"

namespace npstat {
    AbsUnfoldND::AbsUnfoldND(const ResponseMatrix& responseMatrix)
        : responseMatrix_(responseMatrix),
          efficiency_(responseMatrix.shapeData(), responseMatrix.rank()),
          filt_(0),
          useConvolutions_(false)
    {
        if (!responseMatrix_.isValid()) throw std::invalid_argument(
            "In npstat::AbsUnfoldND constructor: response matrix is not valid");
        const unsigned long len = responseMatrix.length();
        for (unsigned long i=0; i<len; ++i)
        {
            const double eff = responseMatrix_.linearEfficiency(i);
            if (eff <= 0.0)
            {
                std::ostringstream os;
                os << "In npstat::AbsUnfoldND constructor: "
                   << "efficiency for linear index " << i
                   << " is not positive. Please remove this cell "
                   << "from the response matrix.";
                throw std::invalid_argument(os.str());
            }
            efficiency_.linearValue(i) = eff;
        }
    }

    void AbsUnfoldND::validateUnfoldedShape(const ArrayND<double>& a) const
    {
        if (!a.isShapeCompatible(responseMatrix_))
            throw std::invalid_argument(
                "In npstat::AbsUnfoldND::validateUnfoldedShape: "
                "incompatible shape of the argument array");
    }

    void AbsUnfoldND::validateObservedShape(const ArrayND<double>& a) const
    {
        if (!a.isCompatible(responseMatrix_.observedShape()))
            throw std::invalid_argument(
                "In npstat::AbsUnfoldND::validateObservedShape: "
                "incompatible shape of the argument array");
    }

    void AbsUnfoldND::validateUnfoldedShape(const ArrayShape& shape) const
    {
        if (!responseMatrix_.isCompatible(shape))
            throw std::invalid_argument(
                "In npstat::AbsUnfoldND::validateUnfoldedShape: "
                "incompatible argument shape");
    }

    void AbsUnfoldND::validateObservedShape(const ArrayShape& shape) const
    {
        if (shape != responseMatrix_.observedShape())
            throw std::invalid_argument(
                "In npstat::AbsUnfoldND::validateObservedShape: "
                "incompatible argument shape");
    }

    void AbsUnfoldND::setInitialApproximation(const ArrayND<double>& approx)
    {
        validateUnfoldedShape(approx);
        initialApproximation_ = approx;
    }

    void AbsUnfoldND::clearInitialApproximation()
    {
        initialApproximation_.uninitialize();
    }

    const ArrayND<double>& AbsUnfoldND::getInitialApproximation() const
    {
        return initialApproximation_;
    }

    void AbsUnfoldND::setFilter(const AbsUnfoldingFilterND* f)
    {
        if (f)
            validateUnfoldedShape(f->dataShape());
        filt_ = f;
    }

    const AbsUnfoldingFilterND* AbsUnfoldND::getFilter(
        const bool throwIfNull) const
    {
        if (!filt_ && throwIfNull) throw std::runtime_error(
            "In npstat::AbsUnfoldND::getFilter: filter has not been set");
        return filt_;
    }

    double AbsUnfoldND::probDelta(const ArrayND<double>& preva,
                                  const ArrayND<double>& nexta)
    {
        if (!preva.isShapeCompatible(nexta)) throw std::invalid_argument(
            "In npstat::AbsUnfoldND::probDelta: incompatible argument shapes");

        const unsigned long len = preva.length();
        const double* prev = preva.data();
        const double* next = nexta.data();
        long double del = 0.0L, sum = 0.0L;
        for (unsigned long i=0; i<len; ++i)
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

    void AbsUnfoldND::buildUniformInitialApproximation(
        const ArrayND<double>& observed, ArrayND<double>* result) const
    {
        validateObservedShape(observed);
        assert(result);
        result->reshape(responseMatrix_.shapeData(), responseMatrix_.rank());
        const long double effSum = efficiency_.sum<long double>();
        const long double eff = effSum/efficiency_.length();
        const long double observedSum = observed.sum<long double>();
        const long double unfoldedSum = observedSum/eff;
        const double u = unfoldedSum/result->length();
        result->constFill(u);
    }
}
