/*******************************************************************************
*									       *
* hsTypes.h -- User include file for Histoscope routines		       *
*									       *
* Copyright (c) 1992 Universities Research Association, Inc.		       *
* All rights reserved.							       *
* 									       *
* This material resulted from work developed under a Government Contract and   *
* is subject to the following license:  The Government retains a paid-up,      *
* nonexclusive, irrevocable worldwide license to reproduce, prepare derivative *
* works, perform publicly and display publicly by or for the Government,       *
* including the right to distribute to other Government contractors.  Neither  *
* the United States nor the United States Department of Energy, nor any of     *
* their employees, makes any warrenty, express or implied, or assumes any      *
* legal liability or responsibility for the accuracy, completeness, or         *
* usefulness of any information, apparatus, product, or process disclosed, or  *
* represents that its use would not infringe privately owned rights.           *
*                                        				       *
* Fermilab Nirvana GUI Library						       *
*									       *
* Created 3/9/92 by Mark Edel						       *
*									       *
* Modified 8/24/93 by Joy Kyriakopulos: added new hsControl data type	       *
*					added error values for histograms      *
* Modified 12/7/93 by Paul Lebrun:      added uid item                         *
*                                             HS_NONE typedef                  *
*                                                                              *
*******************************************************************************/

#ifndef HS_TYPES_H_
#define HS_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HS_TYPES_DEFINED
#define HS_TYPES_DEFINED

/* Definitions below should agree with those in histoscope.h */
typedef enum _hsGroupType {
    HS_INDIVIDUAL, HS_MULTI_PLOT, HS_OVERLAY_PLOT
} hsGroupType;

typedef enum _hsErrorType {
    HS_NO_ERRORS, HS_POS_ERRORS, HS_BOTH_ERRORS, HS_ITEMNOTFOUND_ERRORS
} hsErrorType;

typedef enum _hsAxisType {
    HS_AXIS_NONE = 0, HS_AXIS_X, HS_AXIS_Y, HS_AXIS_Z, N_HS_AXIS_TYPES
} hsAxisType;

typedef enum _hsItemType {
    HS_1D_HISTOGRAM = 0, HS_2D_HISTOGRAM, HS_NTUPLE, HS_INDICATOR, HS_CONTROL,
    HS_TRIGGER, HS_NONE, HS_NFIT, HS_GROUP, HS_CONFIG_STRING, HS_3D_HISTOGRAM,
    N_HS_DATA_TYPES
} hsItemType;

#endif

typedef enum _hsScaleType {HS_LINEAR, HS_LOG} hsScaleType;

#define HS_MAX_TITLE_LENGTH 80
#define HS_MAX_CATEGORY_LENGTH 255
#define HS_MAX_CATEGORY_DEPTH 40
#define HS_MAX_LABEL_LENGTH 80
#define HS_MAX_IDENT_LENGTH 255
#define HS_MAX_NAME_LENGTH 80
#define HS_MAX_NUM_GROUP_ITEMS 81
#define HS_MAX_WINDOW_TITLE_LENGTH 160

/* state values for controls and indicators */
#define VALUE_SET 1
#define VALUE_NOT_SET 0
#define VALUE_UNDERFLOW (-1)
#define VALUE_OVERFLOW (-2)

typedef struct _hs1DHist {
    int type;			/* type of data: histogram, ntuple etc..     */
    int id;			/* identifies histogram to users	     */
    char *title;		/* title for histogram window		     */
    char *category;		/* together, category strings establish a hier-
    				   archy for displaying the hist/ntuple list */
    int uid;			/* Unique (within Category) User identifier  */
    int hbookID;		/* HBOOK histogram # (if it's from HBOOK)    */
    int count;			/* count of fill operations		     */
    int nBins;			/* number of bins in histogram		     */
    float min;			/* low edge of first bin		     */
    float max;			/* high edge of last bin		     */
    float overflow;		/* accumulated data > largest bin	     */
    float underflow;		/* accumulated data < smallest bin	     */
    int xScaleType;		/* how data is binned: linear, log, etc..    */
    float xScaleBase;		/* base for log scaling, i.e. 10, e, 2, etc  */
    char *xLabel;		/* label for histogram x (horiz.) axis       */
    char *yLabel;		/* label for histogram y (vertical) axis     */
    float *bins;		/* the histogram data			     */
    int errFlg;			/* if != 0, error values provided (HBOOK==1) */
    float *pErrs;		/* histogram error values (+)		     */
    float *mErrs;		/* histogram error vals (-), nul if symmetric*/
} hs1DHist;

typedef struct _hs2DHist {
    int type;			/* type of data: histogram, ntuple etc..     */
    int id;			/* identifies histogram to users	     */
    char *title;		/* title for histogram window		     */
    char *category;		/* together, category strings establish a hier-
    				   archy for displaying the hist/ntuple list */
    int uid;			/* Unique (within Category) User identifier  */
    int hbookID;		/* HBOOK histogram # (if it's from HBOOK)    */
    int count;			/* count of fill operations		     */
    int nXBins;			/* number of bins along x axis of histogram  */
    int nYBins;			/* number of bins along y axis of histogram  */
    float xMin;			/* low edge of first x axis bin		     */
    float xMax;			/* high edge of last x axis bin		     */
    float yMin;			/* low edge of first y axis bin		     */
    float yMax;			/* high edge of last x axis bin		     */
    float overflow[3][3];	/* Array of overflow data 	0,0 1,0 2,0
    				   from the edges and corners  	0,1	2,1
    				   of the histogram		0,2 1,2 2,2  */
    int xScaleType;		/* how x data is binned: linear, log, etc..  */
    float xScaleBase;		/* base for x axis log scaling, i.e. 10, e   */
    int yScaleType;		/* how y data is binned: linear, log, etc..  */
    float yScaleBase;		/* base for y axis log scaling, i.e. 10, e   */
    char *xLabel; 		/* label for histogram x axis		     */
    char *yLabel;		/* label for histogram y axis		     */
    char *zLabel;		/* label for histogram z (vertical) axis     */
    float *bins;		/* the histogram data (a 2 dim. array)	     */
    int errFlg;			/* if != 0, error values provided (HBOOK==1) */
    float *pErrs;		/* histogram error values (+)		     */
    float *mErrs;		/* histogram error vals (-), nul if symmetric*/
} hs2DHist;

typedef struct _hs3DHist {
    int type;			/* type of data: histogram, ntuple etc..     */
    int id;			/* identifies histogram to users	     */
    char *title;		/* title for histogram window		     */
    char *category;		/* together, category strings establish a hier-
    				   archy for displaying the hist/ntuple list */
    int uid;			/* Unique (within Category) User identifier  */
    int hbookID;		/* HBOOK histogram # (if it's from HBOOK)    */
    int count;			/* count of fill operations		     */
    int nXBins;			/* number of bins along x axis of histogram  */
    int nYBins;			/* number of bins along y axis of histogram  */
    int nZBins;			/* number of bins along z axis of histogram  */
    float xMin;			/* low edge of first x axis bin		     */
    float xMax;			/* high edge of last x axis bin		     */
    float yMin;			/* low edge of first y axis bin		     */
    float yMax;			/* high edge of last x axis bin		     */
    float zMin;			/* low edge of first z axis bin		     */
    float zMax;			/* high edge of last z axis bin		     */
    float overflow[3][3][3];	/* Array of overflow data                    */
    int xScaleType;		/* how x data is binned: linear, log, etc..  */
    float xScaleBase;		/* base for x axis log scaling, i.e. 10, e   */
    int yScaleType;		/* how y data is binned: linear, log, etc..  */
    float yScaleBase;		/* base for y axis log scaling, i.e. 10, e   */
    int zScaleType;		/* how z data is binned: linear, log, etc..  */
    float zScaleBase;		/* base for z axis log scaling, i.e. 10, e   */
    char *xLabel; 		/* label for histogram x axis		     */
    char *yLabel;		/* label for histogram y axis		     */
    char *zLabel;		/* label for histogram z axis                */
    char *vLabel;		/* label for histogram "vertical" axis       */
    float *bins;		/* the histogram data (a 3 dim. array)	     */
    int errFlg;			/* if != 0, error values provided (HBOOK==1) */
    float *pErrs;		/* histogram error values (+)		     */
    float *mErrs;		/* histogram error vals (-), nul if symmetric*/
} hs3DHist;

typedef struct _hsNTuple {
    int type;			/* type of data: histogram, ntuple etc..     */
    int id;			/* identifies ntuple to users		     */
    char *title;		/* title for the ntuple window		     */
    char *category;		/* together, category strings establish a hier-
    				   archy for displaying the hist/ntuple list */
    int uid;			/* Unique (within Category) User identifier  */
    int hbookID;		/* HBOOK histogram # (if ntuple from HBOOK)  */
    int n;			/* number of elements in the ntuple	     */
    int nVariables;		/* # of variables in each element of ntuple  */
    char **names;		/* array containing name of each variable    */
    int chunkSzData;		/* size of data memory blocks		     */
    int chunkSzExt;		/* size of extensions memory blocks	     */
    float *data;		/* points to the 2 dim. array of ntuple data */
    float *extensions[4];	/* additional blocks of data so ntuple can be
    				   extended dynamically w/less reallocation  */
} hsNTuple;

typedef struct _hsIndicator {
    int type;			/* type of data: histogram, ntuple etc..     */
    int id;			/* identifies indicator to users	     */
    char *title;		/* title for the indicator window	     */
    char *category;		/* together, category strings establish a hier-
    				   archy for displaying the hist/ntuple list */
    int uid;			/* Unique (within Category) User identifier  */
    int hbookID;		/* unused, always 0			     */
    float min;			/* minimum value for indicator		     */
    float max;			/* maximum value for indicator		     */
    int valueSet;		/* == 0 if value not yet set, otherwise != 0 */
    float value;		/* indicator value			     */
} hsIndicator;

typedef struct _hsControl {
    int type;			/* type of data: histogram, ntuple etc..     */
    int id;			/* identifies control to users	  	     */
    char *title;		/* title for the control window		     */
    char *category;		/* together, category strings establish a hier-
    				   archy for displaying the hist/ntuple list */
    int uid;			/* Unique (within Category) User identifier  */
    int hbookID;		/* unused, always 0			     */
    float min;			/* minimum value for control		     */
    float max;			/* maximum value for control		     */
    float value;		/* control value			     */
} hsControl;

typedef struct _hsTrigger {
    int type;			/* type of data: histogram, ntuple etc..     */
    int id;			/* identifies trigger to users		     */
    char *title;		/* title for window and entry name for list  */
    char *category;		/* together, category strings establish a hier-
    				   archy for displaying the hist/ntuple list */
    int uid;			/* Unique (within Category) User identifier  */
    int hbookID;		/* HBOOK histogram # (will == 0)	     */
} hsTrigger;

/* Nfit Item  */
typedef struct _hsNFit {
    int type;			/* type of data: histogram, ntuple etc..     */
    int id;			/* identifies item to users		     */
    char *title;		/* title for window and entry name for list  */
    char *category;		/* together, category strings establish a hier-
    				   archy for displaying the item list	     */
    int uid;			/* Unique (within Category) User identifier  */
    int hbookID;		/* HBOOK histogram # (will == 0)	     */
    int itemVersion;		/* Version of NFit item (for files)	     */
    int endPos;			/* End position of item (on xdr file)	     */
    char *expr;			/* expression				     */
    char *funcPath;		/* path to userfunction or expression string */
    int nParams;		/* number of elements in mParam array	     */
    void *mParam;		/* pointer to the minuitParam structure array*/
    int calculationMode;        /* Either Log Likelihood or chi-square       */
    double lowLimit;            /* Lower limit for fitting                   */
    double upLimit;             /* Upper Limit for fitting                   */
    int istatCovariance;        /* Status for the covariance Matrix          */
    double *covarianceMatrix;   /* Covariance matrix                         */
} hsNFit;

/* Group Item */
typedef struct _hsGroup {
    int type;			/* type of data: histogram, ntuple etc..     */
    int id;			/* identifies item to users		     */
    char *title;		/* title for window and entry name for list  */
    char *category;		/* together, category strings establish a hier-
    				   archy for displaying the item list	     */
    int uid;			/* Unique (within Category) User identifier  */
    int hbookID;		/* HBOOK histogram # (will == 0)	     */
    int groupType;		/* HS_MULTI_PLOT or HS_OVERLAY_PLOT or 0     */
    int numItems;		/* number of items in group		     */
    int *itemId;		/* Ptr to 1d array of item IDs in group      */
    int *errsDisp;		/* Ptr to 1d array of error flags, used only 
    				   if item is a histogram; when != 0, error 
    				   values are displayed by histoscope	     */
} hsGroup;

/* Structure to allow Configuration Strings to be translated by XDR	     */
typedef struct _hsConfigString {
    int type;			/* type of data: Config File String	     */
    int id;			/* identifies item to users (== 0)	     */
    char *title;		/* title for window and entry name for list  */
    char *category;		/* together, category strings establish a hier-
    				   archy for displaying the hist/ntuple list */
    int uid;			/* == 0					     */
    int hbookID;		/* HBOOK histogram # (== 0)		     */
    int stringLength;		/* length of configuration string	     */
    char *configString;	        /* configuration string			     */
} hsConfigString;

/* The common part of hs1DHist, hs2DHist, hsNTuple & hsIndicator structures  */
typedef struct _hsGeneral {
    int type;			/* type of data: histogram, ntuple etc..     */
    int id;			/* identifies item to users		     */
    char *title;		/* title for window and entry name for list  */
    char *category;		/* together, category strings establish a hier-
    				   archy for displaying the hist/ntuple list */
    int uid;			/* Unique (within Category) User identifier  */
    int hbookID;		/* HBOOK histogram # (if item is from HBOOK) */
} hsGeneral;

#ifdef __cplusplus
}
#endif

#endif /* not HS_TYPES_H_ */
