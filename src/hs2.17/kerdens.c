#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "histoscope.h"
#include "kerdens.h"

#define PI 3.14159265358979323846
#define TWOPI (2.0 * PI)
#define SQR2PI 2.5066282746310005

#define RANGE1D_GAUSS 1.3489795003921634864
#define RANGE1D_EPANECHNIKOV 0.6945927106677213954
#define RANGE1D_BIWEIGHT 0.5622553408414116475
#define RANGE1D_TRIWEIGHT 0.48460623743317051755
#define RANGE1D_QUADWEIGHT 0.43214463012730536423
#define RANGE1D_CAUCHY 2.0

#define RANGE2D_GAUSS 1.3862943611198906188
#define RANGE2D_EPANECHNIKOV 0.2928932188134524756
#define RANGE2D_BIWEIGHT 0.206299474015900262624
#define RANGE2D_TRIWEIGHT 0.159103584746285456969
#define RANGE2D_QUADWEIGHT 0.129449436703875860864

static double gauss_kernel_1d(double x);
static double biweight_kernel_1d(double x);
static double triweight_kernel_1d(double x);
static double quadweight_kernel_1d(double x);
static double epan_kernel_1d(double x);
static double rect_kernel_1d(double x);
static double cauchy_kernel_1d(double x);

static double gauss_kernel_2d(double rsquare);
static double biweight_kernel_2d(double rsquare);
static double triweight_kernel_2d(double rsquare);
static double quadweight_kernel_2d(double rsquare);
static double epan_kernel_2d(double rsquare);
static double rect_kernel_2d(double rsquare);

static void mult_2d_matrices(double a[2][2], double b[2][2], double c[2][2]);

static const struct kernel_property_1d {
    char *name;
    Kernel_function_1d *fcn;
} kernel_properties_1d[] = {
    {"gaussian", gauss_kernel_1d},
    {"uniform", rect_kernel_1d},
    {"epanechnikov", epan_kernel_1d},
    {"biweight", biweight_kernel_1d},
    {"triweight", triweight_kernel_1d},
    {"quadweight", quadweight_kernel_1d},
    {"cauchy", cauchy_kernel_1d}
};

static const struct kernel_property_2d {
    char *name;
    Kernel_function_2d *fcn;
} kernel_properties_2d[] = {
    {"gaussian", gauss_kernel_2d},
    {"uniform", rect_kernel_2d},
    {"epanechnikov", epan_kernel_2d},
    {"biweight", biweight_kernel_2d},
    {"triweight", triweight_kernel_2d},
    {"quadweight", quadweight_kernel_2d}
};

/* 1d kernels are normalized so that their interquartile
   range is the same as for the Gaussian kernel.

   Two-dimensional kernels are normalized so that
   50% of the weight is contained within the sphere
   of the same radius.
*/
static double gauss_kernel_1d(double x)
{
    return exp(-x*x/2.0)/SQR2PI;
}

static double biweight_kernel_1d(double x)
{
    double t = x/(RANGE1D_GAUSS/RANGE1D_BIWEIGHT);
    if (t > -1.0 && t < 1.0) {
	double u = (1.0-t*t);
	return 15.0/16.0/(RANGE1D_GAUSS/RANGE1D_BIWEIGHT)*u*u;
    }
    return 0.0;
}

static double triweight_kernel_1d(double x)
{
    double t = x/(RANGE1D_GAUSS/RANGE1D_TRIWEIGHT);
    if (t > -1.0 && t < 1.0) {
	double u = (1.0-t*t);
	return 35.0/32.0/(RANGE1D_GAUSS/RANGE1D_TRIWEIGHT)*u*u*u;
    }
    return 0.0;
}

static double quadweight_kernel_1d(double x)
{
    double t = x/(RANGE1D_GAUSS/RANGE1D_QUADWEIGHT);
    if (t > -1.0 && t < 1.0) {
	double u = (1.0-t*t);
	return 315.0/256.0/(RANGE1D_GAUSS/RANGE1D_QUADWEIGHT)*u*u*u*u;
    }
    return 0.0;
}

static double epan_kernel_1d(double x)
{
    double t = x/(RANGE1D_GAUSS/RANGE1D_EPANECHNIKOV);
    if (t > -1.0 && t < 1.0)
	return 3.0/4.0/(RANGE1D_GAUSS/RANGE1D_EPANECHNIKOV)*(1.0-t*t);
    else
	return 0.0;
}

static double cauchy_kernel_1d(double x)
{
    double t = x/(RANGE1D_GAUSS/RANGE1D_CAUCHY);
    return 1.0/PI/(RANGE1D_GAUSS/RANGE1D_CAUCHY)/(1.0 + t*t);
}

static double rect_kernel_1d(double x)
{
    if (x > RANGE1D_GAUSS || x < -RANGE1D_GAUSS)
	return 0.0;
    else
	return 1.0/2.0/RANGE1D_GAUSS;
}

void append_kernel_names_1d(Tcl_Interp *interp)
{
    int i, n;
    n = sizeof(kernel_properties_1d)/sizeof(kernel_properties_1d[0]);
    for (i=0; i<n; ++i) {
	if (i == n-1)
	    Tcl_AppendResult(interp, "and ", kernel_properties_1d[i].name, NULL);
	else
	    Tcl_AppendResult(interp, kernel_properties_1d[i].name, ", ", NULL);
    }
}

Kernel_function_1d *find_kernel_1d(const char *name)
{
    int i, n;
    n = sizeof(kernel_properties_1d)/sizeof(kernel_properties_1d[0]);
    for (i=0; i<n; ++i)
	if (strcmp(kernel_properties_1d[i].name, name) == 0)
	    return kernel_properties_1d[i].fcn;
    return NULL;
}

static double gauss_kernel_2d(double rsquare)
{
    return exp(-rsquare/2.0)/TWOPI;
}

static double biweight_kernel_2d(double rsquare)
{
    double tsq = rsquare/(RANGE2D_GAUSS/RANGE2D_BIWEIGHT);
    if (tsq >= 0.0 && tsq < 1.0) {
	double u = (1.0-tsq);
	return 3.0/PI/(RANGE2D_GAUSS/RANGE2D_BIWEIGHT)*u*u;
    }
    return 0.0;
}

static double triweight_kernel_2d(double rsquare)
{
    double tsq = rsquare/(RANGE2D_GAUSS/RANGE2D_TRIWEIGHT);
    if (tsq >= 0.0 && tsq < 1.0) {
	double u = (1.0-tsq);
	return 4.0/PI/(RANGE2D_GAUSS/RANGE2D_TRIWEIGHT)*u*u*u;
    }
    return 0.0;
}

static double quadweight_kernel_2d(double rsquare)
{
    double tsq = rsquare/(RANGE2D_GAUSS/RANGE2D_QUADWEIGHT);
    if (tsq >= 0.0 && tsq < 1.0) {
	double u = (1.0-tsq);
	return 5.0/PI/(RANGE2D_GAUSS/RANGE2D_QUADWEIGHT)*u*u*u*u;
    }
    return 0.0;
}

static double epan_kernel_2d(double rsquare)
{
    double tsq = rsquare/(RANGE2D_GAUSS/RANGE2D_EPANECHNIKOV);
    if (tsq >= 0.0 && tsq < 1.0)
	return 2.0/PI/(RANGE2D_GAUSS/RANGE2D_EPANECHNIKOV)*(1.0-tsq);
    else
	return 0.0;
}

static double rect_kernel_2d(double rsquare)
{
    if (rsquare < 0.0 || rsquare > 2.0*RANGE2D_GAUSS)
	return 0.0;
    else
	return 1.0/TWOPI/RANGE2D_GAUSS;
}

void append_kernel_names_2d(Tcl_Interp *interp)
{
    int i, n;
    n = sizeof(kernel_properties_2d)/sizeof(kernel_properties_2d[0]);
    for (i=0; i<n; ++i) {
	if (i == n-1)
	    Tcl_AppendResult(interp, "and ", kernel_properties_2d[i].name, NULL);
	else
	    Tcl_AppendResult(interp, kernel_properties_2d[i].name, ", ", NULL);
    }
}

Kernel_function_2d *find_kernel_2d(const char *name)
{
    int i, n;
    n = sizeof(kernel_properties_2d)/sizeof(kernel_properties_2d[0]);
    for (i=0; i<n; ++i)
	if (strcmp(kernel_properties_2d[i].name, name) == 0)
	    return kernel_properties_2d[i].fcn;
    return NULL;
}

#define check_optional_column(col) do {\
    if (col >= num_var)\
	return 1;\
    else if (col >= 0)\
	++need_columns;\
} while (0);

int estmate_kernel_density_1d(int nt_id, int points_col, int weights_col,
		     int localbw_col, Kernel_function_1d *fcn, double dbw,
		     double dmin, double dmax, int nout, float *result)
{
    static float *old_point_data = NULL;
    static int old_nt_id = 0, old_points_col = -1, old_weights_col = -1;
    static int old_localbw_col = -1, old_num_entries = -1;

    int i, j, num_var, num_entries, need_columns = 0;
    float *point_data = NULL, *localbw_data = NULL, *weight_data = NULL;
    double x, diff, drange, dsum, dbins, lbw;

    /* Check the input */
    num_var = hs_num_variables(nt_id);
    if (num_var <= 0)
	return 1;
    if (points_col < 0 || points_col >= num_var)
	return 1;
    else
	++need_columns;
    check_optional_column(weights_col);
    check_optional_column(localbw_col);
    if (result == NULL || nout <= 0)
	return 1;
    if (fcn == NULL)
	return 1;

    /* Check for bad global bandwidth */
    if (dbw == 0.0) {
	return 3;
    }

    /* Check for an empty ntuple */
    num_entries = hs_num_entries(nt_id);
    if (num_entries == 0) {
	memset(result, 0, nout*sizeof(float));
	return 0;
    }

    /* Allocate and spread the memory */
    if (nt_id != old_nt_id ||
        points_col != old_points_col ||
        weights_col != old_weights_col ||
        localbw_col != old_localbw_col ||
        num_entries != old_num_entries ||
        old_point_data == NULL)
    {
        if (old_point_data)
        {
            free(old_point_data);
            old_point_data = NULL;
        }
        point_data = (float *)malloc(num_entries*need_columns*sizeof(float));
        if (point_data == NULL)
            return 2;

        if (localbw_col >= 0)
            localbw_data = point_data+num_entries;
        if (weights_col >= 0)
            weight_data = point_data+num_entries*(need_columns-1);

        /* Get the data from the ntuple */
        hs_column_contents(nt_id, points_col, point_data);
        if (weight_data)
            hs_column_contents(nt_id, weights_col, weight_data);
        if (localbw_data)
            hs_column_contents(nt_id, localbw_col, localbw_data);

        old_nt_id = nt_id;
        old_points_col = points_col;
        old_weights_col = weights_col;
        old_localbw_col = localbw_col;
        old_num_entries = num_entries;
        old_point_data = point_data;
    }
    else
    {
        point_data = old_point_data;
        if (localbw_col >= 0)
            localbw_data = point_data+num_entries;
        if (weights_col >= 0)
            weight_data = point_data+num_entries*(need_columns-1);        
    }

    /* Go over the output points (in a naive way, no FFT in this function) */
    dbins = (double)(nout-1);
    drange = dmax - dmin;
    for (i=0; i<nout; ++i)
    {
	if (i)
	    x = dmin + ((double)i / dbins) * drange;
	else
	    x = dmin;
	dsum = 0.0;
	if (localbw_data) {
	    for (j=0; j<num_entries; ++j) {
		lbw = dbw * localbw_data[j];
		if (lbw == 0.0)
		    continue;
		diff = (x - point_data[j])/lbw;
		if (lbw < 0.0)
		    lbw = -lbw;
		if (weight_data)
		    dsum += weight_data[j]*fcn(diff)/lbw;
		else
		    dsum += fcn(diff)/lbw;
	    }
	} else {
	    for (j=0; j<num_entries; ++j) {
		diff = (x - point_data[j])/dbw;
		if (weight_data)
		    dsum += weight_data[j]*fcn(diff);
		else
		    dsum += fcn(diff);
	    }
	    dsum /= fabs(dbw);
	}
	result[i] = (float)dsum;
    }

    return 0;
}

#define check_local_column(col) do {\
    if (col >= num_var)\
	return 1;\
    else if (col >= 0)\
	++need_local;\
} while (0);

int estmate_kernel_density_2d(
    int nt_id, int x_col, int y_col, int weights_col,
    int sxsq_col, int sysq_col, int sxsy_col,
    Kernel_function_2d *fcn, double mat[2][2],
    double xmin, double xmax, int nx,
    double ymin, double ymax, int ny, float *result)
{
    typedef struct _point {
	float x;
	float y;
	float w;
    } Point;
    typedef struct _covmat {
	float m00;
	float m01;
	float m11;
	float norm;
    } Covmat;

    int i, j, ix, iy, nout, num_var, num_entries, need_local = 0;
    double det, dsum, yrange, xrange, dnx, dny;
    double dx, dy, xdiff, ydiff, xprime, yprime, rsquared, norm;
    Point *p, *points = NULL;
    Covmat *e, *errors = NULL;
    float *data = NULL, *x, *y, *w, *pcol;
    int memstatus = 2;

    /* Check the input */
    num_var = hs_num_variables(nt_id);
    if (num_var <= 0)
	return 1;
    if (x_col < 0 || x_col >= num_var || y_col < 0 || y_col >= num_var)
	return 1;
    if (weights_col  >= num_var)
	return 1;
    check_local_column(sxsq_col);
    check_local_column(sysq_col);
    check_local_column(sxsy_col);
    if (result == NULL || nx <= 0 || ny <= 0)
	return 1;
    nout = nx*ny;
    if (fcn == NULL)
	return 1;

    /* If we have local bandwidth, both sxsq_col and sysq_col
       must be specified */
    if (need_local && (sxsq_col < 0 || sysq_col < 0))
	return 1;

    /* Check for degenerate transformation */
    det = mat[0][0]*mat[1][1] - mat[0][1]*mat[1][0];
    if (det == 0.0)
	return 3;
    norm = 1.0/fabs(det);

    /* Check for an empty ntuple */
    num_entries = hs_num_entries(nt_id);
    if (num_entries == 0) {
	memset(result, 0, nout*sizeof(float));
	return 0;
    }

    /* Allocate the memory */
    points = malloc(num_entries*sizeof(Point));
    if (points == NULL)
	goto memfail;
    if (need_local) {
	errors = malloc(num_entries*sizeof(Covmat));
	if (errors == NULL)
	    goto memfail;
    }
    data = malloc(3*num_entries*sizeof(float));
    if (data == NULL)
	goto memfail;
    x = data;
    y = x + num_entries;
    w = y + num_entries;

    /* Get the ntuple contents */
    hs_column_contents(nt_id, x_col, x);
    hs_column_contents(nt_id, y_col, y);
    if (weights_col >= 0)
	hs_column_contents(nt_id, weights_col, w);
    for (i=0; i<num_entries; ++i)
    {
	points[i].x = x[i];
	points[i].y = y[i];
	if (weights_col >= 0)
	    points[i].w = w[i];
	else
	    points[i].w = 1.f;
    }

    /* Get the errors */
    if (need_local)
    {
	hs_column_contents(nt_id, sxsq_col, x);
	hs_column_contents(nt_id, sysq_col, y);
	if (sxsy_col >= 0)
	    hs_column_contents(nt_id, sxsy_col, w);
	for (i=0; i<num_entries; ++i)
	{
	    if (x[i] > 0.f && y[i] > 0.f)
	    {
		if (sxsy_col >= 0)
		{
		    float fdet = x[i]*y[i] - w[i]*w[i];
		    if (fdet > 0.f)
		    {
			errors[i].m00 = y[i]/fdet;
			errors[i].m11 = x[i]/fdet;
			errors[i].m01 = -w[i]/fdet;
			errors[i].norm = sqrt(fdet);
		    }
		    else
			errors[i].norm = 0.f;
		}
		else
		{
		    errors[i].m00 = 1.f/x[i];
		    errors[i].m11 = 1.f/y[i];
		    errors[i].m01 = 0.f;
		    errors[i].norm = sqrt(x[i]*y[i]);
		}
	    }
	    else
		errors[i].norm = 0.f;

	    /* Calculate the error matrix in the transformed basis */
	    if (errors[i].norm != 0.f)
	    {
		double tmp, emat[2][2], eprime[2][2], mtranspose[2][2];
		emat[0][0] = errors[i].m00;
		emat[1][0] = emat[0][1] = errors[i].m01;
		emat[1][1] = errors[i].m11;
		memcpy(mtranspose, mat, sizeof(mtranspose));
		tmp = mtranspose[0][1];
		mtranspose[0][1] = mtranspose[1][0];
		mtranspose[1][0] = tmp;
		mult_2d_matrices(mtranspose, emat, eprime);
		mult_2d_matrices(eprime, mat, emat);
		errors[i].m00 = (float)(emat[0][0]);
		errors[i].m01 = (float)(emat[1][0]);
		errors[i].m11 = (float)(emat[1][1]);
		errors[i].norm *= (float)(norm);
	    }
	}
    }

    /* Go over all points and get the result */
    xrange = xmax - xmin;
    yrange = ymax - ymin;
    dnx = (double)(nx - 1);
    dny = (double)(ny - 1);
    for (ix = 0; ix < nx; ++ix)
    {
	if (ix)
	    dx = xmin + ((double)ix / dnx) * xrange;
	else
	    dx = xmin;
	pcol = result + ix*ny;
	for (iy = 0; iy < ny; ++iy)
	{
	    if (iy)
		dy = ymin + ((double)iy / dny) * yrange;
	    else
		dy = ymin;
	    dsum = 0.0;
	    if (need_local)
	    {
		for (j=0; j<num_entries; ++j)
		{
		    e = errors + j;
		    if (e->norm == 0.f)
			continue;
		    p      = points + j;
		    xdiff  = dx - p->x;
		    ydiff  = dy - p->y;
		    xprime = xdiff*e->m00 + ydiff*e->m01;
		    yprime = xdiff*e->m01 + ydiff*e->m11;
		    rsquared = xdiff*xprime + ydiff*yprime;
		    dsum += p->w * fcn(rsquared)/e->norm;
		}
	    }
	    else
	    {
		for (j=0; j<num_entries; ++j)
		{
		    p      = points + j;
		    xdiff  = dx - p->x;
		    ydiff  = dy - p->y;
		    xprime = xdiff*mat[0][0] + ydiff*mat[0][1];
		    yprime = xdiff*mat[1][0] + ydiff*mat[1][1];
		    rsquared = xprime*xprime + yprime*yprime;
		    dsum += p->w * fcn(rsquared);
		}
		dsum /= norm;
	    }
	    pcol[iy] = (float)dsum;
	}
    }

    memstatus = 0;
 memfail:
    if (points)
	free(points);
    if (errors)
	free(errors);
    if (data)
	free(data);
    return memstatus;
}

static void mult_2d_matrices(double a[2][2], double b[2][2], double c[2][2])
{
    c[0][0] = a[0][0]*b[0][0] + a[0][1]*b[1][0];
    c[0][1] = a[0][0]*b[0][1] + a[0][1]*b[1][1];
    c[1][0] = a[1][0]*b[0][0] + a[1][1]*b[1][0];
    c[1][1] = a[1][0]*b[0][1] + a[1][1]*b[1][1];
}

