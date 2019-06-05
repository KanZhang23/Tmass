#include <climits>
#include <cassert>

#include "geners/CPP11_auto_ptr.hh"
#include "geners/binaryIO.hh"
#include "geners/IOException.hh"

#include "npstat/stat/DistributionNDReader.hh"
#include "npstat/stat/GridInterpolatedDistribution.hh"
#include "npstat/stat/UnitMapInterpolationND.hh"
#include "npstat/stat/CopulaInterpolationND.hh"
#include "npstat/stat/CompositeDistributionND.hh"

static npstat::ArrayShape makeGGridShape(
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
    GridInterpolatedDistribution::GridInterpolatedDistribution(
        const std::vector<GridAxis>& axes, const unsigned distroDim,
        const bool interpCopulas, const bool useNearestGridCellOnly)
        : AbsGridInterpolatedDistribution(distroDim),
          axes_(axes),
          cellBuf_(axes.size()),
          interpolated_(makeGGridShape(axes)),
          interpolator_(0),
          single_(0),
          nAxes_(axes.size()),
          pointSet_(false),
          interpCopulas_(!interpCopulas),
          useNearestOnly_(useNearestGridCellOnly)
    {
        if (!(nAxes_ && nAxes_ < CHAR_BIT*sizeof(unsigned)))
            throw std::invalid_argument(
                "In npstat::GridInterpolatedDistribution constructor: "
                "invalid number of grid axes");
        const unsigned long len = interpolated_.length();
        AbsDistributionND** ptr =
            const_cast<AbsDistributionND**>(interpolated_.data());
        for (unsigned long i=0; i<len; ++i)
            ptr[i] = 0;
        interpolateCopulas(interpCopulas);
    }

    void GridInterpolatedDistribution::rebuildInterpolator()
    {
        delete interpolator_;
        interpolator_ = 0;
        if (!useNearestOnly_)
        {
            const unsigned n = 1U << nAxes_;
            if (interpCopulas_)
                interpolator_ = new CopulaInterpolationND(dim(), n);
            else
                interpolator_ = new UnitMapInterpolationND(dim(), n);
        }
        pointSet_ = false;
        single_ = 0;
    }

    void GridInterpolatedDistribution::interpolateCopulas(const bool b)
    {
        if (interpCopulas_ != b)
        {
            interpCopulas_ = b;
            rebuildInterpolator();
        }
    }

    void GridInterpolatedDistribution::useSingleCell(const bool b)
    {
        if (useNearestOnly_ != b)
        {
            useNearestOnly_ = b;
            rebuildInterpolator();
        }
    }

    void GridInterpolatedDistribution::releaseDistros()
    {
        const unsigned long len = interpolated_.length();
        AbsDistributionND* const* ptr = interpolated_.data();
        for (unsigned long i=0; i<len; ++i)
            delete ptr[i];
        pointSet_ = false;
        single_ = 0;
    }

    void GridInterpolatedDistribution::cloneDistros()
    {
        const unsigned long len = interpolated_.length();
        AbsDistributionND** ptr =
            const_cast<AbsDistributionND**>(interpolated_.data());
        for (unsigned long i=0; i<len; ++i)
            if (ptr[i])
                ptr[i] = ptr[i]->clone();
        pointSet_ = false;
        single_ = 0;
    }

    GridInterpolatedDistribution::~GridInterpolatedDistribution()
    {
        releaseDistros();
        delete interpolator_;
    }

    GridInterpolatedDistribution::GridInterpolatedDistribution(
        const GridInterpolatedDistribution& r)
        : AbsGridInterpolatedDistribution(r),
          axes_(r.axes_),
          cellBuf_(r.cellBuf_),
          interpolated_(r.interpolated_),
          interpolator_(0),
          single_(0),
          nAxes_(r.nAxes_),
          pointSet_(false),
          interpCopulas_(!r.interpCopulas_),
          useNearestOnly_(r.useNearestOnly_)
    {
        cloneDistros();
        interpolateCopulas(r.interpCopulas_);
    }

    GridInterpolatedDistribution& GridInterpolatedDistribution::operator=(
        const GridInterpolatedDistribution& r)
    {
        if (this != &r)
        {
            releaseDistros();
            interpolated_.uninitialize();

            AbsGridInterpolatedDistribution::operator=(r);
            axes_ = r.axes_;
            cellBuf_ = r.cellBuf_;
            interpolated_ = r.interpolated_;
            nAxes_ = r.nAxes_;
            pointSet_ = false;
            single_ = 0;
            interpCopulas_ = !r.interpCopulas_;
            useNearestOnly_ = r.useNearestOnly_;

            cloneDistros();
            interpolateCopulas(r.interpCopulas_);
        }
        return *this;
    }

    void GridInterpolatedDistribution::setGridDistro(
        const unsigned* cell, const unsigned lenCell,
        AbsDistributionND* distro)
    {
        delete interpolated_.valueAt(cell, lenCell);
        interpolated_.value(cell, lenCell) = distro;
        pointSet_ = false;
        single_ = 0;
    }

    const AbsDistributionND* GridInterpolatedDistribution::getGridDistro(
        const unsigned* cell, const unsigned lenCell) const
    {
        return interpolated_.valueAt(cell, lenCell);
    }

    void GridInterpolatedDistribution::setLinearDistro(
        const unsigned long idx, AbsDistributionND* distro)
    {
        delete interpolated_.linearValueAt(idx);
        interpolated_.linearValue(idx) = distro;
        pointSet_ = false;
        single_ = 0;
    }

    const AbsDistributionND* GridInterpolatedDistribution::getLinearDistro(
        const unsigned long idx) const
    {
        return interpolated_.linearValueAt(idx);
    }

    void GridInterpolatedDistribution::setGridCoords(
        const double* coords, const unsigned lenCoords)
    {
        if (lenCoords != nAxes_) throw std::invalid_argument(
            "In npstat::GridInterpolatedDistribution::setGridCoords: "
            "incompatible parameter dimensionality");
        assert(coords);
        if (useNearestOnly_)
            setGridCoordsSingle(coords);
        else
        {
            setGridCoordsInterpolating(coords);
            single_ = 0;
        }
        pointSet_ = true;
    }

    void GridInterpolatedDistribution::setGridCoordsSingle(
        const double* coords)
    {
        const GridAxis* ax = &axes_[0];
        unsigned* cell = &cellBuf_[0];
        for (unsigned i=0; i<nAxes_; ++i)
        {
            const std::pair<unsigned,double>& wi = ax[i].getInterval(coords[i]);
            if (wi.second < 0.5)
                cell[i] = wi.first + 1U;
            else
                cell[i] = wi.first;
        }
        single_ = interpolated_.value(cell, nAxes_);
        assert(single_);
    }

    void GridInterpolatedDistribution::setGridCoordsInterpolating(
        const double* coords)
    {
        const unsigned maxdim = CHAR_BIT*sizeof(unsigned);
        double weights[maxdim];

        const GridAxis* ax = &axes_[0];
        unsigned* cell = &cellBuf_[0];
        bool newCell = !pointSet_;
        for (unsigned i=0; i<nAxes_; ++i)
        {
            const std::pair<unsigned,double>& wi = ax[i].getInterval(coords[i]);
            if (wi.first != cell[i])
                newCell = true;
            cell[i] = wi.first;
            weights[i] = wi.second;
        }

        const unsigned maxcycle = 1U << nAxes_;
        if (newCell)
        {
            interpolator_->clear();
            AbsDistributionND* const* ptr = interpolated_.data();
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
    }

    bool GridInterpolatedDistribution::write(std::ostream& of) const
    {
        const unsigned long arrlen = interpolated_.length();
        for (unsigned long i=0UL; i<arrlen; ++i)
            if (interpolated_.linearValue(i) == 0)
                throw std::runtime_error(
                    "In npstat::GridInterpolatedDistribution::write: "
                    "distribution is not fully constructed");

        unsigned u = dim();
        gs::write_pod(of, u);
        unsigned char icop = interpCopulas_;
        gs::write_pod(of, icop);
        unsigned char usenear = useNearestOnly_;
        gs::write_pod(of, usenear);
        gs::write_obj_vector(of, axes_);

        bool status = !of.fail();
        for (unsigned long i=0UL; i<arrlen && status; ++i)
        {
            const AbsDistributionND* p = interpolated_.linearValue(i);
            status = p->classId().write(of) && p->write(of);
        }
        return status;
    }

    GridInterpolatedDistribution* GridInterpolatedDistribution::read(
        const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(
           gs::ClassId::makeId<GridInterpolatedDistribution>());
        id.ensureSameName(current);
        id.ensureVersionInRange(2, 3);

        unsigned d = 0;
        gs::read_pod(in, &d);
        unsigned char icop;
        gs::read_pod(in, &icop);
        unsigned char usenear = 0;
        if (id.version() > 2)
            gs::read_pod(in, &usenear);

        std::vector<GridAxis> axes;
        gs::read_heap_obj_vector_as_placed(in, &axes);

        if (in.fail()) throw gs::IOReadFailure(
            "In npstat::GridInterpolatedDistribution::read: "
            "input stream failure");
        if (!d) throw gs::IOInvalidData(
            "In npstat::GridInterpolatedDistribution::read: invalid data");

        const DistributionNDReader& reader = 
            StaticDistributionNDReader::instance();
        CPP11_auto_ptr<GridInterpolatedDistribution> result(
            new GridInterpolatedDistribution(axes, d, icop, usenear));
        const unsigned long arrlen = result->nDistros();
        for (unsigned long i=0UL; i<arrlen; ++i)
        {
            gs::ClassId classid(in, 1);
            AbsDistributionND* dis = reader.read(classid, in);
            assert(dis);
            result->setLinearDistro(i, dis);
        }
        return result.release();
    }

    bool GridInterpolatedDistribution::isEqual(
        const AbsDistributionND& ri) const
    {
        const GridInterpolatedDistribution& o = 
            static_cast<const GridInterpolatedDistribution&>(ri);
        if (axes_ != o.axes_)
            return false;
        if (interpCopulas_ != o.interpCopulas_)
            return false;
        if (useNearestOnly_ != o.useNearestOnly_)
            return false;
        const unsigned long len = interpolated_.length();
        for (unsigned long i=0; i<len; ++i)
        {
            const AbsDistributionND* left = interpolated_.linearValue(i);
            const AbsDistributionND* right = o.interpolated_.linearValue(i);
            if (left == 0 || right == 0)
                return false;
            if (*left != *right)
                return false;
        }
        return true;
    }

    double GridInterpolatedDistribution::marginalDensity(
        const unsigned idim, const double x) const
    {
        if (!pointSet_) throw std::logic_error(
            "In npstat::GridInterpolatedDistribution::marginalDensity: "
            "grid coordinates not set");
        if (useNearestOnly_)
            return (dynamic_cast<const CompositeDistributionND&>(*single_)).
                marginal(idim)->density(x);
        else
            return (dynamic_cast<const CopulaInterpolationND&>(*interpolator_)).
                marginalDensity(idim, x);
    }

    double GridInterpolatedDistribution::marginalCdf(
        const unsigned idim, const double x) const
    {
        if (!pointSet_) throw std::logic_error(
            "In npstat::GridInterpolatedDistribution::marginalCdf: "
            "grid coordinates not set");
        if (useNearestOnly_)
            return (dynamic_cast<const CompositeDistributionND&>(*single_)).
                marginal(idim)->cdf(x);
        else
            return (dynamic_cast<const CopulaInterpolationND&>(*interpolator_)).
                marginalCdf(idim, x);
    }

    double GridInterpolatedDistribution::marginalExceedance(
        const unsigned idim, const double x) const
    {
        if (!pointSet_) throw std::logic_error(
            "In npstat::GridInterpolatedDistribution::marginalExceedance: "
            "grid coordinates not set");
        if (useNearestOnly_)
            return (dynamic_cast<const CompositeDistributionND&>(*single_)).
                marginal(idim)->exceedance(x);
        else
            return (dynamic_cast<const CopulaInterpolationND&>(*interpolator_)).
                marginalExceedance(idim, x);
    }

    double GridInterpolatedDistribution::marginalQuantile(
        const unsigned idim, const double x) const
    {
        if (!pointSet_) throw std::logic_error(
            "In npstat::GridInterpolatedDistribution::marginalQuantile: "
            "grid coordinates not set");
        if (useNearestOnly_)
            return (dynamic_cast<const CompositeDistributionND&>(*single_)).
                marginal(idim)->quantile(x);
        else
            return (dynamic_cast<const CopulaInterpolationND&>(*interpolator_)).
                marginalQuantile(idim, x);
    }

    double GridInterpolatedDistribution::copulaDensity(
        const double* x, const unsigned dim) const
    {
        if (!pointSet_) throw std::logic_error(
            "In npstat::GridInterpolatedDistribution::copulaDensity: "
            "grid coordinates not set");
        if (useNearestOnly_)
            return (dynamic_cast<const CompositeDistributionND&>(*single_)).
                copulaDensity(x, dim);
        else
            return (dynamic_cast<const CopulaInterpolationND&>(*interpolator_)).
                copulaDensity(x, dim);
    }

    double GridInterpolatedDistribution::productOfTheMarginals(
        const double* x, const unsigned dim) const
    {
        if (!pointSet_) throw std::logic_error(
            "In npstat::GridInterpolatedDistribution::productOfTheMarginals: "
            "grid coordinates not set");
        if (useNearestOnly_)
            return (dynamic_cast<const CompositeDistributionND&>(*single_)).
                productOfTheMarginals(x, dim);
        else
            return (dynamic_cast<const CopulaInterpolationND&>(*interpolator_)).
                productOfTheMarginals(x, dim);
    }

    double GridInterpolatedDistribution::density(
        const double* x, unsigned dim) const
    {
        if (!pointSet_) throw std::logic_error(
            "In npstat::GridInterpolatedDistribution::density: "
            "grid coordinates not set");
        if (useNearestOnly_)
            return single_->density(x, dim);
        else
            return interpolator_->density(x, dim);
    }

    void GridInterpolatedDistribution::unitMap(
        const double* rnd, unsigned len, double* x) const
    {
        if (!pointSet_) throw std::logic_error(
            "In npstat::GridInterpolatedDistribution::unitMap: "
            "grid coordinates not set");
        if (useNearestOnly_)
            return single_->unitMap(rnd, len, x);
        else
            return interpolator_->unitMap(rnd, len, x);
    }

    bool GridInterpolatedDistribution::mappedByQuantiles() const
    {
        if (!pointSet_) throw std::logic_error(
            "In npstat::GridInterpolatedDistribution::mappedByQuantiles: "
            "grid coordinates not set");
        if (useNearestOnly_)
            return single_->mappedByQuantiles();
        else
            return interpolator_->mappedByQuantiles();
    }
}
