/*******************************************************************************
*									       *
* histoApiArytH.c -- Application Interface routines,                           *
* to create or modify Histogram by arythmetic or algebraic                     *
*  manipulation       Statistical properties of  Histograms.                   *
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
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <errno.h>
#ifndef VXWORKS
#include <sys/errno.h>
#endif /*VXWORKS*/
#include <math.h>
#include "histoscope.h"
#include "../histo_util/hsTypes.h"
#include "../histo_util/histoUtil.h"
#include "HistoClient.h"
#include "histoApiItems.h"
#include "histoApiHists.h"
#include "histoApiFiles.h"
#include "histoApiArytH.h"
#include "histoApiNTs.h"

/* ** static stuff used in this file... */
static int list_items_topc(const char *catTop, int *idList, int maxItemsToRtn);

/*
** Calculations on histogram data
**
** hs_sum_histograms, hs_multiply_histograms, hs_divide_histograms
*/

int histo_sum_histograms(int uid, const char *title, const char *category,
                      int id1, int id2, float const1, float const2)
/*
   Create a new Histogram (1D or 2D) whose data is the sum, bin by bin, of 2
   histograms refered as id1 and id2. The bin content of the new histogram
   will be equal to const1 * bin_id1 + const2 * bin_id2.  const1 or const2
   can be 0., or one of the id's can also be set to 0, 
   in which case the corresponding histogram will not be referenced
   and the new histogram will simply reflect the other histogram multiplied by
   its constant.  Errors are propagated assuming that these errors are Gaussian
   ("Low" errors are ignored).  The routine returns an error (an id of -1) if
   both histograms are nonexistant, the histogram types different, or the
   number of bins, or low/upper edges are inconsistent.
   
	uid		User Identification for the newly created histogram
	title, Category	It's title and category. See hs_create* routines. 
   
   	id1, id2	HistoScope ids of the histograms to sum
   	const1, const2	Constants to multiply the bin contents
   			of the corresponding histograms.
   			
   	Return Value:	The id of a new histogram containing the sum
   			of the two histograms, or -1 if the operation could
   			not be performed.
*/
{
    hs1DHist *h1_1, *h1_2, *h1_s = NULL;
    hs2DHist *h2_1, *h2_2, *h2_s = NULL;
    hs3DHist *h3_1, *h3_2, *h3_s = NULL;
    hsGeneral *h_1, *h_2, *h_s = NULL ;
    int i, idc, i_s = 0, nb2, nb3;
    float *valt, *errt = NULL;
    float const_s = 0.;
    double vv;
    
    
    if ((id1 <= 0) && (id2 <= 0) ) {
        fprintf (stderr,
		 "hs_sum_histograms: Invalid ids: %d %d \n", id1, id2);
        return -1;
    }
    if ((id1 < 0) || (id2 < 0) ) {
        fprintf (stderr,
		 "hs_sum_histograms: Invalid id: %d\n", id1 < 0 ? id1 : id2);
        return -1;
    }
    h_1 = h_2 = NULL;
    if (id1 > 0) h_1 = (hsGeneral *) GetItemByPtrID(id1);
    if (id2 > 0) h_2 = (hsGeneral *) GetItemByPtrID(id2);
    if ( (h_1 == NULL && id1 > 0 && const1 != 0.)
	 || (h_2 == NULL && id2 > 0 && const2 != 0.) ) {
        fprintf (stderr,
		 "hs_sum_histograms: Invalid id: %d, histogram does not exist\n",
		 (id1 > 0 && h_1 == NULL && const1 != 0.) ? id1 : id2);
        return -1;
    }
    if ((const1 == 0.) && (const2 == 0.)) {
        fprintf (stderr,
		 "hs_sum_histograms: Meaningless sum, both constants specified are 0.\n");
        return -1;
    }
    
    if ((h_1 == NULL) && (const2 == 0.)) {
        fprintf (stderr,
		 "hs_sum_histograms: Meaningless sum, id1 = %d and const2 = 0.\n",
		 id1);
        return -1;
    }
    
    if ((h_2 == NULL) && (const1 == 0.)) {
        fprintf (stderr,
		 "hs_sum_histograms: Meaningless sum, id2 = %d and const1 = 0.\n",
		 id2);
        return -1;
    }
    
    if (((h_1 == NULL) && (h_2 != NULL)) || (const1 == 0.)) {
	h_s = h_2;
	const_s = const2;
	i_s = id2;
    }  
    if (((h_2 == NULL) && (h_1 != NULL)) || (const2 == 0.)) {
	h_s = h_1;
	const_s = const1;
	i_s = id1;
    }  

    if (h_s != NULL) {
        switch(h_s->type) {
	case HS_1D_HISTOGRAM:
	    h1_s = (hs1DHist *) h_s;
	    idc = histo_create_1d_hist(uid, title, category,
				       h1_s->xLabel, h1_s->yLabel, 
				       h1_s->nBins, h1_s->min, h1_s->max);    
	    valt = (float *) malloc (h1_s->nBins*sizeof(float));
	    if (h1_s->pErrs != NULL)
	        errt = (float *) malloc (h1_s->nBins*sizeof(float));
	    for (i=0; i<h1_s->nBins; i++) {
		valt[i] = const_s * h1_s->bins[i];
		if (errt != NULL) errt[i] = const_s * h1_s->pErrs[i];
	    }
	    histo_1d_hist_block_fill(idc, valt, errt, NULL);
	    free (valt);
	    if (errt != NULL) free(errt);
	    return idc;
	case HS_2D_HISTOGRAM:
	    h2_s = (hs2DHist *) h_s;
	    idc = histo_create_2d_hist(uid, title, category,
				       h2_s->xLabel, h2_s->yLabel, h2_s->zLabel,
				       h2_s->nXBins, h2_s->nYBins,
				       h2_s->xMin, h2_s->xMax , h2_s->yMin, h2_s->yMax );
	    nb2 = h2_s->nXBins * h2_s->nYBins;   
	    valt = (float *) malloc (nb2*sizeof(float));
	    if (h2_s->pErrs != NULL)
	        errt = (float *) malloc (nb2*sizeof(float));
	    for (i=0; i<nb2; i++) {
		valt[i] = const_s * h2_s->bins[i];
		if (errt != NULL) errt[i] = const_s * h2_s->pErrs[i];
	    }
	    histo_2d_hist_block_fill(idc, valt, errt, NULL, 0);
	    free (valt);
	    if (errt != NULL) free(errt);
	    return idc;
	case HS_3D_HISTOGRAM:
	    h3_s = (hs3DHist *) h_s;
	    idc = histo_create_3d_hist(
		uid, title, category,
		h3_s->xLabel, h3_s->yLabel, h3_s->zLabel,
		h3_s->vLabel,
		h3_s->nXBins, h3_s->nYBins, h3_s->nZBins,
		h3_s->xMin, h3_s->xMax,
		h3_s->yMin, h3_s->yMax,
		h3_s->zMin, h3_s->zMax );
	    nb3 = h3_s->nXBins * h3_s->nYBins * h3_s->nZBins;   
	    valt = (float *) malloc (nb3*sizeof(float));
	    if (h3_s->pErrs != NULL)
	        errt = (float *) malloc (nb3*sizeof(float));
	    for (i=0; i<nb3; i++) {
		valt[i] = const_s * h3_s->bins[i];
		if (errt != NULL) errt[i] = const_s * h3_s->pErrs[i];
	    }
	    histo_3d_hist_block_fill(idc, valt, errt, NULL, 0);
	    free (valt);
	    if (errt != NULL) free(errt);
	    return idc;
	default : 
	    fprintf (stderr,
		     "hs_sum_histograms: Item with id = %d is not a histogram\n", i_s);
	    return -1;
	}
    }

    if (h_1->type != h_2->type) { 
	fprintf (stderr,
		 "hs_sum_histograms: Items (id\'s = %d, %d) are not of the same type\n",
		 id1, id2);
	return -1;
    }
	       
    switch(h_1->type) {
    case HS_1D_HISTOGRAM:
	h1_1 = (hs1DHist *) h_1;
	h1_2 = (hs1DHist *) h_2;
	if ((h1_1->nBins != h1_2->nBins) || 
	    (h1_1->min != h1_2->min) || (h1_1->max != h1_2->max)) {
	    fprintf (stderr,
		     "hs_sum_histograms: Items (id\'s = %d, %d) have inconsistent binning\n ",
		     id1, id2);
	    return -1;
	} 
	idc = histo_create_1d_hist(uid, title, category,
				   h1_1->xLabel, h1_1->yLabel, 
				   h1_1->nBins, h1_1->min, h1_1->max);    
	valt = (float *) malloc (h1_1->nBins*sizeof(float));
	if ((h1_1->pErrs != NULL) || (h1_2->pErrs != NULL))
	    errt = (float *) malloc (h1_1->nBins*sizeof(float));
	for (i=0; i<h1_1->nBins; i++) {
	    valt[i] = const1 * h1_1->bins[i] + 
		const2 * h1_2->bins[i];
	    if (errt != NULL) {
		vv = 0.;
		if (h1_1->pErrs !=NULL)
		    vv += (double) ( const1 * h1_1->pErrs[i])  * 
			( const1 * h1_1->pErrs[i]);
		if (h1_2->pErrs !=NULL)
		    vv += (double) ( const2 * h1_2->pErrs[i])  * 
			( const2 * h1_2->pErrs[i]);
		errt[i] = (float) sqrt(vv);
	                         
	    }
	}
	histo_1d_hist_block_fill(idc, valt, errt, NULL);
	free (valt);
	if (errt != NULL) free(errt);
	return idc;
    case HS_2D_HISTOGRAM:
	h2_1 = (hs2DHist *) h_1;
	h2_2 = (hs2DHist *) h_2;
	if ((h2_1->nXBins != h2_2->nXBins) ||
	    (h2_1->nYBins != h2_2->nYBins) ||
	    (h2_1->xMin != h2_2->xMin) || 
	    (h2_1->xMax != h2_2->xMax) ||
	    (h2_1->yMin != h2_2->yMin) || 
	    (h2_1->yMax != h2_2->yMax)) {
	    fprintf (stderr,
		     "hs_sum_histograms: Items (id\'s = %d, %d) have inconsistent binning \n ",
		     id1, id2);
	    return -1;
	}      
	idc = histo_create_2d_hist(uid, title, category,
				   h2_1->xLabel, h2_1->yLabel, h2_1->zLabel,
				   h2_1->nXBins, h2_1->nYBins,
				   h2_1->xMin, h2_1->xMax , h2_1->yMin, h2_1->yMax );
	nb2 = h2_1->nXBins * h2_1->nYBins;   
	valt = (float *) malloc (nb2*sizeof(float));
	if ((h2_1->pErrs != NULL) || (h2_2->pErrs != NULL)) 
	    errt = (float *) malloc (nb2*sizeof(float));
	for (i=0; i<nb2; i++) {
	    valt[i] = const1 * h2_1->bins[i] + 
		const2 * h2_2->bins[i];
	    if (errt != NULL) {
		vv = 0.;
		if (h2_1->pErrs !=NULL)
		    vv += (double) ( const1 * h2_1->pErrs[i])  * 
			( const1 * h2_1->pErrs[i]);
		if (h2_2->pErrs !=NULL)
		    vv += (double) ( const2 * h2_2->pErrs[i])  * 
			( const2 * h2_2->pErrs[i]);
		errt[i] = (float) sqrt(vv);
	                         
	    }
	}
	histo_2d_hist_block_fill(idc, valt, errt, NULL, 0);
	free (valt);
	if (errt != NULL) free(errt);
	return idc;
    case HS_3D_HISTOGRAM:
	h3_1 = (hs3DHist *) h_1;
	h3_2 = (hs3DHist *) h_2;
	if ((h3_1->nXBins != h3_2->nXBins) ||
	    (h3_1->nYBins != h3_2->nYBins) ||
	    (h3_1->nZBins != h3_2->nZBins) ||
	    (h3_1->xMin != h3_2->xMin) || 
	    (h3_1->xMax != h3_2->xMax) ||
	    (h3_1->zMin != h3_2->zMin) || 
	    (h3_1->zMax != h3_2->zMax) ||
	    (h3_1->yMin != h3_2->yMin) || 
	    (h3_1->yMax != h3_2->yMax)) {
	    fprintf (stderr,
		     "hs_sum_histograms: Items (id\'s = %d, %d) have inconsistent binning \n ",
		     id1, id2);
	    return -1;
	}      
	idc = histo_create_3d_hist(
	    uid, title, category,
	    h3_1->xLabel, h3_1->yLabel, h3_1->zLabel, h3_1->vLabel,
	    h3_1->nXBins, h3_1->nYBins, h3_1->nZBins,
	    h3_1->xMin, h3_1->xMax, h3_1->yMin,
	    h3_1->yMax, h3_1->zMin, h3_1->zMax);
	nb3 = h3_1->nXBins * h3_1->nYBins * h3_1->nZBins; 
	valt = (float *) malloc (nb3*sizeof(float));
	if ((h3_1->pErrs != NULL) || (h3_2->pErrs != NULL)) 
	    errt = (float *)malloc(nb3*sizeof(float));
	for (i=0; i<nb3; i++) {
	    valt[i] = const1 * h3_1->bins[i] + 
		const2 * h3_2->bins[i];
	    if (errt != NULL) {
		vv = 0.;
		if (h3_1->pErrs !=NULL)
		    vv += (double) ( const1 * h3_1->pErrs[i])  * 
			( const1 * h3_1->pErrs[i]);
		if (h3_2->pErrs !=NULL)
		    vv += (double) ( const2 * h3_2->pErrs[i])  * 
			( const2 * h3_2->pErrs[i]);
		errt[i] = (float) sqrt(vv);
	    }
	}
	histo_3d_hist_block_fill(idc, valt, errt, NULL, 0);
	free (valt);
	if (errt != NULL) free(errt);
	return idc;
    default : 
	fprintf (stderr,
		 "hs_sum_histograms: Items %d, %d are not histograms \n",
		 id1, id2);
	return -1;
    }
}

int histo_multdiv_histograms(int uid, const char *title, const char *category, 
                   int oper, int id1, int id2, float const1)
/*
   Create a new Histogram (1D or 2D) whose data is the multiplication
   (ioper = 0) or division ( ioper != 0), bin by bin,
   of 2 histograms refered by id1, id2.  The bin content of the new
   histogram will be equal to const * bin_id1 * bin_id2 (ioper = 0) 
   or const1 * bin_ib1/bin_ib2 ( ioper != 0).    If one of the ids
   is specified as 0, the other histogram will just be multiplied by the
   constant.
   
	uid		User Identification for the newly created histogram
	title, Category	It's title and category. See hs_create* routines. 
   
   	id1, id2	HistoScope ids of the histograms to multiply.  One
   			of the ids may be specified as 0.
   	const1		A constant to multiply the contents of the histograms.
   	ioper           Operation : is 0, multiply, is != 0, divide.

   	Return Value:	The id of a new histogram containing the result
   			of the operation, or -1 if an error occured.  			
*/
{
    hs1DHist *h1_1, *h1_2, *h1_s = NULL;
    hs2DHist *h2_1, *h2_2, *h2_s = NULL;
    hs3DHist *h3_1, *h3_2, *h3_s = NULL;
    hsGeneral *h_1, *h_2, *h_s = NULL ;
    int i, idc, i_s = 0, nb2, nb3;
    float *valt, *errt = NULL;
    double vv;

    if ((id1 <= 0) &&(id2 <= 0) ) {
	if (oper == 0)
	    fprintf (stderr,
		     "hs_multiply_histograms: invalid id addresses %d %d \n",id1, id2);
	else
	    fprintf (stderr,
		     "hs_divide_histograms: invalid id addresses %d %d \n",id1, id2);
	return -1;
    }

    h_1 = h_2 = NULL;
    if (id1 > 0) h_1 = (hsGeneral *) GetItemByPtrID(id1);
    if (id2 > 0) h_2 = (hsGeneral *) GetItemByPtrID(id2);
    if ((h_1 == NULL) && (h_2 == NULL)) {
	if (oper == 0) 
	    fprintf (stderr,
		     "hs_multiply_histograms: invalid id addresses, non existant histograms %d %d \n",
		     id1, id2);
	else 
	    fprintf (stderr,
		     "hs_divide_histograms: invalid id addresses, non existant histograms %d %d \n",
		     id1, id2);
	return -1;
    }

    if ((h_1 == NULL) && (h_2 != NULL)) {
	h_s = h_2;
	i_s = id2;
    }  
    if (((h_2 == NULL) && (h_1 != NULL))) {
	h_s = h_1;
	i_s = id1;
    }  
    if (h_s != NULL) {
        switch(h_s->type) {
	case HS_1D_HISTOGRAM:
	    h1_s = (hs1DHist *) h_s;
	    idc = histo_create_1d_hist(uid, title, category,
				       h1_s->xLabel, h1_s->yLabel, 
				       h1_s->nBins, h1_s->min, h1_s->max);    
	    valt = (float *) malloc (h1_s->nBins*sizeof(float));
	    if (h1_s->pErrs != NULL)
	        errt = (float *) malloc (h1_s->nBins*sizeof(float));
	    for (i=0; i<h1_s->nBins; i++) {
		valt[i] = const1 * h1_s->bins[i];
		if (errt != NULL) errt[i] = const1 * h1_s->pErrs[i];
	    }
	    histo_1d_hist_block_fill(idc, valt, errt, NULL);
	    free (valt);
	    if (errt != NULL) free(errt);
	    return idc;
	case HS_2D_HISTOGRAM:
	    h2_s = (hs2DHist *) h_s;
	    idc = histo_create_2d_hist(uid, title, category,
				       h2_s->xLabel, h2_s->yLabel, h2_s->zLabel,
				       h2_s->nXBins, h2_s->nYBins,
				       h2_s->xMin, h2_s->xMax , h2_s->yMin, h2_s->yMax );
	    nb2 = h2_s->nXBins * h2_s->nYBins;   
	    valt = (float *) malloc (nb2*sizeof(float));
	    if (h2_s->pErrs != NULL)
	        errt = (float *) malloc (nb2*sizeof(float));
	    for (i=0; i<nb2; i++) {
		valt[i] = const1 * h2_s->bins[i];
		if (errt != NULL) errt[i] = const1 * h2_s->pErrs[i];
	    }
	    histo_2d_hist_block_fill(idc, valt, errt, NULL, 0);
	    free (valt);
	    if (errt != NULL) free(errt);
	    return idc;
	case HS_3D_HISTOGRAM:
	    h3_s = (hs3DHist *) h_s;
	    idc = histo_create_3d_hist(
		uid, title, category,
		h3_s->xLabel, h3_s->yLabel, h3_s->zLabel, h3_s->vLabel,
		h3_s->nXBins, h3_s->nYBins, h3_s->nZBins,
		h3_s->xMin, h3_s->xMax, h3_s->yMin,
		h3_s->yMax, h3_s->zMin, h3_s->zMax);
	    nb3 = h3_s->nXBins * h3_s->nYBins * h3_s->nZBins;   
	    valt = (float *) malloc (nb3*sizeof(float));
	    if (h3_s->pErrs != NULL)
	        errt = (float *) malloc (nb3*sizeof(float));
	    for (i=0; i<nb3; i++) {
		valt[i] = const1 * h3_s->bins[i];
		if (errt != NULL) errt[i] = const1 * h3_s->pErrs[i];
	    }
	    histo_3d_hist_block_fill(idc, valt, errt, NULL, 0);
	    free (valt);
	    if (errt != NULL) free(errt);
	    return idc;
	default:
	    if (oper == 0) fprintf (
		stderr,
		"hs_multiply_histograms: item id = %d is not a histogram \n", i_s);
	    else fprintf (
		stderr,
		"hs_divide_histograms: item id = %d is not a histogram \n", i_s);
	    return -1;
	}
    }
    if (h_1->type != h_2->type) { 
	if (oper == 0) 
	    fprintf (stderr,
		     "hs_multiply_histograms: item id = %d, id = %d are not of the same type \n",
		     id1, id2); 
	else  
	    fprintf (stderr,
		     "hs_divide_histograms: item id = %d, id = %d are not of the same type \n",
		     id1, id2); 
	return -1;
    }
	       
    switch(h_1->type) {
    case HS_1D_HISTOGRAM:
	h1_1 = (hs1DHist *) h_1;
	h1_2 = (hs1DHist *) h_2;
	if ((h1_1->nBins != h1_2->nBins) || 
	    (h1_1->min != h1_2->min) || (h1_1->max != h1_2->max)) {
	    if (oper == 0)
		fprintf (stderr,
			 "hs_multiply_histograms: item id = %d, id = %d have inconsistent binning \n ",
			 id1, id2);
	    else
		fprintf (stderr,
			 "hs_divide_histograms: item id = %d, id = %d have inconsistent binning \n ",
			 id1, id2); 
	    return -1;
	}
	idc = histo_create_1d_hist(uid, title, category,
				   h1_1->xLabel, h1_1->yLabel, 
				   h1_1->nBins, h1_1->min, h1_1->max);    
	valt = (float *) malloc (h1_1->nBins*sizeof(float));
	if ((h1_1->pErrs != NULL) || (h1_2->pErrs != NULL))
	    errt = (float *) malloc (h1_1->nBins*sizeof(float));
	for (i=0; i<h1_1->nBins; i++) {
	    if (oper == 0)  
		valt[i] = const1 * h1_1->bins[i] *  h1_2->bins[i];
	    else {
		if (h1_2->bins[i] != 0.) 
		    valt[i] = const1 * h1_1->bins[i] / h1_2->bins[i];
		else 
		    valt[i] = const1 * h1_1->bins[i];
	    }   
	    if (errt != NULL) {
		vv = 0.;
		if (oper == 0) {
		    if (h1_1->pErrs !=NULL)
			vv += (double) ( h1_1->pErrs[i]  *  h1_1->pErrs[i] * 
					 h1_2->bins[i] * h1_2->bins[i] ) ;
		    if (h1_2->pErrs !=NULL)
			vv += (double) ( h1_2->pErrs[i]  *  h1_2->pErrs[i] * 
					 h1_1->bins[i] * h1_1->bins[i] ) ;
		} else {
		    if (h1_1->pErrs !=NULL && h1_2->bins[i] != 0.)
			vv += (double) ( h1_1->pErrs[i]  *  h1_1->pErrs[i] / 
					 pow((double) h1_2->bins[i], (double) 2.));
		    if (h1_2->pErrs !=NULL && h1_2->bins[i] != 0.)
			vv += (double) ( h1_2->pErrs[i]  *  h1_2->pErrs[i] * 
					 h1_1->bins[i] * h1_1->bins[i] / 
					 pow((double) h1_2->bins[i], (double) 4.));
		}
		errt[i] = (float) sqrt(vv);
		errt[i] = errt[i] * const1;
	                         
	    }
	}
	histo_1d_hist_block_fill(idc, valt, errt, NULL);
	free (valt);
	if (errt != NULL) free(errt);
	return idc;
    case HS_2D_HISTOGRAM:
	h2_1 = (hs2DHist *) h_1;
	h2_2 = (hs2DHist *) h_2;
	if ((h2_1->nXBins != h2_2->nXBins) ||
	    (h2_1->nYBins != h2_2->nYBins) ||
	    (h2_1->xMin != h2_2->xMin) || 
	    (h2_1->xMax != h2_2->xMax) ||
	    (h2_1->yMin != h2_2->yMin) || 
	    (h2_1->yMax != h2_2->yMax)) {
	    fprintf (stderr,
		     "hs_multiply_histograms: item id = %d, id = %d have inconsistent binning \n ",
		     id1, id2);
	    return -1;
	}      
	idc = histo_create_2d_hist(uid, title, category,
				   h2_1->xLabel, h2_1->yLabel, h2_1->zLabel,
				   h2_1->nXBins, h2_1->nYBins,
				   h2_1->xMin, h2_1->xMax , h2_1->yMin, h2_1->yMax );
	nb2 = h2_1->nXBins * h2_1->nYBins;   
	valt = (float *) malloc (nb2*sizeof(float));
	if ((h2_1->pErrs != NULL) || (h2_2->pErrs != NULL)) 
	    errt = (float *) malloc (nb2*sizeof(float));
	for (i=0; i<nb2; i++) {
	    if (oper == 0)  
		valt[i] = const1 * h2_1->bins[i] *  h2_2->bins[i];
	    else {
		if (h2_2->bins[i] != 0.) 
		    valt[i] = const1 * h2_1->bins[i] / h2_2->bins[i];
		else 
		    valt[i] = const1 * h2_1->bins[i];
	    }   
	    if (errt != NULL) {
		vv = 0.;
		if (oper == 0) {
		    if (h2_1->pErrs !=NULL)
			vv += (double) ( h2_1->pErrs[i]  *  h2_1->pErrs[i] * 
					 h2_2->bins[i] * h2_2->bins[i] ) ;
		    if (h2_2->pErrs !=NULL)
			vv += (double) ( h2_2->pErrs[i]  *  h2_2->pErrs[i] * 
					 h2_1->bins[i] * h2_1->bins[i] ) ;
		} else {
		    if (h2_1->pErrs !=NULL && h2_2->bins[i] != 0.)
			vv += (double) ( h2_1->pErrs[i]  *  h2_1->pErrs[i] / 
					 pow((double) h2_2->bins[i], (double) 2.));
		    if (h2_2->pErrs !=NULL && h2_2->bins[i] != 0.)
			vv += (double) ( h2_2->pErrs[i]  *  h2_2->pErrs[i] * 
					 h2_1->bins[i] * h2_1->bins[i] / 
					 pow((double) h2_2->bins[i], (double) 4.));
		}
		errt[i] = (float) sqrt(vv);
		errt[i] = errt[i] * const1;
	                         
	    }
	}
	histo_2d_hist_block_fill(idc, valt, errt, NULL, 0);
	free (valt);
	if (errt != NULL) free(errt);
	return idc;
    case HS_3D_HISTOGRAM:
	h3_1 = (hs3DHist *) h_1;
	h3_2 = (hs3DHist *) h_2;
	if ((h3_1->nXBins != h3_2->nXBins) ||
	    (h3_1->nYBins != h3_2->nYBins) ||
	    (h3_1->xMin != h3_2->xMin) || 
	    (h3_1->xMax != h3_2->xMax) ||
	    (h3_1->yMin != h3_2->yMin) || 
	    (h3_1->yMax != h3_2->yMax)) {
	    fprintf (stderr,
		     "hs_multiply_histograms: item id = %d, id = %d have inconsistent binning \n ",
		     id1, id2);
	    return -1;
	}      
	idc = histo_create_3d_hist(
	    uid, title, category,
	    h3_1->xLabel, h3_1->yLabel, h3_1->zLabel, h3_1->vLabel,
	    h3_1->nXBins, h3_1->nYBins, h3_1->nZBins,
	    h3_1->xMin, h3_1->xMax,
	    h3_1->yMin, h3_1->yMax,
	    h3_1->zMin, h3_1->zMax);
	nb3 = h3_1->nXBins * h3_1->nYBins * h3_1->nZBins;
	valt = (float *) malloc (nb3*sizeof(float));
	if ((h3_1->pErrs != NULL) || (h3_2->pErrs != NULL)) 
	    errt = (float *) malloc (nb3*sizeof(float));
	for (i=0; i<nb3; i++) {
	    if (oper == 0)  
		valt[i] = const1 * h3_1->bins[i] *  h3_2->bins[i];
	    else {
		if (h3_2->bins[i] != 0.) 
		    valt[i] = const1 * h3_1->bins[i] / h3_2->bins[i];
		else 
		    valt[i] = const1 * h3_1->bins[i];
	    }   
	    if (errt != NULL) {
		vv = 0.;
		if (oper == 0) {
		    if (h3_1->pErrs !=NULL)
			vv += (double) ( h3_1->pErrs[i]  *  h3_1->pErrs[i] * 
					 h3_2->bins[i] * h3_2->bins[i] ) ;
		    if (h3_2->pErrs !=NULL)
			vv += (double) ( h3_2->pErrs[i]  *  h3_2->pErrs[i] * 
					 h3_1->bins[i] * h3_1->bins[i] ) ;
		} else {
		    if (h3_1->pErrs !=NULL && h3_2->bins[i] != 0.)
			vv += (double) ( h3_1->pErrs[i]  *  h3_1->pErrs[i] / 
					 pow((double) h3_2->bins[i], (double) 2.));
		    if (h3_2->pErrs !=NULL && h3_2->bins[i] != 0.)
			vv += (double) ( h3_2->pErrs[i]  *  h3_2->pErrs[i] * 
					 h3_1->bins[i] * h3_1->bins[i] / 
					 pow((double) h3_2->bins[i], (double) 4.));
		}
		errt[i] = (float) sqrt(vv);
		errt[i] = errt[i] * const1;
	    }
	}
	histo_3d_hist_block_fill(idc, valt, errt, NULL, 0);
	free (valt);
	if (errt != NULL) free(errt);
	return idc;
    default : 
	if (oper == 0)
	    fprintf (stderr,
		     "hs_multiply_histograms: items %d, %d are not histograms\n",
		     id1, id2);
	else
	    fprintf (stderr,
		     "hs_divide_histograms: items %d, %d are not histograms\n",
		     id1, id2);
	return -1;
    }
}	  	


void histo_sum_category(const char *cat_top1, const char *cat_top2,
                        const char *prefixsum) 
/*
   Create a collection of new histograms (1D or 2D), or NTuple,  
   based on two existing categories. All item within this 
   category are considered, and it is assumed that the categories have
   parallel trees, e.g., beside a top prefix ( as given from 
   hs_read_file call), the uid/subcategories and binning properties
   are indentical. Items with uid = 0 are skipped. 
   For instance, cat_top1 can be set to 'run23', cat_top2
   to 'run25', prefixsum 'run23_25', then, 
   if histograms  that exist both in 'run23/tracking' and in 
   'run25/tracking' with identical static properties, a histogram 
   refered by the same uid will be created in 'run23_25/tracking'
   As this routine is intend to accumulate statistics,
   no arbitrary constants is provided. 
    
	cat_top1, cat_top2    The top level for  the categories to be 
			      summed.  These canNOT be NULL or empty 
			      (Uncategorized) or contain "...".
	prefixsum	      The prefix for the newly created category.
*/
{
    hsGeneral *h_1, *h_2;
    int i, id1, id2, ltc1, ltc2, ltcr; 
    int nitems, nmatch1, *idlis;
    int uidt;
    char catt[HS_MAX_CATEGORY_LENGTH], catr[HS_MAX_CATEGORY_LENGTH];
    char *cat_sub;

    /* Validate categories and prefix specified */
    if (!CheckValidCategory(cat_top1, "hs_sum_category", 0)) return;
    ltc1 = strlen(cat_top1);
    if (!CheckValidCategory(cat_top2, "hs_sum_category", 0)) return;
    ltc2 = strlen(cat_top2);
    strcpy(catt, cat_top2);
    if (!CheckValidCategory(prefixsum, "hs_sum_category", 0)) return;
    if (cat_top1 == NULL || cat_top2 == NULL || prefixsum == NULL ||
    		strcspn(cat_top1, " ") <= 0 || strcspn(cat_top2, " ") <= 0 ||
    		strcspn(prefixsum, " ") <= 0) {
    	fprintf(stderr,
    		"hs_sum_category: Invalid top category or prefix specified\n");
    	return;
    }
    
    /* Get list of items in first top category */
    ltcr = strlen(prefixsum);
    strcpy(catr, prefixsum);
    nitems = histo_num_items();
    if (nitems <= 0 ) return;
    idlis = (int *) malloc(nitems * sizeof(int));
    nmatch1 = list_items_topc( cat_top1, idlis, nitems);
    if (nmatch1 <= 0) {
      fprintf (stderr, 
       "hs_sum_category : No item found in the first Category %s \n", cat_top1);
      free(idlis);
      return;
    }
    
    /* Go through each item in first top category, looking for histograms/
       ntuples with matching uids in second top category to sum/merge.  */
    for (i=0; i<nmatch1; i++) {
       id1 = idlis[i];
       h_1 = (hsGeneral *) GetItemByPtrID(id1);
       if ((h_1->type != HS_1D_HISTOGRAM) && 
           (h_1->type != HS_2D_HISTOGRAM) &&
           (h_1->type != HS_3D_HISTOGRAM) &&
           (h_1->type != HS_NTUPLE)) continue; 
       cat_sub = (h_1->category + ltc1);
       if ((int)(strlen(h_1->category)) > ltc1) {
              if ((strlen(h_1->category) - ltc1 + ltc2) >= 
                         HS_MAX_CATEGORY_LENGTH) {
                 fprintf (stderr,
         "hs_sum_category : Category name too long, item  uid = %d skipped \n",
                           h_1->uid);
                 continue;		/* skip, category to find is too long */
              }
              
              /* catt = category of item to find in top category 2 */      
              strcpy((catt+ltc2), cat_sub);
              if ((strlen(h_1->category) - ltc1 + ltcr) >= 
                         HS_MAX_CATEGORY_LENGTH) {
                  fprintf (stderr,
         "hs_sum_category : Category name too long, item  uid = %d skipped \n",
                         h_1->uid);
                  continue;		/* skip, new category is too long    */
              }
              
              /* catr = category of new item to create from sum/merge */     
              strcpy((catr+ltcr), cat_sub);
       }
       uidt = h_1->uid;
       if (uidt == 0)
           continue;			/* skip if item's uid is 0           */
           
       /* find item in top category 2 with same uid and sub-category */ 
       id2 = histo_id(uidt, catt);
       if (id2 <= 0)
           continue;			/* skip if no item with matching uid */
       h_2 = (hsGeneral *) GetItemByPtrID(id2);
       if (h_2->type != h_1->type) 
           continue;			/* skip if items are different types */
       if (h_1->type != HS_NTUPLE)
           histo_sum_histograms(uidt, h_1->title, catr, id1, id2, 1., 1.);
       else 
           histo_merge_entries(uidt, h_1->title, catr, id1, id2);
     } /* End of big loop */
     free(idlis);
     return;
}  

void histo_sum_file(const char *file, const char *cat_top,
                    const char *prefixsum)
/*
   Read all of the items from file, sum histograms and/or
   merge Ntuple with those existing under the top category refered by
   cat_top and store all newly created items in a category specified
   by a prefix.  Original items found in file1 will be deleted.
   
   file	 		The input file name.
   
   cat_top		The top category for the items to be summed/merged.
   			CanNOT be NULL or empty or spaces (Uncategorized)
   			or contain "...".
   			
   prefixsum 		The category prefix for the result. It can be equal
   			to cat_top.
   
   Usage: Thus, to sum a bunch a files, one can read the first one by calling
   			hs_read_file(file1, 'allsum') and loop over the 
   			others, calling hs_sum_file(filen, 'allsum', 'allsum')
   			repetitively.
   			
   Known deficiency :  Assumes that there is no top category named 
   			"tmp_sum_file" or "tmp_sum_result". 
    
*/
{
    hsGeneral *h_1;
    int i, id1, ltcr; 
    int nitems, nmatch, *idlis;
    char catr[HS_MAX_CATEGORY_LENGTH+1];
    char *cat_sub;
    int llt = strlen("tmp_sum_result");

    /* Validate top category and prefix specified */
    if (!CheckValidCategory(cat_top, "hs_sum_file", 0)) return;
    if (!CheckValidCategory(prefixsum, "hs_sum_file", 0)) return;
    if (cat_top == NULL || prefixsum == NULL ||
    		strcspn(cat_top, " ") <= 0 || strcspn(prefixsum, " ") <= 0) {
    	fprintf(stderr,
    		"hs_sum_file: Invalid top category or prefix specified\n");
    	return;
    }
    
    ltcr = strlen(prefixsum);		/* length top category result */
    strcpy(catr, prefixsum);		/* category result     */
    
    /* read all items in file, prefixing categories with "tmp_sum_file" */
    if (histo_read_file(file, "tmp_sum_file") <= 0) return;
    
    
    /* Sum items read with items in top category.  However if result prefix
       and top category are the same, do fancy footwork to avoid clashes */
    if (strcmp(cat_top, prefixsum) != 0) { /* easy case, just sum */
      histo_sum_category(cat_top, "tmp_sum_file", prefixsum);
    }
    else {	/* prefix and top category are the same */
    /* Need to rename to avoid clashes, find yet another temporary name... */
      nitems = histo_num_items();
      idlis = (int *) malloc(nitems * sizeof(int));
      histo_sum_category(cat_top,"tmp_sum_file", "tmp_sum_result");
      
      /* Get back a list of all the items so can change category and
         delete the old histograms that were summed by histo_sum_category */
      nmatch = list_items_topc("tmp_sum_result" , idlis, nitems);
      for (i=0; i<nmatch; i++) {
         int dupID;
         hsGeneral *dupHist;
         id1 = idlis[i];
         h_1 = (hsGeneral *) GetItemByPtrID(id1);
         cat_sub = (h_1->category + llt);
         if ((strlen(h_1->category) - llt + ltcr) >= HS_MAX_CATEGORY_LENGTH) {
            fprintf (stderr,
          "hs_sum_file : Category name too long, item  uid = %d not renamed \n",
                           h_1->uid);
            continue;
         }
         strcpy((catr+ltcr), cat_sub);
         dupID = histo_id(h_1->uid, catr);	/* is item a duplicate? */
         if (dupID > 0) {
             dupHist = GetItemByPtrID(dupID);	/*    - yes		*/
             if (dupHist->type == h_1->type)
             	histo_delete(dupID);
             else {
             	fprintf (stderr,
  "hs_sum_file : Found inconsistency: item  uid = %d, category %s not summed\n",
                           h_1->uid, catr);
                continue;
             }
         }
         histo_change_category(id1, catr);             
      }
      free(idlis);
    }
    
    /* Delete tmp_sum_file items */
    histo_delete_category("tmp_sum_file...");
    return;
}

static int list_items_topc(const char *catTop, int *idList, int maxItemsToRtn)
/* 
	Get a list of all the items found in a top category. 
	
	catTop         The top category
	maxItemsToRtn  The maximum number of items found to put in idList.
	idList 	       The list of items; it is assumed that this array 
			 has been dimensioned to at least maxItemsToRtn ints. 
	
	Returns number of items found; may be > maxItemsToRtn.
			            
*/
{
    int i, lenCat, numFound, numRtnd, j;
    int *idSlot;
    hsGeneral **itmPtr;
    hsGeneral *item;
    
    numFound = 0;
    numRtnd = 0;
    idSlot = idList;
    itmPtr = HistoPtrList;
    lenCat = strlen(catTop);
    for (i=0; i < NumOfItems; i++, itmPtr++) {
	if (*itmPtr == NULL) 
	    continue;
	item = *itmPtr;
	j = strncmp(item->category, catTop, lenCat); 
	if ( j == 0 && (item->category[lenCat] == '/' 
	  		 || item->category[lenCat] == '\0') ) {
	    numFound++; 
 	    if (numRtnd >= maxItemsToRtn) 
 	    	continue;
            *idSlot = item->id;
	    idSlot++;
	    numRtnd++;
        }
    }
    return numFound;
}

int histo_1d_hist_derivative(int uid, const char *title, const char *category,
                             int id)
/*
   Create a new Histogram (1D) whose data is the derivative of histogram 
   specified by id. This derivative is computed bin by bin:
    
   	Dh/db = (H(j) - H(j-1))/bin_size.
   	
   If the input histogram has error bars, the errors are computed assuming that
   there is no statistical correlation between bins.
      
	uid		User Identification for the newly created histogram
	title, Category	It's title and category. See hs_create* routines. 
   
   	id     	        HistoScope id  of the histogram for which one 
   			wants to compute the derivative. 
   			
   	Return Value:	The id of a new histogram containing this derivative.
*/
{
    hs1DHist *h1;
    hsGeneral *h;
    int i, idc, nlx, nly;
    float db;
    float *valt, *errt = NULL;
    char *NewyLabel;
    double vv;
    
    
    if ((id <= 0) ) {
        fprintf (stderr,
             "hs_1d_derivative: Invalid id : %d  \n", id);
        return -1;
    }
    h = (hsGeneral *) GetItemByPtrID(id);
    if  (h == NULL) {
        fprintf (stderr,
            "hs_1d_derivative: Invalid id: %d, histogram does not exist\n",
            id);
        return -1;
    }
    if (h->type != HS_1D_HISTOGRAM) {
        fprintf (stderr,
            "hs_1d_derivative: histogram id: %d, is not a 1D histogram \n",
             h->id);
        return -1;
    }
    h1 = (hs1DHist *) h;
    db = (h1->max - h1->min)/h1->nBins;
    nlx = strlen(h1->xLabel);
    nly = strlen(h1->yLabel);
    NewyLabel = (char *) malloc((nlx+3+nly)*sizeof(char));
    strcpy(NewyLabel,h1->yLabel); 
    if ((nlx+1+nly) <  HS_MAX_LABEL_LENGTH) {
      strcat(NewyLabel, "/");
      strcat(NewyLabel, h1->xLabel);
    } 
    idc = histo_create_1d_hist(uid, title, category,
	                    h1->xLabel, NewyLabel, 
	                    (h1->nBins - 1),
	                     (h1->min + db/2.), (h1->max - db/2.));
    free (NewyLabel);
    if (idc <= 0 ) {
        fprintf (stderr,
             "hs_1d_derivative: resulting histogram  could not be created \n");
        return -1;
    }
         
    valt = (float *) malloc ((h1->nBins-1)*sizeof(float));
    if (h1->pErrs != NULL)
	errt = (float *) malloc ((h1->nBins-1)*sizeof(float));
    for (i=1; i<h1->nBins; i++) {
       valt[(i-1)] = (h1->bins[i] - h1->bins[(i-1)])/db;
       if (errt != NULL) {
                   vv = (double) (h1->pErrs[i] * h1->pErrs[i] +
                                 h1->pErrs[(i-1)] * h1->pErrs[(i-1)]);
                   errt[(i-1)] = ( ( float) sqrt(vv)) / db;
	       }
     }
    histo_1d_hist_block_fill(idc, valt, errt, NULL);
    free (valt);
    if (errt != NULL) free (errt);
    return idc;
}	  	


/* This code should become a part of 'histoApiAryH.c' Only 'histo_unary_op'
   and 'histo_binary_op' functions should be in the API. */

static int 
histo_unary_op_1d(hs1DHist *item, int uid, 
		  const char *title, const char *category,
		  float (*f_user_data)(float, float),
		  float (*f_user_errors)(float, float))
{
  int i, id, nbins;
  hs1DHist *result;

  id = histo_create_1d_hist(uid, title, category,
			    item->xLabel, item->yLabel, 
			    item->nBins, item->min, item->max);
  if (id <= 0)
    return id;
  
  nbins = item->nBins;
  result = (hs1DHist *) GetItemByPtrID(id);

  if (f_user_errors)
  {
    if ((result->pErrs = (float *)malloc(nbins*sizeof(float))) == NULL)
    {
      fprintf(stderr, "histo_unary_op: out of memory\n");
      histo_delete(id);
      return 0;
    }
    if (item->pErrs)
    {
      for (i=0; i<nbins; i++)
      {
	result->bins[i] = f_user_data(item->bins[i], item->pErrs[i]);
	result->pErrs[i] = f_user_errors(item->bins[i], item->pErrs[i]);
      }
    }
    else
    {
      for (i=0; i<nbins; i++)
      {
	result->bins[i] = f_user_data(item->bins[i], 0.f);
	result->pErrs[i] = f_user_errors(item->bins[i], 0.f);
      }
    }
  }
  else
  {
    if (item->pErrs)
    {
      for (i=0; i<nbins; i++)
	result->bins[i] = f_user_data(item->bins[i], item->pErrs[i]);
    }
    else
    {
      for (i=0; i<nbins; i++)
	result->bins[i] = f_user_data(item->bins[i], 0.f);
    }
  }

  SetHistResetFlag((hsGeneral *)result);
  return id;
}

static int 
histo_unary_op_2d(hs2DHist *item, int uid, 
		  const char *title, const char *category,
		  float (*f_user_data)(float, float),
		  float (*f_user_errors)(float, float))
{
  int i, id, nbins;
  hs2DHist *result;

  id = histo_create_2d_hist(uid, title, category,
			    item->xLabel, item->yLabel, item->zLabel,
			    item->nXBins, item->nYBins,
			    item->xMin, item->xMax , item->yMin, item->yMax );
  if (id <= 0)
    return id;
  
  nbins = item->nXBins*item->nYBins;
  result = (hs2DHist *) GetItemByPtrID(id);

  if (f_user_errors)
  {
    if ((result->pErrs = (float *)malloc(nbins*sizeof(float))) == NULL)
    {
      fprintf(stderr, "histo_unary_op: out of memory\n");
      histo_delete(id);
      return 0;
    }
    if (item->pErrs)
    {
      for (i=0; i<nbins; i++)
      {
	result->bins[i] = f_user_data(item->bins[i], item->pErrs[i]);
	result->pErrs[i] = f_user_errors(item->bins[i], item->pErrs[i]);
      }
    }
    else
    {
      for (i=0; i<nbins; i++)
      {
	result->bins[i] = f_user_data(item->bins[i], 0.f);
	result->pErrs[i] = f_user_errors(item->bins[i], 0.f);
      }
    }
  }
  else
  {
    if (item->pErrs)
    {
      for (i=0; i<nbins; i++)
	result->bins[i] = f_user_data(item->bins[i], item->pErrs[i]);
    }
    else
    {
      for (i=0; i<nbins; i++)
	result->bins[i] = f_user_data(item->bins[i], 0.f);
    }
  }

  SetHistResetFlag((hsGeneral *)result);
  return id;
}

static int 
histo_unary_op_3d(hs3DHist *item, int uid, 
		  const char *title, const char *category,
		  float (*f_user_data)(float, float),
		  float (*f_user_errors)(float, float))
{
  int i, id, nbins;
  hs3DHist *result;

  id = histo_create_3d_hist(
      uid, title, category,
      item->xLabel, item->yLabel, item->zLabel, item->vLabel,
      item->nXBins, item->nYBins, item->nZBins,
      item->xMin, item->xMax, item->yMin,
      item->yMax, item->zMin, item->zMax);
  if (id <= 0)
    return id;
  
  nbins = item->nXBins*item->nYBins*item->nZBins;
  result = (hs3DHist *) GetItemByPtrID(id);

  if (f_user_errors)
  {
    if ((result->pErrs = (float *)malloc(nbins*sizeof(float))) == NULL)
    {
      fprintf(stderr, "histo_unary_op: out of memory\n");
      histo_delete(id);
      return 0;
    }
    if (item->pErrs)
    {
      for (i=0; i<nbins; i++)
      {
	result->bins[i] = f_user_data(item->bins[i], item->pErrs[i]);
	result->pErrs[i] = f_user_errors(item->bins[i], item->pErrs[i]);
      }
    }
    else
    {
      for (i=0; i<nbins; i++)
      {
	result->bins[i] = f_user_data(item->bins[i], 0.f);
	result->pErrs[i] = f_user_errors(item->bins[i], 0.f);
      }
    }
  }
  else
  {
    if (item->pErrs)
    {
      for (i=0; i<nbins; i++)
	result->bins[i] = f_user_data(item->bins[i], item->pErrs[i]);
    }
    else
    {
      for (i=0; i<nbins; i++)
	result->bins[i] = f_user_data(item->bins[i], 0.f);
    }
  }

  SetHistResetFlag((hsGeneral *)result);
  return id;
}

static int 
histo_binary_op_1d(hs1DHist *item1, hs1DHist *item2, 
		   int uid, const char *title, const char *category,
		   float (*f_user_data)(float, float, float, float),
		   float (*f_user_errors)(float, float, float, float))
{
  int i, id, nbins;
  hs1DHist *result;

  id = histo_create_1d_hist(uid, title, category,
			    item1->xLabel, item1->yLabel, 
			    item1->nBins, item1->min, item1->max);
  if (id <= 0)
    return id;
  
  nbins = item1->nBins;
  result = (hs1DHist *) GetItemByPtrID(id);

  if (f_user_errors)
  {
    if ((result->pErrs = (float *)malloc(nbins*sizeof(float))) == NULL)
    {
      fprintf(stderr, "histo_binary_op: out of memory\n");
      histo_delete(id);
      return 0;
    }
    if (item1->pErrs && item2->pErrs)
    {
      for (i=0; i<nbins; i++)
      {
	result->bins[i] = f_user_data(item1->bins[i], item1->pErrs[i],
				      item2->bins[i], item2->pErrs[i]);
        result->pErrs[i] = f_user_errors(item1->bins[i], item1->pErrs[i],
					 item2->bins[i], item2->pErrs[i]);
      }
    }
    else if (item1->pErrs)
    {
      for (i=0; i<nbins; i++)
      {
	result->bins[i] = f_user_data(item1->bins[i], item1->pErrs[i],
				      item2->bins[i], 0.f);
        result->pErrs[i] = f_user_errors(item1->bins[i], item1->pErrs[i],
					 item2->bins[i], 0.f);
      }
    }
    else if (item2->pErrs)
    {
      for (i=0; i<nbins; i++)
      {
	result->bins[i] = f_user_data(item1->bins[i], 0.f,
				      item2->bins[i], item2->pErrs[i]);
        result->pErrs[i] = f_user_errors(item1->bins[i], 0.f,
					 item2->bins[i], item2->pErrs[i]);
      }
    }
    else
    {
      for (i=0; i<nbins; i++)
      {
	result->bins[i] = f_user_data(item1->bins[i], 0.f,
				      item2->bins[i], 0.f);
        result->pErrs[i] = f_user_errors(item1->bins[i], 0.f,
					 item2->bins[i], 0.f);
      }
    }
  }
  else
  {
    if (item1->pErrs && item2->pErrs)
    {
      for (i=0; i<nbins; i++)
	result->bins[i] = f_user_data(item1->bins[i], item1->pErrs[i],
				      item2->bins[i], item2->pErrs[i]);
    }
    else if (item1->pErrs)
    {
      for (i=0; i<nbins; i++)
	result->bins[i] = f_user_data(item1->bins[i], item1->pErrs[i],
				      item2->bins[i], 0.f);
    }
    else if (item2->pErrs)
    {
      for (i=0; i<nbins; i++)
	result->bins[i] = f_user_data(item1->bins[i], 0.f,
				      item2->bins[i], item2->pErrs[i]);
    }
    else
    {
      for (i=0; i<nbins; i++)
	result->bins[i] = f_user_data(item1->bins[i], 0.f,
				      item2->bins[i], 0.f);
    }
  }

  SetHistResetFlag((hsGeneral *)result);
  return id;
}

static int 
histo_binary_op_2d(hs2DHist *item1, hs2DHist *item2, 
		   int uid, const char *title, const char *category,
		   float (*f_user_data)(float, float, float, float),
		   float (*f_user_errors)(float, float, float, float))
{
  int i, id, nbins;
  hs2DHist *result;

  id = histo_create_2d_hist(uid, title, category,
			    item1->xLabel, item1->yLabel, item1->zLabel,
			    item1->nXBins, item1->nYBins,
			    item1->xMin, item1->xMax , item1->yMin, item1->yMax );
  if (id <= 0)
    return id;
  
  nbins = item1->nXBins*item1->nYBins;
  result = (hs2DHist *) GetItemByPtrID(id);

  if (f_user_errors)
  {
    if ((result->pErrs = (float *)malloc(nbins*sizeof(float))) == NULL)
    {
      fprintf(stderr, "histo_binary_op: out of memory\n");
      histo_delete(id);
      return 0;
    }
    if (item1->pErrs && item2->pErrs)
    {
      for (i=0; i<nbins; i++)
      {
	result->bins[i] = f_user_data(item1->bins[i], item1->pErrs[i],
				      item2->bins[i], item2->pErrs[i]);
        result->pErrs[i] = f_user_errors(item1->bins[i], item1->pErrs[i],
					 item2->bins[i], item2->pErrs[i]);
      }
    }
    else if (item1->pErrs)
    {
      for (i=0; i<nbins; i++)
      {
	result->bins[i] = f_user_data(item1->bins[i], item1->pErrs[i],
				      item2->bins[i], 0.f);
        result->pErrs[i] = f_user_errors(item1->bins[i], item1->pErrs[i],
					 item2->bins[i], 0.f);
      }
    }
    else if (item2->pErrs)
    {
      for (i=0; i<nbins; i++)
      {
	result->bins[i] = f_user_data(item1->bins[i], 0.f,
				      item2->bins[i], item2->pErrs[i]);
        result->pErrs[i] = f_user_errors(item1->bins[i], 0.f,
					 item2->bins[i], item2->pErrs[i]);
      }
    }
    else
    {
      for (i=0; i<nbins; i++)
      {
	result->bins[i] = f_user_data(item1->bins[i], 0.f,
				      item2->bins[i], 0.f);
        result->pErrs[i] = f_user_errors(item1->bins[i], 0.f,
					 item2->bins[i], 0.f);
      }
    }
  }
  else
  {
    if (item1->pErrs && item2->pErrs)
    {
      for (i=0; i<nbins; i++)
	result->bins[i] = f_user_data(item1->bins[i], item1->pErrs[i],
				      item2->bins[i], item2->pErrs[i]);
    }
    else if (item1->pErrs)
    {
      for (i=0; i<nbins; i++)
	result->bins[i] = f_user_data(item1->bins[i], item1->pErrs[i],
				      item2->bins[i], 0.f);
    }
    else if (item2->pErrs)
    {
      for (i=0; i<nbins; i++)
	result->bins[i] = f_user_data(item1->bins[i], 0.f,
				      item2->bins[i], item2->pErrs[i]);
    }
    else
    {
      for (i=0; i<nbins; i++)
	result->bins[i] = f_user_data(item1->bins[i], 0.f,
				      item2->bins[i], 0.f);
    }
  }

  SetHistResetFlag((hsGeneral *)result);
  return id;
}

static int 
histo_binary_op_3d(hs3DHist *item1, hs3DHist *item2, 
		   int uid, const char *title, const char *category,
		   float (*f_user_data)(float, float, float, float),
		   float (*f_user_errors)(float, float, float, float))
{
  int i, id, nbins;
  hs3DHist *result;

  id = histo_create_3d_hist(
      uid, title, category,
      item1->xLabel, item1->yLabel, item1->zLabel, item1->vLabel,
      item1->nXBins, item1->nYBins, item1->nZBins,
      item1->xMin, item1->xMax, item1->yMin,
      item1->yMax, item1->zMin, item1->zMax);
  if (id <= 0)
    return id;
  
  nbins = item1->nXBins*item1->nYBins*item1->nZBins;
  result = (hs3DHist *) GetItemByPtrID(id);

  if (f_user_errors)
  {
    if ((result->pErrs = (float *)malloc(nbins*sizeof(float))) == NULL)
    {
      fprintf(stderr, "histo_binary_op: out of memory\n");
      histo_delete(id);
      return 0;
    }
    if (item1->pErrs && item2->pErrs)
    {
      for (i=0; i<nbins; i++)
      {
	result->bins[i] = f_user_data(item1->bins[i], item1->pErrs[i],
				      item2->bins[i], item2->pErrs[i]);
        result->pErrs[i] = f_user_errors(item1->bins[i], item1->pErrs[i],
					 item2->bins[i], item2->pErrs[i]);
      }
    }
    else if (item1->pErrs)
    {
      for (i=0; i<nbins; i++)
      {
	result->bins[i] = f_user_data(item1->bins[i], item1->pErrs[i],
				      item2->bins[i], 0.f);
        result->pErrs[i] = f_user_errors(item1->bins[i], item1->pErrs[i],
					 item2->bins[i], 0.f);
      }
    }
    else if (item2->pErrs)
    {
      for (i=0; i<nbins; i++)
      {
	result->bins[i] = f_user_data(item1->bins[i], 0.f,
				      item2->bins[i], item2->pErrs[i]);
        result->pErrs[i] = f_user_errors(item1->bins[i], 0.f,
					 item2->bins[i], item2->pErrs[i]);
      }
    }
    else
    {
      for (i=0; i<nbins; i++)
      {
	result->bins[i] = f_user_data(item1->bins[i], 0.f,
				      item2->bins[i], 0.f);
        result->pErrs[i] = f_user_errors(item1->bins[i], 0.f,
					 item2->bins[i], 0.f);
      }
    }
  }
  else
  {
    if (item1->pErrs && item2->pErrs)
    {
      for (i=0; i<nbins; i++)
	result->bins[i] = f_user_data(item1->bins[i], item1->pErrs[i],
				      item2->bins[i], item2->pErrs[i]);
    }
    else if (item1->pErrs)
    {
      for (i=0; i<nbins; i++)
	result->bins[i] = f_user_data(item1->bins[i], item1->pErrs[i],
				      item2->bins[i], 0.f);
    }
    else if (item2->pErrs)
    {
      for (i=0; i<nbins; i++)
	result->bins[i] = f_user_data(item1->bins[i], 0.f,
				      item2->bins[i], item2->pErrs[i]);
    }
    else
    {
      for (i=0; i<nbins; i++)
	result->bins[i] = f_user_data(item1->bins[i], 0.f,
				      item2->bins[i], 0.f);
    }
  }

  SetHistResetFlag((hsGeneral *)result);
  return id;
}

int 
histo_unary_op(int uid, const char *title, const char *category, int id,
	       float (*f_user_data)(float, float),
	       float (*f_user_errors)(float, float))
{
    hsGeneral *item;
     
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_unary_op: Error, called before hs_initialize.\n");
    	return 0;		     /* hs_initialize must be called first */
    }
	
    item = (hsGeneral *) GetItemByPtrID(id);
    if (item == NULL) {
      fprintf(stderr, "hs_unary_op: Error - id: %d does not exist.\n", id);
      return 0;
    }

    if (f_user_data == NULL)
    {
      fprintf(stderr, "hs_unary_op: Error, NULL user data function.\n");
      return 0;
    }

    switch (item->type)
    {
	case HS_1D_HISTOGRAM:
	  return histo_unary_op_1d((hs1DHist *)item, uid, title, category,
				   f_user_data, f_user_errors);

	case HS_2D_HISTOGRAM:
	  return histo_unary_op_2d((hs2DHist *)item, uid, title, category,
				   f_user_data, f_user_errors);

	case HS_3D_HISTOGRAM:
	  return histo_unary_op_3d((hs3DHist *)item, uid, title, category,
				   f_user_data, f_user_errors);
	  
	default:
	  fprintf(stderr, "hs_unary_op: Error, item with id %d is not a histogram.\n", id);
	  return 0;
    }    
}

int 
histo_binary_op(int uid, const char *title, const char *category,
                int id1, int id2, 
		float (*f_user_data)(float, float, float, float),
		float (*f_user_errors)(float, float, float, float))
{
    hsGeneral *item1, *item2;
     
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_binary_op: Error, called before hs_initialize.\n");
    	return 0;		     /* hs_initialize must be called first */
    }
	
    item1 = (hsGeneral *) GetItemByPtrID(id1);
    if (item1 == NULL) {
      fprintf(stderr, "hs_binary_op: Error - id: %d does not exist.\n", id1);
      return 0;
    }
    item2 = (hsGeneral *) GetItemByPtrID(id2);
    if (item2 == NULL) {
      fprintf(stderr, "hs_binary_op: Error - id: %d does not exist.\n", id2);
      return 0;
    }
    if (item1->type != item2->type)
    {
      fprintf(stderr, 
	      "hs_binary_op: Error, items with ids %d and %d are of different type.\n", 
	      id1, id2);
      return 0;
    }

    if (f_user_data == NULL)
    {
      fprintf(stderr, "hs_binary_op: Error, NULL user data function.\n");
      return 0;
    }

    switch (item1->type)
    {
	case HS_1D_HISTOGRAM:
	  if (((hs1DHist *)item1)->nBins != ((hs1DHist *)item2)->nBins)
	  {
	    fprintf(stderr, 
		    "hs_binary_op: histograms with ids %d and %d are not bin-compatible.\n", 
		    id1, id2);
	    return 0;
	  }
	  else
	    return histo_binary_op_1d((hs1DHist *)item1, (hs1DHist *)item2, 
				      uid, title, category,
				      f_user_data, f_user_errors);
	  
	case HS_2D_HISTOGRAM:
	  if (((hs2DHist *)item1)->nXBins != ((hs2DHist *)item2)->nXBins ||
	      ((hs2DHist *)item1)->nYBins != ((hs2DHist *)item2)->nYBins)
	  {
	    fprintf(stderr, 
		    "hs_binary_op: histograms with ids %d and %d are not bin-compatible.\n", 
		    id1, id2);
	    return 0;
	  }
	  else
	    return histo_binary_op_2d((hs2DHist *)item1, (hs2DHist *)item2, 
				      uid, title, category,
				      f_user_data, f_user_errors);
	  
	case HS_3D_HISTOGRAM:
	  if (((hs3DHist *)item1)->nXBins != ((hs3DHist *)item2)->nXBins ||
	      ((hs3DHist *)item1)->nYBins != ((hs3DHist *)item2)->nYBins ||
	      ((hs3DHist *)item1)->nZBins != ((hs3DHist *)item2)->nZBins)
	  {
	    fprintf(stderr, 
		    "hs_binary_op: histograms with ids %d and %d are not bin-compatible.\n", 
		    id1, id2);
	    return 0;
	  }
	  else
	    return histo_binary_op_3d((hs3DHist *)item1, (hs3DHist *)item2, 
				      uid, title, category,
				      f_user_data, f_user_errors);
	  
	default:
	  fprintf(stderr, 
		  "hs_binary_op: Error, items with ids %d and %d are not histograms.\n",
		  id1, id2);
	  return 0;
    }    
}
