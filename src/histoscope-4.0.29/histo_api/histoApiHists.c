/*******************************************************************************
*									       *
* histoApiHists.c -- Application Interface routines,                           *
* to acces Histogram  properties, such #bins, bin contents  and so forth       *
*									       *
* Copyright (c) 1993, 1994 Universities Research Association, Inc.	       *
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
* December 1993 							       *
*									       *
* Written by Mark Edel, Paul Lebrun and Joy Kyriakopulos                       *
*									       *
*									       *
*******************************************************************************/
/*
* REQUIRED INCLUDE FILES
*/
#include <stdio.h>
#include <stdlib.h>
#ifdef VMS	/* ask rpc/types to refrain from declaring malloc */
#define DONT_DECLARE_MALLOC
#endif /*VMS*/  
#include <string.h>
#include <limits.h>
#include <errno.h>
#ifndef VXWORKS
#include <sys/errno.h>
#endif /*VXWORKS*/
#include <math.h>
#include "histoscope.h"
#include "../histo_util/hsTypes.h"
#include "../histo_util/histoUtil.h"
#include "HistoClient.h"
#include "histoApiHists.h"

/*
** Functions for accessing histogram data
**
** hs_1d_hist_block_fill, hs_2d_hist_block_fill, hs_1d_hist_num_bins,
** hs_2d_hist_num_bins, hs_1d_hist_range, hs_2d_hist_range,
** hs_1d_hist_bin_contents, hs_2d_hist_bin_contents, hs_1d_hist_overflows,
** hs_2d_hist_overflows, hs_1d_hist_x_value,
** hs_2d_hist_xy_value, hs_1d_hist_bin_value, hs_2d_hist_bin_value,
** hs_1d_hist_errors, hs_2d_hist_errors
** hs_1d_hist_set_overflows, hs_2d_hist_set_overflows 
*/

void histo_1d_hist_block_fill(int id, float *data, float *err, float *err_m)
/*
   Replaces all of the accumulated bin and error data in a 1D histogram and
   clears the overflow bins. The number of entries is set to the number of bins
	
	id		Histogram identifier.
	data		A one dimensional array containing the new data 
			for the histogram.  Must be of the appropriate
			size (the correct number of bins) for the 1D
			histogram being filled.
	err		A one dimensional array of data for upper error bars.
			Error values are specified as offsets from top
			of histogram bar.  Can be NULL, meaning no error
			bar data.
	err_m		A one dimensional array of data for lower error bars.
			Error values are specified as negative offsets from
			top of histogram bar (meaning they should be positive
			numbers). Can be NULL, meaning use the data from
			the upper error bars.
*/
{
    hs1DHist *h1;
    
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_1d_hist_block_fill: Invalid id: %d\n",id);
        return;
    }
      
    h1 = (hs1DHist *) GetItemByPtrID(id);
    if (h1 == NULL) {
        fprintf (stderr,
            "hs_1d_hist_block_fill: Invalid id: %d, histogram does not exist\n",
             id);
        return;
    }
    if (h1->type !=HS_1D_HISTOGRAM) {
        fprintf (stderr,
        "hs_1d_hist_block_fill: Item (id = %d) is not a 1-dim histogram\n", id);
        return;
    }
    
    memcpy(h1->bins, data, h1->nBins*sizeof(float));
    h1->count = h1->nBins;
    h1->overflow = 0.f;
    h1->underflow = 0.f;
    if (err != NULL) {
        h1->errFlg = 1; 
        if (h1->pErrs == NULL)
            h1->pErrs = (float *) malloc((h1->nBins)*sizeof(float));
	memcpy(h1->pErrs, err, h1->nBins*sizeof(float));
    }

    if (err_m != NULL) {
        if (err == NULL) {
            fprintf (stderr,
		     "hs_1d_hist_block_fill: Input Error: "
		     "specify Negative errors only when also\n");
            fprintf (stderr,
              "   specifying positive errors. No errors defined for id = %d.\n",
               h1->id);
            return;
        }   
        if (h1->mErrs == NULL)
            h1->mErrs = (float *) malloc((h1->nBins)*sizeof(float));
	memcpy(h1->mErrs, err_m, h1->nBins*sizeof(float));
    }

    SetHistResetFlag((hsGeneral *)h1);
    return;
}
 
void histo_2d_hist_block_fill(int id, float *data,
                              float *err, float *err_m, int bindingflag)
/*
   Replaces all of the accumulated bin and error data in a 2D histogram and
   clears the overflow bins. 
	
	id		Histogram identifier.
	data		A two dimensional array containing the new data 
			for the histogram.  Must be of the appropriate
			size (the correct number of bins) for the 2D
			histogram being filled.
	err		A two dimensional array of data for upper error bars.
			Error values are specified as offsets from top
			of histogram bar.  Can be NULL, meaning no error
			bar data.
	err_m		A two dimensional array of data for lower error bars.
			Error values are specified as negative offsets from
			top of histogram bar (meaning they should be positive
			numbers). Can be NULL, meaning use the data from
			the upper error bars.
	bindingflag     0 if c binding, 1 if Fortran, in such case one has to 
			invert ( row vs colum) the array.
*/
{
    int i, j, nb2;
    hs2DHist *h2;
    
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_2d_hist_block_fill: Invalid id: %d\n",id);
        return;
    }
      
    h2 = (hs2DHist *) GetItemByPtrID(id);
    if (h2 == NULL) {
        fprintf (stderr,
            "hs_2d_hist_block_fill: Invalid id: %d, histogram does not exist\n",
             id);
        return;
    }
    if (h2->type !=HS_2D_HISTOGRAM) {
        fprintf (stderr,
        "hs_2d_hist_block_fill: Item (id = %d) is not a 2-dim histogram\n", id);
        return;
    }
    
    nb2 = (h2->nXBins) * (h2->nYBins);
    h2->count = nb2;
    if (bindingflag == 0) { 
	memcpy(h2->bins, data, nb2*sizeof(float));
    } else {
	for (i = 0; i < h2->nXBins; ++i)
	    for (j = 0; j < h2->nYBins; ++j) 
		h2->bins[i*(h2->nYBins) +j] = data[j*(h2->nXBins) +i];
    } 

    for (i=0; i<3; i++) { 
	for (j=0; j<3; j++) {
	    h2->overflow[i][j] = 0.f;
        }
    }

    if (err != NULL) {
	h2->errFlg = 1; 
	if (h2->pErrs == NULL)
	    h2->pErrs = (float *) malloc(nb2*sizeof(float));
	if (bindingflag == 0) {
	    memcpy(h2->pErrs, err, nb2*sizeof(float));
        } else {
	    for (i = 0; i < h2->nXBins; ++i)
		for (j = 0; j < h2->nYBins; ++j) 
		    h2->pErrs[i*h2->nYBins+j] = err[j*h2->nXBins+i];
	}
    }

    if (err_m != NULL) {
        if (err == NULL) {
            fprintf (stderr,
		     "hs_2d_hist_block_fill: Input Error: specify "
		     "Negative errors only when also\n");
            fprintf (stderr,
              "   specifying positive errors. No errors defined for id = %d.\n",
               h2->id);
            return;
        }   
        if (h2->mErrs == NULL)
            h2->mErrs = (float *) malloc(nb2*sizeof(float));
        if (bindingflag == 0) { 
	    memcpy(h2->mErrs, err_m, nb2*sizeof(float));
        } else {
	    for (i = 0; i < h2->nXBins; ++i)
	        for (j = 0; j < h2->nYBins; ++j) 
	            h2->mErrs[i*h2->nYBins+j] = err_m[j*h2->nXBins+i];
        } 
    }

    SetHistResetFlag((hsGeneral *)h2);
    return;
}

void histo_3d_hist_block_fill(int id, float *data,
                              float *err, float *err_m, int bindingflag)
/*
   Replaces all of the accumulated bin and error data in a 3D histogram and
   clears the overflow bins. 
	
	id		Histogram identifier.
	data		A two dimensional array containing the new data 
			for the histogram.  Must be of the appropriate
			size (the correct number of bins) for the 3D
			histogram being filled.
	err		A two dimensional array of data for upper error bars.
			Error values are specified as offsets from top
			of histogram bar.  Can be NULL, meaning no error
			bar data.
	err_m		A two dimensional array of data for lower error bars.
			Error values are specified as negative offsets from
			top of histogram bar (meaning they should be positive
			numbers). Can be NULL, meaning use the data from
			the upper error bars.
	bindingflag     0 if c binding, 1 if Fortran, in such case one has to 
			invert ( row vs colum) the array.
*/
{
    int i, j, k, nb3;
    hs3DHist *h3;
    
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_3d_hist_block_fill: Invalid id: %d\n",id);
        return;
    }
      
    h3 = (hs3DHist *) GetItemByPtrID(id);
    if (h3 == NULL) {
        fprintf (stderr,
            "hs_3d_hist_block_fill: Invalid id: %d, histogram does not exist\n",
             id);
        return;
    }
    if (h3->type != HS_3D_HISTOGRAM) {
        fprintf (stderr,
        "hs_3d_hist_block_fill: Item (id = %d) is not a 3-dim histogram\n", id);
        return;
    }

    nb3 = (h3->nXBins) * (h3->nYBins) * (h3->nZBins);
    if (bindingflag == 0) {
	memcpy(h3->bins, data, nb3*sizeof(float));
    } else {
	for (i = 0; i < h3->nXBins; ++i)
	    for (j = 0; j < h3->nYBins; ++j) 
		for (k=0; k < h3->nZBins; ++k)
		    h3->bins[i*h3->nYBins*h3->nZBins + j*h3->nZBins + k] =
			data[k*h3->nYBins*h3->nXBins + j*h3->nXBins + i];
    } 

    h3->count = nb3;
    for (i=0; i<3; i++) { 
	for (j=0; j<3; j++) {
	    for (k=0; k<3; k++) {
		h3->overflow[i][j][k] = 0.f;
	    }
	}
    }

    if (err != NULL) {
	h3->errFlg = 1; 
	if (h3->pErrs == NULL)
	    h3->pErrs = (float *) malloc(nb3*sizeof(float));
	if (bindingflag == 0) { 
	    memcpy(h3->pErrs, err, nb3*sizeof(float));
        } else {
	    for (i = 0; i < h3->nXBins; ++i)
		for (j = 0; j < h3->nYBins; ++j) 
		    for (k=0; k < h3->nZBins; ++k)
			h3->pErrs[i*h3->nYBins*h3->nZBins + j*h3->nZBins + k] =
			    err[k*h3->nYBins*h3->nXBins + j*h3->nXBins + i];
	} 
    }

    if (err_m != NULL) {
        if(err == NULL) {
            fprintf (stderr,
		     "hs_3d_hist_block_fill: Input Error: specify "
		     "Negative errors only when also\n");
            fprintf (stderr,
		     "   specifying positive errors. No errors defined for id = %d.\n",
		     h3->id);
            return;
        }   
        if (h3->mErrs == NULL)
            h3->mErrs = (float *) malloc(nb3*sizeof(float));
        if (bindingflag == 0) { 
	    memcpy(h3->mErrs, err_m, nb3*sizeof(float));
        } else {
	    for (i = 0; i < h3->nXBins; ++i)
	        for (j = 0; j < h3->nYBins; ++j) 
		    for (k=0; k < h3->nZBins; ++k)
			h3->mErrs[i*h3->nYBins*h3->nZBins + j*h3->nZBins + k] =
			    err_m[k*h3->nYBins*h3->nXBins + j*h3->nXBins + i];
       } 
    }

    SetHistResetFlag((hsGeneral *)h3);
    return;
}
     
int histo_1d_hist_num_bins(int id)
/*
   Return the number of bins in a 1D histogram
   
	id		HistoScope id, as returned by create routines
	
	Return Value:	The number of bins; or -1 if the id did not exist
			or does not refer to a 2D dimensional histogram.
*/
{
    hs1DHist *h1;
    
    if (!InitializedAndActive)
    	return -1;			/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_1d_hist_num_bins: Invalid id: %d\n",id);
        return -1;
    }
      
    h1 = (hs1DHist *) GetItemByPtrID(id);
    if (h1 == NULL) {
        fprintf (stderr,
            "hs_1d_hist_num_bins: Invalid id: %d, histogram does not exist\n",
             id);
        return -1;
    }
    if (h1->type !=HS_1D_HISTOGRAM) {
        fprintf (stderr,
          "hs_1d_hist_num_bins: Item (id = %d) is not a 1-dim histogram\n", id);
        return -1;
    }
    
    return h1->nBins;
}
    

void histo_2d_hist_num_bins(int id, int *num_x_bins, int *num_y_bins)
/*
   Return the number of bins in a 2D histogram
   
	id		HistoScope id, as returned by create routines
   	num_x_bins	Returns number of bins in X; or -1 if invalid id
   	num_y_bins	Returns number of bins in Y; or -1 if invalid id
	
*/
{
    hs2DHist *h2;
    
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_2d_hist_num_bins: Invalid id: %d\n",id);
        *num_x_bins = -1;
        *num_y_bins = -1;
        return;
    }
      
    h2 = (hs2DHist *) GetItemByPtrID(id);
    if (h2 == NULL) {
        fprintf (stderr,
            "hs_2d_hist_num_bins: Invalid id: %d, histogram does not exist\n",
             id);
        *num_x_bins = -1;
        *num_y_bins = -1;
        return;
    }
    if (h2->type !=HS_2D_HISTOGRAM) {
        fprintf (stderr,
          "hs_2d_hist_num_bins: Item (id = %d) is not a 2-dim histogram\n", id);
        *num_x_bins = -1;
        *num_y_bins = -1;
        return;
    }
    
    *num_x_bins = h2->nXBins;
    *num_y_bins = h2->nYBins;
    return;
}

void histo_3d_hist_num_bins(int id, int *num_x_bins,
			    int *num_y_bins, int *num_z_bins)
/*
   Return the number of bins in a 3D histogram
   
	id		HistoScope id, as returned by create routines
   	num_x_bins	Returns number of bins in X; or -1 if invalid id
   	num_y_bins	Returns number of bins in Y; or -1 if invalid id
   	num_z_bins	Returns number of bins in Z; or -1 if invalid id
	
*/
{
    hs3DHist *h3;
    
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_3d_hist_num_bins: Invalid id: %d\n",id);
        *num_x_bins = -1;
        *num_y_bins = -1;
        *num_z_bins = -1;
        return;
    }
      
    h3 = (hs3DHist *) GetItemByPtrID(id);
    if (h3 == NULL) {
        fprintf (stderr,
            "hs_3d_hist_num_bins: Invalid id: %d, histogram does not exist\n",
             id);
        *num_x_bins = -1;
        *num_y_bins = -1;
        *num_z_bins = -1;
        return;
    }
    if (h3->type != HS_3D_HISTOGRAM) {
        fprintf (stderr,
          "hs_3d_hist_num_bins: Item (id = %d) is not a 3-dim histogram\n", id);
        *num_x_bins = -1;
        *num_y_bins = -1;
        *num_z_bins = -1;
        return;
    }

    *num_x_bins = h3->nXBins;
    *num_y_bins = h3->nYBins;
    *num_z_bins = h3->nZBins;
    return;
}

void histo_1d_hist_range(int id, float *min, float *max)
/*
   Return the minimum and maximum values representing the horizontal limits
   of a one dimensional histogram.
   
	id		HistoScope id, as returned by create routines
	min		Returns low edge of first bin; or 0 if invalid id
	max		Returns high edge of last bin; or 0 if invalid id
*/	
{
    hs1DHist *h1;
    
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_1d_hist_range: Invalid id: %d\n",id);
        *min = 0.;
        *max = 0.;
        return;
    }
      
    h1 = (hs1DHist *) GetItemByPtrID(id);
    if (h1 == NULL) {
        fprintf (stderr, 
            "hs_1d_hist_range: Invalid id: %d, histogram does not exist\n", id);
        *min = 0.;
        *max = 0.;
        return;
    }
    if (h1->type !=HS_1D_HISTOGRAM) {
        fprintf (stderr,
            "hs_1d_hist_range: Item (id = %d) is not a 1-dim histogram\n", id);
        *min = 0.;
        *max = 0.;
        return;
    }
    
    *min = h1->min;
    *max = h1->max;
    return;
}

void histo_2d_hist_range(int id, float *x_min, float *x_max, float *y_min,
	float *y_max)
/*
   Return the minimum and maximum values representing the horizontal limits
   of a 2 dimensional histogram.
   
	id		HistoScope id, as returned by create routines
	x_min		Returns low edge of first bin along x
	x_max		Returns high edge of last bin along x
	y_min		Returns low edge of first bin along y
	y_max		Returns high edge of last bin along y
*/	
{
    hs2DHist *h2;
    
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_2d_hist_range: Invalid id: %d\n",id);
        goto cleanerror;
    }
      
    h2 = (hs2DHist *) GetItemByPtrID(id);
    if (h2 == NULL) {
        fprintf (stderr,
            "hs_2d_hist_range: Invalid id: %d, histogram does not exist\n", id);
        goto cleanerror;
    }
    if (h2->type !=HS_2D_HISTOGRAM) {
        fprintf (stderr,
            "hs_2d_hist_range: Item (id = %d) is not a 2-dim histogram", id);
        goto cleanerror;
    }
    
    *x_min = h2->xMin;
    *x_max = h2->xMax;
    *y_min = h2->yMin;
    *y_max = h2->yMax;
    return;
    
 cleanerror: 
    *x_min = 0.f;
    *x_max = 0.f;
    *y_min = 0.f;
    *y_max = 0.f;
    return;
    
}

void histo_3d_hist_range(int id, float *x_min, float *x_max, float *y_min,
			 float *y_max, float *z_min, float *z_max)
/*
   Return the minimum and maximum values representing the horizontal limits
   of a 3 dimensional histogram.
*/	
{
    hs3DHist *h3;
    
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_3d_hist_range: Invalid id: %d\n",id);
        goto cleanerror;
    }
      
    h3 = (hs3DHist *) GetItemByPtrID(id);
    if (h3 == NULL) {
        fprintf (stderr,
            "hs_3d_hist_range: Invalid id: %d, histogram does not exist\n", id);
        goto cleanerror;
    }
    if (h3->type !=HS_3D_HISTOGRAM) {
        fprintf (stderr,
            "hs_3d_hist_range: Item (id = %d) is not a 3-dim histogram", id);
        goto cleanerror;
    }
    
    *x_min = h3->xMin;
    *x_max = h3->xMax;
    *y_min = h3->yMin;
    *y_max = h3->yMax;
    *z_min = h3->zMin;
    *z_max = h3->zMax;
    return;
    
 cleanerror: 
    *x_min = 0.f;
    *x_max = 0.f;
    *y_min = 0.f;
    *y_max = 0.f;
    *z_min = 0.f;
    *z_max = 0.f;
    return;
    
}

void histo_1d_hist_bin_contents(int id, float *data)
/*
   Return the bin data from a one dimensional histogram
   
	id		HistoScope id, as returned by create routines
	data		A one dimensional array to receive the bin contents.
			Must be dimensioned appropriatly for the number
			of bins in the histogram.
*/		
{
    hs1DHist *h1;
    
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr,
            "hs_1d_hist_bin_contents: Invalid id: %d\n",id);
        return;
    }
      
    h1 = (hs1DHist *) GetItemByPtrID(id);
    if (h1 == NULL) {
        fprintf (stderr,
          "hs_1d_hist_bin_contents: Invalid id: %d, histogram does not exist\n",
              id);
        return;
    }
    if (h1->type !=HS_1D_HISTOGRAM) {
        fprintf (stderr,
      "hs_1d_hist_bin_contents: Item (id = %d) is not a 1-dim histogram\n", id);
        return;
    }
    memcpy(data, h1->bins, h1->nBins*sizeof(float));
    return;
}

void histo_2d_hist_bin_contents(int id, float *data, int bindingflag)
/*
   Return the bin data from a 2 dimensional histogram
   
	id		HistoScope id, as returned by create routines
	data		A 2 dimensional array to receive the bin contents.
			Must be dimensioned appropriatly for the number
			of bins in the histogram.
*/
{
    int i, j, nb2;
    hs2DHist *h2;
    
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_2d_hist_bin_contents: Invalid id: %d\n",id);
        return;
    }
      
    h2 = (hs2DHist *) GetItemByPtrID(id);
    if (h2 == NULL) {
        fprintf (stderr,
          "hs_2d_hist_bin_contents: Invalid id: %d, histogram does not exist\n",
              id);
        return;
    }
    if (h2->type !=HS_2D_HISTOGRAM) {
        fprintf (stderr,
      "hs_2d_hist_bin_contents: Item (id = %d) is not a 2-dim histogram\n", id);
        return;
    }
    
    nb2 = (h2->nXBins) * (h2->nYBins);
    if (bindingflag == 0) { 
	memcpy(data, h2->bins, nb2*sizeof(float));
    } else {
	register float *pfrom = h2->bins;
	const int stride = h2->nXBins;
	for (i = 0; i < h2->nXBins; ++i)
	{
	    register float *pto = data + i;
	    for (j = h2->nYBins; j > 0; --j)
	    {
		*pto = *pfrom++;
		pto += stride;
	    }
	}
    }

    return;
}

void histo_3d_hist_bin_contents(int id, float *data, int bindingflag)
/*
   Return the bin data from a 3 dimensional histogram
   
	id		HistoScope id, as returned by create routines
	data		A 3 dimensional array to receive the bin contents.
			Must be dimensioned appropriatly for the number
			of bins in the histogram.
*/
{
    int i, j, k, nb3;
    hs3DHist *h3;
    
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_3d_hist_bin_contents: Invalid id: %d\n",id);
        return;
    }
      
    h3 = (hs3DHist *) GetItemByPtrID(id);
    if (h3 == NULL) {
        fprintf (stderr,
          "hs_3d_hist_bin_contents: Invalid id: %d, histogram does not exist\n",
              id);
        return;
    }
    if (h3->type != HS_3D_HISTOGRAM) {
        fprintf (stderr,
	"hs_3d_hist_bin_contents: Item (id = %d) is not a 3-dim histogram\n", id);
        return;
    }
    
    nb3 = (h3->nXBins) * (h3->nYBins) * (h3->nZBins);
    if (bindingflag == 0) { 
	memcpy(data, h3->bins, nb3*sizeof(float));
    } else {
	register float *pfrom = h3->bins;
	const int stride = h3->nYBins*h3->nXBins;
	for (i = 0; i < h3->nXBins; ++i)
	    for (j = 0; j < h3->nYBins; ++j)
	    {
		register float *pto = data + j*h3->nXBins + i;
		for (k = h3->nZBins; k > 0; --k)
		{
		    *pto = *pfrom++;
		    pto += stride;
		}
	    }
    }

    return;
}

int histo_1d_hist_errors(int id, float *err, float *err_m)
/*
   Return the error bar data from a one dimensional histogram
   
   	id		HistoScope id, as returned by create routines
   	err		A one dimensional array to receive data for upper error
   			bars.  Must be dimensioned appropriatly for the number
			of bins in the histogram.
	err_m		A one dimensional array to receive data for lower error
			bars.  Can be set to NULL pointer, in such a case 
			only positiv errors are returned. If non NULL,
			Must be dimensioned appropriatly for the number
			of bins in the histogram.
			
	returns		HS_ITEMNOTFOUND_ERRORS - invalid id or histogram 
						 doesn't exist
			HS_NO_ERRORS   - item has no error data stored
			HS_POS_ERRORS  - positive errors (only) returned
			HS_BOTH_ERRORS - positive and negative errors returned
*/
{
    hs1DHist *h1;
    
    if (!InitializedAndActive)
    	return HS_ITEMNOTFOUND_ERRORS;	/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_1d_hist_errors: Invalid id: %d \n",id);
       return HS_ITEMNOTFOUND_ERRORS;
    }
      
    h1 = (hs1DHist *) GetItemByPtrID(id);
    if (h1 == NULL) {
        fprintf (stderr,
           "hs_1d_hist_errors: Invalid id: %d, histogram does not exist\n", id);
        return HS_ITEMNOTFOUND_ERRORS;
    }
    if (h1->type !=HS_1D_HISTOGRAM) {
        fprintf (stderr,
            "hs_1d_hist_errors: Item (id = %d) is not a 1-dim histogram\n", id);
        return HS_ITEMNOTFOUND_ERRORS;
    }
    
    if ((h1->errFlg == 0) || h1->pErrs == NULL) { 
        /* fprintf (stderr,
           "hs_1d_hist_errors: Item (id = %d) has NO errors stored\n", id); */
        return HS_NO_ERRORS;
    }

    memcpy(err, h1->pErrs, h1->nBins*sizeof(float));
    if ((h1->mErrs == NULL) ||  (err_m == NULL)) 
    	return HS_POS_ERRORS;
    memcpy(err_m, h1->mErrs, h1->nBins*sizeof(float));
    return HS_BOTH_ERRORS;
}

int histo_2d_hist_errors(int id, float *err, float *err_m, 
					      int bindingflag)
/*
   Return the error bar data from a two dimensional histogram
   
   	id		HistoScope id, as returned by create routines
   	err		A two dimensional array to receive data for upper error
   			bars.  Must be dimensioned appropriatly for the number
			of bins in the histogram.
	err_m		A two dimensional array to receive data for lower error
			bars.  Must be dimensioned appropriatly for the number
			of bins in the histogram.
			
	returns		HS_ITEMNOTFOUND_ERRORS - invalid id or histogram 
						 doesn't exist
			HS_NO_ERRORS   - item has no error data stored
			HS_POS_ERRORS  - positive errors (only) returned
			HS_BOTH_ERRORS - positive and negative errors returned
*/
{
    int i, j, nb2;
    hs2DHist *h2;
    
    if (!InitializedAndActive)
    	return HS_ITEMNOTFOUND_ERRORS;	/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_2d_hist_errors: Invalid id: %d\n",id);
        return HS_ITEMNOTFOUND_ERRORS;
    }
      
    h2 = (hs2DHist *) GetItemByPtrID(id);
    if (h2 == NULL) {
        fprintf (stderr,
           "hs_2d_hist_errors: Invalid id: %d, histogram does not exist\n", id);
        return HS_ITEMNOTFOUND_ERRORS;
    }
    if (h2->type !=HS_2D_HISTOGRAM) {
        fprintf (stderr,
            "hs_2d_hist_errors: Item (id = %d) is not a 2-dim histogram\n", id);
        return HS_ITEMNOTFOUND_ERRORS;
    }
    
    if ((h2->errFlg == 0) || h2->pErrs == NULL) { 
        /* fprintf (stderr,
           "hs_2d_hist_errors: Item (id = %d) has NO errors stored\n", id); */
        return HS_NO_ERRORS;
    }
    nb2 = (h2->nXBins) * (h2->nYBins);
    if (bindingflag == 0) {
	memcpy(err, h2->pErrs, nb2*sizeof(float));
    } else {
	for (i = 0; i < h2->nXBins; ++i)
	    for (j = 0; j < h2->nYBins; ++j) 
	        err[j*(h2->nXBins) +i] = h2->pErrs[i*(h2->nYBins) +j];
    }
     
    if ((h2->mErrs == NULL) ||  (err_m == NULL)) 
    	return HS_POS_ERRORS;
    if (bindingflag == 0) {
	memcpy(err_m, h2->mErrs, nb2*sizeof(float));
    } else {
	for (i = 0; i < h2->nXBins; ++i)
	    for (j = 0; j < h2->nYBins; ++j) 
	        err_m[j*(h2->nXBins) +i] = h2->mErrs[i*(h2->nYBins) +j];
    }

    return HS_BOTH_ERRORS;
}

int histo_3d_hist_errors(int id, float *err, float *err_m, 
					      int bindingflag)
/*
   Return the error bar data from a three dimensional histogram
*/
{
    int i, j, k, nb3;
    hs3DHist *h3;
    
    if (!InitializedAndActive)
    	return HS_ITEMNOTFOUND_ERRORS;	/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_3d_hist_errors: Invalid id: %d\n",id);
        return HS_ITEMNOTFOUND_ERRORS;
    }
      
    h3 = (hs3DHist *) GetItemByPtrID(id);
    if (h3 == NULL) {
        fprintf (stderr,
           "hs_3d_hist_errors: Invalid id: %d, histogram does not exist\n", id);
        return HS_ITEMNOTFOUND_ERRORS;
    }
    if (h3->type !=HS_3D_HISTOGRAM) {
        fprintf (stderr,
            "hs_3d_hist_errors: Item (id = %d) is not a 3-dim histogram\n", id);
        return HS_ITEMNOTFOUND_ERRORS;
    }
    
    if ((h3->errFlg == 0) || h3->pErrs == NULL) { 
        /* fprintf (stderr,
           "hs_3d_hist_errors: Item (id = %d) has NO errors stored\n", id); */
        return HS_NO_ERRORS;
    }
    nb3 = (h3->nXBins) * (h3->nYBins) * (h3->nZBins);
    if (bindingflag == 0) {
	memcpy(err, h3->pErrs, nb3*sizeof(float));
    } else {
	for (i = 0; i < h3->nXBins; ++i)
	    for (j = 0; j < h3->nYBins; ++j)
		for (k = 0; k < h3->nZBins; ++k)
		    err[k*h3->nYBins*h3->nXBins + j*h3->nXBins + i] = 
			h3->pErrs[i*(h3->nYBins)*(h3->nZBins) + j*h3->nZBins + k];
    }
     
    if ((h3->mErrs == NULL) || (err_m == NULL)) 
    	return HS_POS_ERRORS;
    if (bindingflag == 0) {
	memcpy(err_m, h3->mErrs,  nb3*sizeof(float));
    } else {
	for (i = 0; i < h3->nXBins; ++i)
	    for (j = 0; j < h3->nYBins; ++j) 
		for (k = 0; k < h3->nZBins; ++k)
		    err_m[k*h3->nYBins*h3->nXBins + j*h3->nXBins + i] = 
			h3->mErrs[i*(h3->nYBins)*(h3->nZBins) + j*h3->nZBins + k];
    }
     
    return HS_BOTH_ERRORS;
}

void histo_1d_hist_overflows(int id, float *underflow, float *overflow)
/*
   Return the overflow data from a one dimensional histogram
   
   	id		HistoScope id, as returned by create routines
   	underflow	Returns underflow data from the histogram.
	overflow	Returns overflow data from the histogram.
*/
{
    hs1DHist *h1;
    
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_1d_hist_overflows: Invalid id: %d\n",id);
        return;
    }
      
    h1 = (hs1DHist *) GetItemByPtrID(id);
    if (h1 == NULL) {
        fprintf (stderr,
            "hs_1d_hist_overflows: Invalid id: %d, histogram does not exist\n",
             id);
        return;
    }
    if (h1->type !=HS_1D_HISTOGRAM) {
        fprintf (stderr,
         "hs_1d_hist_overflows: Item (id = %d) is not a 1-dim histogram\n", id);
        return;
    }
    
    *underflow = h1->underflow;
    *overflow = h1->overflow;
    return;
}

void histo_2d_hist_overflows(int id, float overflows[3][3], int bindingflag)
/*
   Return the overflow data from a two dimensional histogram
   
   	id		HistoScope id, as returned by create routines
	overflows	A 3 x 3 array of floats to receive overflow data
			  for the histogram.
   	bindingflag	== 1 for FORTRAN;      == 0 for C
*/
{
    int i, j;
    hs2DHist *h2;
    
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_2d_hist_overflows: Invalid id: %d\n",id);
        return;
    }
      
    h2 = (hs2DHist *) GetItemByPtrID(id);
    if (h2 == NULL) {
        fprintf (stderr,
            "hs_2d_hist_overflows: Invalid id: %d, histogram does not exist\n",
             id);
        return;
    }
    if (h2->type !=HS_2D_HISTOGRAM) {
        fprintf (stderr,
         "hs_2d_hist_overflows: Item (id = %d) is not a 2-dim histogram\n", id);
        return;
    }
    
    if (bindingflag == 0) {		/* C Binding - row order */ 
        for (i=0; i < 3; i++)
            for (j=0; j < 3; j++) 
                overflows[i][j] = h2->overflow[i][j];
    } else {
        for (i=0; i < 3; i++)		/* FORTRAN Binding - column order */
            for (j=0; j < 3; j++) 
                overflows[j][i] = h2->overflow[i][j];
    }
     
    return;
}

void histo_3d_hist_overflows(int id, float overflows[3][3][3], int bindingflag)
/*
   Return the overflow data from a three dimensional histogram
*/
{
    int i, j, k;
    hs3DHist *h3;
    
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_3d_hist_overflows: Invalid id: %d\n",id);
        return;
    }
      
    h3 = (hs3DHist *) GetItemByPtrID(id);
    if (h3 == NULL) {
        fprintf (stderr,
            "hs_3d_hist_overflows: Invalid id: %d, histogram does not exist\n",
             id);
        return;
    }
    if (h3->type !=HS_3D_HISTOGRAM) {
        fprintf (stderr,
         "hs_3d_hist_overflows: Item (id = %d) is not a 3-dim histogram\n", id);
        return;
    }
    
    if (bindingflag == 0) {		/* C Binding - row order */ 
        for (i=0; i < 3; i++)
            for (j=0; j < 3; j++) 
		for (k=0; k < 3; k++) 
		    overflows[i][j][k] = h3->overflow[i][j][k];
    } else {
        for (i=0; i < 3; i++)		/* FORTRAN Binding - column order */
            for (j=0; j < 3; j++) 
		for (k=0; k < 3; k++) 
		    overflows[k][j][i] = h3->overflow[i][j][k];
    }

    return;
}

void histo_1d_hist_set_overflows(int id, float underflow, float overflow)
/*
   Set the overflow data from a one dimensional histogram.  To be used for
  various translators, and other such rare and highly specific cases.
   
   	id		HistoScope id, as returned by create routines
   	underflow	Underflow data to set for the histogram.
	overflow	Overflow data to set for the histogram.
*/
{
    hs1DHist *h1;
    
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_1d_hist_set_overflows: Invalid id: %d\n",id);
        return;
    }
      
    h1 = (hs1DHist *) GetItemByPtrID(id);
    if (h1 == NULL) {
        fprintf (stderr,
        "hs_1d_hist_set_overflows: Invalid id: %d, histogram does not exist\n",
             id);
        return;
    }
    if (h1->type !=HS_1D_HISTOGRAM) {
        fprintf (stderr,
    "hs_1d_hist_set_overflows: Item (id = %d) is not a 1-dim histogram\n", id);
        return;
    }
    
    h1->underflow = underflow;
    h1->overflow = overflow;
    return;
}

void histo_2d_hist_set_overflows(int id,
                          float overflows[3][3], int bindingflag)
/*
   Set the overflow data from a two dimensional histogram
   
   	id		HistoScope id, as returned by create routines
	overflows	A 3 x 3 array of floats to receive overflow data
			  for the histogram.
   	bindingflag	== 1 for FORTRAN;      == 0 for C
*/
{
    int i, j;
    hs2DHist *h2;
    
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_2d_hist_set_overflows: Invalid id: %d\n",id);
        return;
    }
      
    h2 = (hs2DHist *) GetItemByPtrID(id);
    if (h2 == NULL) {
        fprintf (stderr,
            "hs_2d_hist_set_overflows: Invalid id: %d, histogram does not exist\n",
             id);
        return;
    }
    if (h2->type !=HS_2D_HISTOGRAM) {
        fprintf (stderr,
         "hs_2d_hist_set_overflows: Item (id = %d) is not a 2-dim histogram\n", id);
        return;
    }
    
    if (bindingflag == 0) {		/* C Binding - row order */ 
        for (i=0; i < 3; i++)
            for (j=0; j < 3; j++) 
                h2->overflow[i][j] = overflows[i][j];
    } else {
        for (i=0; i < 3; i++)		/* FORTRAN Binding - column order */
            for (j=0; j < 3; j++) 
               h2->overflow[i][j] = overflows[j][i];
    }
     
    return;
}

void histo_3d_hist_set_overflows(int id,
                          float overflows[3][3][3], int bindingflag)
/*
   Set the overflow data for a three dimensional histogram
   */
{
    int i, j, k;
    hs3DHist *h3;
    
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_3d_hist_set_overflows: Invalid id: %d\n",id);
        return;
    }
      
    h3 = (hs3DHist *) GetItemByPtrID(id);
    if (h3 == NULL) {
        fprintf (stderr,
            "hs_3d_hist_set_overflows: Invalid id: %d, histogram does not exist\n",
             id);
        return;
    }
    if (h3->type !=HS_3D_HISTOGRAM) {
        fprintf (stderr,
         "hs_3d_hist_set_overflows: Item (id = %d) is not a 3-dim histogram\n", id);
        return;
    }
    
    if (bindingflag == 0) {		/* C Binding - row order */ 
        for (i=0; i < 3; i++)
            for (j=0; j < 3; j++) 
		for (k=0; k < 3; k++)
		    h3->overflow[i][j][k] = overflows[i][j][k];
    } else {
        for (i=0; i < 3; i++)		/* FORTRAN Binding - column order */
            for (j=0; j < 3; j++) 
		for (k=0; k < 3; k++) 
		    h3->overflow[i][j][k] = overflows[k][j][i];
    }
     
    return;
}

int histo_num_entries(int id)
/*
   Returns the number of fill operations that have been performed on the
   histogram referred to by id.
   
   	id		HistoScope id, as returned by create routines

   	Return Value:	The number of fills or entries, or -1 if id was not 
   			found or not a 1D or 2D histogram, or an Ntuple
*/
{
    hs1DHist *h1;
    hs2DHist *h2;
    hs3DHist *h3;
    hsNTuple *hnt;
    hsGeneral *h;
    
    if (!InitializedAndActive)
    	return -1;				/* hs_initialize must be called first */
    	
    if (id <= 0) { 
        fprintf (stderr, "hs_num_entries: Invalid id: %d\n",id);
        return -1;
    }
      
    h = (hsGeneral *) GetItemByPtrID(id);
    if (h == NULL) {
        fprintf (stderr,
            "hs_num_entries: Invalid id: %d, item does not exist\n", id);
        return -1;
    }
    
    switch(h->type) {
     case HS_1D_HISTOGRAM:
        h1 = (hs1DHist *) h;
        return h1->count;
     case HS_2D_HISTOGRAM:
        h2 = (hs2DHist *) h;
        return h2->count;
     case HS_3D_HISTOGRAM:
        h3 = (hs3DHist *) h;
        return h3->count;
     case HS_NTUPLE:
        hnt = (hsNTuple *) h;
        return hnt->n;
     default:
      fprintf (stderr,
     "hs_num_entries: Invalid item type, id %d is not a histogram or NTuple \n",
           id);
      return -1;
    }
}
       
float histo_1d_hist_x_value(int id, float x)
/*
   Returns the value in the histogram bin that would be filled by the 
   value x.  The result is undefined if id is not a one dimensional
   histogram, or if x is not within the range of the histogram.
   
   	id		HistoScope id, as returned by create routines
   	x		The x coordinate to the bin whose value the function
   			should return.
   	
   	Return Value:	The value stored in the histogram bin referenced by x.
*/
{
    hs1DHist *h1;
    int bin;
    
    if (id <= 0) {
        fprintf (stderr, "hs_1d_hist_x_value: Invalid id: %d\n",id);
        return 0.;
    }
      
    h1 = (hs1DHist *) GetItemByPtrID(id);
    if (h1 == NULL) {
        fprintf (stderr,
            "hs_1d_hist_x_value: Invalid id: %d, histogram does not exist\n",
             id);
        return 0.;
    }
    if (h1->type !=HS_1D_HISTOGRAM) {
        fprintf (stderr,
           "hs_1d_hist_x_value: Item (id = %d) is not a 1-dim histogram\n", id);
        return 0.;
    }
    
    if ((x < h1->min) || (x > h1->max)) {
        fprintf (stderr, "hs_1d_hist_x_value: specified X coordinate (%f)", x);
        fprintf (stderr, " for Histogram (id = %d)\n     is out of range\n",id);
        return 0.;
    }
    bin = ((x - h1->min) * (float) h1->nBins) / (h1->max - h1->min);
    return h1->bins[bin];
}

float histo_2d_hist_xy_value(int id, float x, float y)
/*
   Returns the value in the histogram bin that would be filled by the 
   values x and y.  The result is undefined if id is not a two dimensional
   histogram, or if x or y are not within the range of the histogram.
   
   	id		HistoScope id, as returned by create routines
   	x		The x coordinate to the bin whose value the function
   			should return.
   	y		The y coordinate to the bin whose value the function
   			should return.
   	
   	Return Value:	The value stored in the histogram bin referenced by
   			x and y.
*/
{
    hs2DHist *h2;
    int bin_x, bin_y; 
    
    if (!InitializedAndActive)
    	return 0.;			/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_2d_hist_xy_value: Invalid id: %d\n",id);
        return 0.;
    }
      
    h2 = (hs2DHist *) GetItemByPtrID(id);
    if (h2 == NULL) {
        fprintf (stderr,
         "hs_2d_hist_xy_value: Invalid id: %d, histogram does not exist\n", id);
        return 0.;
    }
    if (h2->type !=HS_2D_HISTOGRAM) {
        fprintf (stderr,
          "hs_2d_hist_xy_value: Item (id = %d) is not a 2-dim histogram\n", id);
        return 0.;
    }
    if ((x < h2->xMin) || (x > h2->xMax)) {
        fprintf (stderr, "hs_2d_hist_xy_value: specified X coordinate (%f)", x);
        fprintf (stderr, " for Histogram (id = %d)\n    is out of range\n", id);
        return 0.;
    }
    
    if ((y < h2->yMin) || (y > h2->yMax)) {
        fprintf (stderr, "hs_2d_hist_xy_value: specified y coordinate (%f)", y);
        fprintf (stderr, " for Histogram (id = %d)\n    is out of range\n", id);
        return 0.;
    }
    
    bin_x = ((x - h2->xMin) * (float) h2->nXBins)
    					 / (h2->xMax - h2->xMin);
    bin_y = ((y - h2->yMin) * (float) h2->nYBins)
    					 / (h2->yMax - h2->yMin);
    return h2->bins[(bin_x)*h2->nYBins + bin_y]; 
}


float histo_3d_hist_xyz_value(int id, float x, float y, float z)
/*
   Returns the value in the histogram bin that would be filled by the 
   values x, y, and z.  The result is undefined if id is not a 3 dimensional
   histogram, or if x, y, and z are not within the range of the histogram.
*/
{
    hs3DHist *h3;
    int bin_x, bin_y, bin_z; 
    
    if (!InitializedAndActive)
    	return 0.;			/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_3d_hist_xyz_value: Invalid id: %d\n",id);
        return 0.;
    }
      
    h3 = (hs3DHist *) GetItemByPtrID(id);
    if (h3 == NULL) {
        fprintf (stderr,
         "hs_3d_hist_xy_value: Invalid id: %d, histogram does not exist\n", id);
        return 0.;
    }
    if (h3->type !=HS_3D_HISTOGRAM) {
        fprintf (stderr,
          "hs_3d_hist_xy_value: Item (id = %d) is not a 3-dim histogram\n", id);
        return 0.;
    }
    if ((x < h3->xMin) || (x > h3->xMax)) {
        fprintf (stderr, "hs_3d_hist_xyz_value: specified X coordinate (%f)", x);
        fprintf (stderr, " for Histogram (id = %d)\n    is out of range\n", id);
        return 0.;
    }
    
    if ((y < h3->yMin) || (y > h3->yMax)) {
        fprintf (stderr, "hs_3d_hist_xyz_value: specified y coordinate (%f)", y);
        fprintf (stderr, " for Histogram (id = %d)\n    is out of range\n", id);
        return 0.;
    }
    
    if ((z < h3->zMin) || (z > h3->zMax)) {
        fprintf (stderr, "hs_3d_hist_xyz_value: specified z coordinate (%f)", z);
        fprintf (stderr, " for Histogram (id = %d)\n    is out of range\n", id);
        return 0.;
    }
    
    bin_x = ((x - h3->xMin) * (float) h3->nXBins)
    					 / (h3->xMax - h3->xMin);
    bin_y = ((y - h3->yMin) * (float) h3->nYBins)
    					 / (h3->yMax - h3->yMin);
    bin_z = ((z - h3->zMin) * (float) h3->nZBins)
    					 / (h3->zMax - h3->zMin);
    return h3->bins[(bin_x*h3->nYBins + bin_y)*h3->nZBins + bin_z];
}


float histo_1d_hist_bin_value(int id, int bin_num, int bindingflag)
/*
   Returns the value in the histogram bin referred to by bin (or channel)
   number.  The result is undefined if id is not a one dimensional histogram
   or bin_num is less than 1 or greater than the number of bins.
   
   	id		HistoScope id, as returned by create routines
   	bin_num		The number of the bin whose value the function should
   			return. For Fortran user, starts at 1, c user, at 0.
   	Return Value:	The value stored in the histogram bin referenced by
   			bin_num.  The result is undefined if id does not
   			exist, or does not refer to a one dimensional histogram
*/
{    
    hs1DHist *h1;
    int binn;
    
    if (!InitializedAndActive)
    	return 0.;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_1d_hist_bin_value: Invalid id: %d\n",id);
        return 0.;
    }
      
    h1 = (hs1DHist *) GetItemByPtrID(id);
    if (h1 == NULL) {
        fprintf (stderr,
            "hs_1d_hist_bin_value: Invalid id: %d, histogram does not exist\n",
             id);
        return 0.;
    }
    if (h1->type !=HS_1D_HISTOGRAM) {
        fprintf (stderr,
         "hs_1d_hist_bin_value: Item (id = %d) is not a 1-dim histogram\n", id);
        return 0.;
    }
    
    binn = bin_num;
    if (bindingflag != 0) binn--;
    if ((binn < 0) || (binn >= h1->nBins)) {
        fprintf (stderr, "hs_1d_hist_bin_value: Specified bin number ");
        fprintf (stderr, "(%d) for Histogram (id = %d)\n     is out of range\n",
            bin_num, id);
        return 0.;
    }
    return h1->bins[binn];
}

float histo_2d_hist_bin_value(int id, int x_bin_num,
                           int y_bin_num, int bindingflag)
/*
   Returns the value in the histogram bin referred to by bin (or channel)
   number.  The result is undefined if id is not a 2 dimensional histogram
   or bin_num is less than 1 or greater than the number of bins.
   
   	id		HistoScope id, as returned by create routines
   	x_bin_num	The x index of the bin.
   	y_bin_num	The y index of the bin.
   			For Fortran user, bin # starts at 1, c user, at 0.

   	Return Value:	The value stored in the histogram bin referenced by
   			the bin numbers.  The result is undefined if id does
   			not exist, or does not refer to a one dimensional
   			histogram.
*/
{
    hs2DHist *h2;
    int bin_x, bin_y; 
    
    if (!InitializedAndActive)
    	return 0.;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_2d_hist_bin_value: Invalid id: %d\n",id);
        return 0.;
    }
      
    h2 = (hs2DHist *) GetItemByPtrID(id);
    if (h2 == NULL) {
        fprintf (stderr,
            "hs_2d_hist_bin_value: Invalid id: %d, histogram does not exist\n",
             id);
        return 0.;
    }
    if (h2->type !=HS_2D_HISTOGRAM) {
        fprintf (stderr,
         "hs_2d_hist_bin_value: Item (id = %d) is not a 2-dim histogram\n", id);
        return 0.;
    }
    bin_x = x_bin_num;
    if (bindingflag != 0) bin_x--;
    bin_y = y_bin_num;
    if (bindingflag != 0) bin_y--;
    
    if ((bin_x < 0 ) || (bin_x >= h2->nXBins)) {
        fprintf (stderr, "hs_2d_hist_bin_value: Specified x bin number ");
        fprintf (stderr, "(%d) for Histogram (id = %d)\n     is out of range\n",
            x_bin_num, id);
        return 0.;
    }
    
    if ((bin_y < 0 ) || (bin_y >= h2->nYBins)) {
        fprintf (stderr, "hs_2d_hist_bin_value: Specified y bin number ");
        fprintf (stderr, "(%d) for Histogram (id = %d)\n     is out of range\n",
            y_bin_num, id);
        return 0.;
    }
    
    return h2->bins[(bin_x)*h2->nYBins + bin_y]; 
}

float histo_3d_hist_bin_value(int id, int x_bin_num, int y_bin_num,
			      int z_bin_num, int bindingflag)
/*
   Returns the value in the histogram bin referred to by bin (or channel)
   number.  The result is undefined if id is not a 3 dimensional histogram
   or bin_num is less than 1 or greater than the number of bins.
*/
{
    hs3DHist *h3;
    int bin_x, bin_y, bin_z; 
    
    if (!InitializedAndActive)
    	return 0.;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_3d_hist_bin_value: Invalid id: %d\n",id);
        return 0.;
    }
      
    h3 = (hs3DHist *) GetItemByPtrID(id);
    if (h3 == NULL) {
        fprintf (stderr,
            "hs_3d_hist_bin_value: Invalid id: %d, histogram does not exist\n",
             id);
        return 0.;
    }
    if (h3->type !=HS_3D_HISTOGRAM) {
        fprintf (stderr,
         "hs_3d_hist_bin_value: Item (id = %d) is not a 3-dim histogram\n", id);
        return 0.;
    }
    bin_x = x_bin_num;
    if (bindingflag != 0) bin_x--;
    bin_y = y_bin_num;
    if (bindingflag != 0) bin_y--;
    bin_z = z_bin_num;
    if (bindingflag != 0) bin_z--;

    if ((bin_x < 0 ) || (bin_x >= h3->nXBins)) {
        fprintf (stderr, "hs_3d_hist_bin_value: Specified x bin number ");
        fprintf (stderr, "(%d) for Histogram (id = %d)\n     is out of range\n",
            x_bin_num, id);
        return 0.;
    }
    
    if ((bin_y < 0 ) || (bin_y >= h3->nYBins)) {
        fprintf (stderr, "hs_3d_hist_bin_value: Specified y bin number ");
        fprintf (stderr, "(%d) for Histogram (id = %d)\n     is out of range\n",
            y_bin_num, id);
        return 0.;
    }
    
    if ((bin_z < 0 ) || (bin_z >= h3->nZBins)) {
        fprintf (stderr, "hs_3d_hist_bin_value: Specified z bin number ");
        fprintf (stderr, "(%d) for Histogram (id = %d)\n     is out of range\n",
            z_bin_num, id);
        return 0.;
    }
    
    return h3->bins[(bin_x*h3->nYBins + bin_y)*h3->nZBins + bin_z];
}


int histo_1d_hist_labels(int id, char *xlabel, char *ylabel)
/*
   Return the labels in a 1D histogram
   
	id	       HistoScope id, as returned by create routines
	xlabel         Returns X label; or empty string if invalid id
	ylabel         Returns Y label; or empty string if invalid id

   xlabel and ylabel must be long enough to fit the labels

   Function return value is equal id if everything is OK, and 0 if
   the histogram was not found.
*/
{
    hs1DHist *h1;

    if (!InitializedAndActive)
    	return 0;		   /* hs_initialize must be called first */

    xlabel[0] = '\0';
    ylabel[0] = '\0';
    if (id <= 0) {
        fprintf (stderr, "hs_1d_hist_labels: Invalid id: %d\n",id);
        return 0;
    }
      
    h1 = (hs1DHist *) GetItemByPtrID(id);
    if (h1 == NULL) {
        fprintf (stderr,
            "hs_1d_hist_labels: Invalid id: %d, histogram does not exist\n",
             id);
        return 0;
    }
    if (h1->type !=HS_1D_HISTOGRAM) {
        fprintf (stderr,
          "hs_1d_hist_labels: Item (id = %d) is not a 1-dim histogram\n", id);
        return 0;
    }
    
    strcpy(xlabel, h1->xLabel);
    strcpy(ylabel, h1->yLabel);
    return id;
}


int histo_2d_hist_labels(int id, char *xlabel, char *ylabel, char *zlabel)
/*
   Return the labels in a 2D histogram
   
	id	       HistoScope id, as returned by create routines
	xlabel         Returns X label; or empty string if invalid id
	ylabel         Returns Y label; or empty string if invalid id
        zlabel         Returns Z label; or empty string if invalid id

   xlabel, ylabel, and zlabel must be long enough to fit the labels

   Function return value is equal id if everything is OK, and 0 if
   the histogram was not found.
*/
{
    hs2DHist *h2;
    
    if (!InitializedAndActive)
    	return 0;		   /* hs_initialize must be called first */
    	
    xlabel[0] = '\0';
    ylabel[0] = '\0';
    zlabel[0] = '\0';
    if (id <= 0) {
        fprintf (stderr, "hs_2d_hist_labels: Invalid id: %d\n",id);
        return 0;
    }
      
    h2 = (hs2DHist *) GetItemByPtrID(id);
    if (h2 == NULL) {
        fprintf (stderr,
            "hs_2d_hist_labels: Invalid id: %d, histogram does not exist\n",
             id);
        return 0;
    }
    if (h2->type !=HS_2D_HISTOGRAM) {
        fprintf (stderr,
          "hs_2d_hist_labels: Item (id = %d) is not a 2-dim histogram\n", id);
        return 0;
    }
    
    strcpy(xlabel, h2->xLabel);
    strcpy(ylabel, h2->yLabel);
    strcpy(zlabel, h2->zLabel);
    return id;
}


int histo_3d_hist_labels(int id, char *xlabel, char *ylabel,
			 char *zlabel, char *vlabel)
/*
   Return the labels in a 3D histogram
   
	id	       HistoScope id, as returned by create routines
	xlabel         Returns X label; or empty string if invalid id
	ylabel         Returns Y label; or empty string if invalid id
        zlabel         Returns Z label; or empty string if invalid id
        vlabel         Returns "vertical" label; or empty string if invalid id

   xlabel, ylabel, zlabel, and vlabel must be long enough to fit the labels

   Function return value is equal id if everything is OK, and 0 if
   the histogram was not found.
*/
{
    hs3DHist *h3;
    
    if (!InitializedAndActive)
    	return 0;		   /* hs_initialize must be called first */
    	
    xlabel[0] = '\0';
    ylabel[0] = '\0';
    zlabel[0] = '\0';
    vlabel[0] = '\0';
    if (id <= 0) {
        fprintf (stderr, "hs_3d_hist_labels: Invalid id: %d\n",id);
        return 0;
    }
      
    h3 = (hs3DHist *) GetItemByPtrID(id);
    if (h3 == NULL) {
        fprintf (stderr,
            "hs_3d_hist_labels: Invalid id: %d, histogram does not exist\n",
             id);
        return 0;
    }
    if (h3->type !=HS_3D_HISTOGRAM) {
        fprintf (stderr,
          "hs_3d_hist_labels: Item (id = %d) is not a 3-dim histogram\n", id);
        return 0;
    }
    
    strcpy(xlabel, h3->xLabel);
    strcpy(ylabel, h3->yLabel);
    strcpy(zlabel, h3->zLabel);
    strcpy(vlabel, h3->vLabel);
    return id;
}


int histo_hist_error_status(int id)
{
    hsGeneral *item;
    hs1DHist *h1;
    hs2DHist *h2;
    hs3DHist *h3;

    if (!InitializedAndActive)
    	return HS_ITEMNOTFOUND_ERRORS;	/* hs_initialize must be called first */

    if (id <= 0) {
	return HS_ITEMNOTFOUND_ERRORS;
    }

    item = (hsGeneral *)GetItemByPtrID(id);
    if (item == NULL) {
	return HS_ITEMNOTFOUND_ERRORS;
    }

    switch (item->type)
    {
    case HS_1D_HISTOGRAM:
	h1 = (hs1DHist *)item;
	if (h1->errFlg == 0 || h1->pErrs == NULL)
	    return HS_NO_ERRORS;
	if (h1->mErrs == NULL)
	    return HS_POS_ERRORS;
	return HS_BOTH_ERRORS;

    case HS_2D_HISTOGRAM:
	h2 = (hs2DHist *)item;
	if (h2->errFlg == 0 || h2->pErrs == NULL)
	    return HS_NO_ERRORS;
	if (h2->mErrs == NULL)
	    return HS_POS_ERRORS;
	return HS_BOTH_ERRORS;

    case HS_3D_HISTOGRAM:
	h3 = (hs3DHist *)item;
	if (h3->errFlg == 0 || h3->pErrs == NULL)
	    return HS_NO_ERRORS;
	if (h3->mErrs == NULL)
	    return HS_POS_ERRORS;
	return HS_BOTH_ERRORS;

    default:
	return HS_ITEMNOTFOUND_ERRORS;
    }
}
