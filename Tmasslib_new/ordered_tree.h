#ifndef ORDERED_TREE_H_
#define ORDERED_TREE_H_

#include <stdio.h>
#include <rpc/xdr.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The following two structs can be anything */
typedef double OTPayload;
typedef unsigned long OTKeyPayload;

typedef struct _ordered_tree_node {
    const struct _ordered_tree_node *parent;
    unsigned n_in_parent;              /* Daughter number of this
                                        * node in the parent node
                                        */
    unsigned split_dim;                /* The number of the dimension
                                        * which is split in this node
                                        */
    float *bounds;                     /* Array of size ndaus-1 */
    float *centers;                    /* Array of size ndaus   */
    struct _ordered_tree_node **daus;
    unsigned ndaus;
    unsigned inode;                    /* This member is used to
                                        * map the tree structure into
                                        * a linear array before dumping
                                        */
    OTPayload payload;                 /* An arbitrary struct. Can be,
                                        * for example, a void pointer.
                                        * Filled only for the last
                                        * tree level.
                                        */
} Ordered_tree_node;

typedef struct {
    unsigned split_dim;
    unsigned nsplits;
} Ordered_tree_split;

typedef struct {
    Ordered_tree_split *splits;
    unsigned nlevels;
} Ordered_tree_shape;

typedef struct {
    float *index;
    float weight;
    OTKeyPayload keyload;
} Ordered_tree_key;

typedef OTPayload (*OTPayload_constructor)(void *userData);
typedef OTPayload (*OTPayload_copy_constructor)(const OTPayload orig,
                                                void *userData);
typedef OTPayload (*OTPayload_combiner)(const OTPayload, const OTPayload,
                                        void *userData);
typedef void (*OTPayload_destructor)(OTPayload);
typedef void (*OTNode_visitor)(void *userData, const Ordered_tree_node *);
typedef void (*OTPayload_visitor)(void *userData, OTPayload *);
typedef void (*OTPayload_text_dumper)(void *userData, FILE *f, const OTPayload);
typedef void (*OTPayload_writer)(void *userData, XDR *xdrs, const OTPayload);
typedef int  (*OTPayload_comparator)(const OTPayload, const OTPayload);
typedef void (*OTPayload_filler)(void *userData, OTPayload *,
                                 const Ordered_tree_key *pkey);

/* The following function should return 0 on success */
typedef int (*OTPayload_reader)(void *userData, XDR *xdrs, OTPayload *);

/* Tree constructor. The "data" array will be modified. */
Ordered_tree_node *ot_create_tree(
    Ordered_tree_key* data, unsigned ndata,
    const Ordered_tree_shape *shape,
    OTPayload_constructor constructor, void *constructorUserData, 
    OTPayload_filler filler, void *fillerUserData);

/* Copy constructor. If "copier" is NULL then payloads
 * will be copied by the assignment operator.
 */
Ordered_tree_node *ot_copy_node(const Ordered_tree_node *orig,
                                OTPayload_copy_constructor copier,
                                void *constructorUserData);
/* Destructor */
void ot_delete_node(Ordered_tree_node *node, OTPayload_destructor destr);

/* In the following function "splitdata" array should have
 * at least 2*split->nsplits-1 elements. "payloads" array
 * should have at least split->nsplits elements.
 */
void ot_add_level(Ordered_tree_node *node, const float *splitdata,
		  const OTPayload *payloads, const Ordered_tree_split* split,
		  OTPayload_destructor destr);

/* The following function is intended for dumping the tree
 * into a .c file. fcn is the name of the function
 * with prototype "Ordered_tree_node* fcn(void);" which
 * should be used to retrieve the tree.
 */
void ot_dump_c(const char *filename, const char *fcn,
               const Ordered_tree_node *topnode,
               OTPayload_text_dumper payDumper, void *payDumperData);

/* The following function is intended for dumping the tree
 * into a binary file.
 */
void ot_binary_dump(XDR *xdrs, const Ordered_tree_node *topnode,
                    OTPayload_writer payDumper, void *payDumperData);

/* The reverse of the binary dump. */
Ordered_tree_node* ot_binary_restore(XDR *xdrs, OTPayload_reader payReader,
                                     void *payReaderData,
                                     OTPayload_destructor destr);

/* The tree comparison function. If "cmp" is NULL then payloads
 * will be compared by memcmp.
 */
int ot_compare(const Ordered_tree_node *t1, const Ordered_tree_node *t2,
               double eps, OTPayload_comparator cmp);

/* The tree payload combination function. Can be used
 * to perform various arithmetic operations on the tree contents.
 * This function returns 0 on success, something else on failure.
 */
int ot_combine_payloads(const Ordered_tree_node *t1,
                        const Ordered_tree_node *t2,
                        Ordered_tree_node *result,
                        OTPayload_combiner cmb,
                        void *combinerData);

/* The next function iterates over the tree. The payload
 * visitor will be called only on the lowest-level nodes
 * while node visitor will be called on all nodes. If called,
 * the payload visitor is called before the node visitor.
 */
void ot_walk(const Ordered_tree_node *topnode,
             OTNode_visitor, void *nodeVisitorData,
             OTPayload_visitor, void *payloadVisitorData);

/* The next function is a generic function which traverses
 * the tree down at a given coordinate. The payload visitor 
 * will be called only on the lowest-level node.
 */
void ot_descend(const Ordered_tree_node *topnode,
                const float *index,
                OTNode_visitor, void *nodeVisitorData,
                OTPayload_visitor, void *payloadVisitorData);

/* Various Utility methods */
Ordered_tree_node* ot_find_subnode(const Ordered_tree_node *node, float x);
Ordered_tree_node* ot_subnode_at(const Ordered_tree_node *node, unsigned i);
unsigned ot_n_all_nodes(const Ordered_tree_node *topnode);

/* The following function returns the number of nodes at the lowest level */
unsigned ot_n_load_nodes(const Ordered_tree_node *topnode);

/* Node lookup by location and by cell number. Set "maxdepth"
 * to something large if you want to get the lowest-level node.
 * The "cell number" (unsigned *index) is the sequence of the
 * leaf numbers in the parent node, level-by-level. This array
 * should have at least as many elements as the smallest of
 * "maxdepth" and the actual tree depth.
 */
Ordered_tree_node* ot_find_node(const Ordered_tree_node *topnode,
				const float *index, unsigned maxdepth);
Ordered_tree_node* ot_cell_node(const Ordered_tree_node *topnode,
				const unsigned *index, unsigned maxdepth);

/* Payload lookup by location and by cell number */
OTPayload* ot_find_payload(const Ordered_tree_node *topnode,
                           const float *index, unsigned maxdepth);
OTPayload* ot_cell_payload(const Ordered_tree_node *topnode,
                           const unsigned *index, unsigned maxdepth);

/* Lookup the cell center by location and by cell number */
void ot_find_center(const Ordered_tree_node *topnode,
		    const float *index, unsigned maxdepth,
                    float *center);
void ot_cell_center(const Ordered_tree_node *topnode,
		    const unsigned *index, unsigned maxdepth,
                    float *center);

/* Find lowest-level cell boundaries by location */
void ot_find_bounds(const Ordered_tree_node *topnode,
                    const float *index, unsigned maxdepth,
                    float *min_bound, float *max_bound);

/* Lookup the cell center from a low-level node.
 * The "dimensions" array specifies which coordinates
 * should be looked up and in which order they are
 * returned in the "center" array. All numbers in
 * the "dimensions" array must be distinct. The function
 * returns 0 if all coordinates are found, 1 otherwise.
 */
int ot_thisnode_center(const Ordered_tree_node *node,
                       const unsigned *dimensions, 
                       unsigned ndim, float *center);

/* Lookup the cell bounds from a low-level node.
 * The function returns 0 if all coordinates are found, 1 otherwise.
 */
int ot_thisnode_bounds(const Ordered_tree_node *node,
                       const unsigned *dimensions,
                       unsigned ndim, float *min_bound,
                       float *max_bound);

/* Maximum number of daughters at each tree level.
 * The function will fill out the "maxdaus" array which
 * should have at least "maxdepth" elements.
 */
void ot_max_daus(const Ordered_tree_node *topnode,
		 unsigned maxdepth, unsigned *maxdaus);

/* Minimum number of daughters at each tree level.
 * The function will fill out the "mindaus" array which
 * should have at least "mindepth" elements.
 */
void ot_min_daus(const Ordered_tree_node *topnode,
		 unsigned maxdepth, unsigned *mindaus);

/* Determine min and max tree depth */
void ot_min_max_depth(const Ordered_tree_node *topnode,
		      unsigned *mindepth, unsigned *maxdepth);

/* The following function returns the tree shape in case
 * the shape is a well-defined concept for this tree,
 * otherwise it returns NULL. The shape must be destroyed
 * later using the "ot_destroy_shape" function.
 */
Ordered_tree_shape *ot_get_shape(const Ordered_tree_node *topnode);

/* The following function destroys Ordered_tree_shape object
 * previously allocated on the heap.
 */
void ot_destroy_shape(Ordered_tree_shape *shape);

/* The following function maps multivariate uniform distribution
 * into the distribution represented by the tree. This function
 * can be used for generating random numbers according to the
 * distribution represented by the tree. Within each cell the
 * distribution is considered to be uniform. For this function
 * to work properly, the tree must be created by the "ot_create_tree"
 * function (it must have a regular shape).
 *
 * "min_bound" and "max_bound" arrays should be set to the
 * distribution boundaries in the respective dimensions.
 */
void ot_from_uniform(const Ordered_tree_node *topnode,
                     const float *u, const float *min_bound,
                     const float *max_bound, float *index);

/* The following function maps the distribution represented
 * by the tree into the multivariate uniform distribution.
 */
void ot_to_uniform(const Ordered_tree_node *topnode,
                   const float *index, const float *min_bound,
                   const float *max_bound, float *u);

/* Function for looking up min/max bounds in all cells.
 * It will set min/max bounds to 1.f/-1.f, respectively,
 * for unused dimensions.
 */
void ot_extremum_bounds(const Ordered_tree_node *topnode,
                        float *min_bound, float *max_bound);

/* Density estimation function (a-la variable bin histogram).
 * This function assumes that all tree leafs corresponds to approximately
 * the same fraction of the cumulative distribution, so that the density
 * in each cell is 1/(cell area)/(total number of cells).
 */
double ot_block_density(const Ordered_tree_node *topnode,
                        const float *index, const float *min_bound,
                        const float *max_bound);

/* Density estimation function (a-la frequency polygon).
 * Currently supported only for 1d and 2d trees.
 * To first order, the interpolation is performed in a manner
 * which preserves the overall density normalization.
 */
double ot_polygon_density(const Ordered_tree_node *topnode,
                          const float *index, const float *min_bound,
                          const float *max_bound);

/* Random numbers consistent with the above density.
 * Works just like "ot_from_uniform".
 */
void ot_polygon_random(const Ordered_tree_node *topnode,
                       const float *rand, const float *min_bound,
                       const float *max_bound, float *index);

/* Linear payload interpolation in the space of cell numbers.
 * Works for regularly-shaped trees only, just like the density
 * estimation.
 */
OTPayload ot_interpolate_payload(const Ordered_tree_node *topnode,
                                 const float *index);

/* The following are internal utility functions.
 * They are not for use by applications codes.
 */
void ot_split_node(Ordered_tree_node *node,
                   const Ordered_tree_split* split);

#ifdef __cplusplus
}
#endif

#endif /* ORDERED_TREE_H_ */
