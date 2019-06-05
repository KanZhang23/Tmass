#include "geners/GenericIO.hh"

#include "npstat/stat/InterpolatedDistro1DNP.hh"
#include "npstat/stat/Distribution1DReader.hh"
#include "npstat/stat/InterpolatedDistribution1D.hh"
#include "npstat/stat/VerticallyInterpolatedDistribution1D.hh"

static npstat::ArrayShape makeGGGridShape(
    const std::vector<npstat::GridAxis>& axes)
{
    const unsigned n = axes.size();
    npstat::ArrayShape result;
    result.reserve(n);
    for (unsigned i=0; i<n; ++i)
        result.push_back(axes[i].nCoords());
    return result;
}

namespace npstat {
    InterpolatedDistro1DNP::InterpolatedDistro1DNP(
        const std::vector<GridAxis>& axes, const bool i_nterpolateVertically)
        : axes_(axes),
          cellBuf_(axes.size()),
          interpolated_(makeGGGridShape(axes)),
          interpolator_(0),
          nAxes_(axes.size()),
          vertical_(!i_nterpolateVertically),
          pointSet_(false)
    {
        if (!(nAxes_ && nAxes_ < CHAR_BIT*sizeof(unsigned)))
            throw std::invalid_argument(
                "In npstat::InterpolatedDistro1DNP constructor: "
                "invalid number of grid axes");
        const unsigned long len = interpolated_.length();
        AbsDistribution1D** ptr =
            const_cast<AbsDistribution1D**>(interpolated_.data());
        for (unsigned long i=0; i<len; ++i)
            ptr[i] = 0;
        interpolateVertically(i_nterpolateVertically);
    }

    void InterpolatedDistro1DNP::interpolateVertically(const bool b)
    {
        if (b != vertical_)
        {
            const unsigned n = 1U << nAxes_;
            vertical_ = b;
            delete interpolator_;
            interpolator_ = 0;
            if (vertical_)
                interpolator_ = new VerticallyInterpolatedDistribution1D(n);
            else
                interpolator_ = new InterpolatedDistribution1D(n);
            pointSet_ = false;
        }
    }

    InterpolatedDistro1DNP::InterpolatedDistro1DNP(
        const InterpolatedDistro1DNP& r)
        : AbsDistribution1D(r),
          axes_(r.axes_),
          cellBuf_(r.cellBuf_),
          interpolated_(r.interpolated_),
          interpolator_(0),
          nAxes_(r.nAxes_),
          vertical_(!r.vertical_),
          pointSet_(false)
    {
        cloneDistros();
        interpolateVertically(r.vertical_);
    }

    InterpolatedDistro1DNP& InterpolatedDistro1DNP::operator=(
        const InterpolatedDistro1DNP& r)
    {
        if (this != &r)
        {
            releaseDistros();
            interpolated_.uninitialize();
            axes_ = r.axes_;
            cellBuf_ = r.cellBuf_;
            interpolated_ = r.interpolated_;
            nAxes_ = r.nAxes_;
            pointSet_ = false;
            cloneDistros();
            interpolateVertically(r.vertical_);
        }
        return *this;
    }

    InterpolatedDistro1DNP::~InterpolatedDistro1DNP()
    {
        releaseDistros();
        delete interpolator_;
    }

    void InterpolatedDistro1DNP::releaseDistros()
    {
        const unsigned long len = interpolated_.length();
        AbsDistribution1D* const* ptr = interpolated_.data();
        for (unsigned long i=0; i<len; ++i)
            delete ptr[i];
        pointSet_ = false;
    }

    void InterpolatedDistro1DNP::cloneDistros()
    {
        const unsigned long len = interpolated_.length();
        AbsDistribution1D** ptr =
            const_cast<AbsDistribution1D**>(interpolated_.data());
        for (unsigned long i=0; i<len; ++i)
            if (ptr[i])
                ptr[i] = ptr[i]->clone();
        pointSet_ = false;
    }

    void InterpolatedDistro1DNP::setGridDistro(
        const unsigned* cell, const unsigned lenCell,
        AbsDistribution1D* distro)
    {
        delete interpolated_.valueAt(cell, lenCell);
        interpolated_.value(cell, lenCell) = distro;
        pointSet_ = false;
    }

    const AbsDistribution1D* InterpolatedDistro1DNP::getGridDistro(
        const unsigned* cell, const unsigned lenCell) const
    {
        return interpolated_.valueAt(cell, lenCell);
    }

    void InterpolatedDistro1DNP::setLinearDistro(
        const unsigned long idx, AbsDistribution1D* distro)
    {
        delete interpolated_.linearValueAt(idx);
        interpolated_.linearValue(idx) = distro;
        pointSet_ = false;
    }

    const AbsDistribution1D* InterpolatedDistro1DNP::getLinearDistro(
        const unsigned long idx) const
    {
        return interpolated_.linearValueAt(idx);
    }

    bool InterpolatedDistro1DNP::isEqual(const AbsDistribution1D& otherBase) const
    {
        const InterpolatedDistro1DNP& o =
            static_cast<const InterpolatedDistro1DNP&>(otherBase);
        if (axes_ != o.axes_)
            return false;
        if (vertical_ != o.vertical_)
            return false;
        const unsigned long len = interpolated_.length();
        for (unsigned long i=0; i<len; ++i)
        {
            const AbsDistribution1D* left = interpolated_.linearValue(i);
            const AbsDistribution1D* right = o.interpolated_.linearValue(i);
            if (left == 0 || right == 0)
                return false;
            if (*left != *right)
                return false;
        }
        return true;
    }

    bool InterpolatedDistro1DNP::write(std::ostream& of) const
    {
        const unsigned long arrlen = interpolated_.length();
        for (unsigned long i=0UL; i<arrlen; ++i)
            if (interpolated_.linearValue(i) == 0)
                throw std::runtime_error(
                    "In npstat::InterpolatedDistro1DNP::write: "
                    "distribution is not fully constructed");

        gs::write_obj_vector(of, axes_);
        unsigned char c = vertical_;
        gs::write_pod(of, c);
        bool status = !of.fail();
        for (unsigned long i=0UL; i<arrlen && status; ++i)
        {
            const AbsDistribution1D* p = interpolated_.linearValue(i);
            status = p->classId().write(of) && p->write(of);
        }
        return status;
    }

    InterpolatedDistro1DNP* InterpolatedDistro1DNP::read(const gs::ClassId& id,
                                                         std::istream& in)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<InterpolatedDistro1DNP>());
        current.ensureSameId(id);

        std::vector<GridAxis> axes;
        gs::read_heap_obj_vector_as_placed(in, &axes);
        unsigned char c;
        gs::read_pod(in, &c);

        if (in.fail()) throw gs::IOReadFailure(
            "In npstat::InterpolatedDistro1DNP::read: input stream failure");

        const Distribution1DReader& reader = 
            StaticDistribution1DReader::instance();
        CPP11_auto_ptr<InterpolatedDistro1DNP> result(
            new InterpolatedDistro1DNP(axes, c));
        const unsigned long arrlen = result->nDistros();
        for (unsigned long i=0UL; i<arrlen; ++i)
        {
            gs::ClassId classid(in, 1);
            AbsDistribution1D* dis = reader.read(classid, in);
            assert(dis);
            result->setLinearDistro(i, dis);
        }
        return result.release();
    }

    void InterpolatedDistro1DNP::setGridCoords(
        const double* coords, const unsigned lenCoords)
    {
        if (lenCoords != nAxes_) throw std::invalid_argument(
            "In npstat::InterpolatedDistro1DNP::setGridCoords: "
            "incompatible parameter dimensionality");
        assert(coords);

        const unsigned maxdim = CHAR_BIT*sizeof(unsigned);
        double weights[maxdim];

        const GridAxis* ax = &axes_[0];
        unsigned* cell = &cellBuf_[0];
        bool newCell = !pointSet_;
        for (unsigned i=0; i<nAxes_; ++i)
        {
            const std::pair<unsigned,double> wi(ax[i].getInterval(coords[i]));
            if (wi.first != cell[i])
                newCell = true;
            cell[i] = wi.first;
            weights[i] = wi.second;
        }

        const unsigned maxcycle = 1U << nAxes_;
        if (newCell)
        {
            interpolator_->clear();
            AbsDistribution1D* const* ptr = interpolated_.data();
            const unsigned long* strides = interpolated_.strides();
            for (unsigned icycle=0; icycle<maxcycle; ++icycle)
            {
                double w = 1.0;
                unsigned long icell = 0UL;
                for (unsigned i=0; i<nAxes_; ++i)
                {
                    if (icycle & (1U << i))
                    {
                        w *= (1.0 - weights[i]);
                        icell += strides[i]*(cell[i] + 1U);
                    }
                    else
                    {
                        w *= weights[i];
                        icell += strides[i]*cell[i];
                    }
                }
                assert(ptr[icell]);
                interpolator_->add(*ptr[icell], w);
            }
        }
        else
        {
            interpolator_->normalizeAutomatically(false);
            for (unsigned icycle=0; icycle<maxcycle; ++icycle)
            {
                double w = 1.0;
                for (unsigned i=0; i<nAxes_; ++i)
                {
                    if (icycle & (1U << i))
                        w *= (1.0 - weights[i]);
                    else
                        w *= weights[i];
                }
                interpolator_->setWeight(icycle, w);
            }
            interpolator_->normalizeAutomatically(true);
        }

        pointSet_ = true;
    }
}
