#include <cassert>
#include <stdexcept>
#include <sstream>

#include "geners/Record.hh"
#include "npstat/stat/AbsCopulaSmootherBase.hh"

namespace npstat{
    AbsCopulaSmootherBase::AbsCopulaSmootherBase(
        const unsigned* nBinsInEachDim, const unsigned dim,
        const double tolerance, const unsigned maxNormCycles)
        : h_(0), tol_(tolerance), dim_(dim), nCycles_(maxNormCycles), ar_(0)
    {
        if (dim < 2 || dim >= CHAR_BIT*sizeof(unsigned long))
            throw std::invalid_argument(
                "In npstat::AbsCopulaSmootherBase constructor: "
                "invalid number of copula dimensions");
        assert(nBinsInEachDim);
        for (unsigned i=0; i<dim; ++i)
        {
            shape_[i] = nBinsInEachDim[i];
            if (shape_[i] < 2) throw std::invalid_argument(
                "In npstat::AbsCopulaSmootherBase constructor: "
                "invalid number of bins");
        }
    }

    void AbsCopulaSmootherBase::makeMarginalsUniform()
    {
        if (!h_) throw std::runtime_error(
            "In npstat::AbsCopulaSmootherBase::makeMarginalsUniform: "
            "no histogram to process");
        if (nCycles_)
        {
            const unsigned cyclesMade = (const_cast<ArrayND<double>&>(
                h_->binContents())).makeCopulaSteps(tol_, nCycles_);
            if (cyclesMade >= nCycles_) throw std::runtime_error(
                "In npstat::AbsCopulaSmootherBase::makeMarginalsUniform: "
                "failed to achieve requested tolerance of the uniform "
                "marginal distribution");
        }
    }

    void AbsCopulaSmootherBase::setArchive(gs::AbsArchive* ar,
                                           const char* category)
    {
        ar_ = ar;
        category_ = std::string(category ? category : "");
    }

    HistoND<double>& AbsCopulaSmootherBase::clearHisto()
    {
        if (h_)
            h_->clear();
        else
        {
            std::vector<HistoAxis> axes;
            axes.reserve(dim_);
            for (unsigned i=0; i<dim_; ++i)
            {
                std::ostringstream os;
                os << "X_" << i;
                const std::string& label = os.str();
                axes.push_back(HistoAxis(shape_[i], 0.0, 1.0, label.c_str()));
            }
            h_ = new HistoND<double>(axes);
        }
        return *h_;
    }

    void AbsCopulaSmootherBase::storeHisto(const unsigned long uniqueId,
                                           const double bw) const
    {
        if (ar_ && h_)
        {
            std::ostringstream os;
            if (bw > 0.0)
                os << "Smoothed";
            else
                os << "Binned";
            os << " copula, id " << uniqueId;
            if (bw > 0.0)
                os << ", bw " << bw;
            *ar_ << gs::Record(*h_, os.str(), category_);
        }
    }
}
