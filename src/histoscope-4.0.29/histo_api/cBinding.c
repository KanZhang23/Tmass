/*******************************************************************************
*									       *
* cBinding.c --  C Binding module for Histo-Scope users.		       *
*									       *
*     This module contains the following user-callable routines:	       *
*									       *
*     For all users:						               *
*									       *
*	 hs_initialize       - Initialize the Histo-Scope connection software  *
*			       and set up a potential connection to a Histo-   *
*			       Scope process.				       *
*	 hs_update	     - Updates the Histo-Scope display.		       *
*	 hs_complete	     - Closes all connections w/Histo-Scope processes. *
*	 hs_complete_and_wait- Waits until all Histo-Scopes finish scoping and *
*			       then performs an hs_complete.		       *
*	 hs_histoscope	     - Invokes Histo-Scope as a sub-process.           *
*	 hs_histo_with_config- Invokes Histo-Scope with a configuration file.  *
*	 hs_num_connected_scopes - Returns the number of connected scopes.     *
*									       *
*     For Histo-Scope item users:				               *
*									       *
*	 hs_create_1d_hist   - Books a one-dimensional histogram.	       *
*	 hs_create_2d_hist   - Books a two-dimensional histogram.	       *
*	 hs_create_ntuple    - Defines an n-tuple.  N-tuples have a specified  *
*			       number of variables and automatic storage       *
*			       allocated as they grow.			       *
*	 hs_create_indicator - Creates an indicator (a scalar value).          *
*	 hs_create_control   - Creates a control (a scalar value from H-S).    *
*	 hs_create_trigger   - Creates a trigger (an event set by HS).	       *
*	 hs_create_group     - Creates a group of data items.		       *
*	 hs_fill_1d_hist     - Adds a value to a one-dimensional histogram.    *
*	 hs_fill_2d_hist     - Adds a value to a two-dimensional histogram.    *
*	 hs_fill_ntuple      - Adds an array of real values to an n-tuple.     *
*	 hs_set_indicator    - Sets the value of an indicator.		       *
*	 hs_read_control     - Reads a control (a scalar value set by HS).     *
*	 hs_check_trigger    - Checks whether a trigger has been set by HS.    *
*	 hs_set_1d_errors    - Copies a vector of real numbers as error info   *
*	 hs_set_2d_errors    - Copies an array of real numbers as error info   *
*	 hs_reset	     - Resets all of the bins of a histogram to 0, or  *
*			       removes all of the data from an n-tuple, or     *
*			       sets an indicator to 0.			       *
*	 hs_save_file	     - Saves all current histograms, n-tuples, and     *
*			       indicators in a Histo-Scope-format file.        *
*	 hs_delete	     - Deletes a histogram, n-tuple, or indicator.     *
*	 hs_delete_items     - Deletes a list of histograms, n-tuples, etc.    *
*									       *
*     For HBOOK users:						               *
*									       *
*	 hs_hbook_setup       - Sets up all HBOOK histograms and ntuples in a  *
*			        top directory for use with Histo-Scope.	       *
*	 hs_reset_hbook_setup - Call this routine if you have previously       *
*			        called hs_hbook_setup and have booked new      *
*			        histograms or ntuples, or deleted, renamed,    *
*			        rebinned, or resetted existing ones.	       *
*									       *
* Copyright (c) 1993 Universities Research Association, Inc.		       *
* All rights reserved.							       *
* 									       *
* This material resulted from work developed under a Government Contract and   *
* is subject to the following license:  The Government retains a paid-up,      *
* nonexclusive, irrevocable worldwide license to reproduce, prepare derivative *
* works, perform publicly and display publicly by or for the Government,       *
* including the right to distribute to other Government contractors.  Neither  *
* the United States nor the United States Department of Energy, nor any of     *
* their employees, makes any warranty, express or implied, or assumes any      *
* legal liability or responsibility for the accuracy, completeness, or         *
* usefulness of any information, apparatus, product, or process disclosed, or  *
* represents that its use would not infringe privately owned rights.           *
*                                        				       *
* Fermilab Nirvana GUI Library						       *
* July 12, 1993								       *
*									       *
* Written by Joy Kyriakopulos						       *
*									       *
* Modification History:							       *
*	P. Lebrun, December 10 1993, adding uid arguments and new API routines *
*									       *
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "histoscope.h"
#include "../histo_util/hsTypes.h"
#include "../histo_util/histoUtil.h"
#include "HistoClient.h"
#include "histoApiFiles.h"

void get_config_string(const char *cfgFile, char **cfgBuf, int *strLen);

void hs_initialize(const char *id_string)
{
    histo_initialize(id_string);
}

void hs_update(void)
{
    histo_update();
}

void hs_complete(void)
{
    histo_complete();
}

void hs_complete_and_wait(void)
{
    histo_complete_and_wait();
}

#ifndef VXWORKS

void hs_histoscope(int return_immediately)
{
    histo_histoscope(return_immediately, 0, 0);
}

void hs_histoscope_hidden(void)
{
    histo_histoscope(1, 0, 1);
}

void hs_kill_histoscope(void)
{
    histo_kill_histoscope();
}

void hs_histo_with_config(int return_immediately, const char *config_file)
{
    histo_histoscope(return_immediately, config_file, 0);
}

#endif /*VXWORKS*/

int hs_num_connected_scopes(void)
{
    return histo_num_connected_scopes();
}

void hs_load_config_file(const char *cfgFile)
{
    int cfgBufLen = 0;
    char *cfgBuf = 0;
    
    get_config_string(cfgFile, &cfgBuf, &cfgBufLen);
    if (cfgBuf != 0) {
	histo_load_config(cfgBuf, cfgBufLen);
	free(cfgBuf);
    }
}

void hs_load_config_string(const char *cfgBuf)
{
    int cfgBufLen = strlen(cfgBuf);
    
    histo_load_config(cfgBuf, cfgBufLen);
}

int hs_create_1d_hist(int uid, const char *title, const char *category, const char *x_label,
			 const char *y_label, int n_bins, float min, float max)
{
    return (histo_create_1d_hist(uid, title, category, x_label, 
    			y_label, n_bins, min, max));
}

int hs_create_2d_hist(int uid, const char *title, const char *category, const char *x_label,
		        const char *y_label, const char *z_label, int x_bins, int y_bins,
		        float x_min, float x_max, float y_min, float y_max)
{
    return (histo_create_2d_hist(uid, title, category, x_label, y_label,
    			 z_label, x_bins, y_bins, x_min, x_max, y_min, y_max));
}

int hs_create_3d_hist(int uid, const char *title, const char *category,
		      const char *x_label, const char *y_label,
		      const char *z_label, const char *v_label,
		      int x_bins, int y_bins, int z_bins,
		      float x_min, float x_max, float y_min,
		      float y_max, float z_min, float z_max)
{
    return (histo_create_3d_hist(uid, title, category, x_label, y_label,
    		  z_label, v_label, x_bins, y_bins, z_bins,
                  x_min, x_max, y_min, y_max, z_min, z_max));
}

int hs_create_ntuple(int uid, const char *title, const char *category, int n_variables,
					 char **names)
{
    return (histo_create_ntuple(uid, title, category, n_variables, names));
}

int hs_create_indicator(int uid, const char *title, const char *category, float min, 
				float max)
{
    return (histo_create_indicator(uid, title, category, min, max));
}

int hs_create_control(int uid, const char *title, const char *category,
			 float min, float max, float default_value)
{
    return (histo_create_control(uid, title, category,
    			           min, max, default_value));
}

int hs_create_trigger(int uid, const char *title, const char *category)
{
    return (histo_create_trigger(uid, title, category));
}
int hs_create_group(int uid, const char *title, const char *category, int groupType,
			int numItems, int *itemId, int *errsDisp)
{
    return (histo_create_group(uid, title, category, groupType, numItems,
    			itemId, errsDisp));
}

void hs_fill_1d_hist(int id, float x, float weight)
{
    histo_fill_1d_hist(id, x, weight);
}

void hs_fill_2d_hist(int id, float x, float y, float weight)
{
    histo_fill_2d_hist(id, x, y, weight);
}

void hs_fill_3d_hist(int id, float x, float y, float z, float weight)
{
    histo_fill_3d_hist(id, x, y, z, weight);
}

void hs_1d_hist_set_bin(int id, int ix, float value)
{
    histo_1d_hist_set_bin(id, ix, value);
}

void hs_2d_hist_set_bin(int id, int ix, int iy, float value)
{
    histo_2d_hist_set_bin(id, ix, iy, value);
}

void hs_3d_hist_set_bin(int id, int ix, int iy, int iz, float value)
{
    histo_3d_hist_set_bin(id, ix, iy, iz, value);
}

void hs_1d_hist_set_bin_errors(int id, int ix,
                               float pos, float neg, int flag)
{
    histo_1d_hist_set_bin_errors(id, ix, pos, neg, flag);
}

void hs_2d_hist_set_bin_errors(int id, int ix, int iy,
                               float pos, float neg, int flag)
{
    histo_2d_hist_set_bin_errors(id, ix, iy, pos, neg, flag);
}

void hs_3d_hist_set_bin_errors(int id, int ix, int iy, int iz,
                               float pos, float neg, int flag)
{
    histo_3d_hist_set_bin_errors(id, ix, iy, iz, pos, neg, flag);
}

void hs_hist_set_slice(int id, int bin0, int stride, int count, float *data)
{
    histo_hist_set_slice(id, bin0, stride, count, data);
}

void hs_hist_set_slice_errors(int id, int bin0, int stride, int count,
                              float *err_valsP, float *err_valsM)
{
    histo_hist_set_slice_errors(id, bin0, stride, count, err_valsP, err_valsM);
}

int hs_fill_ntuple(int id, float *values)
{
    return (histo_fill_ntuple(id, values));
}

void hs_set_indicator(int id, float value)
{
    histo_set_indicator(id, value);
}

void hs_read_control(int id, float *value)
{
    histo_read_control(id, value);
}

int hs_check_trigger(int id)
{
    return (histo_check_trigger(id));
}

void hs_set_1d_errors(int id, float *err_valsP, float *err_valsM)
{
    histo_set_1d_errors(id, err_valsP, err_valsM);
}

void hs_set_2d_errors(int id, float *err_valsP, float *err_valsM)
{
    /* Note: just like HBOOK, we require the 2d array to be dimensioned
	     exactly as specified to hs_create_2d_hist: x_bins x y_bins. */
		 
    histo_set_2d_errors(id, err_valsP, err_valsM, 0); 	/* row-major order */
}

void hs_set_3d_errors(int id, float *err_valsP, float *err_valsM)
{
    histo_set_3d_errors(id, err_valsP, err_valsM, 0); 	/* row-major order */
}

void hs_reset(int id)
{
    histo_reset(id);
}

int hs_save_file(const char *name)
{
    return (histo_save_file(name));
}

void hs_delete(int id)
{
    histo_delete(id);
}

void hs_delete_items(int *ids, int num_items)
{
    histo_delete_items(ids, num_items);
}

void get_config_string(const char *cfgFile, char **cfgBuf, int *strLen)
{
    int fildes, fileLen;
    struct stat statBuf;

    fildes = open(cfgFile, O_RDONLY);
    if (fildes > 0) {
    	if (fstat(fildes, &statBuf) != 0) {
    	    printf("HS_LOAD_CONFIG_FILE - Error getting file status for %s\n",
    	    	cfgFile);
    	    perror("                    - ");
    	    *cfgBuf = 0;
    	    return;
    	}
    	fileLen = statBuf.st_size;
    	*cfgBuf = malloc(fileLen + 1);
    	if (*cfgBuf == 0) {
    	    printf("HS_LOAD_CONFIG_FILE - Config file too large: %s\n",
    	    	cfgFile);
    	    return;
    	}
    	*strLen = read(fildes, *cfgBuf, fileLen);
    	if (*strLen <= 0) {
    	    printf("HS_LOAD_CONFIG_FILE - Error reading configuration file %s\n",
    	    	cfgFile);
    	    perror("                    - ");
    	    *cfgBuf = 0;
    	    return;
    	}
    	close(fildes);
    	(*cfgBuf)[*strLen] = '\0';
    }
    else {
    	printf("HS_LOAD_CONFIG_FILE - Could not open %s\n", cfgFile);
    	perror("                    - ");
    	*cfgBuf = 0;
    }
}

#ifdef LINKED_WITH_HBOOK

void hs_hbook_setup(const char *topDirectory)
{
    histo_hbook_setup(topDirectory);
}

void hs_reset_hbook_setup(const char *topDirectory)
{
    histo_reset_hbook_setup(topDirectory);
}

#endif /*LINKED_WITH_HBOOK*/
