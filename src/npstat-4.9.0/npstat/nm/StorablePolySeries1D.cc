#include <cassert>
#include <stdexcept>

#include "geners/GenericIO.hh"
#include "geners/IOException.hh"

#include "npstat/nm/StorablePolySeries1D.hh"
#include "npstat/nm/GaussLegendreQuadrature.hh"

namespace npstat {
    namespace Private {
        class StorablePolyPower : public Functor1<double, double>
        {
        public:
            inline StorablePolyPower(const StorablePolySeries1D& i_poly,
                                     const unsigned deg, const unsigned power)
                : poly(i_poly), deg1(deg), polypow(power) {}

            inline double operator()(const double& x) const
            {
                double p = 1.0;
                if (polypow)
                {
                    p = poly.poly(deg1, x);
                    switch (polypow)
                    {
                    case 1U:
                        break;
                    case 2U:
                        p *= p;
                        break;
                    default:
                        p = pow(p, polypow);
                        break;
                    }
                }
                return p;
            }

        private:
            const StorablePolySeries1D& poly;
            unsigned deg1;
            unsigned polypow;
        };
    }

    StorablePolySeries1D::StorablePolySeries1D(
        const std::vector<std::pair<long double,long double> >& rCoeffs,
        const double xmin, const double xmax, const double shift,
        const double *coeffs, const unsigned maxdeg)
        : recur_(rCoeffs), xmin_(xmin), xmax_(xmax), shift_(shift)
    {
        if (recur_.empty()) throw std::invalid_argument(
            "In npstat::StorablePolySeries1D constructor: "
            "empty set of recurrence coefficients");
        if (coeffs)
            setCoeffs(coeffs, maxdeg);
    }

    void StorablePolySeries1D::setCoeffs(const double *coeffs,
                                         const unsigned maxdeg)
    {
        if (maxdeg > this->maxDegree()) throw std::invalid_argument(
            "In npstat::StorablePolySeries1D::setCoeffs: "
            "maximum degree argument is out of range");
        assert(coeffs);
        coeffs_.clear();
        coeffs_.reserve(maxdeg + 1U);
        for (unsigned i=0; i<=maxdeg; ++i)
            coeffs_.push_back(coeffs[i]);
    }

    double StorablePolySeries1D::integratePoly(
        const unsigned degree, const unsigned power,
        const double xmin, const double xmax) const
    {
        const unsigned degSum = degree*power;
        if (degSum)
        {
            const unsigned nPt = GaussLegendreQuadrature::minimalExactRule(degSum);
            if (!nPt) throw std::invalid_argument(
                "In npstat::StorablePolySeries1D::integratePoly: "
                "poly degree or power is too high");
            GaussLegendreQuadrature quad(nPt);
            Private::StorablePolyPower fcn(*this, degree, power);
            return quad.integrate(fcn, xmin, xmax);
        }
        else
            return xmax - xmin;
    }

    long double StorablePolySeries1D::weight(long double /* x */) const
    {
        throw std::runtime_error("In npstat::StorablePolySeries1D::weight: "
                                 "this function should be unreachable. "
                                 "This is a bug. Please report.");
        return 0.0L;
    }

    bool StorablePolySeries1D::isEqual(const StorablePolySeries1D& other) const
    {
        const StorablePolySeries1D& r = static_cast<const StorablePolySeries1D&>(other);
        return coeffs_ == r.coeffs_ &&
               recur_ == r.recur_ &&
               xmin_ == r.xmin_ &&
               xmax_ == r.xmax_ &&
               shift_ == r.shift_;
    }

    bool StorablePolySeries1D::write(std::ostream& os) const
    {
        gs::write_item(os, coeffs_);
        gs::write_item(os, recur_);
        gs::write_pod(os, xmin_);
        gs::write_pod(os, xmax_);
        gs::write_pod(os, shift_);
        return !os.fail();
    }

    StorablePolySeries1D* StorablePolySeries1D::read(
        const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<StorablePolySeries1D>());
        current.ensureSameId(id);

        CPP11_auto_ptr<StorablePolySeries1D> p(new StorablePolySeries1D());
        gs::restore_item(in, &p->coeffs_);
        gs::restore_item(in, &p->recur_);
        gs::read_pod(in, &p->xmin_);
        gs::read_pod(in, &p->xmax_);
        gs::read_pod(in, &p->shift_);
        if (in.fail()) throw gs::IOReadFailure(
            "In npstat::StorablePolySeries1D::read: input stream failure");
        return p.release();
    }
}
