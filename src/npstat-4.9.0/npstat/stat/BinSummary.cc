#include <cmath>
#include <algorithm>
#include <sstream>

#include "npstat/stat/BinSummary.hh"

#include "geners/binaryIO.hh"
#include "geners/IOException.hh"

using namespace gs;

namespace npstat {
    void BinSummary::validateBinSummary(const char* where,
                                        const double location,
                                        const double down,
                                        const double up,
                                        const double dmin,
                                        const double dmax)
    {
        bool bad = down < 0.0 || up < 0.0;
        if (dmin < DBL_MAX*(1.0 - 2.0*DBL_EPSILON))
            if (dmin > location)
                bad = true;
        if (dmax > -DBL_MAX*(1.0 - 2.0*DBL_EPSILON))
            if (dmax < location)
                bad = true;
        if (bad)
        {
            std::ostringstream os;
            os << "In " << where
               << ": operation would make an invalid object with"
               << " location = " << location
               << ", rangeDown = " << down
               << ", rangeUp = " << up
               << ", min = " << dmin
               << ", max = " << dmax;
            throw std::invalid_argument(os.str());
        }
    }

    BinSummary::BinSummary()
        : location_(0.0), rangeDown_(0.0), rangeUp_(0.0),
          min_(DBL_MAX), max_(-DBL_MAX)
    {
    }

    BinSummary::BinSummary(const bool loc)
        : location_(loc), rangeDown_(0.0), rangeUp_(0.0),
          min_(DBL_MAX), max_(-DBL_MAX)
    {
    }

    BinSummary::BinSummary(const unsigned char loc)
        : location_(loc), rangeDown_(0.0), rangeUp_(0.0),
          min_(DBL_MAX), max_(-DBL_MAX)
    {
    }

    BinSummary::BinSummary(const int loc)
        : location_(loc), rangeDown_(0.0), rangeUp_(0.0),
          min_(DBL_MAX), max_(-DBL_MAX)
    {
    }

    BinSummary::BinSummary(const long long loc)
        : location_(loc), rangeDown_(0.0), rangeUp_(0.0),
          min_(DBL_MAX), max_(-DBL_MAX)
    {
    }

    BinSummary::BinSummary(const float loc)
        : location_(loc), rangeDown_(0.0), rangeUp_(0.0),
          min_(DBL_MAX), max_(-DBL_MAX)
    {
    }

    BinSummary::BinSummary(const double loc)
        : location_(loc), rangeDown_(0.0), rangeUp_(0.0),
          min_(DBL_MAX), max_(-DBL_MAX)
    {
    }

    BinSummary& BinSummary::operator=(const double& loc)
    {
        location_ = loc;
        rangeDown_ = 0.0;
        rangeUp_ = 0.0;
        min_ = DBL_MAX;
        max_ = -DBL_MAX;
        return *this;
    }

    BinSummary::BinSummary(const double ilocation, const double irangeDown,
                           const double irangeUp)
        : location_(ilocation), rangeDown_(irangeDown), rangeUp_(irangeUp),
          min_(DBL_MAX), max_(-DBL_MAX)
    {
        validateBinSummary("npstat::BinSummary constructor",
                           location_, rangeDown_, rangeUp_, min_, max_);
    }

    BinSummary::BinSummary(const double ilocation, const double irangeDown,
                           const double irangeUp, const double imin,
                           const double imax)
        : location_(ilocation), rangeDown_(irangeDown), rangeUp_(irangeUp),
          min_(imin), max_(imax)
    {
        validateBinSummary("npstat::BinSummary constructor",
                           location_, rangeDown_, rangeUp_, min_, max_);
    }

    BinSummary::BinSummary(const double ilocation, const double stdev)
        : location_(ilocation), rangeDown_(stdev), rangeUp_(stdev),
          min_(DBL_MAX), max_(-DBL_MAX)
    {
        validateBinSummary("npstat::BinSummary constructor",
                           location_, rangeDown_, rangeUp_, min_, max_);
    }

    BinSummary& BinSummary::operator=(const BinSummary& r)
    {
        if (this != &r)
        {
            location_ = r.location_;
            rangeDown_ = r.rangeDown_;
            rangeUp_ = r.rangeUp_;
            min_ = r.min_;
            max_ = r.max_;
        }
        return *this;
    }

    BinSummary::BinSummary(const BinSummary& r)
        : location_(r.location_), rangeDown_(r.rangeDown_),
          rangeUp_(r.rangeUp_), min_(r.min_), max_(r.max_)
    {
    }

    bool BinSummary::operator==(const BinSummary& r) const
    {
        bool sameMins = false;
        if (min_ == r.min_)
            sameMins = true;
        if (!sameMins && !hasMin() && !r.hasMin())
            sameMins = true;

        bool sameMaxs = false;
        if (max_ == r.max_)
            sameMaxs = true;
        if (!sameMaxs && !hasMax() && !r.hasMax())
            sameMaxs = true;

        return location_ == r.location_ &&
            rangeDown_ == r.rangeDown_ &&
            rangeUp_ == r.rangeUp_ &&
            sameMins && sameMaxs;
    }

    double BinSummary::stdev() const
    {
        if (hasStdev())
            return rangeDown_;
        else
            throw std::runtime_error("In npstat::BinSummary::stdev: "
                                     "standard deviation is not defined");
    }

    double BinSummary::min() const
    {
        if (hasMin())
            return min_;
        else
            throw std::runtime_error("In npstat::BinSummary::min: "
                                     "minimum value is not set");
    }

    double BinSummary::max() const
    {
        if (hasMax())
            return max_;
        else
            throw std::runtime_error("In npstat::BinSummary::max: "
                                     "maximum value is not set");
    }

    bool BinSummary::hasStdev() const
    {
        if (rangeDown_ == rangeUp_)
            return true;
        if (rangeDown_ == 0.0 || rangeUp_ == 0.0)
            return false;
        return fabs(rangeDown_ - rangeUp_)/
            (fabs(rangeDown_) + fabs(rangeUp_)) < 4.0*DBL_EPSILON;
    }

    bool BinSummary::hasMin() const
    {
        return min_ < DBL_MAX*(1.0 - 2.0*DBL_EPSILON);
    }

    bool BinSummary::hasMax() const
    {
        return max_ > -DBL_MAX*(1.0 - 2.0*DBL_EPSILON);
    }

    bool BinSummary::hasLimits() const
    {
        return min_ < DBL_MAX*(1.0 - 2.0*DBL_EPSILON) &&
               max_ > -DBL_MAX*(1.0 - 2.0*DBL_EPSILON);
    }

    double BinSummary::noThrowStdev(const double valueIfNoData) const
    {
        if (hasStdev())
            return rangeDown_;
        else
            return valueIfNoData;
    }

    double BinSummary::noThrowMin(const double valueIfNoData) const
    {
        if (hasMin())
            return min_;
        else
            return valueIfNoData;
    }

    double BinSummary::noThrowMax(const double valueIfNoData) const
    {
        if (hasMax())
            return max_;
        else
            return valueIfNoData;
    }

    void BinSummary::symmetrizeRanges()
    {
        const double ave = (rangeDown_ + rangeUp_)/2.0;
        validateBinSummary("npstat::BinSummary::symmetrizeRanges",
                           location_, ave, ave, min_, max_);
        rangeDown_ = ave;
        rangeUp_ = ave;
    }

    void BinSummary::setLocation(const double newValue)
    {
        validateBinSummary("npstat::BinSummary::setLocation",
                           newValue, rangeDown_, rangeUp_, min_, max_);
        location_ = newValue;
    }

    void BinSummary::setStdev(const double ave)
    {
        validateBinSummary("npstat::BinSummary::setStdev",
                           location_, ave, ave, min_, max_);
        rangeDown_ = ave;
        rangeUp_ = ave;
    }

    void BinSummary::setRangeDown(const double newValue)
    {
        validateBinSummary("npstat::BinSummary::setRangeDown",
                           location_, newValue, rangeUp_, min_, max_);
        rangeDown_ = newValue;
    }

    void BinSummary::setRangeUp(const double newValue)
    {
        validateBinSummary("npstat::BinSummary::setRangeUp",
                           location_, rangeDown_, newValue, min_, max_);
        rangeUp_ = newValue;
    }

    void BinSummary::setRanges(const double newRangeDown,
                               const double newRangeUp)
    {
        validateBinSummary("npstat::BinSummary::setRanges",
                           location_, newRangeDown, newRangeUp, min_, max_);
        rangeDown_ = newRangeDown;
        rangeUp_ = newRangeUp;
    }

    void BinSummary::setMin(const double newValue)
    {
        validateBinSummary("npstat::BinSummary::setMin",
                           location_, rangeDown_, rangeUp_, newValue, max_);
        min_ = newValue;
    }

    void BinSummary::setMax(const double newValue)
    {
        validateBinSummary("npstat::BinSummary::setMax",
                           location_, rangeDown_, rangeUp_, min_, newValue);
        max_ = newValue;
    }

    void BinSummary::setLimits(const double newMin, const double newMax)
    {
        validateBinSummary("npstat::BinSummary::setLimits",
                           location_, rangeDown_, rangeUp_, newMin, newMax);
        min_ = newMin;
        max_ = newMax;
    }

    void BinSummary::setLocationAndLimits(const double newLocation,
                                      const double newMin, const double newMax)
    {
        validateBinSummary("npstat::BinSummary::setLocationAndLimits",
                           newLocation, rangeDown_, rangeUp_, newMin, newMax);
        location_ = newLocation;
        min_ = newMin;
        max_ = newMax;
    }

    void BinSummary::shift(const double delta)
    {
        location_ += delta;
        if (hasMin())
            min_ += delta;
        if (hasMax())
            max_ += delta;
    }

    void BinSummary::scaleWidth(const double s)
    {
        if (s < 0.0)
            throw std::invalid_argument("In npstat::BinSummary::scaleWidth: "
                                        "scale factor must be non-negative");
        rangeDown_ *= s;
        rangeUp_ *= s;
        if (hasMin())
            min_ = location_ - s*(location_ - min_);
        if (hasMax())
            max_ = location_ + s*(max_ - location_);
    }

    BinSummary& BinSummary::operator*=(const double r)
    {
        const double ar = fabs(r);
        location_ *= r;
        rangeDown_ *= ar;
        rangeUp_ *= ar;
        const bool hmin = hasMin();
        if (hmin)
            min_ *= r;
        const bool hmax = hasMax();
        if (hmax)
            max_ *= r;
        if (r < 0.0)
        {
            std::swap(rangeDown_, rangeUp_);
            const double oldMin = min_;
            const double oldMax = max_;
            if (hmin)
                max_ = oldMin;
            else
                max_ = -DBL_MAX;
            if (hmax)
                min_ = oldMax;
            else
                min_ = DBL_MAX;
        }
        return *this;
    }

    BinSummary& BinSummary::operator+=(const BinSummary& r)
    {
        if (hasStdev() && r.hasStdev())
        {
            location_ += r.location_;
            const double s = sqrt(rangeDown_*rangeDown_ +
                                  r.rangeDown_*r.rangeDown_);
            rangeDown_ = s;
            rangeUp_ = s;
            if (hasLimits() && r.hasLimits())
            {
                const double range = r.max_ - r.min_;
                min_ -= range;
                max_ += range;
            }
            else
            {
                min_ = DBL_MAX;
                max_ = -DBL_MAX;
            }
        }
        else
            throw std::runtime_error("In npstat::BinSummary::operator+=: "
                                     "can not combine asymmetric distributions");
        return *this;
    }

    BinSummary& BinSummary::operator-=(const BinSummary& r)
    {
        if (hasStdev() && r.hasStdev())
        {
            location_ -= r.location_;
            const double s = sqrt(rangeDown_*rangeDown_ +
                                  r.rangeDown_*r.rangeDown_);
            rangeDown_ = s;
            rangeUp_ = s;
            if (hasLimits() && r.hasLimits())
            {
                const double range = r.max_ - r.min_;
                min_ -= range;
                max_ += range;
            }
            else
            {
                min_ = DBL_MAX;
                max_ = -DBL_MAX;
            }
        }
        else
            throw std::runtime_error("In npstat::BinSummary::operator+=: "
                                     "can not combine asymmetric distributions");
        return *this;
    }

    bool BinSummary::write(std::ostream& of) const
    {
        validateBinSummary("npstat::BinSummary::write",
                           location_, rangeDown_, rangeUp_, min_, max_);

        write_pod(of, location_);
        write_pod(of, rangeDown_);
        write_pod(of, rangeUp_);

        const unsigned char hmin = hasMin();
        write_pod(of, hmin);
        if (hmin)
            write_pod(of, min_);

        const unsigned char hmax = hasMax();
        write_pod(of, hmax);
        if (hmax)
            write_pod(of, max_);

        return !of.fail();
    }

    void BinSummary::restore(const gs::ClassId& id, std::istream& in,
                             BinSummary* ptr)
    {
        static const ClassId myClassId(ClassId::makeId<BinSummary>());
        myClassId.ensureSameId(id);

        double location=0, rangeDown=0, rangeUp=0, dmin=DBL_MAX, dmax=-DBL_MAX;
        unsigned char hmin=0, hmax=0;

        read_pod(in, &location);
        read_pod(in, &rangeDown);
        read_pod(in, &rangeUp);

        read_pod(in, &hmin);
        if (hmin)
            read_pod(in, &dmin);

        read_pod(in, &hmax);
        if (hmax)
            read_pod(in, &dmax);

        if (in.fail()) throw IOReadFailure("In npstat::BinSummary::restore: "
                                           "input stream failure");
        try
        {
            validateBinSummary("", location, rangeDown, rangeUp, dmin, dmax);
        }
        catch (std::invalid_argument& e)
        {
            throw IOInvalidData("In npstat::BinSummary::restore: "
                                "corrupted data");
        }

        assert(ptr);
        ptr->location_ = location;
        ptr->rangeDown_ = rangeDown;
        ptr->rangeUp_ = rangeUp;
        ptr->min_ = dmin;
        ptr->max_ = dmax;
    }
}
