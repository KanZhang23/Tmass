#ifndef ORDERED_TREE_COLLECTION_H_
#define ORDERED_TREE_COLLECTION_H_

#include "ordered_tree.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    Ordered_tree_node *index;
    Ordered_tree_node **trees;
    unsigned n_trees;
} Ordered_tree_collection;

Ordered_tree_node* ot_get_indexed_tree(const Ordered_tree_collection *coll,
                                       const float *index_coords);

void ot_destroy_collection(Ordered_tree_collection *coll,
                           OTPayload_destructor destr);

void ot_write_collection(const char *filename,
                         const Ordered_tree_collection *coll,
                         OTPayload_writer payDumper, void *payDumperData);

Ordered_tree_collection* ot_read_collection(
    const char *filename, OTPayload_reader payReader,
    void *payReaderData, OTPayload_destructor destr);

int ot_compare_collections(
    const Ordered_tree_collection *c1, const Ordered_tree_collection *c2,
    double eps, OTPayload_comparator cmp);

#ifdef __cplusplus
}
#endif

#endif /* ORDERED_TREE_COLLECTION_H_ */
