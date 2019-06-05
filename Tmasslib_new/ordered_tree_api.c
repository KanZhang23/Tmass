#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <dlfcn.h>

#include "histoscope.h"
#include "ordered_tree_collection.h"
#include "ordered_tree_api.h"
#include "cdf_2d.h"
#include "topmass_utils.h"

#define MAX_DIM_NUMBER 100

#ifdef __cplusplus
extern "C" {
#endif

int find_duplicate_name(char **names, int count);
int hs_duplicate_ntuple_header(int id, int uid, char *title, char *category);

static Ordered_tree_node** piecedata_set = 0;
static unsigned piecedata_set_size = 0;
static unsigned collnum = 0;

static Tcl_Interp *commandInterp = 0;

typedef struct {
    char *category;
    char **varnames;
    int nvars;
    int uid;
} NtupleCreationInfo;

typedef struct {
    int parent_nrows;
    int parent_ncols;
    int *projected_columns;
    int n_projected_columns;
    float *parent_data;
    float *out_buf;
} NtupleFillInfo;

typedef struct {
    Tcl_Interp *interp;
    Tcl_Obj *result;
    unsigned *projected_columns;
    unsigned n_projected_columns;
    float *buf;
} CellListInfo;

typedef struct {
    Tcl_Interp *interp;
    Tcl_Obj *result;
    const unsigned *projected_columns;
    unsigned n_projected_columns;
    float *bb_min;
    float *bb_max;
} LeafBoxInfo;

typedef struct {
    int column;
    float min;
    float max;
} SimpleCut;

static float *row_data = NULL;
static unsigned n_row_data = 0;

static SimpleCut *filter_cuts = NULL;
static unsigned n_filter_cuts = 0;

#define init_mem(ptr, size) do {\
    get_static_mem((void **)&ptr, sizeof(*ptr), &n_ ## ptr, size);\
} while(0);

#define verify_ntuple(id,objnum) do {\
    if (Tcl_GetIntFromObj(interp, objv[objnum], &id) != TCL_OK)\
	return TCL_ERROR;\
    if (hs_type(id) != HS_NTUPLE)\
    {\
	if (hs_type(id) == HS_NONE)\
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not a valid Histo-Scope id", NULL);\
	else\
	    Tcl_AppendResult(interp, "item with id ",\
			     Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not an ntuple", NULL);\
	return TCL_ERROR;\
    }\
} while(0);

#define tcl_require_objc(N) do {\
  if (objc != N)\
  {\
      Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[0],NULL),\
                       " : wrong # of arguments", NULL);\
      return TCL_ERROR;\
  }\
} while(0);

#define tcl_objc_range(N,M) do {\
  if (objc < N || objc > M)\
  {\
      Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[0],NULL),\
                       " : wrong # of arguments", NULL);\
      return TCL_ERROR;\
  }\
} while(0);

static void get_static_mem(void **ptr, size_t structsize,
                           unsigned *nmemb, unsigned newmemb)
{
    if (newmemb > *nmemb)
    {
	if (*ptr)
	    free(*ptr);
	*ptr = malloc(newmemb * structsize);
	if (*ptr == 0)
	{
	    fprintf(stderr, "Fatal error: out of memory. Exiting.\n");
	    exit(EXIT_FAILURE);
	}
	*nmemb = newmemb;
    }
}

static int row_passes_cuts(const float *row, const SimpleCut *cuts,
                           const int ncuts)
{
    int i;
    const SimpleCut *c;

    for (i=0; i<ncuts; ++i)
    {
        c = cuts + i;
        {
            register float datum = row[c->column];
            if (datum < c->min || datum >= c->max)
                return 0;
        }
    }
    return 1;
}

static int parse_filter_cut(Tcl_Interp *interp, const int nvars,
                            Tcl_Obj *obj, SimpleCut *cut)
{
    int listlen, col;
    Tcl_Obj **listObjElem;
    double dmin, dmax;

    if (Tcl_ListObjGetElements(interp, obj, &listlen, &listObjElem) != TCL_OK)
        return TCL_ERROR;
    if (listlen != 3)
    {
        Tcl_SetResult(interp, "wrong cut specification", TCL_STATIC);
        return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, listObjElem[0], &col) != TCL_OK)
        return TCL_ERROR;
    if (col < 0 || col >= nvars)
    {
        Tcl_SetResult(interp, "column number is out of range", TCL_STATIC);
        return TCL_ERROR;
    }
    if (Tcl_GetDoubleFromObj(interp, listObjElem[1], &dmin) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, listObjElem[2], &dmax) != TCL_OK)
        return TCL_ERROR;
    cut->column = col;
    if (dmin > -FLT_MAX)
        cut->min = dmin;
    else
        cut->min = -FLT_MAX;
    if (dmax < FLT_MAX)
        cut->max = dmax;
    else
        cut->max = FLT_MAX;
    return TCL_OK;
}

static int parse_bounding_box(Tcl_Interp *interp, const int maxdim, Tcl_Obj *obj,
                              int *bb_ndim, float *bb_min, float *bb_max)
{
    int i, listlen, listlen2;
    Tcl_Obj **listObjElem, **listObjElem2;
    double ival;

    if (Tcl_ListObjGetElements(interp, obj, &listlen, &listObjElem) != TCL_OK)
        return TCL_ERROR;
    if (listlen == 0)
    {
        Tcl_SetResult(interp, "empty bounding box", TCL_STATIC);
        return TCL_ERROR;
    }
    if (listlen > maxdim)
    {
        Tcl_SetResult(interp, "bounding box dimensionality too large", TCL_STATIC);
        return TCL_ERROR;
    }
    *bb_ndim = listlen;
    for (i=0; i<listlen; ++i)
    {
        if (Tcl_ListObjGetElements(interp, listObjElem[i], &listlen2, &listObjElem2) != TCL_OK)
            return TCL_ERROR;
        if (listlen2 != 2)
        {
            Tcl_SetResult(interp, "invalid bounding box specification", TCL_STATIC);
            return TCL_ERROR;
        }
        if (Tcl_GetDoubleFromObj(interp, listObjElem2[0], &ival) != TCL_OK)
            return TCL_ERROR;
        bb_min[i] = ival;
        if (Tcl_GetDoubleFromObj(interp, listObjElem2[1], &ival) != TCL_OK)
            return TCL_ERROR;
        bb_max[i] = ival;
        if (bb_min[i] >= bb_max[i])
        {
            Tcl_SetResult(interp, "wrong boundaries, min >= max", TCL_STATIC);
            return TCL_ERROR;
        }
    }
    return TCL_OK;
}

static int parse_tree_shape(Tcl_Interp *interp, Tcl_Obj *obj,
                            Ordered_tree_shape *shape, int n_ntuple_vars)
{
    int i, ival, listlen, listlen2;
    Tcl_Obj **listObjElem, **listObjElem2;

    if (Tcl_ListObjGetElements(interp, obj, &listlen, &listObjElem) != TCL_OK)
        return TCL_ERROR;
    if (listlen == 0)
    {
        Tcl_SetResult(interp, "empty tree shape", TCL_STATIC);
        return TCL_ERROR;
    }
    shape->splits = (Ordered_tree_split *)malloc(listlen*sizeof(Ordered_tree_split));
    if (shape->splits == NULL)
    {
        Tcl_SetResult(interp, "out of memory", TCL_STATIC);
        return TCL_ERROR;
    }
    shape->nlevels = listlen;
    for (i=0; i<listlen; ++i)
    {
        if (Tcl_ListObjGetElements(interp, listObjElem[i], &listlen2, &listObjElem2) != TCL_OK)
            return TCL_ERROR;
        if (listlen2 != 2)
        {
            Tcl_SetResult(interp, "invalid tree shape", TCL_STATIC);
            return TCL_ERROR;
        }
        if (Tcl_GetIntFromObj(interp, listObjElem2[0], &ival) != TCL_OK)
            return TCL_ERROR;
        if (ival < 0 || ival >= n_ntuple_vars)
        {
            Tcl_SetResult(interp, "tree shape dimension out of range", TCL_STATIC);
            return TCL_ERROR;
        }
        shape->splits[i].split_dim = ival;
        if (Tcl_GetIntFromObj(interp, listObjElem2[1], &ival) != TCL_OK)
            return TCL_ERROR;
        if (ival < 2)
        {
            Tcl_SetResult(interp, "number of splits out of range", TCL_STATIC);
            return TCL_ERROR;
        }
        shape->splits[i].nsplits = ival;
    }
    return TCL_OK;
}

static int parse_tree_number(Tcl_Interp *interp, Tcl_Obj *obj,
			     Ordered_tree_node** topnode)
{
    int i;
    if (Tcl_GetIntFromObj(interp, obj, &i) != TCL_OK)
	return TCL_ERROR;
    if (i<0 || i>=(int)piecedata_set_size)
    {
	Tcl_AppendResult(interp, "ordered tree number ",
			 Tcl_GetStringFromObj(obj, NULL),
			 " is out of range", NULL);
	return TCL_ERROR;
    }
    if (piecedata_set[i] == NULL)
    {
	Tcl_AppendResult(interp, "ordered tree number ",
			 Tcl_GetStringFromObj(obj, NULL),
			 " does not exist", NULL);
	return TCL_ERROR;
    }
    *topnode = piecedata_set[i];
    return TCL_OK;
}

const Ordered_tree_node* get_ordered_tree_by_number(int i)
{
    if (i<0 || i>=(int)piecedata_set_size)
        return NULL;
    else
        return piecedata_set[i];
}

static unsigned find_unused_index(void)
{
    unsigned i;
    for (i=0; i<piecedata_set_size; ++i)
        if (piecedata_set[i] == NULL)
            return i;
    piecedata_set = (Ordered_tree_node **)realloc(
        piecedata_set, (piecedata_set_size + 1)*sizeof(Ordered_tree_node *));
    if (piecedata_set == NULL)
    {
        fprintf(stderr, "Fatal error in find_unused_index: out of memory. Exiting.\n");
        exit(EXIT_FAILURE);
    }
    piecedata_set[piecedata_set_size] = NULL;
    return piecedata_set_size++;
}

static void payload_dumper(void *userData, FILE *f, const OTPayload p)
{
    fprintf(f, "%.17e", (double)p);
}

static void binary_payload_dumper(void *userData, XDR *xdrs, const OTPayload p)
{
    OTPayload tmp = p;
    assert(xdr_double(xdrs, &tmp));
}

static int binary_payload_reader(void *userData, XDR *xdrs, OTPayload *pay)
{
    if (xdr_double(xdrs, pay) == 1)
        return 0;
    else
        return 1;
}

static void payload_destructor(OTPayload p)
{
    if (p > 0)
        if (hs_type((int)p) != HS_NONE)
            hs_delete((int)p);
}

static OTPayload payload_constructor(void *userData)
{
    NtupleCreationInfo *nt = (NtupleCreationInfo *)userData;
    char title[32];
    int id;

    sprintf(title, "Ntuple %d", nt->uid);
    id = hs_create_ntuple(nt->uid++, title, nt->category, nt->nvars, nt->varnames);
    if (id <= 0)
    {
        fflush(stderr);
        fflush(stdout);
        assert(!"Ntuple creation failed, can't recover here");
    }
    return id;
}

static void payload_filler(void *userData, OTPayload *payload,
                           const Ordered_tree_key *pkey)
{
    const NtupleFillInfo *nt = (const NtupleFillInfo *)userData;
    int i;
    float *row;

    assert((int)pkey->keyload < nt->parent_nrows);
    row = nt->parent_data + pkey->keyload*nt->parent_ncols;
    for (i=0; i<nt->n_projected_columns; ++i)
        nt->out_buf[i] = row[nt->projected_columns[i]];
    hs_fill_ntuple(*payload, nt->out_buf);
}

static int c_ordered_tree_create_6(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_create ntuple_id shape_specifier \
     *           projected_columns new_category varnames
     *
     * shape_specifier  is a list of lists {{col0 nsplits0} {col1 nsplits1} ...}
     * projected_columns  is a list of integers
     * varnames  is a list of strings, same length as "projected_columns"
     */
    const unsigned maxlen = 80;
    int i, ntuple_id, listlen, n_projected_columns, n_ntuple_vars, n_rows;
    char **listelem = 0;
    int *projected_columns = 0;
    Tcl_Obj **listObjElem;
    NtupleCreationInfo nt_creation_info;
    NtupleFillInfo nt_fill_info;
    float *buf = 0;
    Ordered_tree_key *keys = 0;
    Ordered_tree_node *topnode;
    Ordered_tree_shape shape = {0, 0};

    tcl_require_objc(6);
    verify_ntuple(ntuple_id, 1);
    n_ntuple_vars = hs_num_variables(ntuple_id);
    n_rows = hs_num_entries(ntuple_id);
    if (n_rows == 0)
    {
        Tcl_SetResult(interp, "input ntuple is empty", TCL_STATIC);
        goto fail;
    }

    if (parse_tree_shape(interp, objv[2], &shape, n_ntuple_vars) != TCL_OK)
        goto fail;

    if (Tcl_ListObjGetElements(interp, objv[3], &listlen, &listObjElem) != TCL_OK)
        goto fail;
    if (listlen == 0)
    {
        Tcl_SetResult(interp, "the list of projected columns can not be empty",
                      TCL_STATIC);
        goto fail;
    }
    projected_columns = (int *)malloc(listlen*sizeof(int));
    if (projected_columns == NULL)
    {
        Tcl_SetResult(interp, "out of memory", TCL_STATIC);
        goto fail;
    }
    for (i=0; i<listlen; ++i)
    {
        int icol;
        if (Tcl_GetIntFromObj(interp, listObjElem[i], &icol) != TCL_OK)
            goto fail;
        if (icol < 0 || icol >= n_ntuple_vars)
        {
            Tcl_SetResult(interp, "projected column index out of range", TCL_STATIC);
            goto fail;
        }
        projected_columns[i] = icol;
    }
    n_projected_columns = listlen;

    nt_creation_info.category = Tcl_GetStringFromObj(objv[4], NULL);

    if (Tcl_ListObjGetElements(interp, objv[5], &listlen, &listObjElem) != TCL_OK)
        goto fail;
    if (listlen == 0)
    {
        Tcl_SetResult(interp, "the list of column names is empty", TCL_STATIC);
        goto fail;
    }
    if (listlen != n_projected_columns)
    {
        Tcl_SetResult(interp, "incompatible number of variables", TCL_STATIC);
        goto fail;
    }
    if ((listelem = (char **)malloc(listlen*sizeof(char *))) == NULL)
    {
        Tcl_SetResult(interp, "out of memory", TCL_STATIC);
        goto fail;
    }
    for (i=0; i<listlen; ++i)
    {
        listelem[i] = Tcl_GetStringFromObj(listObjElem[i], NULL);
        if (*listelem[i] == '\0')
        {
            Tcl_SetResult(interp,
                          "ntuple variable name can not be an empty string",
                          TCL_STATIC);
            goto fail;
        }
        if (strlen(listelem[i]) > maxlen)
        {
            char buf[32];
            sprintf(buf, "%d", maxlen);
            Tcl_AppendResult(interp, "ntuple variable name \"", listelem[i],
                             "\" is too long, should have at most ",
                             buf, " characters", NULL);
            goto fail;
        }
    }
    i = find_duplicate_name(listelem, listlen);
    if (i >= 0)
    {
        Tcl_AppendResult(interp, "duplicate variable name \"", listelem[i], "\"", NULL);
        goto fail;
    }
    nt_creation_info.varnames = listelem;
    nt_creation_info.nvars = listlen;
    nt_creation_info.uid = 0;

    buf = (float *)malloc((n_ntuple_vars*n_rows + n_projected_columns)*sizeof(float));
    if (buf == NULL)
    {
        Tcl_SetResult(interp, "out of memory", TCL_STATIC);
        goto fail;
    }
    hs_ntuple_contents(ntuple_id, buf);

    nt_fill_info.parent_nrows = n_rows;
    nt_fill_info.parent_ncols = n_ntuple_vars;
    nt_fill_info.projected_columns = projected_columns;
    nt_fill_info.n_projected_columns = n_projected_columns;
    nt_fill_info.parent_data = buf;
    nt_fill_info.out_buf = buf + n_ntuple_vars*n_rows;

    keys = (Ordered_tree_key *)malloc(n_rows*sizeof(Ordered_tree_key));
    if (keys == NULL)
    {
        Tcl_SetResult(interp, "out of memory", TCL_STATIC);
        goto fail;
    }
    for (i=0; i<n_rows; ++i)
    {
        keys[i].index = buf + n_ntuple_vars*i;
        keys[i].weight = 1.f;
        keys[i].keyload = i;
    }

    topnode = ot_create_tree(keys, n_rows, &shape,
                             payload_constructor, &nt_creation_info,
                             payload_filler, &nt_fill_info);
    assert(topnode);

    i = find_unused_index();
    piecedata_set[i] = topnode;
    Tcl_SetObjResult(interp, Tcl_NewIntObj(i));

    free(keys);
    free(buf);
    free(listelem);
    free(projected_columns);
    free(shape.splits);
    return TCL_OK;

 fail:
    if (keys)
        free(keys);
    if (buf)
        free(buf);
    if (listelem)
        free(listelem);
    if (projected_columns)
        free(projected_columns);
    if (shape.splits)
        free(shape.splits);
    return TCL_ERROR;
}

static double rectangle_split(
    const Cdf_2d_data *cdf,
    const double xmin, const double xmax,
    const double ymin, const double ymax,
    const double coverage, const double rcdf, const double releps,
    double old_min, const double old_coverage, const int isY)
{
    double old_max = isY ? ymax : xmax;
    const double eps = isY ? (ymax - ymin)*releps : (xmax - xmin)*releps;

    assert(coverage > old_coverage && coverage < rcdf);
    assert(old_min < old_max);

    do {
        const double try_max = (old_min + old_max)/2.0;
        double try_coverage;
        if (isY)
            try_coverage = cdf_2d_rectangle_coverage(
                cdf, xmin, ymin, xmax, try_max);
        else
            try_coverage = cdf_2d_rectangle_coverage(
                cdf, xmin, ymin, try_max, ymax);
        if (try_coverage > coverage)
            old_max = try_max;
        else
            old_min = try_max;
    } while (old_max - old_min > eps);

    return (old_min + old_max)/2.0;
}

static Ordered_tree_node *ot_create_node_cdf2d(
    const Ordered_tree_shape *shape, const unsigned level,
    const Ordered_tree_node *parent, unsigned n_in_parent,
    const Cdf_2d_data *cdf_2d,
    const double xmin, const double xmax,
    const double ymin, const double ymax,
    const double eps)
{
    Ordered_tree_node *node = (Ordered_tree_node *)calloc(
        1, sizeof(Ordered_tree_node));
    assert(node);

    node->parent = parent;
    node->n_in_parent = n_in_parent;

    if (level < shape->nlevels)
    {
        /* Process this level split */
	ot_split_node(node, shape->splits + level);
        {
            const unsigned ntargets = 2*node->ndaus - 1;
            const double rcdf = cdf_2d_rectangle_coverage(
                cdf_2d, xmin, ymin, xmax, ymax);
            const double target_interval = rcdf/(2*node->ndaus);
            unsigned i, target_number;
            double old_min;

            if (node->split_dim == 0)
                old_min = xmin;
            else if (node->split_dim == 1)
                old_min = ymin;
            else
                assert(0);

            /* Figure stuff out */
            for (target_number=0; target_number<ntargets; ++target_number)
            {
                const double old_coverage = target_number*target_interval;
                const double coverage = (target_number+1)*target_interval;
                const double split = rectangle_split(
                    cdf_2d, xmin, xmax, ymin, ymax,
                    coverage, rcdf, eps,
                    old_min, old_coverage, node->split_dim);

                if (target_number % 2)
                    /* This is a bound */
                    node->bounds[target_number/2] = split;
                else
                    /* This is a center */
                    node->centers[target_number/2] = split;

                old_min = split;
            }

            for (i=0; i<node->ndaus; ++i)
            {
                if (node->split_dim == 0)
                {
                    double xlo, xhi;

                    if (i == 0)
                    {
                        xlo = xmin;
                        xhi = node->bounds[i];
                    }
                    else if (i == node->ndaus-1)
                    {
                        xlo = node->bounds[i-1];
                        xhi = xmax;
                    }
                    else
                    {
                        xlo = node->bounds[i-1];
                        xhi = node->bounds[i];
                    }

                    node->daus[i] = ot_create_node_cdf2d(
                        shape, level+1, node, i, cdf_2d,
                        xlo, xhi, ymin, ymax, eps);
                }
                else if (node->split_dim == 1)
                {
                    double ylo, yhi;

                    if (i == 0)
                    {
                        ylo = ymin;
                        yhi = node->bounds[i];
                    }
                    else if (i == node->ndaus-1)
                    {
                        ylo = node->bounds[i-1];
                        yhi = ymax;
                    }
                    else
                    {
                        ylo = node->bounds[i-1];
                        yhi = node->bounds[i];
                    }

                    node->daus[i] = ot_create_node_cdf2d(
                        shape, level+1, node, i, cdf_2d,
                        xmin, xmax, ylo, yhi, eps);
                }
                else
                    assert(0);
            }
        }
    }

    return node;
}

static int c_ordered_tree_create_cdf2d(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_create cdf_2d_handle shape_specifier eps? */
    int i;
    Tcl_CmdInfo cmdInfo;
    char *cmd;
    Cdf_2d_data *cdf_2d;
    Ordered_tree_shape shape = {0, 0};
    Ordered_tree_node *topnode = NULL;
    double eps = 1.0e-10;

    tcl_objc_range(3,4);
    cmd = Tcl_GetStringFromObj(objv[1], NULL);
    if (!Tcl_GetCommandInfo(interp, cmd, &cmdInfo))
    {
        Tcl_AppendResult(interp, "invalid command name \"", cmd, "\"", NULL);
        return TCL_ERROR;
    }
    cdf_2d = (Cdf_2d_data *)cmdInfo.objClientData;
    assert(cdf_2d);

    if (parse_tree_shape(interp, objv[2], &shape, 2) != TCL_OK)
        return TCL_ERROR;

    if (objc > 3)
        if (Tcl_GetDoubleFromObj(interp, objv[3], &eps) != TCL_OK)
            return TCL_ERROR;
    if (eps <= 0.0)
    {
        Tcl_SetResult(interp, "tolerance must be positive", TCL_VOLATILE);
        return TCL_ERROR;
    }

    topnode = ot_create_node_cdf2d(&shape, 0, NULL, 0, cdf_2d,
                                   cdf_2d->xmin, cdf_2d->xmax,
                                   cdf_2d->ymin, cdf_2d->ymax, eps);
    assert(topnode);

    i = find_unused_index();
    piecedata_set[i] = topnode;
    Tcl_SetObjResult(interp, Tcl_NewIntObj(i));

    return TCL_OK;
}

static int c_ordered_tree_create_3(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_create ntuple_id shape_specifier weight_column?
     *
     * shape_specifier is a list of lists {{col0 nsplits0} {col1 nsplits1} ...}
     */
    int i, ntuple_id, n_ntuple_vars, n_rows, weight_column = -1;
    float *buf = 0;
    Ordered_tree_key *keys = 0;
    Ordered_tree_node *topnode;
    Ordered_tree_shape shape = {0, 0};

    tcl_objc_range(3,4);
    verify_ntuple(ntuple_id, 1);
    n_ntuple_vars = hs_num_variables(ntuple_id);
    n_rows = hs_num_entries(ntuple_id);
    if (n_rows == 0)
    {
        Tcl_SetResult(interp, "input ntuple is empty", TCL_STATIC);
        goto fail;
    }

    if (parse_tree_shape(interp, objv[2], &shape, n_ntuple_vars) != TCL_OK)
        goto fail;

    if (objc > 3)
    {
        if (Tcl_GetIntFromObj(interp, objv[3], &weight_column) != TCL_OK)
            return TCL_ERROR;
        if (weight_column < 0 || weight_column >= n_ntuple_vars)
        {
            Tcl_SetResult(interp, "weight column number is out of range",
                          TCL_STATIC);
            return TCL_ERROR;
        }
    }

    buf = (float *)malloc((n_ntuple_vars*n_rows)*sizeof(float));
    if (buf == NULL)
    {
        Tcl_SetResult(interp, "out of memory", TCL_STATIC);
        goto fail;
    }
    hs_ntuple_contents(ntuple_id, buf);

    keys = (Ordered_tree_key *)malloc(n_rows*sizeof(Ordered_tree_key));
    if (keys == NULL)
    {
        Tcl_SetResult(interp, "out of memory", TCL_STATIC);
        goto fail;
    }
    for (i=0; i<n_rows; ++i)
    {
        keys[i].index = buf + n_ntuple_vars*i;
        if (weight_column >= 0)
            keys[i].weight = buf[n_ntuple_vars*i + weight_column];
        else
            keys[i].weight = 1.f;
        keys[i].keyload = i;
    }

    topnode = ot_create_tree(keys, n_rows, &shape, NULL, NULL, NULL, NULL);
    assert(topnode);

    i = find_unused_index();
    piecedata_set[i] = topnode;
    Tcl_SetObjResult(interp, Tcl_NewIntObj(i));

    free(keys);
    free(buf);
    free(shape.splits);
    return TCL_OK;

 fail:
    if (keys)
        free(keys);
    if (buf)
        free(buf);
    if (shape.splits)
        free(shape.splits);
    return TCL_ERROR;
}

static int c_ordered_tree_create(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    if (objc == 3 || objc == 4)
    {
        if (strncmp(Tcl_GetStringFromObj(objv[1], NULL), "cdf_2d_", 7) == 0)
            return c_ordered_tree_create_cdf2d(clientData, interp, objc, objv);
        else
            return c_ordered_tree_create_3(clientData, interp, objc, objv);
    }
    else
        return c_ordered_tree_create_6(clientData, interp, objc, objv);
}

static int c_ordered_tree_load(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_load filename funcname */
    char *filename, *funcname;
    int flags = RTLD_NOW;
    void *handle;
    Ordered_tree_node* (*location)(void);

    tcl_require_objc(3);
    filename = Tcl_GetStringFromObj(objv[1], NULL);
    funcname = Tcl_GetStringFromObj(objv[2], NULL);
    dlerror();
    handle = dlopen(filename, flags);
    if (handle == NULL)
    {
	Tcl_AppendResult(interp, "failed to open file \"", filename,
			 "\": ", dlerror(), NULL);
	return TCL_ERROR;
    }
    location = (Ordered_tree_node* (*)(void))dlsym(handle, funcname);
    if (dlerror() == NULL)
    {
        unsigned i = find_unused_index();
	piecedata_set[i] = location();
        Tcl_SetObjResult(interp, Tcl_NewIntObj(i));
        return TCL_OK;
    }
    else
    {
	Tcl_AppendResult(interp, "failed to find function \"", funcname,
			 "\" in file \"", filename, "\"", NULL);
        dlclose(handle);
	return TCL_ERROR;
    }
}

static int c_ordered_tree_binrestore(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_binrestore filename */
    char *filename;
    Ordered_tree_node *topnode;
    FILE *f;
    int i;
    XDR xdrs;

    tcl_require_objc(2);
    filename = Tcl_GetStringFromObj(objv[1], NULL);
    f = fopen(filename, "rb");
    if (f == NULL)
    {
        fprintf(stderr, "failed to open file %s", filename);
        fflush(stderr);
        return TCL_ERROR;
    }
    xdrstdio_create(&xdrs, f, XDR_DECODE);
    topnode = ot_binary_restore(&xdrs, binary_payload_reader, NULL, NULL);
    xdr_destroy(&xdrs);
    fclose(f);

    if (topnode)
    {
        i = find_unused_index();
        piecedata_set[i] = topnode;
        Tcl_SetObjResult(interp, Tcl_NewIntObj(i));
        return TCL_OK;
    }
    else
    {
        fprintf(stderr, "failed to parse file %s", filename);
        return TCL_ERROR;
    }
}

static void destroy_collection(ClientData uData)
{
    Ordered_tree_collection *coll = (Ordered_tree_collection *)uData;
    ot_destroy_collection(coll, NULL);
}

static int c_process_ot_collection(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    char *op;
    Ordered_tree_collection *coll = (Ordered_tree_collection *)clientData;

    tcl_objc_range(2, INT_MAX);
    op = Tcl_GetStringFromObj(objv[1], NULL);

    if (strcmp(op, "del") == 0)
    {
        tcl_require_objc(2);
        Tcl_DeleteCommand(interp, Tcl_GetStringFromObj(objv[0], NULL));
    }
    else if (strcmp(op, "write") == 0)
    {
        char *filename;
        tcl_require_objc(3);
        filename = Tcl_GetStringFromObj(objv[2], NULL);
        ot_write_collection(filename, coll, binary_payload_dumper, NULL);
    }
    else if (strcmp(op, "compare") == 0)
    {
        Tcl_CmdInfo cmdInfo;
        char *cmd;
        Ordered_tree_collection *other;
        int result;

        tcl_require_objc(3);
        cmd = Tcl_GetStringFromObj(objv[2], NULL);
        if (!Tcl_GetCommandInfo(interp, cmd, &cmdInfo))
        {
            Tcl_AppendResult(interp, "invalid command name \"", cmd, "\"", NULL);
            return TCL_ERROR;
        }
        other = (Ordered_tree_collection *)cmdInfo.objClientData;
        assert(other);
        result = ot_compare_collections(coll, other, 0.0, NULL);
        Tcl_SetObjResult(interp, Tcl_NewIntObj(result));
    }
    else
    {
        Tcl_AppendResult(interp, "invalid ot_collection operation \"", op, "\"", NULL);
        return TCL_ERROR;
    }

    return TCL_OK;
}

static int c_ordered_tree_collection_read(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_collection_read filename */
    char *filename;
    char buf[64];
    Ordered_tree_collection *coll;

    tcl_require_objc(2);
    filename = Tcl_GetStringFromObj(objv[1], NULL);
    coll = ot_read_collection(filename, binary_payload_reader, 0, 0);
    if (coll == 0)
    {
        Tcl_AppendResult(interp, "failed to read file \"", filename, "\"", NULL);
        return TCL_ERROR;
    }
    sprintf(buf, "ot_coll_%u", collnum++);
    if (Tcl_CreateObjCommand(interp, buf, c_process_ot_collection, 
                             coll, destroy_collection) == NULL)
        assert(0);
    Tcl_SetResult(interp, buf, TCL_VOLATILE);
    return TCL_OK;
}

static int c_ordered_tree_collection(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_collection num_master num_trees copy? */
    int i, listlen, copy = 1;
    Ordered_tree_node *topnode;
    Ordered_tree_collection *coll;
    Tcl_Obj **listObjElem;
    char buf[64];

    tcl_objc_range(3, 4);
    if (parse_tree_number(interp, objv[1], &topnode) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_ListObjGetElements(interp, objv[2], &listlen, &listObjElem) != TCL_OK)
        return TCL_ERROR;
    if ((int)ot_n_load_nodes(topnode) != listlen)
    {
        Tcl_SetResult(interp, "incompatible list length", TCL_VOLATILE);
        return TCL_ERROR;
    }
    if (objc > 3)
        if (Tcl_GetBooleanFromObj(interp, objv[3], &copy) != TCL_OK)
            return TCL_ERROR;

    coll = (Ordered_tree_collection *)calloc(1, sizeof(Ordered_tree_collection));
    assert(coll);
    coll->n_trees = listlen;
    if (copy)
        coll->index = ot_copy_node(topnode, NULL, NULL);
    else
        coll->index = topnode;
    coll->trees = (Ordered_tree_node **)calloc(1, coll->n_trees*
                                               sizeof(Ordered_tree_node *));
    assert(coll->trees);

    for (i=0; i<listlen; ++i)
    {
        if (parse_tree_number(interp, listObjElem[i], &topnode) != TCL_OK)
            goto fail;
        if (copy)
            coll->trees[i] = ot_copy_node(topnode, NULL, NULL);
        else
            coll->trees[i] = topnode;
    }
    sprintf(buf, "ot_coll_%u", collnum++);
    if (Tcl_CreateObjCommand(interp, buf, c_process_ot_collection, 
                             coll, destroy_collection) == NULL)
        goto fail;
    Tcl_SetResult(interp, buf, TCL_VOLATILE);
    return TCL_OK;

fail:
    ot_destroy_collection(coll, NULL);
    return TCL_ERROR;
}

static int c_ordered_tree_bindump(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_bindump num filename */
    char *filename;
    Ordered_tree_node *topnode;
    FILE *f;
    XDR xdrs;

    tcl_require_objc(3);
    if (parse_tree_number(interp, objv[1], &topnode) != TCL_OK)
	return TCL_ERROR;
    filename = Tcl_GetStringFromObj(objv[2], NULL);
    f = fopen(filename, "wb");
    if (f == NULL)
    {
        fprintf(stderr, "failed to open file %s", filename);
        fflush(stderr);
        return TCL_ERROR;
    }
    xdrstdio_create(&xdrs, f, XDR_ENCODE);
    ot_binary_dump(&xdrs, topnode, binary_payload_dumper, NULL);
    xdr_destroy(&xdrs);
    fclose(f);
    return TCL_OK;
}

static int c_ordered_tree_dump(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_dump num filename funcname */
    char *filename, *funcname;
    Ordered_tree_node *topnode;

    tcl_require_objc(4);
    if (parse_tree_number(interp, objv[1], &topnode) != TCL_OK)
	return TCL_ERROR;
    filename = Tcl_GetStringFromObj(objv[2], NULL);
    funcname = Tcl_GetStringFromObj(objv[3], NULL);
    ot_dump_c(filename, funcname, topnode, payload_dumper, NULL);
    return TCL_OK;
}

static int c_ordered_tree_shape(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    unsigned i;
    Ordered_tree_node *topnode;
    Ordered_tree_shape *shape;
    Tcl_Obj *result;

    tcl_require_objc(2);
    if (parse_tree_number(interp, objv[1], &topnode) != TCL_OK)
	return TCL_ERROR;
    shape = ot_get_shape(topnode);
    if (shape == 0)
    {
        Tcl_AppendResult(interp, "tree shape is not regular", NULL);
        return TCL_ERROR;
    }
    result = Tcl_NewListObj(0,0);
    for (i=0; i<shape->nlevels; ++i)
    {
        Tcl_Obj *pair[2];
        pair[0] = Tcl_NewIntObj(shape->splits[i].split_dim);
        pair[1] = Tcl_NewIntObj(shape->splits[i].nsplits);
        Tcl_ListObjAppendElement(interp, result, Tcl_NewListObj(2, pair));
    }
    ot_destroy_shape(shape);
    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

static int c_ordered_tree_destroy(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    unsigned i;
    Ordered_tree_node *topnode;

    tcl_require_objc(2);
    if (parse_tree_number(interp, objv[1], &topnode) != TCL_OK)
	return TCL_ERROR;
    for (i=0; i<piecedata_set_size; ++i)
        if (piecedata_set[i] == topnode)
            break;
    assert(i < piecedata_set_size);
    ot_delete_node(topnode, payload_destructor);
    piecedata_set[i] = 0;
    return TCL_OK;
}

static OTPayload payload_divide(const OTPayload i, const OTPayload j,
                                void *userData)
{
    assert(j != 0.0);
    return i / j;
}

static int c_ordered_tree_divide(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_divide num1 num2 */
    int i;
    Ordered_tree_node *nodei, *nodej, *res;

    tcl_require_objc(3);
    if (parse_tree_number(interp, objv[1], &nodei) != TCL_OK)
	return TCL_ERROR;
    if (parse_tree_number(interp, objv[2], &nodej) != TCL_OK)
	return TCL_ERROR;
    res = ot_copy_node(nodei, 0, 0);

    if (ot_combine_payloads(nodei, nodej, res, payload_divide, 0))
    {
        ot_delete_node(res, 0);
        Tcl_SetResult(interp, "incompatible tree structure", TCL_STATIC);
        return TCL_ERROR;
    }

    i = find_unused_index();
    piecedata_set[i] = res;
    Tcl_SetObjResult(interp, Tcl_NewIntObj(i));
    return TCL_OK;
}

static int c_ordered_tree_compare(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_compare num1 num2 eps */
    double eps;
    Ordered_tree_node *nodei, *nodej;

    tcl_require_objc(4);
    if (parse_tree_number(interp, objv[1], &nodei) != TCL_OK)
	return TCL_ERROR;
    if (parse_tree_number(interp, objv[2], &nodej) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[3], &eps) != TCL_OK)
	return TCL_ERROR;
    if (eps < 0.0)
    {
        Tcl_SetResult(interp, "bad tolerance", TCL_VOLATILE);
        return TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(ot_compare(
          nodei, nodej, eps, NULL)));
    return TCL_OK;
}

static void leafbox_lister(void *userData, const Ordered_tree_node *node)
{
    LeafBoxInfo *lbinfo = (LeafBoxInfo *)userData;
    if (node->ndaus == 0)
    {
        Tcl_Obj *leafdata = Tcl_NewListObj(0,0);
        Tcl_Obj *nodecenter = Tcl_NewListObj(0,0);
        Tcl_Obj *bounds = Tcl_NewListObj(0,0);
        float *minvalues, *maxvalues;
        int status;
        unsigned i;

        init_mem(row_data, lbinfo->n_projected_columns*2);
        status = ot_thisnode_center(node, lbinfo->projected_columns,
                                    lbinfo->n_projected_columns, row_data);
        assert(status == 0);

        for (i=0; i<lbinfo->n_projected_columns; ++i)
            Tcl_ListObjAppendElement(lbinfo->interp, nodecenter,
                                     Tcl_NewDoubleObj(row_data[i]));

        minvalues = row_data;
        maxvalues = row_data + lbinfo->n_projected_columns;
        memcpy(minvalues, lbinfo->bb_min, lbinfo->n_projected_columns*sizeof(float));
        memcpy(maxvalues, lbinfo->bb_max, lbinfo->n_projected_columns*sizeof(float));
        ot_thisnode_bounds(node, lbinfo->projected_columns,
                           lbinfo->n_projected_columns, minvalues, maxvalues);
        for (i=0; i<lbinfo->n_projected_columns; ++i)
        {
            Tcl_Obj *thisbound[2];
            thisbound[0] = Tcl_NewDoubleObj(minvalues[i]);
            thisbound[1] = Tcl_NewDoubleObj(maxvalues[i]);
            Tcl_ListObjAppendElement(lbinfo->interp, bounds, Tcl_NewListObj(2, thisbound));
        }
        Tcl_ListObjAppendElement(lbinfo->interp, leafdata, nodecenter);
        Tcl_ListObjAppendElement(lbinfo->interp, leafdata, bounds);
        Tcl_ListObjAppendElement(lbinfo->interp, leafdata, Tcl_NewIntObj(node->payload));
        Tcl_ListObjAppendElement(lbinfo->interp, lbinfo->result, leafdata);
    }
}

static void cell_lister(void *userData, const Ordered_tree_node *node)
{
    CellListInfo *mylist = (CellListInfo *)userData;
    if (node->ndaus == 0)
    {
        int status = ot_thisnode_center(node, mylist->projected_columns,
                                        mylist->n_projected_columns, mylist->buf);
        if (status == 0)
        {
            Tcl_Obj *nodecenter = Tcl_NewListObj(0,0);
            unsigned i;
            for (i=0; i<mylist->n_projected_columns; ++i)
                Tcl_ListObjAppendElement(mylist->interp, nodecenter,
                                         Tcl_NewDoubleObj(mylist->buf[i]));
            Tcl_ListObjAppendElement(mylist->interp, mylist->result, nodecenter);
            Tcl_ListObjAppendElement(mylist->interp, mylist->result,
                                     Tcl_NewDoubleObj(node->payload));
        }
    }
}

static int c_ordered_tree_leaf_boxes(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_leaf_boxes tree_id dimensions tree_bounding_box */
    int bb_ndim = 0, i, listlen;
    float bb_min[MAX_DIM_NUMBER], bb_max[MAX_DIM_NUMBER];
    unsigned columns[MAX_DIM_NUMBER];
    Ordered_tree_node *topnode;
    LeafBoxInfo lbinfo;
    Tcl_Obj **listObjElem;

    tcl_require_objc(4);
    if (parse_tree_number(interp, objv[1], &topnode) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_ListObjGetElements(interp, objv[2], &listlen, &listObjElem) != TCL_OK)
        return TCL_ERROR;
    if (listlen > MAX_DIM_NUMBER)
    {
        Tcl_SetResult(interp, "list of dimensions is too long",
                      TCL_STATIC);
        return TCL_ERROR;
    }
    for (i=0; i<listlen; ++i)
    {
        int j;
        if (Tcl_GetIntFromObj(interp, listObjElem[i], &j) != TCL_OK)
            return TCL_ERROR;
        if (j < 0)
        {
            Tcl_SetResult(interp, "dimension numbers can not be negative",
                          TCL_STATIC);
            return TCL_ERROR;
        }
        columns[i] = j;
    }
    if (parse_bounding_box(interp, MAX_DIM_NUMBER, objv[3],
                           &bb_ndim, bb_min, bb_max) != TCL_OK)
        return TCL_ERROR;
    if (bb_ndim != listlen)
    {
        Tcl_SetResult(interp, "bounding box is not compatible with the list of dimensions",
                      TCL_STATIC);
        return TCL_ERROR;
    }

    lbinfo.interp  = interp;
    lbinfo.result  = Tcl_NewListObj(0,0);
    lbinfo.projected_columns = columns;
    lbinfo.n_projected_columns = bb_ndim;
    lbinfo.bb_min  = bb_min;
    lbinfo.bb_max  = bb_max;

    ot_walk(topnode, leafbox_lister, &lbinfo, NULL, NULL);

    Tcl_SetObjResult(interp, lbinfo.result);
    return TCL_OK;
}

static int c_ordered_tree_cells(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_cells tree_id dimension_list */
    Ordered_tree_node *topnode;
    unsigned *projected_columns = 0;
    float *buf = 0;
    int i, listlen;
    Tcl_Obj **listObjElem;
    Tcl_Obj *result;
    CellListInfo clinfo;

    tcl_require_objc(3);
    if (parse_tree_number(interp, objv[1], &topnode) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_ListObjGetElements(interp, objv[2], &listlen, &listObjElem) != TCL_OK)
        goto fail;
    if (listlen == 0)
    {
        Tcl_SetResult(interp, "the list of dimensions is empty", TCL_STATIC);
        goto fail;
    }
    projected_columns = (unsigned *)malloc(listlen*sizeof(unsigned));
    buf = (float *)malloc(listlen*sizeof(float));
    if (projected_columns == NULL || buf == NULL)
    {
        Tcl_SetResult(interp, "out of memory", TCL_STATIC);
        goto fail;
    }
    for (i=0; i<listlen; ++i)
    {
        int icol;
        if (Tcl_GetIntFromObj(interp, listObjElem[i], &icol) != TCL_OK)
            goto fail;
        if (icol < 0)
        {
            Tcl_SetResult(interp, "dimension index out of range", TCL_STATIC);
            goto fail;
        }
        projected_columns[i] = icol;
    }
    result = Tcl_NewListObj(0,0);

    clinfo.interp = interp;
    clinfo.result = result;
    clinfo.projected_columns = projected_columns;
    clinfo.n_projected_columns = listlen;
    clinfo.buf = buf;

    ot_walk(topnode, cell_lister, &clinfo, NULL, NULL);

    Tcl_SetObjResult(interp, result);
    free(buf);
    free(projected_columns);
    return TCL_OK;

 fail:
    if (buf)
        free(buf);
    if (projected_columns)
        free(projected_columns);
    return TCL_ERROR;
}

static void bin_centers(float xmin, const float xmax,
                        const int nbins, float *centers)
{
    const float step = (xmax - xmin)/nbins;
    int i;

    assert(nbins > 0);
    xmin += step/2.0;
    for (i=0; i<nbins; ++i)
        centers[i] = xmin + i*step;
}

static float box_area(const float *minb, const float *maxb,
                      const int ndim)
{
    double area = 1.0;
    int i;
    for (i=0; i<ndim; ++i)
        area *= (maxb[i] - minb[i]);
    return area;
}

static int c_ordered_tree_fill_fromntuple(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_fill_fromntuple tree_id ntuple_id weight_column? */
    Ordered_tree_node *topnode;
    OTPayload *pload;
    int ntuple_id, ncols, nrows, i, weight_column = -1;
    float *buf;

    tcl_objc_range(3,4);
    if (parse_tree_number(interp, objv[1], &topnode) != TCL_OK)
	return TCL_ERROR;
    verify_ntuple(ntuple_id, 2);
    ncols = hs_num_variables(ntuple_id);
    if (objc > 3)
    {
        if (Tcl_GetIntFromObj(interp, objv[3], &weight_column) != TCL_OK)
            return TCL_ERROR;
        if (weight_column < 0 || weight_column >= ncols)
        {
            Tcl_SetResult(interp, "weight column number is out of range",
                          TCL_STATIC);
            return TCL_ERROR;
        }
    }
    buf = (float *)malloc(ncols*sizeof(float));
    if (buf == NULL)
    {
        Tcl_SetResult(interp, "out of memory", TCL_STATIC);
        return TCL_ERROR;        
    }
    nrows = hs_num_entries(ntuple_id);
    for (i=0; i<nrows; ++i)
    {
        hs_row_contents(ntuple_id, i, buf);
        pload = ot_find_payload(topnode, buf, INT_MAX);
        if (weight_column >= 0)
            *pload += buf[weight_column];
        else
            *pload += 1;
    }
    free(buf);
    return TCL_OK;
}

static void payload_resetter(void *userData, OTPayload *p)
{
    *p = 0;
}

static int c_ordered_tree_copy(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_copy tree_id */
    int i;
    Ordered_tree_node *topnode, *copy;
    tcl_require_objc(2);
    if (parse_tree_number(interp, objv[1], &topnode) != TCL_OK)
	return TCL_ERROR;
    copy = ot_copy_node(topnode, 0, 0);
    assert(copy);
    i = find_unused_index();
    piecedata_set[i] = copy;
    Tcl_SetObjResult(interp, Tcl_NewIntObj(i));
    return TCL_OK;
}

static OTPayload payload_scaler(const OTPayload orig, void *userData)
{
    double scale = *((double *)userData);
    return orig*scale;
}

static int c_ordered_tree_scale(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_scale tree_id scale */
    int i;
    Ordered_tree_node *topnode, *copy;
    double scale;

    tcl_require_objc(3);
    if (parse_tree_number(interp, objv[1], &topnode) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[2], &scale) != TCL_OK)
	return TCL_ERROR;
    copy = ot_copy_node(topnode, payload_scaler, &scale);
    assert(copy);
    i = find_unused_index();
    piecedata_set[i] = copy;
    Tcl_SetObjResult(interp, Tcl_NewIntObj(i));
    return TCL_OK;
}

static int c_ordered_tree_reset(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_reset tree_id */
    Ordered_tree_node *topnode;
    tcl_require_objc(2);
    if (parse_tree_number(interp, objv[1], &topnode) != TCL_OK)
	return TCL_ERROR;
    ot_walk(topnode, NULL, NULL, payload_resetter, NULL);
    return TCL_OK;
}

static void payload_summer(void *userData, OTPayload *p)
{
    double *psum = (double *)userData;
    *psum += *p;
}

static void payload_stdev_calc(void *userData, OTPayload *p)
{
    double *pmeanstdev = (double *)userData;
    double diff = *p - pmeanstdev[0];
    pmeanstdev[1] += diff*diff;
}

static int c_ordered_tree_n_payloads(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_n_payloads tree_id */
    Ordered_tree_node *topnode;
    int n_payloads;

    tcl_require_objc(2);
    if (parse_tree_number(interp, objv[1], &topnode) != TCL_OK)
	return TCL_ERROR;
    n_payloads = ot_n_load_nodes(topnode);
    Tcl_SetObjResult(interp, Tcl_NewIntObj(n_payloads));
    return TCL_OK;
}

static int c_ordered_tree_extremum_bounds(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_extremum_bounds tree_id */
    unsigned i;
    float bb_min[MAX_DIM_NUMBER], bb_max[MAX_DIM_NUMBER];
    Ordered_tree_node *topnode;
    Tcl_Obj *result;

    tcl_require_objc(2);
    if (parse_tree_number(interp, objv[1], &topnode) != TCL_OK)
	return TCL_ERROR;
    for (i=0; i<MAX_DIM_NUMBER; ++i)
    {
        bb_min[i] = FLT_MAX;
        bb_max[i] = -FLT_MAX;
    }
    ot_extremum_bounds(topnode, bb_min, bb_max);
    result = Tcl_NewListObj(0,0);
    for (i=0; i<MAX_DIM_NUMBER; ++i)
    {
        Tcl_Obj *pair[2];
        if (bb_min[i] > FLT_MAX*0.99999f && bb_max[i] < -FLT_MAX*0.99999f)
            break;
        pair[0] = Tcl_NewDoubleObj(bb_min[i]);
        pair[1] = Tcl_NewDoubleObj(bb_max[i]);
        Tcl_ListObjAppendElement(interp, result, Tcl_NewListObj(2, pair));
    }
    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

static int c_ordered_tree_payload_stats(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_payload_stats tree_id
     *
     * Returns 4-element list {n_payloads sum mean stdev}
     */
    Ordered_tree_node *topnode;
    unsigned n_payloads;
    double sum = 0.0, meanstdev[2] = {0.0, 0.0};
    Tcl_Obj *result;

    tcl_require_objc(2);
    if (parse_tree_number(interp, objv[1], &topnode) != TCL_OK)
	return TCL_ERROR;
    n_payloads = ot_n_load_nodes(topnode);
    ot_walk(topnode, NULL, NULL, payload_summer, &sum);
    meanstdev[0] = sum/n_payloads;
    ot_walk(topnode, NULL, NULL, payload_stdev_calc, meanstdev);
    meanstdev[1] = sqrt(meanstdev[1]/(n_payloads - 1));
    
    result = Tcl_NewListObj(0,0);
    Tcl_ListObjAppendElement(interp, result, Tcl_NewIntObj(n_payloads));
    Tcl_ListObjAppendElement(interp, result, Tcl_NewDoubleObj(sum));
    Tcl_ListObjAppendElement(interp, result, Tcl_NewDoubleObj(meanstdev[0]));
    Tcl_ListObjAppendElement(interp, result, Tcl_NewDoubleObj(meanstdev[1]));
    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

static int random_row(int maxrows)
{
    int i;
    do {
        double f = uniform_random();
        i = (int)(maxrows*f);
    } while(i < 0 || i >= maxrows);
    return i;
}

static int c_resample_ntuple_rows(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: resample_ntuple_rows id nsamples uid title category */
    int i, id, id1 = -1, npoints, nsamples, uid1;
    char *title1, *category1;

    tcl_require_objc(6);
    verify_ntuple(id,1);
    npoints = hs_num_entries(id);
    if (npoints == 0)
    {
        Tcl_AppendResult(interp, "ntuple with id ",
                         Tcl_GetStringFromObj(objv[1], NULL),
                         " is empty", NULL);
        return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, objv[2], &nsamples) != TCL_OK)
        return TCL_ERROR;
    if (nsamples < 0)
    {
        Tcl_SetResult(interp, "number of samples can not be negative",
                      TCL_STATIC);
        return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, objv[3], &uid1) != TCL_OK)
        return TCL_ERROR;
    title1 = Tcl_GetStringFromObj(objv[4], NULL);
    category1 = Tcl_GetStringFromObj(objv[5], NULL);
    id1 = hs_duplicate_ntuple_header(id, uid1, title1, category1);
    if (id1 <= 0)
    {
	Tcl_SetResult(interp, "failed to create new ntuple", TCL_STATIC);
	return TCL_ERROR;
    }
    init_mem(row_data, hs_num_variables(id));
    for (i=0; i<nsamples; ++i)
    {
        hs_row_contents(id, random_row(npoints), row_data);
        if (hs_fill_ntuple(id1, row_data) != id1)
        {
            Tcl_SetResult(interp, "out of memory", TCL_STATIC);
            goto fail;
        }
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(id1));
    return TCL_OK;

 fail:
    if (id1 > 0)
        hs_delete(id1);
    return TCL_ERROR;
}

static int c_ordered_tree_payload(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_payload tree_id coords new_value?
     *
     *  If "new_value" argument is provided, the payload
     *  will be set, otherwise existing payload is returned.
     *
     *  In the current implementation, "new_value" can be either
     *  a double or a binary string.
     */
    static const Tcl_ObjType * byteArrayTypePtr = NULL;
    Ordered_tree_node *topnode;
    float local_coords[20];
    float *coords = local_coords;
    int i, listlen, status = TCL_ERROR;
    Tcl_Obj **listObjElem;
    double value;
    OTPayload* pay;

    if (byteArrayTypePtr == NULL)
	byteArrayTypePtr = Tcl_GetObjType("bytearray");

    tcl_objc_range(3, 4);
    if (parse_tree_number(interp, objv[1], &topnode) != TCL_OK)
	goto fail;
    if (Tcl_ListObjGetElements(interp, objv[2], &listlen, &listObjElem) != TCL_OK)
        goto fail;
    if (listlen == 0)
    {
        Tcl_SetResult(interp, "empty coordinate list", TCL_STATIC);
        goto fail;
    }
    if (listlen > (int)(sizeof(local_coords)/sizeof(local_coords[0])))
    {
        coords = (float *)malloc(listlen*sizeof(float));
        assert(coords);
    }
    for (i=0; i<listlen; ++i)
    {
        if (Tcl_GetDoubleFromObj(interp, listObjElem[i], &value) != TCL_OK)
            goto fail;
        coords[i] = value;
    }
    pay = ot_find_payload(topnode, coords, INT_MAX);
    assert(pay);
    if (objc > 3)
    {
        if (objv[3]->typePtr == byteArrayTypePtr)
        {
            void* data;
            i = 0;
            data = Tcl_GetByteArrayFromObj(objv[3], &i);
            if (i != sizeof(OTPayload))
            {
                Tcl_SetResult(interp, "invalid byte array length", TCL_STATIC);
                goto fail;
            }
            memcpy(pay, data, sizeof(OTPayload));
        }
        else 
        {
            if (Tcl_GetDoubleFromObj(interp, objv[3], &value) != TCL_OK)
                goto fail;
            *pay = value;
        }
        Tcl_SetObjResult(interp, objv[3]);
    }
    else
        Tcl_SetObjResult(interp, Tcl_NewDoubleObj(*pay));

    status = TCL_OK;
fail:
    if (coords != local_coords)
        free(coords);
    return status;
}

static int c_ordered_tree_payload_scan(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_payload_scan tree_id histo_id \
     *    histo_dim_numbers remaining_dim_values scale interpolate?
     *
     * histo_dim_numbers is a list of dimensions to which
     * x, y, ... histogram axes should be mapped. The length
     * of the list should be equal to the histogram dimensionality.
     * Example: {1 2}
     *
     * remaining_dim_values is a list of lists of values for
     * the remaining tree dimensions. The dimension numbers 
     * should coincide with the numbers used when the tree
     * was created. Example: {{0 1.0} {3 4.5}}
     *
     * scale is just a number by which all payloads will
     * be multiplied.
     *
     * The last parameter says whether the payloads should
     * be interpolated or not.
     */
    Ordered_tree_node *topnode;
    int i, j, k, histo_id, histo_dim, status = TCL_ERROR;
    float xmin, xmax, ymin, ymax, zmin, zmax;
    int listlen, nbins, n_x_bins = 0, n_y_bins = 0, n_z_bins = 0;
    float *mem = 0, *xbins, *ybins = 0, *zbins = 0, *index;
    Tcl_Obj **listObjElem;
    int max_dim = -1, histo_dims[3], interpolate = 0;
    int *other_dims = 0;
    double scale;
    OTPayload *ppay;

    tcl_objc_range(6, 7);
    if (parse_tree_number(interp, objv[1], &topnode) != TCL_OK)
	goto fail;
    if (Tcl_GetIntFromObj(interp, objv[2], &histo_id) != TCL_OK)
	goto fail;
    switch (hs_type(histo_id))
    {
    case HS_NONE:
        Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[2], NULL),
                         " is not a valid Histo-Scope id", NULL);
        goto fail;
    case HS_1D_HISTOGRAM:
        histo_dim = 1;
        break;
    case HS_2D_HISTOGRAM:
        histo_dim = 2;
        break;
    case HS_3D_HISTOGRAM:
        histo_dim = 3;
        break;
    default:
        Tcl_AppendResult(interp, "item with id ", 
                         Tcl_GetStringFromObj(objv[2], NULL),
                         " is not a histogram", NULL);
        goto fail;
    }

    if (Tcl_ListObjGetElements(interp, objv[3], &listlen, &listObjElem) != TCL_OK)
        goto fail;
    if (listlen != histo_dim)
    {
        Tcl_SetResult(interp, "wrong length of histogram dimension mapping", TCL_VOLATILE);
        goto fail;
    }
    for (i=0; i<listlen; ++i)
    {
        if (Tcl_GetIntFromObj(interp, listObjElem[i], histo_dims+i) != TCL_OK)
            goto fail;
        if (histo_dims[i] < 0)
        {
            Tcl_SetResult(interp, "invalid histogram dimension mapping", TCL_VOLATILE);
            goto fail;
        }
        if (histo_dims[i] > max_dim)
            max_dim = histo_dims[i];
        for (j=0; j<i; ++j)
            if (histo_dims[j] == histo_dims[i])
            {
                Tcl_SetResult(interp, "duplicate dimension detected", TCL_VOLATILE);
                goto fail;
            }
    }

    if (Tcl_ListObjGetElements(interp, objv[4], &listlen, &listObjElem) != TCL_OK)
        goto fail;
    other_dims = (int *)malloc(listlen*sizeof(int));
    if (other_dims == NULL)
    {
        Tcl_SetResult(interp, "out of memory", TCL_STATIC);
        goto fail;        
    }
    for (i=0; i<listlen; ++i)
    {
        int two, dim;
        Tcl_Obj **pair;

        if (Tcl_ListObjGetElements(interp, listObjElem[i], &two, &pair) != TCL_OK)
            goto fail;
        if (two != 2)
        {
            Tcl_SetResult(interp, "invalid slice specification", TCL_VOLATILE);
            goto fail;
        }
        if (Tcl_GetIntFromObj(interp, pair[0], &dim) != TCL_OK)
            goto fail;
        if (dim < 0)
        {
            Tcl_SetResult(interp, "invalid slice dimension", TCL_VOLATILE);
            goto fail;
        }
        if (dim > max_dim)
            max_dim = dim;
        other_dims[i] = dim;
        for (j=0; j<histo_dim; ++j)
            if (histo_dims[j] == dim)
            {
                Tcl_SetResult(interp, "duplicate dimension detected", TCL_VOLATILE);
                goto fail;
            }
        for (j=0; j<i; ++j)
            if (other_dims[j] == dim)
            {
                Tcl_SetResult(interp, "duplicate dimension detected", TCL_VOLATILE);
                goto fail;
            }
    }

    if (Tcl_GetDoubleFromObj(interp, objv[5], &scale) != TCL_OK)
	goto fail;

    if (objc > 6)
        if (Tcl_GetBooleanFromObj(interp, objv[6], &interpolate) != TCL_OK)
            goto fail;

    switch (histo_dim)
    {
    case 1:
        n_x_bins = hs_1d_hist_num_bins(histo_id);
        hs_1d_hist_range(histo_id, &xmin, &xmax);
        nbins = n_x_bins;
        break;
    case 2:
        hs_2d_hist_num_bins(histo_id, &n_x_bins, &n_y_bins);
        hs_2d_hist_range(histo_id, &xmin, &xmax, &ymin, &ymax);
        nbins = n_x_bins*n_y_bins;
        break;
    case 3:
        hs_3d_hist_num_bins(histo_id, &n_x_bins, &n_y_bins, &n_z_bins);
        hs_3d_hist_range(histo_id, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
        nbins = n_x_bins*n_y_bins*n_z_bins;
        break;
    default:
        assert(0);
    }

    assert(max_dim >= 0);
    mem = (float *)malloc((nbins + n_x_bins + n_y_bins + n_z_bins + max_dim + 1)*sizeof(float));
    if (mem == NULL)
    {
        Tcl_SetResult(interp, "out of memory", TCL_STATIC);
        goto fail;        
    }
    memset(mem, 0, (nbins + max_dim + 1)*sizeof(float));
    index = mem + nbins;
    xbins = index + max_dim + 1;
    bin_centers(xmin, xmax, n_x_bins, xbins);
    if (histo_dim > 1)
    {
        ybins = xbins + n_x_bins;
        bin_centers(ymin, ymax, n_y_bins, ybins);
    }
    if (histo_dim > 2)
    {
        zbins = ybins + n_y_bins;
        bin_centers(zmin, zmax, n_z_bins, zbins);
    }

    for (i=0; i<listlen; ++i)
    {
        int two;
        double slice_value;
        Tcl_Obj **pair;

        if (Tcl_ListObjGetElements(interp, listObjElem[i], &two, &pair) != TCL_OK)
            goto fail;
        assert(two == 2);
        if (Tcl_GetDoubleFromObj(interp, pair[1], &slice_value) != TCL_OK)
            goto fail;
        index[other_dims[i]] = slice_value;
    }

    switch (histo_dim)
    {
        case 1:
            for (i=0; i<n_x_bins; ++i)
            {
                index[histo_dims[0]] = xbins[i];
                if (interpolate)
                {
                    mem[i] = scale * ot_interpolate_payload(topnode, index);
                }
                else
                {
                    ppay = ot_find_payload(topnode, index, INT_MAX);
                    mem[i] = *ppay * scale;
                }
            }
            hs_1d_hist_block_fill(histo_id, mem, NULL, NULL);
            break;

        case 2:
            for (i=0; i<n_x_bins; ++i)
            {
                index[histo_dims[0]] = xbins[i];
                for (j=0; j<n_y_bins; ++j)
                {
                    index[histo_dims[1]] = ybins[j];
                    if (interpolate)
                    {
                        mem[i*n_y_bins + j] = scale * 
                            ot_interpolate_payload(topnode, index);
                    }
                    else
                    {
                        ppay = ot_find_payload(topnode, index, INT_MAX);
                        mem[i*n_y_bins + j] = *ppay * scale;
                    }
                }
            }
            hs_2d_hist_block_fill(histo_id, mem, NULL, NULL);
            break;

        case 3:
            for (i=0; i<n_x_bins; ++i)
            {
                index[histo_dims[0]] = xbins[i];
                for (j=0; j<n_y_bins; ++j)
                {
                    index[histo_dims[1]] = ybins[j];
                    for (k=0; k<n_z_bins; ++k)
                    {
                        index[histo_dims[2]] = zbins[k];
                        if (interpolate)
                        {
                            mem[(i*n_y_bins + j)*n_z_bins + k] = scale *
                                ot_interpolate_payload(topnode, index);
                        }
                        else
                        {
                            ppay = ot_find_payload(topnode, index, INT_MAX);
                            mem[(i*n_y_bins + j)*n_z_bins + k] = *ppay * scale;
                        }
                    }
                }
            }
            hs_3d_hist_block_fill(histo_id, mem, NULL, NULL);
            break;

        default:
            assert(0);
    }

    status = TCL_OK;
 fail:
    if (mem)
        free(mem);
    if (other_dims)
        free(other_dims);
    return status;
}

static int c_ordered_tree_to_from_uniform(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[],
    void (*fcn_to_call)(const Ordered_tree_node *topnode,
                        const float *input, const float *min_bound_in,
                        const float *max_bound_in, float *output))
{
    Ordered_tree_node *topnode;
    int bb_ndim = 0, i, listlen;
    Tcl_Obj **listObjElem;
    float coords[MAX_DIM_NUMBER], u[MAX_DIM_NUMBER] = {0};
    float bb_min[MAX_DIM_NUMBER], bb_max[MAX_DIM_NUMBER];
    Tcl_Obj *result;

    tcl_require_objc(4);
    if (parse_tree_number(interp, objv[1], &topnode) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_ListObjGetElements(interp, objv[2], &listlen, &listObjElem) != TCL_OK)
        return TCL_ERROR;
    if (listlen <= 0)
    {
        Tcl_SetResult(interp, "list of coordinates is empty", TCL_STATIC);
        return TCL_ERROR;
    }
    if (listlen > MAX_DIM_NUMBER)
    {
        Tcl_SetResult(interp, "list of coordinates is too long", TCL_STATIC);
        return TCL_ERROR;
    }
    for (i=0; i<listlen; ++i)
    {
        double d;
        if (Tcl_GetDoubleFromObj(interp, listObjElem[i], &d) != TCL_OK)
            return TCL_ERROR;
        coords[i] = d;
    }
    if (parse_bounding_box(interp, MAX_DIM_NUMBER, objv[3],
                           &bb_ndim, bb_min, bb_max) != TCL_OK)
        return TCL_ERROR;
    if (bb_ndim != listlen)
    {
        Tcl_SetResult(interp, "incompatible bounding box dimensionality",
                      TCL_STATIC);
        return TCL_ERROR;
    }
    fcn_to_call(topnode, coords, bb_min, bb_max, u);
    result = Tcl_NewListObj(0,0);
    for (i=0; i<listlen; ++i)
        Tcl_ListObjAppendElement(interp, result, Tcl_NewDoubleObj(u[i]));
    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

static int c_ordered_tree_to_uniform(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_to_uniform tree_number coords bounding_box */
    return c_ordered_tree_to_from_uniform(clientData, interp, 
                                          objc, objv, ot_to_uniform);
}

static int c_ordered_tree_from_uniform(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_from_uniform tree_number uni_vector bounding_box */
    return c_ordered_tree_to_from_uniform(clientData, interp, 
                                          objc, objv, ot_from_uniform);
}

static int c_ordered_tree_polygon_random(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_polygon_random tree_number uni_vector bounding_box */
    return c_ordered_tree_to_from_uniform(clientData, interp, 
                                          objc, objv, ot_polygon_random);
}

static int c_ordered_tree_density(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_density tree_id_list histo_id ?bounding_box? */
    Ordered_tree_node **topnodes = 0;
    int i, j, k, itop, histo_id, histo_dim, bb_ndim = 0, status = TCL_ERROR;
    float bb_min[3], bb_max[3], xvals[3], minbounds[3], maxbounds[3];
    float xmin, xmax, ymin, ymax, zmin, zmax, area;
    int listlen, nbins, n_x_bins = 0, n_y_bins = 0, n_z_bins = 0;
    float *mem = 0, *xbins, *ybins = 0, *zbins = 0;
    Tcl_Obj **listObjElem;
    unsigned *n_cells = 0;

    tcl_objc_range(3,4);
    if (Tcl_ListObjGetElements(interp, objv[1], &listlen, &listObjElem) != TCL_OK)
        return TCL_ERROR;
    topnodes = (Ordered_tree_node **)malloc(listlen*sizeof(Ordered_tree_node *));
    n_cells = (unsigned *)malloc(listlen*sizeof(unsigned));
    if (topnodes == NULL || n_cells == NULL)
    {
        Tcl_SetResult(interp, "out of memory", TCL_STATIC);
        return TCL_ERROR;        
    }
    for (i=0; i<listlen; ++i)
    {
        if (parse_tree_number(interp, listObjElem[i], topnodes+i) != TCL_OK)
            goto fail;
        n_cells[i] = ot_n_load_nodes(topnodes[i]);
        if (n_cells[i] == 0)
        {
            Tcl_SetResult(interp, "detected a tree without payloads", TCL_STATIC);
            goto fail;
        }
    }
    if (Tcl_GetIntFromObj(interp, objv[2], &histo_id) != TCL_OK)
	goto fail;
    switch (hs_type(histo_id))
    {
    case HS_NONE:
        Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[2], NULL),
                         " is not a valid Histo-Scope id", NULL);
        goto fail;
    case HS_1D_HISTOGRAM:
        histo_dim = 1;
        break;
    case HS_2D_HISTOGRAM:
        histo_dim = 2;
        break;
    case HS_3D_HISTOGRAM:
        histo_dim = 3;
        break;
    default:
        Tcl_AppendResult(interp, "item with id ", 
                         Tcl_GetStringFromObj(objv[2], NULL),
                         " is not a histogram", NULL);
        goto fail;
    }
    if (objc > 3)
    {
        if (parse_bounding_box(interp, 3, objv[3], &bb_ndim, bb_min, bb_max) != TCL_OK)
            goto fail;
        if (bb_ndim != histo_dim)
        {
            Tcl_SetResult(interp, "incompatible bounding box dimensionality", TCL_STATIC);
            goto fail;
        }
    }

    switch (histo_dim)
    {
    case 1:
        n_x_bins = hs_1d_hist_num_bins(histo_id);
        hs_1d_hist_range(histo_id, &xmin, &xmax);
        nbins = n_x_bins;
        break;
    case 2:
        hs_2d_hist_num_bins(histo_id, &n_x_bins, &n_y_bins);
        hs_2d_hist_range(histo_id, &xmin, &xmax, &ymin, &ymax);
        nbins = n_x_bins*n_y_bins;
        break;
    case 3:
        hs_3d_hist_num_bins(histo_id, &n_x_bins, &n_y_bins, &n_z_bins);
        hs_3d_hist_range(histo_id, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
        nbins = n_x_bins*n_y_bins*n_z_bins;
        break;
    default:
        assert(0);
    }

    if (bb_ndim == 0)
    {
        bb_ndim = histo_dim;
        bb_min[0] = xmin;
        bb_max[0] = xmax;
        bb_min[1] = ymin;
        bb_max[1] = ymax;
        bb_min[2] = zmin;
        bb_max[2] = zmax;
    }

    mem = (float *)malloc((nbins + n_x_bins + n_y_bins + n_z_bins)*sizeof(float));
    if (mem == NULL)
    {
        Tcl_SetResult(interp, "out of memory", TCL_STATIC);
        goto fail;        
    }
    memset(mem, 0, nbins*sizeof(float));
    xbins = mem + nbins;
    bin_centers(xmin, xmax, n_x_bins, xbins);
    if (histo_dim > 1)
    {
        ybins = xbins + n_x_bins;
        bin_centers(ymin, ymax, n_y_bins, ybins);
    }
    if (histo_dim > 2)
    {
        zbins = ybins + n_y_bins;
        bin_centers(zmin, zmax, n_z_bins, zbins);
    }

    for (itop=0; itop<listlen; ++itop)
    {
        const float n_cells_norm = 1.f/(float)listlen/(float)n_cells[itop];
        const Ordered_tree_node *topnode = topnodes[itop];

        switch (histo_dim)
        {
        case 1:
            for (i=0; i<n_x_bins; ++i)
            {
                xvals[0] = xbins[i];
                minbounds[0] = bb_min[0];
                maxbounds[0] = bb_max[0];
                ot_find_bounds(topnode, xvals, INT_MAX, minbounds, maxbounds);
                area = box_area(minbounds, maxbounds, histo_dim);
                if (area > 0.f)
                    mem[i] += n_cells_norm/area;
                else
                {
                    assert(area == 0.f);
                    Tcl_SetResult(interp, "zero cell size detected", TCL_STATIC);
                    goto fail;        
                }
            }
            break;
        case 2:
            for (i=0; i<n_x_bins; ++i)
            {
                xvals[0] = xbins[i];
                for (j=0; j<n_y_bins; ++j)
                {
                    xvals[1] = ybins[j];
                    minbounds[0] = bb_min[0];
                    maxbounds[0] = bb_max[0];
                    minbounds[1] = bb_min[1];
                    maxbounds[1] = bb_max[1];
                    ot_find_bounds(topnode, xvals, INT_MAX, minbounds, maxbounds);
                    area = box_area(minbounds, maxbounds, histo_dim);
                    if (area > 0.f)
                        mem[i*n_y_bins + j] += n_cells_norm/area;
                    else
                    {
                        assert(area == 0.f);
                        Tcl_SetResult(interp, "zero cell size detected", TCL_STATIC);
                        goto fail;        
                    }
                }
            }
            break;
        case 3:
            for (i=0; i<n_x_bins; ++i)
            {
                xvals[0] = xbins[i];
                for (j=0; j<n_y_bins; ++j)
                {
                    xvals[1] = ybins[j];
                    for (k=0; k<n_z_bins; ++k)
                    {
                        xvals[2] = zbins[k];
                        minbounds[0] = bb_min[0];
                        maxbounds[0] = bb_max[0];
                        minbounds[1] = bb_min[1];
                        maxbounds[1] = bb_max[1];
                        minbounds[2] = bb_min[2];
                        maxbounds[2] = bb_max[2];
                        ot_find_bounds(topnode, xvals, INT_MAX, minbounds, maxbounds);
                        area = box_area(minbounds, maxbounds, histo_dim);
                        if (area > 0.f)
                            mem[(i*n_y_bins + j)*n_z_bins + k] += n_cells_norm/area;
                        else
                        {
                            assert(area == 0.f);
                            Tcl_SetResult(interp, "zero cell size detected", TCL_STATIC);
                            goto fail;        
                        }
                    }
                }
            }
            break;
        default:
            assert(0);
        }
    }

    switch (histo_dim)
    {
    case 1:
        hs_1d_hist_block_fill(histo_id, mem, NULL, NULL);
        break;
    case 2:
        hs_2d_hist_block_fill(histo_id, mem, NULL, NULL);
        break;
    case 3:
        hs_3d_hist_block_fill(histo_id, mem, NULL, NULL);
        break;
    default:
        assert(0);
    }

    status = TCL_OK;
 fail:
    if (mem)
        free(mem);
    if (n_cells)
        free(n_cells);
    if (topnodes)
        free(topnodes);
    return status;
}

static int c_ordered_tree_density_2(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_density_2 tree_id_list histo_id ?bounding_box? */
    Ordered_tree_node **topnodes = 0;
    int i, j, k, itop, histo_id, histo_dim, bb_ndim = 0, status = TCL_ERROR;
    float bb_min[3], bb_max[3], xvals[3], minbounds[3], maxbounds[3];
    float xmin, xmax, ymin, ymax, zmin, zmax;
    int listlen, nbins, n_x_bins = 0, n_y_bins = 0, n_z_bins = 0;
    float *mem = 0, *xbins, *ybins = 0, *zbins = 0;
    Tcl_Obj **listObjElem;
    unsigned *n_cells = 0;

    tcl_objc_range(3,4);
    if (Tcl_ListObjGetElements(interp, objv[1], &listlen, &listObjElem) != TCL_OK)
        return TCL_ERROR;
    topnodes = (Ordered_tree_node **)malloc(listlen*sizeof(Ordered_tree_node *));
    n_cells = (unsigned *)malloc(listlen*sizeof(unsigned));
    if (topnodes == NULL || n_cells == NULL)
    {
        Tcl_SetResult(interp, "out of memory", TCL_STATIC);
        return TCL_ERROR;        
    }
    for (i=0; i<listlen; ++i)
    {
        if (parse_tree_number(interp, listObjElem[i], topnodes+i) != TCL_OK)
            goto fail;
        n_cells[i] = ot_n_load_nodes(topnodes[i]);
        if (n_cells[i] == 0)
        {
            Tcl_SetResult(interp, "detected a tree without payloads", TCL_STATIC);
            goto fail;
        }
    }
    if (Tcl_GetIntFromObj(interp, objv[2], &histo_id) != TCL_OK)
	goto fail;
    switch (hs_type(histo_id))
    {
    case HS_NONE:
        Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[2], NULL),
                         " is not a valid Histo-Scope id", NULL);
        goto fail;
    case HS_1D_HISTOGRAM:
        histo_dim = 1;
        break;
    case HS_2D_HISTOGRAM:
        histo_dim = 2;
        break;
    case HS_3D_HISTOGRAM:
        histo_dim = 3;
        break;
    default:
        Tcl_AppendResult(interp, "item with id ", 
                         Tcl_GetStringFromObj(objv[2], NULL),
                         " is not a histogram", NULL);
        goto fail;
    }
    if (objc > 3)
    {
        if (parse_bounding_box(interp, 3, objv[3], &bb_ndim, bb_min, bb_max) != TCL_OK)
            goto fail;
        if (bb_ndim != histo_dim)
        {
            Tcl_SetResult(interp, "incompatible bounding box dimensionality", TCL_STATIC);
            goto fail;
        }
    }

    switch (histo_dim)
    {
    case 1:
        n_x_bins = hs_1d_hist_num_bins(histo_id);
        hs_1d_hist_range(histo_id, &xmin, &xmax);
        nbins = n_x_bins;
        break;
    case 2:
        hs_2d_hist_num_bins(histo_id, &n_x_bins, &n_y_bins);
        hs_2d_hist_range(histo_id, &xmin, &xmax, &ymin, &ymax);
        nbins = n_x_bins*n_y_bins;
        break;
    case 3:
        hs_3d_hist_num_bins(histo_id, &n_x_bins, &n_y_bins, &n_z_bins);
        hs_3d_hist_range(histo_id, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
        nbins = n_x_bins*n_y_bins*n_z_bins;
        break;
    default:
        assert(0);
    }

    if (bb_ndim == 0)
    {
        bb_ndim = histo_dim;
        bb_min[0] = xmin;
        bb_max[0] = xmax;
        bb_min[1] = ymin;
        bb_max[1] = ymax;
        bb_min[2] = zmin;
        bb_max[2] = zmax;
    }

    mem = (float *)malloc((nbins + n_x_bins + n_y_bins + n_z_bins)*sizeof(float));
    if (mem == NULL)
    {
        Tcl_SetResult(interp, "out of memory", TCL_STATIC);
        goto fail;        
    }
    memset(mem, 0, nbins*sizeof(float));
    xbins = mem + nbins;
    bin_centers(xmin, xmax, n_x_bins, xbins);
    if (histo_dim > 1)
    {
        ybins = xbins + n_x_bins;
        bin_centers(ymin, ymax, n_y_bins, ybins);
    }
    if (histo_dim > 2)
    {
        zbins = ybins + n_y_bins;
        bin_centers(zmin, zmax, n_z_bins, zbins);
    }

    for (itop=0; itop<listlen; ++itop)
    {
        const Ordered_tree_node *topnode = topnodes[itop];

        switch (histo_dim)
        {
        case 1:
            for (i=0; i<n_x_bins; ++i)
            {
                xvals[0] = xbins[i];
                minbounds[0] = bb_min[0];
                maxbounds[0] = bb_max[0];
                mem[i] += ot_block_density(
                    topnode, xvals, minbounds, maxbounds);
            }
            break;
        case 2:
            for (i=0; i<n_x_bins; ++i)
            {
                xvals[0] = xbins[i];
                for (j=0; j<n_y_bins; ++j)
                {
                    xvals[1] = ybins[j];
                    minbounds[0] = bb_min[0];
                    maxbounds[0] = bb_max[0];
                    minbounds[1] = bb_min[1];
                    maxbounds[1] = bb_max[1];
                    mem[i*n_y_bins + j] += ot_block_density(
                        topnode, xvals, minbounds, maxbounds);
                }
            }
            break;
        case 3:
            for (i=0; i<n_x_bins; ++i)
            {
                xvals[0] = xbins[i];
                for (j=0; j<n_y_bins; ++j)
                {
                    xvals[1] = ybins[j];
                    for (k=0; k<n_z_bins; ++k)
                    {
                        xvals[2] = zbins[k];
                        minbounds[0] = bb_min[0];
                        maxbounds[0] = bb_max[0];
                        minbounds[1] = bb_min[1];
                        maxbounds[1] = bb_max[1];
                        minbounds[2] = bb_min[2];
                        maxbounds[2] = bb_max[2];
                        mem[(i*n_y_bins + j)*n_z_bins + k] += ot_block_density(
                            topnode, xvals, minbounds, maxbounds);
                    }
                }
            }
            break;
        default:
            assert(0);
        }
    }

    switch (histo_dim)
    {
    case 1:
        hs_1d_hist_block_fill(histo_id, mem, NULL, NULL);
        break;
    case 2:
        hs_2d_hist_block_fill(histo_id, mem, NULL, NULL);
        break;
    case 3:
        hs_3d_hist_block_fill(histo_id, mem, NULL, NULL);
        break;
    default:
        assert(0);
    }

    status = TCL_OK;
 fail:
    if (mem)
        free(mem);
    if (n_cells)
        free(n_cells);
    if (topnodes)
        free(topnodes);
    return status;
}

static int c_ordered_tree_density_3(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ordered_tree_density_3 tree_id_list histo_id ?bounding_box? */
    Ordered_tree_node **topnodes = 0;
    int i, j, k, itop, histo_id, histo_dim, bb_ndim = 0, status = TCL_ERROR;
    float bb_min[3], bb_max[3], xvals[3], minbounds[3], maxbounds[3];
    float xmin, xmax, ymin, ymax, zmin, zmax;
    int listlen, nbins, n_x_bins = 0, n_y_bins = 0, n_z_bins = 0;
    float *mem = 0, *xbins, *ybins = 0, *zbins = 0;
    Tcl_Obj **listObjElem;
    unsigned *n_cells = 0;

    tcl_objc_range(3,4);
    if (Tcl_ListObjGetElements(interp, objv[1], &listlen, &listObjElem) != TCL_OK)
        return TCL_ERROR;
    topnodes = (Ordered_tree_node **)malloc(listlen*sizeof(Ordered_tree_node *));
    n_cells = (unsigned *)malloc(listlen*sizeof(unsigned));
    if (topnodes == NULL || n_cells == NULL)
    {
        Tcl_SetResult(interp, "out of memory", TCL_STATIC);
        return TCL_ERROR;        
    }
    for (i=0; i<listlen; ++i)
    {
        if (parse_tree_number(interp, listObjElem[i], topnodes+i) != TCL_OK)
            goto fail;
        n_cells[i] = ot_n_load_nodes(topnodes[i]);
        if (n_cells[i] == 0)
        {
            Tcl_SetResult(interp, "detected a tree without payloads", TCL_STATIC);
            goto fail;
        }
    }
    if (Tcl_GetIntFromObj(interp, objv[2], &histo_id) != TCL_OK)
	goto fail;
    switch (hs_type(histo_id))
    {
    case HS_NONE:
        Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[2], NULL),
                         " is not a valid Histo-Scope id", NULL);
        goto fail;
    case HS_1D_HISTOGRAM:
        histo_dim = 1;
        break;
    case HS_2D_HISTOGRAM:
        histo_dim = 2;
        break;
    case HS_3D_HISTOGRAM:
        histo_dim = 3;
        break;
    default:
        Tcl_AppendResult(interp, "item with id ", 
                         Tcl_GetStringFromObj(objv[2], NULL),
                         " is not a histogram", NULL);
        goto fail;
    }
    if (objc > 3)
    {
        if (parse_bounding_box(interp, 3, objv[3], &bb_ndim, bb_min, bb_max) != TCL_OK)
            goto fail;
        if (bb_ndim != histo_dim)
        {
            Tcl_SetResult(interp, "incompatible bounding box dimensionality", TCL_STATIC);
            goto fail;
        }
    }

    switch (histo_dim)
    {
    case 1:
        n_x_bins = hs_1d_hist_num_bins(histo_id);
        hs_1d_hist_range(histo_id, &xmin, &xmax);
        nbins = n_x_bins;
        break;
    case 2:
        hs_2d_hist_num_bins(histo_id, &n_x_bins, &n_y_bins);
        hs_2d_hist_range(histo_id, &xmin, &xmax, &ymin, &ymax);
        nbins = n_x_bins*n_y_bins;
        break;
    case 3:
        hs_3d_hist_num_bins(histo_id, &n_x_bins, &n_y_bins, &n_z_bins);
        hs_3d_hist_range(histo_id, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
        nbins = n_x_bins*n_y_bins*n_z_bins;
        break;
    default:
        assert(0);
    }

    if (bb_ndim == 0)
    {
        bb_ndim = histo_dim;
        bb_min[0] = xmin;
        bb_max[0] = xmax;
        bb_min[1] = ymin;
        bb_max[1] = ymax;
        bb_min[2] = zmin;
        bb_max[2] = zmax;
    }

    mem = (float *)malloc((nbins + n_x_bins + n_y_bins + n_z_bins)*sizeof(float));
    if (mem == NULL)
    {
        Tcl_SetResult(interp, "out of memory", TCL_STATIC);
        goto fail;        
    }
    memset(mem, 0, nbins*sizeof(float));
    xbins = mem + nbins;
    bin_centers(xmin, xmax, n_x_bins, xbins);
    if (histo_dim > 1)
    {
        ybins = xbins + n_x_bins;
        bin_centers(ymin, ymax, n_y_bins, ybins);
    }
    if (histo_dim > 2)
    {
        zbins = ybins + n_y_bins;
        bin_centers(zmin, zmax, n_z_bins, zbins);
    }

    for (itop=0; itop<listlen; ++itop)
    {
        const Ordered_tree_node *topnode = topnodes[itop];

        switch (histo_dim)
        {
        case 1:
            for (i=0; i<n_x_bins; ++i)
            {
                xvals[0] = xbins[i];
                minbounds[0] = bb_min[0];
                maxbounds[0] = bb_max[0];
                mem[i] += ot_polygon_density(
                    topnode, xvals, minbounds, maxbounds);
            }
            break;
        case 2:
            for (i=0; i<n_x_bins; ++i)
            {
                xvals[0] = xbins[i];
                for (j=0; j<n_y_bins; ++j)
                {
                    xvals[1] = ybins[j];
                    minbounds[0] = bb_min[0];
                    maxbounds[0] = bb_max[0];
                    minbounds[1] = bb_min[1];
                    maxbounds[1] = bb_max[1];
                    mem[i*n_y_bins + j] += ot_polygon_density(
                        topnode, xvals, minbounds, maxbounds);
                }
            }
            break;
        case 3:
            for (i=0; i<n_x_bins; ++i)
            {
                xvals[0] = xbins[i];
                for (j=0; j<n_y_bins; ++j)
                {
                    xvals[1] = ybins[j];
                    for (k=0; k<n_z_bins; ++k)
                    {
                        xvals[2] = zbins[k];
                        minbounds[0] = bb_min[0];
                        maxbounds[0] = bb_max[0];
                        minbounds[1] = bb_min[1];
                        maxbounds[1] = bb_max[1];
                        minbounds[2] = bb_min[2];
                        maxbounds[2] = bb_max[2];
                        mem[(i*n_y_bins + j)*n_z_bins + k] += ot_polygon_density(
                            topnode, xvals, minbounds, maxbounds);
                    }
                }
            }
            break;
        default:
            assert(0);
        }
    }

    switch (histo_dim)
    {
    case 1:
        hs_1d_hist_block_fill(histo_id, mem, NULL, NULL);
        break;
    case 2:
        hs_2d_hist_block_fill(histo_id, mem, NULL, NULL);
        break;
    case 3:
        hs_3d_hist_block_fill(histo_id, mem, NULL, NULL);
        break;
    default:
        assert(0);
    }

    status = TCL_OK;
 fail:
    if (mem)
        free(mem);
    if (n_cells)
        free(n_cells);
    if (topnodes)
        free(topnodes);
    return status;
}

static int c_simple_ntuple_filter(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: simple_ntuple_filter input_id filtered_id cut0 cut1 ...
     *
     * Each cut specification is a 3-element list {column_number min max}
     */
    int i, input_id, filtered_id, nvars, ncuts, nrows;

    tcl_objc_range(4,INT_MAX-1);
    verify_ntuple(input_id, 1);
    verify_ntuple(filtered_id, 2);
    nvars = hs_num_variables(input_id);
    if (nvars != hs_num_variables(filtered_id))
    {
        Tcl_SetResult(interp, "ntuples have incompatible number of columns",
                      TCL_STATIC);
        return TCL_ERROR;
    }

    ncuts = objc-3;
    init_mem(filter_cuts, objc-3);
    for (i=0; i<ncuts; ++i)
        if (parse_filter_cut(interp, nvars, objv[i+3], filter_cuts+i) != TCL_OK)
            return TCL_ERROR;

    nrows = hs_num_entries(input_id);
    init_mem(row_data, nvars*nrows);
    hs_ntuple_contents(input_id, row_data);
    for (i=0; i<nrows; ++i)
        if (row_passes_cuts(row_data + i*nvars, filter_cuts, ncuts))
            hs_fill_ntuple(filtered_id, row_data + i*nvars);
    return TCL_OK;
}

static int c_simple_ntuple_counter(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: simple_ntuple_counter input_id cut0 cut1 ...
     *
     * Each cut specification is a 3-element list {column_number min max}
     */
    int i, input_id, nvars, ncuts, nrows, counter=0;

    tcl_objc_range(3,INT_MAX-1);
    verify_ntuple(input_id, 1);
    nvars = hs_num_variables(input_id);

    ncuts = objc-2;
    init_mem(filter_cuts, objc-2);
    for (i=0; i<ncuts; ++i)
        if (parse_filter_cut(interp, nvars, objv[i+2], filter_cuts+i) != TCL_OK)
            return TCL_ERROR;

    nrows = hs_num_entries(input_id);
    init_mem(row_data, nvars*nrows);
    hs_ntuple_contents(input_id, row_data);
    for (i=0; i<nrows; ++i)
        if (row_passes_cuts(row_data + i*nvars, filter_cuts, ncuts))
            ++counter;

    Tcl_SetObjResult(interp, Tcl_NewIntObj(counter));
    return TCL_OK;
}

int init_ordered_tree_api(Tcl_Interp *interp)
{
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_create",
			     c_ordered_tree_create,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_copy",
			     c_ordered_tree_copy,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_scale",
			     c_ordered_tree_scale,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_destroy",
			     c_ordered_tree_destroy,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_shape",
			     c_ordered_tree_shape,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_divide",
			     c_ordered_tree_divide,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_dump",
			     c_ordered_tree_dump,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_bindump",
			     c_ordered_tree_bindump,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_binrestore",
			     c_ordered_tree_binrestore,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_load",
			     c_ordered_tree_load,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_compare",
			     c_ordered_tree_compare,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_collection",
			     c_ordered_tree_collection,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_collection_read",
			     c_ordered_tree_collection_read,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_cells",
			     c_ordered_tree_cells,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_leaf_boxes",
			     c_ordered_tree_leaf_boxes,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_density",
			     c_ordered_tree_density,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_density_2",
			     c_ordered_tree_density_2,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_density_3",
			     c_ordered_tree_density_3,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_fill_fromntuple",
			     c_ordered_tree_fill_fromntuple,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_reset",
			     c_ordered_tree_reset,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_payload_stats",
			     c_ordered_tree_payload_stats,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_payload",
			     c_ordered_tree_payload,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_n_payloads",
			     c_ordered_tree_n_payloads,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_payload_scan",
			     c_ordered_tree_payload_scan,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "resample_ntuple_rows",
			     c_resample_ntuple_rows,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "simple_ntuple_filter",
			     c_simple_ntuple_filter,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "simple_ntuple_counter",
			     c_simple_ntuple_counter,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_to_uniform",
			     c_ordered_tree_to_uniform,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_from_uniform",
			     c_ordered_tree_from_uniform,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_polygon_random",
			     c_ordered_tree_polygon_random,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "ordered_tree_extremum_bounds",
			     c_ordered_tree_extremum_bounds,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    commandInterp = interp;
    return TCL_OK;
}

void cleanup_ordered_tree_api(void)
{
    if (piecedata_set)
    {
        unsigned i;
        for (i=0; i<piecedata_set_size; ++i)
            ot_delete_node(piecedata_set[i], payload_destructor);
	free(piecedata_set);
    }
    if (row_data)
        free(row_data);
    if (filter_cuts)
        free(filter_cuts);
    if (commandInterp)
    {
	Tcl_DeleteCommand(commandInterp, "ordered_tree_create");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_copy");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_scale");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_dump");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_bindump");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_binrestore");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_load");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_compare");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_destroy");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_shape");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_divide");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_cells");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_collection");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_collection_read");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_leaf_boxes");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_density");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_density_2");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_density_3");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_fill_fromntuple");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_reset");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_payload_stats");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_n_payloads");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_payload");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_payload_scan");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_to_uniform");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_from_uniform");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_polygon_random");
	Tcl_DeleteCommand(commandInterp, "ordered_tree_extremum_bounds");

	Tcl_DeleteCommand(commandInterp, "resample_ntuple_rows");
	Tcl_DeleteCommand(commandInterp, "simple_ntuple_filter");
	Tcl_DeleteCommand(commandInterp, "simple_ntuple_counter");
    }
}

#ifdef __cplusplus
}
#endif
