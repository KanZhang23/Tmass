#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "histoscope.h"
#include "histo_tcl_api.h"
#include "histo_utils.h"

float hs_hist_bin_width(int id)
{
    int n_x_bins, n_y_bins, n_z_bins;
    float xmin, xmax, ymin, ymax, zmin, zmax, width;

    switch (hs_type(id))
    {
    case HS_1D_HISTOGRAM:
	n_x_bins = hs_1d_hist_num_bins(id);
	hs_1d_hist_range(id, &xmin, &xmax);
	width = (xmax - xmin)/(float)n_x_bins;
	break;
    case HS_2D_HISTOGRAM:
	hs_2d_hist_num_bins(id, &n_x_bins, &n_y_bins);
	hs_2d_hist_range(id, &xmin, &xmax, &ymin, &ymax);
	width = ((xmax-xmin)/(float)n_x_bins)*((ymax-ymin)/(float)n_y_bins);
	break;
    case HS_3D_HISTOGRAM:
	hs_3d_hist_num_bins(id, &n_x_bins, &n_y_bins, &n_z_bins);
	hs_3d_hist_range(id, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
	width = ((xmax-xmin)/(float)n_x_bins)*
	    ((ymax-ymin)/(float)n_y_bins)*
	    ((zmax-zmin)/(float)n_z_bins);
	break;
    default:
	width = -1.0f;
	break;
    }
    return width;
}

int hs_hist_num_bins(int id)
{
    int n_x_bins, n_y_bins, n_z_bins, n_bins;

    switch (hs_type(id))
    {
    case HS_1D_HISTOGRAM:
	n_bins = hs_1d_hist_num_bins(id);
	break;
    case HS_2D_HISTOGRAM:
	hs_2d_hist_num_bins(id, &n_x_bins, &n_y_bins);
	n_bins = n_x_bins * n_y_bins;
	break;
    case HS_3D_HISTOGRAM:
	hs_3d_hist_num_bins(id, &n_x_bins, &n_y_bins, &n_z_bins);
	n_bins = n_x_bins * n_y_bins * n_z_bins;
	break;
    default:
	n_bins = -1;
	break;
    }
    return n_bins;
}

int hs_copy_data(int id_to, int id_from, float mult)
{
    int i, type_to, type_from, nbins, errtype;
    float *data, *poserr, *negerr;

    type_to = hs_type(id_to);
    type_from = hs_type(id_from);
    if ((type_to != HS_1D_HISTOGRAM && 
	 type_to != HS_2D_HISTOGRAM && 
	 type_to != HS_3D_HISTOGRAM) ||
	(type_from != HS_1D_HISTOGRAM && 
	 type_from != HS_2D_HISTOGRAM && 
	 type_from != HS_3D_HISTOGRAM))
    {
/*  	fprintf(stderr, "hs_copy_data: one of the items is not a histogram\n"); */
	return -1;
    }
    nbins = hs_hist_num_bins(id_to);
    if (nbins != hs_hist_num_bins(id_from))
    {
/*  	fprintf(stderr, "hs_copy_data: histograms are not bin-compatible\n"); */
	return -1;
    }
    if ((data = (float *)malloc(3*nbins*sizeof(float))) == NULL)
    {
	fprintf(stderr, "hs_copy_data: out of memory\n");
	return -2;
    }
    poserr = data + nbins;
    negerr = data + 2*nbins;
    switch (type_from)
    {
    case HS_1D_HISTOGRAM:
	hs_1d_hist_bin_contents(id_from, data);
	errtype = hs_1d_hist_errors(id_from, poserr, negerr);
	break;
    case HS_2D_HISTOGRAM:
	hs_2d_hist_bin_contents(id_from, data);
	errtype = hs_2d_hist_errors(id_from, poserr, negerr);
	break;
    case HS_3D_HISTOGRAM:
	hs_3d_hist_bin_contents(id_from, data);
	errtype = hs_3d_hist_errors(id_from, poserr, negerr);
	break;
    default:
	assert(0);
    }
    if (errtype == HS_NO_ERRORS)
    {
	negerr = NULL;
	poserr = NULL;
    }
    else if (errtype == HS_POS_ERRORS)
    {
	negerr = NULL;
    }
    if (mult != 1.0f)
    {
	for (i=0; i<nbins; ++i)
	    data[i] *= mult;
	if (poserr)
	    for (i=0; i<nbins; ++i)
		poserr[i] *= mult;
	if (negerr)
	    for (i=0; i<nbins; ++i)
		negerr[i] *= mult;
    }

    if (type_to == HS_1D_HISTOGRAM)
	hs_1d_hist_block_fill(id_to, data, poserr, negerr);
    else if (type_to == HS_2D_HISTOGRAM)
	hs_2d_hist_block_fill(id_to, data, poserr, negerr);
    else if (type_to == HS_3D_HISTOGRAM)
	hs_3d_hist_block_fill(id_to, data, poserr, negerr);
    else
	assert(0);

    free(data);
    return 0;
}

tcl_routine(copy_data)
{
    int id_to, id_from, status;
    float fmult = 1.0f;
    double dmult;

    tcl_objc_range(3,4);
    if (Tcl_GetIntFromObj(interp, objv[1], &id_to) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[2], &id_from) != TCL_OK)
	return TCL_ERROR;
    if (objc > 3)
    {
	if (Tcl_GetDoubleFromObj(interp, objv[3], &dmult) != TCL_OK)
	    return TCL_ERROR;
	fmult = dmult;
    }
    status = hs_copy_data(id_to, id_from, fmult);
    if (status)
    {
	if (status == -1)
	    Tcl_AppendResult(interp, "Histoscope items with ids ",
			     Tcl_GetStringFromObj(objv[1],NULL), " and ",
			     Tcl_GetStringFromObj(objv[2],NULL), 
			     " are either incompatible or do not exist", NULL);
	else if (status == -2)
	    Tcl_SetResult(interp, "out of memory", TCL_VOLATILE);
	else
	{
	    /* This should never happen */
	    fprintf(stderr, "Internal bug in hs_copy_data API. "
		    "Please report. Aborting\n.");
	    fflush(stderr);
	    abort();
	}
	return TCL_ERROR;
    }
    return TCL_OK;
}

int hs_duplicate_ntuple_header(int id, int uid, char *title, char *category)
{
    int type, length, column, nvars, nnames = 0, newid = -1;
    char varname[260];
    char **names = NULL;

    type = hs_type(id);
    if (type != HS_NTUPLE)
    {
	if (type == HS_NONE)
	    fprintf(stderr, "hs_duplicate_ntuple_header: "
		    "item with id %d does not exist\n", id);
	else
	    fprintf(stderr, "hs_duplicate_ntuple_header: "
		    "item with id %d is not an ntuple\n", id);
	return -1;
    }
    nvars = hs_num_variables(id);
    names = (char **)malloc(nvars*sizeof(char *));
    if (names == NULL)
    {
	fprintf(stderr, "hs_duplicate_ntuple_header: out of memory\n");
	return -1;
    }
    for (column = 0; column < nvars; ++column)
    {
	length = hs_variable_name(id, column, varname);
        if (length >= 260)
        {
            /* This should never happen */
            fprintf(stderr, "Fatal error in hs_duplicate_ntuple_header: buffer overrun\n");
            exit(EXIT_FAILURE);
        }
	varname[length] = '\0';
	names[nnames] = strdup(varname);
	if (names[nnames] == NULL)
	{
	    fprintf(stderr, "hs_duplicate_ntuple_header: out of memory\n");
	    goto fail;
	}
	++nnames;
    }
    newid = hs_create_ntuple(uid, title, category, nvars, names);

 fail:
    if (names)
    {
	for (column = 0; column < nnames; ++column)
	    free(names[column]);
	free(names);
    }
    return newid;
}

int hs_duplicate_axes(int id, int uid, char *title, char *category)
{
    char x_label[256], y_label[256], z_label[256], v_label[256];
    int new_id, type, x_bins, y_bins, z_bins;
    float x_min, x_max, y_min, y_max, z_min, z_max;

    type = hs_type(id);
    if (type != HS_1D_HISTOGRAM && 
	type != HS_2D_HISTOGRAM && 
	type != HS_3D_HISTOGRAM)
    {
	if (type == HS_NONE)
	    fprintf(stderr, "hs_duplicate_axes: "
		    "item with id %d does not exist\n", id);
	else
	    fprintf(stderr, "hs_duplicate_axes: "
		    "item with id %d is not a histogram", id);
	return -1;
    }
    
    if (type == HS_2D_HISTOGRAM)
    {
	hs_2d_hist_labels(id, x_label, y_label, z_label);
	hs_2d_hist_num_bins(id, &x_bins, &y_bins);
	hs_2d_hist_range(id, &x_min, &x_max, &y_min, &y_max);
	new_id = hs_create_2d_hist(uid, title, category,
				   x_label, y_label,
				   z_label, x_bins, y_bins, x_min,
				   x_max, y_min, y_max);
    }
    else if (type == HS_1D_HISTOGRAM)
    {
	hs_1d_hist_labels(id, x_label, y_label);
	x_bins = hs_1d_hist_num_bins(id);
	hs_1d_hist_range(id, &x_min, &x_max);
	new_id = hs_create_1d_hist(uid, title, category, 
				   x_label, y_label,
				   x_bins, x_min, x_max);
    }
    else if (type == HS_3D_HISTOGRAM)
    {
	hs_3d_hist_labels(id, x_label, y_label, z_label, v_label);
	hs_3d_hist_num_bins(id, &x_bins, &y_bins, &z_bins);
	hs_3d_hist_range(id, &x_min, &x_max, &y_min, &y_max, &z_min, &z_max);
	new_id = hs_create_3d_hist(
	    uid, title, category,
	    x_label, y_label, z_label, v_label,
	    x_bins, y_bins, z_bins, x_min,
	    x_max, y_min, y_max, z_min, z_max);
    }
    else
	assert(0);

    if (new_id <= 0)
    {
	fprintf(stderr, "hs_duplicate_axes: "
		"failed to create a new histogram\n");
    }
    return new_id;
}

tcl_routine(duplicate_ntuple_header)
{
    /* Usage: duplicate_ntuple_header id uid title category */
    int id, uid, new_id;

    tcl_require_objc(5);
    verify_ntuple(id,1);
    if (Tcl_GetIntFromObj(interp, objv[2], &uid) != TCL_OK)
	return TCL_ERROR;
    new_id = hs_duplicate_ntuple_header(
	id, uid, Tcl_GetStringFromObj(objv[3], NULL), 
	Tcl_GetStringFromObj(objv[4], NULL));
    if (new_id <= 0)
    {
	Tcl_SetResult(interp, "operation failed", TCL_STATIC);
	return TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(new_id));
    return TCL_OK;    
}

tcl_routine(duplicate_axes)
{
    int id, uid, new_id;
    
    tcl_require_objc(5);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[2], &uid) != TCL_OK)
	return TCL_ERROR;
    new_id = hs_duplicate_axes(id, uid, Tcl_GetStringFromObj(objv[3], NULL), 
			       Tcl_GetStringFromObj(objv[4], NULL));
    if (new_id <= 0)
    {
	Tcl_SetResult(interp, "operation failed", TCL_STATIC);
	return TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(new_id));
    return TCL_OK;
}

void hs_swap_data_errors(int id, int errquest)
{
    int type, nbins, errtype;
    float *data = NULL, *poserr = NULL, *negerr = NULL;

    type = hs_type(id);
    if (type != HS_1D_HISTOGRAM && 
	type != HS_2D_HISTOGRAM && 
	type != HS_3D_HISTOGRAM)
    {
	if (type == HS_NONE)
	    fprintf(stderr, "hs_swap_data_errors: item with id %d does not exist\n", id);
	else
	    fprintf(stderr, "hs_swap_data_errors: item with id %d is not a histogram\n", id);
	return;
    }
    nbins = hs_hist_num_bins(id);
    data = (float *)malloc(3*nbins*sizeof(float));
    if (data == NULL)
    {
	fprintf(stderr, "hs_swap_data_errors: out of memory\n");
	goto fail;
    }
    poserr = data + nbins;
    negerr = poserr + nbins;
    if (type == HS_1D_HISTOGRAM)
    {
	hs_1d_hist_bin_contents(id, data);
	errtype = hs_1d_hist_errors(id, poserr, negerr);
    }
    else if (type == HS_2D_HISTOGRAM)
    {
	hs_2d_hist_bin_contents(id, data);
	errtype = hs_2d_hist_errors(id, poserr, negerr);
    }
    else if (type == HS_3D_HISTOGRAM)
    {
	hs_3d_hist_bin_contents(id, data);
	errtype = hs_3d_hist_errors(id, poserr, negerr);
    }
    else
	assert(0);

    if (errquest == HS_POS_ERRORS)
    {
	if (errtype == HS_NO_ERRORS || errtype == HS_POS_ERRORS)
	{
	    negerr = NULL;
	    if (errtype == HS_NO_ERRORS)
		memset(poserr, 0, nbins*sizeof(float));
	}
	if (type == HS_2D_HISTOGRAM)
	    hs_2d_hist_block_fill(id, poserr, data, negerr);
	else if (type == HS_1D_HISTOGRAM)
	    hs_1d_hist_block_fill(id, poserr, data, negerr);
	else if (type == HS_3D_HISTOGRAM)
	    hs_3d_hist_block_fill(id, poserr, data, negerr);
	else
	    assert(0);
    }
    else
    {
	if (errtype == HS_POS_ERRORS || errtype == HS_NO_ERRORS)
	{
	    memset(negerr, 0, nbins*sizeof(float));
	    if (errtype == HS_NO_ERRORS)
		memset(poserr, 0, nbins*sizeof(float));
	}
	if (type == HS_2D_HISTOGRAM)
	    hs_2d_hist_block_fill(id, negerr, poserr, data);
	else if (type == HS_1D_HISTOGRAM)
	    hs_1d_hist_block_fill(id, negerr, poserr, data);
	else if (type == HS_3D_HISTOGRAM)
	    hs_3d_hist_block_fill(id, negerr, poserr, data);
	else
	    assert(0);
    }
    
fail:
    if (data)
	free(data);
}
