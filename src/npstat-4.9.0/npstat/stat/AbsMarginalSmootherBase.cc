#include <stdexcept>
#include <sstream>

#include "geners/Record.hh"

#include "npstat/nm/Interval.hh"
#include "npstat/stat/AbsMarginalSmootherBase.hh"

namespace npstat {
    AbsMarginalSmootherBase::AbsMarginalSmootherBase(
        const unsigned nbins, const double xmin,
        const double xmax, const char* axisLabel)
        : histo_(0), xmin_(xmin), xmax_(xmax), nbins_(nbins), ar_(0)
    {
        if (xmin_ >= xmax_ || nbins_ < 1U) throw std::invalid_argument(
            "In npstat::AbsMarginalSmootherBase constructor: "
            "invalid histogram axis specification");
        setAxisLabel(axisLabel);
    }

    void AbsMarginalSmootherBase::setAxisLabel(const char* axisLabel)
    {
        axlabel_ = std::string(axisLabel ? axisLabel : "");
    }

    void AbsMarginalSmootherBase::setArchive(gs::AbsArchive* ar,
                                             const char* category)
    {
        ar_ = ar;
        category_ = std::string(category ? category : "");
    }

    HistoND<double>& AbsMarginalSmootherBase::clearHisto(const double xleft,
                                                         const double xright)
    {
        double newMin = xmin_, newMax = xmax_;
        if (xleft < xright)
        {
            const Interval<double> mask(xmin_, xmax_);
            const Interval<double> request(xleft, xright);
            const Interval<double>& limits = mask.overlap(request);
            if (limits.length() == 0.0) throw std::runtime_error(
                "In AbsMarginalSmootherBase::clearHisto: "
                "zero overlap with the requested interval");
            newMin = limits.min();
            newMax = limits.max();
        }
        if (histo_)
        {
            const HistoAxis& axis(histo_->axis(0));
            if (!(axis.min() == newMin && axis.max() == newMax))
            {
                delete histo_;
                histo_ = 0;
            }
        }
        if (histo_)
            histo_->clear();
        else
        {
            const char* l = 0;
            if (!axlabel_.empty()) l = axlabel_.c_str();
            histo_ = new HistoND<double>(HistoAxis(nbins_, newMin, newMax, l));
        }
        return *histo_;
    }

    void AbsMarginalSmootherBase::storeHisto(const unsigned long uniqueId,
                                             const unsigned dim,
                                             const double bw) const
    {
        if (ar_ && histo_)
        {
            std::ostringstream os;
            if (bw > 0.0)
                os << "Smoothed";
            else
                os << "Binned";
            os << " marginal, id " << uniqueId << ", dim " << dim;
            if (bw > 0.0)
                os << ", bw " << bw;
            *ar_ << gs::Record(*histo_, os.str(), category_);
        }
    }
}
