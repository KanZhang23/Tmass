#ifndef NPSTAT_HISTOBARS_HH_
#define NPSTAT_HISTOBARS_HH_

#include <vector>
#include <stdexcept>

#include "npstat/nm/BoxND.hh"
#include "npstat/wrap/NumpyTypecode.hh"

namespace npstat {
    //
    // Matplotlib command (member of matplotlib.axes):
    //
    // bar(left, height, width=0.8, bottom=0, **kwargs)
    //
    // We are going to generate left, height, width, and bottom
    // (in this particular order in the result tuple) as instances
    // of PyArray.
    //
    // Input arguments are as follows:
    //
    //  h      -- The input histogram.
    //
    //  xscale -- The scale factor for the bin width. Value of 1
    //            corresponds to the normal histogram bin.
    //
    //  xshift -- Shift of the bar with respect to its normal position
    //            (centered in its bin slot), in the units of bin width.
    //
    //  yscale -- Vertical scale factor for the bars.
    //
    //  yshift -- Vertical shifts for the bars (in Y axis units, whatever
    //            they are). The vector of shifts can be empty in which
    //            case it is assumed that all shifts are 0. If the vector
    //            is not empty, its length must be equal to the number
    //            of histogram bins.
    //
    // If the histogram axis is uniformly binned, the result will be as
    // expected: when yscale = 1, the height of the bar will be set to
    // the number of entries in the bin. However, this makes a poor
    // representation of non-uniformly binned histograms. In this case
    // the area of the complete bar (not taking into account the "xscale"
    // factor) will be set to the number of entries. Therefore, the user
    // should figure out an appropriate "yscale" factor if some meaningful
    // correspondence between the bar height and the number of entries
    // is desired.
    //
    template <class Histo>
    PyObject* histoBars(
        const Histo& h, const double xscale, const double xshift,
        const double yscale, const std::vector<double>& yshift)
    {
        if (h.dim() != 1U)
            throw std::invalid_argument("npstat::histoBars can only be used "
                                        "with 1-d histograms");
        const bool notUniform = !h.isUniformlyBinned();

        const unsigned long nbins = h.nBins();
        const unsigned long nYshift = yshift.size();
        if (nYshift && nYshift != nbins)
            throw std::invalid_argument("npstat::histoBars: incompatible "
                                        "Y shift info");

        const int typenum = NumpyTypecode<double>::code;
        npy_intp sh = nbins;
        PyObject* leftarr = PyArray_SimpleNew(1, &sh, typenum);
        PyObject* heightarr = PyArray_SimpleNew(1, &sh, typenum);
        PyObject* widtharr = PyArray_SimpleNew(1, &sh, typenum);
        PyObject* bottomarr = PyArray_SimpleNew(1, &sh, typenum);
        PyObject* result = NULL;

        if (leftarr == NULL || heightarr == NULL || 
            widtharr == NULL || bottomarr == NULL)
            goto fail;
        else
        {
            // Cycle over bins
            BoxND<double> box;
            double boxCenter[1], scales[1];
            scales[0] = xscale;
            const typename Histo::value_type* data = h.binContents().data();

            double* left = (double*)array_data(leftarr);
            double* height = (double*)array_data(heightarr);
            double* width = (double*)array_data(widtharr);
            double* bottom = (double*)array_data(bottomarr);

            for (unsigned long ibin=0; ibin<nbins; ++ibin)
            {
                h.binBox(ibin, &box);
                box.getMidpoint(boxCenter, 1);
                boxCenter[0] += box[0].length()*xshift;
                box.moveToOrigin();
                box.expand(scales, 1);
                box.shift(boxCenter, 1);

                left[ibin] = box[0].min();
                width[ibin] = box[0].length();
                bottom[ibin] = nYshift ? yshift[ibin] : 0.0;

                // Figure out the bar height
                double binScale = yscale;
                if (notUniform)
                    binScale /= h.binVolume(ibin);
                height[ibin] = static_cast<double>(data[ibin])*binScale;
            }

            result = Py_BuildValue("OOOO", leftarr, heightarr,
                                   widtharr, bottomarr);
            if (result == NULL)
                goto fail;
        }

        return result;

    fail:
        Py_XDECREF(leftarr);
        Py_XDECREF(heightarr);
        Py_XDECREF(widtharr);
        Py_XDECREF(bottomarr);

        return result;
    }

    //
    // Matplotlib command (member of mpl_toolkits.mplot3d.axes3d):
    //
    // bar3d(x, y, z, dx, dy, dz, color='b', zsort='average', *args, **kwargs)
    //
    // We are going to generate x, y, z, dx, dy, dz.
    //
    // Input arguments are similar to "histoBars".
    //
    template <class Histo>
    PyObject* histoBars3d(
        const Histo& h, const double xscale, const double xshift,
        const double yscale, const double yshift, const double zscale,
        const std::vector<double>& zshift)
    {
        if (h.dim() != 2U)
            throw std::invalid_argument("npstat::histoBars3d can only be used "
                                        "with 2-d histograms");
        const bool notUniform = !h.isUniformlyBinned();

        const unsigned long nbins = h.nBins();
        const unsigned long nZshift = zshift.size();
        if (nZshift && nZshift != nbins)
            throw std::invalid_argument("npstat::histoBars3d: incompatible "
                                        "Z shift info");

        const int typenum = NumpyTypecode<double>::code;
        npy_intp sh = nbins;
        PyObject* xarr  = PyArray_SimpleNew(1, &sh, typenum);
        PyObject* yarr  = PyArray_SimpleNew(1, &sh, typenum);
        PyObject* zarr  = PyArray_SimpleNew(1, &sh, typenum);
        PyObject* dxarr = PyArray_SimpleNew(1, &sh, typenum);
        PyObject* dyarr = PyArray_SimpleNew(1, &sh, typenum);
        PyObject* dzarr = PyArray_SimpleNew(1, &sh, typenum);
        PyObject* result = NULL;

        if (xarr == NULL || yarr == NULL || zarr == NULL ||
            dxarr == NULL || dyarr == NULL || dzarr == NULL)
            goto fail;
        else
        {
            double* x = (double*)array_data(xarr);
            double* y = (double*)array_data(yarr);
            double* z = (double*)array_data(zarr);
            double* dx = (double*)array_data(dxarr);
            double* dy = (double*)array_data(dyarr);
            double* dz = (double*)array_data(dzarr);

            // Cycle over bins
            BoxND<double> box;
            double boxCenter[2], scales[2];
            scales[0] = xscale;
            scales[1] = yscale;
            const typename Histo::value_type* data = h.binContents().data();

            for (unsigned long ibin=0; ibin<nbins; ++ibin)
            {
                h.binBox(ibin, &box);
                box.getMidpoint(boxCenter, 2);
                boxCenter[0] += box[0].length()*xshift;
                boxCenter[1] += box[1].length()*yshift;
                box.moveToOrigin();
                box.expand(scales, 2);
                box.shift(boxCenter, 2);

                x[ibin] = (box[0].min());
                y[ibin] = (box[1].min());
                dx[ibin] = (box[0].length());
                dy[ibin] = (box[1].length());
                z[ibin] = (nZshift ? zshift[ibin] : 0.0);

                double binScale = zscale;
                if (notUniform)
                    binScale /= h.binVolume(ibin);
                dz[ibin] = static_cast<double>(data[ibin])*binScale;
            }

            result = Py_BuildValue("OOOOOO", xarr, yarr, zarr,
                                   dxarr, dyarr, dzarr);
            if (result == NULL)
                goto fail;
        }

        return result;

    fail:
        Py_XDECREF(xarr);
        Py_XDECREF(yarr);
        Py_XDECREF(zarr);
        Py_XDECREF(dxarr);
        Py_XDECREF(dyarr);
        Py_XDECREF(dzarr);

        return result;
    }
}

#endif // NPSTAT_HISTOBARS_HH_
