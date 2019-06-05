#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "tcl.h"
#include "fit_function.h"
#include "histoscope_stub.h"

Minuit_aux_function data_curve_1d_init;
Minuit_aux_function data_curve_1d_cleanup;
Minuit_c_fit_function data_curve_1d;

typedef struct
{
    double x;
    double y;
} Point;

typedef struct
{
    int fromHisto;
    double binwidth;
    int npoints;
    Point * datapoints;
} Curve_data;

static Tcl_HashTable curve_table;
static int n_curves = 0;
static int last_id = -1;

static Curve_data *find_curve_data_1d(int id);
static int pointcompare(const void *i, const void *j);

int data_curve_1d_init(const int *mode)
{
    if (mode == NULL)
    {
	fprintf(stderr, "Error in data_curve_1d_init: NULL argument pointer\n");
	return 1;
    }
    if (*mode < 0)
    {
	fprintf(stderr, "Error in data_curve_1d_init: Histo-Scope ");
	fprintf(stderr, "item with id %d doesn't exist\n", *mode);
	return 1;
    }
    data_curve_1d_cleanup(mode);
    if (find_curve_data_1d(*mode))
	return 0;
    else
	return 1;
}

int data_curve_1d_cleanup(const int *mode)
{
    Tcl_HashEntry *entryPtr;
    Curve_data *curve;

    if (mode == NULL)
    {
	fprintf(stderr, "Error in data_curve_1d_cleanup: NULL argument pointer\n");
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
		if (curve->datapoints)
		    free(curve->datapoints);
		free(curve);
	    }
	    if (last_id == *mode)
		last_id = -1;
	}
    }
    return 0;
}

Minuit_futil(data_curve_1d)
{
    /* Need 4 parameters: h_scale, h_shift, v_scale, v_shift */
    static Curve_data *curve_data = NULL;
    static double dxmin = 0.0, dxmax = 0.0;
    static int ilow_old = -1;

    Curve_data *current_data;
    Point *datapoints;
    int n, itry, ihi, nsteps;
    double xstep, fval, h_scale;

    /* Set up the data pointer */
    if (mode != last_id || last_id < 0 || curve_data == NULL)
    {
	current_data = find_curve_data_1d(mode);
	if (current_data == 0)
	{
	    /* The error message should be printed by "find_curve_data_1d" */
	    *ierr = 1;
	    return 0.0;
	}
	last_id = mode;
	curve_data = current_data;
	dxmin = curve_data->datapoints[0].x;
	dxmax = curve_data->datapoints[curve_data->npoints - 1].x;
    }

    /* Find the scaled argument value */
    h_scale = pars[0];
    if (h_scale == 0.0)
	return 0.0;
    /* h_shift = pars[1]; */
    x = (x - pars[1])/h_scale;
    /* v_scale = pars[2]; */
    /* v_shift = pars[3]; */

    /* Check boundary values */
    if (x < dxmin)
    {
	ilow_old = 0;
	return 0.0;
    }
    if (x > dxmax)
	return 0.0;
    if (x == dxmin)
    {
	ilow_old = 0;
	return pars[2]*curve_data->datapoints[0].y + pars[3];
    }
    if (x == dxmax)
	return pars[2]*curve_data->datapoints[curve_data->npoints - 1].y + pars[3];

    /* Check previous index values */
    n = curve_data->npoints;
    datapoints = curve_data->datapoints;
    ihi = ilow_old + 1;
    if (ilow_old >= 0 && ihi < n)
    {
	if (x >= datapoints[ilow_old].x && x <= datapoints[ihi].x)
	{
	    xstep = datapoints[ihi].x - datapoints[ilow_old].x;
	    if (xstep == 0.0)
		fval = (datapoints[ilow_old].y + datapoints[ihi].y)/2.0;
	    else
		fval = datapoints[ilow_old].y + 
		    ((x - datapoints[ilow_old].x)/xstep) *
		    (datapoints[ihi].y - datapoints[ilow_old].y);
	    return pars[2]*fval + pars[3];
	}
	/* Check the next index in increasing order */
	++ilow_old;
	ihi = ilow_old + 1;
	if (ihi < n)
	{
	    if (x >= datapoints[ilow_old].x && x <= datapoints[ihi].x)
	    {
		xstep = datapoints[ihi].x - datapoints[ilow_old].x;
		if (xstep == 0.0)
		    fval = (datapoints[ilow_old].y + datapoints[ihi].y)/2.0;
		else
		    fval = datapoints[ilow_old].y + 
			((x - datapoints[ilow_old].x)/xstep) *
			(datapoints[ihi].y - datapoints[ilow_old].y);
		return pars[2]*fval + pars[3];
	    }
	}
    }

    /* The value of the function is calculated using
     * linear interpolation between the closest points
     */
    if (curve_data->fromHisto)
    {
	if (x < datapoints[1].x)
	{
	    ilow_old = 0;
	    fval = 2.0*(x - dxmin)/curve_data->binwidth*datapoints[1].y;
	}
	else if (x > datapoints[n-2].x)
	{
	    ilow_old = n-2;
	    fval = 2.0*(dxmax - x)/curve_data->binwidth*datapoints[n-2].y;
	}
	else if (x == datapoints[n-2].x)
	{
	    ilow_old = n-2;
	    fval = datapoints[n-2].y;
	}
	else
	{
	    /* Find the closest points */
	    nsteps = (int)((x - datapoints[1].x)/curve_data->binwidth);
	    ilow_old = 1 + nsteps;
	    if (ilow_old > n-3)
		ilow_old = n-3;
	    ihi = ilow_old + 1;
	    fval = datapoints[ilow_old].y + 
		((x - datapoints[ilow_old].x)/curve_data->binwidth) *
		(datapoints[ihi].y - datapoints[ilow_old].y);
	}
    }
    else
    {
	/* The data were taken from an ntuple. The x points may be irregular */
	ilow_old = 0;
	ihi = n - 1;
	while (ihi - ilow_old > 1)
	{
	    itry = (ihi - ilow_old)/2 + ilow_old;
	    if (x >= datapoints[itry].x)
		ilow_old = itry;
	    else
		ihi = itry;
	}
	xstep = datapoints[ihi].x - datapoints[ilow_old].x;
	if (xstep == 0.0)
	    fval = (datapoints[ilow_old].y + datapoints[ihi].y)/2.0;
	else
	    fval = datapoints[ilow_old].y + 
		((x - datapoints[ilow_old].x)/xstep) *
		(datapoints[ihi].y - datapoints[ilow_old].y);
    }

    return pars[2]*fval + pars[3];
}

static Curve_data *find_curve_data_1d(int id)
{
    Tcl_HashEntry *entryPtr;
    int i, nbins, itype, ntuple_dim, npoints;
    Curve_data *newdata = 0;
    float *data = 0;
    float xmin, xmax;
    double dmin, drange, dbins;

    if (id < 0)
    {
	fprintf(stderr, "Error in find_curve_data_1d: Histo-Scope ");
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
    case HS_1D_HISTOGRAM:
	nbins = hs_1d_hist_num_bins(id);
	npoints = nbins + 2;
	newdata = (Curve_data *)calloc(1, sizeof(Curve_data));
	if (newdata == NULL)
	{
	    fprintf(stderr, "Error in find_curve_data_1d: out of memory\n");
	    return NULL;
	}
	newdata->fromHisto = 1;
	newdata->npoints = npoints;
	newdata->datapoints = (Point *)malloc(npoints*sizeof(Point));
	if (newdata->datapoints == NULL)
	{
	    fprintf(stderr, "Error in find_curve_data_1d: out of memory\n");
	    goto fail0;
	}
	data = (float *)malloc(npoints*sizeof(float));
	if (data == NULL)
	{
	    fprintf(stderr, "Error in find_curve_data_1d: out of memory\n");
	    goto fail0;
	}
	hs_1d_hist_bin_contents(id, data+1);
	hs_1d_hist_range(id, &xmin, &xmax);
	newdata->datapoints[0].x = xmin;
	newdata->datapoints[0].y = 0.0;
	newdata->datapoints[npoints-1].x = xmax;
	newdata->datapoints[npoints-1].y = 0.0;
	dbins = (double)(nbins);
	drange = (double)xmax - (double)xmin;
	newdata->binwidth = drange/dbins;
	dmin = (double)xmin - 0.5*newdata->binwidth;
	for (i=1; i<=nbins; ++i)
	{
	    newdata->datapoints[i].x = dmin + ((double)i/dbins)*drange;
	    newdata->datapoints[i].y = data[i];
	}
	break;

    case HS_NTUPLE:
	/* Check that the ntuple has 2 variables */
	ntuple_dim = hs_num_variables(id);
	if (ntuple_dim != 2)
	{
	    fprintf(stderr, "Error in find_curve_data_1d: ntuple with id %d ", id);
	    fprintf(stderr, "has %d variable(s), expect 2\n", ntuple_dim);
	    return NULL;
	}
	npoints = hs_num_entries(id);
	if (npoints == 0)
	{
	    fprintf(stderr, "Error in find_curve_data_1d: ");
	    fprintf(stderr, "ntuple with id %d is empty\n", id);
	    return NULL;
	}
	newdata = (Curve_data *)calloc(1, sizeof(Curve_data));
	if (newdata == NULL)
	{
	    fprintf(stderr, "Error in find_curve_data_1d: out of memory\n");
	    return NULL;
	}
	newdata->fromHisto = 0;
	newdata->binwidth = 0.0;
	newdata->npoints = npoints;
	newdata->datapoints = (Point *)malloc(npoints*sizeof(Point));
	if (newdata->datapoints == NULL)
	{
	    fprintf(stderr, "Error in find_curve_data_1d: out of memory\n");
	    goto fail0;
	}
	data = (float *)malloc(npoints*ntuple_dim*sizeof(float));
	if (data == NULL)
	{
	    fprintf(stderr, "Error in find_curve_data_1d: out of memory\n");
	    goto fail0;
	}
	hs_ntuple_contents(id, data);
	for (i=0; i<npoints; ++i)
	{
	    newdata->datapoints[i].x = data[2*i];
	    newdata->datapoints[i].y = data[2*i+1];
	}
	/* Sort the points */
	qsort(newdata->datapoints, npoints, sizeof(Point), pointcompare);
	break;

    case HS_NONE:
	fprintf(stderr, "Error in find_curve_data_1d: Histo-Scope ");
	fprintf(stderr, "item with id %d doesn't exist\n", id);
	return NULL;

    default:
	fprintf(stderr, "Error in find_curve_data_1d: Histo-Scope item ");
	fprintf(stderr, "with id %d is not a 1d histogram or an ntuple\n", id);
	return NULL;
    }
    if (data) free(data);

    if (n_curves == 0)
	Tcl_InitHashTable(&curve_table, TCL_ONE_WORD_KEYS);
    entryPtr = Tcl_CreateHashEntry(&curve_table, (char *)((long)id), &i);
    assert(i);
    Tcl_SetHashValue(entryPtr, newdata);
    ++n_curves;

    return newdata;

 fail0:
    if (data)
	free(data);
    if (newdata)
    {
	if (newdata->datapoints)
	    free(newdata->datapoints);
	free(newdata);
    }
    return NULL;
}

static int pointcompare(const void *i, const void *j)
{
    Point *pi, *pj;
    pi = (Point *)i;
    pj = (Point *)j;
    if (pi->x < pj->x)
	return -1;
    else if (pi->x > pj->x)
	return 1;
    else
	return 0;
}
