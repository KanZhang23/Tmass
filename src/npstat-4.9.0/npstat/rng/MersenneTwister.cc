#include "npstat/rng/MersenneTwister.hh"
#include "npstat/rng/MersenneTwisterImpl.hh"

namespace npstat {
    MersenneTwister::MersenneTwister(const unsigned long seed)
        : impl_(new Private::MTRand(seed))
    {
    }

    MersenneTwister::MersenneTwister()
        : impl_(new Private::MTRand())
    {
    }

    MersenneTwister::MersenneTwister(const MersenneTwister& r)
        : AbsRandomGenerator(r),
          impl_(new Private::MTRand(*r.impl_))
    {
    }

    MersenneTwister& MersenneTwister::operator=(const MersenneTwister& r)
    {
        if (this != &r)
        {
            delete impl_;
            impl_ = 0;
            impl_ = new Private::MTRand(*r.impl_);
        }
        return *this;
    }

    MersenneTwister::~MersenneTwister()
    {
        delete impl_;
    }

    double MersenneTwister::operator()()
    {
        return impl_->rand53();
    }

    CPP11_auto_ptr<AbsRandomGenerator> make_MersenneTwister(
        const unsigned long seed)
    {
        CPP11_auto_ptr<AbsRandomGenerator> p;
        if (seed)
            p = CPP11_auto_ptr<AbsRandomGenerator>(new MersenneTwister(seed));
        else
            p = CPP11_auto_ptr<AbsRandomGenerator>(new MersenneTwister());
        return p;
    }
}
