#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include "histoscope.h"
#include "histo_tcl_api.h"
#include "histo_utils.h"

tcl_routine(special_percentiles)
{
    /* Usage: hs::special_percentiles $id_source $suppress_zero $percentages */
    int i, source_type, nbins, id_source, suppress_zero, listlen;
    int status = TCL_ERROR;
    float *sourcedata = NULL, *percentages = NULL, *percentiles;
    double dtmp;
    Tcl_Obj **listObjElem;
    Tcl_Obj *result;

    tcl_require_objc(4);    
    if (Tcl_GetIntFromObj(interp, objv[1], &id_source) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[2], &suppress_zero) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_ListObjGetElements(interp, objv[3], &listlen, &listObjElem) != TCL_OK)
	return TCL_ERROR;
    if ((percentages = (float *)malloc(2*listlen*sizeof(float))) == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	goto fail;
    }
    percentiles = percentages + listlen;
    for (i=0; i < listlen; i++)
    {
	if (Tcl_GetDoubleFromObj(interp, listObjElem[i], &dtmp) != TCL_OK)
	    goto fail;
	else
	    percentages[i] = (float)(dtmp);
    }

    source_type = hs_type(id_source);
    if (source_type != HS_1D_HISTOGRAM && 
	source_type != HS_2D_HISTOGRAM &&
	source_type != HS_3D_HISTOGRAM)
    {
	if (source_type == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a valid histoscope id", NULL);
	else
	    Tcl_AppendResult(interp, "item with id ", 
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a histogram", NULL);
	goto fail;
    }

    nbins = hs_hist_num_bins(id_source);
    if ((sourcedata = (float *)malloc(nbins*sizeof(float))) == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	goto fail;
    }
    if (source_type == HS_1D_HISTOGRAM)
	hs_1d_hist_bin_contents(id_source, sourcedata);
    else if (source_type == HS_2D_HISTOGRAM)
	hs_2d_hist_bin_contents(id_source, sourcedata);
    else if (source_type == HS_3D_HISTOGRAM)
	hs_3d_hist_bin_contents(id_source, sourcedata);
    else
	assert(0);

    if (arr_percentiles(sourcedata, nbins, !suppress_zero, 
			percentages, listlen, percentiles))
    {
	Tcl_SetResult(interp, "operation failed", TCL_STATIC);
	goto fail;
    }

    result = Tcl_NewListObj(0, NULL);
    for (i=0; i<listlen; i++)
	Tcl_ListObjAppendElement(
	    interp, result, Tcl_NewDoubleObj((double)percentiles[i]));    
    Tcl_SetObjResult(interp, result);

    status = TCL_OK;
 fail:
    if (sourcedata)
	free(sourcedata);
    if (percentages)
	free(percentages);
    return status;
}


tcl_routine(special_stats)
{
    int i, id_source, suppress_zero;
    int source_type, nbins;
    float *sourcedata;
    float sum, mean, sigma, arrmin, q25, median, q75, arrmax;
    Tcl_Obj *objlist[8];
  
    tcl_require_objc(3);    
    if (Tcl_GetIntFromObj(interp, objv[1], &id_source) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[2], &suppress_zero) != TCL_OK)
	return TCL_ERROR;

    source_type = hs_type(id_source);
    if (source_type != HS_1D_HISTOGRAM &&
	source_type != HS_2D_HISTOGRAM &&
	source_type != HS_3D_HISTOGRAM)
    {
	if (source_type == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a valid histoscope id", NULL);
	else
	    Tcl_AppendResult(interp, "item with id ", 
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a histogram", NULL);
	return TCL_ERROR;
    }
  
    nbins = hs_hist_num_bins(id_source);
    if ((sourcedata = (float *)malloc(nbins*sizeof(float))) == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	return TCL_ERROR;
    }
    if (source_type == HS_1D_HISTOGRAM)
	hs_1d_hist_bin_contents(id_source, sourcedata);
    else if (source_type == HS_2D_HISTOGRAM)
	hs_2d_hist_bin_contents(id_source, sourcedata);
    else if (source_type == HS_3D_HISTOGRAM)
	hs_3d_hist_bin_contents(id_source, sourcedata);
    else
	assert(0);
    if (arr_medirange(sourcedata, nbins, !suppress_zero, 
		      &arrmin, &q25, &median, &q75, &arrmax))
    {
	Tcl_SetResult(interp, "operation failed", TCL_STATIC);
	free(sourcedata);
	return TCL_ERROR;
    }
    arr_stats(sourcedata, nbins, !suppress_zero, &mean, &sigma);
    if (suppress_zero)
    {
	sum = 0.f;
	for (i=0; i<nbins; i++)
	    sum += sourcedata[i];
    }
    else
	sum = mean*(float)nbins;
    free(sourcedata);
  
    objlist[0] = Tcl_NewDoubleObj((double)sum);
    objlist[1] = Tcl_NewDoubleObj((double)mean);
    objlist[2] = Tcl_NewDoubleObj((double)sigma);
    objlist[3] = Tcl_NewDoubleObj((double)arrmin);
    objlist[4] = Tcl_NewDoubleObj((double)q25);
    objlist[5] = Tcl_NewDoubleObj((double)median);
    objlist[6] = Tcl_NewDoubleObj((double)q75);
    objlist[7] = Tcl_NewDoubleObj((double)arrmax);

    Tcl_SetObjResult(interp, Tcl_NewListObj(8, objlist));
    return TCL_OK;
}


tcl_routine(column_stats)
{
    /* Usage: column_stats $ntuple_id $column_number $suppress_zero */
    int i, id_source, column, suppress_zero, nrows;
    float *sourcedata;
    float sum, mean, sigma, arrmin, q25, median, q75, arrmax;
    Tcl_Obj *objlist[8];
  
    tcl_require_objc(4);
    verify_ntuple(id_source,1);
    if (Tcl_GetIntFromObj(interp, objv[2], &column) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[3], &suppress_zero) != TCL_OK)
	return TCL_ERROR;

    if (column < 0 || column >= hs_num_variables(id_source))
    {
	Tcl_SetResult(interp, "column number is out of range", TCL_STATIC);
	return TCL_ERROR;
    }
    if ((nrows = hs_num_entries(id_source)) == 0)
    {
	Tcl_SetResult(interp, "ntuple is empty", TCL_STATIC);
	return TCL_ERROR;
    }
    if ((sourcedata = (float *)malloc(nrows*sizeof(float))) == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	return TCL_ERROR;
    }
    hs_column_contents(id_source, column, sourcedata);
    if (arr_medirange(sourcedata, nrows, !suppress_zero, 
		      &arrmin, &q25, &median, &q75, &arrmax))
    {
	Tcl_SetResult(interp, "operation failed", TCL_STATIC);
	free(sourcedata);
	return TCL_ERROR;
    }
    arr_stats(sourcedata, nrows, !suppress_zero, &mean, &sigma);
    if (suppress_zero)
    {
	sum = 0.f;
	for (i=0; i<nrows; i++)
	    sum += sourcedata[i];
    }
    else
	sum = mean*(float)nrows;
    free(sourcedata);
  
    objlist[0] = Tcl_NewDoubleObj((double)sum);
    objlist[1] = Tcl_NewDoubleObj((double)mean);
    objlist[2] = Tcl_NewDoubleObj((double)sigma);
    objlist[3] = Tcl_NewDoubleObj((double)arrmin);
    objlist[4] = Tcl_NewDoubleObj((double)q25);
    objlist[5] = Tcl_NewDoubleObj((double)median);
    objlist[6] = Tcl_NewDoubleObj((double)q75);
    objlist[7] = Tcl_NewDoubleObj((double)arrmax);

    Tcl_SetObjResult(interp, Tcl_NewListObj(8, objlist));
    return TCL_OK;
}


tcl_routine(weighted_column_stats)
{
    /* Usage: weighted_column_stats $ntuple_id $column_number $weight_column */
    int i, id_source, column, weight_column, nvars, nrows;
    float *sourcedata = NULL, *weightdata;
    struct weighted_point *wpoints = NULL;
    float mean, sigma, arrmin, q25, median, q75, arrmax;
    Tcl_Obj *objlist[8];
    int status = TCL_ERROR;
    double sum;

    /* Parse the arguments */
    tcl_require_objc(4);
    verify_ntuple(id_source, 1);
    if (Tcl_GetIntFromObj(interp, objv[2], &column) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[3], &weight_column) != TCL_OK)
	return TCL_ERROR;

    /* Check column numbers */
    nvars = hs_num_variables(id_source);
    if (column < 0 || column >= nvars)
    {
	Tcl_SetResult(interp, "column number is out of range", TCL_STATIC);
	return TCL_ERROR;
    }
    if (weight_column < 0 || weight_column >= nvars)
    {
	Tcl_SetResult(interp, "weight column number is out of range",
		      TCL_STATIC);
	return TCL_ERROR;
    }

    /* Allocate the memory */
    if ((nrows = hs_num_entries(id_source)) == 0)
    {
	Tcl_SetResult(interp, "ntuple is empty", TCL_STATIC);
	return TCL_ERROR;
    }
    wpoints = (struct weighted_point *)malloc(
	nrows*sizeof(struct weighted_point));
    sourcedata = (float *)malloc(2*nrows*sizeof(float));
    if (sourcedata == NULL || wpoints == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	goto fail;
    }
    weightdata = sourcedata+nrows;

    /* Get the ntuple data */
    hs_column_contents(id_source, column, sourcedata);
    hs_column_contents(id_source, weight_column, weightdata);
    sum = 0.0;
    for (i=0; i<nrows; ++i)
    {
	register float w = weightdata[i];
	if (w < 0.f)
	{
	    char buf[64];
	    sprintf(buf, "negative weight in row %d", i);
	    Tcl_SetResult(interp, buf, TCL_VOLATILE);
	    goto fail;
	}
	wpoints[i].x = sourcedata[i];
	wpoints[i].w = w;
	sum += (double)(w);
    }
    free(sourcedata);
    sourcedata = NULL;

    /* Calculate the statistics */
    if (arr_medirange_weighted(wpoints, nrows, &arrmin,
			       &q25, &median, &q75, &arrmax))
    {
	Tcl_SetResult(interp, "calculation of robust statistics failed",
		      TCL_STATIC);
	goto fail;
    }
    if (arr_stats_weighted(wpoints, nrows, &mean, &sigma))
    {
	Tcl_SetResult(interp,
		      "calculation of mean and standard deviation failed",
		      TCL_STATIC);
	goto fail;
    }

    objlist[0] = Tcl_NewDoubleObj(sum);
    objlist[1] = Tcl_NewDoubleObj((double)mean);
    objlist[2] = Tcl_NewDoubleObj((double)sigma);
    objlist[3] = Tcl_NewDoubleObj((double)arrmin);
    objlist[4] = Tcl_NewDoubleObj((double)q25);
    objlist[5] = Tcl_NewDoubleObj((double)median);
    objlist[6] = Tcl_NewDoubleObj((double)q75);
    objlist[7] = Tcl_NewDoubleObj((double)arrmax);

    Tcl_SetObjResult(interp, Tcl_NewListObj(8, objlist));
    status = TCL_OK;

 fail:
    if (sourcedata)
	free(sourcedata);
    if (wpoints)
	free(wpoints);
    return status;
}


tcl_routine(column_range)
{
    /* Usage: column_range $ntuple_id $column_number */
    int i, id_source, column, nrows;
    float *sourcedata;
    float arrmin, arrmax;
    Tcl_Obj *objlist[2];
  
    tcl_require_objc(3);
    verify_ntuple(id_source,1);
    if (Tcl_GetIntFromObj(interp, objv[2], &column) != TCL_OK)
	return TCL_ERROR;

    if (column < 0 || column >= hs_num_variables(id_source))
    {
	Tcl_SetResult(interp, "column number is out of range", TCL_STATIC);
	return TCL_ERROR;
    }
    if ((nrows = hs_num_entries(id_source)) == 0)
    {
	Tcl_SetResult(interp, "ntuple is empty", TCL_STATIC);
	return TCL_ERROR;
    }
    if ((sourcedata = (float *)malloc(nrows*sizeof(float))) == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	return TCL_ERROR;
    }
    hs_column_contents(id_source, column, sourcedata);
    arrmin = FLT_MAX;
    arrmax = -FLT_MAX;
    for (i=0; i<nrows; ++i)
    {
	if (sourcedata[i] > arrmax)
	    arrmax = sourcedata[i];
	if (sourcedata[i] < arrmin)
	    arrmin = sourcedata[i];
    }
    free(sourcedata);
  
    objlist[0] = Tcl_NewDoubleObj((double)arrmin);
    objlist[1] = Tcl_NewDoubleObj((double)arrmax);
    Tcl_SetObjResult(interp, Tcl_NewListObj(2, objlist));
    return TCL_OK;
}


tcl_routine(stats_histogram)
{
    int id_source, suppress_zero, id_fill;
    int i, source_type, nbins;
    float *sourcedata;
    double w;
    float weight;
  
    if (objc != 4 && objc != 5)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, objv[1], &id_source) != TCL_OK)
	return TCL_ERROR;  
    if (Tcl_GetIntFromObj(interp, objv[2], &suppress_zero) != TCL_OK)
	return TCL_ERROR;
    verify_1d_histo(id_fill,3);
    if (objc == 5) 
    {
	if (Tcl_GetDoubleFromObj(interp, objv[4], &w) != TCL_OK)
	    return TCL_ERROR;
	else
	    weight = (float)w;
    }
    else
	weight = 1.0f;

    source_type = hs_type(id_source);
    if (source_type != HS_1D_HISTOGRAM &&
	source_type != HS_2D_HISTOGRAM &&
	source_type != HS_3D_HISTOGRAM)
    {
	if (source_type == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a valid histoscope id", NULL);
	else
	    Tcl_AppendResult(interp, "item with id ", 
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a histogram", NULL);
	return TCL_ERROR;
    }

    nbins = hs_hist_num_bins(id_source);
    if ((sourcedata = (float *)malloc(nbins*sizeof(float))) == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	return TCL_ERROR;
    }
    if (source_type == HS_1D_HISTOGRAM)
	hs_1d_hist_bin_contents(id_source, sourcedata);
    else if (source_type == HS_2D_HISTOGRAM)
	hs_2d_hist_bin_contents(id_source, sourcedata);
    else if (source_type == HS_3D_HISTOGRAM)
	hs_3d_hist_bin_contents(id_source, sourcedata);
    else
	assert(0);
    if (suppress_zero == 0)
	for (i=0; i<nbins; i++)
	    hs_fill_1d_hist(id_fill, sourcedata[i], weight);
    else
	for (i=0; i<nbins; i++)
	    if (sourcedata[i] != 0.f)
		hs_fill_1d_hist(id_fill, sourcedata[i], weight);
    free(sourcedata);
    return TCL_OK;
}


tcl_routine(adaptive_stats_histogram)
{  
    int id_source, suppress_zero, uid;
    int i, id_fill, new_bins, source_type, nbins;
    float *sourcedata;
    float arrmin, q25, median, q75, arrmax;
    float xmin, xmax, binwidth;
    double width_multiplier = 1.0;
    char stringbuf[1024];

    tcl_objc_range(6,7);
    if (Tcl_GetIntFromObj(interp, objv[1], &id_source) != TCL_OK)
	return TCL_ERROR;  
    if (Tcl_GetIntFromObj(interp, objv[2], &suppress_zero) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[3], &uid) != TCL_OK)
	return TCL_ERROR;
    if (objc > 6)
    {
	if (Tcl_GetDoubleFromObj(interp, objv[6], &width_multiplier) != TCL_OK)
	    return TCL_ERROR;
	if (width_multiplier <= 0.0)
	{
	    Tcl_SetResult(interp, "width multiplier is not positive", TCL_STATIC);
	    return TCL_ERROR;
	}
    }  

    source_type = hs_type(id_source);
    if (source_type != HS_1D_HISTOGRAM &&
	source_type != HS_2D_HISTOGRAM &&
	source_type != HS_3D_HISTOGRAM)
    {      
	if (source_type == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a valid histoscope id", NULL);
	else
	    Tcl_AppendResult(interp, "item with id ", 
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a histogram", NULL);
	return TCL_ERROR;
    }
    
    nbins = hs_hist_num_bins(id_source);

    if ((sourcedata = (float *)malloc(nbins*sizeof(float))) == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	return TCL_ERROR;
    }
    if (source_type == HS_1D_HISTOGRAM)
    {
	hs_1d_hist_bin_contents(id_source, sourcedata);
	hs_1d_hist_labels(id_source, stringbuf+256, stringbuf);
    }
    else if (source_type == HS_2D_HISTOGRAM)
    {
	hs_2d_hist_bin_contents(id_source, sourcedata);
	hs_2d_hist_labels(id_source, stringbuf+512, stringbuf+256, stringbuf);
    }
    else if (source_type == HS_3D_HISTOGRAM)
    {
	hs_3d_hist_bin_contents(id_source, sourcedata);
	hs_3d_hist_labels(id_source, stringbuf+768, stringbuf+512,
			  stringbuf+256, stringbuf);
    }
    else
	assert(0);
    if (arr_medirange(sourcedata, nbins, !suppress_zero, 
		      &arrmin, &q25, &median, &q75, &arrmax))
    {
	Tcl_SetResult(interp, "operation failed", TCL_STATIC);
	free(sourcedata);
	return TCL_ERROR;
    }
    if (arrmin == 0.f)
	xmin = 0.f;
    else
	xmin = arrmin - 0.05f*(arrmax - arrmin);
    xmax = arrmax + 0.05f*(arrmax - arrmin);
    if (xmax == xmin)
	xmax += 0.1f;

    /* The following is the MISE-optimal binwidth for the histogram 
       of normal density. The constant factor 3.49083 = (24 sqrt(Pi))**(1/3) */
    binwidth = 3.49083f*(q75-q25)*RANGE2SIG*(float)pow((double)nbins,-1.0/3.0);
    
    /* Allow the user to adjust the binwidth */
    binwidth *= (float)width_multiplier;
    
    if (binwidth > 0.f)
	new_bins = (int)((xmax - xmin)/binwidth)+1;
    else
	new_bins = 50;

    /* It is possible to get a very large number of bins if the data sample
       has a very far outlier. We will just limit the number of bins. */
    if (new_bins > 10000)
	new_bins = 10000;
    
    id_fill = hs_create_1d_hist(uid, Tcl_GetStringFromObj(objv[4], NULL), 
				Tcl_GetStringFromObj(objv[5], NULL), stringbuf,
				"Frequency", new_bins, xmin, xmax);
    if (id_fill <= 0)
    {
	Tcl_SetResult(interp, "histogram creation failed", TCL_STATIC);
	free(sourcedata);
	return TCL_ERROR;
    }

    if (suppress_zero == 0)
	for (i=0; i<nbins; i++)
	    hs_fill_1d_hist(id_fill, sourcedata[i], 1.f);
    else
	for (i=0; i<nbins; i++)
	    if (sourcedata[i] != 0.f)
		hs_fill_1d_hist(id_fill, sourcedata[i], 1.f);
    free(sourcedata);

    Tcl_SetObjResult(interp, Tcl_NewIntObj(id_fill));
    return TCL_OK;
}


tcl_routine(Ntuple_covar)
{
    /* Usage: Ntuple_covar $ntuple_id $column_indices */
    int i, col, ntuple_id, listlen, nrows, ncols, status = TCL_ERROR;
    Tcl_Obj **listObjElem;
    size_t *columns = NULL;
    float *data = NULL, *averages, *covar;
    Tcl_Obj *result, *aver, *mat, *row;

    tcl_require_objc(3);
    verify_ntuple(ntuple_id, 1);
    if (Tcl_ListObjGetElements(interp, objv[2], &listlen, &listObjElem) != TCL_OK)
	return TCL_ERROR;
    if (listlen == 0)
    {
	Tcl_SetResult(interp, "the list of column indices is empty", TCL_STATIC);
	return TCL_ERROR;
    }
    columns = (size_t *)malloc(listlen*sizeof(size_t));
    if (columns == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	goto fail;
    }
    ncols = hs_num_variables(ntuple_id);
    for (i=0; i<listlen; ++i)
    {
	if (Tcl_GetIntFromObj(interp, listObjElem[i], &col) != TCL_OK)
	    goto fail;
	if (col < 0 || col >= ncols)
	{
	    Tcl_AppendResult(interp, "column number ",
			     Tcl_GetStringFromObj(listObjElem[i], NULL),
			     " is out of range for ntuple with id ",
			     Tcl_GetStringFromObj(objv[1], NULL), NULL);
	    goto fail;
	}
	columns[i] = col;
    }
    nrows = hs_num_entries(ntuple_id);
    if (nrows == 0)
    {
	Tcl_SetResult(interp, "the ntuple is empty", TCL_STATIC);
	goto fail;
    }
    data = (float *)malloc((ncols*nrows + listlen + listlen*listlen)*sizeof(float));
    if (data == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	goto fail;
    }
    averages = data + ncols*nrows;
    covar = averages + listlen;
    hs_ntuple_contents(ntuple_id, data);
    if (arr_2d_weighted_covar(data, NULL, ncols, nrows, columns,
			      listlen, averages, covar))
    {
	Tcl_SetResult(interp, "operation failed", TCL_STATIC);
	goto fail;
    }

    /* Construct the result */
    aver = Tcl_NewListObj(0, NULL);
    for (i=0; i<listlen; ++i)
	Tcl_ListObjAppendElement(interp, aver, Tcl_NewDoubleObj(averages[i]));

    mat = Tcl_NewListObj(0, NULL);
    for (i=0; i<listlen; ++i)
    {
	row = Tcl_NewListObj(0, NULL);
	for (col=0; col<listlen; ++col)
	    Tcl_ListObjAppendElement(
		interp, row, Tcl_NewDoubleObj(covar[i*listlen + col]));
	Tcl_ListObjAppendElement(interp, mat, row);
    }

    result = Tcl_NewListObj(0, NULL);
    Tcl_ListObjAppendElement(interp, result, aver);
    Tcl_ListObjAppendElement(interp, result, mat);
    Tcl_SetObjResult(interp, result);

    status = TCL_OK;
 fail:
    if (columns) free(columns);
    if (data) free(data);
    return status;
}

