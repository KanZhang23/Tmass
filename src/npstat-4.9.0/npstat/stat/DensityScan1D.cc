#include "npstat/stat/DensityScan1D.hh"

#include "npstat/nm/GaussLegendreQuadrature.hh"

namespace npstat {
    DensityScan1D::DensityScan1D(const AbsDistribution1D& fcn, 
                                 const double normfactor,
                                 const unsigned long nbins,
                                 const double xmin, const double xmax,
                                 const unsigned nIntegrationPoints)
        : fcn_(fcn),
          norm_(normfactor),
          xmin_(xmin),
          bw_((xmax-xmin)/nbins),
          nPoints_(nIntegrationPoints)
    {
        if (!nbins) throw std::invalid_argument(
            "In npstat::DensityScan1D constructor: "
            "number of bins must be positive");
        if (nPoints_ > 1U)
        {
            GaussLegendreQuadrature q(nPoints_);
            if (!q.npoints()) throw std::invalid_argument(
                "In npstat::DensityScan1D constructor: this number "
                "of integration points per bin is not supported by "
                "npstat::GaussLegendreQuadrature");
            weights_.resize(nPoints_/2U);
            a_.resize(nPoints_/2U);
            q.getWeights(&weights_[0], weights_.size());
            q.getAbscissae(&a_[0], a_.size());
        }
    }

    double DensityScan1D::operator()(const unsigned* index, unsigned len) const
    {
        if (len != 1U) throw std::invalid_argument(
            "In npstat::DensityScan1D::operator(): "
            "incompatible dimensionality of the input index");
        assert(index);
        if (nPoints_)
        {
            const double midpoint = xmin_ + (*index + 0.5)*bw_;
            if (nPoints_ == 1U)
                return fcn_.density(midpoint)*norm_;
            else
            {
                const long double *w = &weights_[0];
                const long double *ab = &a_[0];
                const unsigned halfpoints = nPoints_/2U;
                long double sum = 0.0L;
                for (unsigned i=0; i<halfpoints; ++i)
                {
                    const double delta = bw_/2.0*static_cast<double>(ab[i]);
                    sum += w[i]*fcn_.density(midpoint - delta);
                    sum += w[i]*fcn_.density(midpoint + delta);
                }
                return norm_*static_cast<double>(sum/2.0L);
            }
        }
        else
        {
            const double leftedge = xmin_ + *index*bw_;
            const double rightedge = leftedge + bw_;
            return norm_*(fcn_.cdf(rightedge) - fcn_.cdf(leftedge))/bw_;
        }
    }
}
