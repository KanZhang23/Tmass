#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <float.h>

#include "ordered_tree.h"
#include "range_lookup_flt.h"

#define MAX_DIM_NUMBER 20

#ifdef __cplusplus
extern "C" {
#endif

static double lin_interpolate(double x0, double x1,
                              double y0, double y1, double x)
{
    return y0 + (y1 - y0)*((x - x0)/(x1 - x0));
}

static int solve_quadratic_stable(const double b, const double c,
                                  double *x1, double *x2)
{
    const double d = b*b - 4.0*c;

    if (d < 0.0)
        return 0;

    if (d == 0)
    {
        *x1 = -b/2.0;
        *x2 = *x1;
    }
    else if (b == 0.0)
    {
        *x1 = sqrt(-c);
        *x2 = -(*x1);
    }
    else
    {
        *x1 = -(b + (b < 0.0 ? -1.0 : 1.0)*sqrt(d))/2.0;
        *x2 = c/(*x1);
    }
    return 2;
}

typedef struct {
    double d1;
    double d2;
} d_d_pair;

static int sort_d_d_pair_incr_d1(const void *ip1, const void *ip2)
{
    const d_d_pair *p1 = (const d_d_pair *)ip1;
    const d_d_pair *p2 = (const d_d_pair *)ip2;
    if (p1->d1 < p2->d1)
        return -1;
    else if (p1->d1 > p2->d1)
        return 1;
    else
        return 0;
}

/* The following function can be used to generate random numbers
 * according to the density y(x) = y0 + 2*(1-y0)*x on the interval [0, 1].
 * In order for this function to be a density, the following condition
 * must be satisfied: y0 >= 0 && y0 <= 2.
 */
static double standard_linear_density_invcdf(const double r, const double y0)
{
    const double a = 1.0 - y0;

    assert(y0 >= 0 && y0 <= 2.0);
    assert(r >= 0.0 && r <= 1.0);

    if (fabs(a) < 1.e-8)
        return r;
    else
    {
        double x1, x2;
        const int n = solve_quadratic_stable(y0/a, -r/a, &x1, &x2);
        assert(n == 2);
        if (x1 >= 0.0 && x1 <= 1.0)
            return x1;
        else if (x2 >= 0.0 && x2 <= 1.0)
            return x2;
        else
        {
            d_d_pair p[4];
            p[0].d1 = fabs(x1 - 0.0);
            p[0].d2 = 0.0;
            p[1].d1 = fabs(x2 - 0.0);
            p[1].d2 = 0.0;
            p[2].d1 = fabs(x1 - 1.0);
            p[2].d2 = 1.0;
            p[3].d1 = fabs(x2 - 1.0);
            p[3].d2 = 1.0;
            qsort(p, 4, sizeof(d_d_pair), sort_d_d_pair_incr_d1);
            return p[0].d2;
        }
    }
}

static void standard_linear_density_2d_inverse(
    const double rx, const double ry,
    const double z00, const double z10,
    const double z01, const double z11,
    double *x, double *y)
{
    const double zleft = (z00 + z01)/2.0;
    const double zright = (z10 + z11)/2.0;
    if (rx < 0.0 || rx > 1.0)
    {
        fprintf(stderr, "ERROR in standard_linear_density_2d_inverse: rx is %.17f\n", rx);
        fflush(stderr);
    }
    if (ry < 0.0 || ry > 1.0)
    {
        fprintf(stderr, "ERROR in standard_linear_density_2d_inverse: ry is %.17f\n", ry);
        fflush(stderr);
    }
    *x = standard_linear_density_invcdf(rx, 2.0*zleft/(zleft + zright));
    {
        const double zlow = (1.0 - *x)*z00 + *x*z10;
        const double zhi = (1.0 - *x)*z01 + *x*z11;
        *y = standard_linear_density_invcdf(ry, 2.0*zlow/(zlow + zhi));
    }
}

Ordered_tree_node* ot_subnode_at(const Ordered_tree_node *node, unsigned i)
{
    assert(node);
    if (i >= node->ndaus)
	return NULL;
    else
	return node->daus[i];
}

Ordered_tree_node* ot_find_subnode(const Ordered_tree_node *node, float x)
{
    assert(node);
    return node->daus[range_lookup_flt(node->bounds, node->ndaus-1, x)];
}

void ot_find_bounds(const Ordered_tree_node *topnode,
                    const float *index, unsigned maxdepth,
                    float *min_bound, float *max_bound)
{
    unsigned j, dpth;
    const Ordered_tree_node *current = topnode;
    assert(current);
    assert(index);
    assert(min_bound);
    assert(max_bound);
    for (dpth=0; dpth<maxdepth && current->ndaus > 0; ++dpth)
    {
	j = range_lookup_flt(current->bounds, current->ndaus-1,
                             index[current->split_dim]);
        if (j)
            min_bound[current->split_dim] = current->bounds[j-1];
        if (j < current->ndaus-1)
            max_bound[current->split_dim] = current->bounds[j];
	current = ot_subnode_at(current, j);
    }    
}

void ot_find_center(const Ordered_tree_node *topnode,
		    const float *index, const unsigned treedepth, 
		    float *center)
{
    unsigned j, dpth;
    const Ordered_tree_node *current = topnode;
    assert(index);
    assert(center);
    for (dpth=0; dpth<treedepth && current->ndaus; ++dpth)
    {
	j = range_lookup_flt(current->bounds, current->ndaus-1,
                             index[current->split_dim]);
	center[current->split_dim] = current->centers[j];
	current = ot_subnode_at(current, j);
    }
}

void ot_cell_center(const Ordered_tree_node *topnode,
		    const unsigned *index, const unsigned treedepth,
		    float *center)
{
    unsigned j, dpth;
    const Ordered_tree_node *current = topnode;
    assert(index);
    assert(center);
    for (dpth=0; dpth<treedepth && current->ndaus; ++dpth)
    {
	j = index[dpth];
	center[current->split_dim] = current->centers[j];
	current = ot_subnode_at(current, j);
    }
}

Ordered_tree_node* ot_find_node(const Ordered_tree_node *topnode,
				const float *index,
				const unsigned maxdepth)
{
    unsigned dpth;
    const Ordered_tree_node *current = topnode;
    assert(index);
    assert(topnode);
    for (dpth=0; dpth<maxdepth && current->ndaus>0; ++dpth)
        current = ot_find_subnode(current, index[current->split_dim]);
    return (Ordered_tree_node*)current;
}

OTPayload* ot_find_payload(const Ordered_tree_node *topnode,
                           const float *index, const unsigned treedepth)
{
    return &ot_find_node(topnode, index, treedepth)->payload;
}

int ot_thisnode_bounds(const Ordered_tree_node *node,
                       const unsigned *dimensions,
                       unsigned ndim, float *min_bound,
                       float *max_bound)
{
    unsigned i, allfound = 0;
    int found_min[MAX_DIM_NUMBER], found_max[MAX_DIM_NUMBER];

    assert(node);
    assert(dimensions);
    assert(ndim > 0 && ndim < MAX_DIM_NUMBER);
    assert(min_bound);
    assert(max_bound);

    for (i=0; i<ndim; ++i)
    {
        found_min[i] = 0;
        found_max[i] = 0;
    }

    while (node->parent)
    {
        allfound = 1;
        for (i=0; i<ndim; ++i)
        {
            if (dimensions[i] == node->parent->split_dim)
            {
                if (!found_min[i] && node->n_in_parent > 0)
                {
                    found_min[i] = 1;
                    min_bound[i] = node->parent->bounds[node->n_in_parent-1];
                }
                if (!found_max[i] && node->n_in_parent < (node->parent->ndaus - 1))
                {
                    found_max[i] = 1;
                    max_bound[i] = node->parent->bounds[node->n_in_parent];
                }
            }
            if (!found_min[i] || !found_max[i])
                allfound = 0;
        }
        if (allfound)
            return 0;
        node = node->parent;
    }
    return 1;
}

int ot_thisnode_center(const Ordered_tree_node *node,
                       const unsigned *dimensions, 
                       unsigned ndim, float *center)
{
    unsigned i, allfound = 0;
    int found[MAX_DIM_NUMBER];

    assert(node);
    assert(dimensions);
    assert(ndim > 0 && ndim < MAX_DIM_NUMBER);
    assert(center);
    
    for (i=0; i<ndim; ++i)
        found[i] = 0;

    while (node->parent)
    {
        allfound = 1;
        for (i=0; i<ndim; ++i)
        {
            if (dimensions[i] == node->parent->split_dim)
            {
                if (!found[i])
                {
                    found[i] = 1;
                    center[i] = node->parent->centers[node->n_in_parent];
                }
            }
            if (!found[i])
                allfound = 0;
        }
        if (allfound)
            return 0;
        node = node->parent;
    }
    return 1;
}

Ordered_tree_node* ot_cell_node(const Ordered_tree_node *topnode,
				const unsigned *index,
				const unsigned treedepth)
{
    unsigned dpth;
    const Ordered_tree_node *current = topnode;
    assert(topnode);
    assert(index);
    for (dpth=0; dpth<treedepth && current->ndaus>0; ++dpth)
	current = ot_subnode_at(current, index[dpth]);
    return (Ordered_tree_node*)current;
}

OTPayload* ot_cell_payload(const Ordered_tree_node *topnode,
                           const unsigned *index,
			   const unsigned treedepth)
{
    return &ot_cell_node(topnode, index, treedepth)->payload;
}

void ot_delete_node(Ordered_tree_node *node, OTPayload_destructor destr)
{
    if (node)
    {
        if (node->bounds)
            free(node->bounds);
        if (node->daus)
        {
            unsigned i;
            for (i=0; i<node->ndaus; ++i)
                ot_delete_node(node->daus[i], destr);
            free(node->daus);
        }
        if (destr)
            destr(node->payload);
        free(node);
    }
}

void ot_split_node(Ordered_tree_node *node,
                   const Ordered_tree_split* split)
{
    assert(node);
    assert(node->ndaus == 0);
    assert(node->bounds == 0);
    assert(node->daus == 0);

    assert(split);
    assert(split->split_dim < MAX_DIM_NUMBER);
    assert(split->nsplits >= 2);

    node->bounds = (float *)malloc((2*split->nsplits-1)*sizeof(float));
    assert(node->bounds);
    node->centers = node->bounds + (split->nsplits-1);
    node->daus = (Ordered_tree_node **)calloc(split->nsplits, sizeof(Ordered_tree_node*));
    assert(node->daus);
    node->ndaus = split->nsplits;
    node->split_dim = split->split_dim;
}

void ot_add_level(Ordered_tree_node *node, const float *splitdata,
		  const OTPayload *payloads, const Ordered_tree_split* split,
		  OTPayload_destructor destr)
{
    unsigned i;
    const unsigned nsplits = split->nsplits;

    assert(payloads);
    assert(is_strictly_increasing_flt(splitdata, 2*nsplits-1));
    ot_split_node(node, split);
    for (i=0; i<nsplits-1; ++i)
	node->bounds[i] = splitdata[2*i+1];
    for (i=0; i<nsplits; ++i)
    {
	node->centers[i] = splitdata[2*i];
	node->daus[i] = (Ordered_tree_node *)calloc(1, sizeof(Ordered_tree_node));
	assert(node->daus[i]);
	node->daus[i]->payload = payloads[i];
	node->daus[i]->parent = node;
        node->daus[i]->n_in_parent = i;
    }
    if (destr)
	destr(node->payload);
    memset(&node->payload, 0, sizeof(OTPayload));
}

#define tree_key_sorter(DIM) static int OT_key_sorter_ ## DIM/**/(\
const Ordered_tree_key *p1, const Ordered_tree_key *p2)\
{\
    if (p1->index[ DIM ] < p2->index[ DIM ])\
        return -1;\
    else if (p2->index[ DIM ] < p1->index[ DIM ])\
        return 1;\
    else\
        return 0;\
}

tree_key_sorter(0)
tree_key_sorter(1)
tree_key_sorter(2)
tree_key_sorter(3)
tree_key_sorter(4)
tree_key_sorter(5)
tree_key_sorter(6)
tree_key_sorter(7)
tree_key_sorter(8)
tree_key_sorter(9)
tree_key_sorter(10)
tree_key_sorter(11)
tree_key_sorter(12)
tree_key_sorter(13)
tree_key_sorter(14)
tree_key_sorter(15)
tree_key_sorter(16)
tree_key_sorter(17)
tree_key_sorter(18)
tree_key_sorter(19)

static Ordered_tree_node *ot_create_node(
    Ordered_tree_key* data, const unsigned ndata,
    const unsigned level, const Ordered_tree_shape *shape,
    const Ordered_tree_node *parent, unsigned n_in_parent,
    OTPayload_constructor constructor, void * const constructorUserData, 
    OTPayload_filler filler, void * const fillerUserData)
{
    typedef int (*comparator)(const  Ordered_tree_key*, const Ordered_tree_key*);
    static const comparator cmp[MAX_DIM_NUMBER] = {
        OT_key_sorter_0,
        OT_key_sorter_1,
        OT_key_sorter_2,
        OT_key_sorter_3,
        OT_key_sorter_4,
        OT_key_sorter_5,
        OT_key_sorter_6,
        OT_key_sorter_7,
        OT_key_sorter_8,
        OT_key_sorter_9,
        OT_key_sorter_10,
        OT_key_sorter_11,
        OT_key_sorter_12,
        OT_key_sorter_13,
        OT_key_sorter_14,
        OT_key_sorter_15,
        OT_key_sorter_16,
        OT_key_sorter_17,
        OT_key_sorter_18,
        OT_key_sorter_19
    };

    /* Construct the node */
    Ordered_tree_node *node = (Ordered_tree_node *)calloc(
        1, sizeof(Ordered_tree_node));
    assert(node);

    node->parent = parent;
    node->n_in_parent = n_in_parent;

    if (level < shape->nlevels)
    {
        /* Process this level split */
	ot_split_node(node, shape->splits + level);
        if (ndata == 0)
        {
            const unsigned splits = node->ndaus;
            const unsigned thisdim = node->split_dim;
            const unsigned ntargets = 2*node->ndaus - 1;
            unsigned i, target_number;
            float min_bound, max_bound, delta;

            const int status = ot_thisnode_bounds(node, &thisdim, 1,
                                                  &min_bound, &max_bound);
            /* Both bounds must be found, otherwise we have no way
             * to determine the scale (bin width) in this dimension
             */
            assert(status == 0);
            delta = 0.5f*(max_bound - min_bound)/splits;

            for (target_number=0; target_number<ntargets; ++target_number)
            {
                const float x = min_bound + delta*(target_number + 1);
                if (target_number % 2)
                    /* This is a bound */
                    node->bounds[target_number/2] = x;
                else
                    /* This is a center */
                    node->centers[target_number/2] = x;
            }

            for (i=0; i<splits; ++i)
                node->daus[i] = ot_create_node(data, 0, level+1,
                                               shape, node, i,
                                               constructor, constructorUserData,
                                               filler, fillerUserData);
        }
        else
        {
            const unsigned splits = node->ndaus;
            const unsigned thisdim = node->split_dim;
            const unsigned ntargets = 2*node->ndaus - 1;
            const unsigned ndatam1 = ndata - 1;
            double target_interval, target_weight, wsum = 0.0;
            double next_weight, current_weight;
            unsigned idat, target_number, oldhi = 0;
            int location = 0;

            /* First, try to find toplevel bounds in this dimension */
            float min_bound = -FLT_MAX, max_bound = FLT_MAX;
            ot_thisnode_bounds(node, &thisdim, 1, &min_bound, &max_bound);

            if (ndata == 1)
                /* Check that we can determine the scale in this dimension.
                 * The condition asserted below is necessary but not sufficient.
                 */
                assert(min_bound > -FLT_MAX || max_bound < FLT_MAX);

            /* Sort the data in the requested dimension */
            qsort(data, ndata, sizeof(Ordered_tree_key),
                  (int(*)(const void*,const void*))cmp[node->split_dim]);

            /* Now, sum up all weights */
            if (min_bound > -FLT_MAX)
            {
                location = -1;
                wsum += data[0].weight/2.0;
            }
            if (ndatam1)
                wsum += data[0].weight/2.0;
            for (idat=1; idat<ndatam1; ++idat)
            {
                wsum += data[idat].weight/2.0;
                wsum += data[idat].weight/2.0;
            }
            if (ndatam1)
                wsum += data[ndatam1].weight/2.0;
            if (max_bound < FLT_MAX)
                wsum += data[ndatam1].weight/2.0;
            assert(wsum > 0.0);
            target_interval = 0.5*wsum/splits;

            /* Run over the weights and build the bounds and the centers */
            current_weight = 0.0;
            target_number = 0;
            while (location <= (int)ndatam1 && target_number < ntargets)
            {
                if (location >= 0 && location < (int)ndatam1)
                    next_weight = current_weight + data[location].weight/2.0 +
                        data[location+1].weight/2.0;
                else if (location == -1)
                    next_weight = data[0].weight/2.0;
                else if (location == (int)ndatam1)
                {
                    assert(max_bound < FLT_MAX);
                    next_weight = current_weight + data[location].weight/2.0;
                }
                else
                    assert(0);

                while (target_number < ntargets)
                {
                    target_weight = (target_number + 1)*target_interval;
                    assert(target_weight >= current_weight);
                    if (target_weight < next_weight)
                    {
                        double x;
                        if (location >= 0 && location < (int)ndatam1)
                            x = lin_interpolate(current_weight, next_weight,
                                                data[location].index[thisdim],
                                                data[location+1].index[thisdim],
                                                target_weight);
                        else if (location == -1)
                            x = lin_interpolate(current_weight, next_weight,
                                                min_bound,
                                                data[location+1].index[thisdim],
                                                target_weight);
                        else if (location == (int)ndatam1)
                            x = lin_interpolate(current_weight, next_weight,
                                                data[location].index[thisdim],
                                                max_bound,
                                                target_weight);
                        else
                            assert(0);

                        if (target_number % 2)
                            /* This is a bound */
                            node->bounds[target_number/2] = x;
                        else
                            /* This is a center */
                            node->centers[target_number/2] = x;

                        ++target_number;
                    }
                    else
                        break;
                }

                current_weight = next_weight;
                ++location;
            }
            assert(target_number == ntargets);

            /* Run over the weights again and build the subnodes */
            location = 0;
            if (min_bound > -FLT_MAX)
                location = -1;
            current_weight = 0.0;
            target_number = 1;
            while (location <= (int)ndatam1 && target_number < ntargets)
            {
                if (location >= 0 && location < (int)ndatam1)
                    next_weight = current_weight + data[location].weight/2.0 +
                        data[location+1].weight/2.0;
                else if (location == -1)
                    next_weight = data[0].weight/2.0;
                else if (location == (int)ndatam1)
                {
                    assert(max_bound < FLT_MAX);
                    next_weight = current_weight + data[location].weight/2.0;
                }
                else
                    assert(0);

                while (target_number < ntargets)
                {
                    target_weight = (target_number + 1)*target_interval;
                    assert(target_weight >= current_weight);
                    if (target_weight < next_weight)
                    {
                        const unsigned i = target_number/2;
                        const unsigned hi = location + 1;
                        node->daus[i] = ot_create_node(data+oldhi, hi-oldhi, level+1,
                                                       shape, node, i,
                                                       constructor, constructorUserData,
                                                       filler, fillerUserData);
                        oldhi = hi;
                        target_number += 2;
                    }
                    else
                        break;
                }

                current_weight = next_weight;
                ++location;
            }

            /* Build the last subnode */
            assert(node->daus[splits-1] == 0);
            node->daus[splits-1] = ot_create_node(data+oldhi, ndata-oldhi, level+1,
                                                  shape, node, splits-1,
                                                  constructor, constructorUserData,
                                                  filler, fillerUserData);
        }
    }
    else
    {
        /* Last level. Build the payloads. */
        if (constructor)
        {
            node->payload = constructor(constructorUserData);
            if (filler)
            {
                unsigned i;
                for (i=0; i<ndata; ++i)
                    filler(fillerUserData, &node->payload, data+i);
            }
        }
    }

    return node;
}

Ordered_tree_node *ot_copy_node(const Ordered_tree_node *orig,
                                OTPayload_copy_constructor copier,
                                void *constructorUserData)
{
    Ordered_tree_node *copy;

    assert(orig);
    copy = (Ordered_tree_node *)calloc(1, sizeof(Ordered_tree_node));
    assert(copy);

    if (orig->ndaus > 0)
    {
        unsigned i;
        Ordered_tree_split split;
        split.split_dim = orig->split_dim;
        split.nsplits = orig->ndaus;
        ot_split_node(copy, &split);
        memcpy(copy->bounds, orig->bounds, (orig->ndaus-1)*sizeof(copy->bounds[0]));
        memcpy(copy->centers, orig->centers, orig->ndaus*sizeof(copy->centers[0]));
        for (i=0; i<orig->ndaus; ++i)
        {
            copy->daus[i] = ot_copy_node(orig->daus[i], copier,
                                         constructorUserData);
            copy->daus[i]->parent = copy;
            copy->daus[i]->n_in_parent = i;
        }
    }
    else
    {   
        if (copier)
            copy->payload = copier(orig->payload, constructorUserData);
        else
            copy->payload = orig->payload;
    }

    return copy;
}

Ordered_tree_node *ot_create_tree(
    Ordered_tree_key* data, unsigned ndata,
    const Ordered_tree_shape *shape,
    OTPayload_constructor constructor, void *constructorUserData, 
    OTPayload_filler filler, void *fillerUserData)
{
    /* Check the shape */
    unsigned i, n_non_zero = 0;
    for (i=0; i<shape->nlevels; ++i)
    {
        if (shape->splits[i].split_dim >= MAX_DIM_NUMBER)
            assert(!"Dimension too high, define additional"
                   " sort functions and and recompile");
        assert(shape->splits[i].nsplits >= 2);
    }

    /* Check the weights. Need at least two
     * data points with non-zero weights.
     */
    for (i=0; i<ndata; ++i)
    {
        assert(data[i].weight >= 0.f);
        if (data[i].weight > 0.f)
            ++n_non_zero;
    }
    assert(n_non_zero >= 2);

    return ot_create_node(data, ndata, 0, shape, NULL, 0,
                          constructor, constructorUserData,
                          filler, fillerUserData);
}

void ot_descend(const Ordered_tree_node *topnode,
                const float *index,
                OTNode_visitor noder, void *nodeVisitorData,
                OTPayload_visitor loader, void *payloadVisitorData)
{
    assert(index);
    assert(topnode);
    while (topnode->ndaus > 0)
    {
        if (noder)
            noder(nodeVisitorData, topnode);
        topnode = ot_find_subnode(topnode, index[topnode->split_dim]);
    }
    if (loader)
        loader(payloadVisitorData, &((Ordered_tree_node *)topnode)->payload);
    if (noder)
        noder(nodeVisitorData, topnode);
}

void ot_walk(const Ordered_tree_node *topnode,
             OTNode_visitor noder, void *nodeVisitorData,
             OTPayload_visitor loader, void *payloadVisitorData)
{
    if (topnode->ndaus)
    {
        unsigned i;
        assert(topnode->daus);
        for (i=0; i<topnode->ndaus; ++i)
            ot_walk(topnode->daus[i], noder, nodeVisitorData,
                    loader, payloadVisitorData);
    }
    else if (loader)
        loader(payloadVisitorData, &((Ordered_tree_node *)topnode)->payload);
    if (noder)
        noder(nodeVisitorData, topnode);
}

int ot_combine_payloads(const Ordered_tree_node *t1,
                        const Ordered_tree_node *t2,
                        Ordered_tree_node *result,
                        OTPayload_combiner cmb,
                        void *combinerData)
{
    if (t1->n_in_parent != t2->n_in_parent || t1->n_in_parent != result->n_in_parent)
        return 1;
    if (t1->split_dim != t2->split_dim || t1->split_dim != result->split_dim)
        return 1;
    if (t1->ndaus != t2->ndaus || t1->ndaus != result->ndaus)
        return 1;
    if (t1->ndaus)
    {
        unsigned i;
        assert(t1->daus);
        assert(t2->daus);
        assert(result->daus);
        for (i=0; i<t1->ndaus; ++i)
            if (ot_combine_payloads(t1->daus[i], t2->daus[i], result->daus[i],
                                    cmb, combinerData))
                return 1;
    }
    else if (cmb)
        result->payload = cmb(t1->payload, t2->payload, combinerData);
    return 0;
}

int ot_compare(const Ordered_tree_node *t1, const Ordered_tree_node *t2,
               double eps, OTPayload_comparator cmp)
{
    if (t1->n_in_parent != t2->n_in_parent)
        return 1;
    if (t1->split_dim != t2->split_dim)
        return 1;
    if (t1->ndaus != t2->ndaus)
        return 1;
    if (t1->ndaus)
    {
        unsigned i;
        assert(t1->bounds);
        assert(t2->bounds);
        assert(t1->centers);
        assert(t2->centers);
        assert(t1->daus);
        assert(t2->daus);
        for (i=0; i<t1->ndaus-1; ++i)
            if (fabs(t1->bounds[i] - t2->bounds[i]) > eps)
                return 1;
        for (i=0; i<t1->ndaus; ++i)
            if (fabs(t1->centers[i] - t2->centers[i]) > eps)
                return 1;
        for (i=0; i<t1->ndaus; ++i)
            if (ot_compare(t1->daus[i], t2->daus[i], eps, cmp))
                return 1;
    }
    else
    {
        assert(t1->bounds == 0);
        assert(t2->bounds == 0);
        assert(t1->centers == 0);
        assert(t2->centers == 0);
        assert(t1->daus == 0);
        assert(t2->daus == 0);        
        if (cmp)
        {
            if (cmp(t1->payload, t2->payload))
                return 1;
        }
        else if (memcmp(&t1->payload, &t2->payload, sizeof(OTPayload)))
            return 1;
    }
    return 0;
}

static void max_daus_at_depth(const Ordered_tree_node *topnode,
			      const unsigned treedepth,
			      const unsigned thisdepth,
			      unsigned *maxdaus)
{
    if (thisdepth >= treedepth)
	return;
    assert(topnode);
    if (topnode->ndaus > maxdaus[thisdepth])
	maxdaus[thisdepth] = topnode->ndaus;
    if (topnode->ndaus)
    {
        unsigned i;
        assert(topnode->daus);
        for (i=0; i<topnode->ndaus; ++i)
            max_daus_at_depth(topnode->daus[i], treedepth,
			      thisdepth+1, maxdaus);
    }
}

static void min_daus_at_depth(const Ordered_tree_node *topnode,
			      const unsigned treedepth,
			      const unsigned thisdepth,
			      unsigned *mindaus)
{
    if (thisdepth >= treedepth)
	return;
    assert(topnode);
    if (topnode->ndaus < mindaus[thisdepth])
	mindaus[thisdepth] = topnode->ndaus;
    if (topnode->ndaus)
    {
        unsigned i;
        assert(topnode->daus);
        for (i=0; i<topnode->ndaus; ++i)
            min_daus_at_depth(topnode->daus[i], treedepth,
			      thisdepth+1, mindaus);
    }
}

static int check_shape(const Ordered_tree_node *topnode,
                       const unsigned thisdepth,
                       const Ordered_tree_shape *shape)
{
    assert(topnode);
    if (topnode->ndaus)
    {
        unsigned i;
        assert(topnode->daus);
        for (i=0; i<topnode->ndaus; ++i)
            if (check_shape(topnode->daus[i], thisdepth+1, shape))
                return 1;
        if (thisdepth >= shape->nlevels)
            return 1;
        if (shape->splits[thisdepth].split_dim != topnode->split_dim)
            return 1;
        if (shape->splits[thisdepth].nsplits != topnode->ndaus)
            return 1;
    }
    return 0;
}

static void check_depth(const Ordered_tree_node *topnode,
			const unsigned thisdepth,
			unsigned *mindepth, unsigned *maxdepth)
{
    assert(topnode);
    if (topnode->ndaus)
    {
        unsigned i;
        assert(topnode->daus);
        for (i=0; i<topnode->ndaus; ++i)
            check_depth(topnode->daus[i], thisdepth+1, mindepth, maxdepth);
    }
    else
    {
	if (thisdepth < *mindepth)
	    *mindepth = thisdepth;
	if (thisdepth > *maxdepth)
	    *maxdepth = thisdepth;
    }
}

void ot_min_max_depth(const Ordered_tree_node *topnode,
		      unsigned *mindepth, unsigned *maxdepth)
{
    *maxdepth = 0;
    *mindepth = INT_MAX;
    check_depth(topnode, 0, mindepth, maxdepth);
}

void ot_max_daus(const Ordered_tree_node *topnode,
		 const unsigned treedepth,
		 unsigned *maxdaus)
{
    unsigned i;
    for (i=0; i<treedepth; ++i)
	maxdaus[i] = 0;
    max_daus_at_depth(topnode, treedepth, 0, maxdaus);
}

void ot_min_daus(const Ordered_tree_node *topnode,
		 const unsigned treedepth,
		 unsigned *mindaus)
{
    unsigned i;
    for (i=0; i<treedepth; ++i)
	mindaus[i] = UINT_MAX;
    min_daus_at_depth(topnode, treedepth, 0, mindaus);
    for (i=0; i<treedepth; ++i)
        if (mindaus[i] == UINT_MAX)
            mindaus[i] = 0;
}

static void node_counter(void *userData, const Ordered_tree_node *node)
{
    unsigned *pc = (unsigned *)userData;
    (*pc)++;
}

unsigned ot_n_all_nodes(const Ordered_tree_node *topnode)
{
    unsigned n = 0;
    ot_walk(topnode, node_counter, &n, NULL, NULL);
    return n;
}

static void payload_counter(void *userData, OTPayload *p)
{
    unsigned *pc = (unsigned *)userData;
    (*pc)++;
}

unsigned ot_n_load_nodes(const Ordered_tree_node *topnode)
{
    unsigned n = 0;
    ot_walk(topnode, NULL, NULL, payload_counter, &n);
    return n;
}

typedef struct {
    FILE *f;
    OTPayload_text_dumper payDumper;
    void *payDumperData;
    unsigned inode;
} ot_dump_c_data;

typedef struct {
    XDR *xdrs;
    OTPayload_writer payDumper;
    void *payDumperData;
    unsigned inode;
} ot_dump_binary_data;

static void ot_binary_dump_firstpass(void *userData, const Ordered_tree_node *node)
{
    ot_dump_binary_data *d = (ot_dump_binary_data*)userData;
    ((Ordered_tree_node *)node)->inode = d->inode;
    ++d->inode;
}

static void ot_binary_dump_secondpass(void *userData, const Ordered_tree_node *node_in)
{
    Ordered_tree_node *node = (Ordered_tree_node *)node_in;
    ot_dump_binary_data *d = (ot_dump_binary_data*)userData;

    unsigned tmp = 0xdeadbeef;
    assert(xdr_u_int(d->xdrs, &tmp));
    if (node->parent)
        tmp = node->parent->inode;
    else
        tmp = UINT_MAX;
    assert(xdr_u_int(d->xdrs, &tmp));
    assert(xdr_u_int(d->xdrs, &node->n_in_parent));
    assert(xdr_u_int(d->xdrs, &node->split_dim));
    assert(xdr_u_int(d->xdrs, &node->ndaus));
    if (node->ndaus > 0)
    {
        u_int ndaus = node->ndaus;
        u_int ndausm1 = node->ndaus-1;
        assert(node->ndaus > 1);
        assert(xdr_array(d->xdrs, (char **)&node->bounds, &ndausm1, ndausm1,
                         sizeof(float), (xdrproc_t)xdr_float));
        assert(xdr_array(d->xdrs, (char **)&node->centers, &ndaus, ndaus,
                         sizeof(float), (xdrproc_t)xdr_float));
    }
    else
        d->payDumper(d->payDumperData, d->xdrs, node->payload);
}

Ordered_tree_node* ot_binary_restore(XDR *xdrs, OTPayload_reader payReader,
                                     void *payReaderData,
                                     OTPayload_destructor destr)
{
    unsigned i, n_nodes, tmp, parent_inode;
    Ordered_tree_node **nodes;
    Ordered_tree_node *node;
    Ordered_tree_node *topnode = 0;
    Ordered_tree_split split;

    if (xdr_u_int(xdrs, &n_nodes) != 1)
        return 0;
    nodes = (Ordered_tree_node **)calloc(1, n_nodes*sizeof(Ordered_tree_node *));
    assert(nodes);

    /* First pass: read the data */
    for (i=0; i<n_nodes; ++i)
    {
        if (xdr_u_int(xdrs, &tmp) != 1)
            goto fail;
        if (tmp != 0xdeadbeef)
            goto fail;

        node = (Ordered_tree_node *)calloc(1, sizeof(Ordered_tree_node));
        assert(node);
        nodes[i] = node;

        /* Read parent inode into inode */
        if (xdr_u_int(xdrs, &node->inode) != 1)
            goto fail;
        if (xdr_u_int(xdrs, &node->n_in_parent) != 1)
            goto fail;
        if (xdr_u_int(xdrs, &split.split_dim) != 1)
            goto fail;
        if (xdr_u_int(xdrs, &split.nsplits) != 1)
            goto fail;

        if (split.nsplits)
        {
            ot_split_node(node, &split);
            u_int ndaus = node->ndaus;
            u_int ndausm1 = node->ndaus-1;
            if (xdr_array(xdrs, (char **)&node->bounds, &ndausm1, ndausm1,
                          sizeof(float), (xdrproc_t)xdr_float) != 1)
                goto fail;
            if (xdr_array(xdrs, (char **)&node->centers, &ndaus, ndaus,
                          sizeof(float), (xdrproc_t)xdr_float) != 1)
                goto fail;
        }
        else
        {
            if (payReader(payReaderData, xdrs, &node->payload))
                goto fail;
        }
    }

    /* Second pass: restore parent-daughter relationships */
    for (i=0; i<n_nodes; ++i)
    {
        node = nodes[i];
        parent_inode = node->inode;
        node->inode = 0;
        if (parent_inode != UINT_MAX)
        {
            assert(parent_inode > i && parent_inode < n_nodes);
            node->parent = nodes[parent_inode];
            assert(node->parent->ndaus > node->n_in_parent);
            node->parent->daus[node->n_in_parent] = node;
        }
        else
        {
            assert(topnode == 0);
            topnode = node;
        }
    }

    if (topnode == 0)
        goto fail;
    free(nodes);
    return topnode;

fail:
    for (i=0; i<n_nodes; ++i)
        ot_delete_node(nodes[i], destr);
    free(nodes);
    return 0;
}

static void ot_dump_firstpass(void *userData, const Ordered_tree_node *node)
{
    unsigned i;
    ot_dump_c_data *d = (ot_dump_c_data *)userData;
    ((Ordered_tree_node *)node)->inode = d->inode;
    if (node->ndaus > 0)
    {
	/* Bounds */
        fprintf(d->f, "static float b%u[%u] = {\n", d->inode, node->ndaus-1);
        for (i=0; i<node->ndaus-1; ++i)
            fprintf(d->f, "  %.10e%s\n", node->bounds[i], i<node->ndaus-2 ? "," : "");
        fprintf(d->f, "};\n");
	/* Centers */
        fprintf(d->f, "static float c%u[%u] = {\n", d->inode, node->ndaus);
        for (i=0; i<node->ndaus; ++i)
            fprintf(d->f, "  %.10e%s\n", node->centers[i], i<node->ndaus-1 ? "," : "");
        fprintf(d->f, "};\n");
	/* Daughters */
        fprintf(d->f, "static T *d%u[%u];\n", d->inode, node->ndaus);
        fprintf(d->f, "static T n%u = {0,%u,%u,0,0,0,%u,0,0};\n",
                d->inode, node->n_in_parent, node->split_dim, node->ndaus);
    }
    else
    {
        fprintf(d->f, "static T n%u = {0,%u,%u,0,0,0,0,0,",
                d->inode, node->n_in_parent, node->split_dim);
        d->payDumper(d->payDumperData, d->f, node->payload);
        fprintf(d->f, "};\n");
    }
    ++d->inode;
}

static void ot_dump_secondpass(void *userData, const Ordered_tree_node *node)
{
    unsigned i;
    ot_dump_c_data *d = (ot_dump_c_data *)userData;
    if (node->parent)
        fprintf(d->f, "    n%u.parent=&n%u;\n",
                node->inode, node->parent->inode);
    if (node->ndaus > 0)
    {
        for (i=0; i<node->ndaus; ++i)
            fprintf(d->f, "    d%u[%u]=&n%u;\n",
                    node->inode, i, node->daus[i]->inode);
        fprintf(d->f, "    n%u.bounds=b%u;\n",
                node->inode, node->inode);
        fprintf(d->f, "    n%u.centers=c%u;\n",
                node->inode, node->inode);
        fprintf(d->f, "    n%u.daus=d%u;\n",
                node->inode, node->inode);
    }
}

void ot_binary_dump(XDR *xdrs, const Ordered_tree_node *topnode,
                    OTPayload_writer payDumper, void *payDumperData)
{
    ot_dump_binary_data dump;

    assert(xdrs);
    dump.xdrs = xdrs;
    dump.payDumper = payDumper;
    dump.payDumperData = payDumperData;
    dump.inode = 0;

    ot_walk(topnode, ot_binary_dump_firstpass, &dump, NULL, NULL);
    assert(xdr_u_int(xdrs, &dump.inode));
    ot_walk(topnode, ot_binary_dump_secondpass, &dump, NULL, NULL);
}

void ot_dump_c(const char *filename, const char *fcn,
               const Ordered_tree_node *topnode,
               OTPayload_text_dumper payDumper, void *payDumperData)
{
    ot_dump_c_data dump;

    FILE *f = fopen(filename, "w");
    assert(f);

    dump.f = f;
    dump.payDumper = payDumper;
    dump.payDumperData = payDumperData;
    dump.inode = 0;

    fprintf(f, "/* Computer-generated code. Do not edit! */\n\n");
    fprintf(f, "#include \"ordered_tree.h\"\n\n");
    fprintf(f, "#ifdef __cplusplus\n");
    fprintf(f, "extern \"C\" {\n");
    fprintf(f, "#endif\n\n");
    fprintf(f, "Ordered_tree_node* %s(void);\n\n", fcn);
    fprintf(f, "typedef Ordered_tree_node T;\n\n");
    ot_walk(topnode, ot_dump_firstpass, &dump, NULL, NULL);

    fprintf(f, "\nOrdered_tree_node* %s(void)\n", fcn);
    fprintf(f, "{\n");
    fprintf(f, "  static int firstcall=1;\n");
    fprintf(f, "  if (firstcall)\n");
    fprintf(f, "  {\n");
    fprintf(f, "    firstcall=0;\n");
    ot_walk(topnode, ot_dump_secondpass, &dump, NULL, NULL);
    fprintf(f, "  }\n");
    fprintf(f, "  return &n%u;\n", topnode->inode);
    fprintf(f, "}\n\n");
    fprintf(f, "#ifdef __cplusplus\n");
    fprintf(f, "}\n");
    fprintf(f, "#endif\n");

    fclose(f);
}

void ot_destroy_shape(Ordered_tree_shape *shape)
{
    if (shape)
    {
        if (shape->splits)
            free(shape->splits);
        free(shape);
    }
}

Ordered_tree_shape *ot_get_shape(const Ordered_tree_node *topnode)
{
    unsigned i, mindepth, maxdepth;
    Ordered_tree_shape *shape = 0;
    const Ordered_tree_node *node;

    assert(topnode);
    ot_min_max_depth(topnode, &mindepth, &maxdepth);
    if (mindepth != maxdepth)
        goto fail;

    shape = (Ordered_tree_shape *)malloc(sizeof(Ordered_tree_shape));
    assert(shape);
    shape->nlevels = maxdepth;
    shape->splits = (Ordered_tree_split *)malloc(
        maxdepth*sizeof(Ordered_tree_split));
    assert(shape->splits);

    /* Fill the tree shape using the leftmost branch */
    i = 0;
    for (node=topnode; node->ndaus; node=ot_subnode_at(node,0), ++i)
    {
        shape->splits[i].split_dim = node->split_dim;
        shape->splits[i].nsplits = node->ndaus;
    }
    assert(i == maxdepth);

    /* Check that the shape is the same in all branches */
    if (check_shape(topnode, 0, shape))
        goto fail;

    return shape;

 fail:
    ot_destroy_shape(shape);
    return NULL;
}

void ot_from_uniform(const Ordered_tree_node *topnode,
                     const float *u_in, const float *min_bound_in,
                     const float *max_bound_in, float *index)
{
    unsigned j, idim;
    int dim_found[MAX_DIM_NUMBER] = {0};
    float step;
    float u[MAX_DIM_NUMBER];
    float min_bound[MAX_DIM_NUMBER], max_bound[MAX_DIM_NUMBER];

    const Ordered_tree_node *current = topnode;

    assert(index);
    assert(u_in);
    assert(topnode);
    assert(min_bound_in);
    assert(max_bound_in);

    while (current->ndaus)
    {
        idim = current->split_dim;
        if (!dim_found[idim])
        {
            dim_found[idim] = 1;
            u[idim] = u_in[idim];
            if (u[idim] < 0.f)
                u[idim] = 0.f;
            if (u[idim] > 1.f)
                u[idim] = 1.f;
            min_bound[idim] = min_bound_in[idim];
            max_bound[idim] = max_bound_in[idim];
        }
        step = 1.f/(float)current->ndaus;
        j = u[idim]/step;
        if (j >= current->ndaus)
            j = current->ndaus - 1;
        u[idim] -= j*step;
        u[idim] /= step;
        if (u[idim] < 0.f)
            u[idim] = 0.f;
        if (u[idim] > 1.f)
            u[idim] = 1.f;
        if (j)
            min_bound[idim] = current->bounds[j-1];
        if (j < current->ndaus-1)
            max_bound[idim] = current->bounds[j];
	current = ot_subnode_at(current, j);
    }

    for (idim=0; idim<MAX_DIM_NUMBER; ++idim)
        if (dim_found[idim])
        {
            step = max_bound[idim] - min_bound[idim];
            index[idim] = min_bound[idim] + u[idim]*step;
        }
}

static void ot_coord_to_abs_cell(const Ordered_tree_node *topnode,
                                 const float *index, const float *min_bound_in,
                                 const float *max_bound_in,
                                 unsigned cell_number[MAX_DIM_NUMBER],
                                 unsigned cell_factor[MAX_DIM_NUMBER],
                                 float within_cell_deltas[MAX_DIM_NUMBER])
{
    unsigned j;
    float min_bound[MAX_DIM_NUMBER], max_bound[MAX_DIM_NUMBER];

    const Ordered_tree_node *current = topnode;

    assert(topnode);
    assert(index);
    assert(min_bound_in);
    assert(max_bound_in);

    for (j=0; j<MAX_DIM_NUMBER; ++j)
    {
        cell_number[j] = 0;
        cell_factor[j] = 0;
    }

    while (current->ndaus)
    {
        const unsigned idim = current->split_dim;
        assert(idim < MAX_DIM_NUMBER);
        if (cell_factor[idim] == 0)
        {
            cell_factor[idim] = current->ndaus;
            min_bound[idim] = min_bound_in[idim];
            max_bound[idim] = max_bound_in[idim];
        }
        else
            cell_factor[idim] *= current->ndaus;            
	j = range_lookup_flt(current->bounds, current->ndaus-1,
                             index[idim]);
        if (j)
            min_bound[idim] = current->bounds[j-1];
        if (j < current->ndaus-1)
            max_bound[idim] = current->bounds[j];
        cell_number[idim] *= current->ndaus;
        cell_number[idim] += j;
	current = ot_subnode_at(current, j);
    }

    for (j=0; j<MAX_DIM_NUMBER; ++j)
        if (cell_factor[j])
            within_cell_deltas[j] = (index[j] - min_bound[j])/
                (max_bound[j] - min_bound[j]);
        else
            within_cell_deltas[j] = 0.f;
}

static void ot_coord_to_abs_cell_2(const Ordered_tree_node *topnode,
                                   const float *index,
                                   unsigned cell_number[MAX_DIM_NUMBER],
                                   unsigned cell_factor[MAX_DIM_NUMBER],
                                   float within_cell_deltas[MAX_DIM_NUMBER])
{
    unsigned j;
    float min_bound[MAX_DIM_NUMBER], max_bound[MAX_DIM_NUMBER];
    float center[MAX_DIM_NUMBER];

    const Ordered_tree_node *current = topnode;

    assert(topnode);
    assert(index);

    for (j=0; j<MAX_DIM_NUMBER; ++j)
    {
        cell_number[j] = 0;
        cell_factor[j] = 0;
        min_bound[j] = -FLT_MAX;
        max_bound[j] = FLT_MAX;
    }

    while (current->ndaus)
    {
        const unsigned idim = current->split_dim;
        assert(idim < MAX_DIM_NUMBER);
        if (cell_factor[idim] == 0)
            cell_factor[idim] = current->ndaus;
        else
            cell_factor[idim] *= current->ndaus;            
	j = range_lookup_flt(current->bounds, current->ndaus-1,
                             index[idim]);
        if (j)
            min_bound[idim] = current->bounds[j-1];
        if (j < current->ndaus-1)
            max_bound[idim] = current->bounds[j];
        cell_number[idim] *= current->ndaus;
        cell_number[idim] += j;
        center[idim] = current->centers[j];
	current = ot_subnode_at(current, j);
    }

    for (j=0; j<MAX_DIM_NUMBER; ++j)
        if (cell_factor[j])
        {
            if (index[j] >= center[j])
                within_cell_deltas[j] = 0.5f + 0.5f*((index[j] - center[j])/
                        (max_bound[j] - center[j]));
            else
                within_cell_deltas[j] = 0.5f*((index[j] - min_bound[j])/
                        (center[j] - min_bound[j]));
        }
        else
            within_cell_deltas[j] = 0.f;
}

void ot_to_uniform(const Ordered_tree_node *topnode,
                   const float *index, const float *min_bound,
                   const float *max_bound, float *u)
{
    unsigned idim;
    unsigned cell_number[MAX_DIM_NUMBER], cell_factor[MAX_DIM_NUMBER];
    float deltas[MAX_DIM_NUMBER];

    ot_coord_to_abs_cell(topnode, index, min_bound, max_bound,
                         cell_number, cell_factor, deltas);
    for (idim=0; idim<MAX_DIM_NUMBER; ++idim)
        if (cell_factor[idim])
            u[idim] = (cell_number[idim]+deltas[idim])/cell_factor[idim];
}

double ot_block_density(const Ordered_tree_node *topnode,
                        const float *index, const float *min_bound_in,
                        const float *max_bound_in)
{
    unsigned ncells = 1;
    double area = 1.0;
    float min_bound[MAX_DIM_NUMBER], max_bound[MAX_DIM_NUMBER];
    unsigned dim_seen[MAX_DIM_NUMBER] = {0};
    const Ordered_tree_node *current = topnode;

    assert(topnode);
    assert(index);
    assert(min_bound_in);
    assert(max_bound_in);

    while (current->ndaus)
    {
        const unsigned idim = current->split_dim;
	const unsigned j = range_lookup_flt(
            current->bounds, current->ndaus-1, index[idim]);
        assert(idim < MAX_DIM_NUMBER);
        if (!dim_seen[idim])
        {
            dim_seen[idim] = 1;
            min_bound[idim] = min_bound_in[idim];
            max_bound[idim] = max_bound_in[idim];
        }
        if (j)
            min_bound[idim] = current->bounds[j-1];
        if (j < current->ndaus-1)
            max_bound[idim] = current->bounds[j];
        ncells *= current->ndaus;
	current = ot_subnode_at(current, j);
    }
    {
        unsigned idim;
        for (idim=0; idim<MAX_DIM_NUMBER; ++idim)
            if (dim_seen[idim])
                area *= (max_bound[idim] - min_bound[idim]);
    }
    return 1.0/area/ncells;
}

/* In the following function, "i_in_dim" is the array of
 * cell numbers in the "absolute" sense, rather than in
 * the sense of the sell number in the parent. Dimensionality
 * of "i_in_dim" works in the same way as the dimensionality
 * of array of coordinates. This is in contrast with "ot_cell_node"
 * and similar function where cell numbers are cell indices
 * in the parent node and whose dimensionality is the same
 * as the tree depth.
 */
static double ot_abs_cell_density(const Ordered_tree_node *topnode,
                                  const unsigned *i_in_dim_in,
                                  const float *min_bound_in,
                                  const float *max_bound_in,
                                  const unsigned *dim_cells_in)
{
    unsigned j, ncells = 1;
    double area = 1.0;
    float min_bound[MAX_DIM_NUMBER], max_bound[MAX_DIM_NUMBER];
    unsigned dim_cells[MAX_DIM_NUMBER] = {0}, i_in_dim[MAX_DIM_NUMBER];

    const Ordered_tree_node *current = topnode;

    while (current->ndaus)
    {
        const unsigned idim = current->split_dim;
        assert(idim < MAX_DIM_NUMBER);
        if (!dim_cells[idim])
        {
            dim_cells[idim] = dim_cells_in[idim];
            i_in_dim[idim] = i_in_dim_in[idim];
            min_bound[idim] = min_bound_in[idim];
            max_bound[idim] = max_bound_in[idim];
            ncells *= dim_cells_in[idim];
        }
        assert(i_in_dim[idim] < dim_cells[idim]);
        dim_cells[idim] /= current->ndaus;
        assert(dim_cells[idim]);
        j = i_in_dim[idim]/dim_cells[idim];

        if (j)
            min_bound[idim] = current->bounds[j-1];
        if (j < current->ndaus-1)
            max_bound[idim] = current->bounds[j];
	current = ot_subnode_at(current, j);

        i_in_dim[idim] -= dim_cells[idim]*j;
    }

    for (j=0; j<MAX_DIM_NUMBER; ++j)
        if (dim_cells[j])
            area *= (max_bound[j] - min_bound[j]);
    return 1.0/area/ncells;
}


inline static float harmonic2(const float a, const float b)
{
    if (a > 0.f || b > 0.f)
        return 2.f*a*b/(a + b);
    else
        return 0.f;
}

inline static float harmonic4(const float a, const float b,
                              const float c, const float d)
{
    if (a > 0.f && b > 0.f && c > 0.f && d > 0.f)
        return 4.f/(1.f/a + 1.f/b + 1.f/c + 1.f/d);
    else
        return 0.f;
}

inline static float interpolate_1d_halfdx(
    const float y0, const float y1, const float halfdx)
{
    return y0 + (y1 - y0)*2.f*halfdx;
}

inline static float interpolate_2d_halfdelta(
    const float z00, const float z10, const float z01, const float z11,
    const float halfdx, const float halfdy)
{
    const float dx = 2.f*halfdx;
    const float dy = 2.f*halfdy;
    return (dx - 1.0)*(dy - 1.0)*z00 + dx*z10 + dy*z01 + 
            dx*dy*(z11-z01-z10);
}

double ot_polygon_density(const Ordered_tree_node *topnode,
                          const float *index, const float *min_bound_in,
                          const float *max_bound_in)
{
    unsigned j, ndims=0;
    unsigned cell_number[MAX_DIM_NUMBER], dim_cells[MAX_DIM_NUMBER];
    unsigned dim_nums[MAX_DIM_NUMBER], cellnum[MAX_DIM_NUMBER];
    float deltas[MAX_DIM_NUMBER];

    ot_coord_to_abs_cell(topnode, index, min_bound_in, max_bound_in,
                         cell_number, dim_cells, deltas);
    for (j=0; j<MAX_DIM_NUMBER; ++j)
        if (dim_cells[j])
            dim_nums[ndims++] = j;

    switch (ndims)
    {
    case 1:
    {
        float dx, z00, z10;
        int ix;

        const unsigned idim = dim_nums[0];
        if (deltas[idim] < 0.5f)
        {
            ix = (int)cell_number[idim] - 1;
            dx = deltas[idim] + 0.5f;
        }
        else
        {
            ix = (int)cell_number[idim];
            dx = deltas[idim] - 0.5f;
        }
        if (ix < 0)
        {
            ix = 0;
            dx = 0.f;
        }
        else if (ix >= (int)dim_cells[idim]-1)
        {
            ix = dim_cells[idim]-2;
            dx = 1.f;
        }
        cellnum[idim] = ix;
        z00 = ot_abs_cell_density(topnode, cellnum, min_bound_in,
                                  max_bound_in, dim_cells);
        cellnum[idim] = ix+1;
        z10 = ot_abs_cell_density(topnode, cellnum, min_bound_in,
                                  max_bound_in, dim_cells);
        if (dx > 0.5f)
            return interpolate_1d_halfdx(harmonic2(z00, z10), z10, dx-0.5f);
        else
            return interpolate_1d_halfdx(z00, harmonic2(z00, z10), dx);
    }
    case 2:
    {
        float dx, dy, z00, z10, z01, z11, have;
        int ix, iy;

        const unsigned idimx = dim_nums[0];
        const unsigned idimy = dim_nums[1];

        if (deltas[idimx] < 0.5f)
        {
            ix = (int)cell_number[idimx] - 1;
            dx = deltas[idimx] + 0.5f;
        }
        else
        {
            ix = (int)cell_number[idimx];
            dx = deltas[idimx] - 0.5f;
        }
        if (ix < 0)
        {
            ix = 0;
            dx = 0.f;
        }
        else if (ix >= (int)dim_cells[idimx]-1)
        {
            ix = dim_cells[idimx]-2;
            dx = 1.f;
        }

        if (deltas[idimy] < 0.5f)
        {
            iy = (int)cell_number[idimy] - 1;
            dy = deltas[idimy] + 0.5f;
        }
        else
        {
            iy = (int)cell_number[idimy];
            dy = deltas[idimy] - 0.5f;
        }
        if (iy < 0)
        {
            iy = 0;
            dy = 0.f;
        }
        else if (iy >= (int)dim_cells[idimy]-1)
        {
            iy = dim_cells[idimy]-2;
            dy = 1.f;
        }

        cellnum[idimx] = ix;
        cellnum[idimy] = iy;
        z00 = ot_abs_cell_density(topnode, cellnum, min_bound_in,
                                  max_bound_in, dim_cells);
        cellnum[idimx] = ix+1;
        cellnum[idimy] = iy;
        z10 = ot_abs_cell_density(topnode, cellnum, min_bound_in,
                                  max_bound_in, dim_cells);
        cellnum[idimx] = ix;
        cellnum[idimy] = iy+1;
        z01 = ot_abs_cell_density(topnode, cellnum, min_bound_in,
                                  max_bound_in, dim_cells);
        cellnum[idimx] = ix+1;
        cellnum[idimy] = iy+1;
        z11 = ot_abs_cell_density(topnode, cellnum, min_bound_in,
                                  max_bound_in, dim_cells);
        have = harmonic4(z00, z10, z01, z11);
        if (dx > 0.5f)
        {
            if (dy > 0.5f)
                return interpolate_2d_halfdelta(
                    have, harmonic2(z10, z11),
                    harmonic2(z01, z11), z11, dx-0.5f, dy-0.5f);
            else
                return interpolate_2d_halfdelta(
                    harmonic2(z00, z10), z10, have,
                    harmonic2(z10, z11), dx-0.5f, dy);
        }
        else
        {
            if (dy > 0.5f)
                return interpolate_2d_halfdelta(
                    harmonic2(z00, z01), have, z01,
                    harmonic2(z01, z11), dx, dy-0.5f);
            else
                return interpolate_2d_halfdelta(
                    z00, harmonic2(z00, z10),
                    harmonic2(z00, z01), have, dx, dy);
        }
    }
    default:
        fprintf(stderr, "Polygon tree density is supported "
                "for 1 and 2 dimensions only, not %u\n", ndims);
        return 0.0;
    }
}

static void ot_uniform_to_abs_cell(const Ordered_tree_node *topnode,
                                   const float *u_in, const float *min_bound_in,
                                   const float *max_bound_in,
                                   unsigned cell_number[MAX_DIM_NUMBER],
                                   unsigned cell_factor[MAX_DIM_NUMBER],
                                   float min_bound[MAX_DIM_NUMBER],
                                   float max_bound[MAX_DIM_NUMBER],
                                   float u[MAX_DIM_NUMBER])
{
    unsigned j;
    float step;

    const Ordered_tree_node *current = topnode;

    assert(u_in);
    assert(topnode);
    assert(min_bound_in);
    assert(max_bound_in);

    for (j=0; j<MAX_DIM_NUMBER; ++j)
    {
        cell_number[j] = 0;
        cell_factor[j] = 0;
    }

    while (current->ndaus)
    {
        const unsigned idim = current->split_dim;
        if (!cell_factor[idim])
        {
            cell_factor[idim] = current->ndaus;
            u[idim] = u_in[idim];
            if (u[idim] < 0.f)
                u[idim] = 0.f;
            if (u[idim] > 1.f)
                u[idim] = 1.f;
            min_bound[idim] = min_bound_in[idim];
            max_bound[idim] = max_bound_in[idim];
        }
        else
            cell_factor[idim] *= current->ndaus;
        step = 1.f/(float)current->ndaus;
        j = u[idim]/step;
        if (j >= current->ndaus)
            j = current->ndaus - 1;
        u[idim] -= j*step;
        u[idim] /= step;
        if (u[idim] < 0.f)
            u[idim] = 0.f;
        if (u[idim] > 1.f)
            u[idim] = 1.f;
        if (j)
            min_bound[idim] = current->bounds[j-1];
        if (j < current->ndaus-1)
            max_bound[idim] = current->bounds[j];
        cell_number[idim] *= current->ndaus;
        cell_number[idim] += j;
	current = ot_subnode_at(current, j);
    }
}

void ot_polygon_random(const Ordered_tree_node *topnode,
                       const float *u, const float *min_bound_in,
                       const float *max_bound_in, float *index)
{
    unsigned j, ndims=0;
    unsigned cell_number[MAX_DIM_NUMBER], dim_cells[MAX_DIM_NUMBER];
    unsigned dim_nums[MAX_DIM_NUMBER], cellnum[MAX_DIM_NUMBER];
    float min_bound[MAX_DIM_NUMBER], max_bound[MAX_DIM_NUMBER];
    float deltas[MAX_DIM_NUMBER];

    ot_uniform_to_abs_cell(topnode, u, min_bound_in, max_bound_in,
                           cell_number, dim_cells, min_bound, max_bound, deltas);
    for (j=0; j<MAX_DIM_NUMBER; ++j)
        if (dim_cells[j])
            dim_nums[ndims++] = j;

    switch (ndims)
    {
    case 1:
    {
        float dx, z00, z10, cdf_split, have, s;
        int ix;

        const unsigned idim = dim_nums[0];
        if (deltas[idim] < 0.5f)
        {
            ix = (int)cell_number[idim] - 1;
            dx = deltas[idim] + 0.5f;
        }
        else
        {
            ix = (int)cell_number[idim];
            dx = deltas[idim] - 0.5f;
        }
        if (ix < 0 || ix >= (int)dim_cells[idim]-1)
        {
            /* We are on one of the edges.
             * Generate flat random numbers here.
             */
            const float step = max_bound[idim] - min_bound[idim];
            index[idim] = min_bound[idim] + deltas[idim]*step;
            return;
        }
        cellnum[idim] = ix;
        z00 = ot_abs_cell_density(topnode, cellnum, min_bound_in,
                                  max_bound_in, dim_cells);
        cellnum[idim] = ix+1;
        z10 = ot_abs_cell_density(topnode, cellnum, min_bound_in,
                                  max_bound_in, dim_cells);
        have = harmonic2(z00, z10);
        cdf_split = (z00 + 3.f*z10)/4.f/(z00 + z10);
        if (dx > cdf_split)
        {
            s = 0.5f*standard_linear_density_invcdf((dx - cdf_split)/(1.f - cdf_split),
                                                    2.0*have/(have + z10));
            ++ix;
        }
        else
            s = 0.5f*(1.f+standard_linear_density_invcdf(dx/cdf_split, 2.0*z00/(z00+have)));
        if (ix == (int)cell_number[idim])
        {
            const float step = max_bound[idim] - min_bound[idim];
            index[idim] = min_bound[idim] + s*step;
        }
        else
        {
            /* The probability has flown into another cell */
            deltas[idim] = (ix + s)/dim_cells[idim];
            ot_from_uniform(topnode, deltas, min_bound_in, max_bound_in, index);
        }
    }
    break;

    case 2:
    {
        float dx, dy, z00, z10, z01, z11, have, cdf_split_x, cdf_split_y;
        double sx, sy;
        int ix, iy, overwritex = 0, overwritey = 0;

        const unsigned idimx = dim_nums[0];
        const unsigned idimy = dim_nums[1];

        if (deltas[idimx] < 0.5f)
        {
            ix = (int)cell_number[idimx] - 1;
            dx = deltas[idimx] + 0.5f;
        }
        else
        {
            ix = (int)cell_number[idimx];
            dx = deltas[idimx] - 0.5f;
        }
        if (ix < 0)
        {
            ix = 0;
            dx = 0.f;
            overwritex = 1;
        }
        else if (ix >= (int)dim_cells[idimx]-1)
        {
            ix = dim_cells[idimx]-2;
            dx = 1.f;
            overwritex = 1;
        }

        if (deltas[idimy] < 0.5f)
        {
            iy = (int)cell_number[idimy] - 1;
            dy = deltas[idimy] + 0.5f;
        }
        else
        {
            iy = (int)cell_number[idimy];
            dy = deltas[idimy] - 0.5f;
        }
        if (iy < 0)
        {
            iy = 0;
            dy = 0.f;
            overwritey = 1;
        }
        else if (iy >= (int)dim_cells[idimy]-1)
        {
            iy = dim_cells[idimy]-2;
            dy = 1.f;
            overwritey = 1;
        }

        cellnum[idimx] = ix;
        cellnum[idimy] = iy;
        z00 = ot_abs_cell_density(topnode, cellnum, min_bound_in,
                                  max_bound_in, dim_cells);
        cellnum[idimx] = ix+1;
        cellnum[idimy] = iy;
        z10 = ot_abs_cell_density(topnode, cellnum, min_bound_in,
                                  max_bound_in, dim_cells);
        cellnum[idimx] = ix;
        cellnum[idimy] = iy+1;
        z01 = ot_abs_cell_density(topnode, cellnum, min_bound_in,
                                  max_bound_in, dim_cells);
        cellnum[idimx] = ix+1;
        cellnum[idimy] = iy+1;
        z11 = ot_abs_cell_density(topnode, cellnum, min_bound_in,
                                  max_bound_in, dim_cells);
        have = harmonic4(z00, z10, z01, z11);
        cdf_split_x = (have/harmonic2(z00,z01) + 2.f + z10/(z00 + z10) + z11/(z01 + z11))/8.f;
        if (dx > cdf_split_x)
        {
            cdf_split_y = (have/z10 + 3.f + 2.f*z00/(z00 + z10) - 2.f*z10/(z10 + z11))/16.f/(1.f - cdf_split_x);
            if (dy > cdf_split_y)
            {
                standard_linear_density_2d_inverse(
                    (dx - cdf_split_x)/(1.f - cdf_split_x), (dy - cdf_split_y)/(1.f - cdf_split_y),
                    have, harmonic2(z10, z11), harmonic2(z01, z11), z11, &sx, &sy);
                sx *= 0.5f;
                sy *= 0.5f;
                ++ix;
                ++iy;
            }
            else
            {
                standard_linear_density_2d_inverse(
                    (dx - cdf_split_x)/(1.f - cdf_split_x), dy/cdf_split_y,
                    harmonic2(z00, z10), z10, have, harmonic2(z10, z11), &sx, &sy);
                sx *= 0.5;
                sy = 0.5*(sy + 1.0);
                ++ix;
            }
        }
        else
        {
            cdf_split_y = (have/z00 + 1.f + 2.f*z01/(z00 + z01) + 2.f*z10/(z00 + z10))/16.f/cdf_split_x;
            if (dy > cdf_split_y)
            {
                standard_linear_density_2d_inverse(
                    dx/cdf_split_x, (dy - cdf_split_y)/(1.f - cdf_split_y),
                    harmonic2(z00, z01), have, z01, harmonic2(z01, z11), &sx, &sy);
                sx = 0.5*(sx + 1.0);
                sy *= 0.5;
                ++iy;
            }
            else
            {
                standard_linear_density_2d_inverse(
                    dx/cdf_split_x, dy/cdf_split_y,
                    z00, harmonic2(z00, z10), harmonic2(z00, z01), have, &sx, &sy);
                sx = 0.5*(sx + 1.0);
                sy = 0.5*(sy + 1.0);
            }
        }

        if (overwritex)
            sx = deltas[idimx];
        if (overwritey)
            sy = deltas[idimy];

        if (ix == (int)cell_number[idimx] && iy == (int)cell_number[idimy])
        {
            float step = max_bound[idimx] - min_bound[idimx];
            index[idimx] = min_bound[idimx] + sx*step;
            step = max_bound[idimy] - min_bound[idimy];
            index[idimy] = min_bound[idimy] + sy*step;
        }
        else
        {
            deltas[idimx] = (ix + sx)/dim_cells[idimx];
            deltas[idimy] = (iy + sy)/dim_cells[idimy];
            ot_from_uniform(topnode, deltas, min_bound_in, max_bound_in, index);
        }
    }
    break;

    default:
        fprintf(stderr, "Polygon random number generation is supported "
                "for 1 and 2 dimensions only, not %u\n", ndims);
    }
}

static OTPayload ot_abs_cell_payload(const Ordered_tree_node *topnode,
                                     const unsigned *i_in_dim_in,
                                     const unsigned *dim_cells_in)
{
    unsigned j;
    unsigned dim_cells[MAX_DIM_NUMBER] = {0}, i_in_dim[MAX_DIM_NUMBER];

    const Ordered_tree_node *current = topnode;

    while (current->ndaus)
    {
        const unsigned idim = current->split_dim;
        if (!dim_cells[idim])
        {
            dim_cells[idim] = dim_cells_in[idim];
            i_in_dim[idim] = i_in_dim_in[idim];
        }
        assert(i_in_dim[idim] < dim_cells[idim]);
        dim_cells[idim] /= current->ndaus;
        assert(dim_cells[idim]);
        j = i_in_dim[idim]/dim_cells[idim];
	current = ot_subnode_at(current, j);
        i_in_dim[idim] -= dim_cells[idim]*j;
    }

    return current->payload;
}

OTPayload ot_interpolate_payload(const Ordered_tree_node *topnode,
                                 const float *index)
{
    unsigned j, icycle, maxcycle, ndims=0;
    unsigned cell_number[MAX_DIM_NUMBER], dim_cells[MAX_DIM_NUMBER];
    unsigned dim_nums[MAX_DIM_NUMBER];
    float deltas[MAX_DIM_NUMBER];
    int ix[MAX_DIM_NUMBER];
    float dx[MAX_DIM_NUMBER];
    OTPayload sum = 0;

    ot_coord_to_abs_cell_2(topnode, index, cell_number, dim_cells, deltas);
    for (j=0; j<MAX_DIM_NUMBER; ++j)
        if (dim_cells[j])
            dim_nums[ndims++] = j;

    for (j=0; j<ndims; ++j)
    {
        const unsigned idim = dim_nums[j];
        if (deltas[idim] < 0.5f)
        {
            ix[j] = (int)cell_number[idim] - 1;
            dx[j] = deltas[idim] + 0.5f;
        }
        else
        {
            ix[j] = (int)cell_number[idim];
            dx[j] = deltas[idim] - 0.5f;
        }
        if (ix[j] < 0)
        {
            ix[j] = 0;
            dx[j] = 0.f;
        }
        else if (ix[j] >= (int)dim_cells[idim]-1)
        {
            ix[j] = dim_cells[idim]-2;
            dx[j] = 1.f;
        }
    }

    maxcycle = 1U << ndims;
    for (icycle=0; icycle<maxcycle; ++icycle)
    {
        double w = 1.0;
        for (j=0; j<ndims; ++j)
        {
            const unsigned idim = dim_nums[j];
            if (icycle & (1U << j))
            {
                w *= dx[j];
                cell_number[idim] = ix[j] + 1;
            }
            else
            {
                w *= (1.0 - dx[j]);
                cell_number[idim] = ix[j];
            }
        }
        sum += ot_abs_cell_payload(topnode, cell_number, dim_cells)*w;
    }

    return sum;
}

typedef struct {
    float *min_bound;
    float *max_bound;
    int *dim_found;
} ot_extremum_bounds_data;

static void ot_extremum_bounds_visitor(void *userData,
                                       const Ordered_tree_node *node)
{
    assert(node);
    if (node->ndaus)
    {
        ot_extremum_bounds_data *data = (ot_extremum_bounds_data *)userData;
        const unsigned idim = node->split_dim;
        if (!data->dim_found[idim])
        {
            data->dim_found[idim] = 1;
            data->min_bound[idim] = FLT_MAX;
            data->max_bound[idim] = -FLT_MAX;
        }
        if (node->bounds[0] < data->min_bound[idim])
            data->min_bound[idim] = node->bounds[0];
        if (node->bounds[node->ndaus-2] > data->max_bound[idim])
            data->max_bound[idim] = node->bounds[node->ndaus-2];
    }
}

void ot_extremum_bounds(const Ordered_tree_node *topnode,
                        float *min_bound, float *max_bound)
{
    int dim_found[MAX_DIM_NUMBER] = {0};
    unsigned i, maxdim = 0;

    ot_extremum_bounds_data data;
    data.min_bound = min_bound;
    data.max_bound = max_bound;
    data.dim_found = dim_found;

    assert(topnode);
    ot_walk(topnode, ot_extremum_bounds_visitor, &data, 0, 0);

    for (i=0; i<MAX_DIM_NUMBER; ++i)
        if (dim_found[i])
            maxdim = i+1;

    for (i=0; i<maxdim; ++i)
        if (!dim_found[i])
        {
            min_bound[i] = 1.f;
            max_bound[i] = -1.f;
        }
}

#ifdef __cplusplus
}
#endif
