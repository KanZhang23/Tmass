#include <assert.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "tcl.h"
#include "histoscope.h"
#include "cdf_2d.h"
#include "cdf_2d_api.h"

#define DEFAULT_EPS 1.0e-14

#ifdef __cplusplus
extern "C" {
#endif

int get_axis_from_obj(Tcl_Interp *interp, Tcl_Obj *obj,
		      int ndim, int strict, int *axis);

static int c_cdf_2d(
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

#define verify_2d_histo(id,objnum) do {\
    if (Tcl_GetIntFromObj(interp, objv[objnum], &id) != TCL_OK)\
	return TCL_ERROR;\
    if (hs_type(id) != HS_2D_HISTOGRAM)\
    {\
	if (hs_type(id) == HS_NONE)\
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not a valid Histo-Scope id", NULL);\
	else\
	    Tcl_AppendResult(interp, "item with id ",\
			     Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not a 2d histogram", NULL);\
	return TCL_ERROR;\
    }\
} while(0);

int init_cdf_2d_api(Tcl_Interp *interp)
{
    if (Tcl_CreateObjCommand(interp,
			     "cdf_2d",
			     c_cdf_2d,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    commandInterp = interp;
    return TCL_OK;
}

void cleanup_cdf_2d_api(void)
{
    if (commandInterp)
	Tcl_DeleteCommand(commandInterp, "cdf_2d");
    if (histogram_buffer)
        free(histogram_buffer);
}

static double xkernel(double x)
{
    if (fabs(x) >= 1.0)
        return 0.0;
    else
        return 1.23046875*pow(1.0-x*x, 4);
}

static double ykernel(double y)
{
    if (fabs(y) >= 1.0)
        return 0.0;
    else
        return 1.23046875*pow(1.0-y*y, 4);
}

static int cdf_2d_handle(ClientData clientData, Tcl_Interp *interp,
                         int objc, Tcl_Obj *CONST objv[])
{
    char *op;
    const Cdf_2d_data *cdf = (Cdf_2d_data *)clientData;

    tcl_objc_range(2, INT_MAX);
    op = Tcl_GetStringFromObj(objv[1], NULL);

    if (strcmp(op, "eval") == 0 || strcmp(op, "value") == 0)
    {
        double x, y, v;

        tcl_require_objc(4);
        if (Tcl_GetDoubleFromObj(interp, objv[2], &x) != TCL_OK)
            return TCL_ERROR;
        if (Tcl_GetDoubleFromObj(interp, objv[3], &y) != TCL_OK)
            return TCL_ERROR;
        v = cdf_2d_value(cdf, x, y);
        Tcl_SetObjResult(interp, Tcl_NewDoubleObj(v));
    }
    else if (strcmp(op, "coverage") == 0)
    {
        /* Usage: handle coverage xmin ymin xmax ymax */
        double xmin, xmax, ymin, ymax, v;

        tcl_require_objc(6);
        if (Tcl_GetDoubleFromObj(interp, objv[2], &xmin) != TCL_OK)
            return TCL_ERROR;
        if (Tcl_GetDoubleFromObj(interp, objv[3], &ymin) != TCL_OK)
            return TCL_ERROR;
        if (Tcl_GetDoubleFromObj(interp, objv[4], &xmax) != TCL_OK)
            return TCL_ERROR;
        if (Tcl_GetDoubleFromObj(interp, objv[5], &ymax) != TCL_OK)
            return TCL_ERROR;
        if (xmax < xmin || ymax < ymin)
        {
            Tcl_SetResult(interp, "problem: max < min", TCL_STATIC);
            return TCL_ERROR;
        }
        v = cdf_2d_rectangle_coverage(cdf,
                               xmin, ymin, xmax, ymax);
        Tcl_SetObjResult(interp, Tcl_NewDoubleObj(v));
    }
    else if (strcmp(op, "window") == 0)
    {
        /* Usage: handle window coverage x y eps? */
        double coverage, x, y, eps = DEFAULT_EPS;

        tcl_objc_range(5,6);
        if (Tcl_GetDoubleFromObj(interp, objv[2], &coverage) != TCL_OK)
            return TCL_ERROR;
        if (Tcl_GetDoubleFromObj(interp, objv[3], &x) != TCL_OK)
            return TCL_ERROR;
        if (Tcl_GetDoubleFromObj(interp, objv[4], &y) != TCL_OK)
            return TCL_ERROR;
        if (objc > 5)
            if (Tcl_GetDoubleFromObj(interp, objv[5], &eps) != TCL_OK)
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
            double xmin, xmax, ymin, ymax;
            Tcl_Obj *result;

            cdf_2d_optimal_window(cdf, x, y, coverage, eps,
                                  &xmin, &ymin, &xmax, &ymax);
            result = Tcl_NewListObj(0, 0);
            Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(xmin));
            Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(ymin));
            Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(xmax));
            Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(ymax));
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
            value = cdf_2d_value(cdf, value, cdf->ymax);
        else if (axis == HS_AXIS_Y)
            value = cdf_2d_value(cdf, cdf->xmax, value);
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
            value = cdf_2d_invcdf_x(cdf, value, eps);
        else if (axis == HS_AXIS_Y)
            value = cdf_2d_invcdf_y(cdf, value, eps);
        else
            assert(0);
        Tcl_SetObjResult(interp, Tcl_NewDoubleObj(value));
        return TCL_OK;
    }
    else if (strcmp(op, "cdf_weights") == 0)
    {
        /* Usage: handle cdf_weights id_2d x_cnt y_cnt cdf_bw_x cdf_bw_y scale?
         *
         * If the scale is not provided, the resulting histo will be
         * normalized in such a way that the sum of all weights is 1.0.
         */
        double x_cnt, y_cnt, cdf_bw_x, cdf_bw_y, scale = 1.0, sum = 0.0;
        double cdf_x_cnt, cdf_y_cnt, kern_x, kern_y = 0.0;
        int i, j, id_2d, nxbins, nybins, nbins;
        float x, y, xmin, xmax, ymin, ymax, bwx, bwy, val;

        tcl_objc_range(7,8);
        verify_2d_histo(id_2d, 2);
        if (Tcl_GetDoubleFromObj(interp, objv[3], &x_cnt) != TCL_OK)
            return TCL_ERROR;
        if (Tcl_GetDoubleFromObj(interp, objv[4], &y_cnt) != TCL_OK)
            return TCL_ERROR;
        if (Tcl_GetDoubleFromObj(interp, objv[5], &cdf_bw_x) != TCL_OK)
            return TCL_ERROR;
        if (Tcl_GetDoubleFromObj(interp, objv[6], &cdf_bw_y) != TCL_OK)
            return TCL_ERROR;
        if (objc > 7)
        {
            if (Tcl_GetDoubleFromObj(interp, objv[7], &scale) != TCL_OK)
                return TCL_ERROR;
            if (scale <= 0.0)
            {
                Tcl_SetResult(interp, "scale must be positive",
                              TCL_VOLATILE);
                return TCL_ERROR;
            }
        }
        if (cdf_bw_x <= 0.0 || cdf_bw_y <= 0.0)
        {
            Tcl_SetResult(interp, "kernel bandwidth must be positive",
                          TCL_VOLATILE);
            return TCL_ERROR;
        }
        cdf_x_cnt = cdf_2d_value(cdf, x_cnt, cdf->ymax);
        cdf_y_cnt = cdf_2d_value(cdf, cdf->xmax, y_cnt);
        hs_2d_hist_num_bins(id_2d, &nxbins, &nybins);
        nbins = nxbins*nybins;
        resize_buffer;
        hs_2d_hist_range(id_2d, &xmin, &xmax, &ymin, &ymax);
        bwx = (xmax - xmin)/nxbins;
        bwy = (ymax - ymin)/nybins;
        for (i=0; i<nxbins; ++i)
        {
            x = xmin + bwx*(i + 0.5);
            if (x >= cdf->xmin && x <= cdf->xmax)
            {
                const double cdf_x = cdf_2d_value(cdf, x, cdf->ymax);
                kern_x = xkernel((cdf_x - cdf_x_cnt)/cdf_bw_x);
            }
            else
                kern_x = 0.0;
            assert(kern_x >= 0.0);
            for (j=0; j<nybins; ++j)
            {
                if (kern_x > 0.0)
                {
                    y = ymin + bwy*(j + 0.5);
                    if (y >= cdf->ymin && y <= cdf->ymax)
                    {
                        const double cdf_y = cdf_2d_value(cdf, cdf->xmax, y);
                        kern_y = ykernel((cdf_y - cdf_y_cnt)/cdf_bw_y);
                    }
                    else
                        kern_y = 0.0;
                    assert(kern_y >= 0.0);
                    val = kern_x*kern_y;
                    histogram_buffer[i*nybins+j] = val;
                    sum += val;
                }
                else
                    histogram_buffer[i*nybins+j] = 0.f;
            }
        }
        assert(sum > 0.0);
        sum /= scale;
        for (i=0; i<nbins; ++i)
            histogram_buffer[i] /= sum;
        hs_2d_hist_block_fill(id_2d, histogram_buffer, NULL, NULL);
    }
    else if (strcmp(op, "scan") == 0)
    {
        /* Usage: handle scan id_2d */
        int i, j, id_2d, nxbins, nybins, nbins;
        float x, y, xmin, xmax, ymin, ymax, bwx, bwy;

        tcl_require_objc(3);
        verify_2d_histo(id_2d, 2);
        hs_2d_hist_num_bins(id_2d, &nxbins, &nybins);
        nbins = nxbins*nybins;
        resize_buffer;
        hs_2d_hist_range(id_2d, &xmin, &xmax, &ymin, &ymax);
        bwx = (xmax - xmin)/nxbins;
        bwy = (ymax - ymin)/nybins;
        for (i=0; i<nxbins; ++i)
        {
            x = xmin + bwx*(i + 0.5);
            for (j=0; j<nybins; ++j)
            {
                y = ymin + bwy*(j + 0.5);
                histogram_buffer[i*nybins+j] = cdf_2d_value(cdf, x, y);
            }
        }
        hs_2d_hist_block_fill(id_2d, histogram_buffer, NULL, NULL);
    }
    else if (strcmp(op, "density") == 0)
    {
        /* Usage: handle density id_2d */
        int i, j, id_2d, nxbins, nybins, nbins;
        float xlo, xhi, ylo, yhi, xmin, xmax, ymin, ymax, bwx, bwy, binarea;

        tcl_require_objc(3);
        verify_2d_histo(id_2d, 2);
        hs_2d_hist_num_bins(id_2d, &nxbins, &nybins);
        nbins = nxbins*nybins;
        resize_buffer;
        hs_2d_hist_range(id_2d, &xmin, &xmax, &ymin, &ymax);
        bwx = (xmax - xmin)/nxbins;
        bwy = (ymax - ymin)/nybins;
        binarea = bwx*bwy;
        for (i=0; i<nxbins; ++i)
        {
            xlo = xmin + bwx*i;
            xhi = xlo + bwx;
            for (j=0; j<nybins; ++j)
            {
                ylo = ymin + bwy*j;
                yhi = ylo + bwy;
                histogram_buffer[i*nybins+j] = cdf_2d_rectangle_coverage(
                    cdf, xlo, ylo, xhi, yhi)/binarea;
            }
        }
        hs_2d_hist_block_fill(id_2d, histogram_buffer, NULL, NULL);
    }
    else if (strcmp(op, "inspect") == 0 ||
             strcmp(op, "examine") == 0)
    {
        Tcl_Obj *data, *result;
        int i, j;

        tcl_require_objc(2);
        result = Tcl_NewListObj(0, 0);
        data = Tcl_NewListObj(0, 0);
        Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(cdf->xmin));
        Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(cdf->ymin));
        Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(cdf->xmax));
        Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(cdf->ymax));
        Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(cdf->bwx));
        Tcl_ListObjAppendElement(0, result, Tcl_NewDoubleObj(cdf->bwy));
        Tcl_ListObjAppendElement(0, result, Tcl_NewIntObj(cdf->nxbins));
        Tcl_ListObjAppendElement(0, result, Tcl_NewIntObj(cdf->nybins));
        for (i=0; i<=cdf->nxbins; ++i)
            for (j=0; j<=cdf->nybins; ++j)
                Tcl_ListObjAppendElement(0, data, Tcl_NewDoubleObj(
                    cdf->data[i*(cdf->nybins+1) + j]));
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
        Tcl_AppendResult(interp, "invalid cdf_2d operation \"", op, "\"", NULL);
        return TCL_ERROR;
    }
    return TCL_OK;
}

static void cdf_2d_cleanup(ClientData clientData)
{
    Cdf_2d_data *cdf = (Cdf_2d_data *)clientData;
    if (cdf)
    {
        if (cdf->data)
            free(cdf->data);
        free(cdf);
    }
}

static int c_cdf_2d(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: cdf_2d id_2d */
    int id_2d, nbins, i, j, nxbins, nybins, has_non_zero;
    int nxbinsp1, nybinsp1, nbinsp1;
    float xmin, xmax, ymin, ymax;
    Cdf_2d_data *cdf = NULL;
    char namebuf[32];
    double *data;

    tcl_require_objc(2);
    verify_2d_histo(id_2d, 1);
    cdf = (Cdf_2d_data *)calloc(1, sizeof(Cdf_2d_data));
    if (cdf == NULL)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        goto fail;
    }
    hs_2d_hist_num_bins(id_2d, &nxbins, &nybins);
    nbins = nxbins*nybins;
    nxbinsp1 = nxbins + 1;
    nybinsp1 = nybins + 1;
    nbinsp1 = nxbinsp1*nybinsp1;
    resize_buffer;
    hs_2d_hist_bin_contents(id_2d, histogram_buffer);

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
    cdf->data = (double *)malloc(nbinsp1*sizeof(double));
    if (cdf->data == NULL)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        goto fail;
    }
    data = cdf->data;
    for (j=0; j<nybinsp1; ++j)
        data[j] = 0.0;
    for (i=1; i<nxbinsp1; ++i)
        data[i*nybinsp1] = 0.0;
    for (i=1; i<nxbinsp1; ++i)
        for (j=1; j<nybinsp1; ++j)
        {
            register const int ibase = (i-1)*nybinsp1+j-1;
            const double lleft = data[ibase];
            data[i*nybinsp1+j] = (data[ibase+1] - lleft) +
                                 (data[ibase+nybinsp1] - lleft) +
                                 histogram_buffer[ibase-i+1] + lleft;
        }
    {
        register double norm = data[nbinsp1 - 1];
        assert(norm > 0.0);
        for (i=0; i<nbinsp1; ++i)
            data[i] /= norm;
    }

    hs_2d_hist_range(id_2d, &xmin, &xmax, &ymin, &ymax);
    cdf->xmin = xmin;
    cdf->xmax = xmax;
    cdf->ymin = ymin;
    cdf->ymax = ymax;    
    cdf->nxbins = nxbins;
    cdf->nybins = nybins;
    cdf->bwx = (cdf->xmax - cdf->xmin)/nxbins;
    cdf->bwy = (cdf->ymax - cdf->ymin)/nybins;
    sprintf(namebuf, "cdf_2d_%d", n_commands++);
    if (Tcl_CreateObjCommand(interp, namebuf, cdf_2d_handle,
                             (ClientData)cdf, cdf_2d_cleanup) == NULL)
        goto fail;
    Tcl_SetResult(interp, namebuf, TCL_VOLATILE);
    return TCL_OK;

 fail:
    cdf_2d_cleanup(cdf);
    return TCL_ERROR;
}
    
#ifdef __cplusplus
}
#endif
