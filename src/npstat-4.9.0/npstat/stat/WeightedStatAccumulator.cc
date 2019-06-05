#include <cmath>
#include <cfloat>
#include <cassert>
#include <stdexcept>

#include "geners/binaryIO.hh"
#include "geners/IOException.hh"

#include "npstat/stat/WeightedStatAccumulator.hh"

namespace npstat {
    void WeightedStatAccumulator::reset()
    {
        sum_ = 0.0L;
        sumsq_ = 0.0L;
        wsum_ = 0.0L;
        wsumsq_ = 0.0L;
        runningMean_ = 0.0L;
        min_ = DBL_MAX;
        max_ = -DBL_MAX;
        maxWeight_ = 0.0;
        cnt_ = 0UL;
        callCount_ = 0UL;
        nextRecenter_ = 0UL;
    }

    WeightedStatAccumulator::WeightedStatAccumulator()
    {
        reset();
    }

    inline void WeightedStatAccumulator::recenter()
    {
        if (cnt_)
        {
            const long double m = sum_/wsum_;
            sumsq_ -= m*m*wsum_;
            if (sumsq_ < 0.0L)
                sumsq_ = 0.0L;
            runningMean_ += m;
            sum_ = 0.0L;
            nextRecenter_ = cnt_*2UL;
        }
    }

    inline void WeightedStatAccumulator::recenter(const long double newCenter)
    {
        recenter();
        const long double dr = runningMean_ - newCenter;
        sumsq_ += dr*dr*wsum_;
        sum_ = dr*wsum_;
        runningMean_ = newCenter;
    }

    void WeightedStatAccumulator::accumulate(const double value,
                                             const double w)
    {
        if (w < 0.0) throw std::invalid_argument(
            "In npstat::WeightedStatAccumulator::accumulate: "
            "weight must be non-negative");
        ++callCount_;
        if (w > 0.0)
        {
            const long double ldv = value - runningMean_;
            const long double longw = w;
            sum_ += ldv*longw;
            sumsq_ += ldv*ldv*longw;
            wsum_ += longw;
            wsumsq_ += longw*longw;
            if (value < min_)
                min_ = value;
            if (value > max_)
                max_ = value;
            if (w > maxWeight_)
                maxWeight_ = w;
            if (++cnt_ >= nextRecenter_)
                recenter();
        }
    }

    void WeightedStatAccumulator::accumulate(const WeightedStatAccumulator& acc)
    {
        callCount_ += acc.callCount_;
        if (acc.cnt_)
        {
            WeightedStatAccumulator a(acc);
            a.recenter(runningMean_);
            sum_ += a.sum_;
            sumsq_ += a.sumsq_;
            wsum_ += a.wsum_;
            wsumsq_ += a.wsumsq_;
            if (a.min_ < min_)
                min_ = a.min_;
            if (a.max_ > max_)
                max_ = a.max_;
            if (a.maxWeight_ > maxWeight_)
                maxWeight_ = a.maxWeight_;
            cnt_ += a.cnt_;
            if (cnt_ >= nextRecenter_)
                recenter();
        }
    }

    void WeightedStatAccumulator::accumulate(const WeightedStatAccumulator& acc,
                                             const double weight)
    {
        if (weight < 0.0) throw std::invalid_argument(
            "In npstat::WeightedStatAccumulator::accumulate: "
            "weight must be non-negative");
        callCount_ += acc.callCount_;
        if (weight && acc.cnt_)
        {
            WeightedStatAccumulator a(acc);
            a.recenter(runningMean_);
            sum_ += a.sum_*weight;
            sumsq_ += a.sumsq_*weight;
            wsum_ += a.wsum_*weight;
            wsumsq_ += a.wsumsq_*weight*weight;
            if (a.min_ < min_)
                min_ = a.min_;
            if (a.max_ > max_)
                max_ = a.max_;
            if (a.maxWeight_*weight > maxWeight_)
                maxWeight_ = a.maxWeight_*weight;
            cnt_ += a.cnt_;
            if (cnt_ >= nextRecenter_)
                recenter();
        }
    }

    double WeightedStatAccumulator::mean() const
    {
        if (!cnt_) throw std::runtime_error(
            "In npstat::WeightedStatAccumulator::mean: no data accumulated");
        return sum_/wsum_ + runningMean_;
    }

    double WeightedStatAccumulator::stdev() const
    {
        if (!cnt_) throw std::runtime_error(
            "In npstat::WeightedStatAccumulator::stdev: no data accumulated");
        if (cnt_ == 1UL)
            return 0.0;
        const long double effco = wsum_*wsum_/wsumsq_;
        if (effco <= 1.0L)
            return 0.0;
        (const_cast<WeightedStatAccumulator*>(this))->recenter();
        return sqrtl(sumsq_/wsum_*effco/(effco - 1.0L));
    }

    double WeightedStatAccumulator::meanUncertainty() const
    {
        if (!cnt_) throw std::runtime_error(
            "In npstat::WeightedStatAccumulator::meanUncertainty: "
            "no data accumulated");
        if (cnt_ == 1UL)
            return 0.0;
        const long double effco = wsum_*wsum_/wsumsq_;
        if (effco <= 1.0L)
            return 0.0;
        (const_cast<WeightedStatAccumulator*>(this))->recenter();
        return sqrtl(sumsq_/wsum_/(effco - 1.0L));
    }

    double WeightedStatAccumulator::noThrowMean(const double valueIfNoData) const
    {
        if (cnt_)
            return mean();
        else
            return valueIfNoData;
    }

    double WeightedStatAccumulator::noThrowStdev(const double valueIfNoData) const
    {
        if (cnt_)
            return stdev();
        else
            return valueIfNoData;
    }

    double WeightedStatAccumulator::noThrowMeanUncertainty(const double valueIfNoData) const
    {
        if (cnt_)
            return meanUncertainty();
        else
            return valueIfNoData;
    }

    double WeightedStatAccumulator::count() const
    {
        if (cnt_)
            return wsum_*wsum_/wsumsq_;
        else
            return 0.0;
    }

    double WeightedStatAccumulator::averageWeight() const
    {
        if (!cnt_) throw std::runtime_error(
            "In npstat::WeightedStatAccumulator::averageWeight: "
            "no data accumulated");
        return wsum_/cnt_;
    }

    double WeightedStatAccumulator::sumOfWeights() const
    {
        return static_cast<double>(wsum_);
    }

    bool WeightedStatAccumulator::operator==(
        const WeightedStatAccumulator& r) const
    {
        if (!(wsum_ == r.wsum_ &&
              wsumsq_ == r.wsumsq_ &&
              min_ == r.min_ &&
              max_ == r.max_ &&
              maxWeight_ == r.maxWeight_ &&
              cnt_ == r.cnt_ &&
              callCount_ == r.callCount_))
            return false;
        if (cnt_ == 0UL)
            return true;
        (const_cast<WeightedStatAccumulator*>(this))->recenter();
        (const_cast<WeightedStatAccumulator&>(r)).recenter();
        return sumsq_ == r.sumsq_ && runningMean_ == r.runningMean_;
    }

    bool WeightedStatAccumulator::write(std::ostream& os) const
    {
        (const_cast<WeightedStatAccumulator*>(this))->recenter();
        gs::write_pod(os, runningMean_);
        gs::write_pod(os, sumsq_);
        gs::write_pod(os, wsum_);
        gs::write_pod(os, wsumsq_);
        gs::write_pod(os, min_);
        gs::write_pod(os, max_);
        gs::write_pod(os, maxWeight_);
        gs::write_pod(os, cnt_);
        gs::write_pod(os, callCount_);
        return !os.fail();
    }

    WeightedStatAccumulator& WeightedStatAccumulator::scaleWeights(
        const double r)
    {
        if (r < 0.0) throw std::invalid_argument(
            "In npstat::WeightedStatAccumulator::scaleWeights: "
            "weight scale factor must be non-negative");
        if (r > 0.0)
        {
            const long double longr = r;
            sum_ *= longr;
            sumsq_ *= longr;
            wsum_ *= longr;
            wsumsq_ *= longr*longr;
            maxWeight_ *= r;
        }
        else
        {
            const unsigned long c = callCount_;
            reset();
            callCount_ = c;
        }

        return *this;
    }

    void WeightedStatAccumulator::restore(const gs::ClassId& id, std::istream& in,
                                          WeightedStatAccumulator* acc)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<WeightedStatAccumulator>());
        current.ensureSameId(id);

        assert(acc);
        acc->sum_ = 0.0L;
        gs::read_pod(in, &acc->runningMean_);
        gs::read_pod(in, &acc->sumsq_);
        gs::read_pod(in, &acc->wsum_);
        gs::read_pod(in, &acc->wsumsq_);
        gs::read_pod(in, &acc->min_);
        gs::read_pod(in, &acc->max_);
        gs::read_pod(in, &acc->maxWeight_);
        gs::read_pod(in, &acc->cnt_);
        gs::read_pod(in, &acc->callCount_);
        acc->nextRecenter_ = 2UL*acc->cnt_;
        if (in.fail()) throw gs::IOReadFailure(
            "In npstat::WeightedStatAccumulator::restore: input stream failure");
    }
}
