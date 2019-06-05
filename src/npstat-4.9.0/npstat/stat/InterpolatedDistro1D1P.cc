#include <stdexcept>

#include "geners/GenericIO.hh"

#include "npstat/stat/InterpolatedDistro1D1P.hh"
#include "npstat/stat/InterpolatedDistribution1D.hh"
#include "npstat/stat/VerticallyInterpolatedDistribution1D.hh"

namespace npstat {
    InterpolatedDistro1D1P::InterpolatedDistro1D1P(
        const GridAxis& axis,
        const std::vector<CPP11_shared_ptr<const AbsDistribution1D> >& distros,
        const double initialParameterValue, const bool interpolateVerticall)
        : axis_(axis),
          distros_(distros),
          interpolator_(0),
          param_(initialParameterValue),
          vertical_(!interpolateVerticall)
    {
        const unsigned nDistros = distros_.size();
        if (!nDistros) throw  std::invalid_argument(
            "In npstat::InterpolatedDistro1D1P constructor: "
            "no distros to interpolate");
        if (axis_.nCoords() != nDistros) throw std::invalid_argument(
            "In npstat::InterpolatedDistro1D1P constructor: "
            "number of provided distributions must be equal to the grid size");
        interpolateVertically(interpolateVerticall);
    }

    InterpolatedDistro1D1P::InterpolatedDistro1D1P(
        const InterpolatedDistro1D1P& r)
        : axis_(r.axis_),
          distros_(r.distros_),
          interpolator_(0),
          param_(r.param_),
          vertical_(!r.vertical_)
    {
        assert(axis_.nCoords() == distros_.size());
        interpolateVertically(r.vertical_);
    }

    InterpolatedDistro1D1P& InterpolatedDistro1D1P::operator=(
        const InterpolatedDistro1D1P& r)
    {
        if (this != &r)
        {
            axis_ = r.axis_;
            distros_ = r.distros_;
            assert(axis_.nCoords() == distros_.size());
            if (vertical_ != r.vertical_)
            {
                param_ = r.param_;
                interpolateVertically(r.vertical_);
            }
            else
                setParameter(r.param_);
        }
        return *this;
    }

    void InterpolatedDistro1D1P::setParameter(const double value)
    {
        param_ = value;
        interpolator_->clear();
        interpolator_->normalizeAutomatically(false);
        const std::pair<unsigned,double>& intv = axis_.getInterval(param_);
        interpolator_->add(*distros_[intv.first], intv.second);
        interpolator_->add(*distros_[intv.first+1], 1.0-intv.second);
        interpolator_->normalizeAutomatically(true);
    }

    void InterpolatedDistro1D1P::interpolateVertically(const bool b)
    {
        if (b != vertical_)
        {
            vertical_ = b;
            delete interpolator_;
            interpolator_ = 0;
            if (vertical_)
                interpolator_ = new VerticallyInterpolatedDistribution1D(2U);
            else
                interpolator_ = new InterpolatedDistribution1D(2U);
            setParameter(param_);
        }
    }

    bool InterpolatedDistro1D1P::isEqual(const AbsDistribution1D& otherBase) const
    {
        const InterpolatedDistro1D1P& r =
            static_cast<const InterpolatedDistro1D1P&>(otherBase);
        if (param_ != r.param_)
            return false;
        if (vertical_ != r.vertical_)
            return false;
        if (axis_ != r.axis_)
            return false;
        const unsigned sz = distros_.size();
        if (sz != r.distros_.size())
            return false;
        for (unsigned i=0; i<sz; ++i)
            if (*distros_[i] != *r.distros_[i])
                return false;
        return true;
    }

    bool InterpolatedDistro1D1P::write(std::ostream& os) const
    {
        const bool status = gs::write_obj(os, axis_) &&
                            gs::write_item(os, distros_);
        if (status)
        {
            gs::write_pod(os, param_);
            unsigned char c = vertical_;
            gs::write_pod(os, c);
        }
        return status && !os.fail();
    }

    InterpolatedDistro1D1P* InterpolatedDistro1D1P::read(const gs::ClassId& id,
                                                         std::istream& in)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<InterpolatedDistro1D1P>());
        current.ensureSameId(id);

        CPP11_auto_ptr<GridAxis> paxis = gs::read_obj<GridAxis>(in);
        std::vector<CPP11_shared_ptr<const AbsDistribution1D> > distros;
        gs::restore_item(in, &distros);
        double p;
        gs::read_pod(in, &p);
        unsigned char c;
        gs::read_pod(in, &c);

        if (in.fail()) throw gs::IOReadFailure(
            "In npstat::InterpolatedDistro1D1P::read: input stream failure");
        return new InterpolatedDistro1D1P(*paxis, distros, p, c);
    }
}
