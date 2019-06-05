#include <limits.h>
#include <assert.h>
#include <stdlib.h>

#include "ordered_tree_collection.h"

#ifdef __cplusplus
extern "C" {
#endif

Ordered_tree_node* ot_get_indexed_tree(const Ordered_tree_collection *coll,
                                       const float *coords)
{
    int i = *ot_find_payload(coll->index, coords, INT_MAX);
    assert(i >= 0 && i < (int)coll->n_trees);
    return coll->trees[i];
}

void ot_destroy_collection(Ordered_tree_collection *coll,
                           OTPayload_destructor destr)
{
    if (coll)
    {
        ot_delete_node(coll->index, destr);
        if (coll->trees)
        {
            unsigned i;
            for (i=0; i<coll->n_trees; ++i)
                ot_delete_node(coll->trees[i], destr);
            free(coll->trees);
        }
        free(coll);
    }
}
    
void ot_write_collection(const char *filename,
                         const Ordered_tree_collection *coll,
                         OTPayload_writer payDumper, void *payDumperData)
{
    FILE *f;
    unsigned i;
    XDR xdrs;

    assert(filename);
    assert(coll);
    assert(coll->n_trees);
    assert(ot_n_load_nodes(coll->index) == coll->n_trees);
    f = fopen(filename, "wb");
    if (f == NULL)
    {
        fprintf(stderr, "Fatal error: failed to open file %s. Exiting.\n",
                filename);
        exit(EXIT_FAILURE);
    }
    xdrstdio_create(&xdrs, f, XDR_ENCODE);
    ot_binary_dump(&xdrs, coll->index, payDumper, payDumperData);
    for (i=0; i<coll->n_trees; ++i)
        ot_binary_dump(&xdrs, coll->trees[i], payDumper, payDumperData);
    xdr_destroy(&xdrs);
    fclose(f);
}

Ordered_tree_collection* ot_read_collection(
    const char *filename, OTPayload_reader payReader,
    void *payReaderData, OTPayload_destructor destr)
{
    Ordered_tree_collection *coll = 0;
    Ordered_tree_node *node;
    unsigned i;
    FILE *f;
    XDR xdrs;

    assert(filename);
    f = fopen(filename, "rb");
    if (f == NULL)
        return NULL;
    xdrstdio_create(&xdrs, f, XDR_DECODE);
    node = ot_binary_restore(&xdrs, payReader, payReaderData, destr);
    if (node == NULL)
        goto fail;

    coll = (Ordered_tree_collection *)calloc(1, sizeof(Ordered_tree_collection));
    assert(coll);
    coll->n_trees = ot_n_load_nodes(node);
    assert(coll->n_trees);
    coll->index = node;
    coll->trees = (Ordered_tree_node **)calloc(1, coll->n_trees*
                                               sizeof(Ordered_tree_node *));
    assert(coll->trees);
    for (i=0; i<coll->n_trees; ++i)
    {
        node = ot_binary_restore(&xdrs, payReader, payReaderData, destr);
        if (node == NULL)
            goto fail;
        coll->trees[i] = node;
    }

    xdr_destroy(&xdrs);
    fclose(f);
    return coll;

fail:
    xdr_destroy(&xdrs);
    fclose(f);
    ot_destroy_collection(coll, destr);
    return NULL;
}

int ot_compare_collections(
    const Ordered_tree_collection *c1, const Ordered_tree_collection *c2,
    double eps, OTPayload_comparator cmp)
{
    unsigned i;

    assert(c1);
    assert(c2);
    if (c1->n_trees != c2->n_trees)
        return 1;
    if (ot_compare(c1->index, c2->index, eps, cmp))
        return 1;
    for (i=0; i<c1->n_trees; ++i)
        if (ot_compare(c1->trees[i], c2->trees[i], eps, cmp))
            return 1;
    return 0;
}

#ifdef __cplusplus
}
#endif
