#include <cassert>

#include "npstat/nm/ConvolutionEngine1D.hh"

namespace npstat {
    ConvolutionEngine1D::ConvolutionEngine1D(
        const unsigned dataLen, const unsigned optimization)
        : dataLen_(dataLen),
          validFilter_(false)
    {
        if (!dataLen) throw std::invalid_argument(
            "In npstat::ConvolutionEngine1D constructor: "
            "expected data length must be positive");

        // fftw_malloc ensures optimal alignment of the allocated buffers
        in = static_cast<double *>(fftw_malloc(
             dataLen * sizeof(double)));
        assert(in);
        out = static_cast<fftw_complex *>(fftw_malloc(
              (dataLen/2 + 1) * sizeof(fftw_complex)));
        assert(out);
        filterImage = static_cast<fftw_complex *>(fftw_malloc(
              (dataLen/2 + 1) * sizeof(fftw_complex)));
        assert(filterImage);

        const unsigned plannerFlags = optimization | FFTW_DESTROY_INPUT;
        pf = fftw_plan_dft_r2c_1d(dataLen, in, out, plannerFlags);
        pb = fftw_plan_dft_c2r_1d(dataLen, out, in, plannerFlags);
        assert(pf);
        assert(pb);
    }

    ConvolutionEngine1D::~ConvolutionEngine1D()
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

    bool ConvolutionEngine1D::discardFilter(const unsigned long id)
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
