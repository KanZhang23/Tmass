#include <algorithm>
#include <utility>
#include <stdexcept>

#include "npstat/nm/LinearMapper1d.hh"

#include "npstat/stat/OSDE1D.hh"

namespace npstat {
    OSDE1D::OSDE1D(const AbsClassicalOrthoPoly1D& poly1d,
                   const double xlo, const double xhi)
        : poly_(poly1d.clone()), xmin_(xlo), xmax_(xhi)
    {
        if (xmin_ == xmax_) throw std::invalid_argument(
            "In npstat::OSDE1D constructor: "
            "density estimation interval has zero length");
        if (xmin_ > xmax_)
            std::swap(xmin_, xmax_);
        const double polyWidth = poly_->xmax() - poly_->xmin();
        scale_ = (xmax_ - xmin_)/polyWidth;
        shift_ = (poly_->xmax()*xmin_ - poly_->xmin()*xmax_)/polyWidth;
    }

    OSDE1D::OSDE1D(const OSDE1D& r)
        : poly_(r.poly_->clone()),
          xmin_(r.xmin_), xmax_(r.xmax_),
          shift_(r.shift_), scale_(r.scale_)
    {
    }

    OSDE1D& OSDE1D::operator=(const OSDE1D& r)
    {
        if (&r != this)
        {
            delete poly_; poly_ = 0;
            poly_ = r.poly_->clone();
            xmin_ = r.xmin_;
            xmax_ = r.xmax_;
            shift_ = r.shift_;
            scale_ = r.scale_;
        }
        return *this;
    }

    OSDE1D::~OSDE1D()
    {
        delete poly_;
    }

    double OSDE1D::series(const double *coeffs, const unsigned maxdeg,
                          const double x) const
    {
        return poly_->series(coeffs, maxdeg, (x - shift_)/scale_);
    }

    double OSDE1D::weight(const double x) const
    {
        return poly_->weight((x - shift_)/scale_)/scale_;
    }

    unsigned OSDE1D::optimalDegreeHart(const double* coeffs,
                                       const double* variances,
                                       const unsigned maxdeg,
                                       const double k)
    {
        assert(coeffs);
        assert(variances);

        long double sum = 0.0L, bestSum = 0.0L;
        unsigned bestM = 0;

        for (unsigned i=1U; i<=maxdeg; ++i)
        {
            sum += k*variances[i] - coeffs[i]*coeffs[i];
            if (sum < bestSum)
            {
                bestSum = sum;
                bestM = i;
            }
        }
        return bestM;
    }

    std::pair<double,double> OSDE1D::supportRegion(
        const AbsClassicalOrthoPoly1D& poly1d,
        const double leftLimit, const bool leftIsFromSample,
        const double rightLimit, const bool rightIsFromSample,
        const unsigned long nPoints)
    {
        if (leftLimit >= rightLimit) throw std::invalid_argument(
            "In npstat::OSDE1D::supportRegion: "
            "the left limit value must be less than the right limit value");
        if (!(leftIsFromSample || rightIsFromSample))
            return std::pair<double,double>(leftLimit, rightLimit);

        const unsigned nMin = leftIsFromSample && rightIsFromSample ? 2U : 1U;
        if (nPoints < nMin) throw std::invalid_argument(
            "In npstat::OSDE1D::supportRegion: insufficient sample size");

        const double xmin = poly1d.xmin();
        const double xmax = poly1d.xmax();
        const double halfstep = (xmax - xmin)/2.0/nPoints;

        if (leftIsFromSample && rightIsFromSample)
        {
            LinearMapper1d mp(xmin + halfstep, leftLimit,
                              xmax - halfstep, rightLimit);
            return std::pair<double,double>(mp(xmin), mp(xmax));
        }
        else if (leftIsFromSample)
        {
            LinearMapper1d mp(xmin + halfstep, leftLimit,
                              xmax, rightLimit);
            return std::pair<double,double>(mp(xmin), rightLimit);
        }
        else
        {
            LinearMapper1d mp(xmin, leftLimit,
                              xmax - halfstep, rightLimit);
            return std::pair<double,double>(leftLimit, mp(xmax));
        }
    }
}
