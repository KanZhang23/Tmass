#include <cassert>

#include "npstat/stat/StatAccumulatorPair.hh"
#include "geners/binaryIO.hh"

namespace npstat {
    double StatAccumulatorPair::cov() const
    {
        const unsigned long count = first_.count();
        if (!count) throw std::runtime_error(
            "In npstat::StatAccumulatorPair::cov: no data accumulated");
        if (count == 1U)
            return 0.0;
        return (crossSumsq_ - first_.sum()/count*second_.sum())/(count - 1UL);
    }

    double StatAccumulatorPair::corr() const
    {
        const unsigned long count = first_.count();
        const double sigma1 = first_.stdev();
        const double sigma2 = second_.stdev();
        double cr = 0.0;
        if (sigma1 > 0.0 && sigma2 > 0.0)
        {
            cr = (crossSumsq_ - first_.sum()/count*second_.sum())/
                (count-1UL)/sigma1/sigma2;
            if (cr < -1.0)
                cr = -1.0;
            if (cr > 1.0)
                cr = 1.0;
        }
        return cr;
    }

    void StatAccumulatorPair::accumulate(const double x, const double y)
    {
        first_.accumulate(x);
        second_.accumulate(y);
        crossSumsq_ += x*y;
    }

    void StatAccumulatorPair::accumulate(const std::pair<double,double>& p)
    {
        first_.accumulate(p.first);
        second_.accumulate(p.second);
        crossSumsq_ += p.first*p.second;
    }

    void StatAccumulatorPair::accumulate(const StatAccumulatorPair& r)
    {
        first_.accumulate(r.first_);
        second_.accumulate(r.second_);
        crossSumsq_ += r.crossSumsq_;
    }

    void StatAccumulatorPair::reset()
    {
        first_.reset();
        second_.reset();
        crossSumsq_= 0.0L;
    }

    bool StatAccumulatorPair::operator==(const StatAccumulatorPair& r) const
    {
        return first_ == r.first_ &&
            second_ == r.second_ &&
            crossSumsq_ == r.crossSumsq_;
    }

    bool StatAccumulatorPair::write(std::ostream& of) const
    {
        gs::write_pod(of, crossSumsq_);
        first_.classId().write(of);
        first_.write(of);
        second_.write(of);
        return !of.fail();
    }

    void StatAccumulatorPair::restore(const gs::ClassId& id, std::istream& in,
                                      StatAccumulatorPair* acc)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<StatAccumulatorPair>());
        current.ensureSameId(id);

        assert(acc);
        gs::read_pod(in, &acc->crossSumsq_);
        gs::ClassId id1(in, 1);
        StatAccumulator::restore(id1, in, &acc->first_);
        StatAccumulator::restore(id1, in, &acc->second_);
    }
}
