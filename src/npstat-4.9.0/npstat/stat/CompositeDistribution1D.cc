#include <stdexcept>

#include "geners/IOException.hh"
#include "npstat/stat/CompositeDistribution1D.hh"

namespace npstat {
    void CompositeDistribution1D::cleanup()
    {
        delete pH_; pH_ = 0;
        delete pG_; pG_ = 0;
    }

    CompositeDistribution1D::CompositeDistribution1D(
        const AbsDistribution1D& pG, const AbsDistribution1D& pH)
        : AbsDistribution1D(), pG_(0), pH_(0)
    {
        if (!(pG.quantile(0.0) == 0.0 &&
              pG.quantile(1.0) == 1.0 &&
              pG.cdf(0.0) == 0.0 &&
              pG.cdf(1.0) == 1.0))
            throw std::domain_error(
                "In npstat::CompositeDistribution1D constructor: "
                "inappropriate input distribution");
        pG_ = pG.clone();
        pH_ = pH.clone();
    }

    CompositeDistribution1D::CompositeDistribution1D(
        const CompositeDistribution1D& r)
        : AbsDistribution1D(), pG_(0), pH_(0)
    {
        pG_ = r.pG_->clone();
        pH_ = r.pH_->clone();
    }

    CompositeDistribution1D& CompositeDistribution1D::operator=(
        const CompositeDistribution1D& r)
    {
        if (this != &r)
        {
            AbsDistribution1D::operator=(r);
            cleanup();
            pG_ = r.pG_->clone();
            pH_ = r.pH_->clone();
        }
        return *this;
    }

    bool CompositeDistribution1D::isEqual(const AbsDistribution1D& other) const
    {
        const CompositeDistribution1D& r = 
            static_cast<const CompositeDistribution1D&>(other);
        return *pG_ == *r.pG_ && *pH_ == *r.pH_;
    }

    bool CompositeDistribution1D::write(std::ostream& os) const
    {
        pG_->classId().write(os);
        pG_->write(os);
        pH_->classId().write(os);
        pH_->write(os);
        return !os.fail();
    }

    CompositeDistribution1D::CompositeDistribution1D()
        : pG_(0), pH_(0)
    {
    }

    CompositeDistribution1D* CompositeDistribution1D::read(
        const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId myClassId(
            gs::ClassId::makeId<CompositeDistribution1D>());
        myClassId.ensureSameId(id);

        gs::ClassId id1(in, 1);
        AbsDistribution1D* d1 = AbsDistribution1D::read(id1, in);
        gs::ClassId id2(in, 1);
        AbsDistribution1D* d2 = AbsDistribution1D::read(id2, in);
        if (d1 && d2)
        {
            CompositeDistribution1D* cd = new CompositeDistribution1D();
            cd->pG_ = d1;
            cd->pH_ = d2;
            return cd;
        }
        else
        {
            delete d1;
            delete d2;
            if (in.fail()) throw gs::IOReadFailure(
                "In npstat::CompositeDistribution1D::read: "
                "input stream failure");
            else throw gs::IOInvalidData(
                "In npstat::CompositeDistribution1D::read: "
                "failed to read component distributions");
        }
    }
}
