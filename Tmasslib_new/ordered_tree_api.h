#ifndef ORDERED_TREE_API_H_
#define ORDERED_TREE_API_H_

#include "tcl.h"
#include "ordered_tree.h"

#ifdef __cplusplus
extern "C" {
#endif

int init_ordered_tree_api(Tcl_Interp *interp);
void cleanup_ordered_tree_api(void);
const Ordered_tree_node* get_ordered_tree_by_number(int i);

#ifdef __cplusplus
}
#endif

#endif /* ORDERED_TREE_API_H_ */
