#include <cassert>
#include <stdexcept>

#include "npstat/stat/WeightedStatAccumulatorPair.hh"
#include "geners/binaryIO.hh"

namespace npstat {
    double WeightedStatAccumulatorPair::cov() const
    {
        if (!first_.nfills()) throw std::runtime_error(
            "In npstat::WeightedStatAccumulatorPair::cov: "
            "no data accumulated");
        const double count = first_.count();
        if (count <= 1.0)
            return 0.0;
        return (crossSumsq_/first_.sumOfWeights() - 
                first_.mean()*second_.mean())*
               count/(count - 1UL);
    }

    double WeightedStatAccumulatorPair::corr() const
    {
        const double sigma1 = first_.stdev();
        const double sigma2 = second_.stdev();
        double cr = 0.0;
        if (sigma1 > 0.0 && sigma2 > 0.0)
        {
            cr = cov()/sigma1/sigma2;
            if (cr < -1.0)
                cr = -1.0;
            if (cr > 1.0)
                cr = 1.0;
        }
        return cr;
    }

    void WeightedStatAccumulatorPair::accumulate(
        const double x, const double y, const double w)
    {
        first_.accumulate(x, w);
        second_.accumulate(y, w);
        crossSumsq_ += w*x*y;
    }

    void WeightedStatAccumulatorPair::accumulate(
        const std::pair<double,double>& p, const double w)
    {
        first_.accumulate(p.first, w);
        second_.accumulate(p.second, w);
        crossSumsq_ += w*p.first*p.second;
    }

    void WeightedStatAccumulatorPair::accumulate(
        const WeightedStatAccumulatorPair& r)
    {
        first_.accumulate(r.first_);
        second_.accumulate(r.second_);
        crossSumsq_ += r.crossSumsq_;
    }

    void WeightedStatAccumulatorPair::accumulate(
        const WeightedStatAccumulatorPair& r, const double w)
    {
        first_.accumulate(r.first_, w);
        second_.accumulate(r.second_, w);
        crossSumsq_ += w*r.crossSumsq_;
    }

    void WeightedStatAccumulatorPair::reset()
    {
        first_.reset();
        second_.reset();
        crossSumsq_= 0.0L;
    }

    bool WeightedStatAccumulatorPair::operator==(
        const WeightedStatAccumulatorPair& r) const
    {
        return first_ == r.first_ &&
            second_ == r.second_ &&
            crossSumsq_ == r.crossSumsq_;
    }

    bool WeightedStatAccumulatorPair::write(std::ostream& of) const
    {
        gs::write_pod(of, crossSumsq_);
        first_.classId().write(of);
        first_.write(of);
        second_.write(of);
        return !of.fail();
    }

    void WeightedStatAccumulatorPair::restore(
        const gs::ClassId& id, std::istream& in,
        WeightedStatAccumulatorPair* acc)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<WeightedStatAccumulatorPair>());
        current.ensureSameId(id);

        assert(acc);
        gs::read_pod(in, &acc->crossSumsq_);
        gs::ClassId id1(in, 1);
        WeightedStatAccumulator::restore(id1, in, &acc->first_);
        WeightedStatAccumulator::restore(id1, in, &acc->second_);
    }
}
