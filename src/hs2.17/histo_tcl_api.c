#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <dlfcn.h>
#include "cernlib.h"
#include "histoscope.h"
#include "histo_tcl_api.h"
#include "histo_utils.h"
#include "kerdens.h"
#include "ranlux.h"
#include "fit_function.h"

typedef struct _sorted_column_data {
    int row;
    int sorted_row;
    float value;
} sorted_column_data;

#define NTUPLE_FILL_BUFLEN 100
#define NTUPLE_MAXKEYLEN 25

static int fcompare(const float *i, const float *j);
static int parse_2d_matrix(Tcl_Interp *interp, Tcl_Obj *obj,
			   double mat[2][2]);
static int sort_column_by_value(const sorted_column_data* p1,
                                const sorted_column_data* p2);
static int inverse_sort_column_by_value(const sorted_column_data* p1,
                                        const sorted_column_data* p2);
static int sort_column_by_row(const sorted_column_data* p1,
                              const sorted_column_data* p2);
static int sort_float_rows_incr(const float *row1, const float *row2);
static int sort_float_rows_decr(const float *row1, const float *row2);

const char * const hs_typename_strings[N_HS_DATA_TYPES] = {
    "HS_1D_HISTOGRAM",
    "HS_2D_HISTOGRAM",
    "HS_NTUPLE",
    "HS_INDICATOR",
    "HS_CONTROL",
    "HS_TRIGGER",
    "HS_NONE",
    "HS_NFIT",
    "HS_GROUP",
    "HS_CONFIG_STRING",
    "HS_3D_HISTOGRAM"
};

static const char * const c_keywords[] = {
    "asm",
    "auto",
    "break",
    "case",
    "char",
    "const",
    "continue",
    "default",
    "do",
    "double",
    "else",
    "enum",
    "extern",
    "float",
    "for",
    "goto",
    "if",
    "int",
    "long",
    "register",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "struct",
    "switch",
    "typedef",
    "union",
    "unsigned",
    "void",
    "volatile",
    "while"
};

static char *OUT_OF_MEMORY = "out of memory";

static int *ntuple_sort_columns = NULL;
static int n_ntuple_sort_columns = 0;

/*
 * Tcl functions from the Histo-Scope V4.0 API
 */

VOID_FUNCT_WITH_ONE_CHAR_ARG(initialize)
INT_FUNCT_WITH_VOID_ARG(server_port)
VOID_FUNCT_WITH_VOID_ARG(complete)
VOID_FUNCT_WITH_VOID_ARG(complete_and_wait)
VOID_FUNCT_WITH_ONE_BOOLEAN_ARG(histoscope)
VOID_FUNCT_WITH_VOID_ARG(hs_update)

void hs_hs_update(void)
{
    hs_update();
}

tcl_routine(histo_with_config)
{
  int return_immediately;
  
  tcl_require_objc(3);
  if (Tcl_GetBooleanFromObj(interp, objv[1], &return_immediately) != TCL_OK)
      return TCL_ERROR;
  hs_histo_with_config(return_immediately, 
		       Tcl_GetStringFromObj(objv[2], NULL));
  return TCL_OK;
}

INT_FUNCT_WITH_VOID_ARG(num_connected_scopes)

tcl_routine(load_config_string)
{
    Tcl_DString ds;

    tcl_require_objc(2);
    obj_2_dstring(objv[1], ds);
    hs_load_config_string(Tcl_DStringValue(&ds));
    Tcl_DStringFree(&ds);
    return TCL_OK;
}

VOID_FUNCT_WITH_ONE_CHAR_ARG(load_config_file)
VOID_FUNCT_WITH_VOID_ARG(kill_histoscope)

tcl_routine(create_1d_hist)
{
  int id, uid, n_bins;
  double dmin, dmax;
  Tcl_DString xlabel, ylabel;

  tcl_require_objc(9);
  if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[6], &n_bins) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[7], &dmin) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[8], &dmax) != TCL_OK)
    return TCL_ERROR;
  obj_2_dstring(objv[4], xlabel);
  obj_2_dstring(objv[5], ylabel);
  id = hs_create_1d_hist(uid, 
      Tcl_GetStringFromObj(objv[2], NULL), 
      Tcl_GetStringFromObj(objv[3], NULL),
      Tcl_DStringValue(&xlabel),
      Tcl_DStringValue(&ylabel),
      n_bins, (float)dmin, (float)dmax);
  Tcl_DStringFree(&xlabel);
  Tcl_DStringFree(&ylabel);
  if (id <= 0)
  {
      Tcl_AppendResult(interp, "failed to create 1d histogram with uid ",
		       Tcl_GetStringFromObj(objv[1], NULL),
		       ", title \"", Tcl_GetStringFromObj(objv[2], NULL),
		       "\", and category \"",
		       Tcl_GetStringFromObj(objv[3], NULL), "\"", NULL);
      return TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(id));
  return TCL_OK;
}

tcl_routine(create_2d_hist)
{
  int id, uid, x_bins, y_bins;
  double x_min, x_max, y_min, y_max;
  Tcl_DString xlabel, ylabel, zlabel;
  
  tcl_require_objc(13);
  if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[7], &x_bins) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[8], &y_bins) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[9], &x_min) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[10], &x_max) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[11], &y_min) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[12], &y_max) != TCL_OK)
    return TCL_ERROR;
  obj_2_dstring(objv[4], xlabel);
  obj_2_dstring(objv[5], ylabel);
  obj_2_dstring(objv[6], zlabel);
  id = hs_create_2d_hist(uid, 
			Tcl_GetStringFromObj(objv[2], NULL),
			Tcl_GetStringFromObj(objv[3], NULL),
			Tcl_DStringValue(&xlabel),
			Tcl_DStringValue(&ylabel),
			Tcl_DStringValue(&zlabel),
			x_bins, y_bins, (float)x_min, (float)x_max,
			(float)y_min, (float)y_max);
  Tcl_DStringFree(&xlabel);
  Tcl_DStringFree(&ylabel);
  Tcl_DStringFree(&zlabel);
  if (id <= 0)
  {
      Tcl_AppendResult(interp, "failed to create 2d histogram with uid ",
		       Tcl_GetStringFromObj(objv[1], NULL),
		       ", title \"", Tcl_GetStringFromObj(objv[2], NULL),
		       "\", and category \"",
		       Tcl_GetStringFromObj(objv[3], NULL), "\"", NULL);
      return TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(id));
  return TCL_OK;
}

tcl_routine(create_3d_hist)
{
  int id, uid, x_bins, y_bins, z_bins;
  double x_min, x_max, y_min, y_max, z_min, z_max;
  Tcl_DString xlabel, ylabel, zlabel, vlabel;

  tcl_require_objc(17);
  if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[8], &x_bins) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[9], &y_bins) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[10], &z_bins) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[11], &x_min) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[12], &x_max) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[13], &y_min) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[14], &y_max) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[15], &z_min) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[16], &z_max) != TCL_OK)
    return TCL_ERROR;
  obj_2_dstring(objv[4], xlabel);
  obj_2_dstring(objv[5], ylabel);
  obj_2_dstring(objv[6], zlabel);
  obj_2_dstring(objv[7], vlabel);
  id = hs_create_3d_hist(uid, 
			 Tcl_GetStringFromObj(objv[2], NULL),
			 Tcl_GetStringFromObj(objv[3], NULL),
			 Tcl_DStringValue(&xlabel),
			 Tcl_DStringValue(&ylabel),
			 Tcl_DStringValue(&zlabel),
			 Tcl_DStringValue(&vlabel),
			 x_bins, y_bins, z_bins,
			 (float)x_min, (float)x_max,
			 (float)y_min, (float)y_max,
			 (float)z_min, (float)z_max);
  Tcl_DStringFree(&xlabel);
  Tcl_DStringFree(&ylabel);
  Tcl_DStringFree(&zlabel);
  Tcl_DStringFree(&vlabel);
  if (id <= 0)
  {
      Tcl_AppendResult(interp, "failed to create 3d histogram with uid ",
		       Tcl_GetStringFromObj(objv[1], NULL),
		       ", title \"", Tcl_GetStringFromObj(objv[2], NULL),
		       "\", and category \"",
		       Tcl_GetStringFromObj(objv[3], NULL), "\"", NULL);
      return TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(id));
  return TCL_OK;
}

tcl_routine(create_ntuple)
{
  int id, i, uid, listlen;
  char **listelem;
  Tcl_Obj **listObjElem;
  /* The maximum length of the variable name should not
   * exceed HS_MAX_NAME_LENGTH defined in the Histo-Scope
   * header file "histo.h", otherwise Histo-Scope will
   * truncate it.
   */
  const int maxlen = 80;

  tcl_require_objc(5);
  if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
      return TCL_ERROR;
  if (Tcl_ListObjGetElements(interp, objv[4], &listlen, &listObjElem) != TCL_OK)
      return TCL_ERROR;
  if (listlen == 0)
  {
      Tcl_SetResult(interp, "the list of column names is empty", TCL_STATIC);
      return TCL_ERROR;
  }
  if ((listelem = (char **)malloc(listlen*sizeof(char *))) == NULL) {
      Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
      return TCL_ERROR;
  }
  for (i=0; i<listlen; ++i)
  {
      listelem[i] = Tcl_GetStringFromObj(listObjElem[i], NULL);
      if (*listelem[i] == '\0')
      {
	  Tcl_SetResult(interp,
			"ntuple variable name can not be an empty string",
			TCL_STATIC);
	  free(listelem);
	  return TCL_ERROR;
      }
      if (strlen(listelem[i]) > (unsigned)maxlen)
      {
          char buf[32];
          sprintf(buf, "%d", maxlen);
          Tcl_AppendResult(interp, "ntuple variable name \"", listelem[i],
                           "\" is too long, should have at most ",
                           buf, " characters", NULL);
          free(listelem);
          return TCL_ERROR;
      }
  }
  i = find_duplicate_name(listelem, listlen);
  if (i >= 0)
  {
      Tcl_AppendResult(interp, "duplicate variable name \"", listelem[i], "\"", NULL);
      free(listelem);
      return TCL_ERROR;
  }
  id = hs_create_ntuple(uid, 
		       Tcl_GetStringFromObj(objv[2], NULL),
		       Tcl_GetStringFromObj(objv[3], NULL),
		       listlen, listelem);
  free(listelem);
  if (id <= 0)
  {
      Tcl_AppendResult(interp, "failed to create ntuple with uid ",
		       Tcl_GetStringFromObj(objv[1], NULL),
		       ", title \"", Tcl_GetStringFromObj(objv[2], NULL),
		       "\", and category \"",
		       Tcl_GetStringFromObj(objv[3], NULL), "\"", NULL);
      return TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(id));
  return TCL_OK;
}

tcl_routine(create_indicator)
{
  int id, uid;
  double xmin, xmax;
  
  tcl_require_objc(6);
  if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
    return TCL_ERROR; 
  if (Tcl_GetDoubleFromObj(interp, objv[4], &xmin) != TCL_OK)
    return TCL_ERROR; 
  if (Tcl_GetDoubleFromObj(interp, objv[5], &xmax) != TCL_OK)
    return TCL_ERROR; 
  id = hs_create_indicator(uid, 
			   Tcl_GetStringFromObj(objv[2], NULL),
			   Tcl_GetStringFromObj(objv[3], NULL),
			   (float)xmin, (float)xmax);
  if (id <= 0)
  {
      Tcl_AppendResult(interp, "failed to create indicator with uid ",
		       Tcl_GetStringFromObj(objv[1], NULL),
		       ", title \"", Tcl_GetStringFromObj(objv[2], NULL),
		       "\", and category \"",
		       Tcl_GetStringFromObj(objv[3], NULL), "\"", NULL);
      return TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(id));
  return TCL_OK;
}

tcl_routine(create_control)
{
  int id, uid;
  double xmin, xmax, xdefault;

  tcl_require_objc(7);
  if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
    return TCL_ERROR; 
  if (Tcl_GetDoubleFromObj(interp, objv[4], &xmin) != TCL_OK)
    return TCL_ERROR; 
  if (Tcl_GetDoubleFromObj(interp, objv[5], &xmax) != TCL_OK)
    return TCL_ERROR; 
  if (Tcl_GetDoubleFromObj(interp, objv[6], &xdefault) != TCL_OK)
    return TCL_ERROR; 
  id = hs_create_control(uid, 
			 Tcl_GetStringFromObj(objv[2], NULL),
			 Tcl_GetStringFromObj(objv[3], NULL),
			 (float)xmin, (float)xmax, (float)xdefault);
  if (id <= 0)
  {
      Tcl_AppendResult(interp, "failed to create control with uid ",
		       Tcl_GetStringFromObj(objv[1], NULL),
		       ", title \"", Tcl_GetStringFromObj(objv[2], NULL),
		       "\", and category \"",
		       Tcl_GetStringFromObj(objv[3], NULL), "\"", NULL);
      return TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(id));
  return TCL_OK;
}

tcl_routine(create_trigger)
{
  int id, uid;
  
  tcl_require_objc(4);
  if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
      return TCL_ERROR; 
  id = hs_create_trigger(uid, 
			 Tcl_GetStringFromObj(objv[2], NULL),
			 Tcl_GetStringFromObj(objv[3], NULL));
  if (id <= 0)
  {
      Tcl_AppendResult(interp, "failed to create trigger with uid ",
		       Tcl_GetStringFromObj(objv[1], NULL),
		       ", title \"", Tcl_GetStringFromObj(objv[2], NULL),
		       "\", and category \"",
		       Tcl_GetStringFromObj(objv[3], NULL), "\"", NULL);
      return TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(id));
  return TCL_OK;
}

tcl_routine(create_group)
{
  int id, i, listlen, uid, groupType, nids;
  int *ids, *errtypes;
  char *c;
  Tcl_Obj **listObjElem;

  tcl_require_objc(7);

  if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
    return TCL_ERROR;

  c = Tcl_GetStringFromObj(objv[4], NULL);
  if ((strstr(c, "Multi") ||
       strstr(c, "MULTI") ||
       strstr(c, "multi")))
    groupType = HS_MULTI_PLOT;
  else if ((strstr(c, "Over") ||
	    strstr(c, "OVER") ||
	    strstr(c, "over")))
    groupType = HS_OVERLAY_PLOT;
  else if ((strstr(c, "INDIV") ||
	    strstr(c, "Indiv") ||
	    strstr(c, "indiv")))
    groupType = HS_INDIVIDUAL;
  else
  {
    Tcl_SetResult(interp,
		  "Invalid group type. Valid types are MULTI, OVERLAY, and INDIVIDUAL",
		  TCL_STATIC);
    return TCL_ERROR;
  }

  if (Tcl_ListObjGetElements(interp, objv[5], &listlen, &listObjElem) != TCL_OK)
      return TCL_ERROR;
  if ((ids = (int *)malloc(listlen*sizeof(int))) == NULL)
  {
    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
    return TCL_ERROR;
  }
  for (i=0; i < listlen; i++)
      if (Tcl_GetIntFromObj(interp, listObjElem[i], ids+i) != TCL_OK)
      {
	  free(ids);
	  return TCL_ERROR;
      }
  nids = listlen;
  
  if (Tcl_ListObjGetElements(interp, objv[6], &listlen, &listObjElem) != TCL_OK)
  {
      free(ids);
      return TCL_ERROR;
  }
  if (listlen != nids)
  {
    Tcl_SetResult(interp, 
		  "list of ids and list of error options are of different length",
		  TCL_STATIC);
    free(ids);
    return TCL_ERROR;
  }
  if ((errtypes = (int *)malloc(listlen*sizeof(int))) == NULL)
  {
    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
    free(ids);
    return TCL_ERROR;
  }
  for (i=0; i < listlen; i++)
  {
    c = Tcl_GetStringFromObj(listObjElem[i], NULL);
    if ((strncmp(c, "hs_", 3) == 0 ||
	 strncmp(c, "Hs_", 3) == 0 ||
	 strncmp(c, "HS_", 3) == 0))
      c += 3;
    if ((strncmp(c, "No", 2) == 0 ||
	 strncmp(c, "NO", 2) == 0 ||
	 strncmp(c, "no", 2) == 0))
      errtypes[i] = HS_NO_ERROR_BARS;
    else if ((strncmp(c, "Dat", 3) == 0 ||
	      strncmp(c, "DAT", 3) == 0 ||
	      strncmp(c, "dat", 3) == 0))
      errtypes[i] = HS_DATA_ERROR_BARS;
    else if ((strncmp(c, "Gaus", 4) == 0 ||
	      strncmp(c, "GAUS", 4) == 0 ||
	      strncmp(c, "gaus", 4) == 0))
      errtypes[i] = HS_GAUSSIAN_ERROR_BARS;
    else
    {
      Tcl_AppendResult(interp, "Invalid error option \"", c, "\"", NULL);
      free(ids);
      free(errtypes);
      return TCL_ERROR;
    }
  }

  id = hs_create_group(uid, 
		       Tcl_GetStringFromObj(objv[2], NULL),
		       Tcl_GetStringFromObj(objv[3], NULL),
		       groupType, nids, ids, errtypes);
  free(ids);
  free(errtypes);
  if (id <= 0)
  {
      Tcl_AppendResult(interp, "failed to create group with uid ",
		       Tcl_GetStringFromObj(objv[1], NULL),
		       ", title \"", Tcl_GetStringFromObj(objv[2], NULL),
		       "\", and category \"",
		       Tcl_GetStringFromObj(objv[3], NULL), "\"", NULL);
      return TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(id));
  return TCL_OK;
}

tcl_routine(fill_1d_hist)
{
  int id;
  double x, weight;
  
  tcl_require_objc(4);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[2], &x) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[3], &weight) != TCL_OK)
    return TCL_ERROR;
  hs_fill_1d_hist(id, (float)x, (float)weight);
  return TCL_OK;
}

tcl_routine(1d_hist_set_bin)
{
  int id, x;
  double weight;
  
  tcl_require_objc(4);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[2], &x) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[3], &weight) != TCL_OK)
    return TCL_ERROR;
  hs_1d_hist_set_bin(id, x, (float)weight);
  return TCL_OK;
}

#define parse_error_numbers do {\
  if (*Tcl_GetStringFromObj(objv[iparse],NULL))\
  {\
      has_pos = 1;\
      if (Tcl_GetDoubleFromObj(interp, objv[iparse], &pos) != TCL_OK)\
          return TCL_ERROR;\
  }\
  if (objc > iparse+1)\
      if (*Tcl_GetStringFromObj(objv[iparse+1],NULL))\
      {\
          has_neg = 1;\
          if (Tcl_GetDoubleFromObj(interp, objv[iparse+1], &neg) != TCL_OK)\
              return TCL_ERROR;\
      }\
} while(0);

tcl_routine(1d_hist_set_bin_errors)
{
  int id, x, has_neg = 0, has_pos = 0;
  double pos = 0.0, neg = 0.0;
  const int iparse = 3;

  tcl_objc_range(4,5);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[2], &x) != TCL_OK)
    return TCL_ERROR;
  parse_error_numbers;
  if (has_neg || has_pos)
  {
      int which = 1;
      if (has_neg && has_pos)
          which = 0;
      else if (has_neg)
          which = -1;
      hs_1d_hist_set_bin_errors(id, x, (float)pos, (float)neg, which);
  }
  return TCL_OK;
}

tcl_routine(fill_2d_hist)
{
  int id;
  double x, y, weight;

  tcl_require_objc(5);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[2], &x) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[3], &y) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[4], &weight) != TCL_OK)
    return TCL_ERROR;
  hs_fill_2d_hist(id, (float)x, (float)y, (float)weight);
  return TCL_OK;
}

tcl_routine(2d_hist_set_bin)
{
  int id, x, y;
  double weight;

  tcl_require_objc(5);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[2], &x) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[3], &y) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[4], &weight) != TCL_OK)
    return TCL_ERROR;
  hs_2d_hist_set_bin(id, x, y, (float)weight);
  return TCL_OK;
}

tcl_routine(2d_hist_set_bin_errors)
{
  int id, x, y, has_neg = 0, has_pos = 0;
  double pos = 0.0, neg = 0.0;
  const int iparse = 4;

  tcl_objc_range(5,6);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[2], &x) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[3], &y) != TCL_OK)
    return TCL_ERROR;
  parse_error_numbers;
  if (has_neg || has_pos)
  {
      int which = 1;
      if (has_neg && has_pos)
          which = 0;
      else if (has_neg)
          which = -1;
      hs_2d_hist_set_bin_errors(id, x, y, (float)pos, (float)neg, which);
  }
  return TCL_OK;
}

tcl_routine(fill_3d_hist)
{
  int id;
  double x, y, z, weight;

  tcl_require_objc(6);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[2], &x) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[3], &y) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[4], &z) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[5], &weight) != TCL_OK)
    return TCL_ERROR;
  hs_fill_3d_hist(id, (float)x, (float)y, (float)z, (float)weight);
  return TCL_OK;
}

tcl_routine(3d_hist_set_bin)
{
  int x, y, z, id;
  double weight;

  tcl_require_objc(6);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[2], &x) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[3], &y) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[4], &z) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[5], &weight) != TCL_OK)
    return TCL_ERROR;
  hs_3d_hist_set_bin(id, x, y, z, (float)weight);
  return TCL_OK;
}

tcl_routine(3d_hist_set_bin_errors)
{
  int x, y, z, id, has_neg = 0, has_pos = 0;
  double pos = 0.0, neg = 0.0;
  const int iparse = 5;

  tcl_objc_range(6,7);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[2], &x) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[3], &y) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[4], &z) != TCL_OK)
    return TCL_ERROR;
  parse_error_numbers;
  if (has_neg || has_pos)
  {
      int which = 1;
      if (has_neg && has_pos)
          which = 0;
      else if (has_neg)
          which = -1;
      hs_3d_hist_set_bin_errors(id, x, y, z, (float)pos, (float)neg, which);
  }
  return TCL_OK;
}

tcl_routine(hist_set_slice)
{
    int id, bin0, stride, count, freeData = 0;
    float *data = NULL;

    tcl_require_objc(5);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[2], &bin0) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[3], &stride) != TCL_OK)
        return TCL_ERROR;
    if (get_float_array_from_binary_or_list(interp, objv[4], &data,
                                            &count, &freeData) != TCL_OK)
        return TCL_ERROR;
    hs_hist_set_slice(id, bin0, stride, count, data);
    if (freeData)
        free(data);
    return TCL_OK;
}

tcl_routine(hist_set_slice_errors)
{
    int id, bin0, stride, count, status = TCL_ERROR;
    int pcount = 0, ncount = 0, freePoserr = 0, freeNegerr = 0;
    float *poserr = NULL, *negerr = NULL;

    tcl_objc_range(5,6);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[2], &bin0) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[3], &stride) != TCL_OK)
        return TCL_ERROR;
    if (get_float_array_from_binary_or_list(interp, objv[4], &poserr,
                                            &pcount, &freePoserr) != TCL_OK)
        goto fail;
    if (objc > 5)
        if (get_float_array_from_binary_or_list(interp, objv[5], &negerr,
                                                &ncount, &freeNegerr) != TCL_OK)
            goto fail;
    if (pcount && ncount)
    {
        if (pcount != ncount)
        {
            Tcl_SetResult(interp, "number of elements is not the same for "
                          "positive and negative errors", TCL_STATIC);
            goto fail;
        }
        count = pcount;
    }
    else if (pcount)
        count = pcount;
    else if (ncount)
        count = ncount;
    else
        count = 0;
    hs_hist_set_slice_errors(id, bin0, stride, count, poserr, negerr);
    status = TCL_OK;

 fail:
    if (freeNegerr)
        free(negerr);
    if (freePoserr)
        free(poserr);
    return status;
}

tcl_routine(pack_item)
{
    int id, itype, size;
    void *mem = 0;
    Tcl_Obj *result = 0;

    tcl_require_objc(2);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
        return TCL_ERROR;
    itype = hs_type(id);
    if (itype != HS_1D_HISTOGRAM &&
        itype != HS_2D_HISTOGRAM &&
        itype != HS_3D_HISTOGRAM &&
        itype != HS_NTUPLE &&
        itype != HS_INDICATOR &&
        itype != HS_CONTROL)
    {
        if (itype == HS_NONE)
            Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
                             " is not a valid Histo-Scope id", NULL);
        else
            Tcl_AppendResult(interp, "item with id ", 
                             Tcl_GetStringFromObj(objv[1], NULL),
                             " can not be packed", NULL);
        return TCL_ERROR;
    }
    size = hs_pack_item(id, &mem);
    if (size <= 0 || mem == 0)
    {
        Tcl_AppendResult(interp, "failed to pack item with id ", 
                         Tcl_GetStringFromObj(objv[1], NULL), NULL);
        return TCL_ERROR;
    }
    result = Tcl_NewByteArrayObj((unsigned char *)mem, size);
    free(mem);
    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

tcl_routine(unpack_item)
{
    int id, size;
    unsigned char *mem;
    char *prefix;

    tcl_require_objc(3);
    prefix = Tcl_GetStringFromObj(objv[1], NULL);
    mem = Tcl_GetByteArrayFromObj(objv[2], &size);
    if (mem == NULL)
    {
        Tcl_SetResult(interp, "failed to extract a binary string",
                      TCL_STATIC);
        return TCL_ERROR;
    }
    if (size <= 0)
    {
        Tcl_SetResult(interp, "zero length binary string "
                      "can't contain a packed item", TCL_STATIC);
        return TCL_ERROR;
    }
    id = hs_unpack_item(mem, size, prefix);
    if (id <= 0)
    {
        Tcl_AppendResult(interp, "failed to unpack a "
                         "Histo-Scope item", NULL);
        return TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(id));
    return TCL_OK;
}

tcl_routine(find_data_mismatch)
{
    int i, size1, size2, sizemin;
    double dtmp, deps;
    float diff, *array1, *array2;

    const Tcl_ObjType* byteArrayTypePtr = Tcl_GetObjType("bytearray");
    tcl_require_objc(4);
    if (objv[1]->typePtr != byteArrayTypePtr)
    {
	Tcl_SetResult(interp, "the first argument is not a byte array", TCL_STATIC);
	return TCL_ERROR;
    }
    if (objv[2]->typePtr != byteArrayTypePtr)
    {
	Tcl_SetResult(interp, "the second argument is not a byte array", TCL_STATIC);
	return TCL_ERROR;
    }
    if (Tcl_GetDoubleFromObj(interp, objv[3], &deps) != TCL_OK)
	return TCL_ERROR;
    if (deps < 0.0)
    {
	Tcl_SetResult(interp, "tolerance can not be negative", TCL_STATIC);
	return TCL_ERROR;
    }
    array1 = (float *)Tcl_GetByteArrayFromObj(objv[1], &size1);
    size1 /= sizeof(float);
    array2 = (float *)Tcl_GetByteArrayFromObj(objv[2], &size2);
    size2 /= sizeof(float);
    if (size1 == 0 && size2 == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewIntObj(-1));
	return TCL_OK;
    }
    if (size2 > size1)
	sizemin = size1;
    else
	sizemin = size2;
    for (i=0; i<sizemin; ++i)
    {
	diff = array1[i] - array2[i];
	if (diff == 0.f) continue;
	dtmp = 2.0*diff/(fabs((double)array1[i]) + fabs((double)array2[i]));
	if (dtmp < -deps || dtmp > deps)
	{
	    /* Outside of tolerance */
	    Tcl_SetObjResult(interp, Tcl_NewIntObj(i));
	    return TCL_OK;
	}
    }
    if (size1 != size2)
    {
	Tcl_SetObjResult(interp, Tcl_NewIntObj(sizemin));
	return TCL_OK;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(-1));
    return TCL_OK;
}

tcl_routine(fill_histogram)
{
    double weight = 1.0, dx, dy, dz;
    int ndim, id, icycle, ncycles, size, objtype;
    float fw, fx, fy = 0.f, fz = 0.f;
    float *array = NULL;
    Tcl_Obj **listObjElem;

    const Tcl_ObjType* byteArrayTypePtr = Tcl_GetObjType("bytearray");
    tcl_objc_range(3,4);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
        return TCL_ERROR;
    ndim = histo_dim_from_type(hs_type(id));
    if (ndim <= 0)
    {
        if (hs_type(id) == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a valid Histo-Scope id", NULL);
        else
	    Tcl_AppendResult(interp, "item with id ",
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a histogram", NULL);
        return TCL_ERROR;
    }
    if (objc > 3)
        if (Tcl_GetDoubleFromObj(interp, objv[3], &weight) != TCL_OK)
            return TCL_ERROR;
    fw = (float)weight;

    /* Check the object type of the second argument */
    if (objv[2]->typePtr == byteArrayTypePtr)
    {
	array = (float *)Tcl_GetByteArrayFromObj(objv[2], &size);
	size /= sizeof(float);
	objtype = 0;
    }
    else
    {
	if (Tcl_ListObjGetElements(interp, objv[2], &size, &listObjElem) != TCL_OK)
	    return TCL_ERROR;    
	objtype = 1;
    }

    /* Check the size of the list or array */
    if (size == 0)
        return TCL_OK;
    if (size % ndim)
    {
        Tcl_SetResult(interp, "argument size is incompatible with "
                      "histogram dimensionality", TCL_STATIC);
	return TCL_ERROR;
    }
    ncycles = size/ndim;

    if (objtype)
    {
	/* Argument is a list */
        for (icycle=0; icycle<ncycles; ++icycle)
        {
            switch (ndim)
            {
            case 3:
                if (Tcl_GetDoubleFromObj(interp, listObjElem[2], &dz) != TCL_OK)
                    return TCL_ERROR;
                else
                    fz = (float)dz;
            case 2:
                if (Tcl_GetDoubleFromObj(interp, listObjElem[1], &dy) != TCL_OK)
                    return TCL_ERROR;
                else
                    fy = (float)dy;
            case 1:
                if (Tcl_GetDoubleFromObj(interp, listObjElem[0], &dx) != TCL_OK)
                    return TCL_ERROR;
                else
                    fx = (float)dx;
                break;
            default:
                assert(0);
            }
            switch (ndim)
            {
            case 1:
                hs_fill_1d_hist(id, fx, fw);
                break;
            case 2:
                hs_fill_2d_hist(id, fx, fy, fw);
                break;
            case 3:
                hs_fill_3d_hist(id, fx, fy, fz, fw);
                break;
             default:
                assert(0);
            }
            listObjElem += ndim;
        }
    }
    else
    {
	/* Argument is a byte array */
        for (icycle=0; icycle<ncycles; ++icycle)
        {
            switch (ndim)
            {
            case 1:
                hs_fill_1d_hist(id, array[0], fw);
                break;
            case 2:
                hs_fill_2d_hist(id, array[0], array[1], fw);
                break;
            case 3:
                hs_fill_3d_hist(id, array[0], array[1], array[2], fw);
                break;
            default:
                assert(0);
            }
            array += ndim;
        }
    }
    return TCL_OK;
}

tcl_routine(fill_ntuple)
{
    static int num_var = -1, old_id = -101239;
    int i, id, size, objtype, status, icycle, ncycles;
    float *array = NULL;
    double dtmp;
    Tcl_Obj **listObjElem;
    float local[NTUPLE_FILL_BUFLEN];

    const Tcl_ObjType* byteArrayTypePtr = Tcl_GetObjType("bytearray");
    tcl_require_objc(3);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
	return TCL_ERROR;

    /* Get the ntuple size */
    if (old_id != id)
    {
	old_id = id;
	num_var = hs_num_variables(id);
	if (num_var < 0)
	{
	    old_id = -101239;
	    if (hs_type(id) == HS_NONE)
		Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
				 " is not a valid Histo-Scope id", NULL);
	    else
		Tcl_AppendResult(interp, "item with id ", 
				 Tcl_GetStringFromObj(objv[1], NULL),
				 " is not an ntuple", NULL);
	    return TCL_ERROR;
	}
    }

    /* Check the object type of the second argument */
    if (objv[2]->typePtr == byteArrayTypePtr)
    {
	array = (float *)Tcl_GetByteArrayFromObj(objv[2], &size);
	size /= sizeof(float);
	objtype = 0;
    }
    else
    {
	if (Tcl_ListObjGetElements(interp, objv[2], &size, &listObjElem) != TCL_OK)
	    return TCL_ERROR;    
	objtype = 1;
    }

    /* Check the size of the list or array */
    if (size == 0)
    {
        Tcl_SetObjResult(interp, Tcl_NewIntObj(id));
        return TCL_OK;
    }
    if (size % num_var)
    {
	if (hs_type(id) == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a valid Histo-Scope id", NULL);
	else
	    Tcl_SetResult(interp, "argument size is incompatible "
                          "with ntuple dimension", TCL_STATIC);
	return TCL_ERROR;
    }
    ncycles = size/num_var;

    /* Cycle over the data */
    status = id;
    if (objtype)
    {
	/* Argument is a list */
	if (num_var <= NTUPLE_FILL_BUFLEN)
	    array = local;
	else if ((array = (float *)malloc(num_var*sizeof(float))) == NULL)
	{
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    return TCL_ERROR;
	}
        for (icycle=0; icycle<ncycles && status == id; ++icycle)
        {
            for (i=0; i<num_var; ++i)
            {
                if (Tcl_GetDoubleFromObj(
                        interp, listObjElem[i], &dtmp) != TCL_OK)
                {
                    if (num_var > NTUPLE_FILL_BUFLEN) free(array);
                    return TCL_ERROR;
                }
                array[i] = (float)dtmp;
            }
            listObjElem += num_var;
            status = hs_fill_ntuple(id, array);
        }
	if (num_var > NTUPLE_FILL_BUFLEN) free(array);
    }
    else
    {
	/* Argument is a byte array */
        for (icycle=0; icycle<ncycles && status == id; ++icycle)
            status = hs_fill_ntuple(id, array+icycle*num_var);
    }

    /* Check for errors */
    if (status < 0)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    else if (status == 0)
    {
	/* Apart from an unlikely case of call before initialization,
	   this can only happen if the ntuple has been deleted */
	if (hs_type(id) == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a valid Histo-Scope id", NULL);
	else
	    Tcl_AppendResult(interp, "Internal error in ",
			     Tcl_GetStringFromObj(objv[0], NULL),
			     ". This is a bug. Please report.", NULL);
	return TCL_ERROR;
    }

    Tcl_SetObjResult(interp, Tcl_NewIntObj(status));
    return TCL_OK;
}

tcl_routine(set_indicator)
{
  int id;
  double value;
  
  tcl_require_objc(3);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[2], &value) != TCL_OK)
    return TCL_ERROR;
  hs_set_indicator(id, (float)value);
  return TCL_OK;
}

tcl_routine(read_control)
{
  int id;
  float value;

  tcl_require_objc(2);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
      return TCL_ERROR;
  hs_read_control(id, &value);
  Tcl_SetObjResult(interp, Tcl_NewDoubleObj((double)value));
  return TCL_OK;  
}

tcl_routine(check_trigger)
{
  int id;

  tcl_require_objc(2);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  Tcl_SetObjResult(interp, Tcl_NewIntObj(hs_check_trigger(id)));
  return TCL_OK;
}

#define parse_block_errors do {\
    if (get_float_array_from_binary_or_list(interp, objv[2], &array_pos,\
                                            &size_pos, &free_pos) != TCL_OK)\
        goto fail;\
    if (size_pos != nbins)\
    {\
        Tcl_SetResult(interp,\
                      "positive errors size is incompatible with histogram binning",\
                      TCL_STATIC);\
        goto fail;\
    }\
    if (size_pos == 0)\
    {\
        if (free_pos)\
            free(array_pos);\
        array_pos = NULL;\
        free_pos = 0;\
    }\
    if (objc > 3)\
    {\
        if (get_float_array_from_binary_or_list(interp, objv[3], &array_neg,\
                                                &size_neg, &free_neg) != TCL_OK)\
            goto fail;\
        if (size_neg != nbins)\
        {\
            Tcl_SetResult(interp,\
                          "negative errors size is incompatible with histogram binning",\
                          TCL_STATIC);\
            goto fail;\
        }\
    }\
} while(0);

tcl_routine(set_1d_errors)
{
    int id, size_pos, size_neg, nbins;
    float *array_pos = NULL, *array_neg = NULL;
    int free_pos = 0, free_neg = 0, status = TCL_ERROR;

    tcl_objc_range(3,4);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
        return TCL_ERROR;
    nbins = hs_1d_hist_num_bins(id);
    if (nbins <= 0)
    {
        if (hs_type(id) != HS_1D_HISTOGRAM)
        {
            if (hs_type(id) == HS_NONE)
                Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
                                 " is not a valid Histo-Scope id", NULL);
            else
                Tcl_AppendResult(interp, "item with id ", 
                                 Tcl_GetStringFromObj(objv[1], NULL),
                                 " is not a 1d histogram", NULL);
        }
        return TCL_ERROR;
    }
    parse_block_errors;
    hs_set_1d_errors(id, array_pos, array_neg);
    status = TCL_OK;

 fail:
    if (array_pos && free_pos)
        free(array_pos);
    if (array_neg && free_neg)
        free(array_neg);
    return status;
}

tcl_routine(set_2d_errors)
{
    int id, size_pos, size_neg;
    int num_x_bins, num_y_bins, nbins;
    float *array_pos = NULL, *array_neg = NULL;
    int free_pos = 0, free_neg = 0, status = TCL_ERROR;
  
    tcl_objc_range(3,4);
    verify_2d_histo(id,1);    
    hs_2d_hist_num_bins(id, &num_x_bins, &num_y_bins);
    nbins = num_x_bins*num_y_bins;
    parse_block_errors;
    hs_set_2d_errors(id, array_pos, array_neg);
    status = TCL_OK;

 fail:
    if (array_pos && free_pos)
        free(array_pos);
    if (array_neg && free_neg)
        free(array_neg);
    return status;
}

tcl_routine(set_3d_errors)
{
    int id, size_pos, size_neg;
    int num_x_bins, num_y_bins, num_z_bins, nbins;
    float *array_pos = NULL, *array_neg = NULL;
    int free_pos = 0, free_neg = 0, status = TCL_ERROR;
  
    tcl_objc_range(3,4);
    verify_3d_histo(id,1);
    hs_3d_hist_num_bins(id, &num_x_bins, &num_y_bins, &num_z_bins);
    nbins = num_x_bins*num_y_bins*num_z_bins;
    parse_block_errors;
    hs_set_3d_errors(id, array_pos, array_neg);
    status = TCL_OK;

 fail:
    if (array_pos && free_pos)
        free(array_pos);
    if (array_neg && free_neg)
        free(array_neg);
    return status;
}

VOID_FUNCT_WITH_ONE_INT_ARG(reset)
INT_FUNCT_WITH_ONE_CHAR_ARG(save_file)
VOID_FUNCT_WITH_ONE_INT_ARG(delete)

tcl_routine(delete_items)
{
  int i, j, status, nids, idnum = 0, idlen = 0;
  int *ids = NULL;
  Tcl_Obj **listObjElem;

  /* Assume that all arguments are lists of items to be deleted */
  /* First, parse all lists and figure out the total number of
     ids to be deleted */
  for (i=1; i<objc; ++i)
  {
      if (Tcl_ListObjGetElements(interp, objv[i], &nids, &listObjElem) != TCL_OK)
	  return TCL_ERROR;
      else
          idlen += nids;
  }
  if (idlen == 0)
      return TCL_OK;

  /* Now, parse the ids */
  if ((ids = (int *)malloc(idlen*sizeof(int))) == NULL)
  {
      Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
      return TCL_ERROR;
  }
  for (i=1; i<objc; ++i)
  {
      if (objc > 2)
      {
          status = Tcl_ListObjGetElements(interp, objv[i], &nids, &listObjElem);
          assert(status == TCL_OK);
      }
      for (j=0; j<nids; ++j)
      {
	  if (Tcl_GetIntFromObj(interp, listObjElem[j], ids+idnum++) != TCL_OK)
	  {
	      free(ids);
	      return TCL_ERROR;
	  }
      }
  }
  assert(idnum == idlen);
  hs_delete_items(ids, idlen);
  free(ids);
  return TCL_OK; 
}

tcl_routine(change_uid)
{
  int id, uid;
  
  tcl_require_objc(3);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[2], &uid) != TCL_OK)
    return TCL_ERROR;
  hs_change_uid(id, uid);
  return TCL_OK;
}

tcl_routine(change_category)
{
  int id;
  
  tcl_require_objc(3);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  hs_change_category(id, Tcl_GetStringFromObj(objv[2], NULL));
  return TCL_OK;
}

tcl_routine(change_title)
{
  int id;
  
  tcl_require_objc(3);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  hs_change_title(id, Tcl_GetStringFromObj(objv[2], NULL));
  return TCL_OK;
}

tcl_routine(id)
{
  int uid;
  
  tcl_require_objc(3);
  if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
    return TCL_ERROR;
  Tcl_SetObjResult(interp, Tcl_NewIntObj(
      hs_id(uid, Tcl_GetStringFromObj(objv[2], NULL))));
  return TCL_OK;
}

tcl_routine(id_from_title)
{
  tcl_require_objc(3);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(
      hs_id_from_title(Tcl_GetStringFromObj(objv[1], NULL), 
		       Tcl_GetStringFromObj(objv[2], NULL))));
  return TCL_OK;
}

tcl_routine(list_items)
{
  int matchFlg, max_items, i, nids;
  int *ids;
  Tcl_Obj *list;

  tcl_require_objc(4);
  if (Tcl_GetIntFromObj(interp, objv[3], &matchFlg) != TCL_OK)
    return TCL_ERROR;

  max_items = hs_num_items()+1;
  if ((ids = (int *)malloc(max_items*sizeof(int))) == NULL)
  {
    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
    return TCL_ERROR;
  }

  list = Tcl_NewListObj(0, NULL);
  nids = hs_list_items(Tcl_GetStringFromObj(objv[1], NULL), 
		       Tcl_GetStringFromObj(objv[2], NULL), 
		       ids, max_items, matchFlg);
  for (i=0; i<nids; i++)
      Tcl_ListObjAppendElement(interp, list, Tcl_NewIntObj(ids[i]));
  Tcl_SetObjResult(interp, list);
  free(ids);
  return TCL_OK;
}

INT_FUNCT_WITH_ONE_INT_ARG(uid)

tcl_routine(category)
{
  int id;
  char tcl_api_buffer[256];
  
  tcl_require_objc(2);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (hs_category(id, tcl_api_buffer) < 0)
  {
    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL), 
		     " is not a valid Histo-Scope id", NULL);
    return TCL_ERROR;
  }
  Tcl_SetResult(interp, tcl_api_buffer, TCL_VOLATILE);
  return TCL_OK;  
}

tcl_routine(title)
{
  int id;
  char tcl_api_buffer[256];
  
  tcl_require_objc(2);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;  
  if (hs_title(id, tcl_api_buffer) < 0)
  {
    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL), 
		     " is not a valid Histo-Scope id", NULL);
    return TCL_ERROR;
  }
  Tcl_SetResult(interp, tcl_api_buffer, TCL_VOLATILE);
  return TCL_OK;  
}

tcl_routine(type)
{
  int type, id;
  char tcl_api_buffer[32];

  tcl_require_objc(2);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  type = hs_type(id);
  if (type >= 0 && type < N_HS_DATA_TYPES)
  {
      Tcl_SetResult(interp, (char *)hs_typename_strings[type], TCL_STATIC);
      return TCL_OK;
  }

  sprintf(tcl_api_buffer, "%d", type);
  Tcl_AppendResult(interp, "Invalid integer type ", tcl_api_buffer,
		   " for histoscope item with id ", 
		   Tcl_GetStringFromObj(objv[1], NULL),
		   ". This is a bug. Please report.", NULL);
  return TCL_ERROR;
}

tcl_routine(read_file)
{
  int i;
  char *c = NULL;

  tcl_objc_range(2, 3);
  if (objc > 2)
  {
      c = Tcl_GetStringFromObj(objv[2], NULL);
      if (strlen(c) == 0)
	  c = NULL;
  }
  i = hs_read_file(Tcl_GetStringFromObj(objv[1], NULL), c);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(i));
  return TCL_OK;
}

tcl_routine(read_file_items)
{
  int i, nids = 0;
  int *ids = NULL;
  Tcl_Obj **listObjElem;

  tcl_require_objc(5);
  if (strcmp(Tcl_GetStringFromObj(objv[4], NULL), "") != 0)
  {  
      if (Tcl_ListObjGetElements(interp, objv[4], &nids, &listObjElem) != TCL_OK)
	  return TCL_ERROR;

      if ((ids = (int *)malloc(nids*sizeof(int))) == NULL)
      {
	  Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	  return TCL_ERROR;
      }

      for (i=0; i < nids; i++)
	  if (Tcl_GetIntFromObj(interp, listObjElem[i], ids+i) != TCL_OK)
	  {
	      free(ids);
	      return TCL_ERROR;
	  }
  }
  
  Tcl_SetObjResult(interp, Tcl_NewIntObj(
      hs_read_file_items(Tcl_GetStringFromObj(objv[1], NULL),
			 Tcl_GetStringFromObj(objv[2], NULL),
			 Tcl_GetStringFromObj(objv[3], NULL),
			 ids, nids)));
  if (ids)
    free(ids);
  return TCL_OK;
}

tcl_routine(save_file_items)
{
  int i, nids = 0;
  int *ids = NULL;
  Tcl_Obj **listObjElem;

  tcl_require_objc(4);
  
  if (strcmp(Tcl_GetStringFromObj(objv[3], NULL), "") != 0)
  {
      if (Tcl_ListObjGetElements(interp, objv[3], &nids, &listObjElem) != TCL_OK)
	  return TCL_ERROR;
      
      if ((ids = (int *)malloc(nids*sizeof(int))) == NULL)
      {
	  Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	  return TCL_ERROR;
      }

      for (i=0; i < nids; i++)
	  if (Tcl_GetIntFromObj(interp, listObjElem[i], ids+i) != TCL_OK)
	  {
	      free(ids);
	      return TCL_ERROR;
	  }
  }

  Tcl_SetObjResult(interp, Tcl_NewIntObj(
	  hs_save_file_items(Tcl_GetStringFromObj(objv[1], NULL), 
			     Tcl_GetStringFromObj(objv[2], NULL), 
			     ids, nids)));
  if (ids)
    free(ids);
  return TCL_OK; 
}

VOID_FUNCT_WITH_ONE_CHAR_ARG(delete_category)
INT_FUNCT_WITH_VOID_ARG(num_items)

#define parse_block_data do {\
    if (get_float_array_from_binary_or_list(interp, objv[2], &array_data,\
                                            &size_data, &free_data) != TCL_OK)\
        goto fail;\
    if (size_data != nbins)\
    {\
        Tcl_SetResult(interp,\
                      "data size is incompatible with histogram binning",\
                      TCL_STATIC);\
        goto fail;\
    }\
    if (objc > 3)\
    {\
        if (get_float_array_from_binary_or_list(interp, objv[3], &array_pos,\
                                                &size_pos, &free_pos) != TCL_OK)\
            goto fail;\
        if (size_pos != nbins)\
        {\
            Tcl_SetResult(interp,\
                          "positive errors size is incompatible with histogram binning",\
                          TCL_STATIC);\
            goto fail;\
        }\
    }\
    if (objc > 4)\
    {\
        if (get_float_array_from_binary_or_list(interp, objv[4], &array_neg,\
                                                &size_neg, &free_neg) != TCL_OK)\
            goto fail;\
        if (size_neg != nbins)\
        {\
            Tcl_SetResult(interp,\
                          "negative errors size is incompatible with histogram binning",\
                          TCL_STATIC);\
            goto fail;\
        }\
    }\
} while(0);

tcl_routine(1d_hist_block_fill)
{
    int id, size_data, size_pos, size_neg, nbins;
    float *array_data = NULL, *array_pos = NULL, *array_neg = NULL;
    int free_data = 0, free_pos = 0, free_neg = 0, status = TCL_ERROR;

    tcl_objc_range(3,5);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
        return TCL_ERROR;
    nbins = hs_1d_hist_num_bins(id);
    if (nbins <= 0)
    {
        if (hs_type(id) != HS_1D_HISTOGRAM)
        {
            if (hs_type(id) == HS_NONE)
                Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
                                 " is not a valid Histo-Scope id", NULL);
            else
                Tcl_AppendResult(interp, "item with id ", 
                                 Tcl_GetStringFromObj(objv[1], NULL),
                                 " is not a 1d histogram", NULL);
        }
        return TCL_ERROR;
    }
    parse_block_data;
    hs_1d_hist_block_fill(id, array_data, array_pos, array_neg);
    status = TCL_OK;

 fail:
    if (array_data && free_data)
        free(array_data);
    if (array_pos && free_pos)
        free(array_pos);
    if (array_neg && free_neg)
        free(array_neg);
    return status;
}

tcl_routine(2d_hist_block_fill)
{
    int id, size_data, size_pos, size_neg;
    int num_x_bins, num_y_bins, nbins;
    float *array_data = NULL, *array_pos = NULL, *array_neg = NULL;
    int free_data = 0, free_pos = 0, free_neg = 0, status = TCL_ERROR;

    tcl_objc_range(3,5);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
        return TCL_ERROR;
    hs_2d_hist_num_bins(id, &num_x_bins, &num_y_bins);
    if (num_x_bins <= 0)
    {
        if (hs_type(id) != HS_2D_HISTOGRAM)
        {
            if (hs_type(id) == HS_NONE)
                Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL), 
                                 " is not a valid Histo-Scope id", NULL);
            else
                Tcl_AppendResult(interp, "item with id ", 
                                 Tcl_GetStringFromObj(objv[1], NULL), 
                                 " is not a 2d histogram", NULL);
        }
        return TCL_ERROR;
    }
    nbins = num_x_bins*num_y_bins;
    parse_block_data;
    hs_2d_hist_block_fill(id, array_data, array_pos, array_neg);
    status = TCL_OK;

 fail:
    if (array_data && free_data)
        free(array_data);
    if (array_pos && free_pos)
        free(array_pos);
    if (array_neg && free_neg)
        free(array_neg);
    return status;
}

tcl_routine(2d_fill_from_matrix)
{
    int i, id, num_x_bins, num_y_bins, nbins, nrows, ncols, status = TCL_ERROR;
    float *array_data = NULL, *array_pos = NULL, *array_neg = NULL, *fmem = NULL;
    double *ddata = NULL, *dpos = NULL, *dneg = NULL;

    tcl_objc_range(3,5);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
        return TCL_ERROR;
    hs_2d_hist_num_bins(id, &num_x_bins, &num_y_bins);
    if (num_x_bins <= 0)
    {
        if (hs_type(id) != HS_2D_HISTOGRAM)
        {
            if (hs_type(id) == HS_NONE)
                Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL), 
                                 " is not a valid Histo-Scope id", NULL);
            else
                Tcl_AppendResult(interp, "item with id ", 
                                 Tcl_GetStringFromObj(objv[1], NULL), 
                                 " is not a 2d histogram", NULL);
        }
        return TCL_ERROR;
    }
    nbins = num_x_bins*num_y_bins;
    fmem = (float *)malloc(3*nbins*sizeof(float));
    if (fmem == NULL)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        return TCL_ERROR;
    }
    if (parse_matrix(interp, objv[2], 'f', &nrows, &ncols, &ddata) != TCL_OK)
	goto fail;
    if (nrows != num_y_bins || ncols != num_x_bins)
    {
        Tcl_SetResult(interp, "bad data dimensionality", TCL_VOLATILE);
        goto fail;
    }
    array_data = fmem;
    for (i=0; i<nbins; ++i)
        array_data[i] = (float)ddata[i];
    if (objc > 3)
    {
        if (parse_matrix(interp, objv[3], 'f', &nrows, &ncols, &dpos) != TCL_OK)
            goto fail;
        if (nrows != num_y_bins || ncols != num_x_bins)
        {
            Tcl_SetResult(interp, "bad positive errors dimensionality", TCL_VOLATILE);
            goto fail;
        }
        array_pos = fmem + nbins;
        for (i=0; i<nbins; ++i)
            array_pos[i] = (float)dpos[i];
    }
    if (objc > 4)
    {
        if (parse_matrix(interp, objv[3], 'f', &nrows, &ncols, &dneg) != TCL_OK)
            goto fail;
        if (nrows != num_y_bins || ncols != num_x_bins)
        {
            Tcl_SetResult(interp, "bad negative errors dimensionality", TCL_VOLATILE);
            goto fail;
        }
        array_neg = fmem + 2*nbins;
        for (i=0; i<nbins; ++i)
            array_neg[i] = (float)dneg[i];
    }
    hs_2d_hist_block_fill(id, array_data, array_pos, array_neg);
    status = TCL_OK;

 fail:
    if (dneg) free(dneg);
    if (dpos) free(dpos);
    if (ddata) free(ddata);
    if (fmem) free(fmem);
    return status;
}

tcl_routine(3d_hist_block_fill)
{
    int id, size_data, size_pos, size_neg;
    int num_x_bins, num_y_bins, num_z_bins, nbins;
    float *array_data = NULL, *array_pos = NULL, *array_neg = NULL;
    int free_data = 0, free_pos = 0, free_neg = 0, status = TCL_ERROR;

    tcl_objc_range(3,5);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
        return TCL_ERROR;
    hs_3d_hist_num_bins(id, &num_x_bins, &num_y_bins, &num_z_bins);
    if (num_x_bins <= 0)
    {
        if (hs_type(id) != HS_3D_HISTOGRAM)
        {
            if (hs_type(id) == HS_NONE)
                Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL), 
                                 " is not a valid Histo-Scope id", NULL);
            else
                Tcl_AppendResult(interp, "item with id ", 
                                 Tcl_GetStringFromObj(objv[1], NULL), 
                                 " is not a 3d histogram", NULL);
        }
        return TCL_ERROR;
    }
    nbins = num_x_bins*num_y_bins*num_z_bins;
    parse_block_data;
    hs_3d_hist_block_fill(id, array_data, array_pos, array_neg);
    status = TCL_OK;

 fail:
    if (array_data && free_data)
        free(array_data);
    if (array_pos && free_pos)
        free(array_pos);
    if (array_neg && free_neg)
        free(array_neg);
    return status;
}

tcl_routine(1d_hist_num_bins)
{
    int id;
    tcl_require_objc(2);
    verify_1d_histo(id,1);
    Tcl_SetObjResult(interp, Tcl_NewIntObj(hs_1d_hist_num_bins(id)));
    return TCL_OK;
}

tcl_routine(2d_hist_num_bins)
{
  int id;
  int num_x_bins, num_y_bins;
  Tcl_Obj *bins[2];

  tcl_require_objc(2);
  verify_2d_histo(id,1);
  hs_2d_hist_num_bins(id, &num_x_bins, &num_y_bins);
  bins[0] = Tcl_NewIntObj(num_x_bins);
  bins[1] = Tcl_NewIntObj(num_y_bins);
  Tcl_SetObjResult(interp, Tcl_NewListObj(2, bins));
  return TCL_OK;
}

tcl_routine(3d_hist_num_bins)
{
  int id;
  int num_x_bins, num_y_bins, num_z_bins;
  Tcl_Obj *bins[3];

  tcl_require_objc(2);
  verify_3d_histo(id,1);
  hs_3d_hist_num_bins(id, &num_x_bins, &num_y_bins, &num_z_bins);
  bins[0] = Tcl_NewIntObj(num_x_bins);
  bins[1] = Tcl_NewIntObj(num_y_bins);
  bins[2] = Tcl_NewIntObj(num_z_bins);
  Tcl_SetObjResult(interp, Tcl_NewListObj(3, bins));
  return TCL_OK;
}

tcl_routine(1d_hist_range)
{
  int id;
  float xmin, xmax;
  Tcl_Obj *x[2];

  tcl_require_objc(2);
  verify_1d_histo(id,1);
  hs_1d_hist_range(id, &xmin, &xmax);
  x[0] = Tcl_NewDoubleObj((double)xmin);
  x[1] = Tcl_NewDoubleObj((double)xmax);
  Tcl_SetObjResult(interp, Tcl_NewListObj(2, x));
  return TCL_OK;
}

tcl_routine(2d_hist_range)
{
  int id;
  float xmin, xmax, ymin, ymax;
  Tcl_Obj *x[4];

  tcl_require_objc(2);
  verify_2d_histo(id,1);
  hs_2d_hist_range(id, &xmin, &xmax, &ymin, &ymax);
  x[0] = Tcl_NewDoubleObj((double)xmin);
  x[1] = Tcl_NewDoubleObj((double)xmax);
  x[2] = Tcl_NewDoubleObj((double)ymin);
  x[3] = Tcl_NewDoubleObj((double)ymax);
  Tcl_SetObjResult(interp, Tcl_NewListObj(4, x));
  return TCL_OK;
}

tcl_routine(3d_hist_range)
{
  int id;
  float xmin, xmax, ymin, ymax, zmin, zmax;
  Tcl_Obj *x[6];

  tcl_require_objc(2);
  verify_3d_histo(id,1);
  hs_3d_hist_range(id, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
  x[0] = Tcl_NewDoubleObj((double)xmin);
  x[1] = Tcl_NewDoubleObj((double)xmax);
  x[2] = Tcl_NewDoubleObj((double)ymin);
  x[3] = Tcl_NewDoubleObj((double)ymax);
  x[4] = Tcl_NewDoubleObj((double)zmin);
  x[5] = Tcl_NewDoubleObj((double)zmax);
  Tcl_SetObjResult(interp, Tcl_NewListObj(6, x));
  return TCL_OK;
}

INT_FUNCT_WITH_ONE_INT_ARG(num_entries)

tcl_routine(1d_hist_bin_contents)
{
  int id, size;
  float *data;
  
  tcl_require_objc(2);
  verify_1d_histo(id, 1);
  size = hs_1d_hist_num_bins(id);
  if ((data = (float *)malloc(size*sizeof(float))) == NULL)
  {
    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
    return TCL_ERROR;
  }
  hs_1d_hist_bin_contents(id, data);
  Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((unsigned char *)data,
					       size*sizeof(float)));
  free(data);
  return TCL_OK;
}

tcl_routine(item_properties)
{
    int type, id;
    char tcl_api_buffer[260];
    Tcl_Obj *answer;

    tcl_require_objc(2);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
	return TCL_ERROR;
    type = hs_type(id);
    if (type < 0 || type >= N_HS_DATA_TYPES)
    {
	sprintf(tcl_api_buffer, "%d", type);
	Tcl_AppendResult(interp, "Invalid integer type ", tcl_api_buffer,
			 " for histoscope item with id ", 
			 Tcl_GetStringFromObj(objv[1], NULL),
			 ". This is a bug. Please report.", NULL);
	return TCL_ERROR;
    }
    if ((answer = Tcl_NewListObj(0, NULL)) == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    if (type == HS_NONE || type == HS_NFIT || type == HS_CONFIG_STRING)
    {
	Tcl_ListObjAppendElement(interp, answer, Tcl_NewStringObj("",-1));
	Tcl_ListObjAppendElement(interp, answer, Tcl_NewStringObj("",-1));
	Tcl_ListObjAppendElement(interp, answer, Tcl_NewStringObj("",-1));
    }
    else
    {
	Tcl_ListObjAppendElement(interp, answer,
				 Tcl_NewIntObj(hs_uid(id)));
	hs_category(id, tcl_api_buffer);
	Tcl_ListObjAppendElement(interp, answer,
				 Tcl_NewStringObj(tcl_api_buffer,-1));
	hs_title(id, tcl_api_buffer);
	Tcl_ListObjAppendElement(interp, answer,
				 Tcl_NewStringObj(tcl_api_buffer,-1));
    }
    Tcl_ListObjAppendElement(interp, answer,
	 Tcl_NewStringObj(hs_typename_strings[type],-1));
    Tcl_SetObjResult(interp, answer);
    return TCL_OK;
}

tcl_routine(2d_rotate_bins)
{
    int i, j, id1, id2, shift, errtype, index, axis;
    int nbins, num_x_bins, num_y_bins, num_x_bins_2, num_y_bins_2;
    float *data1, *poserr1, *negerr1, *data2, *poserr2, *negerr2;
    float *r, *w, *rp, *wp, *rn, *wn;

    tcl_require_objc(5);
    verify_2d_histo(id1, 1);
    verify_2d_histo(id2, 2);
    if (Tcl_GetIntFromObj(interp, objv[3], &shift) != TCL_OK)
	return TCL_ERROR;
    if (get_axis_from_obj(interp, objv[4], 2, 1, &axis) != TCL_OK)
	return TCL_ERROR;
    hs_2d_hist_num_bins(id1, &num_x_bins, &num_y_bins);
    hs_2d_hist_num_bins(id2, &num_x_bins_2, &num_y_bins_2);
    if (num_x_bins != num_x_bins_2)
    {
	Tcl_AppendResult(interp, "2d histograms with ids ",
			 Tcl_GetStringFromObj(objv[1],NULL), " and ",
			 Tcl_GetStringFromObj(objv[2],NULL), 
			 " have different number of X bins", NULL);
	return TCL_ERROR;
    }
    if (num_y_bins != num_y_bins_2)
    {
	Tcl_AppendResult(interp, "2d histograms with ids ",
			 Tcl_GetStringFromObj(objv[1],NULL), " and ",
			 Tcl_GetStringFromObj(objv[2],NULL), 
			 " have different number of Y bins", NULL);
	return TCL_ERROR;
    }
    nbins = num_x_bins * num_y_bins;
    data1 = (float *)malloc(6*nbins*sizeof(float));
    if (data1 == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    poserr1 = data1+nbins;
    negerr1 = poserr1+nbins;
    data2 = negerr1+nbins;
    poserr2 = data2+nbins;
    negerr2 = poserr2+nbins;
    hs_2d_hist_bin_contents(id1, data1);
    errtype = hs_2d_hist_errors(id1, poserr1, negerr1);
    if (errtype == HS_NO_ERRORS)
    {
	poserr2 = NULL;
	negerr2 = NULL;
    }
    else if (errtype == HS_POS_ERRORS)
    {
	negerr2 = NULL;
    }    
    if (axis == HS_AXIS_X)
    {
	for (i=0; i<num_x_bins; ++i)
	{
	    index = (i+shift) % num_x_bins;
	    if (index < 0)
		index += num_x_bins;
	    w  = data2 + index*num_y_bins;
	    wp = poserr2 + index*num_y_bins;
	    wn = negerr2 + index*num_y_bins;
	    r  = data1 + i*num_y_bins;
	    rp = poserr1 + i*num_y_bins;
	    rn = negerr1 + i*num_y_bins;
	    for (j=0; j<num_y_bins; ++j)
	    {
		w[j] = r[j];
		if (poserr2)
		    wp[j] = rp[j];
		if (negerr2)
		    wn[j] = rn[j];
	    }
	}
    }
    else
    {
	for (i=0; i<num_x_bins; ++i)
	{
	    w = data2 + i*num_y_bins;
	    wp = poserr2 + i*num_y_bins;
	    wn = negerr2 + i*num_y_bins;
	    r = data1 + i*num_y_bins;
	    rp = poserr1 + i*num_y_bins;
	    rn = negerr1 + i*num_y_bins;
	    for (j=0; j<num_y_bins; ++j)
	    {
		index = (j+shift) % num_y_bins;
		if (index < 0)
		    index += num_y_bins;
		w[index] = r[j];
		if (poserr2)
		    wp[index] = rp[j];
		if (negerr2)
		    wn[index] = rn[j];
	    }
	}
    }
    hs_2d_hist_block_fill(id2, data2, poserr2, negerr2);
    free(data1);
    return TCL_OK;
}

tcl_routine(1d_rotate_bins)
{
    int i, id1, id2, shift, nbins, errtype, index;
    float *data1, *poserr1, *negerr1, *data2, *poserr2, *negerr2;

    tcl_require_objc(4);
    verify_1d_histo(id1, 1);
    verify_1d_histo(id2, 2);
    if (Tcl_GetIntFromObj(interp, objv[3], &shift) != TCL_OK)
	return TCL_ERROR;
    nbins = hs_1d_hist_num_bins(id1);
    if (nbins != hs_1d_hist_num_bins(id2))
    {
	Tcl_AppendResult(interp, "1d histograms with ids ",
			 Tcl_GetStringFromObj(objv[1],NULL), " and ",
			 Tcl_GetStringFromObj(objv[2],NULL), 
			 " have different number of bins", NULL);
	return TCL_ERROR;
    }
    data1 = (float *)malloc(6*nbins*sizeof(float));
    if (data1 == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    poserr1 = data1+nbins;
    negerr1 = poserr1+nbins;
    data2 = negerr1+nbins;
    poserr2 = data2+nbins;
    negerr2 = poserr2+nbins;
    hs_1d_hist_bin_contents(id1, data1);
    errtype = hs_1d_hist_errors(id1, poserr1, negerr1);
    if (errtype == HS_NO_ERRORS)
    {
	poserr2 = NULL;
	negerr2 = NULL;
    }
    else if (errtype == HS_POS_ERRORS)
    {
	negerr2 = NULL;
    }
    for (i=0; i<nbins; ++i)
    {
	index = (i+shift) % nbins;
	if (index < 0)
	    index += nbins;
	data2[index] = data1[i];
	if (poserr2)
	    poserr2[index] = poserr1[i];
	if (negerr2)
	    negerr2[index] = negerr1[i];
    }
    hs_1d_hist_block_fill(id2, data2, poserr2, negerr2);
    free(data1);
    return TCL_OK;
}

tcl_routine(1d_select_bins)
{
    int i, id, isbinnum, isin, size, nfound;
    char *bin_or_coord, *in_or_out;
    double dmin, dmax;
    float fmin, fmax, xmin, xmax, step;
    float *data;
    int *goodbins;
    Tcl_Obj **list;
    
    tcl_require_objc(6);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
	return TCL_ERROR;
    bin_or_coord = Tcl_GetStringFromObj(objv[2], NULL);
    if (Tcl_GetDoubleFromObj(interp, objv[3], &dmin) != TCL_OK)
	return TCL_ERROR;
    else
	fmin = (float)dmin;
    if (Tcl_GetDoubleFromObj(interp, objv[4], &dmax) != TCL_OK)
	return TCL_ERROR;
    else
	fmax = (float)dmax;
    in_or_out = Tcl_GetStringFromObj(objv[5], NULL);
    if (strncmp("bin", bin_or_coord, 3) == 0)
	isbinnum = 1;
    else if (strncmp("coord", bin_or_coord, 5) == 0)
	isbinnum = 0;
    else
    {
	Tcl_AppendResult(interp, "bad argument \"", bin_or_coord, 
			 "\", expected \"bin\" or \"coord\"", NULL);
	return TCL_ERROR;
    }
    if (strncmp("in", in_or_out, 2) == 0)
	isin = 1;
    else if (strncmp("excl", in_or_out, 4) == 0 || 
	     strcmp("out", in_or_out) == 0)
	isin = 0;
    else
    {
	Tcl_AppendResult(interp, "bad argument \"", in_or_out, 
			 "\", expected \"include\" or \"exclude\"", NULL);
	return TCL_ERROR;
    }

    if (hs_type(id) != HS_1D_HISTOGRAM)
    {
	if (hs_type(id) == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a valid Histo-Scope id", NULL);
	else
	    Tcl_AppendResult(interp, "item with id ", 
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a 1d histogram", NULL);
	return TCL_ERROR;
    }
    size = hs_1d_hist_num_bins(id);
    if ((data = (float *)malloc(size*sizeof(float))) == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    if ((goodbins = (int *)malloc(size*sizeof(float))) == NULL)
    {
	free(data);
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    hs_1d_hist_bin_contents(id, data);
    hs_1d_hist_range(id, &xmin, &xmax);
    step = (xmax - xmin)/(float)size;
    nfound = 0;
    for (i=0; i<size; ++i)
    {
	if (fmin <= data[i] && data[i] <= fmax)
	{
	    if (isin)
		goodbins[nfound++] = i;
	}
	else
	{
	    if (!isin)
		goodbins[nfound++] = i;
	}
    }
    if (nfound)
    {
	if ((list = (Tcl_Obj **)malloc(nfound*sizeof(Tcl_Obj *))) == NULL)
	{
	    free(data);
	    free(goodbins);
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    return TCL_ERROR;
	}
	if (isbinnum)
	{
	    for (i=0; i<nfound; ++i)
		list[i] = Tcl_NewIntObj(goodbins[i]);
	}
	else
	{
	    for (i=0; i<nfound; ++i)
		list[i] = Tcl_NewDoubleObj(
		    (double)(xmin + step*(float)(goodbins[i])));
	}
	Tcl_SetObjResult(interp, Tcl_NewListObj(nfound, list));
	free(list);
    }
    free(goodbins);
    free(data);
    return TCL_OK;
}

tcl_routine(2d_select_bins)
{
    int i, j, id, isbinnum, isin, size, nfound, num_x_bins, num_y_bins;
    char *bin_or_coord, *in_or_out;
    double dmin, dmax;
    float fmin, fmax, xmin, xmax, ymin, ymax, xstep, ystep;
    register float datum;
    float *data;
    typedef struct _goodbins
    {    
	int nx;
	int ny;
    } Goodbins;
    Goodbins *goodbins;
    Tcl_Obj *biglist;
    Tcl_Obj *xy[2];

    tcl_require_objc(6);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
	return TCL_ERROR;
    bin_or_coord = Tcl_GetStringFromObj(objv[2], NULL);
    if (Tcl_GetDoubleFromObj(interp, objv[3], &dmin) != TCL_OK)
	return TCL_ERROR;
    else
	fmin = (float)dmin;
    if (Tcl_GetDoubleFromObj(interp, objv[4], &dmax) != TCL_OK)
	return TCL_ERROR;
    else
	fmax = (float)dmax;
    in_or_out = Tcl_GetStringFromObj(objv[5], NULL);
    if (strncmp("bin", bin_or_coord, 3) == 0)
	isbinnum = 1;
    else if (strncmp("coord", bin_or_coord, 5) == 0)
	isbinnum = 0;
    else
    {
	Tcl_AppendResult(interp, "bad argument \"", bin_or_coord, 
			 "\", expected \"bin\" or \"coord\"", NULL);
	return TCL_ERROR;
    }
    if (strncmp("in", in_or_out, 2) == 0)
	isin = 1;
    else if (strncmp("excl", in_or_out, 4) == 0 || 
	     strcmp("out", in_or_out) == 0)
	isin = 0;
    else
    {
	Tcl_AppendResult(interp, "bad argument \"", in_or_out, 
			 "\", expected \"include\" or \"exclude\"", NULL);
	return TCL_ERROR;
    }
    if (hs_type(id) != HS_2D_HISTOGRAM)
    {
	if (hs_type(id) == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a valid Histo-Scope id", NULL);
	else
	    Tcl_AppendResult(interp, "item with id ", 
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a 2d histogram", NULL);
	return TCL_ERROR;
    }
    hs_2d_hist_num_bins(id, &num_x_bins, &num_y_bins);
    size = num_x_bins*num_y_bins;
    if ((data = (float *)malloc(size*sizeof(float))) == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    if ((goodbins = (Goodbins *)malloc(size*sizeof(Goodbins))) == NULL)
    {
	free(data);
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    hs_2d_hist_range(id, &xmin, &xmax, &ymin, &ymax);
    xstep = (xmax - xmin)/(float)num_x_bins;
    ystep = (ymax - ymin)/(float)num_y_bins;
    hs_2d_hist_bin_contents(id, data);
    nfound = 0;
    for (i=0; i<num_x_bins; ++i)
    {
	for (j=0; j<num_y_bins; ++j)
	{
	    datum = data[i*num_y_bins+j];
	    if (fmin <= datum && datum <= fmax)
	    {
		if (isin)
		{   
		    goodbins[nfound].nx = i;
		    goodbins[nfound].ny = j;
		    ++nfound;
		}
	    }
	    else
	    {
		if (!isin)
		{
		    goodbins[nfound].nx = i;
		    goodbins[nfound].ny = j;
		    ++nfound;
		}
	    }
	}
    }
    if (nfound)
    {
	biglist = Tcl_NewListObj(0, NULL);
	for (i=0; i<nfound; ++i)
	{
	    if (isbinnum)
	    {
		xy[0] = Tcl_NewIntObj(goodbins[i].nx);
		xy[1] = Tcl_NewIntObj(goodbins[i].ny);
	    }
	    else
	    {
		xy[0] = Tcl_NewDoubleObj((double)(xmin + xstep*(float)goodbins[i].nx));
		xy[1] = Tcl_NewDoubleObj((double)(ymin + ystep*(float)goodbins[i].ny));
	    }
	    if (Tcl_ListObjAppendElement(
		interp, biglist, Tcl_NewListObj(2, xy)) != TCL_OK)
	    {
		free(goodbins);
		free(data);
		return TCL_ERROR;
	    }
	}
	Tcl_SetObjResult(interp, biglist);
    }
    free(goodbins);
    free(data);
    return TCL_OK;
}

tcl_routine(2d_hist_apply_weights)
{
    int i, j, id, idw, axis;
    float *data = NULL, *poserr = NULL, *negerr = NULL, *weights = NULL;
    int n_x_bins, n_y_bins, nbins, nw, errtype;

    tcl_require_objc(4);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[2], &idw) != TCL_OK)
	return TCL_ERROR;
    if (get_axis_from_obj(interp, objv[3], 2, 1, &axis) != TCL_OK)
	return TCL_ERROR;
    if (hs_type(id) != HS_2D_HISTOGRAM)
    {
	if (hs_type(id) == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a valid Histo-Scope id", NULL);
	else
	    Tcl_AppendResult(interp, "item with id ", 
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a 2d histogram", NULL);
	return TCL_ERROR;
    }
    if (hs_type(idw) != HS_1D_HISTOGRAM)
    {
	if (hs_type(id) == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[2], NULL),
			     " is not a valid Histo-Scope id", NULL);
	else
	    Tcl_AppendResult(interp, "item with id ", 
			     Tcl_GetStringFromObj(objv[2], NULL),
			     " is not a 1d histogram", NULL);
	return TCL_ERROR;
    }
    nw = hs_1d_hist_num_bins(idw);
    hs_2d_hist_num_bins(id, &n_x_bins, &n_y_bins);
    if ((axis == HS_AXIS_X && n_x_bins != nw) || 
	(axis == HS_AXIS_Y && n_y_bins != nw))
    {
        Tcl_AppendResult(interp, "numbers of bins are not compatible for histograms ",
	                 Tcl_GetStringFromObj(objv[1], NULL), " and ", 
	                 Tcl_GetStringFromObj(objv[2], NULL), NULL);
        return TCL_ERROR;
    }
    nbins = n_x_bins*n_y_bins;

    weights = (float *)malloc(nw*sizeof(float));
    data = (float *)malloc(nbins*sizeof(float));
    poserr = (float *)malloc(nbins*sizeof(float));
    negerr = (float *)malloc(nbins*sizeof(float));
    if (!(weights && data && poserr && negerr))
    {
       Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
       goto fail;
    }
    hs_1d_hist_bin_contents(idw, weights);
    hs_2d_hist_bin_contents(id, data);
    errtype = hs_2d_hist_errors(id, poserr, negerr);
    if (errtype == HS_NO_ERRORS)
    {
       free(poserr);
       poserr = NULL;
       free(negerr);
       negerr = NULL;
    }
    else if (errtype == HS_POS_ERRORS)
    {
       free(negerr);
       negerr = NULL;
    }

    if (axis == HS_AXIS_X)
    {
    	for (i=0; i<n_x_bins; i++)
       	   for (j=0; j<n_y_bins; j++)
               data[i*n_y_bins + j] *= weights[i];
	switch (errtype)
        {
            case HS_BOTH_ERRORS:
    	       for (i=0; i<n_x_bins; i++)
       	           for (j=0; j<n_y_bins; j++)
                   {
                       poserr[i*n_y_bins + j] *= weights[i];
                       negerr[i*n_y_bins + j] *= weights[i];
                   }      
               break;
            case HS_POS_ERRORS:
    	       for (i=0; i<n_x_bins; i++)
       	           for (j=0; j<n_y_bins; j++)
                       poserr[i*n_y_bins + j] *= weights[i];           
               break;
            case HS_NO_ERRORS:
               break;
            default:
               Tcl_AppendResult(interp, 
			 "Invalid error retrieval status for 2d histogram with id ", 
			  Tcl_GetStringFromObj(objv[1], NULL), 
			  ". This is a bug. Please report.", NULL);
               goto fail;
        }
    }
    else
    {
    	for (i=0; i<n_x_bins; i++)
       	   for (j=0; j<n_y_bins; j++)
               data[i*n_y_bins + j] *= weights[j];
	switch (errtype)
        {
            case HS_BOTH_ERRORS:
    	       for (i=0; i<n_x_bins; i++)
       	           for (j=0; j<n_y_bins; j++)
                   {
                       poserr[i*n_y_bins + j] *= weights[j];
                       negerr[i*n_y_bins + j] *= weights[j];
                   }      
               break;
            case HS_POS_ERRORS:
    	       for (i=0; i<n_x_bins; i++)
       	           for (j=0; j<n_y_bins; j++)
                       poserr[i*n_y_bins + j] *= weights[j];           
               break;
            case HS_NO_ERRORS:
               break;
            default:
               Tcl_AppendResult(interp, 
			 "Invalid error retrieval status for 2d histogram with id ", 
			  Tcl_GetStringFromObj(objv[1], NULL), 
			  ". This is a bug. Please report.", NULL);
               goto fail;
        }
    }

    hs_2d_hist_block_fill(id, data, poserr, negerr);

    if (weights)
       free(weights);
    if (data)
       free(data);
    if (poserr)
       free(poserr);
    if (negerr)
       free(negerr);
    return TCL_OK;

fail:
    if (weights)
       free(weights);
    if (data)
       free(data);
    if (poserr)
       free(poserr);
    if (negerr)
       free(negerr);
    return TCL_ERROR;
}

tcl_routine(column_shape)
{
    /* Usage: column_shape $ntuple_id $column_number */
    int i, id_source, column, nrows;
    float mean, m2, m3, m4, diff, diffsq, sigma, skew, kurt;
    float *sourcedata;
    Tcl_Obj *result[5];

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
    mean = 0.0;
    for (i=0; i<nrows; ++i)
	mean += sourcedata[i];
    mean /= (float)nrows;
    if (nrows > 1)
    {
	m2 = 0.f;
	m3 = 0.f;
	m4 = 0.f;
	for (i=0; i<nrows; i++)
	{
	    diff = sourcedata[i] - mean;
	    diffsq = diff*diff;
	    m2 += diffsq;
	    m3 += diffsq*diff;
	    m4 += diffsq*diffsq;
	}
	m2 /= (float)nrows;
	m3 /= (float)nrows;
	m4 /= (float)nrows;
	if (m2 <= 0.f)
	{
	    result[0] =  Tcl_NewDoubleObj((double)nrows);
	    result[1] =  Tcl_NewDoubleObj((double)mean);
	    result[2] =  Tcl_NewDoubleObj(0.0);
	    result[3] =  Tcl_NewDoubleObj(0.0);
	    result[4] =  Tcl_NewDoubleObj(0.0);
	}
	else
	{
	    sigma = (float)sqrt((double)m2);
	    skew = m3/sigma/sigma/sigma;
	    kurt = m4/m2/m2;
	    result[0] =  Tcl_NewDoubleObj((double)nrows);
	    result[1] =  Tcl_NewDoubleObj((double)mean);
	    result[2] =  Tcl_NewDoubleObj((double)sigma);
	    result[3] =  Tcl_NewDoubleObj((double)skew);
	    result[4] =  Tcl_NewDoubleObj((double)kurt);
	}
    }
    else
    {
	result[0] =  Tcl_NewDoubleObj((double)nrows);
	result[1] =  Tcl_NewDoubleObj((double)mean);
	result[2] =  Tcl_NewDoubleObj(0.0);
	result[3] =  Tcl_NewDoubleObj(0.0);
	result[4] =  Tcl_NewDoubleObj(0.0);
    }

    Tcl_SetObjResult(interp, Tcl_NewListObj(5, result));
    free(sourcedata);
    return TCL_OK;
}

tcl_routine(1d_hist_shape)
{
    int i, id, size, leftside = 0;
    float xmin, xmax, step, sum, mean, m2, m3, m4, diff, diffsq;
    double sigma, skew, kurt;
    float *data, *xval;
    Tcl_Obj *result[5];
    
    tcl_objc_range(2,3);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
	return TCL_ERROR;
    if (objc == 3)
	if (Tcl_GetBooleanFromObj(interp, objv[2], &leftside) != TCL_OK)
	    return TCL_ERROR;
    if (hs_type(id) != HS_1D_HISTOGRAM)
    {
	if (hs_type(id) == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a valid Histo-Scope id", NULL);
	else
	    Tcl_AppendResult(interp, "item with id ", 
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a 1d histogram", NULL);
	return TCL_ERROR;
    }
    size = hs_1d_hist_num_bins(id);
    data = (float *)malloc(size*sizeof(float));
    xval = (float *)malloc(size*sizeof(float));
    if (data == NULL || xval == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	if (data)
	    free(data);
	if (xval)
	    free(xval);
	return TCL_ERROR;
    }
    hs_1d_hist_bin_contents(id, data);
    hs_1d_hist_range(id, &xmin, &xmax);
    step = (xmax - xmin)/(float)size;
    if (!leftside)
	xmin += step/2.f;
    sum = 0.f;
    mean = 0.f;
    for (i=0; i<size; i++)
    {
	xval[i] = xmin + step*(float)i;
	sum += data[i];
	mean += data[i]*xval[i];
    }
    if (sum <= 0.f)
    {
	result[0] =  Tcl_NewDoubleObj((double)(sum*step));
	result[1] =  Tcl_NewDoubleObj(0.0);
	result[2] =  Tcl_NewDoubleObj(0.0);
	result[3] =  Tcl_NewDoubleObj(0.0);
	result[4] =  Tcl_NewDoubleObj(0.0);
	goto haveresult;
    }
    mean /= sum;
    m2 = 0.f;
    m3 = 0.f;
    m4 = 0.f;
    for (i=0; i<size; i++)
    {
	diff = xval[i]-mean;
	diffsq = diff*diff;
	m2 += data[i]*diffsq;
	m3 += data[i]*diffsq*diff;
	m4 += data[i]*diffsq*diffsq;
    }
    m2 /= sum;
    m3 /= sum;
    m4 /= sum;
    sigma = (double)m2;
    if (sigma <= 0.0)
    {
	result[0] =  Tcl_NewDoubleObj((double)(sum*step));
	result[1] =  Tcl_NewDoubleObj((double)mean);
	result[2] =  Tcl_NewDoubleObj(0.0);
	result[3] =  Tcl_NewDoubleObj(0.0);
	result[4] =  Tcl_NewDoubleObj(0.0);
	goto haveresult;
    }
    sigma = sqrt(sigma);
    skew = (double)m3/sigma/sigma/sigma;
    kurt = (double)(m4/m2/m2);
    
    result[0] =  Tcl_NewDoubleObj((double)(sum*step));
    result[1] =  Tcl_NewDoubleObj((double)mean);
    result[2] =  Tcl_NewDoubleObj(sigma);
    result[3] =  Tcl_NewDoubleObj(skew);
    result[4] =  Tcl_NewDoubleObj(kurt);

 haveresult:
    Tcl_SetObjResult(interp, Tcl_NewListObj(5, result));
    free(data);
    free(xval);
    return TCL_OK;
}

tcl_routine(1d_hist_cdfvalues)
{
    int i, size, id, listlen, binnum, veryclose;
    float *data = NULL;
    float xmin, xmax, sum;
    double *argcoords = NULL;
    double dbin, dlo, dmin, dmax, dstep, thiscoord;
    const double eps = 1.0e-9;
    register float oldsum;
    Tcl_Obj **listObjElem;
    Tcl_Obj *result;

    tcl_require_objc(3);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_ListObjGetElements(interp, objv[2], &listlen, &listObjElem) != TCL_OK)
	return TCL_ERROR;
    if ((argcoords = (double *)malloc(listlen*sizeof(double))) == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    for (i=0; i < listlen; i++)
    {
	if (Tcl_GetDoubleFromObj(interp, listObjElem[i], argcoords+i) != TCL_OK)
	    goto fail;
    }
    if (hs_type(id) != HS_1D_HISTOGRAM)
    {
	if (hs_type(id) == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a valid Histo-Scope id", NULL);
	else
	    Tcl_AppendResult(interp, "item with id ", 
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a 1d histogram", NULL);
	goto fail;
    }
    size = hs_1d_hist_num_bins(id);
    if ((data = (float *)malloc((size+1)*sizeof(float))) == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }
    hs_1d_hist_bin_contents(id, data);
    hs_1d_hist_range(id, &xmin, &xmax);

    dmin = (double)xmin;
    dmax = (double)xmax;
    dstep = (dmax - dmin)/(double)size;
    
    /* Check that every entry is non-negative.
       Build the cumulative distribution */
    sum = 0.f;
    for (i=0; i<size; i++)
    {
	oldsum = sum;
	sum += data[i];
	if (data[i] < 0.f)
	{
	    Tcl_AppendResult(interp, "histogram with id ",
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " has negative bin values", NULL);
	    goto fail;
	}
	data[i] = oldsum;
    }

    result = Tcl_NewListObj(0, NULL);

    if (sum == 0.f)
    {
	/* Too bad, all bins are zero. Set all cdf values to 0. */
	for (i=0; i<listlen; i++)
	    Tcl_ListObjAppendElement(interp, result, Tcl_NewDoubleObj(0.0));
    }
    else
    {
	for (i=0; i<size; i++)
	    data[i] /= sum;
	data[size] = 1.f;
	for (i=0; i<listlen; i++)
	{
	    thiscoord = argcoords[i];
	    if (thiscoord <= dmin)
		Tcl_ListObjAppendElement(interp, result, Tcl_NewDoubleObj(0.0));
	    else if (thiscoord >= dmax)
		Tcl_ListObjAppendElement(interp, result, Tcl_NewDoubleObj(100.0));
	    else
	    {
		dbin = (thiscoord-dmin)/dstep;
		binnum = closestint(dbin, eps, &veryclose);
		if (veryclose)
		{
		    Tcl_ListObjAppendElement(
			interp, result, Tcl_NewDoubleObj(100.0*(double)data[binnum]));
		}
		else
		{   
		    binnum = (int)(dbin);
		    dlo = dmin + dstep*(double)binnum;
		    Tcl_ListObjAppendElement(
			interp, result, Tcl_NewDoubleObj(100.0*(double)(
			    data[binnum] + (data[binnum+1]-data[binnum])*
			    (float)((thiscoord-dlo)/dstep))));
		}
	    }
	}
    }

    Tcl_SetObjResult(interp, result);

    free(argcoords);
    free(data);
    return TCL_OK;

 fail:
    if (data)
	free(data);
    if (argcoords)
	free(argcoords);
    return TCL_ERROR;
}

tcl_routine(2d_hist_cdfvalues)
{
    /* Usage:  hs::2d_hist_cdfvalues $newcategory $id $axis $coords */
    char *caxis, *category, *label, *tmplabel;
    int i, j, uid, id, axis, listlen, nbins, veryclose, binnum;
    int num_x_bins, num_y_bins, nchannels, ncounts;
    int *histo_ids = NULL;
    float *hidata = NULL, *transdata = NULL;
    float *data;
    float **histo_data = NULL;
    float xmin, xmax, tmpmin, tmpmax, sum, oldsum, ftmp;
    const double eps = 1.0e-9;
    double *argcoords = NULL;
    double dbin, dlo, dmin, dmax, dstep;
    Tcl_Obj **listObjElem;
    Tcl_Obj *result;
    char stringbuf[768], title[256], parenttitle[256];
    int status = TCL_ERROR;

    tcl_require_objc(5);

    category = Tcl_GetStringFromObj(objv[1], NULL);

    if (Tcl_GetIntFromObj(interp, objv[2], &id) != TCL_OK)
	return TCL_ERROR;
    if (hs_type(id) != HS_2D_HISTOGRAM)
    {
	if (hs_type(id) == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[2], NULL),
			     " is not a valid Histo-Scope id", NULL);
	else
	    Tcl_AppendResult(interp, "item with id ", 
			     Tcl_GetStringFromObj(objv[2], NULL),
			     " is not a 2d histogram", NULL);
	return TCL_ERROR;
    }

    hs_2d_hist_labels(id, stringbuf, stringbuf+256, stringbuf+512);
    hs_2d_hist_num_bins(id, &num_x_bins, &num_y_bins);
    nbins = num_x_bins*num_y_bins;
    caxis = Tcl_GetStringFromObj(objv[3], NULL);
    if (strcasecmp(caxis, "x") == 0)
    {
	axis = HS_AXIS_X;
	label = stringbuf;
	tmplabel = stringbuf+256;
	nchannels = num_y_bins;
	ncounts = num_x_bins;
	hs_2d_hist_range(id, &xmin, &xmax, &tmpmin, &tmpmax);
    }
    else if (strcasecmp(caxis, "y") == 0)
    {
	axis = HS_AXIS_Y;
	label = stringbuf+256;
	tmplabel = stringbuf;
	nchannels = num_x_bins;
	ncounts = num_y_bins;
	hs_2d_hist_range(id, &tmpmin, &tmpmax, &xmin, &xmax);
    }
    else
    {
	Tcl_AppendResult(interp, "Invalid axis specification \"", 
			 caxis, "\"", NULL);
	return TCL_ERROR;
    }
    if (label[0] == '\0')
	label = caxis;
    dmin = (double)xmin;
    dmax = (double)xmax;
    dstep = (dmax - dmin)/(double)ncounts;

    if (Tcl_ListObjGetElements(interp, objv[4], &listlen, &listObjElem) != TCL_OK)
	return TCL_ERROR;
    if (listlen == 0)
	return TCL_OK;
    if ((argcoords = (double *)malloc(listlen*sizeof(double))) == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }
    for (i=0; i<listlen; i++)
    {
	if (Tcl_GetDoubleFromObj(interp, listObjElem[i], argcoords+i) != TCL_OK)
	    goto fail;
    }
    /* Finished parsing input arguments */

    if ((hidata = (float *)malloc((nbins+1)*sizeof(float))) == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }
    hs_2d_hist_bin_contents(id, hidata);
    if (axis == HS_AXIS_X)
    {
	/* Make a transpose */	
	if ((transdata = (float *)malloc((nbins+1)*sizeof(float))) == NULL)
	{
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    goto fail;
	}
	for (i=0; i<num_x_bins; i++)
	    for (j=0; j<num_y_bins; j++)
		transdata[j*num_x_bins+i] = hidata[i*num_y_bins+j];
	free(hidata);
	hidata = NULL;
	data = transdata;
    }
    else
    {
	data = hidata;
    }

    if ((histo_ids = (int *)malloc(listlen*sizeof(int))) == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }
    if ((histo_data = (float **)calloc(listlen, sizeof(float *))) == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }
    uid = hs_next_category_uid(category);
    if (uid < 0)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }
    hs_title(id, parenttitle);
    for (i=0; i<listlen; i++)
    {
	sprintf(title, "Cdf value at %s = %s for \"%s\"", label,
		Tcl_GetStringFromObj(listObjElem[i], NULL), parenttitle);
	histo_ids[i] = hs_create_1d_hist(uid++, title, category, tmplabel,
					     "Cdf (%)", nchannels, tmpmin, tmpmax);
	if (histo_ids[i] < 0)
	{
	    Tcl_SetResult(interp, "failed to create a 1d histogram", TCL_STATIC);
	    goto fail;
	}
	if ((histo_data[i] = (float *)malloc(nchannels*sizeof(float))) == NULL)
	{
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    goto fail;
	}
    }

    ftmp = data[0];
    for (j=0; j<nchannels; ++j, data+=ncounts)
    {
	data[0] = ftmp;
	ftmp = data[ncounts];
	sum = 0.f;
	for (i=0; i<ncounts; i++)
	{
	    oldsum = sum;
	    sum += data[i];
	    if (data[i] < 0.f)
	    {
		Tcl_AppendResult(interp, "histogram with id ",
				 Tcl_GetStringFromObj(objv[2], NULL),
				 " has negative bin values", NULL);
		goto fail;
	    }
	    data[i] = oldsum;
	}
	
	if (sum == 0.f)
	{
	    /* Too bad, all bins are zero. Set all cdf values to 0. */
	    for (i=0; i<listlen; i++)
		histo_data[i][j] = 0.f;
	}
	else
	{
	    for (i=0; i<ncounts; i++)
		data[i] /= sum;
	    data[ncounts] = 1.f;
	    for (i=0; i<listlen; i++)
	    {
		if (argcoords[i] <= dmin)
		    histo_data[i][j] = 0.f;
		else if (argcoords[i] >= dmax)
		    histo_data[i][j] = 100.f;
		else
		{
		    dbin = (argcoords[i]-dmin)/dstep;
		    binnum = closestint(dbin, eps, &veryclose);
		    if (veryclose)
		    {
			histo_data[i][j] = 100.f * data[binnum];
		    }
		    else
		    {   
			binnum = (int)((argcoords[i]-dmin)/dstep);
			dlo = dmin + dstep*(double)binnum;
			histo_data[i][j] = 100.f * (
			    data[binnum] + (data[binnum+1]-data[binnum])*
			    (float)((argcoords[i]-dlo)/dstep));
		    }
		}
	    }
	}
    }

    result = Tcl_NewListObj(0, NULL);
    for (i=0; i<listlen; i++)
    {
	hs_1d_hist_block_fill(histo_ids[i], histo_data[i], NULL, NULL);
	Tcl_ListObjAppendElement(interp, result, Tcl_NewIntObj(histo_ids[i]));
    }
    Tcl_SetObjResult(interp, result);

    status = TCL_OK;
 fail:
    if (argcoords)
	free(argcoords);
    if (hidata)
	free(hidata);
    if (transdata)
	free(transdata);
    if (histo_ids)
	free(histo_ids);
    if (histo_data)
    {
	for (i=0; i<listlen; ++i)
	    if (histo_data[i])
		free(histo_data[i]);
	free(histo_data);
    }
    return status;
}

tcl_routine(2d_hist_percentiles)
{
    /* Usage:  hs::2d_hist_percentiles $newcategory $id $axis $percentages */
    char *caxis, *category, *label, *tmplabel;
    int i, j, uid, id, axis, listlen, nbins;
    int num_x_bins, num_y_bins, nchannels, ncounts;
    int *histo_ids = NULL;
    float *quantiles = NULL, *hidata = NULL, *transdata = NULL, *coords = NULL;
    float *data;
    float **histo_data = NULL;
    float xmin, xmax, tmpmin, tmpmax, step, sum, oldsum, ftmp;
    double dtmp;
    Tcl_Obj **listObjElem;
    Tcl_Obj *result;
    char stringbuf[768], title[256], parenttitle[256];
    int status = TCL_ERROR;

    tcl_require_objc(5);

    category = Tcl_GetStringFromObj(objv[1], NULL);

    if (Tcl_GetIntFromObj(interp, objv[2], &id) != TCL_OK)
	return TCL_ERROR;
    if (hs_type(id) != HS_2D_HISTOGRAM)
    {
	if (hs_type(id) == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[2], NULL),
			     " is not a valid Histo-Scope id", NULL);
	else
	    Tcl_AppendResult(interp, "item with id ", 
			     Tcl_GetStringFromObj(objv[2], NULL),
			     " is not a 2d histogram", NULL);
	return TCL_ERROR;
    }

    hs_2d_hist_labels(id, stringbuf, stringbuf+256, stringbuf+512);
    hs_2d_hist_num_bins(id, &num_x_bins, &num_y_bins);
    nbins = num_x_bins*num_y_bins;
    caxis = Tcl_GetStringFromObj(objv[3], NULL);
    if (strcasecmp(caxis, "x") == 0)
    {
	axis = HS_AXIS_X;
	label = stringbuf;
	tmplabel = stringbuf+256;
	nchannels = num_y_bins;
	ncounts = num_x_bins;
	hs_2d_hist_range(id, &xmin, &xmax, &tmpmin, &tmpmax);
    }
    else if (strcasecmp(caxis, "y") == 0)
    {
	axis = HS_AXIS_Y;
	label = stringbuf+256;
	tmplabel = stringbuf;
	nchannels = num_x_bins;
	ncounts = num_y_bins;
	hs_2d_hist_range(id, &tmpmin, &tmpmax, &xmin, &xmax);
    }
    else
    {
	Tcl_AppendResult(interp, "Invalid axis specification \"", 
			 caxis, "\"", NULL);
	return TCL_ERROR;
    }
    if (label[0] == '\0')
	label = caxis;
    step = (xmax - xmin)/(float)ncounts;

    if (Tcl_ListObjGetElements(interp, objv[4], &listlen, &listObjElem) != TCL_OK)
	return TCL_ERROR;
    if (listlen == 0)
	return TCL_OK;
    if ((quantiles = (float *)malloc(listlen*sizeof(float))) == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }
    for (i=0; i<listlen; i++)
    {
	if (Tcl_GetDoubleFromObj(interp, listObjElem[i], &dtmp) != TCL_OK)
	    goto fail;
	else
	    quantiles[i] = (float)(dtmp/100.0);
    }
    /* Finished parsing input arguments */

    if ((hidata = (float *)malloc((nbins+1)*sizeof(float))) == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }
    hs_2d_hist_bin_contents(id, hidata);
    if (axis == HS_AXIS_X)
    {
	/* Make a transpose */	
	if ((transdata = (float *)malloc((nbins+1)*sizeof(float))) == NULL)
	{
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    goto fail;
	}
	for (i=0; i<num_x_bins; i++)
	    for (j=0; j<num_y_bins; j++)
		transdata[j*num_x_bins+i] = hidata[i*num_y_bins+j];
	free(hidata);
	hidata = NULL;
	data = transdata;
    }
    else
    {
	data = hidata;
    }

    if ((histo_ids = (int *)malloc(listlen*sizeof(int))) == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }
    if ((histo_data = (float **)calloc(listlen, sizeof(float *))) == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }
    uid = hs_next_category_uid(category);
    if (uid < 0)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }
    hs_title(id, parenttitle);
    for (i=0; i<listlen; i++)
    {
	sprintf(title, "%s percentile %s of \"%s\"", label,
		Tcl_GetStringFromObj(listObjElem[i], NULL), parenttitle);
	histo_ids[i] = hs_create_1d_hist(uid++, title, category, tmplabel, 
					     label, nchannels, tmpmin, tmpmax);
	if (histo_ids[i] < 0)
	{
	    Tcl_SetResult(interp, "failed to create a 1d histogram", TCL_STATIC);
	    goto fail;
	}
	if ((histo_data[i] = (float *)malloc(nchannels*sizeof(float))) == NULL)
	{
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    goto fail;
	}
    }

    if ((coords = (float *)malloc((ncounts+1)*sizeof(float))) == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }
    for (i=0; i<ncounts; i++)
	coords[i] = xmin + step*(float)i;
    coords[ncounts] = xmax;
    
    ftmp = data[0];
    for (j=0; j<nchannels; ++j, data+=ncounts)
    {
	data[0] = ftmp;
	ftmp = data[ncounts];
	sum = 0.f;
	for (i=0; i<ncounts; i++)
	{
	    oldsum = sum;
	    sum += data[i];
	    if (data[i] < 0.f)
	    {
		Tcl_AppendResult(interp, "histogram with id ",
				 Tcl_GetStringFromObj(objv[2], NULL),
				 " has negative bin values", NULL);
		goto fail;
	    }
	    data[i] = oldsum;
	}
	
	if (sum == 0.f)
	{
	    /* Too bad, all bins are zero. Set all quantiles to the
	       value of the histogram left edge. */
	    for (i=0; i<listlen; i++)
		histo_data[i][j] = xmin;
	}
	else
	{
	    for (i=0; i<ncounts; i++)
		data[i] /= sum;
	    data[ncounts] = 1.f;
	    for (i=0; i<listlen; i++)
		histo_data[i][j] = find_percentile(coords, data, ncounts+1, quantiles[i]);
	}
    }

    result = Tcl_NewListObj(0, NULL);
    for (i=0; i<listlen; i++)
    {
	hs_1d_hist_block_fill(histo_ids[i], histo_data[i], NULL, NULL);
	Tcl_ListObjAppendElement(interp, result, Tcl_NewIntObj(histo_ids[i]));
    }
    Tcl_SetObjResult(interp, result);

    status = TCL_OK;
 fail:
    if (quantiles)
	free(quantiles);
    if (hidata)
	free(hidata);
    if (coords)
	free(coords);
    if (transdata)
	free(transdata);
    if (histo_ids)
	free(histo_ids);
    if (histo_data)
    {
	for (i=0; i<listlen; ++i)
	    if (histo_data[i])
		free(histo_data[i]);
	free(histo_data);
    }
    return status;
}

tcl_routine(hist_scale_data)
{
    int i, j, k, id, nbins, nxbins, nybins, nzbins, itype;
    double dscale;
    float fscale, *data, underflow, overflow;
    float over2d[3][3], over3d[3][3][3];

    tcl_require_objc(3);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[2], &dscale) != TCL_OK)
	return TCL_ERROR;
    itype = hs_type(id);
    switch (itype)
    {
    case HS_1D_HISTOGRAM:
	nbins = hs_1d_hist_num_bins(id);
	break;

    case HS_2D_HISTOGRAM:
	hs_2d_hist_num_bins(id, &nxbins, &nybins);
	nbins = nxbins*nybins;
	break;

    case HS_3D_HISTOGRAM:
	hs_3d_hist_num_bins(id, &nxbins, &nybins, &nzbins);
	nbins = nxbins*nybins*nzbins;
	break;

    case HS_NONE:
	Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			 " is not a valid Histo-Scope id", NULL);
	return TCL_ERROR;

    default:
	Tcl_AppendResult(interp, "item with id ", 
			 Tcl_GetStringFromObj(objv[1], NULL),
			 " is not a histogram", NULL);
	return TCL_ERROR;
    }
    fscale = (float)dscale;
    if (fscale == 0.f)
    {
	Tcl_SetResult(interp, "scale factor must not be zero", TCL_STATIC);
	return TCL_ERROR;
    }

    data = (float *)malloc(nbins * sizeof(float));
    if (data == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    if (itype == HS_1D_HISTOGRAM) {
	hs_1d_hist_bin_contents(id, data);
	hs_1d_hist_overflows(id, &underflow, &overflow);
	underflow *= fscale;
	overflow *= fscale;
    } else if (itype == HS_2D_HISTOGRAM) {
	hs_2d_hist_bin_contents(id, data);
	hs_2d_hist_overflows(id, over2d);
	for (i=0; i<3; ++i)
	    for (j=0; j<3; ++j)
		over2d[i][j] *= fscale;
    } else if (itype == HS_3D_HISTOGRAM) {
	hs_3d_hist_bin_contents(id, data);
	hs_3d_hist_overflows(id, over3d);
	for (i=0; i<3; ++i)
	    for (j=0; j<3; ++j)
		for (k=0; k<3; ++k)
		    over3d[i][j][k] *= fscale;
    } else {
	assert(0);
    }

    for (i=0; i<nbins; ++i)
	data[i] *= fscale;

    if (itype == HS_1D_HISTOGRAM) {
	hs_1d_hist_block_fill(id, data, NULL, NULL);
	hs_1d_hist_set_overflows(id, underflow, overflow);
    } else if (itype == HS_2D_HISTOGRAM) {
	hs_2d_hist_block_fill(id, data, NULL, NULL);
	hs_2d_hist_set_overflows(id, over2d);
    } else if (itype == HS_3D_HISTOGRAM) {
	hs_3d_hist_block_fill(id, data, NULL, NULL);
	hs_3d_hist_set_overflows(id, over3d);
    } else {
	assert(0);
    }

    free(data);
    return TCL_OK;
}

tcl_routine(hist_scale_errors)
{
    int i, id, nbins, estat, itype;
    double dscale;
    float fscale;
    float *poserr = NULL, *negerr = NULL;
    
    tcl_require_objc(3);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[2], &dscale) != TCL_OK)
	return TCL_ERROR;
    fscale = (float)dscale;
    if (fscale <= 0.f)
    {
	Tcl_SetResult(interp, "scale factor must be positive", TCL_STATIC);
	return TCL_ERROR;
    }
    estat = hs_hist_error_status(id);
    if (estat == HS_NO_ERRORS)
    {
	Tcl_AppendResult(interp, "histogram with id ", 
			 Tcl_GetStringFromObj(objv[1], NULL),
			 " does not have errors", NULL);
	return TCL_ERROR;
    }
    else if (estat == HS_ITEMNOTFOUND_ERRORS)
    {
	if (hs_type(id) == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a valid Histo-Scope id", NULL);
	else
	    Tcl_AppendResult(interp, "item with id ", 
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a histogram", NULL);
	return TCL_ERROR;
    }
    nbins = hs_hist_num_bins(id);
    poserr = (float *)malloc(nbins*sizeof(float));
    if (poserr == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    if (estat == HS_BOTH_ERRORS)
    {
	negerr = (float *)malloc(nbins*sizeof(float));
	if (negerr == NULL)
	{
	    free(poserr);
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    return TCL_ERROR;
	}
    }
    itype = hs_type(id);
    if (itype == HS_1D_HISTOGRAM)
	hs_1d_hist_errors(id, poserr, negerr);
    else if (itype == HS_2D_HISTOGRAM)
	hs_2d_hist_errors(id, poserr, negerr);
    else if (itype == HS_3D_HISTOGRAM)
	hs_3d_hist_errors(id, poserr, negerr);
    else
	assert(0);
    for (i=0; i<nbins; ++i)
    {
	poserr[i] *= fscale;
	if (negerr)
	    negerr[i] *= fscale;
    }
    if (itype == HS_1D_HISTOGRAM)
	hs_set_1d_errors(id, poserr, negerr);
    else if (itype == HS_2D_HISTOGRAM)
	hs_set_2d_errors(id, poserr, negerr);
    else if (itype == HS_3D_HISTOGRAM)
	hs_set_3d_errors(id, poserr, negerr);
    else
	assert(0);
    free(poserr);
    if (negerr) free(negerr);
    return TCL_OK;
}

tcl_routine(hist_error_status)
{
    /* Usage: hs::hist_error_status id
     * Returns an integer:
     *   -1    not a histogram (or no such item)
     *    0    no errors defined
     *    1    only positive errors defined
     *    2    both positive and negative errors defined
     */
    int id;

    tcl_require_objc(2);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
	return TCL_ERROR;
    switch (hs_hist_error_status(id))
    {
    case HS_NO_ERRORS:
	Tcl_SetObjResult(interp, Tcl_NewIntObj(0));
	break;
    case HS_POS_ERRORS:
	Tcl_SetObjResult(interp, Tcl_NewIntObj(1));
	break;
    case HS_BOTH_ERRORS:
	Tcl_SetObjResult(interp, Tcl_NewIntObj(2));
	break;
    case HS_ITEMNOTFOUND_ERRORS:
	Tcl_SetObjResult(interp, Tcl_NewIntObj(-1));
	break;
    default:
	Tcl_SetResult(interp,
	    "hist_error_status: internal error. This is a bug. Please report.",
	    TCL_STATIC);
	return TCL_ERROR;
    }
    return TCL_OK;
}

tcl_routine(Prepare_1d_scatter_plot)
{
    /* Usage: hs::Prepare_1d_scatter_plot ntuple_id varname uid title categ
     *
     * This command makes an ntuple with 2 variables, with all values for
     * the second column filled with 0s so that the user can later make
     * an xy plot which looks like a 1d scatter plot.
     */
    int i, ntuple_id, uid, nrows, ivar, new_id = -1;
    char *varname, *title, *categ;
    float *data = NULL;
    char *second_column = "00";
    char *new_names[2];
    float newdata[2];
    
    tcl_require_objc(6);
    if (Tcl_GetIntFromObj(interp, objv[1], &ntuple_id) != TCL_OK)
	return TCL_ERROR;
    varname = Tcl_GetStringFromObj(objv[2], NULL);
    if (Tcl_GetIntFromObj(interp, objv[3], &uid) != TCL_OK)
	return TCL_ERROR;
    title = Tcl_GetStringFromObj(objv[4], NULL);
    categ = Tcl_GetStringFromObj(objv[5], NULL);
    if (hs_type(ntuple_id) != HS_NTUPLE)
    {
	if (hs_type(ntuple_id) == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a valid Histo-Scope id", NULL);
	else
	    Tcl_AppendResult(interp, "item with id ", 
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " is not an ntuple", NULL);
	return TCL_ERROR;
    }
    ivar = hs_variable_index(ntuple_id, varname);
    if (ivar < 0)
    {
	Tcl_AppendResult(interp, "\"", varname, "\" is not a variable",
			 " of the ntuple with id ",
			 Tcl_GetStringFromObj(objv[1], NULL), NULL);
	return TCL_ERROR;
    }
    if (strcmp(varname, second_column) == 0)
    {
	Tcl_AppendResult(interp, "can not use the column named \"",
			 varname, "\" of the ntuple with id ",
			 Tcl_GetStringFromObj(objv[1], NULL),
			 ": this name is reserved for the special column", NULL);
	return TCL_ERROR;
    }
    new_names[0] = varname;
    new_names[1] = second_column;
    new_id = hs_create_ntuple(uid, title, categ, 2, new_names);
    if (new_id <= 0)
    {
	Tcl_SetResult(interp, "failed to create the new ntuple",
		      TCL_STATIC);
	return TCL_ERROR;
    }
    nrows = hs_num_entries(ntuple_id);
    if (nrows > 0)
    {
	data = (float *)malloc(nrows * sizeof(float));
	if (data == NULL)
	{
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    goto fail0;
	}
	hs_column_contents(ntuple_id, ivar, data);
	qsort(data, nrows, sizeof(float),
	      (int (*)(const void *, const void *))fcompare);
	newdata[1] = 0.f;
	for (i=0; i<nrows; ++i)
	{
	    newdata[0] = data[i];
	    if (hs_fill_ntuple(new_id, newdata) != new_id)
	    {
		Tcl_SetResult(interp, "failed to fill the new ntuple",
			      TCL_STATIC);
		goto fail0;
	    }
	}
	free(data);
	data = NULL;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(new_id));
    return TCL_OK;

 fail0:
    if (data) free(data);
    if (new_id > 0) hs_delete(new_id);
    return TCL_ERROR;
}

tcl_routine(hist_set_error_range)
{
    /* Usage: hs::hist_set_error_range id min max */
    char *upper_limit;
    int bin, id, itype, nbins, errstat;
    double dmin, dmax = -1.0;
    float *poserr;
    float xmin, xmax;

    tcl_require_objc(4);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[2], &dmin) != TCL_OK)
	return TCL_ERROR;
    upper_limit = Tcl_GetStringFromObj(objv[3], NULL);
    if (*upper_limit)
	if (Tcl_GetDoubleFromObj(interp, objv[3], &dmax) != TCL_OK)
	    return TCL_ERROR;
    itype = hs_type(id);
    if (itype != HS_1D_HISTOGRAM && 
	itype != HS_2D_HISTOGRAM && 
	itype != HS_3D_HISTOGRAM)
    {
	if (itype == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a valid Histo-Scope id", NULL);
	else
	    Tcl_AppendResult(interp, "item with id ", 
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a histogram", NULL);
	return TCL_ERROR;
    }
    if (dmin < 0.0 || (*upper_limit && dmax < dmin))
    {
	Tcl_SetResult(interp, "invalid error range", TCL_STATIC);
	return TCL_ERROR;
    }
    xmin = (float)dmin;
    xmax = (float)dmax;
    nbins = hs_hist_num_bins(id);
    assert(nbins > 0);
    poserr = (float *)malloc(nbins * sizeof(float));
    if (poserr == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    if (itype == HS_1D_HISTOGRAM)
	errstat = hs_1d_hist_errors(id, poserr, NULL);
    else if (itype == HS_2D_HISTOGRAM)
	errstat = hs_2d_hist_errors(id, poserr, NULL);
    else if (itype == HS_3D_HISTOGRAM)
	errstat = hs_3d_hist_errors(id, poserr, NULL);
    else
	assert(0);
    if (errstat == HS_NO_ERRORS)
    {
	for (bin=0; bin<nbins; ++bin)
	    poserr[bin] = xmin;
    }
    else
    {
	for (bin=0; bin<nbins; ++bin)
	{
	    if (poserr[bin] < xmin)
		poserr[bin] = xmin;
	    else if (*upper_limit && poserr[bin] > xmax)
		poserr[bin] = xmax;
	}
    }
    if (itype == HS_1D_HISTOGRAM)
	hs_set_1d_errors(id, poserr, NULL);
    else if (itype == HS_2D_HISTOGRAM)
	hs_set_2d_errors(id, poserr, NULL);
    else if (itype == HS_3D_HISTOGRAM)
	hs_set_3d_errors(id, poserr, NULL);
    else
	assert(0);
    free(poserr);
    return TCL_OK;
}

tcl_routine(1d_hist_percentiles)
{
    int i, size, id, listlen;
    double dtmp;
    float *quantiles = NULL, *data = NULL, *coords = NULL;
    float xmin, xmax, step, sum;
    register float oldsum;
    Tcl_Obj **listObjElem;
    Tcl_Obj *result;

    tcl_require_objc(3);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_ListObjGetElements(interp, objv[2], &listlen, &listObjElem) != TCL_OK)
	return TCL_ERROR;
    if ((quantiles = (float *)malloc(listlen*sizeof(float))) == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    for (i=0; i < listlen; i++)
    {
	if (Tcl_GetDoubleFromObj(interp, listObjElem[i], &dtmp) != TCL_OK)
	    goto fail;
	else
	    quantiles[i] = (float)(dtmp/100.0);
    }
    if (hs_type(id) != HS_1D_HISTOGRAM)
    {
	if (hs_type(id) == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a valid Histo-Scope id", NULL);
	else
	    Tcl_AppendResult(interp, "item with id ", 
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a 1d histogram", NULL);
	goto fail;
    }
    size = hs_1d_hist_num_bins(id);
    data = (float *)malloc((size+1)*sizeof(float));
    coords = (float *)malloc((size+1)*sizeof(float));
    if (data == NULL || coords == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }
    hs_1d_hist_bin_contents(id, data);
    hs_1d_hist_range(id, &xmin, &xmax);
    step = (xmax - xmin)/(float)size;
    for (i=0; i<size; i++)
	coords[i] = xmin + step*(float)i;
    coords[size] = xmax;

    /* Check that every entry is non-negative.
       Build the cumulative distribution */
    sum = 0.f;
    for (i=0; i<size; i++)
    {
	oldsum = sum;
	sum += data[i];
	if (data[i] < 0.f)
	{
	    Tcl_AppendResult(interp, "histogram with id ",
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " has negative bin values", NULL);
	    goto fail;
	}
	data[i] = oldsum;
    }

    result = Tcl_NewListObj(0, NULL);

    if (sum == 0.f)
    {
	/* Too bad, all bins are zero. Set all quantiles to the
           value of the histogram left edge. */
	for (i=0; i<listlen; i++)
	    Tcl_ListObjAppendElement(
		interp, result, Tcl_NewDoubleObj((double)xmin));
    }
    else
    {
	for (i=0; i<size; i++)
	    data[i] /= sum;
	data[size] = 1.f;
	for (i=0; i<listlen; i++)
	    Tcl_ListObjAppendElement(
		interp, result, Tcl_NewDoubleObj(
		    (double)find_percentile(coords, data, size+1, quantiles[i])));
    }

    Tcl_SetObjResult(interp, result);

    free(quantiles);
    free(data);
    free(coords);
    return TCL_OK;

 fail:
    if (quantiles)
	free(quantiles);
    if (data)
	free(data);
    if (coords)
	free(coords);
    return TCL_ERROR;
}

tcl_routine(column_cdfvalues)
{
    /* Usage: column_percentiles ntuple_id column percentage_list
     *
     * The code does not handle the case when several points have
     * the same value
     */
    int i, ntuple_id, column, n_entries, nvars, listlen;
    Tcl_Obj **listObjElem;
    double d;
    float *percentages = NULL, *percentiles = NULL, *column_data;
    Tcl_Obj *result;

    tcl_require_objc(4);
    verify_ntuple(ntuple_id,1);
    n_entries = hs_num_entries(ntuple_id);
    if (n_entries == 0)
    {
        Tcl_AppendResult(interp, "ntuple with id ",
                         Tcl_GetStringFromObj(objv[1], NULL),
                         " is empty", NULL);
        return TCL_ERROR;
    }
    nvars = hs_num_variables(ntuple_id);
    if (Tcl_GetIntFromObj(interp, objv[2], &column) != TCL_OK)
        return TCL_ERROR;
    if (column < 0 || column >= nvars)
    {
        Tcl_SetResult(interp, "column number is out of range", TCL_STATIC);
        return TCL_ERROR;
    }
    if (Tcl_ListObjGetElements(interp, objv[3], &listlen, &listObjElem) != TCL_OK)
        return TCL_ERROR;
    if (listlen > 0)
    {
        percentages = (float *)malloc((2*listlen + n_entries)*sizeof(float));
        if (percentages == NULL)
        {
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    return TCL_ERROR;
        }
        percentiles = percentages + listlen;
        column_data = percentiles + listlen;
        for (i=0; i<listlen; ++i)
        {
            if (Tcl_GetDoubleFromObj(interp, listObjElem[i], &d) != TCL_OK)
                goto fail;
            percentiles[i] = (float)d;
        }
        hs_column_contents(ntuple_id, column, column_data);
        qsort(column_data, n_entries, sizeof(float),
              (int (*)(const void *, const void *))fcompare);
        d = (double)(n_entries-1);
        for (i=0; i<listlen; ++i)
        {
            if (percentiles[i] > column_data[n_entries-1])
                percentages[i] = 100.f;
            else if (percentiles[i] < column_data[0])
                percentages[i] = 0.f;
            else if (n_entries == 1)
                percentages[i] = 50.f;
            else
            {
                int j;
                j = hs_find_value_index_in_sorted_array(
                    column_data, n_entries, percentiles[i]);
                if (j == n_entries-1)
                    percentages[i] = 100.f;
                else
                {
                    float diff, cdf1, cdf2;
                    cdf1 = (float)(100.0*j/d);
                    cdf2 = (float)(100.0*(j+1)/d);
                    diff = column_data[j+1] - column_data[j];
                    if (diff == 0.f)
                        percentages[i] = (cdf1 + cdf2)/2.f;
                    else
                        percentages[i] = cdf1 + (cdf2 - cdf1)*
                            ((percentiles[i]-column_data[j])/diff);
                }
            }
        }
    }
    result = Tcl_NewListObj(0, NULL);
    for (i=0; i<listlen; ++i)
        Tcl_ListObjAppendElement(interp, result,
               Tcl_NewDoubleObj((double)percentages[i]));
    if (percentages)
        free(percentages);
    Tcl_SetObjResult(interp, result);
    return TCL_OK;

 fail:
    if (percentages)
        free(percentages);
    return TCL_ERROR;
}

tcl_routine(column_percentiles)
{
    /* Usage: column_percentiles ntuple_id column percentage_list */
    int i, ntuple_id, column, n_entries, nvars, listlen, status;
    Tcl_Obj **listObjElem;
    double d;
    float *percentages = NULL, *percentiles = NULL, *column_data;
    Tcl_Obj *result;

    tcl_require_objc(4);
    verify_ntuple(ntuple_id,1);
    n_entries = hs_num_entries(ntuple_id);
    if (n_entries == 0)
    {
        Tcl_AppendResult(interp, "ntuple with id ",
                         Tcl_GetStringFromObj(objv[1], NULL),
                         " is empty", NULL);
        return TCL_ERROR;
    }
    nvars = hs_num_variables(ntuple_id);
    if (Tcl_GetIntFromObj(interp, objv[2], &column) != TCL_OK)
        return TCL_ERROR;
    if (column < 0 || column >= nvars)
    {
        Tcl_SetResult(interp, "column number is out of range", TCL_STATIC);
        return TCL_ERROR;
    }
    if (Tcl_ListObjGetElements(interp, objv[3], &listlen, &listObjElem) != TCL_OK)
        return TCL_ERROR;
    if (listlen > 0)
    {
        percentages = (float *)malloc((2*listlen + n_entries)*sizeof(float));
        if (percentages == NULL)
        {
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    return TCL_ERROR;
        }
        percentiles = percentages + listlen;
        column_data = percentiles + listlen;
        for (i=0; i<listlen; ++i)
        {
            if (Tcl_GetDoubleFromObj(interp, listObjElem[i], &d) != TCL_OK)
                goto fail;
            percentages[i] = (float)d;
        }
        hs_column_contents(ntuple_id, column, column_data);
        status = arr_percentiles(column_data, n_entries, 1,
                                 percentages, listlen, percentiles);
        assert(status == 0);
    }
    result = Tcl_NewListObj(0, NULL);
    for (i=0; i<listlen; ++i)
        Tcl_ListObjAppendElement(interp, result, 
               Tcl_NewDoubleObj((double)percentiles[i]));
    if (percentages)
        free(percentages);
    Tcl_SetObjResult(interp, result);
    return TCL_OK;

 fail:
    if (percentages)
        free(percentages);
    return TCL_ERROR;
}

tcl_routine(1d_hist_bin_coords)
{
  int i, id, size, leftside = 0;
  float *data;
  float xmin, xmax, step;
  
  tcl_objc_range(2,3);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (objc == 3)
      if (Tcl_GetBooleanFromObj(interp, objv[2], &leftside) != TCL_OK)
	  return TCL_ERROR;
  if (hs_type(id) != HS_1D_HISTOGRAM)
  {
    if (hs_type(id) == HS_NONE)
      Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
		       " is not a valid Histo-Scope id", NULL);
    else
      Tcl_AppendResult(interp, "item with id ", 
		       Tcl_GetStringFromObj(objv[1], NULL),
		       " is not a 1d histogram", NULL);
    return TCL_ERROR;
  }
  size = hs_1d_hist_num_bins(id);
  if ((data = (float *)malloc(size*sizeof(float))) == NULL)
  {
    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
    return TCL_ERROR;
  }
  hs_1d_hist_range(id, &xmin, &xmax);
  step = (xmax - xmin)/(float)size;
  if (!leftside)
      xmin += step/2.f;
  for (i=0; i<size; i++)
      data[i] = xmin + step*(float)i;
  Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((unsigned char *)data,
					       size*sizeof(float)));
  free(data);
  return TCL_OK;
}

tcl_routine(1d_hist_errors)
{
    char *cerrtype;
    int id, size, status;
    float *errpos, *errneg, **answer;

    tcl_objc_range(2,3);
    verify_1d_histo(id, 1);
    if (objc == 3)
    {
	cerrtype = Tcl_GetStringFromObj(objv[2], NULL);
	if (cerrtype[0] == 'p' || cerrtype[0] == 'P')
	    answer = &errpos;
	else if (cerrtype[0] == 'n' || cerrtype[0] == 'N')
	    answer = &errneg;
	else
	{
	    Tcl_AppendResult(interp, "invalid error type \"",
			     cerrtype, "\"", NULL);
	    return TCL_ERROR;
	}
    }
    else
	answer = &errpos;
    size = hs_1d_hist_num_bins(id);
    if ((errpos = (float *)malloc(2*size*sizeof(float))) == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    errneg = errpos + size;
    status = hs_1d_hist_errors(id, errpos, errneg);
    switch (status)
    {
    case HS_BOTH_ERRORS:
	Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((unsigned char *)(*answer),
						     size*sizeof(float)));
	break;

    case HS_POS_ERRORS:
	if (answer == &errpos)
	    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((unsigned char *)errpos,
							 size*sizeof(float)));
	else
	    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(NULL,0));
	break;

    case HS_NO_ERRORS:
	Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(NULL,0));
	break;

    default:
	Tcl_AppendResult(interp, 
			 "Invalid error retrieval status for 1d histogram with id ", 
			 Tcl_GetStringFromObj(objv[1], NULL), 
			 ". This is a bug. Please report.", NULL);
	free(errpos);
	return TCL_ERROR;
    }
    free(errpos);
    return TCL_OK;
}

tcl_routine(2d_hist_bin_coords)
{
    int axis, i, j, id, size, num_x_bins, num_y_bins, origin = 0;
    float xmin, xmax, ymin, ymax, xstep, ystep;
    float *x;

    tcl_objc_range(3,4);
    verify_2d_histo(id,1);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
        return TCL_ERROR;
    if (get_axis_from_obj(interp, objv[2], 2, 1, &axis) != TCL_OK)
        return TCL_ERROR;
    if (objc == 4)
        if (Tcl_GetBooleanFromObj(interp, objv[3], &origin) != TCL_OK)
            return TCL_ERROR;
    hs_2d_hist_num_bins(id, &num_x_bins, &num_y_bins);
    size = num_x_bins*num_y_bins;
    if ((x = (float *)malloc(size*sizeof(float))) == NULL)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        return TCL_ERROR;
    }
    hs_2d_hist_range(id, &xmin, &xmax, &ymin, &ymax);
    xstep = (xmax - xmin)/(float)num_x_bins;
    ystep = (ymax - ymin)/(float)num_y_bins;
    if (!origin)
    {
        xmin += xstep/2.f;
        ymin += ystep/2.f;
    }
    if (axis == HS_AXIS_X)
    {
        for (i=0; i<num_x_bins; ++i)
            for (j=0; j<num_y_bins; ++j)
                x[i*num_y_bins+j] = xmin + xstep*(float)i;
    }
    else
    {
        for (i=0; i<num_x_bins; ++i)
            for (j=0; j<num_y_bins; ++j)
                x[i*num_y_bins+j] = ymin + ystep*(float)j;
    }
    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((unsigned char *)x,
                                                 size*sizeof(float)));
    free(x);
    return TCL_OK;
}

tcl_routine(3d_hist_bin_coords)
{
    int axis, i, j, k, id, size, origin = 0;
    int num_x_bins, num_y_bins, num_z_bins;
    float xmin, xmax, ymin, ymax, zmin, zmax, xstep, ystep, zstep;
    float *x;

    tcl_objc_range(3,4);
    verify_3d_histo(id,1);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
        return TCL_ERROR;
    if (get_axis_from_obj(interp, objv[2], 3, 1, &axis) != TCL_OK)
        return TCL_ERROR;
    if (objc == 4)
        if (Tcl_GetBooleanFromObj(interp, objv[3], &origin) != TCL_OK)
            return TCL_ERROR;
    hs_3d_hist_num_bins(id, &num_x_bins, &num_y_bins, &num_z_bins);
    size = num_x_bins*num_y_bins*num_z_bins;
    if ((x = (float *)malloc(size*sizeof(float))) == NULL)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        return TCL_ERROR;
    }
    hs_3d_hist_range(id, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
    xstep = (xmax - xmin)/(float)num_x_bins;
    ystep = (ymax - ymin)/(float)num_y_bins;
    zstep = (zmax - zmin)/(float)num_z_bins;
    if (!origin)
    {
        xmin += xstep/2.f;
        ymin += ystep/2.f;
        zmin += zstep/2.f;
    }
    if (axis == HS_AXIS_X)
    {
        for (i=0; i<num_x_bins; ++i)
            for (j=0; j<num_y_bins; ++j)
                for (k=0; k<num_z_bins; ++k)
                    x[(i*num_y_bins+j)*num_z_bins+k] = xmin + xstep*(float)i;
    }
    else if (axis == HS_AXIS_Y)
    {
        for (i=0; i<num_x_bins; ++i)
            for (j=0; j<num_y_bins; ++j)
                for (k=0; k<num_z_bins; ++k)
                    x[(i*num_y_bins+j)*num_z_bins+k] = ymin + ystep*(float)j;
    }
    else
    {
        for (i=0; i<num_x_bins; ++i)
            for (j=0; j<num_y_bins; ++j)
                for (k=0; k<num_z_bins; ++k)
                    x[(i*num_y_bins+j)*num_z_bins+k] = zmin + zstep*(float)k;
    }
    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((unsigned char *)x,
                                                 size*sizeof(float)));
    free(x);
    return TCL_OK;
}

tcl_routine(2d_hist_bin_contents)
{
  int id, size, num_x_bins, num_y_bins;
  float *data;
  
  tcl_require_objc(2);
  verify_2d_histo(id, 1);
  hs_2d_hist_num_bins(id, &num_x_bins, &num_y_bins);
  size = num_x_bins*num_y_bins;
  if ((data = (float *)malloc(size*sizeof(float))) == NULL)
  {
    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
    return TCL_ERROR;
  }
  hs_2d_hist_bin_contents(id, data);
  Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((unsigned char *)data,
					       size*sizeof(float)));
  free(data);
  return TCL_OK;
}

tcl_routine(3d_hist_bin_contents)
{
  int id, size, num_x_bins, num_y_bins, num_z_bins;
  float *data;
  
  tcl_require_objc(2);
  verify_3d_histo(id, 1);
  hs_3d_hist_num_bins(id, &num_x_bins, &num_y_bins, &num_z_bins);
  size = num_x_bins*num_y_bins*num_z_bins;
  if ((data = (float *)malloc(size*sizeof(float))) == NULL)
  {
    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
    return TCL_ERROR;
  }
  hs_3d_hist_bin_contents(id, data);
  Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((unsigned char *)data,
					       size*sizeof(float)));
  free(data);
  return TCL_OK;
}

tcl_routine(2d_hist_errors)
{
    char *cerrtype;
    int id, size, status, num_x_bins, num_y_bins;
    float *errpos, *errneg, **answer;
  
    tcl_objc_range(2,3);
    verify_2d_histo(id, 1);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
	return TCL_ERROR;
    if (objc == 3)
    {
	cerrtype = Tcl_GetStringFromObj(objv[2], NULL);
	if (cerrtype[0] == 'p' || cerrtype[0] == 'P')
	    answer = &errpos;
	else if (cerrtype[0] == 'n' || cerrtype[0] == 'N')
	    answer = &errneg;
	else
	{
	    Tcl_AppendResult(interp, "invalid error type \"",
			     cerrtype, "\"", NULL);
	    return TCL_ERROR;
	}
    }
    else
	answer = &errpos;
    hs_2d_hist_num_bins(id, &num_x_bins, &num_y_bins);
    size = num_x_bins*num_y_bins;
    if ((errpos = (float *)malloc(2*size*sizeof(float))) == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    errneg = errpos + size;
    status = hs_2d_hist_errors(id, errpos, errneg);
    switch (status)
    {
    case HS_BOTH_ERRORS:
	Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((unsigned char *)(*answer),
						     size*sizeof(float)));
	break;

    case HS_POS_ERRORS:
	if (answer == &errpos)
	    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((unsigned char *)errpos,
							 size*sizeof(float)));
	else
	    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(NULL,0));
	break;

    case HS_NO_ERRORS:
	Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(NULL,0));
	break;

    default:
	Tcl_AppendResult(interp, 
			 "Invalid error retrieval status for 2d histogram with id ", 
			 Tcl_GetStringFromObj(objv[1], NULL),
			 ". This is a bug. Please report.", NULL);
	free(errpos);
	return TCL_ERROR;
    }
    free(errpos);
    return TCL_OK;    
}

tcl_routine(3d_hist_errors)
{
    char *cerrtype;
    int id, size, status, num_x_bins, num_y_bins, num_z_bins;
    float *errpos, *errneg, **answer;
  
    tcl_objc_range(2,3);
    verify_3d_histo(id, 1);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
	return TCL_ERROR;
    if (objc == 3)
    {
	cerrtype = Tcl_GetStringFromObj(objv[2], NULL);
	if (cerrtype[0] == 'p' || cerrtype[0] == 'P')
	    answer = &errpos;
	else if (cerrtype[0] == 'n' || cerrtype[0] == 'N')
	    answer = &errneg;
	else
	{
	    Tcl_AppendResult(interp, "invalid error type \"",
			     cerrtype, "\"", NULL);
	    return TCL_ERROR;
	}
    }
    else
	answer = &errpos;
    hs_3d_hist_num_bins(id, &num_x_bins, &num_y_bins, &num_z_bins);
    size = num_x_bins*num_y_bins*num_z_bins;
    if ((errpos = (float *)malloc(2*size*sizeof(float))) == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    errneg = errpos + size;
    status = hs_3d_hist_errors(id, errpos, errneg);
    switch (status)
    {
    case HS_BOTH_ERRORS:
	Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((unsigned char *)(*answer),
						     size*sizeof(float)));
	break;

    case HS_POS_ERRORS:
	if (answer == &errpos)
	    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((unsigned char *)errpos,
							 size*sizeof(float)));
	else
	    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(NULL,0));
	break;

    case HS_NO_ERRORS:
	Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(NULL,0));
	break;

    default:
	Tcl_AppendResult(interp, 
			 "Invalid error retrieval status for 3d histogram with id ", 
			 Tcl_GetStringFromObj(objv[1], NULL),
			 ". This is a bug. Please report.", NULL);
	free(errpos);
	return TCL_ERROR;
    }
    free(errpos);
    return TCL_OK;    
}

tcl_routine(1d_hist_overflows)
{
  int id;
  float xmin, xmax;
  Tcl_Obj *x[2];

  tcl_require_objc(2);
  verify_1d_histo(id, 1);
  hs_1d_hist_overflows(id, &xmin, &xmax);
  x[0] = Tcl_NewDoubleObj((double)xmin);
  x[1] = Tcl_NewDoubleObj((double)xmax);
  Tcl_SetObjResult(interp, Tcl_NewListObj(2, x));
  return TCL_OK;
}

tcl_routine(2d_hist_overflows)
{
  int i, j, id;
  float overflows[3][3];
  Tcl_Obj *list;

  tcl_require_objc(2);
  verify_2d_histo(id, 1);
  hs_2d_hist_overflows(id, overflows);
  list = Tcl_NewListObj(0, NULL);
  for (i=0; i<3; i++)
      for (j=0; j<3; j++)
	  Tcl_ListObjAppendElement(
	      interp, list, Tcl_NewDoubleObj((double)overflows[i][j]));
  Tcl_SetObjResult(interp, list);
  return TCL_OK;
}

tcl_routine(3d_hist_overflows)
{
  int i, j, k, id;
  float overflows[3][3][3];
  char index[16];
  char *array_name;
  Tcl_Obj *dbl;

  tcl_require_objc(3);
  verify_3d_histo(id, 1);
  array_name = Tcl_GetStringFromObj(objv[2], NULL);
  hs_3d_hist_overflows(id, overflows);
  for (i=0; i<3; i++)
      for (j=0; j<3; j++)
	  for (k=0; k<3; k++)
	  {
	      sprintf(index, "%d,%d,%d", i, j, k);
	      dbl = Tcl_NewDoubleObj(overflows[i][j][k]);
	      if (Tcl_SetVar2Ex(interp, array_name, index,
				dbl, TCL_LEAVE_ERR_MSG) == NULL)
	      {
		  Tcl_DecrRefCount(dbl);
		  return TCL_ERROR;
	      }
	  }
  return TCL_OK;
}

tcl_routine(1d_hist_x_value)
{
  int id;
  double x;

  tcl_require_objc(3);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[2], &x) != TCL_OK)
    return TCL_ERROR;
  Tcl_SetObjResult(interp, Tcl_NewDoubleObj(
      (double)hs_1d_hist_x_value(id, (float)x)));
  return TCL_OK;
}

tcl_routine(2d_hist_xy_value)
{
  int id;
  double x, y;
  
  tcl_require_objc(4);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[2], &x) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[3], &y) != TCL_OK)
    return TCL_ERROR;
  Tcl_SetObjResult(interp, Tcl_NewDoubleObj(
      (double)hs_2d_hist_xy_value(id, (float)x, (float)y)));
  return TCL_OK;
}

tcl_routine(3d_hist_xyz_value)
{
  int id;
  double x, y, z;
  
  tcl_require_objc(5);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[2], &x) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[3], &y) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[4], &z) != TCL_OK)
    return TCL_ERROR;
  Tcl_SetObjResult(interp, Tcl_NewDoubleObj(
      (double)hs_3d_hist_xyz_value(id, (float)x, (float)y, (float)z)));
  return TCL_OK;
}

tcl_routine(1d_hist_bin_value)
{
  int id, bin;

  tcl_require_objc(3);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[2], &bin) != TCL_OK)
    return TCL_ERROR;
  Tcl_SetObjResult(interp, Tcl_NewDoubleObj(
      (double)hs_1d_hist_bin_value(id, bin)));
  return TCL_OK;
}

tcl_routine(2d_hist_bin_value)
{
  int id, binx, biny;

  tcl_require_objc(4);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[2], &binx) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[3], &biny) != TCL_OK)
    return TCL_ERROR;
  Tcl_SetObjResult(interp, Tcl_NewDoubleObj(
      (double)hs_2d_hist_bin_value(id, binx, biny)));
  return TCL_OK;
}

tcl_routine(3d_hist_bin_value)
{
  int id, binx, biny, binz;

  tcl_require_objc(5);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[2], &binx) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[3], &biny) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[4], &binz) != TCL_OK)
    return TCL_ERROR;
  Tcl_SetObjResult(interp, Tcl_NewDoubleObj(
      (double)hs_3d_hist_bin_value(id, binx, biny, binz)));
  return TCL_OK;
}

tcl_routine(1d_hist_minimum)
{
  int id, bin;
  float x, value;
  Tcl_Obj *res[3];

  tcl_require_objc(2);
  verify_1d_histo(id, 1);
  hs_1d_hist_minimum(id, &x, &bin, &value);
  res[0] = Tcl_NewDoubleObj((double)x);
  res[1] = Tcl_NewIntObj(bin);
  res[2] = Tcl_NewDoubleObj((double)value);
  Tcl_SetObjResult(interp, Tcl_NewListObj(3, res));
  return TCL_OK;
}

tcl_routine(2d_hist_minimum)
{
  int id, x_bin_num, y_bin_num;
  float x, y, value;
  Tcl_Obj *res[5];

  tcl_require_objc(2);
  verify_2d_histo(id, 1);
  hs_2d_hist_minimum(id, &x, &y, &x_bin_num, &y_bin_num, &value);
  res[0] = Tcl_NewDoubleObj((double)x);
  res[1] = Tcl_NewDoubleObj((double)y);
  res[2] = Tcl_NewIntObj(x_bin_num);
  res[3] = Tcl_NewIntObj(y_bin_num);
  res[4] = Tcl_NewDoubleObj((double)value);
  Tcl_SetObjResult(interp, Tcl_NewListObj(5, res));
  return TCL_OK;
}

tcl_routine(3d_hist_minimum)
{
  int id, x_bin_num, y_bin_num, z_bin_num;
  float x, y, z, value;
  Tcl_Obj *res[7];

  tcl_require_objc(2);
  verify_3d_histo(id, 1);
  hs_3d_hist_minimum(id, &x, &y, &z, &x_bin_num,
		     &y_bin_num, &z_bin_num, &value);
  res[0] = Tcl_NewDoubleObj((double)x);
  res[1] = Tcl_NewDoubleObj((double)y);
  res[2] = Tcl_NewDoubleObj((double)z);
  res[3] = Tcl_NewIntObj(x_bin_num);
  res[4] = Tcl_NewIntObj(y_bin_num);
  res[5] = Tcl_NewIntObj(z_bin_num);
  res[6] = Tcl_NewDoubleObj((double)value);
  Tcl_SetObjResult(interp, Tcl_NewListObj(7, res));
  return TCL_OK;
}

tcl_routine(1d_hist_maximum)
{
  int id, bin;
  float x, value;
  Tcl_Obj *res[3];

  tcl_require_objc(2);
  verify_1d_histo(id, 1);
  hs_1d_hist_maximum(id, &x, &bin, &value);
  res[0] = Tcl_NewDoubleObj((double)x);
  res[1] = Tcl_NewIntObj(bin);
  res[2] = Tcl_NewDoubleObj((double)value);
  Tcl_SetObjResult(interp, Tcl_NewListObj(3, res));
  return TCL_OK;
}

tcl_routine(2d_hist_maximum)
{
  int id, x_bin_num, y_bin_num;
  float x, y, value;
  Tcl_Obj *res[5];

  tcl_require_objc(2);
  verify_2d_histo(id, 1);
  hs_2d_hist_maximum(id, &x, &y, &x_bin_num, &y_bin_num, &value);
  res[0] = Tcl_NewDoubleObj((double)x);
  res[1] = Tcl_NewDoubleObj((double)y);
  res[2] = Tcl_NewIntObj(x_bin_num);
  res[3] = Tcl_NewIntObj(y_bin_num);
  res[4] = Tcl_NewDoubleObj((double)value);
  Tcl_SetObjResult(interp, Tcl_NewListObj(5, res));
  return TCL_OK;
}

tcl_routine(3d_hist_maximum)
{
  int id, x_bin_num, y_bin_num, z_bin_num;
  float x, y, z, value;
  Tcl_Obj *res[7];

  tcl_require_objc(2);
  verify_3d_histo(id, 1);
  hs_3d_hist_maximum(id, &x, &y, &z, &x_bin_num,
		     &y_bin_num, &z_bin_num, &value);
  res[0] = Tcl_NewDoubleObj((double)x);
  res[1] = Tcl_NewDoubleObj((double)y);
  res[2] = Tcl_NewDoubleObj((double)z);
  res[3] = Tcl_NewIntObj(x_bin_num);
  res[4] = Tcl_NewIntObj(y_bin_num);
  res[5] = Tcl_NewIntObj(z_bin_num);
  res[6] = Tcl_NewDoubleObj((double)value);
  Tcl_SetObjResult(interp, Tcl_NewListObj(7, res));
  return TCL_OK;
}

tcl_routine(1d_hist_stats)
{
  int id;
  float xmin, xsigma;
  Tcl_Obj *res[2];

  tcl_require_objc(2);
  verify_1d_histo(id, 1);
  hs_1d_hist_stats(id, &xmin, &xsigma);
  res[0] = Tcl_NewDoubleObj((double)xmin);
  res[1] = Tcl_NewDoubleObj((double)xsigma);
  Tcl_SetObjResult(interp, Tcl_NewListObj(2, res));
  return TCL_OK;
}

tcl_routine(2d_hist_stats)
{
  int id;
  float xmean, ymean, xsigma, ysigma;
  Tcl_Obj *res[4];

  tcl_require_objc(2);
  verify_2d_histo(id, 1);
  hs_2d_hist_stats(id, &xmean, &ymean, &xsigma, &ysigma);
  res[0] = Tcl_NewDoubleObj((double)xmean);
  res[1] = Tcl_NewDoubleObj((double)ymean);
  res[2] = Tcl_NewDoubleObj((double)xsigma);
  res[3] = Tcl_NewDoubleObj((double)ysigma);
  Tcl_SetObjResult(interp, Tcl_NewListObj(4, res));
  return TCL_OK;
}

tcl_routine(3d_hist_stats)
{
  int id;
  float xmean, ymean, zmean, xsigma, ysigma, zsigma;
  Tcl_Obj *res[6];

  tcl_require_objc(2);
  verify_3d_histo(id, 1);
  hs_3d_hist_stats(id, &xmean, &ymean, &zmean,
		   &xsigma, &ysigma, &zsigma);
  res[0] = Tcl_NewDoubleObj((double)xmean);
  res[1] = Tcl_NewDoubleObj((double)ymean);
  res[2] = Tcl_NewDoubleObj((double)zmean);
  res[3] = Tcl_NewDoubleObj((double)xsigma);
  res[4] = Tcl_NewDoubleObj((double)ysigma);
  res[5] = Tcl_NewDoubleObj((double)zsigma);
  Tcl_SetObjResult(interp, Tcl_NewListObj(6, res));
  return TCL_OK;
}

tcl_routine(hist_integral)
{
  int id, type;
  
  tcl_require_objc(2);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  type = hs_type(id);
  if (type != HS_1D_HISTOGRAM && 
      type != HS_2D_HISTOGRAM && 
      type != HS_3D_HISTOGRAM)
  {
    if (type == HS_NONE)
      Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
		       " is not a valid Histo-Scope id", NULL);
    else
      Tcl_AppendResult(interp, "item with id ", 
		       Tcl_GetStringFromObj(objv[1], NULL),
		       " is not a histogram", NULL);
    return TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewDoubleObj(hs_hist_integral(id)));
  return TCL_OK;
}

tcl_routine(hist_l2_norm)
{
  int i, id, type, nbins;
  float *data;
  double sumsq;
  
  tcl_require_objc(2);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  type = hs_type(id);
  if (type != HS_1D_HISTOGRAM && 
      type != HS_2D_HISTOGRAM &&
      type != HS_3D_HISTOGRAM)
  {
    if (type == HS_NONE)
      Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
		       " is not a valid Histo-Scope id", NULL);
    else
      Tcl_AppendResult(interp, "item with id ", 
		       Tcl_GetStringFromObj(objv[1], NULL),
		       " is not a histogram", NULL);
    return TCL_ERROR;
  }
  nbins = hs_hist_num_bins(id);
  data = (float *)malloc(nbins * sizeof(float));
  if (data == NULL)
  {
      Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
      return TCL_ERROR;
  }
  if (type == HS_1D_HISTOGRAM)
      hs_1d_hist_bin_contents(id, data);
  else if (type == HS_2D_HISTOGRAM)
      hs_2d_hist_bin_contents(id, data);
  else if (type == HS_3D_HISTOGRAM)
      hs_3d_hist_bin_contents(id, data);
  else
      assert(0);
  sumsq = 0.0;
  for (i=0; i<nbins; i++)
      sumsq += data[i]*data[i];
  free(data);
  sumsq *= hs_hist_bin_width(id);
  Tcl_SetObjResult(interp, Tcl_NewDoubleObj(sumsq));
  return TCL_OK;
}

tcl_routine(hist_l1_norm)
{
  int i, id, type, nbins;
  float *data;
  double sumsq;
  
  tcl_require_objc(2);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  type = hs_type(id);
  if (type != HS_1D_HISTOGRAM && 
      type != HS_2D_HISTOGRAM &&
      type != HS_3D_HISTOGRAM)
  {
    if (type == HS_NONE)
      Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
		       " is not a valid Histo-Scope id", NULL);
    else
      Tcl_AppendResult(interp, "item with id ", 
		       Tcl_GetStringFromObj(objv[1], NULL),
		       " is not a histogram", NULL);
    return TCL_ERROR;
  }
  nbins = hs_hist_num_bins(id);
  data = (float *)malloc(nbins * sizeof(float));
  if (data == NULL)
  {
      Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
      return TCL_ERROR;
  }
  if (type == HS_1D_HISTOGRAM)
      hs_1d_hist_bin_contents(id, data);
  else if (type == HS_2D_HISTOGRAM)
      hs_2d_hist_bin_contents(id, data);
  else if (type == HS_3D_HISTOGRAM)
      hs_3d_hist_bin_contents(id, data);
  else
      assert(0);
  sumsq = 0.0;
  for (i=0; i<nbins; i++)
      sumsq += fabs((double)(data[i]));
  free(data);
  sumsq *= hs_hist_bin_width(id);
  Tcl_SetObjResult(interp, Tcl_NewDoubleObj(sumsq));
  return TCL_OK;
}

tcl_routine(hist_num_bins)
{
  int id, nbins;

  tcl_require_objc(2);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  nbins = hs_hist_num_bins(id);
  if (nbins < 0)
  {
    if (hs_type(id) == HS_NONE)
      Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
		       " is not a valid Histo-Scope id", NULL);
    else
      Tcl_AppendResult(interp, "item with id ", 
		       Tcl_GetStringFromObj(objv[1], NULL),
		       " is not a histogram", NULL);
    return TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nbins));
  return TCL_OK;
}

tcl_routine(hist_bin_width)
{
  int id, type;
  
  tcl_require_objc(2);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  type = hs_type(id);
  if (type != HS_1D_HISTOGRAM && 
      type != HS_2D_HISTOGRAM &&
      type != HS_3D_HISTOGRAM)
  {
    if (type == HS_NONE)
      Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
		       " is not a valid Histo-Scope id", NULL);
    else
      Tcl_AppendResult(interp, "item with id ", 
		       Tcl_GetStringFromObj(objv[1], NULL),
		       " is not a histogram", NULL);
    return TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewDoubleObj(hs_hist_bin_width(id)));
  return TCL_OK;
}

VOID_FUNCT_WITH_ONE_INT_ARG(hist_set_gauss_errors)

tcl_routine(sum_histograms)
{
  int uid, id1, id2, newid;
  double dc1, dc2;

  tcl_require_objc(8);
  if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[4], &id1) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[5], &id2) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[6], &dc1) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[7], &dc2) != TCL_OK)
    return TCL_ERROR;
  newid = hs_sum_histograms(uid,
		Tcl_GetStringFromObj(objv[2], NULL),
		Tcl_GetStringFromObj(objv[3], NULL),
		id1,id2,(float)dc1,(float)dc2);
  if (newid <= 0)
  {
    /* The C function will print a descriptive message */
    Tcl_SetResult(interp, "Operation failed", TCL_STATIC);
    return TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(newid));
  return TCL_OK;
}

tcl_routine(multiply_histograms)
{
  int uid, id1, id2, newid;
  double dc1;

  tcl_require_objc(7);
  if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
      return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[4], &id1) != TCL_OK)
      return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[5], &id2) != TCL_OK)
      return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[6], &dc1) != TCL_OK)
      return TCL_ERROR;
  newid = hs_multiply_histograms(uid,
		Tcl_GetStringFromObj(objv[2], NULL),
		Tcl_GetStringFromObj(objv[3], NULL),
		id1,id2,(float)dc1);
  if (newid <= 0)
  {
    /* The C function will print a descriptive message */
    Tcl_SetResult(interp, "Operation failed", TCL_STATIC);
    return TCL_ERROR;
  }  
  Tcl_SetObjResult(interp, Tcl_NewIntObj(newid));
  return TCL_OK;
}

tcl_routine(divide_histograms)
{
  int uid, id1, id2, newid;
  double dc1;

  tcl_require_objc(7);
  if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[4], &id1) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[5], &id2) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[6], &dc1) != TCL_OK)
    return TCL_ERROR;
  newid = hs_divide_histograms(uid,
		Tcl_GetStringFromObj(objv[2], NULL),
		Tcl_GetStringFromObj(objv[3], NULL),
		id1,id2,(float)dc1);
  if (newid <= 0)
  {
    /* The C function will print a descriptive message */
    Tcl_SetResult(interp, "Operation failed", TCL_STATIC);
    return TCL_ERROR;
  }  
  Tcl_SetObjResult(interp, Tcl_NewIntObj(newid));
  return TCL_OK;
}

tcl_routine(sum_ntuple_columns)
{
    /* Usage: sum_ntuple_columns id_result id1 column1 id2 column2 ... */
    int id_result, n_inputs, tmp_id, i, irow, objnum, n_rows = -1, col_id;
    void *mem = NULL;
    float *result_accumulator = 0, *column_data = 0;
    int *ids = 0, *cols = 0;

    tcl_objc_range(4,2147483647);
    verify_ntuple(id_result,1);
    if (hs_num_variables(id_result) != 1)
    {
        Tcl_SetResult(interp, "result ntuple must have exactly one column",
                      TCL_STATIC);
        return TCL_ERROR;
    }
    if (objc % 2)
    {
        Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[0],NULL),
                         " : wrong # of arguments", NULL);
        return TCL_ERROR;
    }

    /* Parse ntuple ids and column numbers */
    n_inputs = objc/2 - 1;
    for (i=0; i<n_inputs; ++i)
    {
        objnum = (i+1)*2;
        if (Tcl_GetIntFromObj(interp, objv[objnum], &tmp_id) != TCL_OK)
            goto fail;
        if (Tcl_GetIntFromObj(interp, objv[objnum+1], &col_id) != TCL_OK)
            goto fail;
        if (hs_type(tmp_id) != HS_NTUPLE)
        {
            if (hs_type(tmp_id) == HS_NONE)
                Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[objnum], NULL),
                                 " is not a valid Histo-Scope id", NULL);
            else
                Tcl_AppendResult(interp, "item with id ",
                                 Tcl_GetStringFromObj(objv[objnum], NULL),
                                 " is not an ntuple", NULL);
            goto fail;
        }
        if (col_id < 0 || col_id >= hs_num_variables(tmp_id))
        {
            Tcl_AppendResult(interp, "column number ",
                             Tcl_GetStringFromObj(objv[objnum+1], NULL),
                             " is out of range for ntuple with id ",
                             Tcl_GetStringFromObj(objv[objnum], NULL), NULL);
            goto fail;
        }
        if (n_rows < 0)
        {
            /* At this point we can have the complete info about memory needed */
            n_rows = hs_num_entries(tmp_id);
            if (n_rows == 0)
            {
                Tcl_AppendResult(interp, "ntuple with id ",
                                 Tcl_GetStringFromObj(objv[objnum], NULL),
                                 " is empty", NULL);
                goto fail;
            }
            mem = malloc(2*n_rows*sizeof(float) + 2*n_inputs*sizeof(int));
            if (mem == NULL)
            {
                Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
                goto fail;
            }
            result_accumulator = (float *)mem;
            column_data = result_accumulator + n_rows;
            ids = (int *)(column_data + n_rows);
            cols = ids + n_inputs;
        }
        else
        {
            if (n_rows != hs_num_entries(tmp_id))
            {
                Tcl_AppendResult(interp, "ntuple with id ",
                                 Tcl_GetStringFromObj(objv[objnum], NULL),
                                 " has incompatible number of entries", NULL);
                goto fail;
            }
        }
        ids[i] = tmp_id;
        cols[i] = col_id;
    }

    /* Perform the calculation */
    memset(result_accumulator, 0, n_rows*sizeof(float));
    for (i=0; i<n_inputs; ++i)
    {
        hs_column_contents(ids[i], cols[i], column_data);
        for (irow=0; irow<n_rows; ++irow)
            result_accumulator[irow] += column_data[irow];
    }

    /* Fill the result ntuple */
    hs_reset(id_result);
    for (i=0; i<n_rows; ++i)
        if (hs_fill_ntuple(id_result, result_accumulator+i) != id_result)
        {
            Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
            goto fail;
        }

    free(mem);
    return TCL_OK;

 fail:
    if (mem)
        free(mem);
    return TCL_ERROR;
}

tcl_routine(1d_hist_derivative)
{
  int uid, id, newid;

  tcl_require_objc(5);
  if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[4], &id) != TCL_OK)
    return TCL_ERROR;
  newid = hs_1d_hist_derivative(uid, Tcl_GetStringFromObj(objv[2], NULL),
				Tcl_GetStringFromObj(objv[3], NULL), id);
  if (newid <= 0)
  {
    /* The C function will print a descriptive message */
    Tcl_SetResult(interp, "Operation failed", TCL_STATIC);
    return TCL_ERROR;
  }  
  Tcl_SetObjResult(interp, Tcl_NewIntObj(newid));
  return TCL_OK;
}

tcl_routine(1d_hist_set_overflows)
{
  int id;
  double x, y;
  
  tcl_require_objc(4);
  verify_1d_histo(id, 1);
  if (Tcl_GetDoubleFromObj(interp, objv[2], &x) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[3], &y) != TCL_OK)
    return TCL_ERROR;
  hs_1d_hist_set_overflows(id, (float)x, (float)y);
  return TCL_OK;
}

tcl_routine(2d_hist_set_overflows)
{
  int id, i, j, listlen;
  float overflows[3][3];
  double x;
  Tcl_Obj **listObjElem;

  tcl_require_objc(3);
  verify_2d_histo(id, 1);
  if (Tcl_ListObjGetElements(interp, objv[2], &listlen, &listObjElem) != TCL_OK)
      return TCL_ERROR;
  if (listlen != 9)
  {
    Tcl_SetResult(interp, "wrong # of overflow elements", TCL_STATIC);
    return TCL_ERROR;
  }
  for (i=0; i<3; i++)
    for (j=0; j<3; j++)
    {
	if (Tcl_GetDoubleFromObj(interp, listObjElem[i*3+j], &x) != TCL_OK)
	    return TCL_ERROR;
	else
	    overflows[i][j] = (float)x;
    }
  hs_2d_hist_set_overflows(id, overflows);
  return TCL_OK;
}

tcl_routine(3d_hist_set_overflows)
{
  int id, i, j, k;
  float overflows[3][3][3];
  double x;
  char index[16];
  char *array_name;
  Tcl_Obj *dbl;

  tcl_require_objc(3);
  verify_3d_histo(id, 1);
  array_name = Tcl_GetStringFromObj(objv[2], NULL);
  for (i=0; i<3; i++)
      for (j=0; j<3; j++)
	  for (k=0; k<3; k++)
	  {
	      sprintf(index, "%d,%d,%d", i, j, k);
	      if ((dbl = Tcl_GetVar2Ex(interp, array_name,
				       index, TCL_LEAVE_ERR_MSG)) == NULL)
		  return TCL_ERROR;
	      if (Tcl_GetDoubleFromObj(interp, dbl, &x) != TCL_OK)
		  return TCL_ERROR;
	      overflows[i][j][k] = (float)x;
	  }
  hs_3d_hist_set_overflows(id, overflows);
  return TCL_OK;
}

tcl_routine(num_variables)
{
    int id;
    tcl_require_objc(2);
    verify_ntuple(id,1);
    Tcl_SetObjResult(interp, Tcl_NewIntObj(hs_num_variables(id)));
    return TCL_OK;
}

tcl_routine(variable_name)
{
    int id, column, length;
    char tcl_api_buffer[260];

    tcl_require_objc(3);
    verify_ntuple(id,1);
    if (Tcl_GetIntFromObj(interp, objv[2], &column) != TCL_OK)
        return TCL_ERROR;
    length = hs_variable_name(id, column, tcl_api_buffer);
    if (length < 0)
    {
        if (column < 0 || column >= hs_num_variables(id))
            Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[2], NULL),
                             " is not a valid column number for ntuple ", 
                             Tcl_GetStringFromObj(objv[1], NULL), NULL);
        else
            Tcl_SetResult(interp, "operation failed", TCL_STATIC);
        return TCL_ERROR;
    }
    tcl_api_buffer[length] = '\0';
    Tcl_SetResult(interp, tcl_api_buffer, TCL_VOLATILE);
    return TCL_OK;
}

tcl_routine(variable_index)
{
    int id;

    tcl_require_objc(3);
    verify_ntuple(id,1);
    Tcl_SetObjResult(interp, Tcl_NewIntObj(
        hs_variable_index(id, Tcl_GetStringFromObj(objv[2], NULL))));
    return TCL_OK;
}

tcl_routine(ntuple_value)
{
  int id, row, column;

  tcl_require_objc(4);
  /* Checks on id, row, and column are not done for the sake of speed */
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[2], &row) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[3], &column) != TCL_OK)
    return TCL_ERROR;
  Tcl_SetObjResult(interp, Tcl_NewDoubleObj(
      (double)hs_ntuple_value(id, row, column)));
  return TCL_OK;  
}

tcl_routine(replace_column_contents)
{
    /* Usage: replace_column_contents $id $colnum $values */
    int id, column, nrows, ncols, size, objtype, row;
    float *data = NULL, *array = NULL;
    Tcl_Obj **listObjElem;

    const Tcl_ObjType* byteArrayTypePtr = Tcl_GetObjType("bytearray");
    tcl_require_objc(4);
    verify_ntuple(id,1);
    nrows = hs_num_entries(id);
    ncols = hs_num_variables(id);
    if (Tcl_GetIntFromObj(interp, objv[2], &column) != TCL_OK)
        return TCL_ERROR;
    if (column < 0 || column >= ncols)
    {
        Tcl_SetResult(interp, "column number is out of range", TCL_STATIC);
        return TCL_ERROR;
    }

    /* Check the object type of the data argument */
    if (objv[3]->typePtr == byteArrayTypePtr)
    {
	array = (float *)Tcl_GetByteArrayFromObj(objv[3], &size);
	size /= sizeof(float);
	objtype = 0;
    }
    else
    {
	if (Tcl_ListObjGetElements(interp, objv[3], &size, &listObjElem) != TCL_OK)
	    return TCL_ERROR;    
	objtype = 1;
    }

    /* Check the size of the list or array */
    if (size != nrows)
    {
        Tcl_SetResult(interp, "wrong data size", TCL_STATIC);
        return TCL_ERROR;
    }

    /* Get the data from list */
    if (objtype)
    {
        int i;
        double dtmp;
	if ((array = (float *)malloc(size*sizeof(float))) == NULL)
	{
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    return TCL_ERROR;
	}
        for (i=0; i<size; ++i)
        {
            if (Tcl_GetDoubleFromObj(
                    interp, listObjElem[i], &dtmp) != TCL_OK)
            {
                free(array);
                return TCL_ERROR;
            }
            array[i] = (float)dtmp;
        }
    }

    /* Get the data from the ntuple */
    if ((data = (float *)malloc(nrows*ncols*sizeof(float))) == NULL)
    {
        if (objtype && array) free(array);
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        return TCL_ERROR;
    }
    hs_ntuple_contents(id, data);

    /* Replace the column contents */
    for (row=0; row<nrows; ++row)
        data[row*ncols + column] = array[row];

    /* Refill the ntuple */
    hs_reset(id);
    for (row=0; row<nrows; ++row)
        if (hs_fill_ntuple(id, data+row*ncols) != id)
        {
            free(data);
            if (objtype && array) free(array);
            Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
            return TCL_ERROR;
        }

    free(data);
    if (objtype && array) free(array);
    return TCL_OK;
}

tcl_routine(ntuple_contents)
{
  int id, size;
  float *data;
  
  tcl_require_objc(2);
  verify_ntuple(id,1);
  size = hs_num_variables(id)*hs_num_entries(id);
  if (size > 0)
  {
      if ((data = (float *)malloc(size*sizeof(float))) == NULL)
      {
	  Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	  return TCL_ERROR;
      }
      hs_ntuple_contents(id, data);
      Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((unsigned char *)data,
						   size*sizeof(float)));
      free(data);
  }
  else
      Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(NULL,0));
  return TCL_OK;
}

tcl_routine(row_contents)
{
    int id, row, size;
    float *data;

    tcl_require_objc(3);
    verify_ntuple(id,1);
    if (Tcl_GetIntFromObj(interp, objv[2], &row) != TCL_OK)
        return TCL_ERROR;
    if (row < 0 || row >= hs_num_entries(id))
    {
        Tcl_SetResult(interp, "row number is out of range", TCL_STATIC);
        return TCL_ERROR;
    }
    size = hs_num_variables(id);
    if ((data = (float *)malloc(size*sizeof(float))) == NULL)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        return TCL_ERROR;
    }
    hs_row_contents(id, row, data);
    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((unsigned char *)data,
                                                 size*sizeof(float)));
    free(data);
    return TCL_OK;
}

tcl_routine(column_edf)
{
    /* Usage: column_edf $ntuple_id $column_number reverse_order? */
    int i, id, column, size, inverse_order = 0;
    float *data;
    sorted_column_data *sdata;
    int (*sorter)(const void *, const void *);
    double dsize;

    tcl_objc_range(3,4);
    verify_ntuple(id,1);
    if (Tcl_GetIntFromObj(interp, objv[2], &column) != TCL_OK)
        return TCL_ERROR;
    if (objc > 3)
        if (Tcl_GetBooleanFromObj(interp, objv[3], &inverse_order) != TCL_OK)
            return TCL_ERROR;
    if (inverse_order)
        sorter = (int (*)(const void *, const void *))inverse_sort_column_by_value;
    else
        sorter = (int (*)(const void *, const void *))sort_column_by_value;
    if (column < 0 || column >= hs_num_variables(id))
    {
        Tcl_SetResult(interp, "column number is out of range", 
                      TCL_STATIC);
        return TCL_ERROR;
    }
    size = hs_num_entries(id);
    if (size == 0)
    {
        Tcl_SetResult(interp, "the ntuple is empty", TCL_STATIC);
        return TCL_ERROR;
    }
    if ((data = (float *)malloc(size*sizeof(float))) == NULL)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        return TCL_ERROR;
    }
    if ((sdata = (sorted_column_data *)malloc(size*sizeof(sorted_column_data))) == NULL)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        free(data);
        return TCL_ERROR;
    }
    hs_column_contents(id, column, data);
    for (i=0; i<size; ++i)
    {
        sdata[i].row = i;
        sdata[i].value = data[i];
    }
    qsort(sdata, size, sizeof(sorted_column_data), sorter);
    for (i=0; i<size; ++i)
        sdata[i].sorted_row = i;
    qsort(sdata, size, sizeof(sorted_column_data),
          (int (*)(const void *, const void *))sort_column_by_row);
    dsize = (double)size;
    for (i=0; i<size; ++i)
        data[i] = (float)((sdata[i].sorted_row+0.5)/dsize);
    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((unsigned char *)data, 
                                                 size*sizeof(float)));
    free(sdata);
    free(data);
    return TCL_OK;
}

tcl_routine(sort_ntuple)
{
    /* Usage: sort_ntuple $ntuple_id $column_names reverse_order? */
    int i, ntuple_id, nrows, ncolumns, sort_columns;
    int reverse_order = 0, status = TCL_ERROR;
    Tcl_Obj **listObjElem;
    int *column_list = NULL;
    float *data = NULL;
    char *varname;

    tcl_objc_range(3,4);
    verify_ntuple(ntuple_id,1);
    if (Tcl_ListObjGetElements(interp, objv[2], &sort_columns,
                               &listObjElem) != TCL_OK)
        goto fail;
    column_list = (int *)malloc(sort_columns*sizeof(int));
    if (column_list == NULL)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        goto fail;
    }
    for (i=0; i<sort_columns; ++i)
    {
        varname = Tcl_GetStringFromObj(listObjElem[i],NULL);
        column_list[i] = hs_variable_index(ntuple_id, varname);
        if (column_list[i] < 0)
        {
            Tcl_AppendResult(interp, "\"", varname, "\" is not a variable",
                             " of the ntuple with id ",
                             Tcl_GetStringFromObj(objv[1], NULL), NULL);
            goto fail;
        }
    }
    if (objc > 3)
        if (Tcl_GetBooleanFromObj(interp, objv[3], &reverse_order) != TCL_OK)
            goto fail;
    nrows = hs_num_entries(ntuple_id);
    if (nrows > 0)
    {
        ncolumns = hs_num_variables(ntuple_id);
        data = (float *)malloc(nrows*ncolumns*sizeof(float));
        if (data == NULL)
        {
            Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
            goto fail;
        }
        hs_ntuple_contents(ntuple_id, data);

        /* Sort the data */
        ntuple_sort_columns = column_list;
        n_ntuple_sort_columns = sort_columns;
        if (reverse_order)
            qsort(data, nrows, ncolumns*sizeof(float),
                  (int (*)(const void *, const void *))sort_float_rows_decr);
        else
            qsort(data, nrows, ncolumns*sizeof(float),
                  (int (*)(const void *, const void *))sort_float_rows_incr);
        ntuple_sort_columns = NULL;
        n_ntuple_sort_columns = 0;

        /* Refill the ntuple */
        hs_reset(ntuple_id);
        for (i=0; i<nrows; ++i)
            if (hs_fill_ntuple(ntuple_id, data+i*ncolumns) != ntuple_id)
            {
                Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
                goto fail;
            }
    }

    status = TCL_OK;
 fail:
    if (data)
        free(data);
    if (column_list)
        free(column_list);
    return status;
}

tcl_routine(column_contents)
{
    int id, column, size;
    float *data = NULL;

    tcl_require_objc(3);
    verify_ntuple(id,1);
    if (Tcl_GetIntFromObj(interp, objv[2], &column) != TCL_OK)
        return TCL_ERROR;
    if (column < 0 || column >= hs_num_variables(id))
    {
        Tcl_SetResult(interp, "column number is out of range",
                      TCL_STATIC);
        return TCL_ERROR;
    }
    size = hs_num_entries(id);
    if (size > 0)
    {
        if ((data = (float *)malloc(size*sizeof(float))) == NULL)
        {
            Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
            return TCL_ERROR;
        }
        hs_column_contents(id, column, data);
    }
    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((unsigned char *)data,
                                                 size*sizeof(float)));
    if (data)
        free(data);
    return TCL_OK;
}

tcl_routine(merge_entries)
{
    int uid, id1, id2, new_id;

    tcl_require_objc(6);
    if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
        return TCL_ERROR;
    verify_ntuple(id1,4);
    verify_ntuple(id2,5);
    new_id = hs_merge_entries(
        uid, Tcl_GetStringFromObj(objv[2], NULL),
        Tcl_GetStringFromObj(objv[3], NULL), id1, id2);
    if (new_id <= 0)
    {
        Tcl_AppendResult(interp, "failed to merge ntuples with ids ",
                         Tcl_GetStringFromObj(objv[4], NULL), " and ",
                         Tcl_GetStringFromObj(objv[5], NULL), NULL);
        return TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(new_id));
    return TCL_OK;
}

tcl_routine(sum_category)
{
  tcl_require_objc(4);
  hs_sum_category(Tcl_GetStringFromObj(objv[1], NULL),
		  Tcl_GetStringFromObj(objv[2], NULL),
		  Tcl_GetStringFromObj(objv[3], NULL));
  return TCL_OK;
}

tcl_routine(sum_file)
{
  tcl_require_objc(4);
  hs_sum_file(Tcl_GetStringFromObj(objv[1], NULL),
	      Tcl_GetStringFromObj(objv[2], NULL),
	      Tcl_GetStringFromObj(objv[3], NULL));
  return TCL_OK;
}

/* 
 * Additional functions not in the original C API
 */

int hs_is_valid_c_name(Tcl_Interp *interp, const char *name,
		       const char *what)
{
    const char *pc;
    
    pc = name;
    if (*pc != '_' && !isalpha(*pc))
    {
	Tcl_AppendResult(interp, "bad ", what, " \"",
			 name, "\": must begin with ",
			 "a letter or underscore", NULL);
	return 0;
    }
    for ( ; *pc; ++pc)
    {
	if (!isalnum(*pc) && *pc != '_')
	{
	    Tcl_AppendResult(interp, "bad ", what, " \"",
			     name, "\": only letters, digits, ",
			     "and underscores allowed", NULL);
	    return 0;
	}
    }
    return 1;
}

tcl_routine(C_keyword_list)
{
    Tcl_Obj *result;
    int i, n;
    
    result = Tcl_NewListObj(0, NULL);
    n = sizeof(c_keywords)/sizeof(c_keywords[0]);
    for (i=0; i<n; ++i)
	Tcl_ListObjAppendElement(interp, result,
				 Tcl_NewStringObj(c_keywords[i], -1));
    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

tcl_routine(Is_valid_c_identifier)
{
    char *pc;
    int i, n, status = 1;

    tcl_require_objc(2);
    pc = Tcl_GetStringFromObj(objv[1], NULL);
    if (*pc != '_' && !isalpha(*pc))
	status = 0;
    if (status)
	for ( ; *pc; ++pc)
	    if (!isalnum(*pc) && *pc != '_')
	    {
		status = 0;
		break;
	    }
    if (status)
    {
	n = sizeof(c_keywords)/sizeof(c_keywords[0]);
	for (i=0; i<n; ++i)
	    if (strcmp(pc, c_keywords[i]) == 0)
	    {
		status = 0;
		break;
	    }
    }
    Tcl_SetObjResult(interp, Tcl_NewBooleanObj(status));
    return TCL_OK;
}

tcl_routine(ntuple_bit_histo)
{
    /* Usage: ntuple_bit_histo $uid $title $category $strip_ntuple_id varname */
    int i, datum, bit, index, id, uid, strip_ntuple_id, num_entries;
    char *title, *category, *varname;
    int bitcount[8*sizeof(int)];
    float fbitcount[8*sizeof(int)];
    float *column_data;
    static float max_good_float = 0.f;
    volatile float f;
    
    /* When called the first time, determine the smallest positive integer
     * for which the floating point representation is not precise.
     */
    if (max_good_float == 0.f)
    {
	for (bit=0; bit<(int)(8*sizeof(int)-1); ++bit)
	{
	    i = (1 << bit) + 1;
	    f = (float)i;
	    datum = (int)f;
	    if (datum != i)
		break;
	}
	max_good_float = (float)(i - 1);
    }

    tcl_require_objc(6);
    if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[4], &strip_ntuple_id) != TCL_OK)
	return TCL_ERROR;
    title = Tcl_GetStringFromObj(objv[2], NULL);
    category = Tcl_GetStringFromObj(objv[3], NULL);
    varname = Tcl_GetStringFromObj(objv[5], NULL);

    /* Check that we, indeed, have an ntuple */
    if (hs_type(strip_ntuple_id) != HS_NTUPLE)
    {
	if (hs_type(strip_ntuple_id) == HS_NONE)
	    Tcl_AppendResult(interp, "Histo-Scope item with id ",
			     Tcl_GetStringFromObj(objv[4], NULL),
			     " doesn't exist", NULL);
	else
	    Tcl_AppendResult(interp, "Histo-Scope item with id ",
			     Tcl_GetStringFromObj(objv[4], NULL),
			     " is not an Ntuple", NULL);
	return TCL_ERROR;
    }
    
    /* Find the variable named varname */
    index = hs_variable_index(strip_ntuple_id, varname);
    if (index < 0)
    {
	Tcl_AppendResult(interp, "Ntuple with id ",
			 Tcl_GetStringFromObj(objv[4], NULL),
			 " does not have the \"", varname,
			 "\" variable", NULL);
	return TCL_ERROR;
    }

    /* Get the data and count the bits */
    memset(bitcount, 0, 8*sizeof(int)*sizeof(int));
    num_entries = hs_num_entries(strip_ntuple_id);
    if (num_entries > 0)
    {
	column_data = (float *)malloc(num_entries*sizeof(float));
	hs_column_contents(strip_ntuple_id, index, column_data);
	for (i=0; i<num_entries; ++i)
	{
	    if (column_data[i] > max_good_float || column_data[i] < -max_good_float)
	    {
		/* Unable to get the correct status bits */
		Tcl_AppendResult(interp, "Failed to get correct status bits: ",
				 "data too large to be a precise representation ",
				 "of an integer", NULL);
		free(column_data);
		return TCL_ERROR;
	    }
	    datum = (int)column_data[i];
	    for (bit=0; bit<(int)(8*sizeof(int)-1); ++bit)
		if (datum & (1 << bit))
		    ++bitcount[bit];
	    ++bitcount[8*sizeof(int)-1];
	}
	free(column_data);
    }

    /* Create the status histogram */
    id = hs_create_1d_hist(uid, title, category, "Bit", "Count",
			       8*sizeof(int), 0, 8*sizeof(int));
    if (id <= 0)
    {
	Tcl_AppendResult(interp, "Failed to create status histogram with uid ",
			 Tcl_GetStringFromObj(objv[1], NULL), " title \"",
			 title, "\" and category \"", category, "\"", NULL);
	return TCL_ERROR;
    }
    for (i=0; i<(int)(8*sizeof(int)); ++i)
	fbitcount[i] = (float)bitcount[i];
    hs_1d_hist_block_fill(id, fbitcount, NULL, NULL);

    Tcl_SetObjResult(interp, Tcl_NewIntObj(id));
    return TCL_OK;
}

tcl_routine(ntuple_subrange)
{
    /* Usage: ntuple_subrange $uid $title $category
              $ntuple_id $firstrow $lastrow */
    int row, id = -1, parent_id, uid, firstrow, lastrow, nvars, nrows;
    char *title, *category;
    float *row_data = NULL;

    tcl_require_objc(7);
    if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
	return TCL_ERROR;
    title = Tcl_GetStringFromObj(objv[2], NULL);
    category = Tcl_GetStringFromObj(objv[3], NULL);
    verify_ntuple(parent_id,4);
    if (Tcl_GetIntFromObj(interp, objv[5], &firstrow) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[6], &lastrow) != TCL_OK)
	return TCL_ERROR;

    /* Duplicate the ntuple header */
    id = hs_duplicate_ntuple_header(parent_id, uid, title, category);
    if (id <= 0)
    {
	Tcl_SetResult(interp, "failed to create new ntuple", TCL_STATIC);
	return TCL_ERROR;
    }

    /* Fill the new ntuple */
    nvars = hs_num_variables(parent_id);
    row_data = (float *)malloc(nvars*sizeof(float));
    if (row_data == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }
    nrows = hs_num_entries(parent_id);
    for (row = (firstrow > 0 ? firstrow : 0);
	 row <= lastrow && row < nrows; ++row)
    {
	hs_row_contents(parent_id, row, row_data);
	if (hs_fill_ntuple(id, row_data) != id)
	{
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    goto fail;
	}
    }

    free(row_data);
    Tcl_SetObjResult(interp, Tcl_NewIntObj(id));
    return TCL_OK;

 fail:
    if (id > 0)
	hs_delete(id);
    if (row_data)
	free(row_data);
    return TCL_ERROR;
}

tcl_routine(ntuple_subset)
{
    /* Usage: ntuple_subset $uid $title $category $ntuple_id {column_list} */
    int i, j, id = -1, parent_id, uid, listlen, nrows, nvars;
    char *title, *category;
    char **listelem = 0;
    int *indices = 0;
    Tcl_Obj **listObjElem;
    float *data = 0;

    tcl_require_objc(6);
    if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
	return TCL_ERROR;
    title = Tcl_GetStringFromObj(objv[2], NULL);
    category = Tcl_GetStringFromObj(objv[3], NULL);
    verify_ntuple(parent_id,4);
    if (Tcl_ListObjGetElements(interp, objv[5], &listlen, &listObjElem) != TCL_OK)
	return TCL_ERROR;
    if (listlen == 0)
    {
	Tcl_SetResult(interp, "the list of column names is empty", TCL_STATIC);
	return TCL_ERROR;
    }

    /* Check that all column names are valid */
    if ((listelem = (char **)malloc(listlen*sizeof(char *))) == NULL) {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    if ((indices = (int *)malloc(listlen*sizeof(int))) == NULL) {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail0;
    }
    for (i=0; i<listlen; ++i)
    {
	listelem[i] = Tcl_GetStringFromObj(listObjElem[i], NULL);
	indices[i] = hs_variable_index(parent_id, listelem[i]);
	if (indices[i] < 0)
	{
	    Tcl_AppendResult(interp, "there is no column named \"",
			     listelem[i], "\" in the ntuple with id ",
			     Tcl_GetStringFromObj(objv[4], NULL), NULL);
	    goto fail0;
	}
    }
    i = find_duplicate_name(listelem, listlen);
    if (i >= 0)
    {
	Tcl_AppendResult(interp, "duplicate variable name \"", listelem[i], "\"", NULL);
	goto fail0;
    }
    id = hs_create_ntuple(uid, title, category, listlen, listelem);
    if (id <= 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewIntObj(id));
	goto fail0;
    }
    nrows = hs_num_entries(parent_id);
    if (nrows > 0)
    {
	if (listlen == 1)
	{
	    data = (float *)malloc(nrows*sizeof(float));
	    if (data == NULL) {
		Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
		goto fail0;
	    }
	    hs_column_contents(parent_id, indices[0], data);
	    for (i=0; i<nrows; ++i)
		if (hs_fill_ntuple(id, data+i) != id)
		{
		    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
		    goto fail0;
		}
	}
	else
	{
            float *rowdata;
	    nvars = hs_num_variables(parent_id);
	    data = (float *)malloc((nvars+listlen)*sizeof(float));
	    if (data == NULL) {
		Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
		goto fail0;
	    }
            rowdata = data + nvars;
	    for (i=0; i<nrows; ++i)
	    {
		hs_row_contents(parent_id, i, data);
		for (j=0; j<listlen; ++j)
		    rowdata[j] = data[indices[j]];
		if (hs_fill_ntuple(id, rowdata) != id)
		{
		    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
		    goto fail0;
		}
	    }
	}
    }

    if (data)
	free(data);
    free(indices);
    free(listelem);
    Tcl_SetObjResult(interp, Tcl_NewIntObj(id));
    return TCL_OK;

 fail0:
    if (data)
	free(data);
    if (indices)
	free(indices);
    if (listelem)
	free(listelem);
    if (id > 0)
	hs_delete(id);
    return TCL_ERROR;
}

tcl_routine(ntuple_so_filter)
{
    /* Usage: ntuple_so_filter $ntuple_id $uid $title $category $so_file.
     * The .so file must contain function named "hs_ntuple_filter_function":
     * int hs_ntuple_filter_function(const float *row_data);
     */
    int i, length, parent_id, uid, ntuple_id, nrows, ncolumns;
    char *title, *category, *sofile;
    char buffer[256];
    char **varnames;
    void *handle;
    int (*filter)(float *);
    int status = TCL_ERROR;
    float *ntuple_data = NULL, *thisrow;

    tcl_require_objc(6);
    if (Tcl_GetIntFromObj(interp, objv[1], &parent_id) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[2], &uid) != TCL_OK)
	return TCL_ERROR;
    title = Tcl_GetStringFromObj(objv[3], NULL);
    category = Tcl_GetStringFromObj(objv[4], NULL);
    sofile = Tcl_GetStringFromObj(objv[5], NULL);

    /* Check that we, indeed, have an ntuple */
    if (hs_type(parent_id) != HS_NTUPLE)
    {
	if (hs_type(parent_id) == HS_NONE)
	    Tcl_AppendResult(interp, "Histo-Scope item with id ",
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " doesn't exist", NULL);
	else
	    Tcl_AppendResult(interp, "Histo-Scope item with id ",
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " is not an Ntuple", NULL);
	return TCL_ERROR;
    }

    /* Clear the dl error message buffer */
    dlerror();

    /* Load the .so file */
    handle = dlopen(sofile, RTLD_NOW);
    if (handle == NULL)
    {
	Tcl_AppendResult(interp, "failed to open \"", sofile, "\": ",
			 dlerror(), NULL);
	return TCL_ERROR;
    }

    /* Find the function */
    filter = (int (*)(float *))dlsym(handle, "hs_ntuple_filter_function");
    if ((dlerror()))
    {
	Tcl_AppendResult(interp,
			 "hs_ntuple_filter_function is not defined in file \"",
			 sofile, "\"", NULL);
	goto fail1;
    }

    /* Create the new ntuple */
    ncolumns = hs_num_variables(parent_id);
    varnames = (char **)calloc(ncolumns, sizeof(char *));
    if (varnames == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail1;
    }
    for (i=0; i<ncolumns; ++i)
    {
	length = hs_variable_name(parent_id, i, buffer);
	buffer[length] = '\0';
	if ((varnames[i] = strdup(buffer)) == NULL)
	{
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    goto fail2;
	}
    }
    ntuple_id = hs_create_ntuple(uid, title, category, ncolumns, varnames);
    if (ntuple_id <= 0)
    {
	Tcl_SetResult(interp, "failed to create the new Ntuple", TCL_STATIC);
	goto fail2;
    }

    /* Get the data from the old ntuple */
    nrows = hs_num_entries(parent_id);
    if (nrows > 0)
    {
	ntuple_data = (float *)malloc(nrows*ncolumns*sizeof(float));
	if (ntuple_data == NULL)
	{
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    goto fail3;
	}
	hs_ntuple_contents(parent_id, ntuple_data);
	for (i=0; i<nrows; ++i)
	{
	    thisrow = ntuple_data + i*ncolumns;
	    if (filter(thisrow))
	    {
		if (hs_fill_ntuple(ntuple_id, thisrow) <= 0)
		{
		    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
		    goto fail4;
		}
	    }
	}
    }

    Tcl_SetObjResult(interp, Tcl_NewIntObj(ntuple_id));
    status = TCL_OK;

 fail4:
    if (ntuple_data)
	free(ntuple_data);
 fail3:
    if (status != TCL_OK)
	hs_delete(ntuple_id);
 fail2:
    for (i=0; i<ncolumns; ++i)
	if (varnames[i])
	    free(varnames[i]);
    free(varnames);
 fail1:
    dlclose(handle);
    return status;
}

tcl_routine(ntuple_so_scan)
{
    /* Usage: ntuple_so_scan $ntuple_id $so_file_or_dll \
     *              $reverse_scan_order is_loaded some_string?
     *
     * The .so file or dll must contain a function named
     * "hs_ntuple_scan_function":
     *
     * int hs_ntuple_scan_function(Tcl_Interp *interp, const float *row_data);
     *
     * Optionally, the library may also contain functions
     *
     * int hs_ntuple_scan_init(Tcl_Interp *interp, int ntuple_id,
     *                         const char *some_string);
     * int hs_ntuple_scan_conclude(Tcl_Interp *interp, int ntuple_id,
     *                             const char *some_string);
     *
     * All functions should return either TCL_OK or TCL_ERROR.
     * In case "some_string" is not specified on the command line,
     * NULL pointer will be used with the init and conclude functions.
     */
    int i, parent_id, nrows, ncolumns, reverse_scan_order, dll, is_loaded;
    char *sofile, *some_string = NULL;
    void *handle = NULL;
    int (*scanner)(Tcl_Interp *, float *);
    int (*f_init)(Tcl_Interp *, int, char *);
    int (*f_conclude)(Tcl_Interp *, int, char *);
    int init_exists, conclude_exists;
    int status = TCL_ERROR;
    float *ntuple_data = NULL;

    tcl_objc_range(5, 6);
    if (Tcl_GetIntFromObj(interp, objv[1], &parent_id) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetBooleanFromObj(interp, objv[3], &reverse_scan_order) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetBooleanFromObj(interp, objv[4], &is_loaded) != TCL_OK)
	return TCL_ERROR;
    if (is_loaded)
    {
        sofile = NULL;
        if (Tcl_GetIntFromObj(interp, objv[2], &dll) != TCL_OK)
            return TCL_ERROR;
    }
    else
    {
        sofile = Tcl_GetStringFromObj(objv[2], NULL);
        dll = -1;
    }
    if (objc > 5)
    {
	some_string = Tcl_GetStringFromObj(objv[5], NULL);
        if (some_string[0] == '\0')
            some_string = NULL;
    }

    /* Check that we, indeed, have an ntuple */
    if (hs_type(parent_id) != HS_NTUPLE)
    {
	if (hs_type(parent_id) == HS_NONE)
	    Tcl_AppendResult(interp, "Histo-Scope item with id ",
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " doesn't exist", NULL);
	else
	    Tcl_AppendResult(interp, "Histo-Scope item with id ",
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " is not an Ntuple", NULL);
	return TCL_ERROR;
    }

    if (is_loaded)
    {
        /* Find the functions */
        int findcode;
        f_init = (int (*)(Tcl_Interp *, int, char *))
            hs_find_library_function(interp, dll,
                "hs_ntuple_scan_init", &findcode);
        init_exists = (findcode >= 0);
        if (findcode < 0)
            Tcl_ResetResult(interp);

        scanner = (int (*)(Tcl_Interp *, float *))
            hs_find_library_function(interp, dll,
                "hs_ntuple_scan_function", &findcode);
        if (findcode < 0)
            return TCL_ERROR;

        f_conclude = (int (*)(Tcl_Interp *, int, char *))
            hs_find_library_function(interp, dll,
                "hs_ntuple_scan_conclude", &findcode);
        conclude_exists = (findcode >= 0);
        if (findcode < 0)
            Tcl_ResetResult(interp);
    }
    else
    {
        /* Clear the dl error message buffer */
        dlerror();

        /* Load the .so file */
        handle = dlopen(sofile, RTLD_NOW);
        if (handle == NULL)
        {
            Tcl_AppendResult(interp, "failed to open \"", sofile, "\": ",
                             dlerror(), NULL);
            return TCL_ERROR;
        }

        /* Find the functions */
        f_init = (int (*)(Tcl_Interp *, int, char *))dlsym(
            handle, "hs_ntuple_scan_init");
        init_exists = (dlerror() == NULL);
        scanner = (int (*)(Tcl_Interp *, float *))dlsym(
            handle, "hs_ntuple_scan_function");
        if ((dlerror()))
        {
            Tcl_AppendResult(interp, "hs_ntuple_scan_function "
                             "is not defined in file \"",
                             sofile, "\"", NULL);
            goto fail;
        }
        f_conclude = (int (*)(Tcl_Interp *, int, char *))dlsym(
            handle, "hs_ntuple_scan_conclude");
        conclude_exists = (dlerror() == NULL);
    }

    /* Get the data from the ntuple and scan it */
    nrows = hs_num_entries(parent_id);
    ncolumns = hs_num_variables(parent_id);
    if (nrows > 0)
    {
	ntuple_data = (float *)malloc(ncolumns*sizeof(float));
	if (ntuple_data == NULL)
	{
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    goto fail;
	}
	if (init_exists)
	    if (f_init(interp, parent_id, some_string) != TCL_OK)
		goto fail;
        if (reverse_scan_order)   
        {
            for (i=nrows-1; i>=0; --i)
            {
                hs_row_contents(parent_id, i, ntuple_data);
                if (scanner(interp, ntuple_data) != TCL_OK)
                    goto fail;
            }
        }
        else
        {
            for (i=0; i<nrows; ++i)
            {
                hs_row_contents(parent_id, i, ntuple_data);
                if (scanner(interp, ntuple_data) != TCL_OK)
                    goto fail;
            }
        }
	if (conclude_exists)
	    if (f_conclude(interp, parent_id, some_string) != TCL_OK)
		goto fail;
    }

    status = TCL_OK;
 fail:
    if (ntuple_data)
	free(ntuple_data);
    if (!is_loaded)
        dlclose(handle);
    return status;
}

tcl_routine(Column_minmax)
{
    /* Column_minmax id column_number */
    int id, row, column, nvars, nrows;
    float fmin = FLT_MAX, fmax = -FLT_MAX;
    int row_min = 0, row_max = 0;
    float *data;
    Tcl_Obj *results[4];

    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
        return TCL_ERROR;
    nvars = hs_num_variables(id);
    if (nvars < 0)
    {
        if (hs_type(id) == HS_NONE)
            Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
                             " is not a valid Histo-Scope id", NULL);
        else
            Tcl_AppendResult(interp, "item with id ", 
                             Tcl_GetStringFromObj(objv[1], NULL),
                             " is not an ntuple", NULL);
        return TCL_ERROR;
    }
    nrows = hs_num_entries(id);
    if (nrows == 0)
    {
        Tcl_AppendResult(interp, "ntuple with id ", 
                         Tcl_GetStringFromObj(objv[1], NULL),
                         " is empty", NULL);
        return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, objv[2], &column) != TCL_OK)
        return TCL_ERROR;
    if (column < 0 || column >= nvars)
    {
        Tcl_SetResult(interp, "column number "
                      "is out of range", TCL_STATIC);
        return TCL_ERROR;
    }
    data = (float *)malloc(nrows*sizeof(float));
    if (data == NULL)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        return TCL_ERROR;
    }
    hs_column_contents(id, column, data);
    for (row=0; row<nrows; ++row)
    {
        register float f = data[row];
        if (f < fmin)
        {
            fmin = f;
            row_min = row;
        }
        if (f > fmax)
        {
            fmax = f;
            row_max = row;
        }
    }
    free(data);
    results[0] = Tcl_NewIntObj(row_min);
    results[1] = Tcl_NewDoubleObj((double)fmin);
    results[2] = Tcl_NewIntObj(row_max);
    results[3] = Tcl_NewDoubleObj((double)fmax);
    Tcl_SetObjResult(interp, Tcl_NewListObj(4,results));
    return TCL_OK;
}

tcl_routine(ntuple_histo_fill)
{
    int i, j, k, bin, nt_id, histo_id, itype;
    int nbins, nxbins, nybins, nzbins, nvars;
    int errsta, minvars, maxvars, status = TCL_ERROR;
    float *x, *y, *z, *data, *poserr, *negerr, *mem = NULL;
    float xmin, xmax, ymin, ymax, zmin, zmax, xval, yval, ntdata[6];
    double dxmin, dymin, dzmin, dx, dy, dz;

    tcl_require_objc(3);
    verify_ntuple(nt_id,1);
    nvars = hs_num_variables(nt_id);
    if (Tcl_GetIntFromObj(interp, objv[2], &histo_id) != TCL_OK)
	return TCL_ERROR;
    itype = hs_type(histo_id);
    switch (itype)
    {
    case HS_1D_HISTOGRAM:
	nbins = hs_1d_hist_num_bins(histo_id);
	hs_1d_hist_range(histo_id, &xmin, &xmax);
	minvars = 2;
	maxvars = 4;
	break;
    case HS_2D_HISTOGRAM:
	hs_2d_hist_num_bins(histo_id, &nxbins, &nybins);
	hs_2d_hist_range(histo_id, &xmin, &xmax, &ymin, &ymax);
	nbins = nxbins*nybins;
	minvars = 3;
	maxvars = 5;
	break;
     case HS_3D_HISTOGRAM:
	hs_3d_hist_num_bins(histo_id, &nxbins, &nybins, &nzbins);
	hs_3d_hist_range(histo_id, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
	nbins = nxbins*nybins*nzbins;
	minvars = 4;
	maxvars = 6;
	break;
   case HS_NONE:
	Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[2], NULL),
			 " is not a valid Histo-Scope id", NULL);
	return TCL_ERROR;
    default:
	Tcl_AppendResult(interp, "item with id ", 
			 Tcl_GetStringFromObj(objv[2], NULL),
			 " is not a histogram", NULL);
	return TCL_ERROR;
    }
    assert((unsigned)maxvars <= sizeof(ntdata)/sizeof(ntdata[0]));
    if (nvars < minvars)
    {
	Tcl_SetResult(interp, "not enough variables in the ntuple",
		      TCL_STATIC);
	return TCL_ERROR;
    }
    if (nvars > maxvars)
    {
	Tcl_SetResult(interp, "too many variables in the ntuple",
		      TCL_STATIC);
	return TCL_ERROR;
    }

    /* Allocate and pass the memory */
    mem = (float *)malloc(nbins*6*sizeof(float));
    if (mem == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    x = mem;
    switch (itype)
    {
    case HS_1D_HISTOGRAM:
	z = NULL;
	y = NULL;
	data = x + nbins;
	break;
    case HS_2D_HISTOGRAM:
	z = NULL;
	y = x + nbins;
	data = y + nbins;
	break;
    case HS_3D_HISTOGRAM:
	y = x + nbins;
	z = y + nbins;
	data = z + nbins;
	break;
    default:
	assert(0);
    }
    poserr = data + nbins;
    negerr = poserr + nbins;

    /* Get the data */
    switch (itype)
    {
    case HS_1D_HISTOGRAM:
	hs_1d_hist_bin_contents(histo_id, data);
	errsta = hs_1d_hist_errors(histo_id, poserr, negerr);
	dxmin = xmin;
	dx = (xmax - dxmin)/(double)nbins;
	dxmin += (dx/2.0);
	for (i=0; i<nbins; ++i)
	    x[i] = (float)(dxmin + dx*(double)i);
	break;
    case HS_2D_HISTOGRAM:
	hs_2d_hist_bin_contents(histo_id, data);
	errsta = hs_2d_hist_errors(histo_id, poserr, negerr);
	dxmin = xmin;
	dx = (xmax - dxmin)/(double)nxbins;
	dxmin += (dx/2.0);
	dymin = ymin;
	dy = (ymax - dymin)/(double)nybins;
	dymin += (dy/2.0);
	bin = 0;
	for (i=0; i<nxbins; ++i)
	{
	    xval = (float)(dxmin + dx*(double)i);
	    for (j=0; j<nybins; ++j, ++bin)
	    {
		x[bin] = xval;
		y[bin] = (float)(dymin + dy*(double)j);
	    }
	}
	break;
    case HS_3D_HISTOGRAM:
	hs_3d_hist_bin_contents(histo_id, data);
	errsta = hs_3d_hist_errors(histo_id, poserr, negerr);
	dxmin = xmin;
	dx = (xmax - dxmin)/(double)nxbins;
	dxmin += (dx/2.0);
	dymin = ymin;
	dy = (ymax - dymin)/(double)nybins;
	dymin += (dy/2.0);
	dzmin = zmin;
	dz = (zmax - dzmin)/(double)nzbins;
	dzmin += (dz/2.0);
	bin = 0;
	for (i=0; i<nxbins; ++i)
	{
	    xval = (float)(dxmin + dx*(double)i);
	    for (j=0; j<nybins; ++j)
	    {
		yval = (float)(dymin + dy*(double)j);
		for (k=0; k<nzbins; ++k, ++bin)
		{
		    x[bin] = xval;
		    y[bin] = yval;
		    z[bin] = (float)(dzmin + dz*(double)k);
		}
	    }
	}
	break;
    default:
	assert(0);
    }
    switch (errsta)
    {
    case HS_BOTH_ERRORS:
	break;
    case HS_POS_ERRORS:
	memset(negerr, 0, nbins*sizeof(float));
	break;
    case HS_NO_ERRORS:
	memset(poserr, 0, 2*nbins*sizeof(float));
	break;
    default:
	assert(0);
    }

    /* Fill the ntuple */
    for (j=0; j<nbins; ++j)
    {
	for (i=0; i<nvars; ++i)
	    ntdata[i] = mem[j+i*nbins];
	if (hs_fill_ntuple(nt_id, ntdata) != nt_id)
	{
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    goto fail;
	}
    }

    status = TCL_OK;
 fail:
    if (mem)
	free(mem);
    return status;
}

tcl_routine(add_filled_columns)
{
     /* Command looks like this:  
      * hs::add_filled_columns id uid title category name1 data1 name2 data2 ...
      */
    int i, id, uid, nvars_old, nvars_new, n_entries, row, isize;
    int newid = 0, nvars_total = 0, status = TCL_ERROR;
    char *title, *category;
    char **varnames = NULL;
    char buf[260];
    typedef struct _column_ptr {
        int freeit;
        float *data;
    } column_ptr;
    column_ptr *column_data = NULL;
    float *row_data = NULL;

    if (objc < 5)
    {
        Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[0],NULL),
                         " : wrong # of arguments", NULL);
        return TCL_ERROR;
    }
    if ((objc - 5) % 2)
    {
        Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[0],NULL),
                         " : wrong # of arguments", NULL);
        return TCL_ERROR;
    }
    verify_ntuple(id,1);
    if (Tcl_GetIntFromObj(interp, objv[2], &uid) != TCL_OK)
        return TCL_ERROR;
    title = Tcl_GetStringFromObj(objv[3], NULL);
    category = Tcl_GetStringFromObj(objv[4], NULL);

    /* Check if no new variables are named */
    if (objc == 5)
    {
        newid = hs_copy_hist(id, uid, title, category);
        if (newid <= 0)
        {
            Tcl_AppendResult(interp, "failed to create ntuple with uid ",
                             Tcl_GetStringFromObj(objv[2], NULL),
                             ", title \"", title, "\", and category \"",
                             category, "\"", NULL);
            return TCL_ERROR;
        }
        Tcl_SetObjResult(interp, Tcl_NewIntObj(newid));
        return TCL_OK;
    }

    /* Check the column names */
    nvars_new = (objc-5)/2;
    nvars_old = hs_num_variables(id);
    nvars_total = nvars_old+nvars_new;
    varnames = (char **)calloc(nvars_total, sizeof(char *));
    if (varnames == NULL)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        goto fail;
    }
    for (i=0; i<nvars_old; ++i)
    {
        hs_variable_name(id, i, buf);
        varnames[i] = strdup(buf);
        if (varnames[i] == NULL)
        {
            Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
            goto fail;
        }
    }
    for (i=0; i<nvars_new; ++i)
    {
        varnames[nvars_old+i] = strdup(Tcl_GetStringFromObj(objv[5+i*2], NULL));
        if (varnames[nvars_old+i] == NULL)
        {
            Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
            goto fail;
        }
    }
    i = find_duplicate_name(varnames, nvars_total);
    if (i >= 0)
    {
        Tcl_AppendResult(interp, "duplicate variable name \"", varnames[i], "\"", NULL);
        goto fail;
    }

    /* Check the data arguments */
    n_entries = hs_num_entries(id);
    column_data = (column_ptr *)calloc(nvars_new, sizeof(column_ptr));
    if (column_data == NULL)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        goto fail;
    }
    for (i=0; i<nvars_new; ++i)
    {
        if (get_float_array_from_binary_or_list(
            interp, objv[6+i*2], &column_data[i].data,
            &isize, &column_data[i].freeit) != TCL_OK)
            goto fail;
        if (isize != n_entries)
        {
            Tcl_AppendResult(interp, "wrong number of entries for column \"",
                             Tcl_GetStringFromObj(objv[5+i*2], NULL), "\"", NULL);
            goto fail;
        }
    }

    /* Allocate memory for the data array */
    row_data = (float *)malloc(nvars_total*sizeof(float));
    if (row_data == NULL)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        goto fail;
    }

    /* Create and fill the new ntuple */
    newid = hs_create_ntuple(uid, title, category, nvars_total, varnames);
    if (newid <= 0)
    {
        Tcl_AppendResult(interp, "failed to create ntuple with uid ",
                         Tcl_GetStringFromObj(objv[2], NULL),
                         ", title \"", title, "\", and category \"",
                         category, "\"", NULL);
        goto fail;
    }
    for (row=0; row<n_entries; ++row)
    {
        hs_row_contents(id, row, row_data);
        for (i=0; i<nvars_new; ++i)
            row_data[i+nvars_old] = column_data[i].data[row];
        if (hs_fill_ntuple(newid, row_data) != newid)
        {
            Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
            goto fail;
        }
    }

    Tcl_SetObjResult(interp, Tcl_NewIntObj(newid));
    status = TCL_OK;

 fail:
    if (newid > 0 && status == TCL_ERROR)
        hs_delete(newid);
    if (row_data)
        free(row_data);
    if (column_data)
    {
        for (i=nvars_new-1; i>=0; --i)
            if (column_data[i].freeit)
                free(column_data[i].data);
        free(column_data);
    }
    if (varnames)
    {
        for (i=nvars_total-1; i>=0; --i)
            if (varnames[i])
                free(varnames[i]);
        free(varnames);
    }
    return status;
}

VOID_FUNCT_WITH_VOID_ARG(histoscope_hidden)

tcl_routine(change_uid_and_category)
{
  int id, newid;
  
  tcl_require_objc(4);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[2], &newid) != TCL_OK)
    return TCL_ERROR;
  hs_change_uid_and_category(id, newid, Tcl_GetStringFromObj(objv[3], NULL));
  return TCL_OK;
}

tcl_routine(save_file_byids)
{
  int i, nids;
  int *ids;
  Tcl_Obj **listObjElem;

  tcl_require_objc(3);  
  if (Tcl_ListObjGetElements(interp, objv[2], &nids, &listObjElem) != TCL_OK)
      return TCL_ERROR;
  if ((ids = (int *)malloc(nids*sizeof(int))) == NULL)
  {
    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
    return TCL_ERROR;
  }
  for (i=0; i < nids; i++)
    if (Tcl_GetIntFromObj(interp, listObjElem[i], ids+i) != TCL_OK)
    {
      free(ids);
      return TCL_ERROR;
    }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(
      hs_save_file_byids(Tcl_GetStringFromObj(objv[1], NULL), 
			 ids, nids)));
  free(ids);
  return TCL_OK; 
}

tcl_routine(reset_const)
{
  int id, type;
  double dc;
  
  tcl_require_objc(3);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetDoubleFromObj(interp, objv[2], &dc) != TCL_OK)
    return TCL_ERROR;
  type = hs_type(id);
  if (type == HS_1D_HISTOGRAM || 
      type == HS_2D_HISTOGRAM ||
      type == HS_3D_HISTOGRAM)
  {
      hs_reset_const(id, (float)dc);
  }
  else
  {
      if (type == HS_NONE)
	  Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			   " is not a valid Histo-Scope id", NULL);
      else
	  Tcl_AppendResult(interp, "item with id ", 
			   Tcl_GetStringFromObj(objv[1], NULL),
			   " is not a histogram", NULL);
      return TCL_ERROR;
  }
  return TCL_OK;
}

tcl_routine(copy_hist)
{
  int id, uid, newid;

  tcl_require_objc(5);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (Tcl_GetIntFromObj(interp, objv[2], &uid) != TCL_OK)
    return TCL_ERROR;
  newid = hs_copy_hist(id, uid, 
		       Tcl_GetStringFromObj(objv[3], NULL), 
		       Tcl_GetStringFromObj(objv[4], NULL));
  if (newid <= 0)
  {
      Tcl_AppendResult(interp, "Failed to copy Histo-Scope item with id ",
		       Tcl_GetStringFromObj(objv[1], NULL), NULL);
      return TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(newid));
  return TCL_OK;
}

tcl_routine(1d_hist_labels)
{
  int id;
  char tcl_api_buffer[512];
  Tcl_Obj * labels[2];

  tcl_require_objc(2);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (hs_1d_hist_labels(id, tcl_api_buffer, tcl_api_buffer+256) <= 0)
  {
    if (hs_type(id) == HS_NONE)
	Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			 " is not a valid Histo-Scope id", NULL);
    else
	Tcl_AppendResult(interp, "item with id ", 
			 Tcl_GetStringFromObj(objv[1], NULL),
			 " is not a 1d histogram", NULL);
    return TCL_ERROR;
  }
  labels[0] = Tcl_NewStringObj(tcl_api_buffer, -1);
  labels[1] = Tcl_NewStringObj(tcl_api_buffer+256, -1);
  Tcl_SetObjResult(interp, Tcl_NewListObj(2, labels));
  return TCL_OK;
}

tcl_routine(2d_hist_labels)
{
  int id;
  char tcl_api_buffer[768];
  Tcl_Obj * labels[3];

  tcl_require_objc(2);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (hs_2d_hist_labels(id, tcl_api_buffer, 
			tcl_api_buffer+256, tcl_api_buffer+512) <= 0)
  {
    if (hs_type(id) == HS_NONE)
      Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
		       " is not a valid Histo-Scope id", NULL);
    else
      Tcl_AppendResult(interp, "item with id ", 
		       Tcl_GetStringFromObj(objv[1], NULL),
		       " is not a 2d histogram", NULL);
    return TCL_ERROR;
  }
  for (id=0; id<3; ++id)
      labels[id] = Tcl_NewStringObj(tcl_api_buffer+256*id, -1);
  Tcl_SetObjResult(interp, Tcl_NewListObj(3, labels));
  return TCL_OK;
}

tcl_routine(3d_hist_labels)
{
  int id;
  char tcl_api_buffer[1024];
  Tcl_Obj * labels[4];

  tcl_require_objc(2);
  if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
    return TCL_ERROR;
  if (hs_3d_hist_labels(id, tcl_api_buffer, tcl_api_buffer+256,
			tcl_api_buffer+512, tcl_api_buffer+768) <= 0)
  {
    if (hs_type(id) == HS_NONE)
      Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
		       " is not a valid Histo-Scope id", NULL);
    else
      Tcl_AppendResult(interp, "item with id ", 
		       Tcl_GetStringFromObj(objv[1], NULL),
		       " is not a 3d histogram", NULL);
    return TCL_ERROR;
  }
  for (id=0; id<4; ++id)
      labels[id] = Tcl_NewStringObj(tcl_api_buffer+256*id, -1);
  Tcl_SetObjResult(interp, Tcl_NewListObj(4, labels));
  return TCL_OK;
}

VOID_FUNCT_WITH_ONE_INT_ARG(allow_item_send)
BOOL_FUNCT_WITH_VOID_ARG(socket_status)

tcl_routine(allow_reset_refresh)
{
    int id, flag, type;
    
    tcl_require_objc(3);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetBooleanFromObj(interp, objv[2], &flag) != TCL_OK)
	return TCL_ERROR;
    type = hs_type(id);
    if (type != HS_NTUPLE)
    {
	if (type == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a valid Histo-Scope id", NULL);
	else
	    Tcl_AppendResult(interp, "item with id ", 
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " is not an ntuple", NULL);
	return TCL_ERROR;
    }
    hs_allow_reset_refresh(id, flag);
    return TCL_OK;
}

/* The following two definitions comprise the code common for
   "fill_coord_slice" and  "fill_hist_slice" functions */
#define parse_slice_and_parent_id do {\
    if (Tcl_GetIntFromObj(interp, objv[1], &slice_id) != TCL_OK)\
	return TCL_ERROR;\
    slice_type = hs_type(slice_id);\
    if (histo_dim_from_type(slice_type) == 0)\
    {\
	if (slice_type == HS_NONE)\
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),\
			     " is not a valid Histo-Scope id", NULL);\
	else\
	    Tcl_AppendResult(interp, "item with id ",\
			     Tcl_GetStringFromObj(objv[1], NULL),\
			     " is not a histogram", NULL);\
	return TCL_ERROR;\
    }\
    if (Tcl_GetIntFromObj(interp, objv[2], &parent_id) != TCL_OK)\
	return TCL_ERROR;\
    parent_type = hs_type(parent_id);\
    if ((parent_dim = histo_dim_from_type(parent_type)) < 2)\
    {\
	if (parent_type == HS_NONE)\
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[2], NULL),\
			     " is not a valid Histo-Scope id", NULL);\
        else if (parent_type == HS_1D_HISTOGRAM)\
            Tcl_AppendResult(interp, "item with id ",\
			    Tcl_GetStringFromObj(objv[2], NULL),\
			    " can not be sliced: it is a 1d histogram", NULL);\
	else\
	    Tcl_AppendResult(interp, "item with id ",\
			     Tcl_GetStringFromObj(objv[2], NULL),\
			     " is not a histogram", NULL);\
	return TCL_ERROR;\
    }\
} while(0);

#define common_fill_slice_code do {\
    if (parent_type == HS_2D_HISTOGRAM && axis2 != HS_AXIS_NONE) {\
        Tcl_SetResult(interp, "more than one slice axis specified "\
		      "for a 2d parent histogram", TCL_STATIC);\
	return TCL_ERROR;\
    }\
    if (axis1 == axis2) {\
	Tcl_SetResult(interp,\
                      "can not specify the same slice axis more than once",\
		      TCL_STATIC);\
	return TCL_ERROR;\
    }\
    /* Make sure that the types are compatible */\
    if (!((parent_type == HS_2D_HISTOGRAM && \
	   slice_type == HS_1D_HISTOGRAM && \
	   axis2 == HS_AXIS_NONE) || \
	  (parent_type == HS_3D_HISTOGRAM && \
	   slice_type == HS_1D_HISTOGRAM && \
	   axis2 != HS_AXIS_NONE) || \
	  (parent_type == HS_3D_HISTOGRAM && \
	   slice_type == HS_2D_HISTOGRAM && \
	   axis2 == HS_AXIS_NONE)))\
    {\
	Tcl_AppendResult(interp, "slice with id ",\
			 Tcl_GetStringFromObj(objv[1], NULL),\
			 " and parent with id ",\
			 Tcl_GetStringFromObj(objv[2], NULL),\
			 " have incompatible dimensionality", NULL);\
	return TCL_ERROR;\
    }\
    switch (hs_fill_hist_slice(parent_id, axis1, bin1,\
			       axis2, bin2, slice_id))\
    {\
    case 0:\
	/* Success */\
	return TCL_OK;\
    case -2:\
	Tcl_AppendResult(interp, "slice with id ",\
			 Tcl_GetStringFromObj(objv[1], NULL),\
			 " and parent with id ",\
			 Tcl_GetStringFromObj(objv[2], NULL),\
			 " are not bin-compatible", NULL);\
	break;\
    case -3:\
	/* This should never happen (this return code means\
	   incompatible dimensionality). If we are here, we have a bug. */\
	fprintf(stderr, "Internal bug in fill_hist_slice tcl API. "\
		"Please report. Aborting.\n");\
	assert(0);\
	break;\
    case -4:\
	/* Bin number out of range */\
	Tcl_SetResult(interp, "bin number out of range", TCL_STATIC);\
	break;\
    default:\
	/* Generic error. A message should be printed to stderr. */\
	Tcl_SetResult(interp,\
		      "operation failed, check stderr for diagnostics",\
		      TCL_STATIC);\
	break;\
    }\
    return TCL_ERROR;\
} while(0);

tcl_routine(fill_coord_slice)
{
   /* Usage: fill_coord_slice slice_id parent_id axis1 coord1 axis2? coord2? */
    int slice_id, slice_type, parent_id, parent_type, parent_dim, abins;
    int axis1, bin1, axis2 = HS_AXIS_NONE, bin2 = -1;
    double coord1, coord2;
    float amin, amax; 
    static float xmin, xmax, ymin, ymax, zmin, zmax;
    static int parent_old = -1, xbins, ybins, zbins = 0;

    tcl_objc_range(5, 7);
    parse_slice_and_parent_id;
    if (parent_old != parent_id)
    {
	switch (parent_type)
	{
	case HS_2D_HISTOGRAM:
	    hs_2d_hist_num_bins(parent_id, &xbins, &ybins);
	    hs_2d_hist_range(parent_id, &xmin, &xmax, &ymin, &ymax);
	    break;
	case HS_3D_HISTOGRAM:
	    hs_3d_hist_num_bins(parent_id, &xbins, &ybins, &zbins);
	    hs_3d_hist_range(parent_id, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
	    break;
	default:
	    assert(0);
	}
	parent_old = parent_id;
    }

/* The +0.01 part in the above formula for bin_var makes sure that we
   get the correct bin number when we are exactly on the left bin edge.
   Of course, it will screw up the right bin edge. */
#define get_bin_from_axis_and_coord(axis_var, coord_var, bin_var) do {\
    switch (axis_var)\
    {\
    case HS_AXIS_X:\
	amin = xmin;\
	amax = xmax;\
	abins = xbins;\
	break;\
    case HS_AXIS_Y:\
	amin = ymin;\
	amax = ymax;\
	abins = ybins;\
	break;\
    case HS_AXIS_Z:\
	amin = zmin;\
	amax = zmax;\
	abins = zbins;\
	break;\
    default:\
	assert(0);\
    }\
    bin_var = (int)((((float)coord_var-amin)*(float)abins)/(amax-amin) + 0.01);\
    if (bin_var < 0)\
	bin_var = 0;\
    if (bin_var >= abins)\
	bin_var = abins-1;\
} while (0);

    if (get_axis_from_obj(interp, objv[3], parent_dim, 1, &axis1) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[4], &coord1) != TCL_OK)
	return TCL_ERROR;
    get_bin_from_axis_and_coord(axis1, coord1, bin1);
    if (objc > 5)
    {
	tcl_require_objc(7);
	if (get_axis_from_obj(interp, objv[5], parent_dim, 0, &axis2) != TCL_OK)
	    return TCL_ERROR;
	if (Tcl_GetDoubleFromObj(interp, objv[6], &coord2) != TCL_OK)
	    return TCL_ERROR;
	get_bin_from_axis_and_coord(axis2, coord2, bin2);
    }
    common_fill_slice_code;
}

tcl_routine(fill_hist_slice)
{
    /* Usage: fill_hist_slice slice_id parent_id axis1 bin1 axis2? bin2? */
    int slice_id, parent_id, slice_type, parent_type;
    int axis1, bin1, axis2 = HS_AXIS_NONE, bin2 = -1;
    int parent_dim;

    tcl_objc_range(5, 7);
    parse_slice_and_parent_id;
    if (get_axis_from_obj(interp, objv[3], parent_dim, 1, &axis1) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[4], &bin1) != TCL_OK)
	return TCL_ERROR;
    if (objc > 5)
    {
	tcl_require_objc(7);
	if (get_axis_from_obj(interp, objv[5], parent_dim, 0, &axis2) != TCL_OK)
	    return TCL_ERROR;
	if (Tcl_GetIntFromObj(interp, objv[6], &bin2) != TCL_OK)
	    return TCL_ERROR;
    }
    common_fill_slice_code;
}

tcl_routine(slice_contents)
{
    /* Usage: slice_contents id result_specifier axis1 bin1 axis2? bin2?
     * 
     * result_specifier is a list like this {d p n} which tells what to
     * return and in which order. Every element of this list must be one
     * of the letters d (for data), p (for positive errors), or n (for
     * negative errors). The letter is not case-sensitive.
     */
    int i, listlen, parent_id, parent_type, status, nfilled;
    int axis1, bin1, axis2 = HS_AXIS_NONE, bin2 = -1;
    char *spec;
    Tcl_Obj **listObjElem;
    int nxbins = 1, nybins = 1, nzbins = 1, maxslicesize = 1;
    float *mem = NULL, *data = NULL, *poserr = NULL, *negerr = NULL;
    Tcl_Obj *result, *bstring;
    int parent_dim;

    tcl_objc_range(5, 7);
    if (Tcl_GetIntFromObj(interp, objv[1], &parent_id) != TCL_OK)
	return TCL_ERROR;
    parent_type = hs_type(parent_id);
    if (parent_type == HS_2D_HISTOGRAM)
	hs_2d_hist_num_bins(parent_id, &nxbins, &nybins);
    else if (parent_type == HS_3D_HISTOGRAM)
	hs_3d_hist_num_bins(parent_id, &nxbins, &nybins, &nzbins);
    else
    {
	if (parent_type == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a valid Histo-Scope id", NULL);
	else
	    Tcl_AppendResult(interp, "item with id ", 
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " can not be sliced", NULL);
	return TCL_ERROR;
    }
    parent_dim = histo_dim_from_type(parent_type);
    if (get_axis_from_obj(interp, objv[3], parent_dim, 1, &axis1) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[4], &bin1) != TCL_OK)
	return TCL_ERROR;
    if (objc > 5)
    {
	tcl_require_objc(7);
	if (get_axis_from_obj(interp, objv[5], parent_dim, 0, &axis2) != TCL_OK)
	    return TCL_ERROR;
	if (Tcl_GetIntFromObj(interp, objv[6], &bin2) != TCL_OK)
	    return TCL_ERROR;
    }
    if (parent_type == HS_2D_HISTOGRAM && axis2 != HS_AXIS_NONE)
    {
	Tcl_SetResult(interp, "more than one slice axis specified "
		      "for a 2d histogram", TCL_STATIC);
	return TCL_ERROR;
    }
    if (axis1 == axis2)
    {
	Tcl_SetResult(interp,
		      "can not specify the same slice axis more than once",
		      TCL_STATIC);
	return TCL_ERROR;
    }

    /* Figure out the maximum slice size */
    if (parent_type == HS_3D_HISTOGRAM)
	maxslicesize = nxbins*nybins;
    if (nxbins*nzbins > maxslicesize)
	maxslicesize = nxbins*nzbins;
    if (nybins*nzbins > maxslicesize)
	maxslicesize = nybins*nzbins;

    /* Allocate the memory */
    mem = (float *)malloc(3*maxslicesize*sizeof(float));
    if (mem == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }

    /* Check which of data, positive, and negative errors are requested */
    if (Tcl_ListObjGetElements(interp, objv[2], &listlen, &listObjElem) != TCL_OK)
	return TCL_ERROR;
    for (i=0; i<listlen; ++i)
    {
	spec = Tcl_GetStringFromObj(listObjElem[i], NULL);
	if (strcasecmp(spec, "d") == 0)
	    data = mem;
	else if (strcasecmp(spec, "n") == 0)
	    negerr = mem + 2*maxslicesize;
	else if (strcasecmp(spec, "p") == 0)
	    poserr = mem + maxslicesize;
	else
	{
	    Tcl_AppendResult(interp, "bad result type specifier \"", 
			     spec, "\", expected d, n, or p", NULL);
	    goto fail;
	}
    }

    /* Run the slicer code */
    status = hs_slice_contents(parent_id, axis1, bin1, axis2, bin2,
			       maxslicesize, data, poserr, negerr, &nfilled);
    assert(nfilled <= maxslicesize);
    if (status == -4)
    {
	Tcl_SetResult(interp, "bin number out of range", TCL_STATIC);
	goto fail;
    }
    else if (status < 0)
    {
	Tcl_SetResult(interp,
		      "operation failed, check stderr for diagnostics",
		      TCL_STATIC);
	goto fail;
    }
    else
    {
	assert(status == HS_NO_ERRORS ||
	       status == HS_POS_ERRORS ||
	       status == HS_BOTH_ERRORS);
    }

    /* Form the result */
    result = Tcl_NewListObj(0, NULL);
    for (i=0; i<listlen; ++i)
    {
	spec = Tcl_GetStringFromObj(listObjElem[i], NULL);
	if (strcasecmp(spec, "d") == 0)
	{
	    bstring = Tcl_NewByteArrayObj((unsigned char *)(data),
					  nfilled*sizeof(float));
	}
	else if (strcasecmp(spec, "p") == 0)
	{
	    if (status != HS_NO_ERRORS)
		bstring = Tcl_NewByteArrayObj((unsigned char *)(poserr),
					      nfilled*sizeof(float));
	    else
		bstring = Tcl_NewByteArrayObj(NULL, 0);
	}
	else if (strcasecmp(spec, "n") == 0)
	{
	    if (status == HS_BOTH_ERRORS)
		bstring = Tcl_NewByteArrayObj((unsigned char *)(negerr),
					      nfilled*sizeof(float));
	    else
		bstring = Tcl_NewByteArrayObj(NULL, 0);
	}
	else
	    assert(0);
	if (bstring == NULL)
	{
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    goto fail;
	}
	Tcl_ListObjAppendElement(interp, result, bstring);
    }

    Tcl_SetObjResult(interp, result);
    free(mem);
    return TCL_OK;

 fail:
    if (mem)
	free(mem);
    return TCL_ERROR;
}

tcl_routine(swap_data_errors)
{
    int type, id, errtype;
    char *err;
    
    tcl_require_objc(3);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
	return TCL_ERROR;
    type = hs_type(id);
    if (type != HS_1D_HISTOGRAM &&
	type != HS_2D_HISTOGRAM &&
	type != HS_3D_HISTOGRAM)
    {
	if (type == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a valid Histo-Scope id", NULL);
	else
	    Tcl_AppendResult(interp, "item with id ", 
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a histogram", NULL);
	return TCL_ERROR;
    }
    err = Tcl_GetStringFromObj(objv[2],NULL);
    if (strncmp(err, "pos", 3) == 0)
	errtype = HS_POS_ERRORS;
    else if (strncmp(err, "neg", 3) == 0)
	errtype = HS_BOTH_ERRORS;
    else
    {
	Tcl_SetResult(interp, "invalid error type argument", TCL_STATIC);
	return TCL_ERROR;
    }
    hs_swap_data_errors(id, errtype);
    return TCL_OK;
}

tcl_routine(tcl_api_version)
{
    tcl_require_objc(1);
    Tcl_AppendElement(interp, HS_VERSION);
#ifndef FIXED_HISTOSCOPE
    Tcl_AppendElement(interp, "original");
#endif
    return TCL_OK;
}

tcl_routine(lookup_command_info)
{
    int i, len, found = 0;
    char *scommand, *filename, *s, *command;
    FILE *fp;
    fpos_t pos;
    extern int errno;
    char buffer[256], lookup[256];
    const int lookuplines = 5;
    static Tcl_DString *answer = NULL;

    if (answer)
    {
	Tcl_DStringFree(answer);
    }
    else
    {
	answer = (Tcl_DString *)malloc(sizeof(Tcl_DString));
	if (answer == NULL)
	{
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    return TCL_ERROR;
	}
	Tcl_DStringInit(answer);
    }

    tcl_require_objc(3);
    scommand = Tcl_GetStringFromObj(objv[1], NULL);
    if (strncmp("::hs::", scommand, 6) == 0)
	s = scommand+6;
    else if (strncmp("hs::", scommand, 4) == 0)
	s = scommand+4;
    else
	s = scommand;
    filename = Tcl_GetStringFromObj(objv[2], NULL);
    if ((fp = fopen(filename, "r")) == NULL)
    {
	Tcl_AppendResult(interp, "can't open file ", filename, ": ",
			 strerror(errno), NULL);
	return TCL_ERROR;
    }
    command = (char *)malloc((strlen(s)+10)*sizeof(char));
    if (command == NULL)
    {
	fclose(fp);
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    strcpy(command, "hs::");
    strcat(command, s);
    len = strlen(command);
    while (fgets(buffer, 256, fp))
    {
	if (found == 0)
	{
	    if (strncmp(command, buffer, len) == 0)
	    {
		fgetpos(fp, &pos);
		for (i=0; i<lookuplines; ++i)
		{
		    if (fgets(lookup, 256, fp) == NULL)
			break;
		    for (s=lookup; *s && isspace(*s); ++s);
		    if (strncmp("Arguments", s, 9) == 0)
		    {
			found = 1;
			break;
		    }
		}
		if (found)
		{
		    fsetpos(fp, &pos);
		    Tcl_DStringAppend(answer, "\n", -1);
		    Tcl_DStringAppend(answer, buffer, -1);
		}
	    }
	}
	else
	{
	    if (isspace(buffer[0]))
		Tcl_DStringAppend(answer, buffer, -1);
	    else
		break;
	}
    }
    fclose(fp);
    if (!found)
    {
	Tcl_AppendResult(interp, "can't find description of \"", 
			 command, "\" in file ", filename, NULL);
	free(command);
	return TCL_ERROR;
    }
    Tcl_DStringResult(interp, answer);
    free(command);
    return TCL_OK;
}

tcl_routine(unique_rows)
{
    /* Usage: unique_rows uid title category id varnames */
    int i, j, id, uid, numvars, keylen, nrows, entry_created, keylistlen;
    int *keycolumns = NULL;
    Tcl_HashTable ntuple_toc;
    int new_id = 0, hash_initialized = 0;
    Tcl_Obj **listKeys;
    float *keydata = NULL, *ntuple_data = NULL;

    tcl_require_objc(6);
    verify_ntuple(id,4);
    numvars = hs_num_variables(id);
    if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_ListObjGetElements(interp, objv[5], &keylistlen, &listKeys) != TCL_OK)
	return TCL_ERROR;
    if (keylistlen > 0) {
	keycolumns = (int *)malloc(keylistlen*sizeof(int));
	if (keycolumns == NULL)
	{
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    goto fail;
	}
	for (i=0; i<keylistlen; ++i)
	{
	    char *varname = Tcl_GetStringFromObj(listKeys[i], NULL);
	    keycolumns[i] = hs_variable_index(id, varname);
	    if (keycolumns[i] < 0)
	    {
		Tcl_AppendResult(interp, "ntuple with id ",
				 Tcl_GetStringFromObj(objv[4], NULL),
				 " doesn't have a variable named \"",
				 varname, "\"", NULL);
		goto fail;
	    }
	}
    } else {
	keylistlen = numvars;
    }

    /* Allocate the array for the key data */
    keylen = ntuple_hashtable_key_length(keylistlen);
    if (keylen <= 0)
    {
	Tcl_SetResult(interp, "operation failed", TCL_STATIC);
	goto fail;
    }
    keydata = (float *)calloc(keylen, sizeof(int));
    if (keydata == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }

    /* Duplicate the ntuple structure */
    new_id = hs_duplicate_ntuple_header(
	id, uid, Tcl_GetStringFromObj(objv[2], NULL), 
	Tcl_GetStringFromObj(objv[3], NULL));
    if (new_id <= 0)
    {
	Tcl_SetResult(interp, "failed to duplicate ntuple header",
		      TCL_STATIC);
	goto fail;
    }

    /* Go over the ntuple data */
    if (keycolumns == NULL)
	ntuple_data = keydata;
    else
    {
	ntuple_data = (float *)malloc(numvars*sizeof(float));
	if (ntuple_data == NULL)
	{
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    goto fail;
	}
    }
    nrows = hs_num_entries(id);
    Tcl_InitHashTable(&ntuple_toc, keylen);
    hash_initialized = 1;
    for (i=0; i<nrows; ++i)
    {
	hs_row_contents(id, i, ntuple_data);
	if (keycolumns)
	    for (j=0; j<keylistlen; ++j)
		keydata[j] = ntuple_data[keycolumns[j]];
        Tcl_CreateHashEntry(&ntuple_toc, (char *)keydata, &entry_created);
	if (entry_created)
	    if (hs_fill_ntuple(new_id, ntuple_data) <= 0)
	    {
		Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
		goto fail;
	    }
    }

    Tcl_DeleteHashTable(&ntuple_toc);
    free(keydata);
    if (ntuple_data != keydata)
	free(ntuple_data);
    if (keycolumns)
	free(keycolumns);
    Tcl_SetObjResult(interp, Tcl_NewIntObj(new_id));
    return TCL_OK;

 fail:
    if (keydata)
	free(keydata);
    if (ntuple_data != keydata && ntuple_data != NULL)
	free(ntuple_data);
    if (keycolumns)
	free(keycolumns);
    if (new_id > 0)
	hs_delete(new_id);
    if (hash_initialized)
	Tcl_DeleteHashTable(&ntuple_toc);
    return TCL_ERROR;
}

tcl_routine(join_entries)
{
    /* Usage: hs_join_entries uid title category id1 id2 \
     *          keymatchlist varmatchlist1 varmatchlist2
     */
    int i, j, uid, id1, id2, keylistlen, keylen, varlen1, varlen2, keyType;
    Tcl_Obj **listKeys, **match1, **match2;
    float key[NTUPLE_MAXKEYLEN+4];
    int keycolumns1[NTUPLE_MAXKEYLEN], keycolumns2[NTUPLE_MAXKEYLEN];
    int *datacolumns1 = NULL, *datacolumns2;
    float *data = NULL;
    char **resultnames = NULL;
    int row2, usecolumns1, usecolumns2, datalen, nevents, entry_created;
    Tcl_HashTable ntuple_toc;
    int result_id = 0, hash_initialized = 0, status = TCL_ERROR;
    Tcl_HashEntry *hashEntry;

    tcl_require_objc(9);
    if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
	return TCL_ERROR;
    verify_ntuple(id1,4);
    verify_ntuple(id2,5);
    if (Tcl_ListObjGetElements(interp, objv[6], &keylistlen, &listKeys) != TCL_OK)
	return TCL_ERROR;
    if (keylistlen == 0 || keylistlen % 2 == 1)
    {
	Tcl_SetResult(interp,
		      "Bad number of elements in the list of keys",
		      TCL_STATIC);
	return TCL_ERROR;
    }
    keylen = keylistlen/2;
    if (keylen > NTUPLE_MAXKEYLEN)
    {
	Tcl_SetResult(interp,
		      "Too many elements in the list of keys",
		      TCL_STATIC);
	return TCL_ERROR;
    }

    /* Check the validity of specified key names */
    for (i=0; i<keylen; ++i)
    {
	keycolumns1[i] = hs_variable_index(
	    id1, Tcl_GetStringFromObj(listKeys[2*i], NULL));
	if (keycolumns1[i] < 0)
	{
	    Tcl_AppendResult(interp, "ntuple with id ",
			     Tcl_GetStringFromObj(objv[4], NULL),
			     " doesn't have a variable named \"",
			     Tcl_GetStringFromObj(listKeys[2*i], NULL),
			     "\"", NULL);
	    return TCL_ERROR;
	}
	keycolumns2[i] = hs_variable_index(
	    id2, Tcl_GetStringFromObj(listKeys[2*i+1], NULL));
	if (keycolumns2[i] < 0)
	{
	    Tcl_AppendResult(interp, "ntuple with id ",
			     Tcl_GetStringFromObj(objv[5], NULL),
			     " doesn't have a variable named \"",
			     Tcl_GetStringFromObj(listKeys[2*i+1], NULL),
			     "\"", NULL);
	    return TCL_ERROR;
	}
    }
    if (Tcl_ListObjGetElements(interp, objv[7], &varlen1, &match1) != TCL_OK)
	return TCL_ERROR;
    if (varlen1 % 2 == 1)
    {
	Tcl_SetResult(interp,
		      "Bad number of elements in the first list of variables",
		      TCL_STATIC);
	return TCL_ERROR;
    }
    usecolumns1 = varlen1/2;
    if (Tcl_ListObjGetElements(interp, objv[8], &varlen2, &match2) != TCL_OK)
	return TCL_ERROR;
    if (varlen2 % 2 == 1)
    {
	Tcl_SetResult(interp,
		      "Bad number of elements in the second list of variables",
		      TCL_STATIC);
	return TCL_ERROR;
    }
    usecolumns2 = varlen2/2;
    datalen = usecolumns1 + usecolumns2;
    if (datalen == 0)
    {
	Tcl_SetResult(interp,
		      "No variables specified for the resulting ntuple",
		      TCL_STATIC);
	return TCL_ERROR;
    }

    /* Check the validity of specified variable names */
    datacolumns1 = (int *)malloc(datalen*sizeof(int));
    resultnames = (char **)malloc(datalen*sizeof(char *));
    data = (float *)malloc(datalen*sizeof(float));
    if (datacolumns1 == NULL || resultnames == NULL || data == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }
    datacolumns2 = datacolumns1 + usecolumns1;
    for (i=0; i<usecolumns1; ++i)
    {
	char *varname = Tcl_GetStringFromObj(match1[2*i], NULL);
	datacolumns1[i] = hs_variable_index(id1, varname);
	if (datacolumns1[i] < 0)
	{
	    Tcl_AppendResult(interp, "ntuple with id ",
			     Tcl_GetStringFromObj(objv[4], NULL),
			     " doesn't have a variable named \"",
			     varname, "\"", NULL);
	    goto fail;
	}
	resultnames[i] = Tcl_GetStringFromObj(match1[2*i+1], NULL);
	if (*resultnames[i] == '\0')
	{
	    Tcl_SetResult(interp,
			  "ntuple variable name can not be an empty string",
			  TCL_STATIC);
	    goto fail;
	}
    }
    for (i=0; i<usecolumns2; ++i)
    {
	char *varname = Tcl_GetStringFromObj(match2[2*i], NULL);
	datacolumns2[i] = hs_variable_index(id2, varname);
	if (datacolumns2[i] < 0)
	{
	    Tcl_AppendResult(interp, "ntuple with id ",
			     Tcl_GetStringFromObj(objv[5], NULL),
			     " doesn't have a variable named \"",
			     varname, "\"", NULL);
	    goto fail;
	}
	resultnames[i+usecolumns1] = Tcl_GetStringFromObj(match2[2*i+1], NULL);
	if (*resultnames[i+usecolumns1] == '\0')
	{
	    Tcl_SetResult(interp,
			  "ntuple variable name can not be an empty string",
			  TCL_STATIC);
	    goto fail;
	}
    }
    for (i=0; i<datalen-1; ++i)
	for (j=i+1; j<datalen; ++j)
	    if (strcmp(resultnames[i], resultnames[j]) == 0)
	    {
		Tcl_SetResult(interp,
			      "duplicate ntuple variable name",
			      TCL_STATIC);
		goto fail;
	    }

    /* Build the table of contents for the second ntuple */
    memset(key, 0, sizeof(key));
    keyType = ntuple_hashtable_key_length(keylen);
    if (keyType <= 0)
    {
	Tcl_SetResult(interp, "operation failed",
		      TCL_STATIC);
	goto fail;
    }
    Tcl_InitHashTable(&ntuple_toc, keyType);
    hash_initialized = 1;
    nevents = hs_num_entries(id2);
    for (i=0; i<nevents; ++i)
    {
	for (j=0; j<keylen; ++j)
	    key[j] = hs_ntuple_value(id2, i, keycolumns2[j]);
	hashEntry = Tcl_CreateHashEntry(&ntuple_toc, (char *)key, &entry_created);
	if (!entry_created)
	{
	    char buf[40];
	    sprintf(buf, "%d", i);
	    sprintf(buf+20, "%d", id2);
	    Tcl_AppendResult(interp, "duplicate key in row ",
			     buf, " of ntuple with id ", buf+20, NULL);
	    goto fail;
	}
	if (hashEntry == NULL)
	{
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    goto fail;
	}
	Tcl_SetHashValue(hashEntry, (ClientData)((long)i));
    }

    /* Create the result ntuple */
    result_id = hs_create_ntuple(uid,
				 Tcl_GetStringFromObj(objv[2], NULL),
				 Tcl_GetStringFromObj(objv[3], NULL),
				 datalen, resultnames);
    if (result_id <= 0)
    {
	Tcl_AppendResult(interp, "failed to create ntuple with uid ",
			 Tcl_GetStringFromObj(objv[1], NULL),
			 ", title \"", Tcl_GetStringFromObj(objv[2], NULL),
			 "\", and category \"",
			 Tcl_GetStringFromObj(objv[3], NULL), "\"", NULL);
	goto fail;
    }

    /* Go over the rows of the first ntuple and perform the join */
    nevents = hs_num_entries(id1);
    for (i=0; i<nevents; ++i)
    {
	for (j=0; j<keylen; ++j)
	    key[j] = hs_ntuple_value(id1, i, keycolumns1[j]);
	hashEntry = Tcl_FindHashEntry(&ntuple_toc, (char *)key);
	if (hashEntry == NULL)
	    continue;
	row2 = (long)Tcl_GetHashValue(hashEntry);
	for (j=0; j<usecolumns1; ++j)
	    data[j] = hs_ntuple_value(id1, i, datacolumns1[j]);
	for (j=0; j<usecolumns2; ++j)
	    data[j+usecolumns1] = hs_ntuple_value(id2, row2, datacolumns2[j]);
	if (hs_fill_ntuple(result_id, data) != result_id)
	{
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    goto fail;
	}
    }

    Tcl_SetObjResult(interp, Tcl_NewIntObj(result_id));
    status = TCL_OK;

 fail:
    if (datacolumns1)
	free(datacolumns1);
    if (data)
	free(data);
    if (resultnames)
	free(resultnames);
    if (hash_initialized)
	Tcl_DeleteHashTable(&ntuple_toc);
    if (result_id > 0 && status != TCL_OK)
	hs_delete(result_id);
    return status;
}

tcl_routine(pack_ntuple_row)
{
    int id, row, size;
    float *data;
    const int key = 0x01020304;

    tcl_require_objc(3);
    if (sizeof(float) != 4)
    {
        Tcl_SetResult(interp, "floats are not 4 bytes wide", TCL_STATIC);
        return TCL_ERROR;
    }
    if (sizeof(int) != 4)
    {
        Tcl_SetResult(interp, "integers are not 4 bytes wide", TCL_STATIC);
        return TCL_ERROR;
    }
    verify_ntuple(id,1);
    if (Tcl_GetIntFromObj(interp, objv[2], &row) != TCL_OK)
        return TCL_ERROR;
    if (row < 0 || row >= hs_num_entries(id))
    {
        Tcl_SetResult(interp, "row number is out of range", TCL_STATIC);
        return TCL_ERROR;
    }
    size = hs_num_variables(id);
    if ((data = (float *)malloc((size+1)*sizeof(float))) == NULL)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        return TCL_ERROR;
    }
    *((int *)data) = key;
    hs_row_contents(id, row, data+1);
    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((unsigned char *)data,
                                                 (size+1)*sizeof(float)));
    free(data);
    return TCL_OK;
}

tcl_routine(unpack_ntuple_row)
{
    int id, ncols, size;
    int *bincontents;
    float *data;
    const int key = 0x01020304;
    const int reverse_key = 0x04030201;

    tcl_require_objc(3);
    if (sizeof(float) != 4)
    {
        Tcl_SetResult(interp, "floats are not 4 bytes wide", TCL_STATIC);
        return TCL_ERROR;
    }
    if (sizeof(int) != 4)
    {
        Tcl_SetResult(interp, "integers are not 4 bytes wide", TCL_STATIC);
        return TCL_ERROR;
    }
    verify_ntuple(id,1);
    ncols = hs_num_variables(id);
    bincontents = (int *)Tcl_GetByteArrayFromObj(objv[2], &size);
    size /= 4;
    if (size != ncols+1)
    {
        Tcl_SetResult(interp, "argument size is incompatible "
                      "with ntuple dimension", TCL_STATIC);
        return TCL_ERROR;
    }
    if (bincontents[0] == key)
    {
        ; /* Do not need to do anything */
    }
    else if (bincontents[0] == reverse_key)
    {
        /* Reverse the byte order */
        int i;
        for (i=0; i<size; ++i)
        {
            register int datum = bincontents[i];
            bincontents[i] = flip_endian(datum);
        }
    }
    else
    {
        Tcl_SetResult(interp, "encoding is not recognized", TCL_STATIC);
        return TCL_ERROR;
    }
    data = (float *)bincontents+1;
    if (hs_fill_ntuple(id, data) != id)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        return TCL_ERROR;
    }
    return TCL_OK;
}

tcl_routine(Precise_reference_chisq)
{
    /* Usage: Precise_reference_chisq id_histo id_reference
     *
     * Reference histogram errors are assumed to be negligible.
     */
    int i, id, ref_id, type, estat, nbins, ndf = 0;
    int nbinx = 1, nbiny = 1, nbinz = 1;
    int ref_nbinx = 1, ref_nbiny = 1, ref_nbinz = 1;
    float *mem = NULL;
    float *data, *poserr, *negerr, *refdata;
    double cl, r, sum = 0.0;
    Tcl_Obj *result[3];

    tcl_require_objc(3);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
        return TCL_ERROR;
    type = hs_type(id);
    if (type != HS_1D_HISTOGRAM && 
        type != HS_2D_HISTOGRAM &&
        type != HS_3D_HISTOGRAM)
    {
        if (type == HS_NONE)
            Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
                             " is not a valid Histo-Scope id", NULL);
        else
            Tcl_AppendResult(interp, "item with id ", 
                             Tcl_GetStringFromObj(objv[1], NULL),
                             " is not a histogram", NULL);
        return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, objv[2], &ref_id) != TCL_OK)
        return TCL_ERROR;
    if (type != hs_type(ref_id))
    {
        if (hs_type(ref_id) == HS_NONE)
            Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[2], NULL),
                             " is not a valid Histo-Scope id", NULL);
        else
            Tcl_AppendResult(interp, "items with ids ",
                             Tcl_GetStringFromObj(objv[1], NULL),
                             " and ",
                             Tcl_GetStringFromObj(objv[2], NULL),
                             " have different types", NULL);
        return TCL_ERROR;
    }
    estat = hs_hist_error_status(id);
    if (estat == HS_NO_ERRORS)
    {
	Tcl_AppendResult(interp, "histogram with id ", 
			 Tcl_GetStringFromObj(objv[1], NULL),
			 " does not have errors", NULL);
	return TCL_ERROR;
    }
    assert(estat == HS_POS_ERRORS || estat == HS_BOTH_ERRORS);

    /* Check compatibility of bin structures */
    switch (type)
    {
    case HS_1D_HISTOGRAM:
        nbinx = hs_1d_hist_num_bins(id);
        ref_nbinx = hs_1d_hist_num_bins(ref_id);
        break;
    case HS_2D_HISTOGRAM:
        hs_2d_hist_num_bins(id, &nbinx, &nbiny);
        hs_2d_hist_num_bins(ref_id, &ref_nbinx, &ref_nbiny);
        break;
    case HS_3D_HISTOGRAM:
        hs_3d_hist_num_bins(id, &nbinx, &nbiny, &nbinz);
        hs_3d_hist_num_bins(ref_id, &ref_nbinx, &ref_nbiny, &ref_nbinz);
        break;
    default:
        assert(0);        
    }
    if (nbinx != ref_nbinx ||
        nbiny != ref_nbiny ||
        nbinz != ref_nbinz)
    {
        Tcl_AppendResult(interp, "histograms with ids ",
                         Tcl_GetStringFromObj(objv[1], NULL),
                         " and ",
                         Tcl_GetStringFromObj(objv[2], NULL),
                         " are not bin compatible", NULL);
        return TCL_ERROR;
    }

    nbins = nbinx*nbiny*nbinz;
    mem = (float *)malloc(4*nbins*sizeof(float));
    if (mem == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }    
    data = mem;
    poserr = data + nbins;
    negerr = poserr + nbins;
    refdata = negerr + nbins;
    switch (type)
    {
    case HS_1D_HISTOGRAM:
        hs_1d_hist_bin_contents(id, data);        
        hs_1d_hist_bin_contents(ref_id, refdata);
	hs_1d_hist_errors(id, poserr, negerr);
        break;
    case HS_2D_HISTOGRAM:
        hs_2d_hist_bin_contents(id, data);
        hs_2d_hist_bin_contents(ref_id, refdata);
	hs_2d_hist_errors(id, poserr, negerr);
        break;
    case HS_3D_HISTOGRAM:
        hs_3d_hist_bin_contents(id, data);
        hs_3d_hist_bin_contents(ref_id, refdata);
	hs_3d_hist_errors(id, poserr, negerr);
        break;
    default:
        assert(0);        
    }
    if (estat == HS_POS_ERRORS)
        negerr = poserr;
    for (i=0; i<nbins; ++i)
    {
        r = 0.0;
        if (data[i] < refdata[i])
        {
            if (poserr[i] > 0.0)
            {
                ++ndf;
                r = (data[i] - refdata[i])/poserr[i];
            }
        }
        else if (data[i] > refdata[i])
        {
            if (negerr[i] > 0.0)
            {
                ++ndf;
                r = (data[i] - refdata[i])/negerr[i];
            }
        }
        else
        {
            if (poserr[i] > 0.0 || negerr[i] > 0.0)
                ++ndf;
        }
        sum += r*r;
    }
    free(mem);
    if (ndf > 0)
    {
        float fsum = (float)sum;
        int iii = ndf;
        cl = prob_(&fsum, &iii);
    }
    else
    {        
	Tcl_AppendResult(interp, "histogram with id ", 
			 Tcl_GetStringFromObj(objv[1], NULL),
			 " has null errors", NULL);
        return TCL_ERROR;
    }
    result[0] = Tcl_NewIntObj(ndf);
    result[1] = Tcl_NewDoubleObj(sum);
    result[2] = Tcl_NewDoubleObj(cl);
    Tcl_SetObjResult(interp, Tcl_NewListObj(3, result));
    return TCL_OK;
}

tcl_routine(pick_random_rows)
{
    /* Usage: hs::pick_random_rows id nsplit uid1 title1 category1
     *                                     uid2? title2? category2?
     */
    int id, id1 = -1, id2 = -1, nsplit, uid1, uid2 = -1, npoints;
    int i, isel, count, countmax, fill1, fill2, idfill;
    char *title1, *category1, *title2 = NULL, *category2 = NULL;
    int *sets = NULL;
    float fpoints, *rands = NULL, *row_data = NULL;

    if (objc != 6 && objc != 9)
    {
        Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[0],NULL),
                         " : wrong # of arguments", NULL);
        return TCL_ERROR;
    }
    verify_ntuple(id,1);
    npoints = hs_num_entries(id);
    if (npoints == 0)
    {
        Tcl_AppendResult(interp, "ntuple with id ",
                         Tcl_GetStringFromObj(objv[1], NULL),
                         " is empty", NULL);
        return TCL_ERROR;
    }
    if (npoints > 0.1/FLT_EPSILON)
    {
        Tcl_SetResult(interp, "too many rows in the ntuple, can't split"
                      " reliably due to limitations of the RNG used",
                      TCL_STATIC);
        return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, objv[2], &nsplit) != TCL_OK)
        return TCL_ERROR;
    if (nsplit < 0 || nsplit > npoints)
    {
        Tcl_SetResult(interp, "number of rows in the split is out of range",
                      TCL_STATIC);
        return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, objv[3], &uid1) != TCL_OK)
        return TCL_ERROR;
    title1 = Tcl_GetStringFromObj(objv[4], NULL);
    category1 = Tcl_GetStringFromObj(objv[5], NULL);
    if (objc == 9)
    {
        if (Tcl_GetIntFromObj(interp, objv[6], &uid2) != TCL_OK)
            return TCL_ERROR;
        title2 = Tcl_GetStringFromObj(objv[7], NULL);
        category2 = Tcl_GetStringFromObj(objv[8], NULL);
    }
    id1 = hs_duplicate_ntuple_header(id, uid1, title1, category1);
    if (id1 <= 0)
    {
	Tcl_SetResult(interp, "failed to create new ntuple", TCL_STATIC);
	goto fail;
    }
    if (objc == 9)
    {
        id2 = hs_duplicate_ntuple_header(id, uid2, title2, category2);
        if (id2 <= 0)
        {
            Tcl_SetResult(interp, "failed to create new ntuple", TCL_STATIC);
            goto fail;
        }
    }
    sets = (int *)malloc(npoints*sizeof(int));
    if (sets == NULL)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        goto fail;
    }
    memset(sets, 0, npoints*sizeof(int));
    if (npoints - nsplit < nsplit)
    {
        fill1 = id2;
        fill2 = id1;
        countmax = npoints - nsplit;
    }
    else
    {
        fill1 = id1;
        fill2 = id2;
        countmax = nsplit;
    }
    rands = (float *)malloc((2*countmax + 1)*sizeof(float));
    if (rands == NULL)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        goto fail;
    }
    count = 0;
    fpoints = (float)npoints;
    while (count < countmax)
    {
        int N = 2*countmax + 1;
        ranlux_(rands, &N);
        N = 2*countmax + 1;
        for (i=0; i<N && count<countmax; ++i)
        {
            isel = (int)(rands[i]*fpoints);
            if (isel < npoints)
                if (!sets[isel])
                {
                    sets[isel] = 1;
                    ++count;
                }
        }
    }
    free(rands);
    rands = NULL;
    row_data = (float *)malloc(hs_num_variables(id) * sizeof(float));
    if (row_data == NULL)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        goto fail;
    }
    for (i=0; i<npoints; ++i)
    {
        idfill = sets[i] ? fill1 : fill2;
        if (idfill > 0)
        {
            hs_row_contents(id, i, row_data);
            if (hs_fill_ntuple(idfill, row_data) != idfill)
            {
                Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
                goto fail;
            }
        }
    }
    assert(hs_num_entries(id1) == nsplit);

    if (row_data)
        free(row_data);
    if (rands)
        free(rands);
    if (sets)
        free(sets);

    if (objc == 9)
    {
        Tcl_Obj *result[2];
        result[0] = Tcl_NewIntObj(id1);
        result[1] = Tcl_NewIntObj(id2);
        Tcl_SetObjResult(interp, Tcl_NewListObj(2, result));
    }
    else
        Tcl_SetObjResult(interp, Tcl_NewIntObj(id1));
    return TCL_OK;

 fail:
    if (row_data)
        free(row_data);
    if (rands)
        free(rands);
    if (sets)
        free(sets);
    if (id2 > 0)
        hs_delete(id2);
    if (id1 > 0)
        hs_delete(id1);
    return TCL_ERROR;
}

tcl_routine(weighted_unbinned_ks_test)
{
    /* Usage: hs::weighted_unbinned_ks_test ntuple_id_1 x_var1 w_var1 n_eff_1
     *                                      ntuple_id_2 x_var2 w_var2 n_eff_2
     *
     * Weight variables may be specified as empty strings. Unit weight
     * is assumed in this case.
     */
    char *xvar1, *xvar2, *wvar1, *wvar2;
    int i, id1, id2, xcol1, xcol2, wcol1, wcol2, n1, n2, neff1, neff2;
    float d, kslevel;
    float *data = NULL;
    struct weighted_point *wp1 = NULL, *wp2;
    Tcl_Obj *result[2];

    tcl_require_objc(9);

    verify_ntuple(id1,1);
    xvar1 = Tcl_GetStringFromObj(objv[2], NULL);
    xcol1 = hs_variable_index(id1, xvar1);
    if (xcol1 < 0)
    {
	Tcl_AppendResult(interp, "Ntuple with id ",
			 Tcl_GetStringFromObj(objv[1], NULL),
			 " does not have the \"", xvar1,
			 "\" variable", NULL);
	return TCL_ERROR;
    }
    wvar1 = Tcl_GetStringFromObj(objv[3], NULL);
    if (strcmp(wvar1, ""))
    {
        wcol1 = hs_variable_index(id1, wvar1);
        if (wcol1 < 0)
        {
            Tcl_AppendResult(interp, "Ntuple with id ",
                             Tcl_GetStringFromObj(objv[1], NULL),
                             " does not have the \"", wvar1,
                             "\" variable", NULL);
            return TCL_ERROR;
        }
    }
    else
        wcol1 = -1;
    n1 = hs_num_entries(id1);
    if (n1 == 0)
    {
	Tcl_AppendResult(interp, "Ntuple with id ",
			 Tcl_GetStringFromObj(objv[1], NULL),
			 " is empty", NULL);
	return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, objv[4], &neff1) != TCL_OK)
        return TCL_ERROR;
    if (neff1 <= 0)
    {
        Tcl_SetResult(interp, "effective number of events must be positive",
                      TCL_STATIC);
        return TCL_ERROR;
    }

    verify_ntuple(id2,5);
    xvar2 = Tcl_GetStringFromObj(objv[6], NULL);
    xcol2 = hs_variable_index(id2, xvar2);
    if (xcol2 < 0)
    {
	Tcl_AppendResult(interp, "Ntuple with id ",
			 Tcl_GetStringFromObj(objv[5], NULL),
			 " does not have the \"", xvar2,
			 "\" variable", NULL);
	return TCL_ERROR;
    }
    wvar2 = Tcl_GetStringFromObj(objv[7], NULL);
    if (strcmp(wvar2, ""))
    {
        wcol2 = hs_variable_index(id2, wvar2);
        if (wcol2 < 0)
        {
            Tcl_AppendResult(interp, "Ntuple with id ",
                             Tcl_GetStringFromObj(objv[5], NULL),
                             " does not have the \"", wvar2,
                             "\" variable", NULL);
            return TCL_ERROR;
        }
    }
    else
        wcol2 = -1;
    n2 = hs_num_entries(id2);
    if (n2 == 0)
    {
	Tcl_AppendResult(interp, "Ntuple with id ",
			 Tcl_GetStringFromObj(objv[5], NULL),
			 " is empty", NULL);
	return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, objv[8], &neff2) != TCL_OK)
        return TCL_ERROR;
    if (neff2 <= 0)
    {
        Tcl_SetResult(interp, "effective number of events must be positive",
                      TCL_STATIC);
        return TCL_ERROR;
    }

    data = (float *)malloc((n1 > n2 ? n1 : n2)*sizeof(float));
    if (data == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    wp1 = (struct weighted_point *)malloc((n1 + n2)*sizeof(struct weighted_point));
    if (wp1 == NULL)
    {
        free(data);
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    wp2 = wp1 + n1;

    hs_column_contents(id1, xcol1, data);
    for (i=0; i<n1; ++i)
        wp1[i].x = data[i];
    if (wcol1 >= 0)
    {
        hs_column_contents(id1, wcol1, data);
        for (i=0; i<n1; ++i)
            wp1[i].w = data[i];
    }
    else
    {
        for (i=0; i<n1; ++i)
            wp1[i].w = 1.0;
    }
    hs_column_contents(id2, xcol2, data);
    for (i=0; i<n2; ++i)
        wp2[i].x = data[i];
    if (wcol2 >= 0)
    {
        hs_column_contents(id2, wcol2, data);
        for (i=0; i<n2; ++i)
            wp2[i].w = data[i];
    }
    else
    {
        for (i=0; i<n2; ++i)
            wp2[i].w = 1.0;
    }

    weighted_unbinned_ks_probability(wp1, n1, neff1, wp2, n2, neff2, &d, &kslevel);

    free(wp1);
    free(data);
    result[0] = Tcl_NewDoubleObj((double)kslevel);
    result[1] = Tcl_NewDoubleObj((double)d);
    Tcl_SetObjResult(interp, Tcl_NewListObj(2, result));

    return TCL_OK;
}

tcl_routine(unbinned_ks_test)
{
    /* Usage:  hs::unbinned_ks_test ntuple_id_1 var1 ntuple_id_2 var2 */
    char *var1, *var2;
    int id1, id2, col1, col2, n1, n2;
    float d, kslevel;
    float *data1 = NULL, *data2;
    Tcl_Obj *result[2];

    tcl_require_objc(5);
    verify_ntuple(id1,1);
    var1 = Tcl_GetStringFromObj(objv[2], NULL);
    verify_ntuple(id2,3);
    var2 = Tcl_GetStringFromObj(objv[4], NULL);
    col1 = hs_variable_index(id1, var1);
    if (col1 < 0)
    {
	Tcl_AppendResult(interp, "Ntuple with id ",
			 Tcl_GetStringFromObj(objv[1], NULL),
			 " does not have the \"", var1,
			 "\" variable", NULL);
	return TCL_ERROR;
    }
    col2 = hs_variable_index(id2, var2);
    if (col2 < 0)
    {
	Tcl_AppendResult(interp, "Ntuple with id ",
			 Tcl_GetStringFromObj(objv[3], NULL),
			 " does not have the \"", var2,
			 "\" variable", NULL);
	return TCL_ERROR;
    }
    n1 = hs_num_entries(id1);
    if (n1 == 0)
    {
	Tcl_AppendResult(interp, "Ntuple with id ",
			 Tcl_GetStringFromObj(objv[1], NULL),
			 " is empty", NULL);
	return TCL_ERROR;
    }
    n2 = hs_num_entries(id2);
    if (n2 == 0)
    {
	Tcl_AppendResult(interp, "Ntuple with id ",
			 Tcl_GetStringFromObj(objv[3], NULL),
			 " is empty", NULL);
	return TCL_ERROR;
    }
    data1 = (float *)malloc((n1+n2)*sizeof(float));
    if (data1 == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    data2 = data1 + n1;
    hs_column_contents(id1, col1, data1);
    hs_column_contents(id2, col2, data2);
    classic_unbinned_ks_probability(data1, n1, data2, n2, &d, &kslevel);
    free(data1);
    result[0] = Tcl_NewDoubleObj((double)kslevel);
    result[1] = Tcl_NewDoubleObj((double)d);
    Tcl_SetObjResult(interp, Tcl_NewListObj(2, result));
    return TCL_OK;
}

tcl_routine(uniform_random_fill)
{
    /* Usage:  hs::uniform_random_fill $id $numpoints $weight
     *              $xmin $xmax $ymin $ymax $zmin $zmax
     */
    int i, id, ifill, nfills, buf_points, itype, dim;
    float xmin=0.f, xmax=1.f, ymin=0.f, ymax=1.f, zmin=0.f, zmax=1.f;
    float weight;
    static float *rand_buf = NULL;
    static int rand_buf_size = 3000;
    double dtmp;
    char *ctmp;

    if (rand_buf == NULL)
    {
        rand_buf = (float *)malloc(rand_buf_size*sizeof(float));
        if (rand_buf == NULL)
        {
            Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
            return TCL_ERROR;
        }
    }

    tcl_require_objc(10);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[2], &nfills) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[3], &dtmp) != TCL_OK)
        return TCL_ERROR;
    weight = (float)dtmp;

    itype = hs_type(id);
    switch (itype)
    {
    case HS_1D_HISTOGRAM:
        dim = 1;
        hs_1d_hist_range(id, &xmin, &xmax);
        break;

    case HS_2D_HISTOGRAM:
        dim = 2;
        hs_2d_hist_range(id, &xmin, &xmax, &ymin, &ymax);
        break;

    case HS_3D_HISTOGRAM:
        dim = 3;
        hs_3d_hist_range(id, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
        break;

    case HS_NTUPLE:
        dim = hs_num_variables(id);
        if (dim > rand_buf_size)
        {
            rand_buf_size = dim;
            free(rand_buf);
            rand_buf = (float *)malloc(rand_buf_size*sizeof(float));
            if (rand_buf == NULL)
            {
                Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
                return TCL_ERROR;
            }
        }
        break;

    case HS_NONE:
	Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			 " is not a valid Histo-Scope id", NULL);
	return TCL_ERROR;

    default:
 	Tcl_AppendResult(interp, "item with id ", 
			 Tcl_GetStringFromObj(objv[1], NULL),
			 " is not a histogram or ntuple", NULL);
	return TCL_ERROR;
    }
    if (nfills == 0)
        return TCL_OK;
    if (nfills < 0)
    {
 	Tcl_AppendResult(interp, "expected a non-negative integer, got \"", 
			 Tcl_GetStringFromObj(objv[2], NULL), "\"", NULL);
	return TCL_ERROR;
    }
    buf_points = rand_buf_size/dim;

    /* Parse min and max values */
#define extract_limit(N,lim) do {\
    ctmp = Tcl_GetStringFromObj(objv[N],NULL);\
    if (ctmp[0])\
    {\
        if (Tcl_GetDoubleFromObj(interp, objv[N], &dtmp) != TCL_OK)\
            return TCL_ERROR;\
        lim = (float)dtmp;\
    }\
} while(0);

    extract_limit(4,xmin);
    extract_limit(5,xmax);
    extract_limit(6,ymin);
    extract_limit(7,ymax);
    extract_limit(8,zmin);
    extract_limit(9,zmax);
    xmax -= xmin;
    ymax -= ymin;
    zmax -= zmin;

    /* Fill the item */
    ifill = 0;
    while (ifill < nfills)
    {
        if (buf_points < nfills-ifill)
            i = buf_points*dim;
        else
            i = (nfills-ifill)*dim;
        ranlux_(rand_buf, &i);
        switch (itype)
        {
        case HS_1D_HISTOGRAM:
            for (i=0; i<buf_points && ifill<nfills; ++i, ++ifill)
                hs_fill_1d_hist(id, xmin+rand_buf[i]*xmax, weight);
            break;
        case HS_2D_HISTOGRAM:
            for (i=0; i<buf_points && ifill<nfills; ++i, ++ifill)
                hs_fill_2d_hist(id, xmin+rand_buf[i*2]*xmax,
                                ymin+rand_buf[i*2+1]*ymax, weight);
            break;
        case HS_3D_HISTOGRAM:
            for (i=0; i<buf_points && ifill<nfills; ++i, ++ifill)
                hs_fill_3d_hist(id, xmin+rand_buf[i*3]*xmax,
                                ymin+rand_buf[i*3+1]*ymax,
                                zmin+rand_buf[i*3+2]*zmax, weight);
            break;
        case HS_NTUPLE:
            for (i=0; i<buf_points && ifill<nfills; ++i, ++ifill)
                if (hs_fill_ntuple(id, rand_buf+dim*i) != id)
                {
                    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
                    return TCL_ERROR;
                }
            break;
        default:
            assert(0);
        }
    }
    return TCL_OK;
}

tcl_routine(hist_random)
{
    /* Usage: hist_random $id $npoints */
    int i, j, id, nrandom, ndim, itype, nbins, ix, iy = 0, iz = 0;
    int nxbins = 1, nybins = 1, nzbins = 1;
    double x, y = 0.0, z = 0.0, sum, bwx, bwy, bwz;
    float xmin, xmax, fsum;
    float ymin = 0.f, ymax = 0.f, zmin = 0.f, zmax = 0.f;
    float *data = NULL;
    Tcl_Obj *result = NULL;
    int status = TCL_ERROR;

    tcl_require_objc(3);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[2], &nrandom) != TCL_OK)
        return TCL_ERROR;
    itype = hs_type(id);
    ndim = histo_dim_from_type(itype);
    if (ndim == 0)
    {
	if (itype == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a valid Histo-Scope id", NULL);
	else
	    Tcl_AppendResult(interp, "item with id ",\
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a histogram", NULL);
	return TCL_ERROR;
    }
    if (nrandom < 0)
    {
        Tcl_AppendResult(interp, "expected a non-negative integer, got ",
                         Tcl_GetStringFromObj(objv[2], NULL), NULL);
        return TCL_ERROR;
    }
    if (nrandom == 0)
    {
        Tcl_SetObjResult(interp, Tcl_NewListObj(0, NULL));
        return TCL_OK;
    }
    switch (ndim)
    {
    case 1:
        nxbins = hs_1d_hist_num_bins(id);
        hs_1d_hist_range(id, &xmin, &xmax);
        break;
    case 2:
        hs_2d_hist_num_bins(id, &nxbins,&nybins);
        hs_2d_hist_range(id, &xmin, &xmax, &ymin, &ymax);
        break;
    case 3:
        hs_3d_hist_num_bins(id, &nxbins,&nybins,&nzbins);
        hs_3d_hist_range(id, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
        break;
    default:
        assert(0);
    }
    bwx = (double)(xmax - xmin)/nxbins;
    bwy = (double)(ymax - ymin)/nybins;
    bwz = (double)(zmax - zmin)/nzbins;
    nbins = nxbins*nybins*nzbins;
    data = (float *)malloc(nbins * sizeof(float));
    if (data == NULL)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        goto fail;
    }
    switch (ndim)
    {
    case 1:
        hs_1d_hist_bin_contents(id, data);        
        break;
    case 2:
        hs_2d_hist_bin_contents(id, data);
        break;
    case 3:
        hs_3d_hist_bin_contents(id, data);
        break;
    default:
        assert(0);
    }

    /* Create effective cumulative density */
    sum = 0.0;
    for (i=0; i<nbins; ++i)
    {
	if (data[i] < 0.f)
	{
	    Tcl_AppendResult(interp, "histogram with id ",
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " has negative bin values", NULL);
	    goto fail;
	}
        sum += data[i];
	data[i] = (float)sum;
    }
    if (sum == 0.0)
    {
        Tcl_AppendResult(interp, "histogram with id ",
                         Tcl_GetStringFromObj(objv[1], NULL),
                         " is empty", NULL);
        goto fail;
    }
    fsum = (float)sum;
    for (i=0; i<nbins; ++i)
        data[i] /= fsum;

    /* Generate counts */
    result = Tcl_NewListObj(0, NULL);
    for (i=0; i<nrandom; ++i)
    {
        register float ftmp;
        next_uniform_random(ftmp);
        /* Find such j that data[j-1] < ftmp <= data[j] */
        if (ftmp <= data[0])
            j = 0;
        else
        {
            int jmin, jmax;
            jmin = 0;
            jmax = nbins-1;
            do {
                j = (jmin + jmax)/2;
                if (ftmp <= data[j])
                    jmax = j;
                else
                    jmin = j;
            } while(jmax - jmin > 1);
            j = jmax;
        }
        switch (ndim)
        {
        case 1:
            ix = j;
            break;
        case 2:
            ix = j / nybins;
            iy = j % nybins;
            break;
        case 3:
            ix = j / (nybins*nzbins);
            j -= ix*nybins*nzbins;
            iy = j / nzbins;
            iz = j % nzbins;
            break;
        default:
            assert(0);
        }
        switch (ndim)
        {
        case 3:
            next_uniform_random(ftmp);
            z = zmin + iz*bwz + ftmp*bwz;
        case 2:
            next_uniform_random(ftmp);
            y = ymin + iy*bwy + ftmp*bwy;
        case 1:
            next_uniform_random(ftmp);
            x = xmin + ix*bwx + ftmp*bwx;
            break;
        default:
            assert(0);
        }
        switch (ndim)
        {
        case 1:
            if (Tcl_ListObjAppendElement(interp, result, Tcl_NewDoubleObj(x)) != TCL_OK)
            {
                Tcl_DecrRefCount(result);
                goto fail;
            }
            break;
        case 2:
            if (Tcl_ListObjAppendElement(interp, result, Tcl_NewDoubleObj(x)) != TCL_OK)
            {
                Tcl_DecrRefCount(result);
                goto fail;
            }
            if (Tcl_ListObjAppendElement(interp, result, Tcl_NewDoubleObj(y)) != TCL_OK)
            {
                Tcl_DecrRefCount(result);
                goto fail;
            }
            break;
        case 3:
            if (Tcl_ListObjAppendElement(interp, result, Tcl_NewDoubleObj(x)) != TCL_OK)
            {
                Tcl_DecrRefCount(result);
                goto fail;
            }
            if (Tcl_ListObjAppendElement(interp, result, Tcl_NewDoubleObj(y)) != TCL_OK)
            {
                Tcl_DecrRefCount(result);
                goto fail;
            }
            if (Tcl_ListObjAppendElement(interp, result, Tcl_NewDoubleObj(z)) != TCL_OK)
            {
                Tcl_DecrRefCount(result);
                goto fail;
            }
            break;
        default:
            assert(0);
        }
    }
    Tcl_SetObjResult(interp, result);

    status = TCL_OK;
 fail:
    if (data)
        free(data);
    return status;
}

tcl_routine(poisson_random)
{
    /* Usage: poisson_random $mean npoints? */
    double mean;
    float fmean;
    int N, IERR = 0, npoints = -1;

    tcl_objc_range(2,3);
    if (Tcl_GetDoubleFromObj(interp, objv[1], &mean) != TCL_OK)
        return TCL_ERROR;
    if (mean < 0.0)
    {
        Tcl_SetResult(interp, "negative mean value", TCL_STATIC);
        return TCL_ERROR;
    }
    fmean = (float)mean;
    if (objc > 2)
    {
        if (Tcl_GetIntFromObj(interp, objv[2], &npoints) != TCL_OK)
            return TCL_ERROR;
        if (npoints < 0)
        {
            Tcl_SetResult(interp, "negative number of points", TCL_STATIC);
            return TCL_ERROR;
        }
    }

    if (npoints < 0)
    {
        /* Just one point needed */
        if (fmean == 0.f)
            N = 0;
        else
        {
            rnpssn_(&fmean, &N, &IERR);
            assert(IERR == 0);
        }
        Tcl_SetObjResult(interp, Tcl_NewIntObj(N));
    }
    else
    {
        int i;
        Tcl_Obj *result = Tcl_NewListObj(0, NULL);
        for (i=0; i<npoints; ++i)
        {
            if (fmean == 0.f)
                N = 0;
            else
            {
                rnpssn_(&fmean, &N, &IERR);
                assert(IERR == 0);
            }
            Tcl_ListObjAppendElement(interp, result, Tcl_NewIntObj(N));
        }
        Tcl_SetObjResult(interp, result);        
    }
    return TCL_OK;
}

tcl_routine(multinomial_random)
{
    /* Usage:  hs::multinomial_random nsum list_of_probs
     *
     * Number of elements in the list of probabilities
     * should be smaller by one than the number of elements
     * in the returned list.
     */    
    int NSUM, listlen, N, i, IERR = 0;
    Tcl_Obj *result, **listObjElem;
    static float *probs = NULL;
    static int *generated = NULL;
    static int lastN = 0;

    tcl_require_objc(3);
    if (Tcl_GetIntFromObj(interp, objv[1], &NSUM) != TCL_OK)
        return TCL_ERROR;
    if (NSUM < 0)
    {
        Tcl_SetResult(interp, "random sum must not be negative", TCL_STATIC);
        return TCL_ERROR;
    }
    if (Tcl_ListObjGetElements(interp, objv[2],
                               &listlen, &listObjElem) != TCL_OK)
        return TCL_ERROR;
    if (listlen == 0)
    {
        Tcl_SetResult(interp, "list of probabilities is empty", TCL_STATIC);
        return TCL_ERROR;
    }
    N = listlen + 1;
    if (N > lastN)
    {
        probs = (float *)realloc(probs, N*sizeof(float));
        generated = (int *)realloc(generated, N*sizeof(int));
        if (probs == NULL || generated == NULL)
        {
            if (generated)
            {
                free(generated);
                generated = 0;
            }
            if (probs)
            {
                free(probs);
                probs = 0;
            }
            lastN = 0;
            Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
            return TCL_ERROR;
        }
        lastN = N;
    }
    for (i=0; i<listlen; ++i)
    {
        double d;
        if (Tcl_GetDoubleFromObj(interp, listObjElem[i], &d) != TCL_OK)
            return TCL_ERROR;
        if (d < 0.0)
        {
            Tcl_SetResult(interp, "probability can not be negative", TCL_STATIC);
            return TCL_ERROR;
        }
        probs[i] = (float)d;
    }
    for (i=1; i<listlen; ++i)
        probs[i] += probs[i-1];
    if (probs[listlen-1] > 1.f)
    {
        Tcl_SetResult(interp, "total probability must not exceed 1", TCL_STATIC);
        return TCL_ERROR;
    }
    probs[listlen] = 1.f;
    if (N == 2)
    {
        rnbnml_(&NSUM, probs, generated, &IERR);
        generated[1] = NSUM - generated[0];
    }
    else
        rnmnml_(&N, &NSUM, probs, generated, &IERR);
    if (IERR)
    {
        char buf[32];
        sprintf(buf, "%d", IERR);
        Tcl_AppendResult(interp, "failed, IERR = ", buf, NULL);
        return TCL_ERROR;
    }
    result = Tcl_NewListObj(0, NULL);
    for (i=0; i<N; ++i)
        if (Tcl_ListObjAppendElement(interp, result,
                                     Tcl_NewIntObj(generated[i])) != TCL_OK)
            return TCL_ERROR;
    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

static int fcompare(const float *i, const float *j)
{
  if (*i < *j)
    return -1;
  else if (*i > *j)
    return 1;
  else
    return 0;
}

void task_completion_callback(int connect_id, int task_number,
			      int status, char *result, void *cbdata)
{
    char cmdname[] = "::hs::Task_completion_callback";
    Tcl_Interp *interp = (Tcl_Interp *)cbdata;
    Tcl_Obj *args[5];
    int res, i, n = 0;

    args[n++] = Tcl_NewStringObj(cmdname, -1);
    args[n++] = Tcl_NewIntObj(connect_id);
    args[n++] = Tcl_NewIntObj(task_number);
    args[n++] = Tcl_NewIntObj(status);
    if (result)
	args[n++] = Tcl_NewStringObj(result, -1);
    else
	args[n++] = Tcl_NewStringObj("", -1);
    for (i=0; i<n; ++i)
    {
	if (args[i] == NULL)
	{
	    Tcl_ResetResult(interp);
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    Tcl_BackgroundError(interp);
	    return;
	}	
	Tcl_IncrRefCount(args[i]);
    }
    res = Tcl_EvalObjv(interp, n, args, TCL_EVAL_GLOBAL);
    for (i=n-1; i>=0; --i)
	Tcl_DecrRefCount(args[i]);
    if (res != TCL_OK)
	Tcl_BackgroundError(interp);
}

tcl_routine(Apply_1d_range_masks)
{
    /* Usage:  Apply_1d_range_masks id masklist

       masklist is a list of lists {{min1 max1} {min2 max2} ...}
     */
    int id, i, ilen, listlen, nbins, nminmax, errstat;
    Tcl_Obj **listObjElem, **minmax;
    float *data, *mask, *poserr, *negerr;
    double x, rangemin, rangemax, dmin, dmax, dstep;
    float xmin, xmax;

    tcl_require_objc(3);
    verify_1d_histo(id,1);
    if (Tcl_ListObjGetElements(interp, objv[2], &listlen, &listObjElem) != TCL_OK)
        return TCL_ERROR;
    if (listlen == 0)
    {
        hs_reset(id);
        return TCL_OK;
    }
    nbins = hs_1d_hist_num_bins(id);
    hs_1d_hist_range(id, &xmin, &xmax);
    dmin = (double)xmin;
    dmax = (double)xmax;
    dstep = (dmax - dmin)/(double)nbins;
    dmin += (dstep/2.0);
    data = (float *)malloc(4*nbins*sizeof(float));
    if (data == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    poserr = data + nbins;
    negerr = data + 2*nbins;
    mask   = data + 3*nbins;
    hs_1d_hist_bin_contents(id, data);
    errstat = hs_1d_hist_errors(id, poserr, negerr);
    switch (errstat)
    {
    case HS_NO_ERRORS:
        poserr = NULL;
    case HS_POS_ERRORS:
        negerr = NULL;
    case HS_BOTH_ERRORS:
        break;
    default:
        assert(0);
    }
    memset(mask, 0, nbins*sizeof(float));
    for (ilen=0; ilen<listlen; ++ilen)
    {
        if (Tcl_ListObjGetElements(interp, listObjElem[ilen],
                                   &nminmax, &minmax) != TCL_OK)
            goto fail;
        if (nminmax != 2)
        {
            Tcl_AppendResult(interp, "bad range specification \"",
                             Tcl_GetStringFromObj(listObjElem[ilen],NULL),
                             "\"", NULL);
            goto fail;
        }
        if (Tcl_GetDoubleFromObj(interp, minmax[0], &rangemin) != TCL_OK)
            goto fail;
        if (Tcl_GetDoubleFromObj(interp, minmax[1], &rangemax) != TCL_OK)
            goto fail;
        for (i=0; i<nbins; ++i)
        {
            x = dmin + i*(double)dstep;
            if (rangemin < x && x < rangemax)
                mask[i] = 1.f;
        }
    }
    for (i=0; i<nbins; ++i)
    {
        data[i] *= mask[i];
        if (poserr)
            poserr[i] *= mask[i];
        if (negerr)
            negerr[i] *= mask[i];
    }
    hs_1d_hist_block_fill(id, data, poserr, negerr);
    free(data);
    return TCL_OK;

 fail:
    free(data);
    return TCL_ERROR;
}

tcl_routine(Ntuple_paste)
{
    /* Usage:  Ntuple_paste id_result id1 id2 */
    int id_result, id1, id2, nvars0, nvars1, nvars2;
    int row, nrows0, nrows1, nrows2;
    float *data;

    tcl_require_objc(4);
    verify_ntuple(id_result,1);
    verify_ntuple(id1,2);
    verify_ntuple(id2,3);
    nvars0 = hs_num_variables(id_result);
    nvars1 = hs_num_variables(id1);
    nvars2 = hs_num_variables(id2);
    nrows0 = hs_num_entries(id_result);
    nrows1 = hs_num_entries(id1);
    nrows2 = hs_num_entries(id2);

    assert(nvars0 == nvars1 + nvars2);
    assert(nrows0 == 0);
    assert(nrows1 == nrows2);

    data = (float *)malloc(nvars0*sizeof(float));
    if (data == NULL)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        return TCL_ERROR;
    }
    for (row=0; row<nrows1; ++row)
    {
        hs_row_contents(id1, row, data);
        hs_row_contents(id2, row, data+nvars1);
        if (hs_fill_ntuple(id_result, data) != id_result)
        {
            free(data);
            Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
            return TCL_ERROR;
        }
    }
    free(data);
    return TCL_OK;
}

tcl_routine(Project_ntuple_onto_1d_hist)
{
    /* Usage:  Project_ntuple_onto_1d_hist nt_id varindex histo_id weight */
    int i, nt_id, histo_id, ivar, nrows;
    float *column_contents;
    float fw;
    double weight;

    tcl_require_objc(5);
    verify_ntuple(nt_id,1);
    if (Tcl_GetIntFromObj(interp, objv[2], &ivar) != TCL_OK)
        return TCL_ERROR;
    if (ivar < 0 || ivar >= hs_num_variables(nt_id))
    {
	Tcl_SetResult(interp, "invalid ntuple column number", TCL_STATIC);
	return TCL_ERROR;
    }
    verify_1d_histo(histo_id,3);
    if (Tcl_GetDoubleFromObj(interp, objv[4], &weight) != TCL_OK)
        return TCL_ERROR;
    fw = (float)weight;
    nrows = hs_num_entries(nt_id);
    if (nrows == 0)
        return TCL_OK;
    column_contents = (float *)malloc(nrows*sizeof(float));
    if (column_contents == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    hs_column_contents(nt_id, ivar, column_contents);
    for (i=0; i<nrows; ++i)
        hs_fill_1d_hist(histo_id, column_contents[i], fw);
    free(column_contents);
    return TCL_OK;
}

tcl_routine(Copy_1d_data_with_offset)
{
    /* Usage:  Copy_1d_data_with_offset id1 id2 ratio shift 
       ratio    is the number of bins in the histogram $id2
                used to represent one bin in the histogram $id1.
       shift    is measured in bins of $id2.
    */
    int i, j, index, id1, id2, nbins1, nbins2, ratio, shift;
    float *data1, *data2;

    tcl_require_objc(5);
    verify_1d_histo(id1, 1);
    verify_1d_histo(id2, 2);
    if (Tcl_GetIntFromObj(interp, objv[3], &ratio) != TCL_OK)
	return TCL_ERROR;
    if (ratio < 1)
    {
        Tcl_AppendResult(interp, "expected a positive integer, got \"",
                         Tcl_GetStringFromObj(objv[3], NULL), "\"", NULL);
        return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, objv[4], &shift) != TCL_OK)
	return TCL_ERROR;
    nbins1 = hs_1d_hist_num_bins(id1);
    nbins2 = hs_1d_hist_num_bins(id2);
    if (nbins1*ratio > nbins2)
    {
	Tcl_SetResult(interp, "bin ratio is too large", TCL_STATIC);
	return TCL_ERROR;
    }
    data1 = (float *)malloc((nbins1+nbins2)*sizeof(float));
    if (data1 == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    data2 = data1+nbins1;
    hs_1d_hist_bin_contents(id1, data1);
    memset(data2, 0, nbins2*sizeof(float));
    for (i=0; i<nbins1; ++i)
        for (j=0; j<ratio; ++j)
        {
            index = (i*ratio+j+shift) % nbins2;
            if (index < 0)
                index += nbins2;
            data2[index] = data1[i];
        }
    hs_1d_hist_block_fill(id2, data2, NULL, NULL);
    free(data1);
    return TCL_OK;    
}

tcl_routine(Periodic_gauss)
{
    /* Usage:  Periodic_gauss id area width */
    int i, id, nbins, halfbins;
    double area, width;
    float xmin, xmax;
    double x, sqr2pi, dstep;
    float *data = NULL;

    tcl_require_objc(4);
    verify_1d_histo(id,1);
    if (Tcl_GetDoubleFromObj(interp, objv[2], &area) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[3], &width) != TCL_OK)
        return TCL_ERROR;
    if (width <= 0.0)
    {
        Tcl_AppendResult(interp, "expected a positive double, got \"",
                         Tcl_GetStringFromObj(objv[3],NULL), "\"", NULL);
        return TCL_ERROR;
    }
    nbins = hs_1d_hist_num_bins(id);
    if (nbins % 2)
    {
        Tcl_SetResult(interp, "histogram must have an "
                      "even number of bins", TCL_STATIC);
        return TCL_ERROR;
    }
    halfbins = nbins/2;
    hs_1d_hist_range(id, &xmin, &xmax);
    sqr2pi = sqrt(2.0*atan2(0.0,-1.0));
    dstep = ((double)xmax - (double)xmin)/(double)nbins;
    data = (float *)malloc(nbins*sizeof(float));
    if (data == NULL)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        return TCL_ERROR;
    }
    ++halfbins;
    for (i=0; i<halfbins; ++i)
    {
        x = dstep*(double)i;
        data[i] = (float)(area/sqr2pi/width*exp(-0.5*x/width*x/width));
        if (i)
            data[nbins-i] = data[i];
    }
    hs_1d_hist_block_fill(id, data, NULL, NULL);
    free(data);
    return TCL_OK;
}

tcl_routine(Periodic_uniform_2d)
{
    /* Usage:  Periodic_uniform_2d id height x_halfwidth y_halfwidth */
    int i, j, ibin, jbin, id, x_bins, y_bins, x_halfwidth, y_halfwidth;
    double height;
    float fheight;

    tcl_require_objc(5);
    verify_2d_histo(id,1);
    if (Tcl_GetDoubleFromObj(interp, objv[2], &height) != TCL_OK)
        return TCL_ERROR;
    fheight = (float)height;
    if (Tcl_GetIntFromObj(interp, objv[3], &x_halfwidth) != TCL_OK)
        return TCL_ERROR;
    if (x_halfwidth < 0)
    {
        Tcl_AppendResult(interp, "expected a non-negative integer, got \"",
                         Tcl_GetStringFromObj(objv[3],NULL), "\"", NULL);
        return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, objv[4], &y_halfwidth) != TCL_OK)
        return TCL_ERROR;
    if (y_halfwidth < 0)
    {
        Tcl_AppendResult(interp, "expected a non-negative integer, got \"",
                         Tcl_GetStringFromObj(objv[4],NULL), "\"", NULL);
        return TCL_ERROR;
    }
    hs_2d_hist_num_bins(id, &x_bins, &y_bins);
    hs_reset(id);
    for (i=-x_halfwidth; i<=x_halfwidth; ++i)
        for (j=-y_halfwidth; j<=y_halfwidth; ++j)
        {
            ibin = i;
            jbin = j;
            while (ibin < 0)
                ibin += x_bins;
            while (ibin >= x_bins)
                ibin -= x_bins;
            while (jbin < 0)
                jbin += y_bins;
            while (jbin >= y_bins)
                ibin -= y_bins;
            hs_2d_hist_set_bin(id, ibin, jbin, fheight);
        }
    return TCL_OK;
}

tcl_routine(List_stats)
{
    int i, listlen;
    Tcl_Obj **listObjElem;
    double *data;
    double diff, mean = 0.0, stdev = 0.0;
    Tcl_Obj *res[2];

    tcl_require_objc(2);
    if (Tcl_ListObjGetElements(interp, objv[1], &listlen, &listObjElem) != TCL_OK)
	return TCL_ERROR;
    if (listlen == 0)
    {
	Tcl_SetResult(interp, "empty list as an argument", TCL_STATIC);
	return TCL_ERROR;
    }
    data = (double *)malloc(listlen*sizeof(double));
    if (data == NULL) {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    for (i=0; i<listlen; ++i)
    {
	if (Tcl_GetDoubleFromObj(interp, listObjElem[i], data+i) != TCL_OK)
	{
	    free(data);
	    return TCL_ERROR;
	}
	mean += data[i];
    }
    mean /= listlen;
    if (listlen > 1)
    {
	for (i=0; i<listlen; ++i)
	{
	    diff = data[i] - mean;
	    stdev += diff*diff;
	}
	stdev = sqrt(stdev/(double)(listlen - 1));
    }

    free(data);
    res[0] = Tcl_NewDoubleObj(mean);
    res[1] = Tcl_NewDoubleObj(stdev);
    Tcl_SetObjResult(interp, Tcl_NewListObj(2, res));
    return TCL_OK;
}

tcl_routine(Kernel_density_1d)
{
    /* Usage: kernel_density_1d ntuple_id points_col weights_col
     *     localbw_col kernel_function bw id [xmin xmax npoints]
     */
    int i, status, nbins, itype, nt_id, id, nvars;
    int points_col, localbw_col, weights_col;
    Kernel_function_1d *fcn;
    double bw, dxmin, dxmax, drange, dbins;
    float *result, xmin, xmax, x[2], halfstep;

    tcl_objc_range(8,11);
    verify_ntuple(nt_id,1);
    nvars = hs_num_variables(nt_id);

    /* Column of points */
    if (Tcl_GetIntFromObj(interp, objv[2], &points_col) != TCL_OK)
	return TCL_ERROR;
    if (points_col < 0 || points_col >= nvars)
    {
	Tcl_SetResult(interp, "points column number is out of range",
		      TCL_STATIC);
	return TCL_ERROR;
    }

    /* Column of weights */
    if (Tcl_GetIntFromObj(interp, objv[3], &weights_col) != TCL_OK)
	return TCL_ERROR;
    if (weights_col >= nvars)
    {
	Tcl_SetResult(interp, "weight column number is out of range",
		      TCL_STATIC);
	return TCL_ERROR;
    }

    /* Local bandwidth column */
    if (Tcl_GetIntFromObj(interp, objv[4], &localbw_col) != TCL_OK)
	return TCL_ERROR;
    if (localbw_col >= nvars)
    {
	Tcl_SetResult(interp, "bandwidth column number is out of range",
		      TCL_STATIC);
	return TCL_ERROR;
    }

    /* Kernel function */
    fcn = find_kernel_1d(Tcl_GetStringFromObj(objv[5], NULL));
    if (fcn == NULL)
    {
	Tcl_AppendResult(interp, "Bad kernel \"",
			 Tcl_GetStringFromObj(objv[5], NULL),
			 "\". Valid kernels are ", NULL);
	append_kernel_names_1d(interp);
	Tcl_AppendResult(interp, ".", NULL);
	return TCL_ERROR;
    }

    /* Global bandwidth */
    if (Tcl_GetDoubleFromObj(interp, objv[6], &bw) != TCL_OK)
	return TCL_ERROR;

    /* Check the result specifier */
    if (Tcl_GetIntFromObj(interp, objv[7], &id) != TCL_OK)
	return TCL_ERROR;
    itype = hs_type(id);
    switch (itype)
    {
    case HS_1D_HISTOGRAM:
	tcl_require_objc(8);
	nbins = hs_1d_hist_num_bins(id);
	hs_1d_hist_range(id, &xmin, &xmax);
	halfstep = (xmax - xmin)/2.f/(float)nbins;
	dxmin = xmin + halfstep;
	dxmax = xmax - halfstep;
	break;

    case HS_NTUPLE:
	tcl_require_objc(11);
	if (hs_num_variables(id) != 2)
	{
	    Tcl_SetResult(interp, "result ntuple must have two variables",
			  TCL_STATIC);
	    return TCL_ERROR;
	}
	if (Tcl_GetDoubleFromObj(interp, objv[8], &dxmin) != TCL_OK)
	    return TCL_ERROR;
	if (Tcl_GetDoubleFromObj(interp, objv[9], &dxmax) != TCL_OK)
	    return TCL_ERROR;
	if (Tcl_GetIntFromObj(interp, objv[10], &nbins) != TCL_OK)
	    return TCL_ERROR;
	if (nbins <= 0)
	{
	    Tcl_SetResult(interp, "number of result points must be positive",
			  TCL_STATIC);
	    return TCL_ERROR;
	}
	break;

    case HS_NONE:
	Tcl_AppendResult(interp, "Histo-Scope item with id ",
			 Tcl_GetStringFromObj(objv[7], NULL),
			 " doesn't exist", NULL);
	return TCL_ERROR;

    default:
	Tcl_AppendResult(interp, "Histo-Scope item with id ",
			 Tcl_GetStringFromObj(objv[7], NULL),
			 " is not 1d histogram or ntuple", NULL);
	return TCL_ERROR;
    }

    /* Allocate the memory for the result */
    result = (float *)malloc(nbins*sizeof(float));
    if (result == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }

    /* Get the density estimate */
    status = estmate_kernel_density_1d(
	nt_id, points_col, weights_col, localbw_col,
	fcn, bw, dxmin, dxmax, nbins, result);
    switch (status)
    {
    case 0:
	/* success */
	break;
    case 1:
	/* We have failed to spot bad parameter settings */
	fprintf(stderr, "Kernel_density_1d: internal error. Aborting.\n");
	assert(0);	
    case 2:
	/* out of memory */
	free(result);
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    case 3:
	/* bad bandwidth (0) */
	memset(result, 0, nbins*sizeof(float));
	break;
    default:
	/* should never happen */
	fprintf(stderr, "Kernel_density_1d: invalid return status. Aborting.\n");
	assert(0);	
    }

    /* Fill the result structure */
    hs_reset(id);
    switch (itype)
    {
    case HS_1D_HISTOGRAM:
	hs_1d_hist_block_fill(id, result, NULL, NULL);
	break;

    case HS_NTUPLE:
	drange = dxmax - dxmin;
	dbins = (double)nbins;
	for (i=0; i<nbins; ++i)
	{
	    if (i)
		x[0] = (float)(dxmin + ((double)i / dbins) * drange);
	    else
		x[0] = (float)(dxmin);
	    x[1] = result[i];
	    status = hs_fill_ntuple(id, x);
	    if (status != id)
	    {
		free(result);
		Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
		return TCL_ERROR;
	    }
	}
	break;

    default:
	assert(0);
    }

    free(result);
    return TCL_OK;
}

tcl_routine(Kernel_density_2d)
{
    /* Usage: kernel_density_2d ntuple_id x_col y_col weights_col
     *     sxsq_col sysq_col sxsy_col kernel_function bw_matrix
     *     id [xmin xmax npoints ymin ymax ypoints]
     */
    int ix, iy, nt_id, id, nvars, itype, nx, ny, status;
    int x_col, y_col, weights_col, sxsq_col, sysq_col, sxsy_col;
    Kernel_function_2d *fcn;
    double xmin, xmax, ymin, ymax, mat[2][2], yrange, xrange, dnx, dny;
    float fxmin, fxmax, fymin, fymax, halfstep, *result, x[3];

    tcl_objc_range(11,17);
    verify_ntuple(nt_id,1);
    nvars = hs_num_variables(nt_id);

    /* Column of x coords */
    if (Tcl_GetIntFromObj(interp, objv[2], &x_col) != TCL_OK)
	return TCL_ERROR;
    if (x_col < 0 || x_col >= nvars)
    {
	Tcl_SetResult(interp, "x coordinate column number is out of range",
		      TCL_STATIC);
	return TCL_ERROR;
    }

    /* Column of y coords */
    if (Tcl_GetIntFromObj(interp, objv[3], &y_col) != TCL_OK)
	return TCL_ERROR;
    if (y_col < 0 || y_col >= nvars)
    {
	Tcl_SetResult(interp, "y coordinate column number is out of range",
		      TCL_STATIC);
	return TCL_ERROR;
    }
    
    /* Column of weights */
    if (Tcl_GetIntFromObj(interp, objv[4], &weights_col) != TCL_OK)
	return TCL_ERROR;
    if (weights_col >= nvars)
    {
	Tcl_SetResult(interp, "weight column number is out of range",
		      TCL_STATIC);
	return TCL_ERROR;
    }

    /* Column of x variances */
    if (Tcl_GetIntFromObj(interp, objv[5], &sxsq_col) != TCL_OK)
	return TCL_ERROR;
    if (sxsq_col >= nvars)
    {
	Tcl_SetResult(interp, "x variance column number is out of range",
		      TCL_STATIC);
	return TCL_ERROR;
    }

    /* Column of y variances */
    if (Tcl_GetIntFromObj(interp, objv[6], &sysq_col) != TCL_OK)
	return TCL_ERROR;
    if (sysq_col >= nvars)
    {
	Tcl_SetResult(interp, "y variance column number is out of range",
		      TCL_STATIC);
	return TCL_ERROR;
    }

    /* Column of covariances */
    if (Tcl_GetIntFromObj(interp, objv[7], &sxsy_col) != TCL_OK)
	return TCL_ERROR;
    if (sxsy_col >= nvars)
    {
	Tcl_SetResult(interp, "covariance column number is out of range",
		      TCL_STATIC);
	return TCL_ERROR;
    }

    /* Check if we are missing some column specification */
    if (sxsq_col >= 0 || sysq_col >= 0 || sxsy_col >= 0)
    {
	if (sxsq_col < 0)
	{
	    Tcl_SetResult(interp, "x variance column number is out of range",
			  TCL_STATIC);
	    return TCL_ERROR;
	}
	if (sysq_col < 0)
	{
	    Tcl_SetResult(interp, "y variance column number is out of range",
			  TCL_STATIC);
	    return TCL_ERROR;
	}
    }

    /* Kernel function */
    fcn = find_kernel_2d(Tcl_GetStringFromObj(objv[8], NULL));
    if (fcn == NULL)
    {
	Tcl_AppendResult(interp, "Bad kernel \"",
			 Tcl_GetStringFromObj(objv[8], NULL),
			 "\". Valid kernels are ", NULL);
	append_kernel_names_2d(interp);
	Tcl_AppendResult(interp, ".", NULL);
	return TCL_ERROR;
    }

    /* Bandwidth transformation matrix */
    if (parse_2d_matrix(interp, objv[9], mat) != TCL_OK)
    {
	Tcl_SetResult(interp, "bad transformation matrix specification",
		      TCL_STATIC);
	return TCL_ERROR;
    }

    /* id of the result item */
    if (Tcl_GetIntFromObj(interp, objv[10], &id) != TCL_OK)
	return TCL_ERROR;
    itype = hs_type(id);
    switch (itype)
    {
    case HS_2D_HISTOGRAM:
	tcl_require_objc(11);
	hs_2d_hist_num_bins(id, &nx, &ny);
	hs_2d_hist_range(id, &fxmin, &fxmax, &fymin, &fymax);
	halfstep = (fxmax - fxmin)/2.f/(float)nx;
	xmin = fxmin + halfstep;
	xmax = fxmax - halfstep;
	halfstep = (fymax - fymin)/2.f/(float)ny;
	ymin = fymin + halfstep;
	ymax = fymax - halfstep;
	break;

    case HS_NTUPLE:
	tcl_require_objc(17);
	if (hs_num_variables(id) != 3)
	{
	    Tcl_SetResult(interp, "result ntuple must have three variables",
			  TCL_STATIC);
	    return TCL_ERROR;
	}
	if (Tcl_GetDoubleFromObj(interp, objv[11], &xmin) != TCL_OK)
	    return TCL_ERROR;
	if (Tcl_GetDoubleFromObj(interp, objv[12], &xmax) != TCL_OK)
	    return TCL_ERROR;
	if (Tcl_GetIntFromObj(interp, objv[13], &nx) != TCL_OK)
	    return TCL_ERROR;
	if (Tcl_GetDoubleFromObj(interp, objv[14], &ymin) != TCL_OK)
	    return TCL_ERROR;
	if (Tcl_GetDoubleFromObj(interp, objv[15], &ymax) != TCL_OK)
	    return TCL_ERROR;
	if (Tcl_GetIntFromObj(interp, objv[16], &ny) != TCL_OK)
	    return TCL_ERROR;
	if (nx <= 0 || ny <= 0)
	{
	    Tcl_SetResult(interp, "number of result points must be positive",
			  TCL_STATIC);
	    return TCL_ERROR;
	}
	break;

    case HS_NONE:
	Tcl_AppendResult(interp, "Histo-Scope item with id ",
			 Tcl_GetStringFromObj(objv[10], NULL),
			 " doesn't exist", NULL);
	return TCL_ERROR;

    default:
	Tcl_AppendResult(interp, "Histo-Scope item with id ",
			 Tcl_GetStringFromObj(objv[10], NULL),
			 " is not 2d histogram or ntuple", NULL);
	return TCL_ERROR;
    }

    /* Allocate the memory for the result */
    result = (float *)malloc(nx*ny*sizeof(float));
    if (result == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }

    /* Get the density estimate */
    status = estmate_kernel_density_2d(
	nt_id, x_col, y_col, weights_col, sxsq_col, sysq_col, sxsy_col,
	fcn, mat, xmin, xmax, nx, ymin, ymax, ny, result);
    switch (status)
    {
    case 0:
	/* success */
	break;
    case 1:
	/* We have failed to spot bad parameter settings */
	fprintf(stderr, "Kernel_density_2d: internal error. Aborting.\n");
	assert(0);
    case 2:
	/* out of memory */
	free(result);
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    case 3:
	/* bad bandwidth transformation matrix (0 determinant) */
	memset(result, 0, nx*ny*sizeof(float));
	break;
    default:
	/* should never happen */
	fprintf(stderr, "Kernel_density_2d: invalid return status. Aborting.\n");
	assert(0);
    }

    /* Fill the result structure */
    hs_reset(id);
    switch (itype)
    {
    case HS_2D_HISTOGRAM:
	hs_2d_hist_block_fill(id, result, NULL, NULL);
	break;

    case HS_NTUPLE:
	xrange = xmax - xmin;
	yrange = ymax - ymin;
	dnx = (double)(nx - 1);
	dny = (double)(ny - 1);
	for (ix = 0; ix < nx; ++ix)
	{
	    if (ix)
		x[0] = (float)(xmin + ((double)ix / dnx) * xrange);
	    else
		x[0] = (float)(xmin);
	    for (iy = 0; iy < ny; ++iy)
	    {
		if (iy)
		    x[1] = (float)(ymin + ((double)iy / dny) * yrange);
		else
		    x[1] = (float)(ymin);
		x[2] = result[ix*ny + iy];
		status = hs_fill_ntuple(id, x);
		if (status != id)
		{
		    free(result);
		    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
		    return TCL_ERROR;
		}
	    }
	}
	break;

    default:
	assert(0);
    }

    free(result);
    return TCL_OK;
}

tcl_routine(Inverse_symmetric_sqrt_2d)
{
    double tmp, sqrdet, bover2, det, mat[2][2], lambda0, lambda1;
    double e00, e01, e10, e11, len, d00, d11, a00, a01, a11;
    Tcl_Obj *row0[2];
    Tcl_Obj *row1[2];
    Tcl_Obj *rows[2];

    tcl_require_objc(2);
    if (parse_2d_matrix(interp, objv[1], mat) != TCL_OK)
    {
	Tcl_SetResult(interp, "invalid 2d matrix specification",
		      TCL_STATIC);
	return TCL_ERROR;
    }
    if (mat[0][1] != mat[1][0])
    {
	Tcl_SetResult(interp, "matrix is not symmetric",
		      TCL_STATIC);
	return TCL_ERROR;
    }
    det = mat[0][0]*mat[1][1] - mat[0][1]*mat[1][0];
    if (mat[0][0] <= 0.0 || mat[1][1] <= 0.0 || det <= 0.0)
    {
	Tcl_SetResult(interp, "matrix is not positive definite",
		      TCL_STATIC);
	return TCL_ERROR;
    }
    if (mat[0][1] == 0.0)
    {
	a00 = 1.0/sqrt(mat[0][0]);
	a01 = 0.0;
	a11 = 1.0/sqrt(mat[1][1]);
    }
    else
    {
	bover2 = (mat[0][0] + mat[1][1])/2.0;
	tmp = bover2*bover2 - det;
	if (tmp <= 0.0)
	{
	    Tcl_SetResult(interp, "matrix is not numerically stable",
			  TCL_STATIC);
	    return TCL_ERROR;	
	}
	sqrdet = sqrt(tmp);
	lambda0 = bover2 - sqrdet;
	lambda1 = bover2 + sqrdet;
	if (lambda0 <= 0.0)
	{
	    Tcl_SetResult(interp, "matrix is not numerically stable",
			  TCL_STATIC);
	    return TCL_ERROR;	
	}
	e00 = mat[0][1];
	e01 = lambda0 - mat[0][0];
	len = hypot(e00, e01);
	e00 /= len;
	e01 /= len;
	e10 = mat[0][1];
	e11 = lambda1 - mat[0][0];
	len = hypot(e10, e11);
	e10 /= len;
	e11 /= len;
	d00 = 1.0/sqrt(lambda0);
	d11 = 1.0/sqrt(lambda1);
	a00 = d00*e00*e00 + d11*e10*e10;
	a01 = d00*e00*e01 + d11*e10*e11;
	a11 = d00*e01*e01 + d11*e11*e11;
    }
    row0[0] = Tcl_NewDoubleObj(a00);
    row0[1] = Tcl_NewDoubleObj(a01);
    row1[0] = Tcl_NewDoubleObj(a01);
    row1[1] = Tcl_NewDoubleObj(a11);
    rows[0] = Tcl_NewListObj(2, row0);
    rows[1] = Tcl_NewListObj(2, row1);
    Tcl_SetObjResult(interp, Tcl_NewListObj(2, rows));
    return TCL_OK;
}

static int parse_2d_matrix(Tcl_Interp *interp, Tcl_Obj *obj,
			   double mat[2][2])
{
    int listlen;
    Tcl_Obj **listObjElem, **rowelem;

    if (Tcl_ListObjGetElements(interp, obj, &listlen, &listObjElem) != TCL_OK)
	return TCL_ERROR;
    if (listlen != 2)
	return TCL_ERROR;
    if (Tcl_ListObjGetElements(interp, listObjElem[0], &listlen, &rowelem) != TCL_OK)
	return TCL_ERROR;
    if (listlen != 2)
	return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, rowelem[0], &mat[0][0]) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, rowelem[1], &mat[0][1]) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_ListObjGetElements(interp, listObjElem[1], &listlen, &rowelem) != TCL_OK)
	return TCL_ERROR;
    if (listlen != 2)
	return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, rowelem[0], &mat[1][0]) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, rowelem[1], &mat[1][1]) != TCL_OK)
	return TCL_ERROR;
    return TCL_OK;
}

int ntuple_hashtable_key_length(int nkey_columns)
{
    int nbytes, nints;

    if (nkey_columns <= 0)
	return -100;
    nbytes = nkey_columns*sizeof(float);
    nints = nbytes / sizeof(int);
    if (nbytes % sizeof(int))
	++nints;
    if (nints == 1)
	++nints;
    return nints;
}

int histo_dim_from_type(int type)
{
    switch (type)
    {
    case HS_1D_HISTOGRAM:
	return 1;
    case HS_2D_HISTOGRAM:
	return 2;
    case HS_3D_HISTOGRAM:
	return 3;
    default:
	return 0;
    }
}

int get_float_array_from_binary_or_list(
    Tcl_Interp *interp, Tcl_Obj *obj,
    float **parray, int *psize, int *pfreeItLater)
{
    float *arr = NULL;
    int nelem = 0, needToFree = 0;

    const Tcl_ObjType* byteArrayTypePtr = Tcl_GetObjType("bytearray");
    *parray = NULL;
    *psize = 0;
    *pfreeItLater = 0;
    if (obj->typePtr == byteArrayTypePtr)
    {
        arr = (float *)Tcl_GetByteArrayFromObj(obj, &nelem);
        nelem /= sizeof(float);
    }
    else
    {
        /* Treat the argument as a list of doubles */
        Tcl_Obj **listObjElem;
	if (Tcl_ListObjGetElements(interp, obj, &nelem, &listObjElem) != TCL_OK)
	    return TCL_ERROR;
        if (nelem > 0)
        {
            int i;
            double d;
            arr = (float *)malloc(nelem*sizeof(float));
            if (arr == NULL)
            {
                Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
                return TCL_ERROR;
            }
            for (i=0; i<nelem; ++i)
            {
                if (Tcl_GetDoubleFromObj(interp, listObjElem[i], &d) != TCL_OK)
                {
                    free(arr);
                    return TCL_ERROR;
                }
                arr[i] = (float)d;
            }
            needToFree = 1;
        }
    }
    *parray = arr;
    *psize = nelem;
    *pfreeItLater = needToFree;
    return TCL_OK;
}

static int sort_column_by_value(const sorted_column_data* p1,
                                const sorted_column_data* p2)
{
    return (p1->value < p2->value ? -1 :
            p1->value > p2->value ? 1 : 0);
}

static int inverse_sort_column_by_value(const sorted_column_data* p1,
                                        const sorted_column_data* p2)
{
    return (p1->value < p2->value ? 1 :
            p1->value > p2->value ? -1 : 0);
}

static int sort_column_by_row(const sorted_column_data* p1,
                              const sorted_column_data* p2)
{
    return (p1->row < p2->row ? -1 :
            p1->row > p2->row ? 1 : 0);
}

static int sort_float_rows_incr(const float *row1, const float *row2)
{
    int i;
    register float f1, f2;
    for (i=0; i<n_ntuple_sort_columns; ++i)
    {
        f1 = row1[ntuple_sort_columns[i]];
        f2 = row2[ntuple_sort_columns[i]];
        if (f1 < f2)
            return -1;
        else if (f1 > f2)
            return 1;
    }
    return 0;
}

static int sort_float_rows_decr(const float *row1, const float *row2)
{
    int i;
    register float f1, f2;
    for (i=0; i<n_ntuple_sort_columns; ++i)
    {
        f1 = row1[ntuple_sort_columns[i]];
        f2 = row2[ntuple_sort_columns[i]];
        if (f1 < f2)
            return 1;
        else if (f1 > f2)
            return -1;
    }
    return 0;
}
