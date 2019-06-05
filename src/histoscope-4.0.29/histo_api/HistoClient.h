/*******************************************************************************
*									       *
* HistoClient.h -- interface for HistoClient.c routines: functions called by   *
*		   by the C and FORTRAN bindings to HistoClient.c routines.    *
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
* Modified 12/7/93 by Paul Lebrun:      added uid item                         *
*******************************************************************************/

#define SIZE_HSIDLIST 250           /* space allocated for 250 hs_general 
							pointers at start..   */
void histo_initialize(const char *id_string);
int histo_server_port(void);
void histo_update(void);
void histo_complete(void);
void histo_complete_and_wait(void);
void histo_histoscope(int return_immediately, const char *configFile, int hidden);
int histo_num_connected_scopes(void);
void histo_load_config(const char *cfgBuf, int cfgBufLen);
int histo_create_1d_hist(int uid, const char *title, const char *category, const char *x_label,
                        const char *y_label, int n_bins, float min, float max);
int histo_create_2d_hist(int uid, const char *title, const char *category, const char *x_label, 
			const char *y_label, const char *z_label, int x_bins, int y_bins, 
			float x_min, float x_max, float y_min, float y_max);
int histo_create_3d_hist(int uid, const char *title, const char *category, const char *x_label, 
			 const char *y_label, const char *z_label, const char *v_label,
			 int x_bins, int y_bins, int z_bins, float x_min,
			 float x_max, float y_min, float y_max,
                         float z_min, float z_max);
int histo_create_ntuple(int uid, const char *title, const char *category, int n_variables,
			char **names);
int histo_create_indicator(int uid, const char *title, const char *category, float min, 
                             float max);
int histo_create_control(int uid, const char *title, const char *category, float min,
			 float max, float default_value);
int histo_create_trigger(int uid, const char *title, const char *category);
int histo_create_group(int uid, const char *title, const char *category, int groupType,
			int numItems, int *itemId, int *errsDisp);
void histo_fill_1d_hist(int id, float x, float weight);
void histo_fill_2d_hist(int id, float x, float y, float weight);
void histo_fill_3d_hist(int id, float x, float y, float z, float weight);
void histo_1d_hist_set_bin(int id, int ix, float value);
void histo_2d_hist_set_bin(int id, int ix, int iy, float value);
void histo_3d_hist_set_bin(int id, int ix, int iy, int iz, float value);
void histo_1d_hist_set_bin_errors(int id, int ix,
                                  float pos, float neg, int flag);
void histo_2d_hist_set_bin_errors(int id, int ix, int iy,
                                  float pos, float neg, int flag);
void histo_3d_hist_set_bin_errors(int id, int ix, int iy, int iz,
                                  float pos, float neg, int flag);
void histo_hist_set_slice(int id, int bin0, int stride, int count, float *data);
void histo_hist_set_slice_errors(int id, int bin0, int stride, int count,
                                 float *err_valsP, float *err_valsM);
int histo_fill_ntuple(int id, float *values);
void histo_set_indicator(int id, float value);
void histo_read_control(int id, float *value);
int histo_check_trigger(int id);
void histo_set_1d_errors(int id, float *err_valsP, float *err_valsM);
void histo_set_2d_errors(int id, float *err_valsP, float *err_valsM,
						   int col_maj_flag);
void histo_set_3d_errors(int id, float *err_valsP, float *err_valsM,
						   int col_maj_flag);
void histo_reset(int id);
void histo_allow_reset_refresh(int id, int flag);
void histo_delete(int id);

void histo_allow_item_send(int flag);
int histo_socket_status(void);

void SendNewItem(hsGeneral *item);
hsGeneral   *GetItemByPtrID (int id); 
void	     DeleteItemFromPtrList (int id);
void AddItemToHsIdList(hsGeneral *hist);
void SetHistResetFlag(hsGeneral *item);

/***   Function Prototypes for "internal" routines shared with HistoApi*.c  ***/
const char *ValidStr(const char *string, int length,
                     const char *routine, const char *strKind);
						 /* validates string lengths  */
int CheckValidCategory(const char *category, const char *routineName, int dotDotDot);
char *CopyNtrim(const char *fromString);	/* copies strings wo/leading or
						   trailing spaces 	      */
void BeginItemList(void);
void EndItemList(void);

typedef struct _hsCControl {
    hsControl ctrl;
    int       valueSet;	     /* flag: has value been read by client program? */
    float     newValue;	     /* value set by HS until read by client program */
    float     defaultValue;  /* holds default value in case of reset         */
} hsCControl;

typedef struct _hsCTrigger {
    hsTrigger trigger;
    int       numTriggered;  /* number of events triggered for this id by HS */
    			     /* 	waiting to be read (checked) by user */
} hsCTrigger;


/* Global variables defined in HistoClient.c & used by histoApi*.c */
extern int InitializedAndActive;
extern int NumOfItems;
extern hsGeneral **HistoPtrList;


void histo_reset_const(int id, float c);
int histo_copy_hist(int old_id, int uid, const char *title, const char *category);
void histo_delete_items(int *ids, int num_items);
void histo_kill_histoscope(void);
void histo_task_completion_callback(void (*f)(int, int, int, char *, void *), void *);
int histo_fill_hist_slice(int parent_id, int axis1, int bin1,
			  int axis2, int bin2, int slice_id);
int histo_slice_contents(int parent_id, int axis1, int bin1, int axis2, 
			 int bin2, int arrsize, float *data, float *poserr,
			 float *negerr, int *slicesize);
