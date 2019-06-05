#include <cmath>
#include <cfloat>
#include <stdexcept>
#include <algorithm>
#include <numeric>

#include "geners/GenericIO.hh"
#include "geners/CPP11_auto_ptr.hh"
#include "geners/IOException.hh"

#include "npstat/stat/DistributionMix1D.hh"
#include "npstat/stat/StatUtils.hh"

namespace npstat {
    DistributionMix1D::DistributionMix1D()
        : AbsDistribution1D(),
          wsum_(0.0L),
          isNormalized_(false)
    {
    }

    DistributionMix1D::DistributionMix1D(const DistributionMix1D& r)
        : entries_(r.entries_.size(), 0),
          weights_(r.weights_),
          weightCdf_(r.weightCdf_),
          wsum_(r.wsum_),
          isNormalized_(r.isNormalized_)
    {
        const unsigned n = entries_.size();
        for (unsigned i=0; i<n; ++i)
            entries_[i] = r.entries_[i]->clone();
    }

    DistributionMix1D& DistributionMix1D::operator=(const DistributionMix1D& r)
    {
        if (this != &r)
        {
            {
                const unsigned n = entries_.size();
                for (unsigned i=0; i<n; ++i)
                    delete entries_[i];
            }
            entries_ = r.entries_;
            const unsigned n = entries_.size();
            for (unsigned i=0; i<n; ++i)
                entries_[i] = 0;
            for (unsigned i=0; i<n; ++i)
                entries_[i] = r.entries_[i]->clone();
            weights_ = r.weights_;
            weightCdf_ = r.weightCdf_;
            wsum_ = r.wsum_;
            isNormalized_ = r.isNormalized_;
        }
        return *this;
    }

    DistributionMix1D::~DistributionMix1D()
    {
        const unsigned n = entries_.size();
        for (unsigned i=0; i<n; ++i)
            delete entries_[i];
    }

    DistributionMix1D& DistributionMix1D::add(
        const AbsDistribution1D& distro, const double weight)
    {
        entries_.push_back(distro.clone());
        weights_.push_back(weight);
        isNormalized_ = false;
        return *this;
    }

    void DistributionMix1D::setWeights(const double* weights, const unsigned nWeights)
    {
        const unsigned nentries = entries_.size();
        if (nentries != nWeights) throw std::invalid_argument(
            "In npstat::DistributionMix1D::setWeights: wrong # of weights");
        if (nWeights)
        {
            assert(weights);
            for (unsigned i=0; i<nWeights; ++i)
                weights_[i] = weights[i];
            isNormalized_ = false;
        }
    }

    double DistributionMix1D::getWeight(const unsigned n) const
    {
        if (!isNormalized_)
            (const_cast<DistributionMix1D*>(this))->normalize();
        return weights_.at(n)/wsum_;
    }

    double DistributionMix1D::density(const double x) const
    {
        if (!isNormalized_)
            (const_cast<DistributionMix1D*>(this))->normalize();
        const unsigned nentries = entries_.size();
        long double sum = 0.0L;
        for (unsigned i=0; i<nentries; ++i)
            sum += weights_[i]*entries_[i]->density(x);
        return sum/wsum_;
    }

    double DistributionMix1D::cdf(const double x) const
    {
        if (!isNormalized_)
            (const_cast<DistributionMix1D*>(this))->normalize();
        const unsigned nentries = entries_.size();
        long double sum = 0.0L;
        for (unsigned i=0; i<nentries; ++i)
            sum += weights_[i]*entries_[i]->cdf(x);
        return sum/wsum_;
    }

    double DistributionMix1D::exceedance(const double x) const
    {
        if (!isNormalized_)
            (const_cast<DistributionMix1D*>(this))->normalize();
        const unsigned nentries = entries_.size();
        long double sum = 0.0L;
        for (unsigned i=0; i<nentries; ++i)
            sum += weights_[i]*entries_[i]->exceedance(x);
        return sum/wsum_;
    }

    double DistributionMix1D::quantile(const double r1) const
    {
        if (!isNormalized_)
            (const_cast<DistributionMix1D*>(this))->normalize();
        const unsigned nentries = entries_.size();
        if (nentries == 1U)
            return entries_[0]->quantile(r1);
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::DistributionMix1D::quantile: "
            "cdf argument outside of [0, 1] interval");
        double qmax = -DBL_MAX;
        double qmin = DBL_MAX;
        const AbsDistribution1D* const* dr = &entries_[0];
        for (unsigned i=0; i<nentries; ++i)
        {
            const double q = dr[i]->quantile(r1);
            if (q > qmax)
                qmax = q;
            if (q < qmin)
                qmin = q;
        }
        if (qmax == qmin)
            return qmin;
        if (r1 == 1.0)
            return qmax;
        if (r1 == 0.0)
            return qmin;
        const double fmin = cdf(qmin);
        const double fmax = cdf(qmax);
        if (!(fmin < r1 && r1 < fmax)) throw std::runtime_error(
            "In npstat::DistributionMix1D::quantile: "
            "algorithm precondition error");
        for (unsigned i=0; i<1000; ++i)
        {
            const double x = (qmin + qmax)/2.0;
            if (fabs(qmax - qmin)/std::max(fabs(qmin), fabs(qmax)) <
                2.0*DBL_EPSILON)
                return x;
            const double fval = cdf(x);
            if (fval == r1)
                return x;
            else if (fval > r1)
                qmax = x;
            else
                qmin = x;
            if (qmax == qmin)
                return qmin;
        }
        return (qmin + qmax)/2.0;
    }

    unsigned DistributionMix1D::random(AbsRandomGenerator& g, double* rnd) const
    {
        if (!isNormalized_)
            (const_cast<DistributionMix1D*>(this))->normalize();
        const unsigned nentries = entries_.size();
        const unsigned bin = quantileBinFromCdf(&weightCdf_[0], nentries, g());
        assert(bin < nentries);
        return 1U + entries_[bin]->random(g, rnd);
    }

    void DistributionMix1D::normalize()
    {
        const unsigned nentries = entries_.size();
        if (!nentries) throw std::runtime_error(
            "In npstat::DistributionMix1D::normalize: no components in the mix");
        weightCdf_.clear();
        weightCdf_.reserve(nentries);
        const long double wnorm = std::accumulate(
            weights_.begin(), weights_.end(), 0.0L);
        if (wnorm <= 0.0L) throw std::runtime_error(
            "In npstat::DistributionMix1D::normalize: sum of weights is not positive");
        wsum_ = 0.0L;
        for (unsigned i=0; i<nentries; ++i)
        {
            weightCdf_.push_back(static_cast<double>(wsum_/wnorm));
            wsum_ += weights_[i];
        }
        isNormalized_ = true;
    }

    bool DistributionMix1D::isEqual(const AbsDistribution1D& oth) const
    {
        const DistributionMix1D& r = static_cast<const DistributionMix1D&>(oth);
        const unsigned n = entries_.size();
        if (n != r.entries_.size())
            return false;
        if (n == 0)
            return true;
        for (unsigned i=0; i<n; ++i)
            if (getWeight(i) != r.getWeight(i))
                return false;
        for (unsigned i=0; i<n; ++i)
            if (*entries_[i] != *r.entries_[i])
                return false;
        return true;
    }

    bool DistributionMix1D::write(std::ostream& os) const
    {
        gs::write_pod_vector(os, weights_);
        gs::write_pod_vector(os, weightCdf_);
        gs::write_pod(os, wsum_);
        unsigned char c = isNormalized_;
        gs::write_pod(os, c);
        return gs::write_item(os, entries_);
    }

    DistributionMix1D* DistributionMix1D::read(
        const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<DistributionMix1D>());
        current.ensureSameId(id);

        CPP11_auto_ptr<DistributionMix1D> p(new DistributionMix1D());
        gs::read_pod_vector(in, &p->weights_);
        gs::read_pod_vector(in, &p->weightCdf_);
        gs::read_pod(in, &p->wsum_);
        unsigned char c;
        gs::read_pod(in, &c);
        p->isNormalized_ = c;
        gs::restore_item(in, &p->entries_);
        if (in.fail()) throw gs::IOReadFailure(
            "In npstat::DistributionMix1D::read: input stream failure");
        return p.release();
    }
}
