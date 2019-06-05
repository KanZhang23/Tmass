#include <cassert>
#include <stdexcept>

#include "geners/binaryIO.hh"
#include "npstat/rng/AbsRandomGenerator.hh"
#include "npstat/stat/DistributionNDReader.hh"

namespace npstat {
    AbsDistributionND::AbsDistributionND(const unsigned dim)
        : dim_(dim)
    {
        if (!dim_) throw std::invalid_argument(
            "In npstat::AbsDistributionND constructor: distribution "
            "dimensionality can not be 0");
        temp_.resize(dim_);
        ws_ = &temp_[0];
    }

    AbsDistributionND::AbsDistributionND(const AbsDistributionND& r)
        : dim_(r.dim_)
    {
        temp_.resize(dim_);
        ws_ = &temp_[0];
    }

    AbsDistributionND& AbsDistributionND::operator=(const AbsDistributionND& r)
    {
        if (this != &r)
        {
            *(const_cast<unsigned*>(&dim_)) = r.dim_;        
            temp_.resize(dim_);
            ws_ = &temp_[0];
        }
        return *this;
    }

    bool AbsDistributionND::operator==(const AbsDistributionND& r) const
    {
        // Note that "isEqual" function will be called only
        // in case the types are the same. This means that
        // it is safe to make a type cast inside "isEqual",
        // and it can be a static cast instead of dynamic cast.
        return (typeid(*this) == typeid(r)) && this->isEqual(r);
    }

    unsigned AbsDistributionND::random(AbsRandomGenerator& g,
                                       double* x, const unsigned lenX) const
    {
        if (lenX != dim_)
            throw std::invalid_argument("AbsDistributionND::random: "
                                        "incompatible buffer size");
        assert(x);
        const unsigned gdim(g.dim());
        if (!(gdim == dim_ || gdim == 1U))
            throw std::invalid_argument("AbsDistributionND::random: "
                                        "incompatible random number "
                                        "generator dimensionality");
        const unsigned ngen = dim_/gdim;
        g.run(ws_, dim_, ngen);
        unitMap(ws_, dim_, x);
        return ngen;
    }

    void AbsScalableDistributionND::setScale(const unsigned i, const double v)
    {
        if (v <= 0.0) throw std::invalid_argument(
            "In npstat::AbsScalableDistributionND::setScale: "
            "scale parameter must be positive");
        const unsigned dim = scale_.size();
        if (i >= dim) throw std::out_of_range(
            "In npstat::AbsScalableDistributionND::setScale: "
            "dimension index is out of range");
        scale_[i] = v;
        scaleProd_ = 1.0;
        for (unsigned i=0; i<dim; ++i)
            scaleProd_ *= scale_[i];   
    }

    AbsScalableDistributionND::AbsScalableDistributionND(
        const double* location,
        const double* scale, const unsigned dim)
        : AbsDistributionND(dim), scaleProd_(1.0)
    {
        assert(location);
        assert(scale);
        location_.reserve(dim);
        scale_.reserve(dim);
        for (unsigned i=0; i<dim; ++i)
        {
            location_.push_back(location[i]);
            if (scale[i] <= 0.0) throw std::invalid_argument(
                "In npstat::AbsScalableDistributionND constructor: "
                "all input scales must be positive");
            scaleProd_ *= scale[i];
            scale_.push_back(scale[i]);
        }
        work_.resize(dim);
    }

    double AbsScalableDistributionND::density(
        const double* x, const unsigned dim) const
    {
        if (dim != dim_) throw std::invalid_argument(
            "In npstat::AbsScalableDistributionND::density "
            "incompatible input point dimensionality");
        assert(x);
        const double* l_ = &location_[0];
        const double* s_ = &scale_[0];
        double *w_ = &work_[0];
        for (unsigned i=0; i<dim; ++i)
            w_[i] = (x[i] - l_[i])/s_[i];
        return unscaledDensity(w_)/scaleProd_;
    }

    void AbsScalableDistributionND::unitMap(
        const double* rnd, const unsigned dim, double* x) const
    {
        if (dim)
        {
            if (dim > dim_) throw std::out_of_range(
                "In npstat::AbsScalableDistributionND::unitMap: "
                "input point dimensionality is out of range");
            assert(x);
            assert(rnd);
            for (unsigned i=0; i<dim; ++i)
                assert(rnd[i] >= 0.0 && rnd[i] <= 1.0);
            const double* l_ = &location_[0];
            const double* s_ = &scale_[0];
            double *w_ = &work_[0];
            unscaledUnitMap(rnd, dim, w_);
            for (unsigned i=0; i<dim; ++i)
                x[i] = s_[i]*w_[i] + l_[i];
        }
    }

    bool AbsScalableDistributionND::write(std::ostream& os) const
    {
        gs::write_pod(os, dim_);
        gs::write_pod_vector(os, location_);
        gs::write_pod_vector(os, scale_);
        return !os.fail();
    }

    bool AbsScalableDistributionND::read(std::istream& is, unsigned* dim,
                                         std::vector<double>* locations,
                                         std::vector<double>* scales)
    {
        gs::read_pod(is, dim);
        gs::read_pod_vector(is, locations);
        gs::read_pod_vector(is, scales);
        return !is.fail();
    }

    bool AbsScalableDistributionND::isEqual(
        const AbsDistributionND& otherBase) const
    {
        const AbsScalableDistributionND& r = 
            static_cast<const AbsScalableDistributionND&>(otherBase);

        return dim_ == r.dim_ && 
               location_ == r.location_ &&
               scale_ == r.scale_;
    }

    AbsDistributionND* AbsDistributionND::read(const gs::ClassId& id,
                                               std::istream& in)
    {
        return StaticDistributionNDReader::instance().read(id, in);
    }
}
