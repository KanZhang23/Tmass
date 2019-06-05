#include <assert.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>

#include "histoscope.h"
#include "linear_interpolator_nd.h"
#include "linear_interpolator_nd_api.h"

#ifdef __cplusplus
extern "C" {
#endif

static Tcl_Interp *commandInterp = 0;

int histo_dim_from_type(int type);

#define tcl_require_objc(N) do {\
  if (objc != N)\
  {\
      Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[0],NULL),\
                       " : wrong # of arguments", NULL);\
      return TCL_ERROR;\
  }\
} while(0);

#define tcl_objc_range(N,M) do {\
  if (objc < N || objc > M)\
  {\
      Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[0],NULL),\
                       " : wrong # of arguments", NULL);\
      return TCL_ERROR;\
  }\
} while(0);

static int parse_integ_limits(Tcl_Interp *interp, Tcl_Obj *obj,
                              float *xmin, float *xmax, int *nbins)
{
    int listlen;
    Tcl_Obj **listObjElem;
    double d;

    if (Tcl_ListObjGetElements(interp, obj, &listlen,
                               &listObjElem) != TCL_OK)
        return TCL_ERROR;
    if (listlen != 3)
    {
        Tcl_SetResult(interp, "invalid integration limits", TCL_STATIC);
        return TCL_ERROR;
    }
    if (Tcl_GetDoubleFromObj(interp, listObjElem[0], &d) != TCL_OK)
        return TCL_ERROR;
    *xmin = d;
    if (Tcl_GetDoubleFromObj(interp, listObjElem[1], &d) != TCL_OK)
        return TCL_ERROR;
    *xmax = d;
    if (Tcl_GetIntFromObj(interp, listObjElem[2], nbins) != TCL_OK)
        return TCL_ERROR;
    if (*nbins <= 0)
    {
        Tcl_SetResult(interp, "number of integration points must be positive",
                      TCL_STATIC);
        return TCL_ERROR;
    }
    return TCL_OK;
}

static void linear_interpolator_nd_cleanup(ClientData clientData)
{
    Interpolator_data_nd *d = (Interpolator_data_nd *)clientData;
    destroy_interpolator_nd(d);
}

static int linear_interpolator_nd_handle(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    char *op;
    const Interpolator_data_nd *d = (Interpolator_data_nd *)clientData;

    assert(d);
    tcl_objc_range(2, INT_MAX);
    op = Tcl_GetStringFromObj(objv[1], NULL);

    if (strcmp(op, "value") == 0)
    {
        /* Usage: handle value coords */
        float coords[3];
        int i, listlen;
        Tcl_Obj **listObjElem;

        tcl_require_objc(3);
        if (Tcl_ListObjGetElements(interp, objv[2], &listlen,
                                   &listObjElem) != TCL_OK)
            return TCL_ERROR;
        if (listlen != (int)d->dim)
        {
            Tcl_SetResult(interp, "incompatible list length", TCL_STATIC);
            return TCL_ERROR;
        }
        for (i=0; i<listlen; ++i)
        {
            double val;
            if (Tcl_GetDoubleFromObj(interp, listObjElem[i], &val) != TCL_OK)
                return TCL_ERROR;
            coords[i] = val;
        }
        Tcl_SetObjResult(interp, Tcl_NewDoubleObj(
                             linear_interpolate_nd(d, coords)));
    }
    else if (strcmp(op, "scan") == 0)
    {
        /* Usage: handle scan id */
        int histo_id, dim;
        float coords[3];

        tcl_require_objc(3);
        if (Tcl_GetIntFromObj(interp, objv[2], &histo_id) != TCL_OK)
            return TCL_ERROR;
        dim = histo_dim_from_type(hs_type(histo_id));
        if (dim != (int)d->dim)
        {
            Tcl_SetResult(interp, "incompatible dimensionality", TCL_STATIC);
            return TCL_ERROR;
        }
        switch (dim)
        {
        case 1:
            {
                float xmin, xmax, bwx;
                int i;
                int n_x_bins = hs_1d_hist_num_bins(histo_id);
                hs_1d_hist_range(histo_id, &xmin, &xmax);
                bwx = (xmax - xmin)/n_x_bins;
                for (i=0; i<n_x_bins; ++i)
                {
                    coords[0] = (i + 0.5f)*bwx + xmin;
                    hs_1d_hist_set_bin(histo_id, i,
                                       linear_interpolate_nd(d, coords));
                }
            }
            break;
        case 2:
            {
                float xmin, xmax, ymin, ymax, bwx, bwy;
                int i, j, n_x_bins, n_y_bins;
                hs_2d_hist_num_bins(histo_id, &n_x_bins, &n_y_bins);
                hs_2d_hist_range(histo_id, &xmin, &xmax, &ymin, &ymax);
                bwx = (xmax - xmin)/n_x_bins;
                bwy = (ymax - ymin)/n_y_bins;
                for (i=0; i<n_x_bins; ++i)
                {
                    coords[0] = (i + 0.5f)*bwx + xmin;
                    for (j=0; j<n_y_bins; ++j)
                    {
                        coords[1] = (j + 0.5f)*bwy + ymin;
                        hs_2d_hist_set_bin(histo_id, i, j,
                                           linear_interpolate_nd(d, coords));
                    }
                }
            }
            break;
        case 3:
            {
                float xmin, xmax, ymin, ymax, zmin, zmax, bwx, bwy, bwz;
                int i, j, k, n_x_bins, n_y_bins, n_z_bins;
                hs_3d_hist_num_bins(histo_id, &n_x_bins, &n_y_bins, &n_z_bins);
                hs_3d_hist_range(histo_id, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
                bwx = (xmax - xmin)/n_x_bins;
                bwy = (ymax - ymin)/n_y_bins;
                bwz = (zmax - zmin)/n_z_bins;
                for (i=0; i<n_x_bins; ++i)
                {
                    coords[0] = (i + 0.5f)*bwx + xmin;
                    for (j=0; j<n_y_bins; ++j)
                    {
                        coords[1] = (j + 0.5f)*bwy + ymin;
                        for (k=0; k<n_z_bins; ++k)
                        {
                            coords[2] = (k + 0.5f)*bwz + zmin;
                            hs_3d_hist_set_bin(histo_id, i, j, k,
                                               linear_interpolate_nd(d, coords));
                        }
                    }
                }
            }
            break;
        default:
            assert(0);
        }
    }
    else if (strcmp(op, "integrate") == 0)
    {
        /* Usage: handle integrate lim_x lim_y ...
         * 
         * Each lim_x, lim_y, ... is a list {xmin xmax nbins}
         *
         * Integration here is a dummy rectangular integration.
         */
        float coords[3];
        long double sum = 0.0L;

        tcl_require_objc(2 + (int)d->dim);
        switch (d->dim)
        {
        case 1:
            {
                float xmin, xmax, bwx;
                int i, n_x_bins;
                if (parse_integ_limits(interp, objv[2], &xmin, &xmax, &n_x_bins) != TCL_OK)
                    return TCL_ERROR;
                bwx = (xmax - xmin)/n_x_bins;
                for (i=0; i<n_x_bins; ++i)
                {
                    coords[0] = (i + 0.5f)*bwx + xmin;
                    sum += linear_interpolate_nd(d, coords);
                }
                sum *= bwx;
            }
            break;
        case 2:
            {
                float xmin, xmax, ymin, ymax, bwx, bwy;
                int i, j, n_x_bins, n_y_bins;
                if (parse_integ_limits(interp, objv[2], &xmin, &xmax, &n_x_bins) != TCL_OK)
                    return TCL_ERROR;
                if (parse_integ_limits(interp, objv[3], &ymin, &ymax, &n_y_bins) != TCL_OK)
                    return TCL_ERROR;
                bwx = (xmax - xmin)/n_x_bins;
                bwy = (ymax - ymin)/n_y_bins;
                for (i=0; i<n_x_bins; ++i)
                {
                    coords[0] = (i + 0.5f)*bwx + xmin;
                    for (j=0; j<n_y_bins; ++j)
                    {
                        coords[1] = (j + 0.5f)*bwy + ymin;
                        sum += linear_interpolate_nd(d, coords);
                    }
                }
                sum *= (bwx*bwy);
            }
            break;
        case 3:
            {
                float xmin, xmax, ymin, ymax, zmin, zmax, bwx, bwy, bwz;
                int i, j, k, n_x_bins, n_y_bins, n_z_bins;
                if (parse_integ_limits(interp, objv[2], &xmin, &xmax, &n_x_bins) != TCL_OK)
                    return TCL_ERROR;
                if (parse_integ_limits(interp, objv[3], &ymin, &ymax, &n_y_bins) != TCL_OK)
                    return TCL_ERROR;
                if (parse_integ_limits(interp, objv[4], &zmin, &zmax, &n_z_bins) != TCL_OK)
                    return TCL_ERROR;
                bwx = (xmax - xmin)/n_x_bins;
                bwy = (ymax - ymin)/n_y_bins;
                bwz = (zmax - zmin)/n_z_bins;
                for (i=0; i<n_x_bins; ++i)
                {
                    coords[0] = (i + 0.5f)*bwx + xmin;
                    for (j=0; j<n_y_bins; ++j)
                    {
                        coords[1] = (j + 0.5f)*bwy + ymin;
                        for (k=0; k<n_z_bins; ++k)
                        {
                            coords[2] = (k + 0.5f)*bwz + zmin;
                            sum += linear_interpolate_nd(d, coords);
                        }
                    }
                }
                sum *= (bwx*bwy*bwz);
            }
            break;
        default:
            assert(0);
        }
        Tcl_SetObjResult(interp, Tcl_NewDoubleObj(sum));
    }
    else
    {
        Tcl_AppendResult(interp, "invalid linear_interpolator_nd operation \"",
                         op, "\"", NULL);
        return TCL_ERROR;
    }

    return TCL_OK;
}

static int c_create_linear_interpolator_nd(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: create_linear_interpolator_nd histo_id */
    int i, histo_dim, histo_id, nbins = 1;
    static int n_cmd = 0;
    Interpolator_data_nd *d = 0;
    char namebuf[64];
    Interpolator_axis axes[3];
    float *data;

    tcl_require_objc(2);
    if (Tcl_GetIntFromObj(interp, objv[1], &histo_id) != TCL_OK)
	goto fail;
    switch (hs_type(histo_id))
    {
    case HS_NONE:
        Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
                         " is not a valid Histo-Scope id", NULL);
        goto fail;
    case HS_1D_HISTOGRAM:
        histo_dim = 1;
        axes[0].npoints = hs_1d_hist_num_bins(histo_id);
        hs_1d_hist_range(histo_id, &axes[0].xmin, &axes[0].xmax);
        break;
    case HS_2D_HISTOGRAM:
        {
            int nx, ny;
            hs_2d_hist_num_bins(histo_id, &nx, &ny);
            axes[0].npoints = nx;
            axes[1].npoints = ny;
        }
        hs_2d_hist_range(histo_id, &axes[0].xmin, &axes[0].xmax,
                         &axes[1].xmin, &axes[1].xmax);
        histo_dim = 2;
        break;
    case HS_3D_HISTOGRAM:
        {
            int nx, ny, nz;
            hs_3d_hist_num_bins(histo_id, &nx, &ny, &nz);
            axes[0].npoints = nx;
            axes[1].npoints = ny;
            axes[2].npoints = nz;
        }
        hs_3d_hist_range(histo_id, &axes[0].xmin, &axes[0].xmax,
                         &axes[1].xmin, &axes[1].xmax,
                         &axes[2].xmin, &axes[2].xmax);
        histo_dim = 3;
        break;
    default:
        Tcl_AppendResult(interp, "item with id ", 
                         Tcl_GetStringFromObj(objv[1], NULL),
                         " is not a histogram", NULL);
        goto fail;
    }

    for (i=0; i<histo_dim; ++i)
    {
        float halfbw = (axes[i].xmax - axes[i].xmin)/axes[i].npoints/2.f;
        axes[i].xmin += halfbw;
        axes[i].xmax -= halfbw;
        nbins *= axes[i].npoints;
    }

    data = (float *)malloc(nbins*sizeof(float));
    assert(data);
    switch (histo_dim)
    {
    case 1:
        hs_1d_hist_bin_contents(histo_id, data);
        break;
    case 2:
        hs_2d_hist_bin_contents(histo_id, data);
        break;
    case 3:
        hs_3d_hist_bin_contents(histo_id, data);
        break;
    default:
        assert(0);
    }
    d = create_interpolator_nd(axes, histo_dim, data, 1);
    assert(d);

    sprintf(namebuf, "linear_interp_nd_%d", n_cmd++);
    if (Tcl_CreateObjCommand(interp, namebuf, linear_interpolator_nd_handle,
                      (ClientData)d, linear_interpolator_nd_cleanup) == NULL)
        goto fail;
    Tcl_SetResult(interp, namebuf, TCL_VOLATILE);
    return TCL_OK;

 fail:
    destroy_interpolator_nd(d);
    return TCL_ERROR;
}

int init_linear_interpolator_nd_api(Tcl_Interp *interp)
{
    if (Tcl_CreateObjCommand(interp,
			     "create_linear_interpolator_nd",
			     c_create_linear_interpolator_nd,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    commandInterp = interp;
    return TCL_OK;
}

void cleanup_linear_interpolator_nd_api(void)
{
    if (commandInterp)
    {
	Tcl_DeleteCommand(commandInterp, "create_linear_interpolator_nd");
    }
}

#ifdef __cplusplus
}
#endif
