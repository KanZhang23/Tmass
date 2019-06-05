#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tcl.h"
#include "fit_function.h"
#include "histoscope_stub.h"

Minuit_aux_function data_curve_2d_init;
Minuit_aux_function data_curve_2d_cleanup;
Minuit_c_fit_function data_curve_2d;

typedef struct
{
    int nxbins;
    int nybins;
    float xmin;
    float xmax;
    float ymin;
    float ymax;
    float *data;
} Curve_data;

static Tcl_HashTable curve_table;
static int n_curves = 0;
static int last_id = -1;

static Curve_data *find_curve_data_2d(int id);

int data_curve_2d_init(const int *mode)
{
    if (mode == NULL)
    {
	fprintf(stderr, "Error in data_curve_2d_init: NULL argument pointer\n");
	return 1;
    }
    if (*mode < 0)
    {
	fprintf(stderr, "Error in data_curve_2d_init: Histo-Scope ");
	fprintf(stderr, "item with id %d doesn't exist\n", *mode);
	return 1;
    }
    data_curve_2d_cleanup(mode);
    if (find_curve_data_2d(*mode))
	return 0;
    else
	return 1;
}

int data_curve_2d_cleanup(const int *mode)
{
    Tcl_HashEntry *entryPtr;
    Curve_data *curve;

    if (mode == NULL)
    {
	fprintf(stderr, "Error in data_curve_2d_cleanup: NULL argument pointer\n");
	return 1;
    }
    if (n_curves > 0)
    {
	entryPtr = Tcl_FindHashEntry(&curve_table, (char *)((long)(*mode)));
	if (entryPtr)
	{
	    curve = (Curve_data *)Tcl_GetHashValue(entryPtr);
	    Tcl_DeleteHashEntry(entryPtr);
	    if (--n_curves == 0)
		Tcl_DeleteHashTable(&curve_table);
	    if (curve)
	    {
		if (curve->data)
		    free(curve->data);
		free(curve);
	    }
	    if (last_id == *mode)
		last_id = -1;
	}
    }
    return 0;
}

Minuit_futil(data_curve_2d)
{
    /* Parameters: x_scale, y_scale, x_shift, y_shift, angle, v_scale, v_shift */
    static Curve_data *curve_data = NULL;
    Curve_data *current_data;
    double x_shift, y_shift, x_scale, y_scale, angle, v_scale, v_shift;
    double cosa, sina, rx, ry, x_center, y_center, dx, dy, w, h, z00, z01, z10, z11;
    int ixbin, iybin;

    /* Set up the data pointer */
    if (mode != last_id || last_id < 0 || curve_data == NULL)
    {
	current_data = find_curve_data_2d(mode);
	if (current_data == 0)
	{
	    /* The error message should be printed by "find_curve_data_2d" */
	    *ierr = 1;
	    return 0.0;
	}
	last_id = mode;
	curve_data = current_data;
    }

    x_scale = pars[0];
    y_scale = pars[1];
    x_shift = pars[2];
    y_shift = pars[3];
    angle   = pars[4];
    v_scale = pars[5];
    v_shift = pars[6];
    if (x_scale == 0.0 || y_scale == 0.0)
	return 0.0;

    /* Find the transformed point coordinates */
    x_center = (curve_data->xmin + curve_data->xmax)/2.0*x_scale + x_shift;
    y_center = (curve_data->ymin + curve_data->ymax)/2.0*y_scale + y_shift;

    /* Rotate the argument point by -angle */
    rx = x - x_center;
    ry = y - y_center;
    cosa = cos(angle);
    sina = sin(angle);
    x = rx * cosa  + ry * sina + x_center;
    y = -rx * sina + ry * cosa + y_center;

    /* Shift and scale */
    x = (x - x_shift)/x_scale;
    y = (y - y_shift)/y_scale;

    /* Interpolate the z value */
    if (x <= curve_data->xmin || x >= curve_data->xmax ||
	y <= curve_data->ymin || y >= curve_data->ymax)
	return 0.0;
    w = (curve_data->xmax - curve_data->xmin)/curve_data->nxbins;
    h = (curve_data->ymax - curve_data->ymin)/curve_data->nybins;
    ixbin = (int)((x - curve_data->xmin)/w);
    if (ixbin >= curve_data->nxbins)
	ixbin = curve_data->nxbins-1;
    iybin = (int)((y - curve_data->ymin)/h);
    if (iybin >= curve_data->nybins)
	iybin = curve_data->nybins-1;

    /* Point coordinates relative to the bin center */
    dx = (x - curve_data->xmin)/w - ixbin - 0.5;
    dy = (y - curve_data->ymin)/h - iybin - 0.5;

    /* Process all possible boundary conditions */
    if (ixbin == 0 && iybin == 0 && dx <= 0.0 && dy <= 0.0)
    {
	z00 = 0.0;
	z10 = 0.0;
	z01 = 0.0;
	z11 = curve_data->data[0];
	dx  = 1.0 + dx*2.0;
	dy  = 1.0 + dy*2.0;
    }
    else if (ixbin == 0 && iybin == curve_data->nybins-1 && dx <= 0.0 && dy >= 0.0)
    {
	z00 = 0.0;
	z10 = curve_data->data[iybin];
	z01 = 0.0;
	z11 = 0.0;
	dx  = 1.0 + dx*2.0;
	dy *= 2.0;
    }
    else if (ixbin == curve_data->nxbins-1 && iybin == 0 && dx >= 0.0 && dy <= 0.0)
    {
	z00 = 0.0;
	z10 = 0.0;
	z01 = curve_data->data[ixbin*curve_data->nybins];
	z11 = 0.0;
	dx *= 2.0;
	dy  = 1.0 + dy*2.0;
    }
    else if (ixbin == curve_data->nxbins-1 && iybin == curve_data->nybins-1 && 
	     dx >= 0.0 && dy >= 0.0)
    {
	z00 = curve_data->data[curve_data->nxbins*curve_data->nybins-1];
	z10 = 0.0;
	z01 = 0.0;
	z11 = 0.0;
	dx *= 2.0;
	dy *= 2.0;
    }
    else if (ixbin == 0 && dx <= 0.0)
    {
	dx  = 1.0 + dx*2.0;
	if (dy < 0.0)
	{
	    dy += 1.0;
	    --iybin;
	}
	z00 = 0.0;
	z10 = curve_data->data[iybin];
	z01 = 0.0;
	z11 = curve_data->data[iybin+1];
    }
    else if (ixbin == curve_data->nxbins-1 && dx >= 0.0)
    {
	dx *= 2.0;
	if (dy < 0.0)
	{
	    dy += 1.0;
	    --iybin;
	}
	z00 = curve_data->data[ixbin*curve_data->nybins+iybin];
	z10 = 0.0;
	z01 = curve_data->data[ixbin*curve_data->nybins+iybin+1];
	z11 = 0.0;
    }
    else if (iybin == 0 && dy <= 0.0)
    {
	dy = 1.0 + dy*2.0;
	if (dx < 0.0)
	{
	    dx += 1.0;
	    --ixbin;
	}
	z00 = 0.0;
	z10 = 0.0;
	z01 = curve_data->data[ixbin*curve_data->nybins];
	z11 = curve_data->data[(ixbin+1)*curve_data->nybins];
    }
    else if (iybin == curve_data->nybins-1 && dy >= 0.0)
    {
	dy *= 2.0;
	if (dx < 0.0)
	{
	    dx += 1.0;
	    --ixbin;
	}
	z00 = curve_data->data[ixbin*curve_data->nybins+iybin];
	z10 = curve_data->data[(ixbin+1)*curve_data->nybins+iybin];
	z01 = 0.0;
	z11 = 0.0;
    }
    else
    {
	/* We are not on the boundary */
	if (dx < 0.0)
	{
	    dx += 1.0;
	    --ixbin;
	}
	if (dy < 0.0)
	{
	    dy += 1.0;
	    --iybin;
	}
	z00 = curve_data->data[ixbin*curve_data->nybins+iybin];
	z10 = curve_data->data[(ixbin+1)*curve_data->nybins+iybin];
	z01 = curve_data->data[ixbin*curve_data->nybins+iybin+1];
	z11 = curve_data->data[(ixbin+1)*curve_data->nybins+iybin+1];
    }
    return ((dx - 1.0)*(dy - 1.0)*z00 + dx*z10 + dy*z01 + 
	    dx*dy*(z11-z01-z10))*v_scale + v_shift;
}

static Curve_data *find_curve_data_2d(int id)
{
    Tcl_HashEntry *entryPtr;
    Curve_data *newdata = 0;
    int i, itype;

    if (id < 0)
    {
	fprintf(stderr, "Error in find_curve_data_2d: Histo-Scope ");
	fprintf(stderr, "item with id %d doesn't exist\n", id);
	return NULL;
    }
    if (n_curves > 0)
    {
	entryPtr = Tcl_FindHashEntry(&curve_table, (char *)((long)id));
	if (entryPtr)
	    return (Curve_data *)Tcl_GetHashValue(entryPtr);
    }
    itype = hs_type(id);
    switch (itype)
    {
    case HS_2D_HISTOGRAM:
	newdata = (Curve_data *)calloc(1, sizeof(Curve_data));
	if (newdata == NULL)
	{
	    fprintf(stderr, "Error in find_curve_data_2d: out of memory\n");
	    return NULL;
	}
	hs_2d_hist_num_bins(id, &newdata->nxbins, &newdata->nybins);
	newdata->data = (float *)malloc(newdata->nxbins*newdata->nybins*sizeof(float));
	if (newdata->data == NULL)
	{
	    fprintf(stderr, "Error in find_curve_data_2d: out of memory\n");
	    goto fail0;
	}
	hs_2d_hist_bin_contents(id, newdata->data);
	hs_2d_hist_range(id, &newdata->xmin, &newdata->xmax,
			      &newdata->ymin, &newdata->ymax);
	break;

    case HS_NONE:
	fprintf(stderr, "Error in find_curve_data_2d: Histo-Scope ");
	fprintf(stderr, "item with id %d doesn't exist\n", id);
	return NULL;

    default:
	fprintf(stderr, "Error in find_curve_data_2d: Histo-Scope item ");
	fprintf(stderr, "with id %d is not a 2d histogram\n", id);
	return NULL;
    }

    if (n_curves == 0)
	Tcl_InitHashTable(&curve_table, TCL_ONE_WORD_KEYS);
    entryPtr = Tcl_CreateHashEntry(&curve_table, (char *)((long)id), &i);
    assert(i);
    Tcl_SetHashValue(entryPtr, newdata);
    ++n_curves;

    return newdata;

 fail0:
    if (newdata)
    {
	if (newdata->data)
	    free(newdata->data);
	free(newdata);
    }
    return NULL;
}
