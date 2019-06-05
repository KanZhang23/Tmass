/*******************************************************************************
*									       *
* histoApiStatH.c -- Application Interface routines,                           *
* to estimate various Statistical properties of  Histograms.                   *
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
* January 1994 							               *
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
#include <float.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>
#ifndef VXWORKS
#include <sys/errno.h>
#endif /*VXWORKS*/
#include <math.h>
#include "../histo_util/hsTypes.h"
#include "../histo_util/histoUtil.h"
#include "HistoClient.h"
#include "histoApiStatH.h"

/*
** Calculations on histogram data
**
** hs_1d_hist_minimum, hs_2d_hist_minimum, hs_1d_hist_maximum,
** hs_2d_hist_maximum, hs_1d_hist_stats, hs_2d_hist_stats, hs_hist_integral
*/
void histo_1d_hist_minmax(int id, int mode,  float *x, int *bin_num,
                                     float *value, int bindingflag )
/*
   Gives the coordinate and bin content where the histogram data reaches a 
   extremum.  The routine returns a bin_num of 0 if the histogram does not
   refer to a 1D histogram, or the histogram is empty, or does not exist

	id		HistoScope id, as returned by create routines
	mode 		if 0, the minimum will be returned, else maximum
	x		Returns the x coordinate to the bin where the
			minimum appears.
	bin_num		Returns the bin number of bin where the minimum
			appears. 
	value		Returns the value from the bin where the minimum
			appears.
	bindingflag	0 for c, else for Fortran ( bin numbers are different)
*/
{
    int i, j = 0;
    hs1DHist *h1;
    float *tdh;
    float valt = FLT_MAX;
    
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
      fprintf (stderr, "hs_1d_hist_extremum: Invalid id: %d \n",id);
      *bin_num = 0;
      return;
      }
      
    h1 = (hs1DHist *) GetItemByPtrID(id);
    if (h1 == NULL) {
      fprintf (stderr,
         "hs_1d_hist_extremum: Invalid id: %d, histogram does not exist\n", id);
      *bin_num = 0;
      return;
    }
    
    if (h1->type !=HS_1D_HISTOGRAM) {
      fprintf (stderr,
         "hs_1d_hist_extremum: Item (id = %d) is not a 1-dim histogram \n", id);
      *bin_num = 0;
      return;
    }

    if (h1->bins == NULL || h1->count <= 0) {
      *bin_num = 0;
      *x = h1->min;
      *value = 0.f;
      return;
    }

    tdh = h1->bins;
    if (mode == 0) {
        valt = FLT_MAX;
        for (i=0; i<h1->nBins; i++, tdh++)  
                    if (*tdh < valt) {
                         valt = *tdh;
                         j=i;
                    }
    } else {
        valt = -FLT_MAX;
        for (i=0; i<h1->nBins; i++, tdh++)  
                    if (*tdh > valt) {
                         valt = *tdh;
                         j=i;
                    }
    }     
    *x = h1->min + (h1->max - h1->min) * j / ((float) h1->nBins);
    tdh = h1->bins;
    tdh += j;
    *value = *tdh; 
    if (bindingflag == 0) *bin_num = j;
    else *bin_num = (j+1);
    return;
}

void histo_2d_hist_minmax(int id, int mode,  float *x, float *y, 
				int *x_bin_num, int *y_bin_num, 
                                     float *value, int bindingflag )
/*
   Gives the coordinate and bin content where the histogram data reaches a 
   extremum.  The routine returns bin nums of 0 if the histogram does not
   refer to a 2D histogram, or the histogram is empty, or does not exist

	id		HistoScope id, as returned by create routines
	mode 		if 0, the minimum will be returned, else maximum
	x		Returns the x coordinate to the bin where the
			minimum appears.
	y		Returns the y coordinate to the bin where the
			minimum appears.
	x_bin_num	Returns the x bin number of bin where the minimum
			appears. 
	y_bin_num	Returns the y bin number of bin where the minimum
			appears. 
	value		Returns the value from the bin where the minimum
			appears.
	bindingflag	0 for c, else for Fortran ( bin numbers are different)
*/
{
    int i, j, k = 0, nb2;
    hs2DHist *h2;
    float *tdh;
    float valt = FLT_MAX;
    
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
      fprintf (stderr, "hs_2d_hist_extremum: Invalid id: %d \n",id);
      return;
      }
      
    h2 = (hs2DHist *) GetItemByPtrID(id);
    if (h2 == NULL) {
      fprintf (stderr,
          "hs_2d_hist_extremum: Invalid id, histogram %d does not exist\n", id);
      *x_bin_num = 0;
      *y_bin_num = 0;
      return;
    }
    if (h2->type !=HS_2D_HISTOGRAM) {
      fprintf (stderr,
         "hs_2d_hist_extremum: Item (id = %d) not a 2-dim histogram\n", id);
      *x_bin_num = 0;
      *y_bin_num = 0;
      return;
    }

    if (h2->bins == NULL || h2->count <= 0) {
      *x_bin_num = 0;
      *y_bin_num = 0;
      *value = 0.f;
      *x = h2->xMin;
      *y = h2->yMin;
      return;
    }

    tdh = h2->bins;
    nb2 = (h2->nXBins) * (h2->nYBins);
    if (mode == 0) {
    valt = FLT_MAX;
    for (i=0; i<nb2; i++, tdh++)  
                    if (*tdh < valt) {
                         valt = *tdh;
                         k=i;
                         }
      } else {
    valt = -FLT_MAX;
    for (i=0; i<nb2; i++, tdh++)  
                    if (*tdh > valt) {
                         valt = *tdh;
                         k=i;
                         }
    }
    
    i = k/(h2->nYBins);
    j = k - i * h2->nYBins;      
    *x = h2->xMin + (h2->xMax - h2->xMin) * i / ((float) h2->nXBins);
    *y = h2->yMin + (h2->yMax - h2->yMin) * j / ((float) h2->nYBins);
    tdh = h2->bins;
    tdh += k;
    *value = *tdh; 
    if (bindingflag == 0) {
       *x_bin_num = i;
       *y_bin_num = j;
      } else  {
       *x_bin_num = (i+1);
       *y_bin_num = (j+1);
      }
    return;
}

void histo_3d_hist_minmax(int id, int mode,  float *x, float *y, float *z,
			  int *x_bin_num, int *y_bin_num, int *z_bin_num,
			  float *value, int bindingflag )
/*
   Gives the coordinate and bin content where the histogram data reaches a 
   extremum.  The routine returns bin nums of 0 if the histogram does not
   refer to a 3D histogram, or the histogram is empty, or does not exist
*/
{
    int i, j, k, l = 0, nb3;
    hs3DHist *h3;
    float *tdh;
    float valt = FLT_MAX;
    
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */

    if (id <= 0) {
      fprintf (stderr, "hs_3d_hist_extremum: Invalid id: %d \n",id);
      return;
    }

    h3 = (hs3DHist *) GetItemByPtrID(id);
    if (h3 == NULL) {
      fprintf (stderr,
          "hs_3d_hist_extremum: Invalid id, histogram %d does not exist\n", id);
      *x_bin_num = 0;
      *y_bin_num = 0;
      *z_bin_num = 0;
      return;
    }
    if (h3->type !=HS_3D_HISTOGRAM) {
      fprintf (stderr,
         "hs_3d_hist_extremum: Item (id = %d) not a 3-dim histogram\n", id);
      *x_bin_num = 0;
      *y_bin_num = 0;
      *z_bin_num = 0;
      return;
    }

    if (h3->bins == NULL || h3->count <= 0) {
      *x_bin_num = 0;
      *y_bin_num = 0;
      *z_bin_num = 0;
      *value = 0.f;
      *x = h3->xMin;
      *y = h3->yMin;
      *z = h3->zMin;
      return;
    }

    tdh = h3->bins;
    nb3 = (h3->nXBins) * (h3->nYBins) * (h3->nZBins);
    if (mode == 0) {
	valt = FLT_MAX;
	for (i=0; i<nb3; i++, tdh++)  
	    if (*tdh < valt) {
		valt = *tdh;
		l=i;
	    }
    } else {
	valt = -FLT_MAX;
	for (i=0; i<nb3; i++, tdh++)  
	    if (*tdh > valt) {
		valt = *tdh;
		l=i;
	    }
    }

    i = l/(h3->nYBins*h3->nZBins);
    j = (l - i*h3->nYBins*h3->nZBins)/h3->nZBins;
    k = l - i*h3->nYBins*h3->nZBins - j*h3->nZBins;
    *x = h3->xMin + (h3->xMax - h3->xMin) * i / ((float) h3->nXBins);
    *y = h3->yMin + (h3->yMax - h3->yMin) * j / ((float) h3->nYBins);
    *z = h3->zMin + (h3->zMax - h3->zMin) * k / ((float) h3->nZBins);
    tdh = h3->bins;
    tdh += l;
    *value = *tdh; 
    if (bindingflag == 0) {
	*x_bin_num = i;
	*y_bin_num = j;
	*z_bin_num = k;
    } else  {
	*x_bin_num = (i+1);
	*y_bin_num = (j+1);
	*z_bin_num = (k+1);
    }
    return;
}

void histo_1d_hist_stats(int id, float *mean, float *std_dev)
/*
   Calculates the mean and standard deviation of a one dimensional histogram.
   
   	id		HistoScope id, as returned by create routines
   	mean		Returns the mean of the histogram
   	std_dev		Returns the standard deviation of the histogram
*/
{
    int i;
    hs1DHist *h1;
    float *tdh;
    int nBins;
    double xb, binw;
    double binValue, sum = 0., sum2 = 0., dmean, ddstDev, atot = 0.;
    
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_1d_hist_stats: Invalid id: %d \n",id);
        return;
    }
      
    h1 = (hs1DHist *) GetItemByPtrID(id);
    if (h1 == NULL) {
      fprintf (stderr,
         "hs_1d_hist_stats: Invalid id: %d, histogram does not exist\n", id);
      return;
    }
    if (h1->type !=HS_1D_HISTOGRAM) {
      fprintf (stderr,
         "hs_1d_hist_stats: Item (id = %d) is not a 1-dim histogram \n", id);
      return;
    }
    
    tdh = h1->bins;
    nBins = h1->nBins;
    binw = (h1->max - h1->min)/h1->nBins;
    for (i=0; i<nBins; i++) {
    	binValue = tdh[i];
    	atot += binValue;
    	xb = h1->min + binw*(i+.5);
    	sum += xb * binValue;
    	sum2 += xb * xb * binValue;
    }
    
    /* calculate mean (using bin centers) and standard deviation */
    if ((atot < -FLT_MAX) || (atot == 0.)) {
    	dmean = ddstDev = 0.;
    } else {
	dmean = sum/atot;
	if (atot < 1.) ddstDev = 0.;
	 else  ddstDev = sqrt((sum2 - (atot*dmean*dmean))/(atot - 1.));
	 }
    *mean = (float) dmean;
    *std_dev = (float) ddstDev;
    return;
}

void histo_2d_hist_stats(int id, float *x_mean, float *y_mean,
                                 float *x_std_dev, float *y_std_dev)
/*
   Calculates the mean and standard deviation of a two dimensional histogram.
   
   	id		HistoScope id, as returned by create routines
   	x_mean		Returns the mean of the histogram calculated along
   			    the x axis.
   	y_mean		Returns the mean of the histogram calculated along
   			    the y axis.
   	x_std_dev	Returns the standard deviation of the histogram
   			    calculated along the x axis.
   	y_std_dev	Returns the standard deviation of the histogram
   			    calculated along the y axis.
*/
{
    int i, j;
    hs2DHist *h2;
    double x_xb, x_binw, y_xb, y_binw, x_atot, y_atot;
    double binValue, x_sum, y_sum, x_sum2, y_sum2, dmean, ddstDev;
    
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_2d_hist_stats: Invalid id: %d \n",id);
        return;
    }
      
    h2 = (hs2DHist *) GetItemByPtrID(id);
    if (h2 == NULL) {
        fprintf (stderr,
            "hs_2d_hist_stats: Invalid id: %d, histogram does not exist\n", id);
        return;
    }
    if (h2->type !=HS_2D_HISTOGRAM) {
        fprintf (stderr,
            "hs_2d_hist_stats: Item (id = %d) is not a 2-dim histogram \n", id);
        return;
    }
    
    x_binw = (h2->xMax - h2->xMin)/h2->nXBins;
    x_sum = x_sum2 = x_atot = 0.;
    y_binw = (h2->yMax - h2->yMin)/h2->nYBins;
    y_sum = y_sum2 = y_atot = 0.;
    for (i=0; i<h2->nXBins; i++) {
    	x_xb = h2->xMin + x_binw*(i+.5);
    	for (j=0; j<h2->nYBins; j++) {
    	    y_xb = h2->yMin + y_binw*(j+.5);
    	    binValue = h2->bins[i*h2->nYBins + j];
    	    x_atot += binValue;
    	    x_sum += x_xb * binValue;
    	    x_sum2 += x_xb * x_xb * binValue;
    	    y_atot += binValue;
    	    y_sum += y_xb * binValue;
    	    y_sum2 += y_xb * y_xb * binValue;
    	}
    }
    
    /* calculate mean (using bin centers) and standard deviation */
    if ((x_atot < -FLT_MAX) || (x_atot == 0.)) {
    	dmean = ddstDev = 0.;
    } else {
	dmean = x_sum/x_atot;
	if (x_atot < 1.) 
	    ddstDev = 0.;
	else   
	    ddstDev = sqrt((x_sum2 - (x_atot*dmean*dmean))/(x_atot - 1.));
    }
    *x_mean = (float) dmean;
    *x_std_dev = (float) ddstDev;
    
    if ((y_atot < -FLT_MAX) || (y_atot == 0.)) {
    	dmean = ddstDev = 0.;
    } else {
	dmean = y_sum/y_atot;
	if (y_atot < 1.) 
	    ddstDev = 0.;
	else   
	    ddstDev = sqrt((y_sum2 - (y_atot*dmean*dmean))/(y_atot - 1.));
    }
    *y_mean = (float) dmean;
    *y_std_dev = (float) ddstDev;
    return;
}

void histo_3d_hist_stats(int id, float *x_mean, float *y_mean, float *z_mean,
			 float *x_std_dev, float *y_std_dev, float *z_std_dev)
/*
   Calculates the mean and standard deviation of a three dimensional histogram.
*/
{
    int i, j, k;
    hs3DHist *h3;
    double x_xb, x_binw, y_xb, y_binw, z_xb, z_binw, x_atot, y_atot, z_atot;
    double binValue, x_sum, y_sum, z_sum, x_sum2, y_sum2, z_sum2, dmean, ddstDev;

    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_3d_hist_stats: Invalid id: %d \n",id);
        return;
    }
      
    h3 = (hs3DHist *) GetItemByPtrID(id);
    if (h3 == NULL) {
        fprintf (stderr,
            "hs_3d_hist_stats: Invalid id: %d, histogram does not exist\n", id);
        return;
    }
    if (h3->type !=HS_3D_HISTOGRAM) {
        fprintf (stderr,
            "hs_3d_hist_stats: Item (id = %d) is not a 3-dim histogram \n", id);
        return;
    }
    
    x_binw = (h3->xMax - h3->xMin)/h3->nXBins;
    x_sum = x_sum2 = x_atot = 0.;
    y_binw = (h3->yMax - h3->yMin)/h3->nYBins;
    y_sum = y_sum2 = y_atot = 0.;
    z_binw = (h3->zMax - h3->zMin)/h3->nZBins;
    z_sum = z_sum2 = z_atot = 0.;
    for (i=0; i<h3->nXBins; i++) {
    	x_xb = h3->xMin + x_binw*(i+.5);
    	for (j=0; j<h3->nYBins; j++) {
    	    y_xb = h3->yMin + y_binw*(j+.5);
	    for (k=0; k<h3->nZBins; k++) {
		z_xb = h3->zMin + z_binw*(k+.5);
		binValue = h3->bins[(i*h3->nYBins+j)*h3->nZBins + k];
		x_atot += binValue;
		x_sum += x_xb * binValue;
		x_sum2 += x_xb * x_xb * binValue;
		y_atot += binValue;
		y_sum += y_xb * binValue;
		y_sum2 += y_xb * y_xb * binValue;
		z_atot += binValue;
		z_sum += z_xb * binValue;
		z_sum2 += z_xb * z_xb * binValue;
	    }
    	}
    }

    /* calculate mean (using bin centers) and standard deviation */
    if ((x_atot < -FLT_MAX) || (x_atot == 0.)) {
    	dmean = ddstDev = 0.;
    } else {
	dmean = x_sum/x_atot;
	if (x_atot < 1.) 
	    ddstDev = 0.;
	else   
	    ddstDev = sqrt((x_sum2 - (x_atot*dmean*dmean))/(x_atot - 1.));
    }
    *x_mean = (float) dmean;
    *x_std_dev = (float) ddstDev;
    
    if ((y_atot < -FLT_MAX) || (y_atot == 0.)) {
    	dmean = ddstDev = 0.;
    } else {
	dmean = y_sum/y_atot;
	if (y_atot < 1.) 
	    ddstDev = 0.;
	else   
	    ddstDev = sqrt((y_sum2 - (y_atot*dmean*dmean))/(y_atot - 1.));
    }
    *y_mean = (float) dmean;
    *y_std_dev = (float) ddstDev;

    if ((z_atot < -FLT_MAX) || (z_atot == 0.)) {
    	dmean = ddstDev = 0.;
    } else {
	dmean = z_sum/z_atot;
	if (z_atot < 1.) 
	    ddstDev = 0.;
	else   
	    ddstDev = sqrt((z_sum2 - (z_atot*dmean*dmean))/(z_atot - 1.));
    }
    *z_mean = (float) dmean;
    *z_std_dev = (float) ddstDev;

    return;
}

float histo_hist_integral(int id)
/*
   Calculate the integral (the sum of content*binwidth) of a histogram.
   Overflow data is not counted.  The result is undefined if id does not
   exist or refer to a 1D histogram.
   
   
   	id		HistoScope id, as returned by create routines.
   	
   	Return Value:	The integral of the bin data in the histogram.
*/

{
    int i, nb;
    hsGeneral *item;
    hs1DHist *h1;
    hs2DHist *h2;
    hs3DHist *h3;
    float *tdh;
    float binwx = 1.f, binwy = 1.f, binwz = 1.f;
    double valt = 0.0;

    if (!InitializedAndActive)
    	return -1.;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_hist_integral: Invalid id: %d \n",id);
        return -1.;
    }
      
    item = (hsGeneral *) GetItemByPtrID(id);
    if (item == NULL) {
        fprintf (stderr,
            "hs_hist_integral: Invalid id: %d, histogram does not exist\n", id);
        return -1.;
    }

    switch (item->type) {
       case HS_1D_HISTOGRAM:
         h1 = (hs1DHist *) item;
       	 nb = h1->nBins;
       	 binwx = (h1->max - h1->min)/nb;
       	 tdh = h1->bins;
       	 break;
       case HS_2D_HISTOGRAM:
         h2 = (hs2DHist *) item;
       	 nb = (h2->nXBins)*(h2->nYBins);
       	 binwx = (h2->xMax - h2->xMin)/h2->nXBins;
       	 binwy = (h2->yMax - h2->yMin)/h2->nYBins;
       	 tdh = h2->bins;
       	 break;
       case HS_3D_HISTOGRAM:
         h3 = (hs3DHist *) item;
       	 nb = (h3->nXBins)*(h3->nYBins)*(h3->nZBins);
       	 binwx = (h3->xMax - h3->xMin)/h3->nXBins;
       	 binwy = (h3->yMax - h3->yMin)/h3->nYBins;
       	 binwz = (h3->zMax - h3->zMin)/h3->nZBins;
       	 tdh = h3->bins;
       	 break;
       default:
          fprintf (stderr,
              "hs_hist_integral: Item (id = %d) is not a histogram \n", id);
          return -1.;
     }
     for (i=0; i<nb; i++) valt += tdh[i];
     return (binwx * binwy * binwz * valt);
}

void histo_hist_set_gauss_errors(int id)
/*
   Calculate and store Gaussian error in a histogram. If positive 
   error have been previously stored, they will be disregarded. If
   negative errors have been previously stored, they are free'd, since
   gaussian errors are, by definition, symmetric.  The new errors will be  
   stored in the "positive" error array.  The errors are computed assuming 
   the central limit theorem, e.g., the are equal to the square root of the 
   bin content.  If one or more bin content is negativ, no errors are created
   or changed, and an error message is printed. 
   
   	id		HistoScope id, as returned by create routines.
   			if the item refered by id is not a 1D or a 2D 
   			Histogram, no action is taken beside printing an 
   			error message.
*/
{
    int i, nb;
    hsGeneral *item;
    hs1DHist *h1 = NULL;
    hs2DHist *h2 = NULL;
    hs3DHist *h3 = NULL;
    float *tdh, *tde, **tdm;
	
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    if (id <= 0) {
        fprintf (stderr, "hs_hist_set_gauss_errors: Invalid id: %d\n",id);
        return;
    }
      
    item = (hsGeneral *) GetItemByPtrID(id);
    if (item == NULL) {
        fprintf (stderr,
    "hs_hist_set_gauss_errors: Invalid id: %d, histogram does not exist\n", id);
        return;
    }

    switch (item->type) {
       case HS_1D_HISTOGRAM:
         h1 = (hs1DHist *) item;
       	 nb = h1->nBins;
       	 tdh = h1->bins;
       	 tde = h1->pErrs;
       	 tdm = &h1->mErrs;
       	 break;
       case HS_2D_HISTOGRAM:
         h2 = (hs2DHist *) item;
       	 nb = (h2->nXBins)*(h2->nYBins);
       	 tdh = h2->bins;
       	 tde = h2->pErrs;
       	 tdm = &h2->mErrs;
       	 break;
       case HS_3D_HISTOGRAM:
         h3 = (hs3DHist *) item;
       	 nb = (h3->nXBins)*(h3->nYBins)*(h3->nZBins);
       	 tdh = h3->bins;
       	 tde = h3->pErrs;
       	 tdm = &h3->mErrs;
       	 break;
       default:
          fprintf (stderr,
           "hs_hist_set_gauss_errors: Item (id = %d) is not a histogram\n", id);
          return;
     }
     for (i=0; i<nb; i++)
         if (tdh[i] < 0) {
             fprintf (stderr, "hs_hist_set_gauss_errors: Item id %d, \n", id);
             fprintf (stderr,
               "     Cannot compute Gaussian error if contents are negative\n");
             return;
         }
      if (tde == NULL) {
             tde = (float *) malloc (nb * sizeof(float));
	     switch (item->type) {
	     case HS_1D_HISTOGRAM:
		 h1->errFlg = 1;
		 h1->pErrs = tde;
		 break;
	     case HS_2D_HISTOGRAM:
		 h2->errFlg = 1;
		 h2->pErrs = tde;
		 break;
	     case HS_3D_HISTOGRAM:
		 h3->errFlg = 1;
		 h3->pErrs = tde;
		 break;
	     default:
		 assert(0);
	     }
       }
       for (i=0; i<nb; i++)
       	   tde[i] = (float) sqrt( (double) tdh[i]);
       if (*tdm != NULL) {
       	   free(*tdm);			/* free and zero negative errors */
       	   *tdm = NULL;
       }
       SetHistResetFlag(item);
       return;
 
}
