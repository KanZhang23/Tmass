#include <cmath>
#include <cfloat>
#include <cassert>

#include "npstat/stat/StatAccumulator.hh"
#include "geners/binaryIO.hh"
#include "geners/IOException.hh"

namespace npstat {
    void StatAccumulator::reset()
    {
        sum_ = 0.0L;
        sumsq_ = 0.0L;
        runningMean_ = 0.0L;
        min_ = DBL_MAX;
        max_ = -DBL_MAX;
        count_ = 0;
        nextRecenter_ = 0;
    }

    StatAccumulator::StatAccumulator()
    {
        reset();
    }

    inline void StatAccumulator::recenter()
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

    inline void StatAccumulator::recenter(const long double newCenter)
    {
        recenter();
        const long double dr = runningMean_ - newCenter;
        sumsq_ += dr*dr*count_;
        sum_ = dr*count_;
        runningMean_ = newCenter;
    }

    void StatAccumulator::accumulate(const double value)
    {
        const long double ldv = value - runningMean_;
        sum_ += ldv;
        sumsq_ += ldv*ldv;
        if (value < min_)
            min_ = value;
        if (value > max_)
            max_ = value;
        if (++count_ >= nextRecenter_)
            recenter();
    }

    void StatAccumulator::accumulate(const StatAccumulator& acc)
    {
        if (acc.count_)
        {
            StatAccumulator a(acc);
            a.recenter(runningMean_);
            sum_ += a.sum_;
            sumsq_ += a.sumsq_;
            count_ += a.count_;
            if (a.min_ < min_)
                min_ = a.min_;
            if (a.max_ > max_)
                max_ = a.max_;
            if (count_ >= nextRecenter_)
                recenter();
        }
    }

    void StatAccumulator::accumulate(const StatAccumulator& acc,
                                     const double w)
    {
        if (acc.count_)
        {
            StatAccumulator a(acc);
            a *= w;
            a.recenter(runningMean_);
            sum_ += a.sum_;
            sumsq_ += a.sumsq_;
            count_ += a.count_;
            if (a.min_ < min_)
                min_ = a.min_;
            if (a.max_ > max_)
                max_ = a.max_;
            if (count_ >= nextRecenter_)
                recenter();
        }
    }

    double StatAccumulator::mean() const
    {
        if (!count_) throw std::runtime_error(
            "In npstat::StatAccumulator::mean: no data accumulated");
        return sum_/count_ + runningMean_;
    }

    double StatAccumulator::stdev() const
    {
        if (!count_) throw std::runtime_error(
            "In npstat::StatAccumulator::stdev: no data accumulated");
        if (count_ == 1UL)
            return 0.0;
        else
        {
            const long double m = sum_/count_;
            long double sumsq = sumsq_ - m*sum_;
            if (sumsq < 0.0L)
                sumsq = 0.0L;
            return sqrtl(sumsq/(count_ - 1UL));
        }
    }

    double StatAccumulator::noThrowMean(const double valueIfNoData) const
    {
        if (count_)
            return mean();
        else
            return valueIfNoData;
    }

    double StatAccumulator::noThrowStdev(const double valueIfNoData) const
    {
        if (count_)
            return stdev();
        else
            return valueIfNoData;
    }

    double StatAccumulator::noThrowMeanUncertainty(const double valueIfNoData) const
    {
        if (count_)
            return meanUncertainty();
        else
            return valueIfNoData;
    }

    long double StatAccumulator::sum() const
    {
        return sum_ + runningMean_*count_;
    }

    long double StatAccumulator::sumsq() const
    {
        long double s = sumsq_ + runningMean_*(runningMean_*count_ + 2.0*sum_);
        if (s < 0.0L)
            s = 0.0L;
        return s;
    }

    double StatAccumulator::meanUncertainty() const
    {
        if (!count_) throw std::runtime_error(
            "In npstat::StatAccumulator::meanUncertainty: no data accumulated");
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

    bool StatAccumulator::operator==(const StatAccumulator& r) const
    {
        if (!(min_ == r.min_ &&
              max_ == r.max_ &&
              count_ == r.count_))
            return false;
        if (count_ == 0UL)
            return true;
        (const_cast<StatAccumulator*>(this))->recenter();
        (const_cast<StatAccumulator&>(r)).recenter();
        return sumsq_ == r.sumsq_ && runningMean_ == r.runningMean_;
    }

    bool StatAccumulator::write(std::ostream& os) const
    {
        (const_cast<StatAccumulator*>(this))->recenter();
        gs::write_pod(os, runningMean_);
        gs::write_pod(os, sumsq_);
        gs::write_pod(os, min_);
        gs::write_pod(os, max_);
        gs::write_pod(os, count_);
        return !os.fail();
    }

    void StatAccumulator::restore(const gs::ClassId& id, std::istream& in,
                                  StatAccumulator* acc)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<StatAccumulator>());
        current.ensureSameId(id);

        assert(acc);
        acc->sum_ = 0.0L;
        gs::read_pod(in, &acc->runningMean_);
        gs::read_pod(in, &acc->sumsq_);
        gs::read_pod(in, &acc->min_);
        gs::read_pod(in, &acc->max_);
        gs::read_pod(in, &acc->count_);
        acc->nextRecenter_ = 2UL*acc->count_;
        if (in.fail()) throw gs::IOReadFailure(
            "In npstat::StatAccumulator::restore: input stream failure");
    }
}
