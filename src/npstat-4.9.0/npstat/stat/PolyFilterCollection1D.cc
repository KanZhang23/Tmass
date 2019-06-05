#include <cassert>
#include <utility>
#include <stdexcept>

#include "geners/CPP11_auto_ptr.hh"

#include "npstat/stat/PolyFilterCollection1D.hh"

namespace npstat {
    PolyFilterCollection1D::PolyFilterCollection1D(
        const AbsScalableDistribution1D& kernel,
        const double* taper, const unsigned maxDegree,
        const double binwidth, const unsigned dataLen,
        const BoundaryHandling& bm)
        : scanBuf_(dataLen),
          kernel_(kernel.clone()),
          binwidth_(binwidth),
          taper_(0),
          maxDegree_(maxDegree),
          dataLen_(dataLen),
          bm_(bm)
    {
        if (binwidth <= 0.0) throw std::invalid_argument(
            "In npstat::PolyFilterCollection1D constructor: "
            "bin width must be positive");
        if (!dataLen) throw std::invalid_argument(
            "In npstat::PolyFilterCollection1D constructor: "
            "expected data length must be positive");
        if (taper)
        {
            taper_ = new double[maxDegree + 1];
            for (unsigned i=0; i<=maxDegree; ++i)
                taper_[i] = taper[i];
        }
        kernel_->setLocation(0.0);
    }

    PolyFilterCollection1D::~PolyFilterCollection1D()
    {
        delete kernel_;
        delete [] taper_;
        for (std::map<double,const LocalPolyFilter1D*>::iterator it =
                 filterMap_.begin(); it != filterMap_.end(); ++it)
            delete it->second;
    }

    const LocalPolyFilter1D& PolyFilterCollection1D::getPolyFilter(
        const double bandwidth)
    {
        if (bandwidth <= 0.0) throw std::invalid_argument(
            "In npstat::PolyFilterCollection1D::getPolyFilter: "
            "bandwidth must be positive");
        const LocalPolyFilter1D* filter = 0;
        std::map<double,const LocalPolyFilter1D*>::iterator it =
            filterMap_.find(bandwidth);
        if (it == filterMap_.end())
        {
            filter = processBandwidth(bandwidth);
            filterMap_.insert(std::make_pair(bandwidth, filter));
        }
        else
            filter = it->second;
        return *filter;
    }

    LocalPolyFilter1D* PolyFilterCollection1D::processBandwidth(
        const double bandwidth)
    {
        kernel_->setScale(bandwidth);

        // Construct filter builder
        CPP11_auto_ptr<AbsBoundaryFilter1DBuilder> builder =
            getBoundaryFilter1DBuilder(bm_, kernel_, binwidth_);

        // Construct and return the filter
        return new LocalPolyFilter1D(taper_, maxDegree_, *builder, dataLen_);
    }

    double PolyFilterCollection1D::taper(const unsigned i) const
    {
        if (i > maxDegree_)
            return 0.0;
        if (taper_)
            return taper_[i];
        else
            return 1.0;
    }
}
