#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "histoscope.h"
#include "histo_tcl_api.h"
#include "histo_utils.h"

tcl_routine(ascii_dump)
{
    /* Usage: hs::ascii_dump $filename $idlist */
    int i, nids, ndump, type;
    int *ids;
    Tcl_Obj **listObjElem;
    char *pc, *filename;
    FILE *ofile;

    if (objc != 3)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    if (Tcl_ListObjGetElements(interp, objv[2],
			       &nids, &listObjElem) != TCL_OK)
	return TCL_ERROR;
    if ((ids = (int *)malloc(nids*sizeof(int))) == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	return TCL_ERROR;
    }
    for (i=0; i < nids; i++) 
    {
	if (Tcl_GetIntFromObj(interp, listObjElem[i], ids+i) != TCL_OK)
	{
	    free(ids);
	    return TCL_ERROR;
	}
    }
    filename = Tcl_GetStringFromObj(objv[1], NULL);
    pc = strrchr(filename, '/');
    if (pc == NULL)
	pc = filename;
    else
	pc++;
    if (strcmp(pc, "stdout") == 0)
	ofile = stdout;
    else if (strcmp(pc, "stderr") == 0)
	ofile = stderr;
    else
    {
	if ((ofile = fopen(filename, "w")) == NULL)
	{
	    Tcl_AppendResult(interp, "can't open file \"",
			     filename, "\" for write", NULL);
	    free(ids);
	    return TCL_ERROR;
	}
    }

    ndump = 0;
    for (i=0; i<nids; i++)
    {
	type = hs_type(ids[i]);
	if (type == HS_1D_HISTOGRAM)
	{
	    if (i != 0)
		fprintf(ofile, "\n");
	    if (dump_1d_histo(ofile, ids[i]))
	    {
		free(ids);
		return TCL_ERROR;
	    }
	    ndump++;
	}
	else if (type == HS_2D_HISTOGRAM)
	{
	    if (i != 0)
		fprintf(ofile, "\n");
	    if (dump_2d_histo(ofile, ids[i]))
	    {
		free(ids);
		return TCL_ERROR;
	    }
	    ndump++;
	}
	else if (type == HS_3D_HISTOGRAM)
	{
	    if (i != 0)
		fprintf(ofile, "\n");
	    if (dump_3d_histo(ofile, ids[i]))
	    {
		free(ids);
		return TCL_ERROR;
	    }
	    ndump++;
	}
	else if (type == HS_NTUPLE)
	{
	    if (i != 0)
		fprintf(ofile, "\n");
	    if (dump_ntuple(ofile, ids[i]))
	    {
		free(ids);
		return TCL_ERROR;
	    }
	    ndump++;
	}
    }

    free(ids);
    if (ofile == stdout || ofile == stderr)
	fflush(ofile);
    else
	fclose(ofile);

    Tcl_SetObjResult(interp, Tcl_NewIntObj(ndump));
    return TCL_OK; 
}


int dump_1d_histo(FILE *ofile, int id)
{
  int i, num_bins, iferrors;
  float x_min, x_max, x_step, *pdata, *perr, *nerr;
  char title[256];
  
  num_bins = hs_1d_hist_num_bins(id);
  if ((pdata = (float *)malloc(3*num_bins*sizeof(float))) == NULL)
  {
    fprintf(stderr, "dump_1d_histo: out of memory\n");
    return -1;
  }
  perr = pdata + num_bins;
  nerr = perr + num_bins;

  hs_1d_hist_range(id, &x_min, &x_max);
  x_step = (x_max-x_min)/(float)num_bins;
  hs_1d_hist_bin_contents(id, pdata);
  iferrors = hs_1d_hist_errors(id, perr, nerr);
  
  hs_title(id, title);
  fprintf(ofile, "# UID: %d\n", hs_uid(id));
  fprintf(ofile, "# Title: %s\n", title);
  hs_category(id, title);
  fprintf(ofile, "# Category: %s\n", title);
  fprintf(ofile, "# xmin = %f, xmax = %f, nbins = %d\n", 
	  x_min, x_max, num_bins);
  if (iferrors == HS_NO_ERRORS)
  {
    for (i=0; i<num_bins; i++)
	fprintf(ofile, "%f %f\n", x_min+x_step*(float)i, pdata[i]);
  }
  else if (iferrors == HS_POS_ERRORS)
  {
    for (i=0; i<num_bins; i++)
	fprintf(ofile, "%f %f %f\n", x_min+x_step*(float)i,
		pdata[i], perr[i]);
  }
  else if (iferrors == HS_BOTH_ERRORS)
  {
    for (i=0; i<num_bins; i++)
	fprintf(ofile, "%f %f %f %f\n", x_min+x_step*(float)i, 
		pdata[i], perr[i], nerr[i]);
  }
  else
      assert(0);

  free(pdata);
  return 0;
}


int dump_2d_histo(FILE *ofile, int id)
{
  int i, j, num_bins, num_x_bins, num_y_bins, iferrors;
  float x_min, x_max, y_min, y_max, x_step, y_step;
  float *pdata, *perr, *nerr;
  char title[256];

  hs_2d_hist_num_bins(id, &num_x_bins, &num_y_bins);
  num_bins = num_x_bins*num_y_bins;
  if ((pdata = (float *)malloc(3*num_bins*sizeof(float))) == NULL)
  {
    fprintf(stderr, "dump_2d_histo: out of memory\n");
    return -1;
  }
  perr = pdata + num_bins;
  nerr = perr + num_bins;

  hs_2d_hist_range(id, &x_min, &x_max, &y_min, &y_max);
  x_step = (x_max-x_min)/(float)num_x_bins;
  y_step = (y_max-y_min)/(float)num_y_bins;
  hs_2d_hist_bin_contents(id, pdata);
  iferrors = hs_2d_hist_errors(id, perr, nerr);

  hs_title(id, title);
  fprintf(ofile, "# UID: %d\n", hs_uid(id));
  fprintf(ofile, "# Title: %s\n", title);
  hs_category(id, title);
  fprintf(ofile, "# Category: %s\n", title);
  fprintf(ofile, "# xmin = %f, xmax = %f, xbins = %d\n", x_min, x_max, num_x_bins);
  fprintf(ofile, "# ymin = %f, ymax = %f, ybins = %d\n", y_min, y_max, num_y_bins);

  if (iferrors == HS_NO_ERRORS)
  {
    for (i=0; i<num_x_bins; i++)
      for (j=0; j<num_y_bins; j++)
	fprintf(ofile, "%f %f %f\n", 
		x_min+x_step*(float)i, y_min+y_step*(float)j,
		pdata[i*num_y_bins+j]);
  }
  else if (iferrors == HS_POS_ERRORS)
  {
    for (i=0; i<num_x_bins; i++)
      for (j=0; j<num_y_bins; j++)
	fprintf(ofile, "%f %f %f %f\n", 
		x_min+x_step*(float)i, y_min+y_step*(float)j,
		pdata[i*num_y_bins+j], perr[i*num_y_bins+j]);
  }
  else if (iferrors == HS_BOTH_ERRORS)
  {
    for (i=0; i<num_x_bins; i++)
      for (j=0; j<num_y_bins; j++)
	fprintf(ofile, "%f %f %f %f %f\n", 
		x_min+x_step*(float)i, y_min+y_step*(float)j,
		pdata[i*num_y_bins+j], perr[i*num_y_bins+j],
		nerr[i*num_y_bins+j]);
  }
  else
      assert(0);
  
  free(pdata);
  return 0;
}


int dump_3d_histo(FILE *ofile, int id)
{
  int i, j, k, ibin, num_bins, num_x_bins, num_y_bins, num_z_bins, iferrors;
  float x_min, x_max, y_min, y_max, z_min, z_max, x_step, y_step, z_step;
  float *pdata, *perr, *nerr;
  char title[256];

  hs_3d_hist_num_bins(id, &num_x_bins, &num_y_bins, &num_z_bins);
  num_bins = num_x_bins*num_y_bins*num_z_bins;
  if ((pdata = (float *)malloc(3*num_bins*sizeof(float))) == NULL)
  {
    fprintf(stderr, "dump_3d_histo: out of memory\n");
    return -1;
  }
  perr = pdata + num_bins;
  nerr = perr + num_bins;

  hs_3d_hist_range(id, &x_min, &x_max, &y_min, &y_max, &z_min, &z_max);
  x_step = (x_max-x_min)/(float)num_x_bins;
  y_step = (y_max-y_min)/(float)num_y_bins;
  z_step = (z_max-z_min)/(float)num_z_bins;
  hs_3d_hist_bin_contents(id, pdata);
  iferrors = hs_3d_hist_errors(id, perr, nerr);

  hs_title(id, title);
  fprintf(ofile, "# UID: %d\n", hs_uid(id));
  fprintf(ofile, "# Title: %s\n", title);
  hs_category(id, title);
  fprintf(ofile, "# Category: %s\n", title);
  fprintf(ofile, "# xmin = %f, xmax = %f, xbins = %d\n", x_min, x_max, num_x_bins);
  fprintf(ofile, "# ymin = %f, ymax = %f, ybins = %d\n", y_min, y_max, num_y_bins);
  fprintf(ofile, "# zmin = %f, zmax = %f, zbins = %d\n", z_min, z_max, num_z_bins);

  if (iferrors == HS_NO_ERRORS)
  {
    for (i=0; i<num_x_bins; i++)
      for (j=0; j<num_y_bins; j++)
	for (k=0; k<num_z_bins; k++)
	    fprintf(ofile, "%f %f %f %f\n", 
		    x_min+x_step*(float)i, y_min+y_step*(float)j,
		    z_min+z_step*(float)k, pdata[(i*num_y_bins+j)*num_z_bins+k]);
  }
  else if (iferrors == HS_POS_ERRORS)
  {
    for (i=0; i<num_x_bins; i++)
      for (j=0; j<num_y_bins; j++)
	for (k=0; k<num_z_bins; k++)
	{
	    ibin = (i*num_y_bins+j)*num_z_bins+k;
	    fprintf(ofile, "%f %f %f %f %f\n", 
		    x_min+x_step*(float)i, y_min+y_step*(float)j,
		    z_min+z_step*(float)k, pdata[ibin], perr[ibin]);
	}
  }
  else if (iferrors == HS_BOTH_ERRORS)
  {
    for (i=0; i<num_x_bins; i++)
      for (j=0; j<num_y_bins; j++)
	for (k=0; k<num_z_bins; k++)
	{
	    ibin = (i*num_y_bins+j)*num_z_bins+k;
	    fprintf(ofile, "%f %f %f %f %f %f\n", 
		    x_min+x_step*(float)i, y_min+y_step*(float)j,
		    z_min+z_step*(float)k, pdata[ibin], perr[ibin], nerr[ibin]);
	}
  }
  else
      assert(0);

  free(pdata);
  return 0;
}


int dump_ntuple(FILE *ofile, int id)
{
    int i, j, num_var, num_entries;
    char stringbuf[256];
    float *pdata;

    num_entries = hs_num_entries(id);
    num_var = hs_num_variables(id);
    if ((pdata = (float *)malloc(num_var*sizeof(float))) == NULL)
    {
	fprintf(stderr, "dump_ntuple: out of memory\n");
	return -1;
    }

    fprintf(ofile, "# UID: %d\n", hs_uid(id));
    hs_title(id, stringbuf);
    fprintf(ofile, "# Title: %s\n", stringbuf);
    hs_category(id, stringbuf);
    fprintf(ofile, "# Category: %s\n", stringbuf);
    fprintf(ofile, "# Number of variables: %d\n", num_var);
    fprintf(ofile, "# Variable names:\n");
    for (i=0; i<num_var; i++)
    {
	hs_variable_name(id, i, stringbuf);
	fprintf(ofile, "# %d : %s\n", i, stringbuf);
    }
    fprintf(ofile, "# Number of entries: %d\n", num_entries);
    for (i=0; i<num_entries; i++)
    {
	hs_row_contents(id, i, pdata);
        Tcl_PrintDouble(NULL, (double)pdata[0], stringbuf);
	fprintf(ofile, "%s", stringbuf);
	for (j=1; j<num_var; j++)
        {
            Tcl_PrintDouble(NULL, (double)pdata[j], stringbuf);
	    fprintf(ofile, " %s", stringbuf);
        }
	fprintf(ofile, "\n");
    }
    free(pdata);
    return 0;
}
