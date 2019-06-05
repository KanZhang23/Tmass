#include "geners/CPP11_auto_ptr.hh"
#include "npstat/stat/CompositeDistributionND.hh"
#include "npstat/stat/Distribution1DReader.hh"
#include "npstat/stat/DistributionNDReader.hh"

namespace npstat {
    CompositeDistributionND::CompositeDistributionND(
        const AbsDistributionND* copula,
        const std::vector<const AbsDistribution1D*> marginals,
        bool assumePointerOwnership)
        : AbsDistributionND(marginals.size()),
          copula_(copula),
          marginals_(marginals)
    {
        const unsigned mydim = marginals.size();
        if (mydim <= 1U) throw std::invalid_argument(
            "In npstat::CompositeDistributionND constructor: "
            "must have at least two marginal distributions");
        assert(copula);
        if (copula->dim() != mydim) throw std::invalid_argument(
            "In npstat::CompositeDistributionND constructor: "
            "incompatible copula dimensionality");
        for (unsigned i=0; i<mydim; ++i)
            assert(marginals_[i]);
        if (!assumePointerOwnership)
        {
            copula_ = copula->clone();
            for (unsigned i=0; i<mydim; ++i)
                marginals_[i] = marginals[i]->clone();
        }
        work_.resize(mydim);
    }

    void CompositeDistributionND::cleanup()
    {
        const unsigned mydim = marginals_.size();
        for (unsigned i=0; i<mydim; ++i)
        {
            delete marginals_[i];
            marginals_[i] = 0;
        }
        delete copula_;
        copula_ = 0;
    }

    CompositeDistributionND::~CompositeDistributionND()
    {
        cleanup();
    }

    CompositeDistributionND::CompositeDistributionND(
        const CompositeDistributionND& r)
        : AbsDistributionND(r),
          copula_(0),
          marginals_(r.marginals_.size())
    {
        copula_ = r.copula_->clone();
        const unsigned mydim = marginals_.size();
        for (unsigned i=0; i<mydim; ++i)
            marginals_[i] = r.marginals_[i]->clone();
        work_.resize(mydim);
    }

    CompositeDistributionND& CompositeDistributionND::operator=(
        const CompositeDistributionND& r)
    {
        if (this != &r)
        {
            AbsDistributionND::operator=(r);
            cleanup();
            copula_ = r.copula_->clone();
            const unsigned mydim = r.marginals_.size();
            marginals_.resize(mydim);
            for (unsigned i=0; i<mydim; ++i)
                marginals_[i] = r.marginals_[i]->clone();
            work_.resize(mydim);
        }
        return *this;
    }

    double CompositeDistributionND::density(
        const double* x, const unsigned dim) const
    {
        if (dim != dim_) throw std::invalid_argument(
            "In npstat::CompositeDistributionND::density: "
            "incompatible input point dimensionality");
        assert(x);
        double prod = 1.0;
        double* w = &work_[0];
        for (unsigned i=0; i<dim_; ++i)
        {
            prod *= marginals_[i]->density(x[i]);
            w[i] = marginals_[i]->cdf(x[i]);
        }
        return prod*copula_->density(w, dim);
    }

    double CompositeDistributionND::copulaDensity(
        const double* x, const unsigned dim) const
    {
        if (dim != dim_) throw std::invalid_argument(
            "In npstat::CompositeDistributionND::copulaDensity: "
            "incompatible input point dimensionality");
        assert(x);
        double* w = &work_[0];
        for (unsigned i=0; i<dim_; ++i)
            w[i] = marginals_[i]->cdf(x[i]);
        return copula_->density(w, dim);
    }

    double CompositeDistributionND::productOfTheMarginals(
        const double* x, const unsigned dim) const
    {
        if (dim != dim_) throw std::invalid_argument(
            "In npstat::CompositeDistributionND::productOfTheMarginals: "
            "incompatible input point dimensionality");
        assert(x);
        double prod = 1.0;
        for (unsigned i=0; i<dim_; ++i)
            prod *= marginals_[i]->density(x[i]);
        return prod;
    }

    void CompositeDistributionND::unitMap(
        const double* rnd, const unsigned dim, double* x) const
    {
        if (dim != dim_) throw std::invalid_argument(
            "In npstat::CompositeDistributionND::unitMap: "
            "incompatible input point dimensionality");
        assert(x);
        double* w = &work_[0];
        copula_->unitMap(rnd, dim, w);
        for (unsigned i=0; i<dim_; ++i)
            x[i] = marginals_[i]->quantile(w[i]);
    }

    bool CompositeDistributionND::isEqual(const AbsDistributionND& rBase) const
    {
        const CompositeDistributionND& r = 
            static_cast<const CompositeDistributionND&>(rBase);
        if (dim_ != r.dim_)
            return false;
        if (*copula_ != *r.copula_)
            return false;
        for (unsigned i=0; i<dim_; ++i)
            if (*marginals_[i] != *r.marginals_[i])
                return false;
        return true;
    }

    bool CompositeDistributionND::write(std::ostream& of) const
    {
        bool status = copula_->classId().write(of) && copula_->write(of);
        if (status)
        {
            const unsigned long sz = marginals_.size();
            gs::write_pod(of, sz);
            status = !of.fail();
            for (unsigned long i=0; i<sz && status; ++i)
                status = marginals_[i]->classId().write(of) &&
                    marginals_[i]->write(of);
        }
        return status;
    }

    CompositeDistributionND* CompositeDistributionND::read(
        const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<CompositeDistributionND>());
        current.ensureSameId(id);

        gs::ClassId copulaId(in, 1);
        CPP11_auto_ptr<AbsDistributionND> copula(
            StaticDistributionNDReader::instance().read(copulaId, in));

        unsigned long sz = 0;
        gs::read_pod(in, &sz);
        std::vector<const AbsDistribution1D*> marginals(sz, 0);
        const Distribution1DReader& rd = StaticDistribution1DReader::instance();
        bool fail = !(copula.get() && sz);

        try
        {
            for (unsigned long i=0; i<sz && !fail; ++i)
            {
                gs::ClassId marginId(in, 1);
                fail = !(marginals[i] = rd.read(marginId, in));
            }
        }
        catch (...)
        {
            for (unsigned long i=0; i<sz; ++i)
                delete marginals[i];
            throw;
        }

        if (fail)
        {
            for (unsigned long i=0; i<sz; ++i)
                delete marginals[i];
            return 0;
        }
        else
            return new CompositeDistributionND(copula.release(),
                                               marginals, true);
    }
}
