#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "histoscope.h"
#include "histo_utils.h"
#include "f_math.h"
#include "histo_tcl_api.h"

static int find_id_in_table(int id);
static void handle_cleanup(ClientData cd);
static int create_new_handle(Tcl_Interp *interp, int id);

static int tablesize = 0;
static int n_commands = 0;
static int *id_table = NULL;

tcl_routine(calc)
{
    int cnum, id, type;
    char stringbuf[32];
    char *argv1;

    if (objc != 2)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    if (Tcl_GetIntFromObj(interp, objv[1], &id) == TCL_OK)
    {
	type = hs_type(id);
	if (type == HS_NONE)
	{
	    argv1 = Tcl_GetStringFromObj(objv[1],NULL);
	    Tcl_AppendResult(interp, "Histo-Scope item with id ", 
			     argv1, " not found", NULL);
	    return TCL_ERROR;
	}
	else if (type != HS_1D_HISTOGRAM && 
		 type != HS_2D_HISTOGRAM && 
		 type != HS_3D_HISTOGRAM)
	{
	    argv1 = Tcl_GetStringFromObj(objv[1],NULL);
	    Tcl_AppendResult(interp, "Histo-Scope item with id ", 
			     argv1, " is not a histogram", NULL);
	    return TCL_ERROR;
	}

	if ((cnum = find_id_in_table(id)) < 0)
	    return create_new_handle(interp, id);
	else
	{
	    sprintf(stringbuf, "::hs::calc_%d", cnum);
	    Tcl_SetResult(interp, stringbuf, TCL_VOLATILE);
	}
    }
    else
    {
	Tcl_ResetResult(interp);
	argv1 = Tcl_GetStringFromObj(objv[1],NULL);
	if (strcmp(argv1, "purge") == 0)
	{
	    for (cnum = 0; cnum < n_commands; cnum++)
	    {
		if (id_table[cnum] >= 0)
		{
		    sprintf(stringbuf, "::hs::calc_%d", cnum);
		    if (Tcl_DeleteCommand(interp, stringbuf))
			fprintf(stderr, 
				"WARNING in hs::calc purge: command name '%s' does not exist\n",
				stringbuf);
		}
	    }
	    if (id_table)
	    {
		free(id_table);
		id_table = NULL;
		tablesize = 0;
	    }
	    n_commands = 0;
	}

        else if (strcmp(argv1, "manual") == 0)
	    return Tcl_EvalEx(interp, "::hs::Print_manual hs_calc.txt",
			      -1, TCL_EVAL_DIRECT);

	else if (strcmp(argv1, "count") == 0)
	    Tcl_SetObjResult(interp, Tcl_NewIntObj(n_commands));

	else
	{
	    Tcl_AppendResult(interp, "invalid argument \"", argv1, "\"", NULL);
	    return TCL_ERROR;
	}
    }
  
    return TCL_OK;
}


static int calc_accumulate(const int id1, const int id2, const int add)
{
    static float* buf = 0;
    static int buflen = 0;

    int i, nb, dim;
    float* buf2 = 0;

    nb = hs_hist_num_bins(id1);
    if (nb != hs_hist_num_bins(id2))
        return 1;
    dim = histo_dim_from_type(hs_type(id1));
    if (dim != histo_dim_from_type(hs_type(id2)))
        return 2;

    if (nb > buflen)
    {
        if (buf) free(buf);
        buf = (float *)malloc(nb*2*sizeof(float));
        if (buf == NULL)
        {
            buflen = 0;
            return 3;
        }
        buflen = nb;
    }
    buf2 = buf + nb;

    switch (dim)
    {
    case 1:
        hs_1d_hist_bin_contents(id1, buf);
        hs_1d_hist_bin_contents(id2, buf2);
        break;
    case 2:
        hs_2d_hist_bin_contents(id1, buf);
        hs_2d_hist_bin_contents(id2, buf2);
        break;
    case 3:
        hs_3d_hist_bin_contents(id1, buf);
        hs_3d_hist_bin_contents(id2, buf2);
        break;
    default:
        assert(0);
    }

    if (add)
        for (i=0; i<nb; ++i)
            buf[i] += buf2[i];
    else
        for (i=0; i<nb; ++i)
            buf[i] -= buf2[i];

    switch (dim)
    {
    case 1:
        hs_1d_hist_block_fill(id1, buf, NULL, NULL);
        break;
    case 2:
        hs_2d_hist_block_fill(id1, buf, NULL, NULL);
        break;
    case 3:
        hs_3d_hist_block_fill(id1, buf, NULL, NULL);
        break;
    default:
        assert(0);
    }

    return 0;
}


EXTERN int tcl_c_name(calc_proc) (ClientData clientData,
				  Tcl_Interp *interp,
				  int argc, char *argv[])
{
    int id_to_delete = 0;
    int return_status = TCL_ERROR, new_id = -12345679;
    const int efemeral_uid = 2147483637;
    int id1, id2, hs_calc_uid, histo_type, arg2_type; 
    int x_bin_num, y_bin_num, z_bin_num, binnum, axis, proj_type;
    int bin_min, bin_max;
    Float_function_u funct_u;
    Float_function_b funct_b;
    double dx, dy;
    Tcl_CmdInfo hinfo;
    float x_mean, y_mean, z_mean, x_std_dev, y_std_dev, z_std_dev;
    float x, y, z, value, xmin, xmax, ymin, ymax, zmin, zmax, binsize;
    static char stringbuf[1024];
    Tcl_Obj *listelem[9];

    hs_calc_uid = n_commands + 1;
    id1 = id_table[(long)clientData];
    histo_type = hs_type(id1);
  
    if (histo_type != HS_1D_HISTOGRAM && 
	histo_type != HS_2D_HISTOGRAM &&
	histo_type != HS_3D_HISTOGRAM)
    {
	if (histo_type == HS_NONE)
	    sprintf(stringbuf, "%d is not a valid histoscope id", id1);
	else
	    sprintf(stringbuf, "item with id %d is not a histogram", id1);
	Tcl_SetResult(interp, stringbuf, TCL_STATIC);
	goto cleanup;
    }

    if (argc == 1)
    {
	Tcl_SetResult(interp, argv[0], TCL_VOLATILE);
    }

    else if (argc == 2)
    {
	if (strcmp(argv[1], "id") == 0)
	{
	    /* This is the Histo-Scope id operation */
	    Tcl_SetObjResult(interp, Tcl_NewIntObj(id1));
	}
	else if (strcmp(argv[1], "uid") == 0)
	{
	    Tcl_SetObjResult(interp, Tcl_NewIntObj(hs_uid(id1)));
	}
	else if (strcmp(argv[1], "type") == 0)
	{
	    /* Print type */
	    if (histo_type == HS_1D_HISTOGRAM)
		Tcl_SetResult(interp, "HS_1D_HISTOGRAM", TCL_STATIC);
	    else if (histo_type == HS_2D_HISTOGRAM)
		Tcl_SetResult(interp, "HS_2D_HISTOGRAM", TCL_STATIC);
	    else if (histo_type == HS_3D_HISTOGRAM)
		Tcl_SetResult(interp, "HS_3D_HISTOGRAM", TCL_STATIC);
	    else
	    {
		sprintf(stringbuf, "item with id %d is not a histogram", id1);
		Tcl_SetResult(interp, stringbuf, TCL_STATIC);
		goto cleanup;
	    }
	}
	else if (strcmp(argv[1], "title") == 0)
	{
	    hs_title(id1, stringbuf);
	    Tcl_SetResult(interp, stringbuf, TCL_STATIC);
	}
	else if (strncmp(argv[1], "categ", 5) == 0)
	{
	    hs_category(id1, stringbuf);
	    Tcl_SetResult(interp, stringbuf, TCL_STATIC);
	}
	else if (strcmp(argv[1], "print") == 0)
	{
	    if (histo_type == HS_1D_HISTOGRAM)
		dump_1d_histo(stdout, id1);
	    else if (histo_type == HS_2D_HISTOGRAM)
		dump_2d_histo(stdout, id1);
	    else if (histo_type == HS_3D_HISTOGRAM)
		dump_3d_histo(stdout, id1);
	    else
		assert(0);
	    fflush(stdout);
	}
	else if (strncmp(argv[1], "del", 3) == 0)
	{
	    Tcl_DeleteCommand(interp, argv[0]);
	}
	else if (strcmp(argv[1], "show") == 0)
	{
	    hs_category(id1, stringbuf+512);
	    hs_title(id1, stringbuf+768);
	    if (stringbuf[768] == '\0')
		sprintf(stringbuf+768, "Untitled %d", id1);
	    sprintf(stringbuf, 
		    "%s\n Category %s\n UID %d\n WindowGeometry 400x300+0+0\n WindowTitle %s\n HideLegend\n",
		    histo_type == HS_1D_HISTOGRAM ? "Histogram" :
		    (histo_type == HS_2D_HISTOGRAM ? "2DHistogram" : "3DHistogram"),
		    stringbuf+512, hs_uid(id1), stringbuf+768);
	    hs_load_config_string(stringbuf);
	}
	else if (strcmp(argv[1], "/-/") == 0)
	{
	    /* This is the unary minus */
	    new_id = hs_multiply_histograms(hs_calc_uid,"","Hs_calc_tmp",id1,0,-1.f);
	}
	else if (strcmp(argv[1], "/+/") == 0)
	{
	    /* This is the unary plus (histogram copy) */
	    new_id = hs_copy_hist(id1, hs_calc_uid, "", "Hs_calc_tmp");
	}
	else if (strncmp(argv[1], "bin", 3) == 0)
	{
	    if (histo_type == HS_1D_HISTOGRAM)
	    {
		hs_1d_hist_range(id1, &xmin, &xmax);
		listelem[0] = Tcl_NewDoubleObj((double)xmin);
		listelem[1] = Tcl_NewDoubleObj((double)xmax);
		listelem[2] = Tcl_NewIntObj(hs_1d_hist_num_bins(id1));
		Tcl_SetObjResult(interp, Tcl_NewListObj(3, listelem));
	    }
	    else if (histo_type == HS_2D_HISTOGRAM)
	    {
		hs_2d_hist_range(id1, &xmin, &xmax, &ymin, &ymax);
		hs_2d_hist_num_bins(id1, &x_bin_num, &y_bin_num);
		listelem[0] = Tcl_NewDoubleObj((double)xmin);
		listelem[1] = Tcl_NewDoubleObj((double)xmax);
		listelem[2] = Tcl_NewIntObj(x_bin_num);
		listelem[3] = Tcl_NewDoubleObj((double)ymin);
		listelem[4] = Tcl_NewDoubleObj((double)ymax);
		listelem[5] = Tcl_NewIntObj(y_bin_num);
		Tcl_SetObjResult(interp, Tcl_NewListObj(6, listelem));
	    }
	    else if (histo_type == HS_3D_HISTOGRAM)
	    {
		hs_3d_hist_range(id1, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
		hs_3d_hist_num_bins(id1, &x_bin_num, &y_bin_num, &z_bin_num);
		listelem[0] = Tcl_NewDoubleObj((double)xmin);
		listelem[1] = Tcl_NewDoubleObj((double)xmax);
		listelem[2] = Tcl_NewIntObj(x_bin_num);
		listelem[3] = Tcl_NewDoubleObj((double)ymin);
		listelem[4] = Tcl_NewDoubleObj((double)ymax);
		listelem[5] = Tcl_NewIntObj(y_bin_num);
		listelem[6] = Tcl_NewDoubleObj((double)zmin);
		listelem[7] = Tcl_NewDoubleObj((double)zmax);
		listelem[8] = Tcl_NewIntObj(z_bin_num);
		Tcl_SetObjResult(interp, Tcl_NewListObj(9, listelem));
	    }
	    else
		assert(0);
	}
	else if (strcmp(argv[1], "stats") == 0)
	{
	    /* Find statistics */
	    if (histo_type == HS_1D_HISTOGRAM)
	    {
		hs_1d_hist_stats(id1, &x_mean, &x_std_dev);
		listelem[0] = Tcl_NewDoubleObj((double)x_mean);
		listelem[1] = Tcl_NewDoubleObj((double)x_std_dev);
		Tcl_SetObjResult(interp, Tcl_NewListObj(2, listelem));
	    }
	    else if (histo_type == HS_2D_HISTOGRAM)
	    {
		hs_2d_hist_stats(id1, &x_mean, &y_mean, &x_std_dev, &y_std_dev);
		listelem[0] = Tcl_NewDoubleObj((double)x_mean);
		listelem[1] = Tcl_NewDoubleObj((double)y_mean);
		listelem[2] = Tcl_NewDoubleObj((double)x_std_dev);
		listelem[3] = Tcl_NewDoubleObj((double)y_std_dev);
		Tcl_SetObjResult(interp, Tcl_NewListObj(4, listelem));
	    }
	    else if (histo_type == HS_3D_HISTOGRAM)
	    {
		hs_3d_hist_stats(id1, &x_mean, &y_mean, &z_mean,
				 &x_std_dev, &y_std_dev, &z_std_dev);
		listelem[0] = Tcl_NewDoubleObj((double)x_mean);
		listelem[1] = Tcl_NewDoubleObj((double)y_mean);
		listelem[2] = Tcl_NewDoubleObj((double)z_mean);
		listelem[3] = Tcl_NewDoubleObj((double)x_std_dev);
		listelem[4] = Tcl_NewDoubleObj((double)y_std_dev);
		listelem[5] = Tcl_NewDoubleObj((double)z_std_dev);
		Tcl_SetObjResult(interp, Tcl_NewListObj(6, listelem));
	    }
	    else
		assert(0);
	}
	else if (strcmp(argv[1], "max") == 0)
	{
	    /* Find maximum */
	    if (histo_type == HS_1D_HISTOGRAM)
	    {
		hs_1d_hist_maximum(id1, &x, &x_bin_num, &value);
		listelem[0] = Tcl_NewDoubleObj((double)x);
		listelem[1] = Tcl_NewIntObj(x_bin_num);
		listelem[2] = Tcl_NewDoubleObj((double)value);
		Tcl_SetObjResult(interp, Tcl_NewListObj(3, listelem));
	    }
	    else if (histo_type == HS_2D_HISTOGRAM)
	    {
		hs_2d_hist_maximum(id1, &x, &y, &x_bin_num, &y_bin_num, &value);
		listelem[0] = Tcl_NewDoubleObj((double)x);
		listelem[1] = Tcl_NewDoubleObj((double)y);
		listelem[2] = Tcl_NewIntObj(x_bin_num);
		listelem[3] = Tcl_NewIntObj(y_bin_num);
		listelem[4] = Tcl_NewDoubleObj((double)value);
		Tcl_SetObjResult(interp, Tcl_NewListObj(5, listelem));
	    }
	    else if (histo_type == HS_3D_HISTOGRAM)
	    {
		hs_3d_hist_maximum(id1, &x, &y, &z, &x_bin_num,
				   &y_bin_num, &z_bin_num, &value);
		listelem[0] = Tcl_NewDoubleObj((double)x);
		listelem[1] = Tcl_NewDoubleObj((double)y);
		listelem[2] = Tcl_NewDoubleObj((double)z);
		listelem[3] = Tcl_NewIntObj(x_bin_num);
		listelem[4] = Tcl_NewIntObj(y_bin_num);
		listelem[5] = Tcl_NewIntObj(z_bin_num);
		listelem[6] = Tcl_NewDoubleObj((double)value);
		Tcl_SetObjResult(interp, Tcl_NewListObj(7, listelem));
	    }
	    else
		assert(0);
	}
	else if (strcmp(argv[1], "min") == 0)
	{
	    /* Find minimum */
	    if (histo_type == HS_1D_HISTOGRAM)
	    {
		hs_1d_hist_minimum(id1, &x, &x_bin_num, &value);
		listelem[0] = Tcl_NewDoubleObj((double)x);
		listelem[1] = Tcl_NewIntObj(x_bin_num);
		listelem[2] = Tcl_NewDoubleObj((double)value);
		Tcl_SetObjResult(interp, Tcl_NewListObj(3, listelem));
	    }
	    else if (histo_type == HS_2D_HISTOGRAM)
	    {
		hs_2d_hist_minimum(id1, &x, &y, &x_bin_num, &y_bin_num, &value);
		listelem[0] = Tcl_NewDoubleObj((double)x);
		listelem[1] = Tcl_NewDoubleObj((double)y);
		listelem[2] = Tcl_NewIntObj(x_bin_num);
		listelem[3] = Tcl_NewIntObj(y_bin_num);
		listelem[4] = Tcl_NewDoubleObj((double)value);
		Tcl_SetObjResult(interp, Tcl_NewListObj(5, listelem));
	    }
	    else if (histo_type == HS_3D_HISTOGRAM)
	    {
		hs_3d_hist_minimum(id1, &x, &y, &z, &x_bin_num,
				   &y_bin_num, &z_bin_num, &value);
		listelem[0] = Tcl_NewDoubleObj((double)x);
		listelem[1] = Tcl_NewDoubleObj((double)y);
		listelem[2] = Tcl_NewDoubleObj((double)z);
		listelem[3] = Tcl_NewIntObj(x_bin_num);
		listelem[4] = Tcl_NewIntObj(y_bin_num);
		listelem[5] = Tcl_NewIntObj(z_bin_num);
		listelem[6] = Tcl_NewDoubleObj((double)value);
		Tcl_SetObjResult(interp, Tcl_NewListObj(7, listelem));
	    }
	    else
		assert(0);
	}
	else if (strncmp(argv[1], "deriv", 5) == 0)
	{
	    /* Find derivative */
	    if (histo_type == HS_1D_HISTOGRAM)
		new_id = hs_1d_hist_derivative(hs_calc_uid, "", "Hs_calc_tmp", id1);
	    else
		new_id = -1;
	}
	else if (strncmp(argv[1], "integ", 5) == 0)
	{
	    /* Find integral of all bins */
	    Tcl_SetObjResult(interp, Tcl_NewDoubleObj((double)hs_hist_integral(id1)));
	}
	else if (strncmp(argv[1], "trans", 5) == 0)
	{
	    if (histo_type == HS_2D_HISTOGRAM)
	    {
		new_id = hs_transpose_histogram(hs_calc_uid, "", "Hs_calc_tmp", id1);
	    }
	    else
	    {
		sprintf(stringbuf, "item with id %d is not a 2d histogram", id1);
		Tcl_SetResult(interp, stringbuf, TCL_STATIC);
		goto cleanup;
	    }
	}    
	else if (strcmp(argv[1], "sum") == 0)
	{
	    /* Find sum of all bins */
	    binsize = hs_hist_bin_width(id1);
	    Tcl_SetObjResult(interp, Tcl_NewDoubleObj((double)(hs_hist_integral(id1)/binsize)));
	}
	else if ((funct_u = find_unary_funct(argv[1])) != NULL)
	{
	    /* This is some function applied to each bin */
	    new_id = hs_unary_op(hs_calc_uid, "", "Hs_calc_tmp", id1, funct_u, NULL);
	}
	else
	{
	    Tcl_AppendResult(interp, "invalid operator \"", 
			     argv[1], "\"", NULL);
	    goto cleanup;
	}
    }

    else if (argc == 3)
    {
	/* First, check if the last argument is another histogram */
	if (strncmp(argv[2], "::hs::calc_", 11) == 0 || 
	    strncmp(argv[2], "hs::calc_", 9) == 0 ||
	    strncmp(argv[2], "calc_", 5) == 0)
	{
	    if (Tcl_GetCommandInfo(interp, argv[2], &hinfo))
	    {
		id2 = id_table[(long)hinfo.clientData];
		arg2_type = hs_type(id2);
		if (arg2_type != HS_1D_HISTOGRAM && 
		    arg2_type != HS_2D_HISTOGRAM &&
		    arg2_type != HS_3D_HISTOGRAM)
		{
		    sprintf(stringbuf, 
			    "handle %s : invalid Histo-Scope histogram id %d",
			    argv[2], id2);
		    Tcl_SetResult(interp, stringbuf, TCL_STATIC);
		    goto cleanup;
		}
	    }
	    else
	    {
		Tcl_ResetResult(interp);
		Tcl_AppendResult(interp, "invalid argument \"", 
				 argv[2], "\"", NULL);
		goto cleanup;
	    }
	}
	else
	{
	    /* Check if the last argument is a number */
	    if (Tcl_GetDouble(interp, argv[2], &dx) != TCL_OK)
	    {
		Tcl_ResetResult(interp);
		Tcl_AppendResult(interp, "invalid argument \"", 
				 argv[2], "\"", NULL);
		goto cleanup;
	    }      
	    else
	    {
		if ((id2 = hs_copy_hist(id1, efemeral_uid, "", "Hs_calc_tmp")) > 0)
		{
		    id_to_delete = id2;
		    hs_reset_const(id2, (float)dx);
		}
		else
		{
		    Tcl_SetResult(interp, "operation failed", TCL_STATIC);
		    goto cleanup;
		}
	    }
	}

	/* Find out the requested operation */
	if (strcmp(argv[1], "+") == 0)
	{
	    new_id = hs_sum_histograms(hs_calc_uid,"","Hs_calc_tmp",id1,id2,1.f,1.f);
	}
	else if (strcmp(argv[1], "-") == 0)
	{
	    new_id = hs_sum_histograms(hs_calc_uid,"","Hs_calc_tmp",id1,id2,1.f,-1.f);
	}
	else if (strcmp(argv[1], "*") == 0)
	{
	    new_id = hs_multiply_histograms(hs_calc_uid,"","Hs_calc_tmp",id1,id2,1.f);
	}
	else if (strcmp(argv[1], "/") == 0)
	{
	    new_id = hs_divide_histograms(hs_calc_uid,"","Hs_calc_tmp",id1,id2,1.f);
	}
	else if (strcmp(argv[1], "@") == 0)
	{
	    new_id = hs_concat_histograms(hs_calc_uid,"","Hs_calc_tmp",id1,id2);
	}
        else if (strcmp(argv[1], "+=") == 0)
        {
            if (calc_accumulate(id1, id2, 1))
                new_id = 0;
        }
        else if (strcmp(argv[1], "-=") == 0)
        {
            if (calc_accumulate(id1, id2, 0))
                new_id = 0;
        }
	else if ((funct_b = find_binary_funct(argv[1])) != NULL)
	{
	    new_id = hs_binary_op(hs_calc_uid,"","Hs_calc_tmp",id1,id2,funct_b,NULL);
	}
	else
	{
	    Tcl_AppendResult(interp, "invalid operator \"",
			     argv[1], "\"", NULL);
	    goto cleanup;
	}
    }

    else if (argc == 4 && histo_type == HS_1D_HISTOGRAM)
    {
	/* This may happen when we are required to print 
	   a bin value or extract a subrange */
	if (strncmp(argv[1], "val", 3) == 0)
	{
	    if (strcmp(argv[2], "coord") == 0)
	    {
		if (Tcl_GetDouble(interp, argv[3], &dx) != TCL_OK)
		    goto cleanup;
		listelem[0] = Tcl_NewDoubleObj((double)hs_1d_hist_x_value(id1, (float)dx));
	    }
	    else if (strcmp(argv[2], "bin") == 0)
	    {
		if (Tcl_GetInt(interp, argv[3], &x_bin_num) != TCL_OK)
		    goto cleanup;
		listelem[0] = Tcl_NewDoubleObj((double)hs_1d_hist_bin_value(id1, x_bin_num));
	    }
	    else
	    {
		sprintf(stringbuf, "%d", id1);
		Tcl_AppendResult(interp, "invalid 1-d histogram option \"",
				 argv[2], "\", argument id ", stringbuf, NULL);
		goto cleanup;
	    }
	    Tcl_SetObjResult(interp, listelem[0]);
	}
	else if (strncmp(argv[1], "subrang", 7) == 0)
	{
	    if (Tcl_GetInt(interp, argv[2], &bin_min) != TCL_OK)
		goto cleanup;
	    if (Tcl_GetInt(interp, argv[3], &bin_max) != TCL_OK)
		goto cleanup;
	    new_id = hs_1d_hist_subrange(hs_calc_uid,"","Hs_calc_tmp",
					 id1,bin_min,bin_max);
	}
	else
	{
	    sprintf(stringbuf, "%d", id1);
	    Tcl_AppendResult(interp, "invalid 1-d histogram option \"",
			     argv[2], "\", argument id ", stringbuf, NULL);
	    goto cleanup;
	}
    }  

    else if (argc == 4 && histo_type == HS_2D_HISTOGRAM)
    {
	if (strncmp(argv[1], "proj", 4) == 0)
	{
	    if (strcasecmp(argv[2], "x") == 0)
		axis = HS_AXIS_X;
	    else if (strcasecmp(argv[2], "y") == 0)
		axis = HS_AXIS_Y;
	    else
	    {
		Tcl_AppendResult(interp, "invalid 2d histogram axis \"",
				 argv[2], "\"", NULL);
		goto cleanup;
	    }
	    if (strcmp(argv[3], "mean") == 0 || strncmp(argv[3], "ave", 3) == 0)
		proj_type = HS_CALC_PROJ_AVE;
	    else if (strcmp(argv[3], "sum") == 0)
		proj_type = HS_CALC_PROJ_SUM;
	    else if (strncmp(argv[3], "med", 3) == 0)
		proj_type = HS_CALC_PROJ_MED;
	    else if (strncmp(argv[3], "min", 3) == 0)
		proj_type = HS_CALC_PROJ_MIN;
	    else if (strncmp(argv[3], "max", 3) == 0)
		proj_type = HS_CALC_PROJ_MAX;
	    else
	    {
		Tcl_AppendResult(interp, "invalid projection type \"",
				 argv[3], "\"", NULL);
		goto cleanup;
	    }
	    new_id = project_2d_histogram(hs_calc_uid, "", "Hs_calc_tmp", id1, 
				       axis, proj_type, 0);
	}
	else
	{
	    Tcl_AppendResult(interp, "invalid 2d histogram option \"",
			     argv[1], "\"", NULL);
	    goto cleanup;
	}    
    }

    else if (argc == 5 && histo_type == HS_2D_HISTOGRAM)
    {
	if (strncmp(argv[1], "val", 3) == 0)
	{
	    if (strcmp(argv[2], "coord") == 0)
	    {
		if (Tcl_GetDouble(interp, argv[3], &dx) != TCL_OK)
		    goto cleanup;
		if (Tcl_GetDouble(interp, argv[4], &dy) != TCL_OK)
		    goto cleanup;
		listelem[0] = Tcl_NewDoubleObj(
		    (double)hs_2d_hist_xy_value(id1, (float)dx, (float)dy));
	    }
	    else if (strcmp(argv[2], "bin") == 0)
	    {
		if (Tcl_GetInt(interp, argv[3], &x_bin_num) != TCL_OK)
		    goto cleanup;
		if (Tcl_GetInt(interp, argv[4], &y_bin_num) != TCL_OK)
		    goto cleanup;
		listelem[0] = Tcl_NewDoubleObj(
		    (double)hs_2d_hist_bin_value(id1, x_bin_num, y_bin_num));
	    }
	    else
	    {
		Tcl_AppendResult(interp, "invalid option \"",
				 argv[2], "\"", NULL);
		goto cleanup;
	    }
	    Tcl_SetObjResult(interp, listelem[0]);
	}
	else if (strcmp(argv[1], "slice") == 0)
	{
	    if (strcasecmp(argv[3], "x") == 0)
		axis = HS_AXIS_X;
	    else if (strcasecmp(argv[3], "y") == 0)
		axis = HS_AXIS_Y;
	    else
	    {
		Tcl_AppendResult(interp, "invalid 2d histogram axis \"",
				 argv[3], "\"", NULL);
		goto cleanup;
	    }

	    if (strcmp(argv[2], "coord") == 0)
	    {
		if (Tcl_GetDouble(interp, argv[4], &dx) != TCL_OK)
		    goto cleanup;
		hs_2d_hist_num_bins(id1, &x_bin_num, &y_bin_num);
		hs_2d_hist_range(id1, &xmin, &xmax, &ymin, &ymax);
		if (axis == HS_AXIS_X)
		    binnum = (int)(((float)dx-xmin)/(xmax - xmin)*(float)x_bin_num + 0.01);
		else
		    binnum = (int)(((float)dx-ymin)/(ymax - ymin)*(float)y_bin_num + 0.01);
		/* The +0.01 part in the above formulas makes sure that we get the correct
		   bin number when we are exactly on the left bin edge. Of course, it will
		   screw up the right bin edge. */
	    }
	    else if (strcmp(argv[2], "bin") == 0)
	    {
		if (Tcl_GetInt(interp, argv[4], &binnum) != TCL_OK)
		    goto cleanup;
	    }
	    else
	    {
		Tcl_AppendResult(interp, "invalid option \"",
				 argv[2], "\"", NULL);
		goto cleanup;
	    }
	    new_id = slice_2d_histogram(hs_calc_uid, "", "Hs_calc_tmp", id1, axis, binnum);
	}
	else if (strncmp(argv[1], "subrang", 7) == 0)
	{
	    if (strcasecmp(argv[2], "x") == 0)
		axis = HS_AXIS_X;
	    else if (strcasecmp(argv[2], "y") == 0)
		axis = HS_AXIS_Y;
	    else
	    {
		Tcl_AppendResult(interp, "invalid 2d histogram axis \"",
				 argv[2], "\"", NULL);
		goto cleanup;
	    }
	    if (Tcl_GetInt(interp, argv[3], &bin_min) != TCL_OK)
		goto cleanup;
	    if (Tcl_GetInt(interp, argv[4], &bin_max) != TCL_OK)
		goto cleanup;
	    new_id = hs_2d_hist_subrange(hs_calc_uid,"","Hs_calc_tmp",
					 id1,axis,bin_min,bin_max);
	}
	else
	{
	    Tcl_AppendResult(interp, "invalid option \"",
			     argv[1], "\"", NULL);
	    goto cleanup;
	}    
    }

    else
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	goto cleanup;
    }

    if (new_id > 0)
    {
	if (create_new_handle(interp, new_id) != TCL_OK)
	    goto cleanup;
    }
    else if (new_id != -12345679)
    {
	Tcl_SetResult(interp, "operation failed", TCL_STATIC);
	goto cleanup;
    }

    return_status = TCL_OK;
    
 cleanup:
    if (id_to_delete)
	hs_delete(id_to_delete);
    return return_status;
}


static int find_id_in_table(int id)
{
    int i;

    if (id_table)
	for(i=0; i<n_commands; i++)
	    if (id_table[i] == id)
		return i;
    return -1;
}


static void handle_cleanup(ClientData cd)
{
    int id1;
    char stringbuf[256];

    id1 = id_table[(long)cd];
    id_table[(long)cd] = -1;

    if (hs_category(id1, stringbuf) >= 0)
	if (strcmp(stringbuf, "Hs_calc_tmp") == 0)
	    hs_delete(id1);
}


static int create_new_handle(Tcl_Interp *interp, int id)
{
    char stringbuf[32];

    /* Invalidate the table entry in 0th position.
     * It prevents us from obtaining a wrong Histo-Scope id
     * in case clientData is NULL by some reason.
     */
    if (n_commands == 0)
    {
	tablesize = 5;
	id_table = (int *)realloc(id_table, tablesize*sizeof(int));
	if (id_table == NULL)
	{
	    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	    return TCL_ERROR;
	}
	id_table[0] = -1;
	n_commands = 1;
    }
    /* Get more memory if necessary */
    if (n_commands >= tablesize) 
    {
	tablesize += (2 + n_commands/5);
	id_table = (int *)realloc(id_table, tablesize*sizeof(int));
	if (id_table == NULL)
	{
	    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	    return TCL_ERROR;
	}
    }
    sprintf(stringbuf, "::hs::calc_%d", n_commands);
    if (Tcl_CreateCommand(interp, stringbuf,
			  (Tcl_CmdProc *) tcl_c_name(calc_proc),
			  (ClientData)((long)n_commands), handle_cleanup) == NULL)
	return TCL_ERROR;
    else
    {
	id_table[n_commands++] = id;
	Tcl_SetResult(interp, stringbuf, TCL_VOLATILE);
	return TCL_OK;
    }
}
