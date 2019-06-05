#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <float.h>

#include "histoscope.h"
#include "histo_utils.h"
#include "histo_tcl_api.h"

enum {
    CUT_OP_LT = 0,
    CUT_OP_GT,
    CUT_OP_LE,
    CUT_OP_GE,
    CUT_OP_EQ,
    CUT_OP_NE,
    N_CUT_OPERATIONS
};

static const char * const simple_cut_ops[N_CUT_OPERATIONS] = {
    "<",   /* CUT_OP_LT */
    ">",   /* CUT_OP_GT */
    "<=",  /* CUT_OP_LE */
    ">=",  /* CUT_OP_GE */
    "==",  /* CUT_OP_EQ */
    "!="   /* CUT_OP_NE */
};

typedef struct _column_or_real {
    double value;
    unsigned index;
    int is_column;
} column_or_real;

typedef struct _simple_filter {
    column_or_real left;
    unsigned op_index;
    column_or_real right;
} simple_filter;

#define swap(x, y, tmp) do {tmp = x; x = y; y = tmp;} while(0);

static int concat_1d_histograms(int uid, char *title, char *category, 
				int id1, int id2);
static int concat_2d_histograms(int uid, char *title, char *category,
				int id1, int id2);
static int concat_3d_histograms(int uid, char *title, char *category,
				int id1, int id2);
static int projection_fill(float *arr, int n, int proj_type, int suppress_zero,
			   float *locat, float *width);
static int projection_fill_weighted(struct weighted_point *arr, int n,
				    int proj_type, float *locat, float *width);
static int duplicate_axis_binning(int uid, char *title, char *category,
				  int id, int axis, int use_coords);
static int get_projection_type(const char *string_proj_type);
static unsigned get_op_index(const char *opname);
static int get_column_or_real(Tcl_Interp *interp, Tcl_Obj *obj,
                              int ntuple_id, int require_float_range,
                              column_or_real *value);
static int get_simple_filter(Tcl_Interp *interp, Tcl_Obj *obj,
                             int ntuple_id, simple_filter *filter);

int get_axis_from_obj(Tcl_Interp *interp, Tcl_Obj *obj,
		      int ndim, int strict, int *axis)
{
    int status = TCL_ERROR;
    char *caxis = Tcl_GetStringFromObj(obj, NULL);

    if (caxis[0] == '\0')
    {
	if (!strict)
	{
	    *axis = HS_AXIS_NONE;
	    status = TCL_OK;
	}
    }
    else if (caxis[1] == '\0')
    {
	if (ndim > 0 && (caxis[0] == 'x' || caxis[0] == 'X'))
	{
	    *axis = HS_AXIS_X;
	    status = TCL_OK;
	}
	else if (ndim > 1 && (caxis[0] == 'y' || caxis[0] == 'Y'))
	{
	    *axis = HS_AXIS_Y;
	    status = TCL_OK;
	}
	else if (ndim > 2 && (caxis[0] == 'z' || caxis[0] == 'Z'))
	{
	    *axis = HS_AXIS_Z;
	    status = TCL_OK;
	}
    }
    if (status == TCL_ERROR)
    {
	Tcl_AppendResult(interp, "invalid axis argument \"",
			 caxis, "\", expected", NULL);
	if (ndim > 0)
	    Tcl_AppendResult(interp, " x", NULL);
	if (ndim > 1)
	    Tcl_AppendResult(interp, ", y", NULL);
	if (ndim > 2)
	    Tcl_AppendResult(interp, ", z", NULL);
	if (!strict)
	{
	    if (ndim > 0)
		Tcl_AppendResult(interp, ", empty string", NULL);
	    else
		Tcl_AppendResult(interp, " an empty string", NULL);
	}
    }
    return status;
}

int hs_concat_histograms(int uid, char *title, char *category,
			 int id1, int id2)
{
  int htype1, htype2;
  
  htype1 = hs_type(id1);
  htype2 = hs_type(id2);

  if (htype1 != HS_1D_HISTOGRAM &&
      htype1 != HS_2D_HISTOGRAM &&
      htype1 != HS_3D_HISTOGRAM)
  {
      if (htype1 == HS_NONE)
	  fprintf(stderr, 
		  "concat_histograms: item with id %d does not exist\n", id1);
      else
	  fprintf(stderr, 
		  "concat_histograms: item with id %d is not a histogram\n", id1);
      return -1;
  }
  if (htype2 != HS_1D_HISTOGRAM &&
      htype2 != HS_2D_HISTOGRAM &&
      htype2 != HS_3D_HISTOGRAM)
  {
      if (htype2 == HS_NONE)
	  fprintf(stderr, 
		  "concat_histograms: item with id %d does not exist\n", id2);
      else
	  fprintf(stderr, 
		  "concat_histograms: item with id %d is not a histogram\n", id2);
      return -1;
  }
  if (htype1 != htype2)
  {
      fprintf(stderr, 
	      "concat_histograms: histograms %d and %d have different dimensionalities\n",
	      id1, id2);
      return -1;
  }
  if (htype1 == HS_1D_HISTOGRAM)
      return concat_1d_histograms(uid, title, category, id1, id2);
  else if (htype1 == HS_2D_HISTOGRAM)
      return concat_2d_histograms(uid, title, category, id1, id2);
  else if (htype1 == HS_3D_HISTOGRAM)
      return concat_3d_histograms(uid, title, category, id1, id2);
  else
  {
      assert(0);
      return -1;
  }
}


static int concat_3d_histograms(int uid, char *title, char *category, int id1, int id2)
{
    int i, nbins_1, nbins_2, errstat1, errstat2, nbins;
    int axis = -1, newid = -1;
    int nbins1[3], nbins2[3], newbins[3];
    float min1[3], min2[3], max1[3], max2[3];
    float eps, bw1, bw2, bwnew;
    float *ppdat = NULL, *pnerr, *pperr;
    char stringbuf[1024];

    hs_3d_hist_num_bins(id1, &nbins1[0], &nbins1[1], &nbins1[2]);
    hs_3d_hist_num_bins(id2, &nbins2[0], &nbins2[1], &nbins2[2]);
    hs_3d_hist_range(id1, &min1[0], &max1[0], &min1[1], &max1[1], &min1[2], &max1[2]);
    hs_3d_hist_range(id2, &min2[0], &max2[0], &min2[1], &max2[1], &min2[2], &max2[2]);

    if (min1[0] == min2[0] && max1[0] == max2[0] && nbins1[0] == nbins2[0] &&
	min1[1] == min2[1] && max1[1] == max2[1] && nbins1[1] == nbins2[1])
	if (!(min1[2] == min2[2] && max1[2] == max2[2] && nbins1[2] == nbins2[2]))
	{
	    axis = 2; /* Will try to concatenate along z */
	}

    if (min1[0] == min2[0] && max1[0] == max2[0] && nbins1[0] == nbins2[0] &&
	min1[2] == min2[2] && max1[2] == max2[2] && nbins1[2] == nbins2[2])
	if (!(min1[1] == min2[1] && max1[1] == max2[1] && nbins1[1] == nbins2[1]))
	{
	    axis = 1; /* Will try to concatenate along y */
	}

    if (min1[1] == min2[1] && max1[1] == max2[1] && nbins1[1] == nbins2[1] &&
	min1[2] == min2[2] && max1[2] == max2[2] && nbins1[2] == nbins2[2])
	if (!(min1[0] == min2[0] && max1[0] == max2[0] && nbins1[0] == nbins2[0]))
	{
	    axis = 0; /* Will try to concatenate along x */
	}

    if (axis >= 0)
    {
	bw1 = (max1[axis]-min1[axis])/(float)nbins1[axis];
	bw2 = (max2[axis]-min2[axis])/(float)nbins2[axis];
	if (bw1 < bw2)
	    eps = bw1/4.0f;
	else
	    eps = bw2/4.0f;

	if (fabs(max2[axis] - min1[axis]) < eps)
	{
	    /* The histograms may be compatible, but we need to reverse the order */
	    swap(id1, id2, i);
	    hs_3d_hist_num_bins(id1, &nbins1[0], &nbins1[1], &nbins1[2]);
	    hs_3d_hist_num_bins(id2, &nbins2[0], &nbins2[1], &nbins2[2]);
	    hs_3d_hist_range(id1, &min1[0], &max1[0], &min1[1], &max1[1], &min1[2], &max1[2]);
	    hs_3d_hist_range(id2, &min2[0], &max2[0], &min2[1], &max2[1], &min2[2], &max2[2]);
	}

        if (fabs(max1[axis] - min2[axis]) < eps)
	{
	    for (i=0; i<3; ++i)
		newbins[i] = nbins1[i];
	    newbins[axis] = nbins1[axis]+nbins2[axis];
	    bwnew = (max2[axis] - min1[axis])/(float)newbins[axis];
	    if ((fabs(min1[axis] + bwnew*nbins1[axis] - max1[axis]) < eps) && 
		(fabs(min2[axis] + bwnew*nbins2[axis] - max2[axis]) < eps))
	    {
		/* The histograms appear to be bin-compatible. Check that 
		   they either both have errors or both have no errors. */
		nbins_1 = nbins1[0]*nbins1[1]*nbins1[2];
		nbins_2 = nbins2[0]*nbins2[1]*nbins2[2];
		nbins = nbins_1 + nbins_2;
		if ((ppdat = (float *)malloc(3*nbins*sizeof(float))) == NULL)
		{
		    fprintf(stderr, "concat_3d_histograms: out of memory");
		    goto fail;
		}
		pnerr = ppdat+nbins;
		pperr = pnerr+nbins;
		errstat1 = hs_3d_hist_errors(id1, pperr, pnerr);
		errstat2 = hs_3d_hist_errors(id2, pperr+nbins_1, pnerr+nbins_1);
		if (errstat1 != errstat2)
		{
		    fprintf(stderr, 
			    "concat_3d_histograms: histograms %d and %d have incompatible error data\n",
			    id1, id2);
		    goto fail;
		}
		hs_3d_hist_bin_contents(id1, ppdat);
		hs_3d_hist_bin_contents(id2, ppdat+nbins_1);
		hs_3d_hist_labels(id1, stringbuf, stringbuf+256, stringbuf+512, stringbuf+768);
		newid = hs_create_3d_hist(uid, title, category, stringbuf, stringbuf+256,
					  stringbuf+512, stringbuf+768,
					  newbins[0], newbins[1], newbins[2],
					  min1[0], max2[0], min1[1], max2[1], min1[2], max2[2]);
		if (newid <= 0)
		{
		    fprintf(stderr, "concat_3d_histograms: failed to create a new histogram\n");
		    goto fail;
		}

		if (axis == 0)
		{
		    if (errstat1 == HS_NO_ERRORS)
			hs_3d_hist_block_fill(newid, ppdat, NULL, NULL);
		    else if (errstat1 == HS_POS_ERRORS)
			hs_3d_hist_block_fill(newid, ppdat, pperr, NULL);
		    else
			hs_3d_hist_block_fill(newid, ppdat, pperr, pnerr);
		}
		else
		{
		    /* Need to rearrange the data */
		    float *ptmp = NULL;
		    int j, k;
#define rearrange_3d_data(to, from) do {\
    for (i=0; i<nbins1[0]; ++i)\
	for (j=0; j<nbins1[1]; ++j)\
	    for (k=0; k<nbins1[2]; ++k)\
		to[(i*newbins[1]+j)*newbins[2]+k] = \
		    from[(i*nbins1[1]+j)*nbins1[2]+k];\
    if (axis == 1) {\
	for (i=0; i<nbins2[0]; ++i)\
	    for (j=0; j<nbins2[1]; ++j)\
		for (k=0; k<nbins2[2]; ++k)\
		    to[(i*newbins[1]+j+nbins1[1])*newbins[2]+k] = \
			from[nbins_1+(i*nbins2[1]+j)*nbins2[2]+k];\
    } else if (axis == 2) {\
	for (i=0; i<nbins2[0]; ++i)\
	    for (j=0; j<nbins2[1]; ++j)\
		for (k=0; k<nbins2[2]; ++k)\
		    to[(i*newbins[1]+j)*newbins[2]+k+nbins1[2]] = \
			from[nbins_1+(i*nbins2[1]+j)*nbins2[2]+k];\
    } else {\
	assert(0);\
    }\
} while(0);
		    if (errstat1 == HS_BOTH_ERRORS)
		    {
			if ((ptmp = (float *)malloc(nbins*sizeof(float))) == NULL)
			{
			    fprintf(stderr, "concat_3d_histograms: out of memory");
			    goto fail;
			}
			rearrange_3d_data(ptmp, pnerr);
		    }
		    if (errstat1 == HS_BOTH_ERRORS || errstat1 == HS_POS_ERRORS)
			rearrange_3d_data(pnerr, pperr);
		    rearrange_3d_data(pperr, ppdat);

		    if (errstat1 == HS_NO_ERRORS)
			hs_3d_hist_block_fill(newid, pperr, NULL, NULL);
		    else if (errstat1 == HS_POS_ERRORS)
			hs_3d_hist_block_fill(newid, pperr, pnerr, NULL);
		    else
			hs_3d_hist_block_fill(newid, pperr, pnerr, ptmp);
		    if (ptmp)
			free(ptmp);
		}

		free(ppdat);
		return newid;
	    }
	}
    }

    fprintf(stderr, 
	    "concat_3d_histograms: histograms %d and %d are not bin-compatible\n",
	    id1, id2);
    return -1;

 fail:
    if (ppdat)
	free(ppdat);
    if (newid > 0)
	hs_delete(newid);
    return -1;
}


static int concat_2d_histograms(int uid, char *title, char *category, int id1, int id2)
{
    int ax2 = 0, axis = -1;
    int i, j, itmp, nbins1[2], nbins2[2], newbins[2], nbins_1, nbins_2, nbins;
    int errstat1, errstat2, newid = -1;
    float min1[2], min2[2], max1[2], max2[2];
    float eps, bw1, bw2, bwnew;
    float *ppdat = NULL, *pperr, *pnerr;
    char stringbuf[768];

    hs_2d_hist_num_bins(id1, &nbins1[0], &nbins1[1]);
    hs_2d_hist_num_bins(id2, &nbins2[0], &nbins2[1]);
    hs_2d_hist_range(id1, &min1[0], &max1[0], &min1[1], &max1[1]);
    hs_2d_hist_range(id2, &min2[0], &max2[0], &min2[1], &max2[1]);

    if (min1[0] == min2[0] && max1[0] == max2[0] && nbins1[0] == nbins2[0])
	if (!(min1[1] == min2[1] && max1[1] == max2[1] && nbins1[1] == nbins2[1]))
	{
	    axis = 1; /* Will try to concatenate along y */
	    ax2 = 0;
	}
  
    if (min1[1] == min2[1] && max1[1] == max2[1] && nbins1[1] == nbins2[1])
	if (!(min1[0] == min2[0] && max1[0] == max2[0] && nbins1[0] == nbins2[0]))
	{
	    axis = 0; /* Will try to concatenate along x */
	    ax2 = 1;
	}

    if (axis >= 0)
    {
	bw1 = (max1[axis]-min1[axis])/(float)nbins1[axis];
	bw2 = (max2[axis]-min2[axis])/(float)nbins2[axis];
	if (bw1 < bw2)
	    eps = bw1/4.0f;
	else
	    eps = bw2/4.0f;

	if (fabs(max2[axis] - min1[axis]) < eps)
	{
	    /* The histograms may be compatible, but we need to reverse the order */
	    swap(id1, id2, itmp);
	    hs_2d_hist_num_bins(id1, &nbins1[0], &nbins1[1]);
	    hs_2d_hist_num_bins(id2, &nbins2[0], &nbins2[1]);
	    hs_2d_hist_range(id1, &min1[0], &max1[0], &min1[1], &max1[1]);
	    hs_2d_hist_range(id2, &min2[0], &max2[0], &min2[1], &max2[1]);
	}

	if (fabs(max1[axis] - min2[axis]) < eps)
	{
	    newbins[axis] = nbins1[axis]+nbins2[axis];
	    newbins[ax2] = nbins1[ax2];
	    bwnew = (max2[axis] - min1[axis])/(float)newbins[axis];
	    if ((fabs(min1[axis] + bwnew*nbins1[axis] - max1[axis]) < eps) && 
		(fabs(min2[axis] + bwnew*nbins2[axis] - max2[axis]) < eps))
	    {
		/* The histograms appear to be bin-compatible. Check that 
		   they either both have errors or both have no errors. */
		nbins_1 = nbins1[0]*nbins1[1];
		nbins_2 = nbins2[0]*nbins2[1];
		nbins = nbins_1 + nbins_2;
		if ((ppdat = (float *)malloc(3*nbins*sizeof(float))) == NULL)
		{
		    fprintf(stderr, "concat_2d_histograms: out of memory");
		    goto fail;
		}
		pnerr = ppdat+nbins;
		pperr = pnerr+nbins;
		errstat1 = hs_2d_hist_errors(id1, pperr, pnerr);
		errstat2 = hs_2d_hist_errors(id2, pperr+nbins_1, pnerr+nbins_1);
		if (errstat1 != errstat2)
		{
		    fprintf(stderr, 
			    "concat_2d_histograms: histograms %d and %d have incompatible error data\n",
			    id1, id2);
		    goto fail;
		}
		hs_2d_hist_bin_contents(id1, ppdat);
		hs_2d_hist_bin_contents(id2, ppdat+nbins_1);
		hs_2d_hist_labels(id1, stringbuf, stringbuf+256, stringbuf+512);	
		newid = hs_create_2d_hist(uid, title, category, stringbuf, stringbuf+256, 
					  stringbuf+512, newbins[0], newbins[1],
					  min1[0], max2[0], min1[1], max2[1]);
		if (newid <= 0)
		{
		    fprintf(stderr, "concat_2d_histograms: failed to create a new histogram\n");
		    goto fail;
		}

		if (axis == 0)
		{
		    if (errstat1 == HS_NO_ERRORS)
			hs_2d_hist_block_fill(newid, ppdat, NULL, NULL);
		    else if (errstat1 == HS_POS_ERRORS)
			hs_2d_hist_block_fill(newid, ppdat, pperr, NULL);
		    else
			hs_2d_hist_block_fill(newid, ppdat, pperr, pnerr);
		}
		else
		{
		    /* Need to rearrange the data */
		    float *ptmp = NULL;
		    if (errstat1 == HS_BOTH_ERRORS)
		    {
			if ((ptmp = (float *)malloc(nbins*sizeof(float))) == NULL)
			{
			    fprintf(stderr, "concat_2d_histograms: out of memory");
			    goto fail;
			}
			for (i=0; i<nbins1[0]; i++)
			    for (j=0; j<nbins1[1]; j++)
				ptmp[i*newbins[1]+j] = pnerr[i*nbins1[1]+j];
			for (i=0; i<nbins2[0]; i++)
			    for (j=0; j<nbins2[1]; j++)
				ptmp[i*newbins[1]+j+nbins1[1]] = pnerr[nbins_1+i*nbins2[1]+j];
		    }
		    if (errstat1 == HS_BOTH_ERRORS || errstat1 == HS_POS_ERRORS)
		    {
			for (i=0; i<nbins1[0]; i++)
			    for (j=0; j<nbins1[1]; j++)
				pnerr[i*newbins[1]+j] = pperr[i*nbins1[1]+j];
			for (i=0; i<nbins2[0]; i++)
			    for (j=0; j<nbins2[1]; j++)
				pnerr[i*newbins[1]+j+nbins1[1]] = pperr[nbins_1+i*nbins2[1]+j];
		    }
		    for (i=0; i<nbins1[0]; i++)
			for (j=0; j<nbins1[1]; j++)
			    pperr[i*newbins[1]+j] = ppdat[i*nbins1[1]+j];
		    for (i=0; i<nbins2[0]; i++)
			for (j=0; j<nbins2[1]; j++)
			    pperr[i*newbins[1]+j+nbins1[1]] = ppdat[nbins_1+i*nbins2[1]+j];

		    if (errstat1 == HS_NO_ERRORS)
			hs_2d_hist_block_fill(newid, pperr, NULL, NULL);
		    else if (errstat1 == HS_POS_ERRORS)
			hs_2d_hist_block_fill(newid, pperr, pnerr, NULL);
		    else
			hs_2d_hist_block_fill(newid, pperr, pnerr, ptmp);
		    if (ptmp)
			free(ptmp);
		}

		free(ppdat);
		return newid;
	    }
	}
    }
  
    fprintf(stderr, 
	    "concat_2d_histograms: histograms %d and %d are not bin-compatible\n",
	    id1, id2);
    return -1;

 fail:
    if (newid > 0)
	hs_delete(newid);
    if (ppdat)
	free(ppdat);
    return -1;
}


static int concat_1d_histograms(int uid, char *title, char *category, int id1, int id2)
{
    int itmp, errstat1, errstat2, newid, nbins1, nbins2, nbins;
    float ftmp, eps, bw1, bw2, bwnew, xmin1, xmin2, xmax1, xmax2;
    float *ppdat, *pperr, *pnerr;
    char stringbuf[512];

    /* Check the compatibility of the histograms */
    nbins1 = hs_1d_hist_num_bins(id1);
    nbins2 = hs_1d_hist_num_bins(id2);
    nbins = nbins1 + nbins2;
    hs_1d_hist_range(id1, &xmin1, &xmax1);
    hs_1d_hist_range(id2, &xmin2, &xmax2);
    bw1 = (xmax1 - xmin1)/(float)nbins1;
    bw2 = (xmax2 - xmin2)/(float)nbins2;
    if (bw1 < bw2)
	eps = bw1/4.0f;
    else
	eps = bw2/4.0f;
    if (fabs(xmax2 - xmin1) < eps)
    {
	/* The histograms may be compatible, but we need to reverse the order */
	swap(id1, id2, itmp);
	swap(nbins1, nbins2, itmp);
	swap(xmin1, xmin2, ftmp);
	swap(xmax1, xmax2, ftmp);
    }
    if (fabs(xmax1 - xmin2) < eps)
    {
	bwnew = (xmax2 - xmin1)/(float)nbins;
	if ((fabs(xmin1 + bwnew*nbins1 - xmax1) < eps) && 
	    (fabs(xmin2 + bwnew*nbins2 - xmax2) < eps))
	{
	    /* The histograms appear to be bin-compatible. Check that 
	       they either both have errors or both have no errors. */
	    if ((ppdat = (float *)malloc(3*nbins*sizeof(float))) == NULL)
	    {
		fprintf(stderr, "concat_1d_histograms: out of memory");
		return -1;
	    }
	    pperr = ppdat + nbins;
	    pnerr = pperr + nbins;
	    errstat1 = hs_1d_hist_errors(id1, pperr, pnerr);
	    errstat2 = hs_1d_hist_errors(id2, pperr+nbins1, pnerr+nbins1);
	    if (errstat1 != errstat2)
	    {
		fprintf(stderr, 
			"concat_1d_histograms: histograms %d and %d have incompatible error data\n",
			id1, id2);
		free(ppdat);
		return -1;
	    }
	    hs_1d_hist_bin_contents(id1, ppdat);
	    hs_1d_hist_bin_contents(id2, ppdat+nbins1);
	    hs_1d_hist_labels(id1, stringbuf, stringbuf+256);
	    newid = hs_create_1d_hist(uid, title, category, stringbuf, 
				      stringbuf+256, nbins, xmin1, xmax2);
	    if (newid <= 0)
	    {
		fprintf(stderr, "concat_1d_histograms: failed to create a new histogram\n");
		free(ppdat);
		return -1;
	    }
	    if (errstat1 == HS_NO_ERRORS)
		hs_1d_hist_block_fill(newid, ppdat, NULL, NULL);
	    else if (errstat1 == HS_POS_ERRORS)
		hs_1d_hist_block_fill(newid, ppdat, pperr, NULL);
	    else
		hs_1d_hist_block_fill(newid, ppdat, pperr, pnerr);

	    free(ppdat);
	    return newid;
	}
    }

    fprintf(stderr, 
	    "concat_1d_histograms: histograms %d and %d are not bin-compatible\n",
	    id1, id2);
    return -1;
}


int hs_transpose_histogram(int uid, char *title, char *category, int id)
{
    int new_id, i, j, errtype;
    int x_bin_num, y_bin_num, nbins;
    float xmin, xmax, ymin, ymax;
    float *pd = NULL, *pe = NULL, *pn = NULL, *ptmp = NULL;
    char stringbuf[768];
  
    if (hs_type(id) != HS_2D_HISTOGRAM)
    {
	if (hs_type(id) == HS_NONE)
	    fprintf(stderr, 
		    "transpose_histogram: item with id %d does not exist\n", id);
	else
	    fprintf(stderr, 
		    "transpose_histogram: item with id %d is not a 2d histogram\n", id);
	return -1;
    }
  
    hs_2d_hist_range(id, &xmin, &xmax, &ymin, &ymax);
    hs_2d_hist_num_bins(id, &x_bin_num, &y_bin_num);
    hs_2d_hist_labels(id, stringbuf, stringbuf+256, stringbuf+512);

    new_id = hs_create_2d_hist(uid, title, category, 
			       stringbuf+256, stringbuf, stringbuf+512,
			       y_bin_num, x_bin_num,
			       ymin, ymax, xmin, xmax);
    if (new_id <= 0)
    {
	fprintf(stderr, "transpose_histogram: failed to create a new histogram\n");
	return -1;
    }
    nbins = x_bin_num*y_bin_num;
    pe = (float *)malloc(nbins*sizeof(float));
    pn = (float *)malloc(nbins*sizeof(float));
    if (pe == NULL || pn == NULL)
    {
	fprintf(stderr, "transpose_histogram: out of memory\n");
	goto fail;
    }
    errtype = hs_2d_hist_errors(id, pe, pn);
    if (errtype == HS_NO_ERRORS)
    {
	pd = pe;
	pe = NULL;
	ptmp = pn;
	pn = NULL;
    } 
    else if (errtype == HS_POS_ERRORS)
    {
	pd = pn;
	pn = NULL;
    }
    if (pd == NULL)
	pd = (float *)malloc(nbins*sizeof(float));
    if (ptmp == NULL)
	ptmp = (float *)malloc(nbins*sizeof(float));
    if (pd == NULL || ptmp == NULL)
    {
	fprintf(stderr, "transpose_histogram: out of memory\n");
	goto fail;
    }
    hs_2d_hist_bin_contents(id, pd);

    for (i=0; i<x_bin_num; i++)
	for (j=0; j<y_bin_num; j++)
	    ptmp[j*x_bin_num + i] = pd[i*y_bin_num+j];
    if (pe)
    {
	for (i=0; i<x_bin_num; i++)
	    for (j=0; j<y_bin_num; j++)
		pd[j*x_bin_num + i] = pe[i*y_bin_num+j];
	if (pn)
	{
	    for (i=0; i<x_bin_num; i++)
		for (j=0; j<y_bin_num; j++)
		    pe[j*x_bin_num + i] = pn[i*y_bin_num+j];
	}
	else
	{
	    free(pe);
	    pe = NULL;
	}
    }
    else
    {
	free(pd);
	pd = NULL;
    }

    hs_2d_hist_block_fill(new_id, ptmp, pd, pe);

    if (ptmp)
	free(ptmp);
    if (pn)
	free(pn);
    if (pe)
	free(pe);
    if (pd)
	free(pd);

    return new_id;

 fail:
    hs_delete(new_id);
    if (pd)
	free(pd);
    if (pn)
	free(pn);
    if (pe)
	free(pe);
    if (ptmp)
	free(ptmp);
    return -1;
}


int project_3d_histogram(int uid, char *title, char *category, int id,
	       int axis1, int axis2, int proj_type, int suppress_zero)
{
    int i, j, slicesize, status, new_id = -1;
    int x_bin_num, y_bin_num, z_bin_num, nbins, slicebins, use_coords;
    int projbins_x = 0, projbins_y = 0, projbins, arrind;
    float xmin, xmax, ymin, ymax, zmin, zmax, dumm;
    float qmin, q25, q75, qmax, binmin = 0.f, step = 0.f;
    float *projerrors, *projdata = NULL, *slicedata = NULL;
    char xlabel[256], ylabel[256], zlabel[256], qlabel[256];
    struct weighted_point *ortobins = NULL;

    /* Check if we will need bin coordinates */
    use_coords = (proj_type == HS_CALC_PROJ_COORDAVE ||
		  proj_type == HS_CALC_PROJ_COORDRMS ||
		  proj_type == HS_CALC_PROJ_COORDMED ||
		  proj_type == HS_CALC_PROJ_COORDRANGE);

    hs_3d_hist_num_bins(id, &x_bin_num, &y_bin_num, &z_bin_num);
    nbins = x_bin_num*y_bin_num*z_bin_num;
    if (x_bin_num <= 0 || y_bin_num <= 0 || z_bin_num <= 0)
    {
	fprintf(stderr, "project_3d_histogram: bad parent id\n");
	goto fail;
    }
    hs_3d_hist_labels(id, xlabel, ylabel, zlabel, qlabel);
    hs_3d_hist_range(id, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);

    assert(axis1 >= 0 && axis1 < N_HS_AXIS_TYPES);
    assert(axis2 >= 0 && axis2 < N_HS_AXIS_TYPES);
    assert(axis1 != axis2);
    if (axis1 == HS_AXIS_NONE)
    {
	axis1 = axis2;
	axis2 = HS_AXIS_NONE;
    }
    if (axis2 == HS_AXIS_NONE)
    {
	/* Projection onto a 1d histogram */
	if (use_coords)
	{
	    fprintf(stderr, "project_3d_histogram: projection type %d "
		    "can't be used with 1d projections\n", proj_type);
	    goto fail;
	}

	switch (axis1)
	{
	case HS_AXIS_X:
	    projbins = x_bin_num;
	    new_id = hs_create_1d_hist(uid, title, category, xlabel,
				       qlabel, projbins, xmin, xmax);
	    break;
	case HS_AXIS_Y:
	    projbins = y_bin_num;
	    new_id = hs_create_1d_hist(uid, title, category, ylabel,
				       qlabel, projbins, ymin, ymax);
	    break;
	case HS_AXIS_Z:
	    projbins = z_bin_num;
	    new_id = hs_create_1d_hist(uid, title, category, zlabel,
				       qlabel, projbins, zmin, zmax);
	    break;
	default:
	    assert(0);
	}
    }
    else
    {
	/* Projection onto a 2d histogram */
	char *vlabel = (use_coords ? NULL : qlabel);
	if (axis2 < axis1)
	{
	    int itmp = axis2;
	    axis2 = axis1;
	    axis1 = itmp;
	}
	if (axis1 == HS_AXIS_X && axis2 == HS_AXIS_Y)
	{
	    projbins_x = x_bin_num;
	    projbins_y = y_bin_num;
	    if (vlabel == NULL)
		vlabel = zlabel;
	    binmin = zmin;
	    step = (zmax - zmin)/(float)z_bin_num;
	    new_id = hs_create_2d_hist(uid, title, category,
				       xlabel, ylabel, vlabel,
				       projbins_x, projbins_y,
				       xmin, xmax, ymin, ymax);
	}
	else if (axis1 == HS_AXIS_X && axis2 == HS_AXIS_Z)
	{
	    projbins_x = x_bin_num;
	    projbins_y = z_bin_num;
	    if (vlabel == NULL)
		vlabel = ylabel;
	    binmin = ymin;
	    step = (ymax - ymin)/(float)y_bin_num;
	    new_id = hs_create_2d_hist(uid, title, category,
				       xlabel, zlabel, vlabel,
				       projbins_x, projbins_y,
				       xmin, xmax, zmin, zmax);
	}
	else if (axis1 == HS_AXIS_Y && axis2 == HS_AXIS_Z)
	{
	    projbins_x = y_bin_num;
	    projbins_y = z_bin_num;
	    if (vlabel == NULL)
		vlabel = xlabel;
	    binmin = zmin;
	    step = (zmax - zmin)/(float)z_bin_num;
	    new_id = hs_create_2d_hist(uid, title, category,
				       ylabel, zlabel, vlabel,
				       projbins_x, projbins_y,
				       ymin, ymax, zmin, zmax);
	}
	else
	    assert(0);
	projbins = projbins_x * projbins_y;
    }

    if (new_id <= 0)
	goto fail;
    slicebins = nbins/projbins;
    projdata = (float *)malloc(2*projbins*sizeof(float));
    slicedata = (float *)malloc(slicebins*sizeof(float));
    if (projdata == NULL || slicedata == NULL)
    {
	fprintf(stderr, "project_3d_histogram: out of memory\n");
	goto fail;
    }
    projerrors = projdata + projbins;
    if (use_coords)
    {
	ortobins = (struct weighted_point *)malloc(
	    slicebins*sizeof(struct weighted_point));
	if (ortobins == NULL)
	{
	    fprintf(stderr, "project_3d_histogram: out of memory\n");
	    goto fail;
	}
    }

    /* Use sequential slices to get the stats. This is very
       slow but beats rewriting the slicing code. */
#define get_slice_property do {\
	status = hs_slice_contents(id, axis1, i, axis2, j, slicebins,\
				   slicedata, NULL, NULL, &slicesize);\
	assert(status >= 0);\
	assert(slicesize == slicebins);\
	switch (proj_type)\
	{\
	case HS_CALC_PROJ_AVE:\
	    status = arr_stats(slicedata, slicebins, !suppress_zero,\
			       projdata+arrind, projerrors+arrind);\
	    break;\
	case HS_CALC_PROJ_RMS:\
	    status = arr_stats(slicedata, slicebins, !suppress_zero,\
			       &dumm, projdata+arrind);\
            projerrors[arrind] = 0.f;\
	    break;\
	case HS_CALC_PROJ_SUM:\
	    {\
                register int k;\
		register float sum = 0.f;\
		for (k=slicebins-1; k>=0; --k)\
		    sum += slicedata[k];\
		projdata[arrind] = sum;\
                projerrors[arrind] = 0.f;\
	    }\
	    break;\
	case HS_CALC_PROJ_MIN:\
	    {\
                register int k;\
                register float minval = slicedata[0];\
		for (k=slicebins-1; k>0; --k)\
		    if (slicedata[k] < minval)\
			minval = slicedata[k];\
		projdata[arrind] = minval;\
                projerrors[arrind] = 0.f;\
	    }\
	    break;\
	case HS_CALC_PROJ_MAX:\
	    {\
                register int k;\
                register float maxval = slicedata[0];\
		for (k=slicebins-1; k>0; --k)\
		    if (slicedata[k] > maxval)\
			maxval = slicedata[k];\
		projdata[arrind] = maxval;\
                projerrors[arrind] = 0.f;\
	    }\
	    break;\
	case HS_CALC_PROJ_MED:\
	    status = arr_medirange(slicedata, slicebins, !suppress_zero,\
				   &qmin, &q25, projdata+arrind, &q75, &qmax);\
	    projerrors[arrind] = (q75 - q25)*RANGE2SIG;\
	    break;\
	case HS_CALC_PROJ_RANGE:\
	    status = arr_medirange(slicedata, slicebins, !suppress_zero,\
				   &qmin, &q25, &dumm, &q75, &qmax);\
	    projdata[arrind] = (q75 - q25)*RANGE2SIG;\
            projerrors[arrind] = 0.f;\
	    break;\
	default:\
	    assert(0);\
	}\
} while(0);

    arrind = 0;
    if (axis2 == HS_AXIS_NONE)
    {
	/* Projection onto a 1d histogram */
	j = -1;
	if (use_coords)
	    assert(0);
	for (i=0; i<projbins; ++i, ++arrind)
	    get_slice_property;
	hs_1d_hist_block_fill(new_id, projdata, projerrors, NULL);
    }
    else
    {
	/* Projection onto a 2d histogram */
	if (use_coords)
	{
	    /* Figure out the bin coordinates */
	    binmin += step/2.f;
	    for (i=0; i<slicebins; ++i)
		ortobins[i].x = binmin + step*(float)i;
	}
	for (i=0; i<projbins_x; ++i)
	    for (j=0; j<projbins_y; ++j, ++arrind)
	    {
		if (use_coords)
		{
		    status = hs_slice_contents(id, axis1, i, axis2, j, slicebins,
					       slicedata, NULL, NULL, &slicesize);
		    assert(status >= 0);
		    assert(slicesize == slicebins);
		    {
			register int k;
			for (k=0; k<slicebins; ++k)
			    ortobins[k].w = slicedata[k];
		    }
		    switch (proj_type)
		    {
		    case HS_CALC_PROJ_COORDAVE:
			status = arr_stats_weighted(ortobins, slicebins,
						    projdata+arrind, projerrors+arrind);
			break;

		    case HS_CALC_PROJ_COORDRMS:
			status = arr_stats_weighted(ortobins, slicebins, &dumm,
						    projdata+arrind);
                        projerrors[arrind] = 0.f;
			break;
			
		    case HS_CALC_PROJ_COORDMED:
			status = arr_medirange_weighted(ortobins, slicebins, &qmin, &q25,
							projdata+arrind, &q75, &qmax);
                        projerrors[arrind] = (q75 - q25)*RANGE2SIG;
			break;

		    case HS_CALC_PROJ_COORDRANGE:
			status = arr_medirange_weighted(ortobins, slicebins, &qmin, &q25,
							&dumm, &q75, &qmax);
			projdata[arrind] = (q75 - q25)*RANGE2SIG;
                        projerrors[arrind] = 0.f;
			break;

		    default:
			assert(0);
		    }
		}
		else
		    get_slice_property;
	    }
	hs_2d_hist_block_fill(new_id, projdata, projerrors, NULL);
    }

    if (ortobins)
	free(ortobins);
    if (slicedata)
	free(slicedata);
    if (projdata)
	free(projdata);
    return new_id;

 fail:
    if (ortobins)
	free(ortobins);
    if (slicedata)
	free(slicedata);
    if (projdata)
	free(projdata);
    if (new_id > 0)
	hs_delete(new_id);
    return -1;
}


int project_2d_histogram(int uid, char *title, char *category, int id,
		         int axis, int proj_type, int suppress_zero)
{
    int new_id, i, j, norto, use_coords;
    int x_bin_num, y_bin_num, nbins, projbins;
    float *htmp, *projerrors;
    float *projdata = NULL, *hdata = NULL, *htransp = NULL;
    struct weighted_point *ortobins = NULL;

    /* Check if we will need bin coordinates */
    use_coords = (proj_type == HS_CALC_PROJ_COORDAVE ||
		  proj_type == HS_CALC_PROJ_COORDRMS ||
		  proj_type == HS_CALC_PROJ_COORDMED ||
		  proj_type == HS_CALC_PROJ_COORDRANGE);

    /* Create the projection histogram */
    if ((new_id = duplicate_axis_binning(uid, title, category,
					 id, axis, use_coords)) <= 0)
	return new_id;

    hs_2d_hist_num_bins(id, &x_bin_num, &y_bin_num);
    nbins = x_bin_num*y_bin_num;
    projbins = hs_1d_hist_num_bins(new_id);
    hdata = (float *)malloc(nbins*sizeof(float));
    projdata = (float *)malloc(2*projbins*sizeof(float));
    if (hdata == NULL || projdata == NULL)
    {
	fprintf(stderr, "project_2d_histogram: out of memory\n");
	goto fail;
    }
    projerrors = projdata + projbins;
    hs_2d_hist_bin_contents(id, hdata);

    norto = (axis == HS_AXIS_X ? y_bin_num : x_bin_num);
    if (use_coords)
    {
	float xmin, xmax, ymin, ymax, step;
	ortobins = (struct weighted_point *)malloc(
	    norto*sizeof(struct weighted_point));
	if (ortobins == NULL)
	{
	    fprintf(stderr, "project_2d_histogram: out of memory\n");
	    goto fail;
	}
	hs_2d_hist_range(id, &xmin, &xmax, &ymin, &ymax);
	if (axis == HS_AXIS_X)
	{
	    xmin = ymin;
	    xmax = ymax;
	}
	step = (xmax - xmin)/(float)norto;
	xmin += step/2.f;
	for (i=0; i<norto; i++)
	    ortobins[i].x = xmin + step*(float)i;
    }

    /* Make a transpose in case of y projection */
    if (axis == HS_AXIS_Y)
    {
	htransp = (float *)malloc(nbins*sizeof(float));
	if (htransp == NULL)
	{
	    fprintf(stderr, "project_2d_histogram: out of memory\n");
	    goto fail;
	}
	for (i=0; i<x_bin_num; i++)
	    for (j=0; j<y_bin_num; j++)
		htransp[j*x_bin_num+i] = hdata[i*y_bin_num+j];
	htmp = hdata;
	hdata = htransp;
	htransp = htmp;
    }

    /* Finally, go over all bins and fill the projection */
    if (use_coords)
    {
	for (i=0; i<projbins; i++)
	{
	    htmp = hdata+i*norto;
	    for (j=0; j<norto; j++)
		ortobins[j].w = htmp[j];
	    projection_fill_weighted(ortobins, norto, proj_type,
				     projdata+i, projerrors+i);
	}
    }
    else
    {
	for (i=0; i<projbins; i++)
	    projection_fill(hdata+i*norto, norto, proj_type,
			    suppress_zero, projdata+i, projerrors+i);
    }
    hs_1d_hist_block_fill(new_id, projdata, projerrors, NULL);
    if (htransp)
	free(htransp);
    if (ortobins)
	free(ortobins);
    if (projdata)
	free(projdata);
    if (hdata)
	free(hdata);
    return new_id;

 fail:
    if (htransp)
	free(htransp);
    if (ortobins)
	free(ortobins);
    if (projdata)
	free(projdata);
    if (hdata)
	free(hdata);
    hs_delete(new_id);
    return -1;
}


static int slice_histogram(int uid, char *title, char *category,
			   int id, int axis, int binnum,
			   int axis2, int binnum2)
{
    int status, proj_axis, parent_type, new_id = 0;

    assert(axis == HS_AXIS_X ||
	   axis == HS_AXIS_Y ||
	   axis == HS_AXIS_Z);
    assert(axis2 == HS_AXIS_X ||
	   axis2 == HS_AXIS_Y ||
	   axis2 == HS_AXIS_Z ||
	   axis2 == HS_AXIS_NONE);
    /* The bin number should be at least 0 */
    if (binnum < 0)
	return -1;
    if (binnum2 < 0 && axis2 != HS_AXIS_NONE)
	return -1;
    if (axis == axis2)
	/* Can't specify the same axis more than once */
	return -2;
    parent_type = hs_type(id);
    if (parent_type == HS_2D_HISTOGRAM)
    {
	int nxbins, nybins;

	if (axis2 != HS_AXIS_NONE)
	    /* two slice axes specified for a 2d histogram */
	    return -3;
	hs_2d_hist_num_bins(id, &nxbins, &nybins);
	if (axis == HS_AXIS_X) {
	    proj_axis = HS_AXIS_Y;
	    if (binnum >= nxbins)
		return -1;
	} else if (axis == HS_AXIS_Y) {
	    proj_axis = HS_AXIS_X;
	    if (binnum >= nybins)
		return -1;
	} else {
	    /* invalid axis for a 2d histogram */
	    return -4;
	}
	if ((new_id = duplicate_axis_binning(uid, title, category,
					     id, proj_axis, 0)) <= 0)
	    /* This shouldn't happen really (unless we are low on memory) */
	    return -5;
    }
    else if (parent_type == HS_3D_HISTOGRAM)
    {
	int nxbins_p, nybins_p, nzbins_p, nxbins, nybins;
	float xmin_p, xmax_p, ymin_p, ymax_p, zmin_p, zmax_p;
	float xmin, xmax, ymin, ymax;
	char stringbuf[1024];
	char *xlabel, *ylabel, *vlabel = stringbuf+768;

	hs_3d_hist_num_bins(id, &nxbins_p, &nybins_p, &nzbins_p);
	hs_3d_hist_range(id, &xmin_p, &xmax_p, &ymin_p, &ymax_p, &zmin_p, &zmax_p);
	hs_3d_hist_labels(id, stringbuf, stringbuf+256, stringbuf+512, stringbuf+768);
	if (axis2 == HS_AXIS_NONE)
	{
	    if (axis == HS_AXIS_X)
	    {
		xmin = ymin_p;
		xmax = ymax_p;
		ymin = zmin_p;
		ymax = zmax_p;
		nxbins = nybins_p;
		nybins = nzbins_p;
		xlabel = stringbuf+256;
		ylabel = stringbuf+512;
		if (binnum >= nxbins_p)
		    return -1;
	    }
	    else if (axis == HS_AXIS_Y)
	    {
		xmin = xmin_p;
		xmax = xmax_p;
		ymin = zmin_p;
		ymax = zmax_p;
		nxbins = nxbins_p;
		nybins = nzbins_p;
		xlabel = stringbuf;
		ylabel = stringbuf+512;
		if (binnum >= nybins_p)
		    return -1;
	    }
	    else if (axis == HS_AXIS_Z)
	    {
		xmin = xmin_p;
		xmax = xmax_p;
		ymin = ymin_p;
		ymax = ymax_p;
		nxbins = nxbins_p;
		nybins = nybins_p;
		xlabel = stringbuf;
		ylabel = stringbuf+256;
		if (binnum >= nzbins_p)
		    return -1;
	    }
	    else
		assert(0);
	    new_id = hs_create_2d_hist(uid, title, category, xlabel,
				       ylabel, vlabel, nxbins, nybins,
				       xmin, xmax, ymin, ymax);
	    if (new_id <= 0)
		return -6;
	}
	else
	{
	    if (axis > axis2)
	    {
		int tempint;
		swap(axis, axis2, tempint);
		swap(binnum, binnum2, tempint);
	    }
	    if (axis == HS_AXIS_X && axis2 == HS_AXIS_Y)
	    {
		xmin = zmin_p;
		xmax = zmax_p;
		nxbins = nzbins_p;
		xlabel = stringbuf+512;
		if (binnum >= nxbins_p)
		    return -1;
		if (binnum2 >= nybins_p)
		    return -1;
	    }
	    else if (axis == HS_AXIS_X && axis2 == HS_AXIS_Z)
	    {
		xmin = ymin_p;
		xmax = ymax_p;
		nxbins = nybins_p;
		xlabel = stringbuf+256;
		if (binnum >= nxbins_p)
		    return -1;
		if (binnum2 >= nzbins_p)
		    return -1;
	    }
	    else if (axis == HS_AXIS_Y && axis2 == HS_AXIS_Z)
	    {
		xmin = xmin_p;
		xmax = xmax_p;
		nxbins = nxbins_p;
		xlabel = stringbuf;
		if (binnum >= nybins_p)
		    return -1;
		if (binnum2 >= nzbins_p)
		    return -1;
	    }
	    else
		assert(0);
	    new_id = hs_create_1d_hist(uid, title, category, xlabel,
				       vlabel, nxbins, xmin, xmax);
	    if (new_id <= 0)
		return -7;
	}
    }
    else
    {
	/* can't slice this item */
	return -8;
    }

    assert(new_id > 0);
    status = hs_fill_hist_slice(id, axis, binnum, axis2, binnum2, new_id);
    assert(status >= 0);
    return new_id;
}

int slice_2d_histogram(int uid, char *title, char *category, 
		       int id, int axis, int binnum)
{
  int new_id = 0, proj_axis;
  float *dataslice, *poserr, *negerr;
  
  if (axis == HS_AXIS_X)
      proj_axis = HS_AXIS_Y;
  else
      proj_axis = HS_AXIS_X;
  
  if ((new_id = duplicate_axis_binning(uid, title, category,
				       id, proj_axis, 0)) <= 0)
      return -1;

  /* Flush the data cache */
  get_data_slice(0, 0, 0, NULL, NULL, NULL);

  if (get_data_slice(id, axis, binnum, &dataslice, &poserr, &negerr))
  {
      hs_delete(new_id);
      return -1;
  }
  hs_1d_hist_block_fill(new_id, dataslice, poserr, negerr);
  return new_id;
}


static int duplicate_axis_binning(int uid, char *title, char *category,
				  int id, int axis, int use_coords)
{
    float xmin, xmax, ymin, ymax;
    int x_bin_num, y_bin_num;
    char stringbuf[768];

    assert(use_coords >= 0 && use_coords <= 1);
    if (hs_type(id) != HS_2D_HISTOGRAM)
    {
	if (hs_type(id) == HS_NONE)
	    fprintf(stderr, 
		    "duplicate_axis_binning: item with id %d does not exist\n",
		    id);
	else
	    fprintf(stderr, 
		    "duplicate_axis_binning: item with id %d is not a 2d histogram\n",
		    id);
	return -1;
    }
    if (axis == HS_AXIS_X)
    {
	hs_2d_hist_range(id, &xmin, &xmax, &ymin, &ymax);
	hs_2d_hist_num_bins(id, &x_bin_num, &y_bin_num);
	hs_2d_hist_labels(id, stringbuf, stringbuf+256, stringbuf+512);
    }
    else if (axis == HS_AXIS_Y)
    {
	hs_2d_hist_range(id, &ymin, &ymax, &xmin, &xmax);
	hs_2d_hist_num_bins(id, &y_bin_num, &x_bin_num);
	hs_2d_hist_labels(id, stringbuf+256, stringbuf, stringbuf+512);
    }
    else
	assert(0);
    return hs_create_1d_hist(uid, title, category, stringbuf, 
			     stringbuf+512-use_coords*256,
			     x_bin_num, xmin, xmax);
}


int get_data_slice(int id, int axis, int binnum, float **dataslice, 
		   float **err_pos, float **err_neg)
{
    static float *data = NULL, *poserr = NULL, *negerr = NULL;
    static int last_id = -1, last_axis = -1;
    static int axis_bins, other_bins;

    int i, j, x_bin_num, y_bin_num, nbins, errtype;
    float *ptmp, *xtmp;

    if (id == 0 || dataslice == NULL || err_pos == NULL || err_neg == NULL)
	goto fail;

    if (last_id != id || last_axis != axis)
    {
	if (hs_type(id) != HS_2D_HISTOGRAM)
	{
	    if (hs_type(id) == HS_NONE)
		fprintf(stderr, 
			"get_data_slice: item with id %d does not exist\n", id);
	    else
		fprintf(stderr, 
			"get_data_slice: item with id %d is not a 2d histogram\n", id);
	    return -1;
	}
	else
	{
	    last_id = id;
	    last_axis = axis;
	}
	hs_2d_hist_num_bins(id, &x_bin_num, &y_bin_num);
	nbins = x_bin_num*y_bin_num;
	if (axis == HS_AXIS_X)
	{
	    axis_bins = x_bin_num;
	    other_bins = y_bin_num;
	}
	else
	{
	    axis_bins = y_bin_num;
	    other_bins = x_bin_num;
	}
	data = (float *)realloc(data, nbins*sizeof(float));
	poserr = (float *)realloc(poserr, nbins*sizeof(float));
	negerr = (float *)realloc(negerr, nbins*sizeof(float));
	if (data == NULL || poserr == NULL || negerr == NULL)
	{
	    fprintf(stderr, "get_data_slice: out of memory for data\n");
	    goto fail;
	}
	hs_2d_hist_bin_contents(id, data);
	errtype = hs_2d_hist_errors(id, poserr, negerr);
	if (axis == HS_AXIS_Y)
	{
	    /* We will do a transposition for faster loading */
	    if ((ptmp = (float *)malloc(nbins*sizeof(float))) == NULL)
	    {
		fprintf(stderr, "get_data_slice: out of memory for temp\n");
		goto fail;
	    }
	    for (i=0; i<x_bin_num; i++)
		for (j=0; j<y_bin_num; j++)
		    ptmp[j*x_bin_num + i] = data[i*y_bin_num+j];
	    swap(ptmp, data, xtmp);
	    if (errtype == HS_POS_ERRORS || errtype == HS_BOTH_ERRORS)
	    {
		for (i=0; i<x_bin_num; i++)
		    for (j=0; j<y_bin_num; j++)
			ptmp[j*x_bin_num + i] = poserr[i*y_bin_num+j];
		swap(ptmp, poserr, xtmp);
		if (errtype == HS_BOTH_ERRORS)
		{
		    for (i=0; i<x_bin_num; i++)
			for (j=0; j<y_bin_num; j++)
			    ptmp[j*x_bin_num + i] = negerr[i*y_bin_num+j];
		    swap(ptmp, negerr, xtmp);
		}
	    }
	    free(ptmp);
	}
	/* Get rid of the arrays we don't need */
	if (errtype == HS_NO_ERRORS || errtype == HS_POS_ERRORS)
	{
	    free(negerr);
	    negerr = NULL;
	    if (errtype == HS_NO_ERRORS)
	    {
		free(poserr);
		poserr = NULL;
	    }
	}
    }

    if (binnum < 0)
	binnum = 0;
    if (binnum >= axis_bins)
	binnum = axis_bins - 1;
    *dataslice = data + other_bins*binnum;
    if (poserr)
	*err_pos = poserr + other_bins*binnum;
    else
	*err_pos = NULL;
    if (negerr)
	*err_neg = negerr + other_bins*binnum;
    else
	*err_neg = NULL;
    return 0;

 fail:
    if (data)
    {
	free(data);
	data = NULL;
    }
    if (poserr)
    {
	free(poserr);
	poserr = NULL;
    }
    if (negerr)
    {
	free(negerr);
	negerr = NULL;
    }
    last_id = -1;
    last_axis = -1;
    return -1;
}


static int projection_fill_weighted(struct weighted_point *arr, int n,
				    int proj_type, float *locat, float *width)
{
    int status;
    float qmin, q25, q75, qmax;

    switch (proj_type)
    {
    case HS_CALC_PROJ_COORDAVE:
	status = arr_stats_weighted(arr, n, locat, width);
	break;

    case HS_CALC_PROJ_COORDRMS:
	status = arr_stats_weighted(arr, n, width, locat);
	*width = 0.f;
	break;

    case HS_CALC_PROJ_COORDMED:
	status = arr_medirange_weighted(arr, n, &qmin, &q25,
					locat, &q75, &qmax);
	*width = (q75 - q25)*RANGE2SIG;
	break;

    case HS_CALC_PROJ_COORDRANGE:
	status = arr_medirange_weighted(arr, n, &qmin, &q25,
					locat, &q75, &qmax);
	*locat = (q75 - q25)*RANGE2SIG;
	*width = 0.f;
	break;

    default:
	assert(0);
    }
    return status;
}


static int projection_fill(float *arr, int n, int proj_type,
			   int suppress_zero, float *locat, float *width)
{
  int i, status;
  float qmin, q25, q75, qmax;
  
  switch (proj_type)
  {
      case HS_CALC_PROJ_AVE:
	status = arr_stats(arr, n, !suppress_zero, locat, width);
	break;

      case HS_CALC_PROJ_RMS:
	status = arr_stats(arr, n, !suppress_zero, width, locat);
	*width = 0.f;
	break;

      case HS_CALC_PROJ_SUM:
	*locat = 0.f;
	for (i=0; i<n; i++)
	  *locat += arr[i];
	*width = 0.f;
	status = 0;
	break;
	
      case HS_CALC_PROJ_MIN:
	*locat = arr[0];
	for (i=1; i<n; i++)
	  if (arr[i] < *locat)
	    *locat = arr[i];
	*width = 0.f;
	status = 0;
	break;

      case HS_CALC_PROJ_MAX:
	*locat = arr[0];
	for (i=1; i<n; i++)
	  if (arr[i] > *locat)
	    *locat = arr[i];
	*width = 0.f;
	status = 0;
	break;

      case HS_CALC_PROJ_MED:
	status = arr_medirange(arr, n, !suppress_zero,
			       &qmin, &q25, locat, &q75, &qmax);
	*width = (q75 - q25)*RANGE2SIG;
	break;

      case HS_CALC_PROJ_RANGE:
	status = arr_medirange(arr, n, !suppress_zero,
			       &qmin, &q25, locat, &q75, &qmax);
	*locat = (q75 - q25)*RANGE2SIG;
	*width = 0.f;
	break;

      default:
	assert(0);
  }
  return status;
}


int hs_next_category_uid(char *category)
{
  int uid, max_items, i, nids, tmpuid;
  int *ids;

  max_items = hs_num_items()+1;
  if ((ids = (int *)malloc(max_items*sizeof(int))) == NULL)
  {
      fprintf(stderr, "next_category_uid: out of memory\n");
      return -1;
  }
  uid = 0;
  if ((nids = hs_list_items("", category, ids, max_items, 1)) > 0)
  {
      for (i=0; i<nids; i++)
      {
	  tmpuid = hs_uid(ids[i]);
	  if (tmpuid > uid)
	      uid = tmpuid;
      }
  }
  free(ids);
  return ++uid;
}


int hs_1d_hist_subrange(int uid, char *title, char *category,
			int id, int bin_min, int bin_max)
{
  int nbins, newid, errstat;
  float xmin, xmax, binwidth, newmin, newmax;
  float *ppdat, *pperr, *pnerr;
  char stringbuf[512];
  
  if (hs_type(id) != HS_1D_HISTOGRAM)
  {
    if (hs_type(id) == HS_NONE)
      fprintf(stderr, 
	      "hs_1d_hist_subrange: item with id %d does not exist\n", id);
    else
      fprintf(stderr, 
	      "hs_1d_hist_subrange: item with id %d is not a 1d histogram\n", id);
    return -1;
  }
  if (bin_min > bin_max)
  {
    fprintf(stderr, 
	    "hs_1d_hist_subrange: min bin number is larger than max bin number\n");
    return -1;
  }
  if (bin_min < 0)
  {
    fprintf(stderr, "hs_1d_hist_subrange: min bin number is out of range\n");
    return -1;
  }
  nbins = hs_1d_hist_num_bins(id);
  if (bin_max >= nbins)
  {
    fprintf(stderr, "hs_1d_hist_subrange: max bin number is out of range\n");
    return -1;
  }
  hs_1d_hist_range(id, &xmin, &xmax);
  binwidth = (xmax - xmin)/(float)nbins;

  newmin = xmin + binwidth*(float)bin_min;
  newmax = xmin + binwidth*(float)(bin_max+1);
  hs_1d_hist_labels(id, stringbuf, stringbuf+256);
  newid = hs_create_1d_hist(uid, title, category, stringbuf, stringbuf+256,
			    bin_max-bin_min+1, newmin, newmax);
  if (newid <= 0)
  {
    fprintf(stderr, "hs_1d_hist_subrange: failed to create a new histogram\n");
    return -1;
  }
  if ((pnerr = (float *)malloc(nbins*sizeof(float))) == NULL)
  {
    fprintf(stderr, "hs_1d_hist_subrange: out of memory\n");
    goto fail00;
  }
  if ((ppdat = (float *)malloc(nbins*sizeof(float))) == NULL)
  {
    fprintf(stderr, "hs_1d_hist_subrange: out of memory\n");
    goto fail0;
  }
  if ((pperr = (float *)malloc(nbins*sizeof(float))) == NULL)
  {
    fprintf(stderr, "hs_1d_hist_subrange: out of memory\n");
    goto fail1;
  }

  hs_1d_hist_bin_contents(id, ppdat);
  errstat = hs_1d_hist_errors(id, pperr, pnerr);
  
  if (errstat == HS_NO_ERRORS)
    hs_1d_hist_block_fill(newid, ppdat+bin_min, NULL, NULL);
  else if (errstat == HS_POS_ERRORS)
    hs_1d_hist_block_fill(newid, ppdat+bin_min, pperr+bin_min, NULL);
  else
    hs_1d_hist_block_fill(newid, ppdat+bin_min, pperr+bin_min, pnerr+bin_min);

  free(pperr);
  free(ppdat);
  free(pnerr);
  return newid;

fail1:
  free(ppdat);
fail0:
  free(pnerr);
fail00:
  hs_delete(newid);
  return -1;
}


int hs_2d_hist_subrange(int uid, char *title, char *category,
			int id, int axis, int bin_min, int bin_max)
{
    float xmin, xmax, ymin, ymax, binwidth, newmin, newmax;
    int i, j, newid, x_bin_num, y_bin_num, nbins, iferrors;
    float *ppdat, *pperr, *pnerr, *ptmp = NULL;
    char stringbuf[768];

    if (hs_type(id) != HS_2D_HISTOGRAM)
    {
	if (hs_type(id) == HS_NONE)
	    fprintf(stderr, 
		    "hs_2d_hist_subrange: item with id %d does not exist\n", id);
	else
	    fprintf(stderr, 
		    "hs_2d_hist_subrange: item with id %d is not a 2d histogram\n", id);
	return -1;
    }
    if (bin_min > bin_max)
    {
	fprintf(stderr, 
		"hs_2d_hist_subrange: min bin number is larger than max bin number\n");
	return -1;
    }
    if (bin_min < 0)
    {
	fprintf(stderr, "hs_2d_hist_subrange: min bin number is out of range\n");
	return -1;
    }
    hs_2d_hist_range(id, &xmin, &xmax, &ymin, &ymax);
    hs_2d_hist_num_bins(id, &x_bin_num, &y_bin_num);
    nbins = x_bin_num*y_bin_num;
    hs_2d_hist_labels(id, stringbuf, stringbuf+256, stringbuf+512);
    if (axis == HS_AXIS_X)
    {
	if (bin_max >= x_bin_num)
	{
	    fprintf(stderr, "hs_2d_hist_subrange: max bin number is out of range\n");
	    return -1;      
	}
	binwidth = (xmax - xmin)/(float)x_bin_num;
	newmin = xmin + binwidth*(float)bin_min;
	newmax = xmin + binwidth*(float)(bin_max+1);
	newid = hs_create_2d_hist(uid, title, category, stringbuf, stringbuf+256, 
				  stringbuf+512, bin_max+1-bin_min, y_bin_num,
				  newmin, newmax, ymin, ymax);
    }
    else
    {
	if (bin_max >= y_bin_num)
	{
	    fprintf(stderr, "hs_2d_hist_subrange: max bin number is out of range\n");
	    return -1;      
	}
	binwidth = (ymax - ymin)/(float)y_bin_num;
	newmin = ymin + binwidth*(float)bin_min;
	newmax = ymin + binwidth*(float)(bin_max+1);
	newid = hs_create_2d_hist(uid, title, category, stringbuf, stringbuf+256, 
				  stringbuf+512, x_bin_num, bin_max+1-bin_min,
				  xmin, xmax, newmin, newmax);
    }
    if (newid <= 0)
    {
	fprintf(stderr, "hs_2d_hist_subrange: failed to create a new histogram\n");
	return -1;
    }
    if ((pnerr = (float *)malloc(3*nbins*sizeof(float))) == NULL)
    {
	fprintf(stderr, "hs_2d_hist_subrange: out of memory\n");
	goto fail00;
    }
    ppdat = pnerr + nbins;
    pperr = ppdat + nbins;

    hs_2d_hist_bin_contents(id, ppdat);
    iferrors = hs_2d_hist_errors(id, pperr, pnerr);

    if (axis == HS_AXIS_X)
    {
	if (iferrors == HS_NO_ERRORS)
	    hs_2d_hist_block_fill(newid, ppdat+bin_min*y_bin_num,
				  NULL, NULL);
	else if (iferrors == HS_POS_ERRORS)
	    hs_2d_hist_block_fill(newid, ppdat+bin_min*y_bin_num, 
				  pperr+bin_min*y_bin_num, NULL);
	else
	    hs_2d_hist_block_fill(newid, ppdat+bin_min*y_bin_num, 
				  pperr+bin_min*y_bin_num, 
				  pnerr+bin_min*y_bin_num);
    }
    else
    {
	/* Need a temporary array to rearrange the data */
	if ((ptmp = (float *)malloc(nbins*sizeof(float))) == NULL)
	{
	    fprintf(stderr, "hs_2d_hist_subrange: out of memory\n");
	    goto fail2;
	}
	nbins = bin_max-bin_min + 1;
	for (i=0; i<x_bin_num; i++)
	    for (j=0; j<nbins; j++)
		ptmp[i*nbins+j] = ppdat[i*y_bin_num+j+bin_min];
	if (iferrors == HS_NO_ERRORS)
	    hs_2d_hist_block_fill(newid, ptmp, NULL, NULL);
	else
	{
	    for (i=0; i<x_bin_num; i++)
		for (j=0; j<nbins; j++)
		    ppdat[i*nbins+j] = pperr[i*y_bin_num+j+bin_min];
	    if (iferrors == HS_POS_ERRORS)
		hs_2d_hist_block_fill(newid, ptmp, ppdat, NULL);
	    else
	    {
		for (i=0; i<x_bin_num; i++)
		    for (j=0; j<nbins; j++)
			pperr[i*nbins+j] = pnerr[i*y_bin_num+j+bin_min];
		hs_2d_hist_block_fill(newid, ptmp, ppdat, pperr);
	    }
	}
	free(ptmp);
    }

    free(pnerr);
    return newid;

 fail2:
    free(pnerr);
 fail00:
    hs_delete(newid);
    return -1;
}


int hs_3d_hist_subrange(int uid, char *title, char *category,
			int id, int axis, int bin_min, int bin_max)
{
    float xmin, xmax, ymin, ymax, zmin, zmax, binwidth, newmin, newmax;
    int i, j, k, newid, x_bin_num, y_bin_num, z_bin_num, nbins, iferrors;
    float *ppdat, *pperr, *pnerr, *ptmp = NULL;
    char stringbuf[1024];

    if (hs_type(id) != HS_3D_HISTOGRAM)
    {
	if (hs_type(id) == HS_NONE)
	    fprintf(stderr, 
		    "hs_3d_hist_subrange: item with id %d does not exist\n", id);
	else
	    fprintf(stderr, 
		    "hs_3d_hist_subrange: item with id %d is not a 3d histogram\n", id);
	return -1;
    }
    if (bin_min > bin_max)
    {
	fprintf(stderr, 
		"hs_3d_hist_subrange: min bin number is larger than max bin number\n");
	return -1;
    }
    if (bin_min < 0)
    {
	fprintf(stderr, "hs_3d_hist_subrange: min bin number is out of range\n");
	return -1;
    }
    hs_3d_hist_range(id, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
    hs_3d_hist_num_bins(id, &x_bin_num, &y_bin_num, &z_bin_num);
    nbins = x_bin_num*y_bin_num*z_bin_num;
    hs_3d_hist_labels(id, stringbuf, stringbuf+256, stringbuf+512, stringbuf+768);
    if (axis == HS_AXIS_X)
    {
	if (bin_max >= x_bin_num)
	{
	    fprintf(stderr, "hs_3d_hist_subrange: max bin number is out of range\n");
	    return -1;      
	}
	binwidth = (xmax - xmin)/(float)x_bin_num;
	newmin = xmin + binwidth*(float)bin_min;
	newmax = xmin + binwidth*(float)(bin_max+1);
	newid = hs_create_3d_hist(uid, title, category, stringbuf, stringbuf+256, 
				  stringbuf+512, stringbuf+768,
				  bin_max+1-bin_min, y_bin_num, z_bin_num,
				  newmin, newmax, ymin, ymax, zmin, zmax);
    }
    else if (axis == HS_AXIS_Y)
    {
	if (bin_max >= y_bin_num)
	{
	    fprintf(stderr, "hs_3d_hist_subrange: max bin number is out of range\n");
	    return -1;      
	}
	binwidth = (ymax - ymin)/(float)y_bin_num;
	newmin = ymin + binwidth*(float)bin_min;
	newmax = ymin + binwidth*(float)(bin_max+1);
	newid = hs_create_3d_hist(uid, title, category, stringbuf, stringbuf+256, 
				  stringbuf+512, stringbuf+768,
				  x_bin_num, bin_max+1-bin_min, z_bin_num,
				  xmin, xmax, newmin, newmax, zmin, zmax);
    }
    else if (axis == HS_AXIS_Z)
    {
	if (bin_max >= z_bin_num)
	{
	    fprintf(stderr, "hs_3d_hist_subrange: max bin number is out of range\n");
	    return -1;      
	}
	binwidth = (zmax - zmin)/(float)z_bin_num;
	newmin = zmin + binwidth*(float)bin_min;
	newmax = zmin + binwidth*(float)(bin_max+1);
	newid = hs_create_3d_hist(uid, title, category, stringbuf, stringbuf+256, 
				  stringbuf+512, stringbuf+768,
				  x_bin_num, y_bin_num, bin_max+1-bin_min,
				  xmin, xmax, ymin, ymax, newmin, newmax);
    }
    else
	assert(0);
    if (newid <= 0)
    {
	fprintf(stderr, "hs_3d_hist_subrange: failed to create a new histogram\n");
	return -1;
    }
    if ((pnerr = (float *)malloc(3*nbins*sizeof(float))) == NULL)
    {
	fprintf(stderr, "hs_3d_hist_subrange: out of memory\n");
	goto fail00;
    }
    ppdat = pnerr + nbins;
    pperr = ppdat + nbins;

    hs_3d_hist_bin_contents(id, ppdat);
    iferrors = hs_3d_hist_errors(id, pperr, pnerr);

    if (axis == HS_AXIS_X)
    {
	if (iferrors == HS_NO_ERRORS)
	    hs_3d_hist_block_fill(newid, ppdat+bin_min*y_bin_num*z_bin_num,
				  NULL, NULL);
	else if (iferrors == HS_POS_ERRORS)
	    hs_3d_hist_block_fill(newid, ppdat+bin_min*y_bin_num*z_bin_num, 
				  pperr+bin_min*y_bin_num*z_bin_num, NULL);
	else
	    hs_3d_hist_block_fill(newid, ppdat+bin_min*y_bin_num*z_bin_num, 
				  pperr+bin_min*y_bin_num*z_bin_num, 
				  pnerr+bin_min*y_bin_num*z_bin_num);
    }
    else
    {
	/* Need a temporary array to rearrange the data */
	if ((ptmp = (float *)malloc(nbins*sizeof(float))) == NULL)
	{
	    fprintf(stderr, "hs_3d_hist_subrange: out of memory\n");
	    goto fail2;
	}
	nbins = bin_max-bin_min + 1;
	if (axis == HS_AXIS_Y)
	{
	    for (i=0; i<x_bin_num; ++i)
		for (j=0; j<nbins; ++j)
		    for (k=0; k<z_bin_num; ++k)
			ptmp[(i*nbins+j)*z_bin_num+k] = 
			    ppdat[(i*y_bin_num+j+bin_min)*z_bin_num+k];
	    if (iferrors == HS_NO_ERRORS)
		hs_3d_hist_block_fill(newid, ptmp, NULL, NULL);
	    else
	    {
		for (i=0; i<x_bin_num; ++i)
		    for (j=0; j<nbins; ++j)
			for (k=0; k<z_bin_num; ++k)
			    ppdat[(i*nbins+j)*z_bin_num+k] = 
				pperr[(i*y_bin_num+j+bin_min)*z_bin_num+k];
		if (iferrors == HS_POS_ERRORS)
		    hs_3d_hist_block_fill(newid, ptmp, ppdat, NULL);
		else
		{
		    for (i=0; i<x_bin_num; ++i)
			for (j=0; j<nbins; ++j)
			    for (k=0; k<z_bin_num; ++k)
				pperr[(i*nbins+j)*z_bin_num+k] = 
				    pnerr[(i*y_bin_num+j+bin_min)*z_bin_num+k];
		    hs_3d_hist_block_fill(newid, ptmp, ppdat, pperr);
		}
	    }
	} 
	else if (axis == HS_AXIS_Z)
	{
	    for (i=0; i<x_bin_num; ++i)
		for (j=0; j<y_bin_num; ++j)
		    for (k=0; k<nbins; ++k)
			ptmp[(i*y_bin_num+j)*nbins+k] = 
			    ppdat[(i*y_bin_num+j)*z_bin_num+k+bin_min];
	    if (iferrors == HS_NO_ERRORS)
		hs_3d_hist_block_fill(newid, ptmp, NULL, NULL);
	    else
	    {
		for (i=0; i<x_bin_num; ++i)
		    for (j=0; j<y_bin_num; ++j)
			for (k=0; k<nbins; ++k)
			    ppdat[(i*y_bin_num+j)*nbins+k] = 
				pperr[(i*y_bin_num+j)*z_bin_num+k+bin_min];
		if (iferrors == HS_POS_ERRORS)
		    hs_3d_hist_block_fill(newid, ptmp, ppdat, NULL);
		else
		{
		    for (i=0; i<x_bin_num; ++i)
			for (j=0; j<y_bin_num; ++j)
			    for (k=0; k<nbins; ++k)
				pperr[(i*y_bin_num+j)*nbins+k] = 
				    pnerr[(i*y_bin_num+j)*z_bin_num+k+bin_min];
		    hs_3d_hist_block_fill(newid, ptmp, ppdat, pperr);
		}
	    }
	}
	else
	    assert(0);
	free(ptmp);
    }

    free(pnerr);
    return newid;

 fail2:
    free(pnerr);
 fail00:
    hs_delete(newid);
    return -1;
}


tcl_routine(next_category_uid)
{
    int uid;
    
    tcl_require_objc(2);
    uid = hs_next_category_uid(Tcl_GetStringFromObj(objv[1],NULL));
    if (uid < 0)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	return TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(uid));
    return TCL_OK;
}


tcl_routine(transpose_ntuple)
{
    int i, namelen, id, uid, ncolumns, nrows;
    int new_id = -1, status = TCL_ERROR;
    char *title, *category, *namedata = NULL;
    char buf[32];
    char **names = NULL;
    float *contents = NULL;

    tcl_require_objc(5);
    if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
	return TCL_ERROR;
    title = Tcl_GetStringFromObj(objv[2], NULL);
    category = Tcl_GetStringFromObj(objv[3], NULL);
    verify_ntuple(id,4);

    nrows = hs_num_entries(id);
    if (nrows == 0)
    {
        Tcl_SetResult(interp, "empty ntuple", TCL_STATIC);
        return TCL_ERROR;
    }
    ncolumns = hs_num_variables(id);

    /* Allocate the arrays to hold names and data */
    names = (char **)malloc(nrows*sizeof(char *));
    sprintf(buf, "r%d", nrows-1);
    namelen = strlen(buf)+1;
    namedata = (char *)malloc(namelen*nrows*sizeof(char));
    contents = (float *)malloc(nrows*sizeof(float));
    if (names == NULL || namedata == NULL || contents == NULL)
    {
        Tcl_SetResult(interp, "out of memory", TCL_STATIC);
        goto fail;
    }

    /* Create the new ntuple */
    for (i=0; i<nrows; ++i)
    {
        names[i] = namedata+namelen*i;
        sprintf(names[i], "r%d", i);
    }
    new_id = hs_create_ntuple(uid, title, category, nrows, names);
    if (new_id <= 0)
    {
        Tcl_AppendResult(interp, "failed to create ntuple with uid ",
                         Tcl_GetStringFromObj(objv[1],NULL),
                         " and category \"", category, "\"", NULL);
        goto fail;
    }
    for (i=0; i<ncolumns; ++i)
    {
        hs_column_contents(id, i, contents);
        if (hs_fill_ntuple(new_id, contents) != new_id)
        {
            Tcl_SetResult(interp, "out of memory", TCL_STATIC);
            goto fail;
        }
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(new_id));

    status = TCL_OK;
 fail:
    if (new_id > 0 && status != TCL_OK)
        hs_delete(new_id);
    if (contents)
        free(contents);
    if (namedata)
        free(namedata);
    if (names)
        free(names);
    return status;
}


tcl_routine(transpose_histogram)
{
    int id, uid, new_id;

    tcl_require_objc(5);
    if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[4], &id) != TCL_OK)
	return TCL_ERROR;
    new_id = hs_transpose_histogram(uid, Tcl_GetStringFromObj(objv[2], NULL), 
				    Tcl_GetStringFromObj(objv[3], NULL), id);
    if (new_id <= 0)
    {
	Tcl_SetResult(interp, "operation failed", TCL_VOLATILE);
	return TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(new_id));
    return TCL_OK;
}


tcl_routine(concat_histograms)
{
    int id, id2, uid, new_id;

    tcl_require_objc(6);
    if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[4], &id) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[5], &id2) != TCL_OK)
	return TCL_ERROR;
    new_id = hs_concat_histograms(uid, Tcl_GetStringFromObj(objv[2], NULL), 
				  Tcl_GetStringFromObj(objv[3], NULL), id, id2);
    if (new_id <= 0)
    {
	Tcl_SetResult(interp, "operation failed", TCL_VOLATILE);
	return TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(new_id));
    return TCL_OK;
}


tcl_routine(1d_hist_subrange)
{
    int id, bin_min, bin_max, uid, new_id;

    tcl_require_objc(7);
    if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
	return TCL_ERROR;
    verify_1d_histo(id, 4);
    if (Tcl_GetIntFromObj(interp, objv[5], &bin_min) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[6], &bin_max) != TCL_OK)
	return TCL_ERROR;
    new_id = hs_1d_hist_subrange(uid, Tcl_GetStringFromObj(objv[2], NULL), 
				 Tcl_GetStringFromObj(objv[3], NULL), id, 
				 bin_min, bin_max);
    if (new_id <= 0)
    {
	Tcl_SetResult(interp, "operation failed", TCL_VOLATILE);
	return TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(new_id));
    return TCL_OK;
}


tcl_routine(2d_hist_subrange)
{
    int id, axis, bin_min, bin_max, uid, new_id;

    tcl_require_objc(8);
    if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
	return TCL_ERROR;
    verify_2d_histo(id, 4);
    if (get_axis_from_obj(interp, objv[5], 2, 1, &axis) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[6], &bin_min) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[7], &bin_max) != TCL_OK)
	return TCL_ERROR;
    new_id = hs_2d_hist_subrange(uid, Tcl_GetStringFromObj(objv[2], NULL), 
				 Tcl_GetStringFromObj(objv[3], NULL), id, 
				 axis, bin_min, bin_max);
    if (new_id <= 0)
    {
	Tcl_SetResult(interp, "operation failed", TCL_VOLATILE);
	return TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(new_id));
    return TCL_OK;
}


tcl_routine(3d_hist_subrange)
{
    int id, axis, bin_min, bin_max, uid, new_id;

    tcl_require_objc(8);
    if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
	return TCL_ERROR;
    verify_3d_histo(id, 4);
    if (get_axis_from_obj(interp, objv[5], 3, 1, &axis) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[6], &bin_min) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[7], &bin_max) != TCL_OK)
	return TCL_ERROR;
    new_id = hs_3d_hist_subrange(uid, Tcl_GetStringFromObj(objv[2], NULL), 
				 Tcl_GetStringFromObj(objv[3], NULL), id, 
				 axis, bin_min, bin_max);
    if (new_id <= 0)
    {
	Tcl_SetResult(interp, "operation failed", TCL_VOLATILE);
	return TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(new_id));
    return TCL_OK;
}


tcl_routine(slice_histogram)
{
    int id, uid, axis, binnum, new_id, axis2 = HS_AXIS_NONE, binnum2 = -1;
    int parent_dim, parent_type;

    tcl_objc_range(7,9);
    if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[4], &id) != TCL_OK)
	return TCL_ERROR;
    parent_type = hs_type(id);
    if ((parent_dim = histo_dim_from_type(parent_type)) < 2)
    {
	if (parent_type == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[4], NULL),
			     " is not a valid Histo-Scope id", NULL);
        else if (parent_type == HS_1D_HISTOGRAM)
            Tcl_AppendResult(interp, "item with id ",
			    Tcl_GetStringFromObj(objv[4], NULL),
			    " can not be sliced: it is a 1d histogram", NULL);
	else
	    Tcl_AppendResult(interp, "item with id ",
			     Tcl_GetStringFromObj(objv[4], NULL),
			     " is not a histogram", NULL);
	return TCL_ERROR;
    }
    if (get_axis_from_obj(interp, objv[5], parent_dim, 1, &axis) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[6], &binnum) != TCL_OK)
	return TCL_ERROR;
    if (objc > 7)
    {
	tcl_require_objc(9);
	if (get_axis_from_obj(interp, objv[7],
			      parent_dim, 0, &axis2) != TCL_OK)
	    return TCL_ERROR;
	if (Tcl_GetIntFromObj(interp, objv[8], &binnum2) != TCL_OK)
	    return TCL_ERROR;
    }
    if (axis == axis2)
    {
	Tcl_SetResult(interp, 
		      "can not specify the same slice axis more than once",
		      TCL_STATIC);
	return TCL_ERROR;
    }
    new_id = slice_histogram(uid, Tcl_GetStringFromObj(objv[2], NULL),
			     Tcl_GetStringFromObj(objv[3], NULL),
			     id, axis, binnum, axis2, binnum2);
    if (new_id <= 0) {
	if (new_id == -1) {
	    Tcl_SetResult(interp, "bin number out of range", TCL_STATIC);
	} else {
	    char buf[32];
	    sprintf(buf, "%d", new_id);
	    Tcl_AppendResult(interp, "operation failed (status ", buf, ")", NULL);
	}
	return TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(new_id));
    return TCL_OK;
}


tcl_routine(slice_2d_histogram)
{
    int id, uid, axis, binnum, new_id;

    tcl_require_objc(7);
    if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[4], &id) != TCL_OK)
	return TCL_ERROR;
    if (get_axis_from_obj(interp, objv[5], 2, 1, &axis) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[6], &binnum) != TCL_OK)
	return TCL_ERROR;
    new_id = slice_2d_histogram(uid, Tcl_GetStringFromObj(objv[2], NULL), 
			     Tcl_GetStringFromObj(objv[3], NULL),
			     id, axis, binnum);
    if (new_id <= 0)
    {
	Tcl_SetResult(interp, "operation failed", TCL_VOLATILE);
	return TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(new_id));
    return TCL_OK;
}


tcl_routine(project_3d_histogram)
{
    /* Usage: hs::project_3d_histogram $uid $title $category $id \
     *               $axis1 $axis2 $proj_type $suppress_zero
     */
    int id, uid, axis1, axis2, proj_type, new_id, use_coords, suppress_zero;
    const char *proj_str;

    tcl_require_objc(9);
    if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
	return TCL_ERROR;
    verify_3d_histo(id, 4);
    if (Tcl_GetBooleanFromObj(interp, objv[8], &suppress_zero) != TCL_OK)
	return TCL_ERROR;
    proj_str = Tcl_GetStringFromObj(objv[7], NULL);
    proj_type = get_projection_type(proj_str);
    if (proj_type < 0)
    {
	Tcl_AppendResult(interp, "invalid projection type \"",
			 proj_type, "\"", NULL);
	return TCL_ERROR;
    }
    if (get_axis_from_obj(interp, objv[5], 3, 0, &axis1) != TCL_OK)
	return TCL_ERROR;
    if (get_axis_from_obj(interp, objv[6], 3, 0, &axis2) != TCL_OK)
	return TCL_ERROR;
    if (axis1 == axis2)
    {
	Tcl_SetResult(interp, "projection axes must be distinct",
		      TCL_STATIC);
	return TCL_ERROR;
    }

    /* Check that we are not trying to project
       coordinate stats onto a 1d histogram */
    use_coords = (proj_type == HS_CALC_PROJ_COORDAVE ||
		  proj_type == HS_CALC_PROJ_COORDRMS ||
		  proj_type == HS_CALC_PROJ_COORDMED ||
		  proj_type == HS_CALC_PROJ_COORDRANGE);
    if (use_coords && axis2 == HS_AXIS_NONE)
    {
	Tcl_AppendResult(interp, "invalid projection type \"", proj_str,
			 "\" for a 1d projection of a 3d histogram", NULL);
	return TCL_ERROR;
    }

    new_id = project_3d_histogram(uid, Tcl_GetStringFromObj(objv[2], NULL), 
				  Tcl_GetStringFromObj(objv[3], NULL), id, 
				  axis1, axis2, proj_type, suppress_zero);
    if (new_id <= 0)
    {
	Tcl_SetResult(interp, "operation failed", TCL_VOLATILE);
	return TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(new_id));
    return TCL_OK;
}


tcl_routine(project_2d_histogram)
{
    int id, uid, axis, proj_type, new_id, suppress_zero;
    const char *argv6;

    tcl_require_objc(8);
    if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
	return TCL_ERROR;
    verify_2d_histo(id, 4);
    if (Tcl_GetBooleanFromObj(interp, objv[7], &suppress_zero) != TCL_OK)
	return TCL_ERROR;
    argv6 = Tcl_GetStringFromObj(objv[6], NULL);
    proj_type = get_projection_type(argv6);
    if (proj_type < 0)
    {
	Tcl_AppendResult(interp, "invalid projection type \"",
			 argv6, "\"", NULL);
	return TCL_ERROR;
    }
    if (get_axis_from_obj(interp, objv[5], 2, 1, &axis) != TCL_OK)
	return TCL_ERROR;
    new_id = project_2d_histogram(uid, Tcl_GetStringFromObj(objv[2], NULL), 
				  Tcl_GetStringFromObj(objv[3], NULL), id, 
				  axis, proj_type, suppress_zero);
    if (new_id <= 0)
    {
	Tcl_SetResult(interp, "operation failed", TCL_VOLATILE);
	return TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(new_id));
    return TCL_OK;
}


tcl_routine(fill_slice)
{
    static int last_parent = -1, last_slice = -1;
    static int nbins, imax, last_binnum = -1, last_axis = -1;
    static float lolim, hilim;

    int x_bin_num, y_bin_num, binnum;
    int parent_id, type, slice_id, axis;
    double coordval;
    float xmin, xmax, ymin, ymax;
    float *data, *poserr, *negerr;

    /* Tcl usage: hs::fill_slice slice_id parent_id axis coord_value */
    if (objc < 2)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_VOLATILE);
	return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, objv[1], &slice_id) != TCL_OK)
	return TCL_ERROR;
    if (slice_id == 0)
    {
	/* Reset the internal cache of the 'get_data_slice' function */
	get_data_slice(0, 0, 0, NULL, NULL, NULL);
	return TCL_OK;
    }
    tcl_require_objc(5);
    if (Tcl_GetIntFromObj(interp, objv[2], &parent_id) != TCL_OK)
	return TCL_ERROR;
    if ((type = hs_type(parent_id)) != HS_2D_HISTOGRAM)
    {
	if (type == HS_NONE)
	    Tcl_AppendResult(interp, "histoscope item with id ", 
			     Tcl_GetStringFromObj(objv[2], NULL), 
			     " not found", NULL);
	else
	    Tcl_AppendResult(interp, "histoscope item with id ", 
			     Tcl_GetStringFromObj(objv[2], NULL),
			     " is not a 2d histogram", NULL);
	return TCL_ERROR;
    }
    if ((type = hs_type(slice_id)) != HS_1D_HISTOGRAM)
    {
	if (type == HS_NONE)
	    Tcl_AppendResult(interp, "histoscope item with id ", 
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " not found", NULL);
	else
	    Tcl_AppendResult(interp, "histoscope item with id ",
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a 1d histogram", NULL);
	return TCL_ERROR;
    }
    if (get_axis_from_obj(interp, objv[3], 2, 1, &axis) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[4], &coordval) != TCL_OK)
	return TCL_ERROR;

    if (parent_id != last_parent || slice_id != last_slice || axis != last_axis)
    {    
	hs_2d_hist_num_bins(parent_id, &x_bin_num, &y_bin_num);
	hs_2d_hist_range(parent_id, &xmin, &xmax, &ymin, &ymax);
	if (axis == HS_AXIS_X)
	{
	    nbins = x_bin_num;
	    imax = y_bin_num;
	    lolim = xmin;
	    hilim = xmax;
	}
	else
	{
	    nbins = y_bin_num;
	    imax = x_bin_num;
	    lolim = ymin;
	    hilim = ymax;
	}
	if (hs_1d_hist_num_bins(slice_id) != imax)
	{
	    Tcl_SetResult(interp, "items are not bin compatible",
			  TCL_VOLATILE);
	    parent_id = -1;
	    return TCL_ERROR;
	}
	last_axis = axis;
	last_parent = parent_id;
	last_slice = slice_id;
	last_binnum = -1;
    }

    binnum = (int)((((float)coordval-lolim)*(float)nbins)/(hilim-lolim) + 0.01);
    /* The +0.01 part in the above formula makes sure that we get the correct
       bin number when we are exactly on the left bin edge. Of course, it will
       screw up the right bin edge. */
    if (binnum < 0)
	binnum = 0;
    else if (binnum >= nbins)
	binnum = nbins-1;
    if (binnum == last_binnum)
    {
	/* The following is too optimistic. The histogram may be
	   reset/filled by other commands. Commented out for now. */
        /*  	return TCL_OK; */
    }
    else
	last_binnum = binnum;
    if (get_data_slice(parent_id, axis, binnum, &data, &poserr, &negerr))
	return TCL_ERROR;
    hs_1d_hist_block_fill(slice_id, data, poserr, negerr);
    return TCL_OK;
}


tcl_routine(Basic_ntuple_count)
{
    /* Usage: hs::Basic_ntuple_count id filter
     *
     * Counts the number of entries in the ntuple passing a simple cut.
     * This function returns -1 if it can not parse the cut expression.
     */
    int i, ntuple_id, nrows, counter = 0;
    simple_filter filter;
    double ntvalue, cutvalue;

    tcl_require_objc(3);
    verify_ntuple(ntuple_id,1);
    if (get_simple_filter(interp, objv[2], ntuple_id, &filter) != TCL_OK)
    {
        Tcl_SetObjResult(interp, Tcl_NewIntObj(-1));
        return TCL_OK;
    }
    nrows = hs_num_entries(ntuple_id);
    for (i=0; i<nrows; ++i)
    {
        if (filter.left.is_column)
            ntvalue = hs_ntuple_value(ntuple_id, i, filter.left.index);
        else
            ntvalue = filter.left.value;
        if (filter.right.is_column)
            cutvalue = hs_ntuple_value(ntuple_id, i, filter.right.index);
        else
            cutvalue = filter.right.value;
        switch (filter.op_index)
        {
        case CUT_OP_LT:
            counter += (ntvalue < cutvalue);
            break;
        case CUT_OP_GT:
            counter += (ntvalue > cutvalue);
            break;
        case CUT_OP_LE:
            counter += (ntvalue <= cutvalue);
            break;
        case CUT_OP_GE:
            counter += (ntvalue >= cutvalue);
            break;
        case CUT_OP_EQ:
            counter += (ntvalue == cutvalue);
            break;
        case CUT_OP_NE:
            counter += (ntvalue != cutvalue);
            break;
        default:
            assert(0);
        }
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(counter));
    return TCL_OK;
}

#define column_or_real_value(varname, col_or_real) do {\
    if ( col_or_real .is_column)\
        varname = row_data[ col_or_real .index];\
    else\
        varname = col_or_real .value;\
} while(0);

tcl_routine(Basic_ntuple_project_onhisto)
{
    /* Usage: hs::Basic_ntuple_project_onhisto nt_id hs_id \
     *         filter_expr weight_expr x_expr y_expr z_expr
     *
     * Note that we do not expect to parse everything successfully.
     * Therefore, on parsing errors this function simply returns
     * "false". "true" is returned on success. The user MUST check
     * the return value.
     */
    int row, ntuple_id, hs_id, ndim, listlen, ncolumns, nrows, pass;
    simple_filter filter;
    column_or_real weight_expr, x_expr, y_expr, z_expr;
    float *row_data = NULL;
    double fl, fr;
    float w, x, y = 0.f, z = 0.f;

    tcl_require_objc(8);
    verify_ntuple(ntuple_id,1);
    if (Tcl_GetIntFromObj(interp, objv[2], &hs_id) != TCL_OK)
        return TCL_ERROR;
    ndim = histo_dim_from_type(hs_type(hs_id));
    if (ndim == 0)
    {
        if (hs_type(hs_id) == HS_NONE)
            Tcl_AppendResult(interp, "Item with id ",
                             Tcl_GetStringFromObj(objv[2], NULL),
                             " does not exist", NULL);
        else
            Tcl_AppendResult(interp, "Item with id ",
                             Tcl_GetStringFromObj(objv[2], NULL),
                             " is not a histogram", NULL);
        return TCL_ERROR;
    }
    assert(ndim <= 3);
    if (get_simple_filter(interp, objv[3], ntuple_id, &filter) != TCL_OK)
        goto fail_parse;
    if (get_column_or_real(interp, objv[4], ntuple_id, 1, &weight_expr) != TCL_OK)
        goto fail_parse;
    if (get_column_or_real(interp, objv[5], ntuple_id, 1, &x_expr) != TCL_OK)
        goto fail_parse;
    if (ndim > 1)
    {
        if (get_column_or_real(interp, objv[6], ntuple_id, 1, &y_expr) != TCL_OK)
            goto fail_parse;
    }
    else
    {
        if (Tcl_ListObjLength(interp, objv[6], &listlen) != TCL_OK)
            return TCL_ERROR;
        if (listlen)
        {
            Tcl_AppendResult(interp, "expected empty list for y axis ",
                             "projection expression, got \"",
                             Tcl_GetStringFromObj(objv[6],NULL), "\"", NULL);
            return TCL_ERROR;
        }
    }
    if (ndim > 2)
    {
        if (get_column_or_real(interp, objv[7], ntuple_id, 1, &z_expr) != TCL_OK)
            goto fail_parse;
    }
    else
    {
        if (Tcl_ListObjLength(interp, objv[7], &listlen) != TCL_OK)
            return TCL_ERROR;
        if (listlen)
        {
            Tcl_AppendResult(interp, "expected empty list for z axis ",
                             "projection expression, got \"",
                             Tcl_GetStringFromObj(objv[7],NULL), "\"", NULL);
            return TCL_ERROR;
        }
    }

    /* Temporary data array */
    ncolumns = hs_num_variables(ntuple_id);
    row_data = (float *)malloc(ncolumns*sizeof(float));
    if (row_data == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	return TCL_ERROR;
    }
    nrows = hs_num_entries(ntuple_id);
    for (row = 0; row < nrows; ++row)
    {
        hs_row_contents(ntuple_id, row, row_data);
        column_or_real_value(fl, filter.left);
        column_or_real_value(fr, filter.right);
        switch (filter.op_index)
        {
        case CUT_OP_LT:
            pass = (fl < fr);
            break;
        case CUT_OP_GT:
            pass = (fl > fr);
            break;
        case CUT_OP_LE:
            pass = (fl <= fr);
            break;
        case CUT_OP_GE:
            pass = (fl >= fr);
            break;
        case CUT_OP_EQ:
            pass = (fl == fr);
            break;
        case CUT_OP_NE:
            pass = (fl != fr);
            break;
        default:
            assert(0);
        }
        if (pass)
        {
            column_or_real_value(w, weight_expr);
            column_or_real_value(x, x_expr);
            if (ndim > 1)
                column_or_real_value(y, y_expr);
            if (ndim > 2)
                column_or_real_value(z, z_expr);
            switch (ndim)
            {
            case 1:
                hs_fill_1d_hist(hs_id, x, w);
                break;
            case 2:
                hs_fill_2d_hist(hs_id, x, y, w);
                break;
            case 3:
                hs_fill_3d_hist(hs_id, x, y, z, w);
                break;
            default:
                assert(0);
            }
        }
    }

    free(row_data);
    Tcl_SetObjResult(interp, Tcl_NewBooleanObj(1));
    return TCL_OK;

 fail_parse:
    Tcl_SetObjResult(interp, Tcl_NewBooleanObj(0));
    if (row_data) free(row_data);
    return TCL_OK;
}


static int get_projection_type(const char *argv6)
{
    int proj_type = -1;

    if (strcmp(argv6, "mean") == 0 || 
	strncmp(argv6, "ave", 3) == 0)
	proj_type = HS_CALC_PROJ_AVE;
    else if (strcmp(argv6, "distmean") == 0 || 
	     strncmp(argv6, "distave", 7) == 0)
	proj_type = HS_CALC_PROJ_COORDAVE;
    else if (strcmp(argv6, "sum") == 0)
	proj_type = HS_CALC_PROJ_SUM;
    else if (strncmp(argv6, "med", 3) == 0)
	proj_type = HS_CALC_PROJ_MED;
    else if (strncmp(argv6, "distmed", 7) == 0)
	proj_type = HS_CALC_PROJ_COORDMED;
    else if (strncmp(argv6, "rang", 4) == 0)
	proj_type = HS_CALC_PROJ_RANGE;
    else if (strncmp(argv6, "distrang", 8) == 0)
	proj_type = HS_CALC_PROJ_COORDRANGE;
    else if (strcmp(argv6, "rms") == 0 || 
	     strcmp(argv6, "noise") == 0 ||
	     strcmp(argv6, "stdev") == 0)
	proj_type = HS_CALC_PROJ_RMS;
    else if (strcmp(argv6, "distrms") == 0 || 
	     strcmp(argv6, "distnoise") == 0 ||
	     strcmp(argv6, "diststdev") == 0)
	proj_type = HS_CALC_PROJ_COORDRMS;
    else if (strncmp(argv6, "min", 3) == 0)
	proj_type = HS_CALC_PROJ_MIN;
    else if (strncmp(argv6, "max", 3) == 0)
	proj_type = HS_CALC_PROJ_MAX;

    return proj_type;
}

static unsigned get_op_index(const char *opname)
{
    unsigned i;
    for (i=0; i<N_CUT_OPERATIONS; ++i)
        if (strcmp(simple_cut_ops[i], opname) == 0)
            return i;
    return N_CUT_OPERATIONS+1;
}

static int get_column_or_real(Tcl_Interp *interp, Tcl_Obj *obj,
                              int ntuple_id, int require_float_range,
                              column_or_real *v)
{
    double dtmp;
    int index, listlen, len, status;
    char *expr;
    char buf[32];
    Tcl_Obj **listObjElem;

    assert(hs_type(ntuple_id) == HS_NTUPLE);
    memset(v, 0, sizeof(column_or_real));
    if (Tcl_GetDoubleFromObj(NULL, obj, &dtmp) == TCL_OK)
    {
        if (require_float_range && 
            (dtmp > FLT_MAX || dtmp < -FLT_MAX))
        {
            Tcl_SetResult(interp, "floating point value too large "
                          "to represent as C float", TCL_STATIC);
            return TCL_ERROR;
        }
        v->value = dtmp;
        return TCL_OK;
    }
    /* Extract list element. This will trim the string on both sides. */
    if (Tcl_ListObjGetElements(interp, obj, &listlen, &listObjElem) != TCL_OK)
        return TCL_ERROR;
    if (listlen == 1)
    {
        expr = Tcl_GetStringFromObj(listObjElem[0], NULL);
        /* Check if we can parse this string as float in case
           the last letter of this string is "f", like in 1.f or 1.0f */
        if (strchr(expr, '.'))
        {
            len = strlen(expr);
            if (expr[len-1] == 'f')
            {
                expr[len-1] = '0';
                status = Tcl_GetDouble(NULL, expr, &dtmp);
                expr[len-1] = 'f';
                if (status == TCL_OK)
                {
                    if (require_float_range && 
                        (dtmp > FLT_MAX || dtmp < -FLT_MAX))
                    {
                        Tcl_SetResult(interp, "floating point value too large "
                                      "to represent as C float", TCL_STATIC);
                        return TCL_ERROR;
                    }
                    v->value = dtmp;
                    return TCL_OK;
                }
            }
        }
        index = hs_variable_index(ntuple_id, expr);
        if (index >= 0)
        {
            v->index = index;
            v->is_column = 1;
            return TCL_OK;
        }
    }
    sprintf(buf, "%d", ntuple_id);
    Tcl_AppendResult(interp, "expected either float or column ",
                     "name of ntuple with id ", buf, " got \"",
                     Tcl_GetStringFromObj(obj, NULL), "\"", NULL);
    return TCL_ERROR;
}

static int get_simple_filter(Tcl_Interp *interp, Tcl_Obj *obj,
                             int ntuple_id, simple_filter *filter)
{
    int listlen;
    Tcl_Obj **listObjElem;
    char *opname;

    memset(filter, 0, sizeof(simple_filter));
    if (Tcl_ListObjGetElements(interp, obj, &listlen, &listObjElem) != TCL_OK)
        return TCL_ERROR;
    if (listlen == 1)
    {
        if (get_column_or_real(interp, obj, ntuple_id, 0, &filter->left) != TCL_OK)
            return TCL_ERROR;
        filter->op_index = CUT_OP_NE;
    }
    else if (listlen == 3)
    {
        if (get_column_or_real(interp, listObjElem[0],
                               ntuple_id, 0, &filter->left) != TCL_OK)
            return TCL_ERROR;
        opname = Tcl_GetStringFromObj(listObjElem[1], NULL);
        filter->op_index = get_op_index(opname);
        if (filter->op_index >= N_CUT_OPERATIONS)
        {
            Tcl_AppendResult(interp, "invalid operation \"",
                             opname, "\"", NULL);
            return TCL_ERROR;
        }
        if (get_column_or_real(interp, listObjElem[2],
                               ntuple_id, 0, &filter->right) != TCL_OK)
            return TCL_ERROR;
    }
    else
    {
        Tcl_SetResult(interp,
                      "wrong number of list elements in the filter",
                      TCL_STATIC);
        return TCL_ERROR;
    }
    return TCL_OK;
}
