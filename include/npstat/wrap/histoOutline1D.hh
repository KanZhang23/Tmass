#ifndef NPSTAT_HISTOOUTLINE1D_HH_
#define NPSTAT_HISTOOUTLINE1D_HH_

#include <stdexcept>

#include "npstat/wrap/NumpyTypecode.hh"

namespace npstat {
    //
    // Generate xdata, ydata sequence for the outline typically drawn
    // for histograms by such programs as "root" or Histo-Scope 
    //
    template <class Histo>
    PyObject* histoOutline1D(const Histo& h, const double yscale=1.0)
    {
        if (h.dim() != 1U)
            throw std::invalid_argument("npstat::histoOutline1D can only be "
                                        "used with 1-d histograms");
        const bool notUniform = !h.isUniformlyBinned();

        const unsigned long nbins = h.nBins();
        const int typenum = NumpyTypecode<double>::code;
        npy_intp sh = 2*nbins + 2;
        PyObject* xarr  = PyArray_SimpleNew(1, &sh, typenum);
        PyObject* yarr  = PyArray_SimpleNew(1, &sh, typenum);
        PyObject* result = NULL;

        if (xarr == NULL || yarr == NULL)
            goto fail;
        else
        {
            double* x = (double*)array_data(xarr);
            double* y = (double*)array_data(yarr);
            const typename Histo::value_type* data = h.binContents().data();
            const typename Histo::axis_type& ax(h.axis(0));
            unsigned long ix = 0, iy = 0;

            x[ix++] = (ax.leftBinEdge(0));
            y[iy++] = 0.0;
            for (unsigned long i=0; i<nbins; ++i)
            {
                double binScale = yscale;
                if (notUniform)
                    binScale /= h.binVolume(i);
                x[ix++] = (ax.leftBinEdge(i));
                y[iy++] = (static_cast<double>(data[i]))*binScale;
                x[ix++] = (ax.rightBinEdge(i));
                y[iy++] = (static_cast<double>(data[i]))*binScale;
            }
            x[ix++] = (ax.rightBinEdge(nbins-1U));
            y[iy++] = 0.0;
            assert(ix == static_cast<unsigned long>(sh));
            assert(iy == static_cast<unsigned long>(sh));

            result = Py_BuildValue("OO", xarr, yarr);
            if (result == NULL)
                goto fail;
        }

        return result;

    fail:
        Py_XDECREF(xarr);
        Py_XDECREF(yarr);
        return result;
    }

    //
    // Generate xdata, ydata sequence using bin center for x and
    // bin height for y
    //
    template <class Histo>
    PyObject* histoBinContents1D(const Histo& h, const double yscale=1.0)
    {
        if (h.dim() != 1U)
            throw std::invalid_argument("npstat::histoBinContents1D can only be "
                                        "used with 1-d histograms");
        const bool notUniform = !h.isUniformlyBinned();

        const unsigned long nbins = h.nBins();
        const int typenum = NumpyTypecode<double>::code;
        npy_intp sh = nbins;
        PyObject* xarr  = PyArray_SimpleNew(1, &sh, typenum);
        PyObject* yarr  = PyArray_SimpleNew(1, &sh, typenum);
        PyObject* result = NULL;

        if (xarr == NULL || yarr == NULL)
            goto fail;
        else
        {
            double* x = (double*)array_data(xarr);
            double* y = (double*)array_data(yarr);
            const typename Histo::value_type* data = h.binContents().data();
            const typename Histo::axis_type& ax(h.axis(0));

            for (unsigned long i=0; i<nbins; ++i)
            {
                double binScale = yscale;
                if (notUniform)
                    binScale /= h.binVolume(i);
                x[i] = (ax.binCenter(i));
                y[i] = (static_cast<double>(data[i]))*binScale;
            }

            result = Py_BuildValue("OO", xarr, yarr);
            if (result == NULL)
                goto fail;
        }

        return result;

    fail:
        Py_XDECREF(xarr);
        Py_XDECREF(yarr);
        return result;
    }
}

#endif // NPSTAT_HISTOOUTLINE1D_HH_
