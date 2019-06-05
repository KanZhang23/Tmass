#include <cmath>
#include <stdexcept>
#include <cfloat>
#include <climits>
#include <algorithm>

#include "geners/binaryIO.hh"

#include "npstat/rng/permutation.hh"

#include "npstat/stat/DiscreteDistributions1D.hh"
#include "npstat/stat/Distributions1D.hh"
#include "npstat/stat/distributionReadError.hh"

#define POISSON_MAXLAMBDA 1.0e6


static long doubleToLong(const double d)
{
    const double f = floor(d);
    if (f >= static_cast<double>(LONG_MAX))
        return LONG_MAX;
    else if (f <= static_cast<double>(LONG_MIN))
        return LONG_MIN;
    else
        return static_cast<long>(f);
}


namespace npstat {
    Poisson1D::Poisson1D(const double lam)
        : lambda_(lam),
          table_(0),
          minUse_(0),
          lnlambda_(0.0L)
    {
        if (lambda_ < 0.0)
            throw std::invalid_argument("In npstat::Poisson1D constructor: "
                                        "lambda can not be negative");
        if (lambda_ > 0.0)
            lnlambda_ = logl(lambda_);
    }

    void Poisson1D::buildTable()
    {
        assert(table_ == 0);
        assert(lambda_ > 0.0);
        assert(lambda_ < POISSON_MAXLAMBDA);

        const long l = static_cast<long>(floor(lambda_));
        long step = static_cast<long>(floor(sqrt(lambda_)));
        if (step < 1L)
            step = 1L;
        long tableZero = l;
        for (; ; tableZero+=step)
        {
            const double p = probability(tableZero);
            if (p <= DBL_MIN)
                break;
        }
        for (; ; --tableZero)
        {
            const double p = probability(tableZero);
            if (p > DBL_MIN)
                break;
        }
        for (minUse_ = l; ; minUse_-=step)
        {
            const double p = probability(minUse_);
            if (p <= DBL_MIN)
                break;
        }
        if (minUse_ < 0L)
            minUse_ = 0L;
        for (; ; ++minUse_)
        {
            const double p = probability(tableZero);
            if (p > DBL_MIN)
                break;
        }
        const long tableLen = tableZero - minUse_ + 1L;
        assert(tableLen > 0L);
        std::vector<double> probs(tableLen);
        for (long i=0; i<tableLen; ++i)
            probs[i] = probability(minUse_ + i);
        table_ = new DiscreteTabulated1D(minUse_, &probs[0], tableLen);
    }

    Poisson1D::Poisson1D(const Poisson1D& r)
        : lambda_(r.lambda_),
          table_(0),
          minUse_(r.minUse_),
          lnlambda_(r.lnlambda_)
    {
        if (r.table_)
            table_ = r.table_->clone();
    }

    Poisson1D& Poisson1D::operator=(const Poisson1D& r)
    {
        if (this != &r)
        {
            DiscreteTabulated1D* newtable = 0;
            if (r.table_)
                newtable = r.table_->clone();
            AbsDiscreteDistribution1D::operator=(r);
            lambda_ = r.lambda_;
            lnlambda_ = r.lnlambda_;
            minUse_ = r.minUse_;
            delete table_;
            table_ = newtable;
        }
        return *this;
    }

    double Poisson1D::probability(long x) const
    {
        if (x < 0L)
            return 0.0;
        else if (lambda_ == 0.0)
        {
            if (x == 0L)
                return 1.0;
            else
                return 0.0;
        }
        else if (lambda_ >= POISSON_MAXLAMBDA)
        {
            Gauss1D g(lambda_, sqrt(lambda_));
            return g.density(x);
        }
        else
        {
            const long double lg = x*lnlambda_ - logfactorial(x) - lambda_;
            return expl(lg);
        }
    }

    double Poisson1D::cdf(double x) const
    {
        if (x < 0.0)
            return 0.0;
        else if (lambda_ == 0.0)
        {
            if (x < 0.0)
                return 0.0;
            else
                return 1.0;
        }
        else if (lambda_ >= POISSON_MAXLAMBDA)
        {
            Gauss1D g(lambda_, sqrt(lambda_));
            return g.cdf(x);
        }
        else
        {
            if (table_ == 0)
                (const_cast<Poisson1D*>(this))->buildTable();
            return table_->cdf(x);
        }
    }

    double Poisson1D::exceedance(double x) const
    {
        if (x < 0.0)
            return 1.0;
        else if (lambda_ == 0.0)
        {
            if (x < 0.0)
                return 1.0;
            else
                return 0.0;
        }
        else if (lambda_ >= POISSON_MAXLAMBDA)
        {
            Gauss1D g(lambda_, sqrt(lambda_));
            return g.exceedance(x);
        }
        else
        {
            if (table_ == 0)
                (const_cast<Poisson1D*>(this))->buildTable();
            return table_->exceedance(x);
        }
    }

    long Poisson1D::quantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::Poisson1D::quantile: "
            "cdf argument outside of [0, 1] interval");
        if (lambda_ == 0.0)
            return 0L;
        else if (lambda_ >= POISSON_MAXLAMBDA)
        {
            Gauss1D g(lambda_, sqrt(lambda_));
            long l = static_cast<long>(round(g.quantile(r1)));
            if (l < 0L)
                l = 0L;
            return l;
        }
        else
        {
            if (table_ == 0)
                (const_cast<Poisson1D*>(this))->buildTable();
            return table_->quantile(r1);
        }
    }

    unsigned Poisson1D::random(AbsRandomGenerator& g,
                               long* generatedRandom) const
    {
        assert(generatedRandom);
        if (lambda_ == 0.0)
        {
            *generatedRandom = 0L;
            return 0U;
        }
        else if (lambda_ >= POISSON_MAXLAMBDA)
        {
            Gauss1D ga(lambda_, sqrt(lambda_));
            double d;
            const unsigned nUsed = ga.random(g, &d);
            *generatedRandom = static_cast<long>(round(d));
            if (*generatedRandom < 0L)
                *generatedRandom = 0L;
            return nUsed;
        }
        else
        {
            if (table_ == 0)
                (const_cast<Poisson1D*>(this))->buildTable();
            *generatedRandom = table_->quantile(g());
            return 1U;
        }
    }

    bool Poisson1D::write(std::ostream& os) const
    {
        gs::write_pod(os, lambda_);
        return !os.fail();
    }

    Poisson1D* Poisson1D::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<Poisson1D>());
        current.ensureSameId(id);

        double d(-1.0);
        gs::read_pod(in, &d);
        if (in.fail() || d < 0.0)
            distributionReadError(in, classname());
        return new Poisson1D(d);
    }

    bool DiscreteTabulated1D::isEqual(const AbsDiscreteDistribution1D& o) const
    {
        const DiscreteTabulated1D& r = static_cast<const DiscreteTabulated1D&>(o);
        return ShiftableDiscreteDistribution1D::isEqual(o) && table_ == r.table_;
    }

    bool DiscreteTabulated1D::write(std::ostream& os) const
    {
        long locat = this->location();
        gs::write_pod(os, locat);
        gs::write_pod_vector(os, table_);
        return !os.fail();
    }

    DiscreteTabulated1D* DiscreteTabulated1D::read(
        const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<DiscreteTabulated1D>());
        current.ensureSameId(id);

        long locat;
        gs::read_pod(in, &locat);
        std::vector<double> t;
        gs::read_pod_vector(in, &t);
        if (in.fail() || t.empty())
            distributionReadError(in, classname());
        return new DiscreteTabulated1D(locat, &t[0], t.size());
    }

    void DiscreteTabulated1D::initialize()
    {
        long double sum = 0.0L;
        const unsigned len = table_.size();
        if (!len)
            throw std::invalid_argument("In npstat::DiscreteTabulated1D::initialize: "
                                        "probability table is empty");
        cdf_.reserve(len);
        for (unsigned i=0; i<len; ++i)
        {
            if (table_[i] < 0.0)
                throw std::invalid_argument("In npstat::DiscreteTabulated1D::initialize: "
                                            "probability can not be negative");
            sum += table_[i];
            cdf_.push_back(sum);
        }
        double dsum = sum;
        if (dsum <= 0.0)
            throw std::invalid_argument("In npstat::DiscreteTabulated1D::initialize: "
                                        "all probabilities appear to be 0");
        firstNonZero_ = len;
        for (unsigned i=0; i<len; ++i)
        {
            table_[i] /= dsum;
            cdf_[i] /= dsum;
            if (firstNonZero_ == len && cdf_[i] > 0.0)
                firstNonZero_ = i;
        }
        for (lastNonZero_ = len - 1; lastNonZero_ > 0U; --lastNonZero_)
            if (cdf_[lastNonZero_] > cdf_[lastNonZero_ - 1U])
                break;

        sum = 0.0L;
        exceedance_.resize(len);
        for (long i = len-1U; i>=0L; --i)
        {
            sum += table_[i];
            exceedance_[i] = sum;
        }
        dsum = sum;
        for (unsigned i=0; i<len; ++i)
            exceedance_[i] /= dsum;
    }

    double DiscreteTabulated1D::unshiftedProbability(const long x) const
    {
        if (x < 0)
            return 0.0;
        else
        {
            const unsigned long ul = x;
            if (ul >= table_.size())
                return 0.0;
            else
                return table_[ul];
        }
    }

    double DiscreteTabulated1D::unshiftedCdf(const double x) const
    {
        const long l = doubleToLong(x);
        if (l < 0)
            return 0.0;
        else
        {
            const unsigned long ul = l;
            if (ul >= cdf_.size())
                return 1.0;
            else
                return cdf_[ul];
        }
    }

    double DiscreteTabulated1D::unshiftedExceedance(const double x) const
    {
        const long l = doubleToLong(x) + 1L;
        if (l < 0)
            return 1.0;
        else
        {
            const unsigned long ul = l;
            if (ul >= exceedance_.size())
                return 0.0;
            else
                return exceedance_[ul];
        }
    }

    long DiscreteTabulated1D::unshiftedQuantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::DiscreteTabulated1D::unshiftedQuantile: "
            "cdf argument outside of [0, 1] interval");
        if (r1 == 0.0)
            return firstNonZero_;
        else if (r1 == 1.0)
            return lastNonZero_;
        else
        {
            const long double lr = r1;
            return std::lower_bound(cdf_.begin(), cdf_.end(), lr) - cdf_.begin();
        }
    }

    DiscreteTabulated1D pooledDiscreteTabulated1D(
        const DiscreteTabulated1D& d1,
        const double sampleSize1,
        const DiscreteTabulated1D& d2,
        const double sampleSize2,
        const long first, const long oneAfterLast)
    {
        const long len = oneAfterLast - first;
        if (len <= 0L) throw std::invalid_argument(
            "In npstat::pooledDiscreteTabulated1D: invalid first/last range");
        if (sampleSize1 < 0.0 || sampleSize2 < 0.0) throw std::invalid_argument(
            "In npstat::pooledDiscreteTabulated1D: invalid sample size");
        std::vector<double> table(len);
        for (long i=0; i<len; ++i)
        {
            const long x = i + first;
            table[i]=sampleSize1*d1.probability(x)+sampleSize2*d2.probability(x);
        }
        return DiscreteTabulated1D(first, table);
    }
}
