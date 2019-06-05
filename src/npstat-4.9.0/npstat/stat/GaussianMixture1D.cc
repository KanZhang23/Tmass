#include <cmath>
#include <cfloat>
#include <algorithm>
#include <cassert>

#include "geners/CPP11_auto_ptr.hh"
#include "geners/GenericIO.hh"

#include "npstat/stat/StatUtils.hh"
#include "npstat/stat/GaussianMixture1D.hh"
#include "npstat/stat/amiseOptimalBandwidth.hh"
#include "npstat/stat/distributionReadError.hh"

#include "npstat/rng/permutation.hh"

#include "npstat/nm/SpecialFunctions.hh"
#include "npstat/nm/goldenSectionSearch.hh"
#include "npstat/nm/MathUtils.hh"

#define SQRTPIL 1.772453850905516027298167L

// Expression for the variance constant of Marron and Wand
static long double varianceConstant(const unsigned r)
{
    const unsigned ncached = 50;
    static long double cache[ncached] = {0.0L,};

    long double result = 0.0;
    bool recalculate = true;
    if (r < ncached)
        if (cache[r] != 0.0)
        {
            result = cache[r];
            recalculate = false;
        }

    if (recalculate)
    {
        std::vector<std::pair<long double,long double> > terms;
        const unsigned nterms = r*r;
        terms.reserve(nterms);
        for (unsigned s=0; s<r; ++s)
            for (unsigned sprime=0; sprime<r; ++sprime)
            {
                long double term = npstat::ldfactorial(2*s + 2*sprime)/
                    powl(2.0L,3*s+3*sprime+1)/
                    npstat::ldfactorial(s)/npstat::ldfactorial(sprime)/
                    npstat::ldfactorial(s+sprime);
                terms.push_back(std::make_pair(fabsl(term), term));
            }
        assert(terms.size() == nterms);
        std::sort(terms.begin(), terms.end());
        long double sum = 0.0L;
        for (unsigned i=0; i<nterms; ++i)
            sum += terms[i].second;
        result = sum/SQRTPIL;
        if (r < ncached)
            cache[r] = result;
    }
    return result;
}

static inline int minusonetos(const unsigned s)
{
    return (s % 2U) ? -1 : 1;
}

namespace npstat {
    bool GaussianMixtureEntry::write(std::ostream& os) const
    {
        gs::write_pod(os, w_);
        gs::write_pod(os, mean_);
        gs::write_pod(os, stdev_);
        return !os.fail();
    }

    bool GaussianMixtureEntry::operator==(const GaussianMixtureEntry& r) const
    {
        return w_ == r.w_ && mean_ == r.mean_ && stdev_ == r.stdev_;
    }

    void GaussianMixtureEntry::restore(const gs::ClassId& id, std::istream& in,
                                       GaussianMixtureEntry* entry)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<GaussianMixtureEntry>());
        current.ensureSameId(id);

        assert(entry);
        gs::read_pod(in, &entry->w_);
        gs::read_pod(in, &entry->mean_);
        gs::read_pod(in, &entry->stdev_);
        if (in.fail()) throw gs::IOReadFailure(
            "In npstat::GaussianMixtureEntry::restore: input stream failure");
    }

    GaussianMixture1D::GaussianMixture1D(const double location,
                                         const double scale,
                                         const GaussianMixtureEntry* ientries,
                                         const unsigned nentries)
        : AbsScalableDistribution1D(location, scale)
    {
        assert(ientries);
        unsigned n_not_0 = 0U;
        long double wsum = 0.0L;
        for (unsigned i=0; i<nentries; ++i)
            if (ientries[i].w_ > 0.0)
            {
                wsum += ientries[i].w_;
                ++n_not_0;
            }
        if (!n_not_0) throw std::invalid_argument(
            "In npstat::GaussianMixture1D constructor: "
            "must have at least one mixture component with positive weight");
        entries_.reserve(n_not_0);
        const double norm = wsum;
        for (unsigned i=0; i<nentries; ++i)
            if (ientries[i].w_ > 0.0)
            {
                entries_.push_back(ientries[i]);
                entries_[i].w_ /= norm;
            }
        initialize();
    }

    bool GaussianMixture1D::isEqual(const AbsDistribution1D& otherBase) const
    {
        const GaussianMixture1D& r = 
            static_cast<const GaussianMixture1D&>(otherBase);
        return AbsScalableDistribution1D::isEqual(r) && entries_ == r.entries_;
    }

    void GaussianMixture1D::initialize()
    {
        const unsigned nentries = entries_.size();
        weightCdf_.clear();
        weightCdf_.reserve(nentries);
        distros_.clear();
        distros_.reserve(nentries);
        long double wsum = 0.0L;
        for (unsigned i=0; i<nentries; ++i)
        {
            distros_.push_back(Gauss1D(entries_[i].mean_, entries_[i].stdev_));
            weightCdf_.push_back(static_cast<double>(wsum));
            wsum += entries_[i].w_;
        }
    }

    bool GaussianMixture1D::write(std::ostream& os) const
    {
        return AbsScalableDistribution1D::write(os) &&
               gs::write_item(os, entries_);
    }

    GaussianMixture1D::GaussianMixture1D(const double location,
                                         const double scale,
                                         const std::vector<double>& params)
        : AbsScalableDistribution1D(location, scale)
    {
        if (!(params.size() && params.size() % 3U == 0U))
            throw std::invalid_argument(
                "In npstat::GaussianMixture1D constructor: "
                "invalid number of parameters");
        const unsigned ntriples = params.size() / 3U;
        entries_.reserve(ntriples);
        long double wsum = 0.0L;
        for (unsigned i=0; i<ntriples; ++i)
        {
            const double w = params[i*3];
            if (w < 0.0) throw std::invalid_argument(
                "In npstat::GaussianMixture1D constructor: "
                "all weights must be non-negative");
            const double mean = params[i*3 + 1];
            const double stdev = params[i*3 + 2];
            if (w > 0.0)
            {
                wsum += w;
                entries_.push_back(GaussianMixtureEntry(w, mean, stdev));
            }
        }
        const unsigned nentries = entries_.size();
        if (!nentries) throw std::invalid_argument(
            "In npstat::GaussianMixture1D constructor: "
            "must have at least one component with positive weight");
        const double norm = wsum;
        for (unsigned i=0; i<nentries; ++i)
            entries_[i].w_ /= norm;
        initialize();
    }

    GaussianMixture1D::GaussianMixture1D(const Gauss1D& g)
        : AbsScalableDistribution1D(0.0, 1.0),
          entries_(1, GaussianMixtureEntry(1.0, g.location(), g.scale()))
    {
        initialize();
    }

    GaussianMixture1D* GaussianMixture1D::read(
        const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<GaussianMixture1D>());
        current.ensureSameId(id);

        double location, scale;
        if (!AbsScalableDistribution1D::read(in, &location, &scale))
        {
            distributionReadError(in, classname());
            return 0;
        }
        CPP11_auto_ptr<GaussianMixture1D> ptr(new GaussianMixture1D(
                                                 location, scale));
        gs::restore_item(in, &ptr->entries_);
        ptr->initialize();
        return ptr.release();
    }

    double GaussianMixture1D::unscaledDensity(const double x) const
    {
        const unsigned nentries = entries_.size();
        long double sum = 0.0L;
        for (unsigned i=0; i<nentries; ++i)
            sum += entries_[i].w_ * distros_[i].density(x);
        return sum;
    }

    double GaussianMixture1D::unscaledCdf(const double x) const
    {
        const unsigned nentries = entries_.size();
        long double sum = 0.0L;
        for (unsigned i=0; i<nentries; ++i)
            sum += entries_[i].w_ * distros_[i].cdf(x);
        return sum;
    }

    double GaussianMixture1D::unscaledExceedance(const double x) const
    {
        const unsigned nentries = entries_.size();
        long double sum = 0.0L;
        for (unsigned i=0; i<nentries; ++i)
            sum += entries_[i].w_ * distros_[i].exceedance(x);
        return sum;
    }

    double GaussianMixture1D::unscaledQuantile(const double r1) const
    {
        const unsigned nentries = entries_.size();
        if (nentries == 1U)
            return distros_[0].quantile(r1);
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::GaussianMixture1D::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        double qmax = -DBL_MAX;
        double qmin = DBL_MAX;
        const Gauss1D* dr = &distros_[0];
        for (unsigned i=0; i<nentries; ++i)
        {
            const double q = dr[i].quantile(r1);
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
        const double fmin = unscaledCdf(qmin);
        const double fmax = unscaledCdf(qmax);
        if (!(fmin < r1 && r1 < fmax))
            throw std::runtime_error("In GaussianMixture1D::unscaledQuantile: "
                                     "algorithm precondition error");
        for (unsigned i=0; i<1000; ++i)
        {
            const double x = (qmin + qmax)/2.0;
            if (fabs(qmax - qmin)/std::max(fabs(qmin), fabs(qmax)) <
                2.0*DBL_EPSILON)
                return x;
            const double fval = unscaledCdf(x);
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

    double GaussianMixture1D::unscaledMean() const
    {
        const unsigned nentries = entries_.size();
        const GaussianMixtureEntry* ent = &entries_[0];
        long double sum = 0.0L;
        for (unsigned i=0; i<nentries; ++i)
            sum += ent[i].mean_ * ent[i].w_;
        return sum;
    }

    double GaussianMixture1D::unscaledStdev() const
    {
        const unsigned nentries = entries_.size();
        const GaussianMixtureEntry* ent = &entries_[0];
        long double sum = 0.0L;
        long double meansum = 0.0L;
        for (unsigned i=0; i<nentries; ++i)
        {
            meansum += ent[i].mean_ * ent[i].w_;
            sum += ent[i].w_ * (ent[i].mean_*ent[i].mean_ +
                                ent[i].stdev_*ent[i].stdev_);
        }
        long double delta = sum - meansum*meansum;
        if (delta > 0.0L)
            return sqrtl(delta);
        else
            return 0.0;
    }

    unsigned GaussianMixture1D::random(AbsRandomGenerator& g, double* rnd) const
    {
        const unsigned nentries = entries_.size();
        const unsigned bin = quantileBinFromCdf(&weightCdf_[0], nentries, g());
        assert(bin < nentries);
        return 1U + distros_[bin].random(g, rnd);
    }

    double GaussianMixture1D::mean() const
    {
        return unscaledMean()*this->scale() + this->location();
    }

    double GaussianMixture1D::stdev() const
    {
        return unscaledStdev()*this->scale();
    }

    double GaussianMixture1D::gaussianMISE(
        const unsigned maxPolyDegree,
        const double h, const unsigned long n) const
    {
        return gmise(h, maxPolyDegree, n);
    }

    double GaussianMixture1D::miseOptimalBw(const unsigned maxPolyDegree,
                                            const unsigned long npoints,
                                            double* mise) const
    {
        const unsigned nentries = entries_.size();
        const double scal = this->scale();

        long double bestmise = -1.0;
        double bestbw = -1.0;

        for (unsigned i=0; i<nentries; ++i)            
        {
            const double sigma = entries_[i].stdev_;
            const double w = entries_[i].w_;
            const unsigned long n = static_cast<unsigned long>(w*npoints)+1UL;

            double bw = 0.0;
            long double tmp = 0.0L;

            if (goldenSectionSearchInLogSpace(
                    GaussianMISEFunctor(*this, maxPolyDegree, npoints), 
                    amisePluginBwGauss(maxPolyDegree, n, sigma*scal),
                    sqrt(DBL_EPSILON), &bw, &tmp, 0.1))
            {
                if (bestbw < 0.0 || tmp < bestmise)
                {
                    bestmise = tmp;
                    bestbw = bw;
                }
            }
        }
        if (mise)
            *mise = bestmise;
        return bestbw;
    }

    long double GaussianMixture1D::gmise(const long double h,
                                         const unsigned maxPolyDegree,
                                         const unsigned long n) const
    {
        // We will accumulate coefficients to a polynomial in x = h^2
        // The degree of this polynomial is 2r - 2.
        const unsigned r = maxPolyDegree/2U + 1U;
        const unsigned bwpolydeg = 2*r - 2;

        polyterms_.resize(bwpolydeg + 1);
        long double* terms = &polyterms_[0];
        for (unsigned i=0; i<=bwpolydeg; ++i)
            terms[i] = 0.0L;

        for (unsigned int s=0; s<r; ++s)
            terms[s] -= 2*minusonetos(s)/powl(2.0L,s)/ldfactorial(s)*U(h,s,1);

        const long double factor = (1.0L - 1.0L/n);
        for (unsigned int s=0; s<r; ++s)
            for (unsigned int sprime=0; sprime<r; ++sprime)
            {
                const unsigned sums = s+sprime;
                terms[sums] += factor*minusonetos(sums)/powl(2.0L,sums)/
                    ldfactorial(s)/ldfactorial(sprime)*U(h, sums, 2);
            }
        terms[0] += U(h, 0U, 0U);
        terms[0] += varianceConstant(r)/n/h;

        return polySeriesSum(terms, bwpolydeg, h*h);
    }

    // Big U function of Marron and Wand in which the h^(2s) term is removed
    long double GaussianMixture1D::U(const long double h,
                                     const unsigned s, const unsigned q) const
    {
        uterms_.clear();

        const unsigned nentries = entries_.size();
        const unsigned nterms = nentries*nentries;
        const GaussianMixtureEntry* ent = &entries_[0];
        const double loc = this->location();
        const double scal = this->scale();

        for (unsigned i=0; i<nentries; ++i)
        {
            const long double wi = ent[i].w_;
            const long double mi = ent[i].mean_*scal + loc;
            const long double si = ent[i].stdev_*scal;

            for (unsigned j=0; j<nentries; ++j)
            {
                const long double wj = ent[j].w_;
                const long double mj = ent[j].mean_*scal + loc;
                const long double sj = ent[j].stdev_*scal;
                const long double sig = sqrtl(si*si + sj*sj + q*h*h);
                const long double t = normalDensityDerivative(2*s,(mi-mj)/sig)/
                    powl(sig,2*s+1)*wj*wi;
                uterms_.push_back(std::make_pair(fabsl(t), t));
            }
        }
        assert(uterms_.size() == nterms);

        std::sort(uterms_.begin(), uterms_.end());
        long double sum = 0.0L;
        for (unsigned i=0; i<nterms; ++i)
            sum += uterms_[i].second;
        return sum;
    }
}
