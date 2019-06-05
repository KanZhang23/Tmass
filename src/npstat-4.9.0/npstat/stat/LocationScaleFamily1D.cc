#include "npstat/stat/LocationScaleFamily1D.hh"
#include "npstat/stat/distributionReadError.hh"

#include "geners/binaryIO.hh"

namespace npstat {
    LocationScaleFamily1D::LocationScaleFamily1D(
        const LocationScaleFamily1D& r)
        : AbsScalableDistribution1D(r), d_(r.d_->clone())
    {
    }

    LocationScaleFamily1D& LocationScaleFamily1D::operator=(
        const LocationScaleFamily1D& r)
    {
        if (this != &r)
        {
            AbsScalableDistribution1D::operator=(r);
            delete d_; d_ = 0;
            d_ = r.d_->clone();
        }
        return *this;
    }

    bool LocationScaleFamily1D::write(std::ostream& os) const
    {
        AbsScalableDistribution1D::write(os);
        return gs::write_obj(os, *d_);
    }

    LocationScaleFamily1D* LocationScaleFamily1D::read(
        const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId myClassId(
            gs::ClassId::makeId<LocationScaleFamily1D>());
        myClassId.ensureSameId(id);

        double location, scale;
        if (AbsScalableDistribution1D::read(in, &location, &scale))
        {
            CPP11_auto_ptr<AbsDistribution1D> d =
                gs::read_obj<AbsDistribution1D>(in);
            if (!in.fail())
            {
                CPP11_auto_ptr<LocationScaleFamily1D> tmp(new LocationScaleFamily1D());
                tmp->setLocation(location);
                tmp->setScale(scale);
                tmp->d_ = d.release();
                return tmp.release();
            }
        }
        distributionReadError(in, classname());
        return 0;
    }
}
