#include <cassert>

#include "npstat/nm/ConvolutionEngineND.hh"

namespace npstat {
    ConvolutionEngineND::ConvolutionEngineND(
        const unsigned *sh, const unsigned rank, const unsigned optimization)
        : shape_(sh, sh+rank),
          dataLen_(1UL),
          validFilter_(false)
    {
        if (!rank) throw std::invalid_argument(
            "In npstat::ConvolutionEngineND constructor: "
            "expected data dimensionality must be positive");
        assert(sh);

        std::vector<int> ish(rank);
        for (unsigned i=0; i<rank; ++i)
        {
            ish[i] = sh[i];
            dataLen_ *= sh[i];
        }
        cmplLen_ = dataLen_/sh[rank-1U]*(sh[rank-1U]/2U + 1U);

        // fftw_malloc ensures optimal alignment of the allocated buffers
        in = static_cast<double *>(fftw_malloc(
              dataLen_ * sizeof(double)));
        assert(in);
        out = static_cast<fftw_complex *>(fftw_malloc(
              cmplLen_ * sizeof(fftw_complex)));
        assert(out);
        filterImage = static_cast<fftw_complex *>(fftw_malloc(
              cmplLen_ * sizeof(fftw_complex)));
        assert(filterImage);

        const unsigned plannerFlags = optimization | FFTW_DESTROY_INPUT;
        pf = fftw_plan_dft_r2c(rank, &ish[0], in, out, plannerFlags);
        pb = fftw_plan_dft_c2r(rank, &ish[0], out, in, plannerFlags);
        assert(pf);
        assert(pb);
    }

    ConvolutionEngineND::~ConvolutionEngineND()
    {
        fftw_destroy_plan(pb);
        fftw_destroy_plan(pf);
        fftw_free(filterImage);
        fftw_free(out);
        fftw_free(in);

        for (FilterMap::iterator it = filterMap_.begin();
             it != filterMap_.end(); ++it)
            delete it->second;
    }

    bool ConvolutionEngineND::isShapeCompatible(const unsigned *dataShape,
                                                const unsigned dataRank) const
    {
        if (shape_.size() != dataRank)
            return false;
        assert(dataShape);
        for (unsigned i=0; i<dataRank; ++i)
            if (dataShape[i] != shape_[i])
                return false;
        return true;
    }

    bool ConvolutionEngineND::discardFilter(const unsigned long id)
    {
        FilterMap::iterator it = filterMap_.find(id);
        if (it != filterMap_.end())
        {
            delete it->second;
            filterMap_.erase(it);
            return true;
        }
        else
            return false;
    }
}
