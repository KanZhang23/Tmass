#include <cmath>
#include <cfloat>
#include <stdexcept>

#include "PseudoMCUncertaintyCalculator.hh"

namespace mcuncert {
    PseudoMCUncertaintyCalculator::PseudoMCUncertaintyCalculator()
    {
        clear();
    }

    void PseudoMCUncertaintyCalculator::clear()
    {
        sum_ = 0.0L;
        sumsq_ = 0.0L;
        runningMean_ = 0.0L;
        min_ = DBL_MAX;
        max_ = -DBL_MAX;
        count_ = 0;
        nextRecenter_ = 0;
    }

    void PseudoMCUncertaintyCalculator::addPoint(const long double functionValue)
    {
        const long double ldv = functionValue - runningMean_;
        sum_ += ldv;
        sumsq_ += ldv*ldv;
        if (functionValue < min_)
            min_ = functionValue;
        if (functionValue > max_)
            max_ = functionValue;
        if (++count_ >= nextRecenter_)
            recenter();
    }

    void PseudoMCUncertaintyCalculator::recenter()
    {
        if (count_)
        {
            const long double m = sum_/count_;
            sumsq_ -= m*sum_;
            if (sumsq_ < 0.0L)
                sumsq_ = 0.0L;
            runningMean_ += m;
            sum_ = 0.0L;
            nextRecenter_ = count_*2UL;
        }
    }

    unsigned long PseudoMCUncertaintyCalculator::count() const
    {
        return count_;
    }

    long double PseudoMCUncertaintyCalculator::mean() const
    {
        if (!count_) throw std::runtime_error(
            "In mcuncert::PseudoMCUncertaintyCalculator::mean: "
            "no data accumulated");
        return sum_/count_ + runningMean_;
    }

    long double PseudoMCUncertaintyCalculator::meanUncertainty() const
    {
        if (!count_) throw std::runtime_error(
            "In npstat::PseudoMCUncertaintyCalculator::meanUncertainty: "
            "no data accumulated");
        if (count_ == 1UL)
            return 0.0;
        else
        {
            const long double m = sum_/count_;
            long double sumsq = sumsq_ - m*sum_;
            if (sumsq < 0.0L)
                sumsq = 0.0L;
            return sqrtl(sumsq/(count_ - 1UL)/count_);
        }
    }

    long double PseudoMCUncertaintyCalculator::min() const
    {
        return min_;
    }

    long double PseudoMCUncertaintyCalculator::max() const
    {
        return max_;
    }

    long double PseudoMCUncertaintyCalculator::sum() const
    {
        return sum_ + runningMean_*count_;
    }

    long double PseudoMCUncertaintyCalculator::sumsq() const
    {
        long double s = sumsq_ + runningMean_*(runningMean_*count_ + 2.0*sum_);
        if (s < 0.0L)
            s = 0.0L;
        return s;
    }

    void PseudoMCUncertaintyCalculator::print() const
    {

    }

    bool PseudoMCUncertaintyCalculator::nec_cond_fail() const
    {
        return false;
    }
}
