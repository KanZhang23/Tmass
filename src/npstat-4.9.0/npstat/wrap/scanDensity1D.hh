#ifndef NPSTAT_SCANDENSITY1D_HH_
#define NPSTAT_SCANDENSITY1D_HH_

#include "npstat/stat/AbsDistribution1D.hh"
#include "npstat/wrap/NumpyTypecode.hh"

namespace npstat {
    inline PyObject* scanDensity1D(
        const AbsDistribution1D& distro,
        const unsigned npoints, const double xmin, const double xmax)
    {
        const int typenum = NumpyTypecode<double>::code;
        npy_intp sh = npoints;
        PyObject* xarr  = PyArray_SimpleNew(1, &sh, typenum);
        PyObject* yarr  = PyArray_SimpleNew(1, &sh, typenum);
        PyObject* result = NULL;

        if (xarr == NULL || yarr == NULL)
            goto fail;
        else if (npoints)
        {
            double* xv = (double*)array_data(xarr);
            double* yv = (double*)array_data(yarr);
            const double step = (xmax - xmin)/npoints;
            for (unsigned i=0; i<npoints; ++i)
            {
                const double x = xmin + (i + 0.5)*step;
                xv[i] = x;
                yv[i] = distro.density(x);
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

#endif // NPSTAT_SCANDENSITY1D_HH_
