#include <assert.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "tcl.h"
#include "histoscope.h"
#include "cdf_3d.h"
#include "cdf_3d_api.h"

#define DEFAULT_EPS 1.0e-12

#ifdef __cplusplus
extern "C" {
#endif

int get_axis_from_obj(Tcl_Interp *interp, Tcl_Obj *obj,
		      int ndim, int strict, int *axis);

static int c_cdf_3d(
    ClientData clientData,Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static Tcl_Interp *commandInterp = 0;
static int n_commands = 0;

static float *histogram_buffer = 0;
static unsigned histogram_buffer_len = 0;

static char *OUT_OF_MEMORY = "out of memory";

#define resize_buffer do {\
    if ((unsigned)nbins > histogram_buffer_len)\
    {\
        if (histogram_buffer)\
        {\
            free(histogram_buffer);\
            histogram_buffer_len = 0;\
        }\
        histogram_buffer = (float *)malloc(nbins*sizeof(float));\
        if (histogram_buffer == NULL)\
        {\
            Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);\
            return TCL_ERROR;\
        }\
        histogram_buffer_len = (unsigned)nbins;\
    }\
} while(0);

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

#define verify_3d_histo(id,objnum) do {\
    if (Tcl_GetIntFromObj(interp, objv[objnum], &id) != TCL_OK)\
	return TCL_ERROR;\
    if (hs_type(id) != HS_3D_HISTOGRAM)\
    {\
	if (hs_type(id) == HS_NONE)\
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not a valid Histo-Scope id", NULL);\
	else\
	    Tcl_AppendResult(interp, "item with id ",\
			     Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not a 3d histogram", NULL);\
	return TCL_ERROR;\
    }\
} while(0);

int init_cdf_3d_api(Tcl_Interp *interp)
{
    if (Tcl_CreateObjCommand(interp,
			     "cdf_3d",
			     c_cdf_3d,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    commandInterp = interp;
    return TCL_OK;
}

void cleanup_cdf_3d_api(void)
{
    if (commandInterp)
	Tcl_DeleteCommand(commandInterp, "cdf_3d");
    if (histogram_buffer)
        free(histogram_buffer);
}

static int cdf_3d_handle(ClientData clientData, Tcl_Interp *interp,
                         int objc, Tcl_Obj *CONST objv[])
{
    char *op;
    const Cdf_3d_data *cdf = (Cdf_3d_data *)clientData;

    tcl_objc_range(2, INT_MAX);
    op = Tcl_GetStringFromObj(objv[1], NULL);

    if (strcmp(op, "eval") == 0 || strcmp(op, "value") == 0)
    {
        double x, y, z, v;

        tcl_require_objc(5);
        if (Tcl_GetDoubleFromObj(interp, objv[2], &x) != TCL_OK)
            return TCL_ERROR;
        if (Tcl_GetDoubleFromObj(interp, objv[3], &y) != TCL_OK)
            return TCL_ERROR;
        if (Tcl_GetDoubleFromObj(interp, objv[4], &z) != TCL_OK)
            return TCL_ERROR;
        v = cdf_3d_value(cdf, x, y, z);
        Tcl_SetObjResult(interp, Tcl_NewDoubleObj(v));
    }
    else if (strcmp(op, "coverage") == 0)
    {
        /* Usage: handle coverage xmin ymin zmin xmax ymax zmax */
        double xmin, xmax, ymin, ymax, zmin, zmax, v;

        tcl_require_objc(8);
        if (Tcl_GetDoubleFromObj(interp, objv[2], &xmin) != TCL_OK)
            return TCL_ERROR;
        if (Tcl_GetDoubleFromObj(interp, objv[3], &ymin) != TCL_OK)
            return TCL_ERROR;
        if (Tcl_GetDoubleFromObj(interp, objv[4], &zmin) != TCL_OK)
            return TCL_ERROR;
        if (Tcl_GetDoubleFromObj(interp, objv[5], &xmax) != TCL_OK)
            return TCL_ERROR;
        if (Tcl_GetDoubleFromObj(interp, objv[6], &ymax) != TCL_OK)
            return TCL_ERROR;
        if (Tcl_GetDoubleFromObj(interp, objv[7], &zmax) != TCL_OK)
            return TCL_ERROR;
        if (xmax < xmin || ymax < ymin || zmax < zmin)
        {
            Tcl_SetResult(interp, "problem: max < min", TCL_STATIC);
            return TCL_ERROR;
        }
        v = cdf_3d_rectangle_coverage(
              cdf, xmin, ymin, zmin, xmax, ymax, zmax);
        Tcl_SetObjResult(interp, Tcl_NewDoubleObj(v));
    }
    else if (strcmp(op, "window") == 0)
    {
        /* Usage: handle window coverage x y z eps? */
        double coverage, x, y, z, eps = DEFAULT_EPS;

        tcl_objc_range(6,7);
        if (Tcl_GetDoubleFromObj(interp, objv[2], &coverage) != TCL_OK)
            return TCL_ERROR;
        if (Tcl_GetDoubleFromObj(interp, objv[3], &x) != TCL_OK)
            return TCL_ERROR;
        if (Tcl_GetDoubleFromObj(interp, objv[4], &y) != TCL_OK)
            return TCL_ERROR;
        if (Tcl_GetDoubleFromObj(interp, objv[5], &z) != TCL_OK)
            return TCL_ERROR;
        if (objc > 6)
            if (Tcl_GetDoubleFromObj(interp, objv[6], &eps) != TCL_OK)
                return TCL_ERROR;
        if (eps <= 0.0)
        {
            Tcl_SetResult(interp, "precision argument must be positive",
                          TCL_STATIC);
            return TCL_ERROR;
        }
        if (coverage < 0.0 || coverage > 1.0)
        {
            Tcl_SetResult(interp, "coverage argument out of range",
                          TCL_STATIC);
            return TCL_ERROR;
        }
        else
        {
            double xmin, xmax, ymin, ymax, zmin, zmax;
            Tcl_Obj *result;

            cdf_3d_optimal_window(cdf, x, y, z, coverage, eps,
                                  &xmin, &ymin, &zmin, &xmax, &ymax, &zmax);
            result = Tcl_NewListObj(0, 0);
            Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(xmin));
            Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(ymin));
            Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(zmin));
            Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(xmax));
            Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(ymax));
            Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(zmax));
            Tcl_SetObjResult(interp, result);
            return TCL_OK;
        }
    }
    else if (strcmp(op, "marginal") == 0)
    {
        /* Usage: handle marginal axis value */
        double value;
        int axis;

        tcl_require_objc(4);
        if (get_axis_from_obj(interp, objv[2], 2, 1, &axis) != TCL_OK)
            return TCL_ERROR;
        if (Tcl_GetDoubleFromObj(interp, objv[3], &value) != TCL_OK)
            return TCL_ERROR;
        if (axis == HS_AXIS_X)
            value = cdf_3d_value(cdf, value, cdf->ymax, cdf->zmax);
        else if (axis == HS_AXIS_Y)
            value = cdf_3d_value(cdf, cdf->xmax, value, cdf->zmax);
        else if (axis == HS_AXIS_Z)
            value = cdf_3d_value(cdf, cdf->xmax, cdf->ymax, value);
        else
            assert(0);
        Tcl_SetObjResult(interp, Tcl_NewDoubleObj(value));
        return TCL_OK;
    }
    else if (strcmp(op, "invcdf") == 0)
    {
        /* Usage: handle invcdf axis value eps? */
        double value, eps = DEFAULT_EPS;
        int axis;

        tcl_objc_range(4,5);
        if (get_axis_from_obj(interp, objv[2], 2, 1, &axis) != TCL_OK)
            return TCL_ERROR;
        if (Tcl_GetDoubleFromObj(interp, objv[3], &value) != TCL_OK)
            return TCL_ERROR;
        if (objc > 4)
            if (Tcl_GetDoubleFromObj(interp, objv[4], &eps) != TCL_OK)
                return TCL_ERROR;
        if (eps <= 0.0)
        {
            Tcl_SetResult(interp, "precision argument must be positive",
                          TCL_STATIC);
            return TCL_ERROR;
        }
        if (axis == HS_AXIS_X)
            value = cdf_3d_invcdf_x(cdf, value, eps);
        else if (axis == HS_AXIS_Y)
            value = cdf_3d_invcdf_y(cdf, value, eps);
        else if (axis == HS_AXIS_Z)
            value = cdf_3d_invcdf_z(cdf, value, eps);
        else
            assert(0);
        Tcl_SetObjResult(interp, Tcl_NewDoubleObj(value));
        return TCL_OK;
    }
    else if (strcmp(op, "scan") == 0)
    {
        /* Usage: handle scan id_3d */
        int i, j, k, id_3d, nxbins, nybins, nzbins, nbins;
        float x, y, z, xmin, xmax, ymin, ymax, zmin, zmax, bwx, bwy, bwz;

        tcl_require_objc(3);
        verify_3d_histo(id_3d, 2);
        hs_3d_hist_num_bins(id_3d, &nxbins, &nybins, &nzbins);
        nbins = nxbins*nybins*nzbins;
        resize_buffer;
        hs_3d_hist_range(id_3d, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
        bwx = (xmax - xmin)/nxbins;
        bwy = (ymax - ymin)/nybins;
        bwz = (zmax - zmin)/nzbins;
        for (i=0; i<nxbins; ++i)
        {
            x = xmin + bwx*(i + 0.5);
            for (j=0; j<nybins; ++j)
            {
                y = ymin + bwy*(j + 0.5);
                for (k=0; k<nzbins; ++k)
                {
                    z = zmin + bwz*(k + 0.5);
                    histogram_buffer[(i*nybins+j)*nzbins + k] = 
                        cdf_3d_value(cdf, x, y, z);
                }
            }
        }
        hs_3d_hist_block_fill(id_3d, histogram_buffer, NULL, NULL);
    }
    else if (strcmp(op, "density") == 0)
    {
        /* Usage: handle density id_3d */
        int i, j, k, id_3d, nxbins, nybins, nzbins, nbins;
        float xlo, xhi, ylo, yhi, zlo, zhi;
        float xmin, xmax, ymin, ymax, zmin, zmax, bwx, bwy, bwz, binarea;

        tcl_require_objc(3);
        verify_3d_histo(id_3d, 2);
        hs_3d_hist_num_bins(id_3d, &nxbins, &nybins, &nzbins);
        nbins = nxbins*nybins*nzbins;
        resize_buffer;
        hs_3d_hist_range(id_3d, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
        bwx = (xmax - xmin)/nxbins;
        bwy = (ymax - ymin)/nybins;
        bwz = (zmax - zmin)/nzbins;
        binarea = bwx*bwy*bwz;
        for (i=0; i<nxbins; ++i)
        {
            xlo = xmin + bwx*i;
            xhi = xlo + bwx;
            for (j=0; j<nybins; ++j)
            {
                ylo = ymin + bwy*j;
                yhi = ylo + bwy;
                for (k=0; k<nzbins; ++k)
                {
                    zlo = zmin + bwz*k;
                    zhi = zlo + bwz;
                    histogram_buffer[(i*nybins+j)*nzbins + k] = 
                        cdf_3d_rectangle_coverage(
                            cdf, xlo, ylo, zlo, xhi, yhi, zhi)/binarea;
                }
                
            }
        }
        hs_3d_hist_block_fill(id_3d, histogram_buffer, NULL, NULL);
    }
    else if (strcmp(op, "inspect") == 0 ||
             strcmp(op, "examine") == 0)
    {
        Tcl_Obj *data, *result;
        int i, j, k;

        tcl_require_objc(2);
        result = Tcl_NewListObj(0, 0);
        data = Tcl_NewListObj(0, 0);
        Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(cdf->xmin));
        Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(cdf->ymin));
        Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(cdf->zmin));
        Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(cdf->xmax));
        Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(cdf->ymax));
        Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(cdf->zmax));
        Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(cdf->bwx));
        Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(cdf->bwy));
        Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(cdf->bwz));
        Tcl_ListObjAppendElement(0, result, Tcl_NewIntObj(cdf->nxbins));
        Tcl_ListObjAppendElement(0, result, Tcl_NewIntObj(cdf->nybins));
        Tcl_ListObjAppendElement(0, result, Tcl_NewIntObj(cdf->nzbins));
        for (i=0; i<=cdf->nxbins; ++i)
            for (j=0; j<=cdf->nybins; ++j)
                for (k=0; k<=cdf->nzbins; ++k)
                    Tcl_ListObjAppendElement(0, data, Tcl_NewDoubleObj(
                        cdf->data[(i*(cdf->nybins+1) + j)*(cdf->nzbins+1) + k]));
        Tcl_ListObjAppendElement(0, result, data);
        Tcl_SetObjResult(interp, result);
    }
    else if (strcmp(op, "del") == 0 || strcmp(op, "delete") == 0)
    {
        tcl_require_objc(2);
        Tcl_DeleteCommand(interp, Tcl_GetStringFromObj(objv[0], NULL));
    }
    else
    {
        Tcl_AppendResult(interp, "invalid cdf_3d operation \"", op, "\"", NULL);
        return TCL_ERROR;
    }
    return TCL_OK;
}

static void cdf_3d_cleanup(ClientData clientData)
{
    Cdf_3d_data *cdf = (Cdf_3d_data *)clientData;
    if (cdf)
    {
        if (cdf->data)
            free(cdf->data);
        free(cdf);
    }
}

static int c_cdf_3d(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: cdf_3d id_3d */
    int id_3d, nbins, i, j, k, nxbins, nybins, nzbins, has_non_zero;
    int nxbinsp1, nybinsp1, nzbinsp1, nbinsp1;
    float xmin, xmax, ymin, ymax, zmin, zmax;
    Cdf_3d_data *cdf = NULL;
    char namebuf[32];
    double *data;

    tcl_require_objc(2);
    verify_3d_histo(id_3d, 1);
    cdf = (Cdf_3d_data *)calloc(1, sizeof(Cdf_3d_data));
    if (cdf == NULL)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        goto fail;
    }
    hs_3d_hist_num_bins(id_3d, &nxbins, &nybins, &nzbins);
    nbins = nxbins*nybins*nzbins;
    nxbinsp1 = nxbins + 1;
    nybinsp1 = nybins + 1;
    nzbinsp1 = nzbins + 1;
    nbinsp1 = nxbinsp1*nybinsp1*nzbinsp1;
    resize_buffer;
    hs_3d_hist_bin_contents(id_3d, histogram_buffer);

    has_non_zero = 0;
    for (i=0; i<nbins; ++i)
        if (histogram_buffer[i] < 0.f)
        {
            Tcl_SetResult(interp, "negative histogram bin found", TCL_STATIC);
            goto fail;
        }
        else if (histogram_buffer[i] > 0.f)
            has_non_zero = 1;
    if (!has_non_zero)
    {
        Tcl_SetResult(interp, "input histogram is empty", TCL_STATIC);
        goto fail;
    }
    cdf->data = (double *)calloc(1, nbinsp1*sizeof(double));
    if (cdf->data == NULL)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        goto fail;
    }
    data = cdf->data;
    for (i=1; i<nxbinsp1; ++i)
        for (j=1; j<nybinsp1; ++j)
            for (k=1; k<nzbinsp1; ++k)
            {
                register const int ibase = ((i-1)*nybinsp1+j-1)*nzbinsp1+k-1;
                const double v000 = data[ibase];
                const double v001 = data[ibase + 1];
                const double v010 = data[ibase + nzbinsp1];
                const double v011 = data[ibase + nzbinsp1 + 1];
                const double v100 = data[ibase + nybinsp1*nzbinsp1];
                const double v101 = data[ibase + nybinsp1*nzbinsp1 + 1];
                const double v110 = data[ibase + (nybinsp1 + 1)*nzbinsp1];

                data[(i*nybinsp1+j)*nzbinsp1+k] = 
                    (double)(histogram_buffer[((i-1)*nybins+j-1)*nzbins+k-1]) +
                    (v011 - v001) + (v101 - v100) + (v110 - v010) + v000;
            }
    {
        register double norm = data[nbinsp1 - 1];
        assert(norm > 0.0);
        for (i=0; i<nbinsp1; ++i)
            data[i] /= norm;
    }

    hs_3d_hist_range(id_3d, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
    cdf->xmin = xmin;
    cdf->xmax = xmax;
    cdf->ymin = ymin;
    cdf->ymax = ymax;    
    cdf->zmin = zmin;
    cdf->zmax = zmax;    
    cdf->nxbins = nxbins;
    cdf->nybins = nybins;
    cdf->nzbins = nzbins;
    cdf->bwx = (cdf->xmax - cdf->xmin)/nxbins;
    cdf->bwy = (cdf->ymax - cdf->ymin)/nybins;
    cdf->bwz = (cdf->zmax - cdf->zmin)/nzbins;
    sprintf(namebuf, "cdf_3d_%d", n_commands++);
    if (Tcl_CreateObjCommand(interp, namebuf, cdf_3d_handle,
                             (ClientData)cdf, cdf_3d_cleanup) == NULL)
        goto fail;
    Tcl_SetResult(interp, namebuf, TCL_VOLATILE);
    return TCL_OK;

 fail:
    cdf_3d_cleanup(cdf);
    return TCL_ERROR;
}
    
#ifdef __cplusplus
}
#endif
