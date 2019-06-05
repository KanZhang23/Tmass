#ifndef RATIO_TF_SHIFT_H_
#define RATIO_TF_SHIFT_H_

#include "ordered_tree.h"

#ifdef __cplusplus
extern "C" {
#endif

double ratio_tf_shift(double predictor, double m94,
                      double parton_eta, double jet_eta, int isB);

void set_ratio_shift_tree(const Ordered_tree_node *tree, int isB);

#ifdef __cplusplus
}
#endif

#endif /* RATIO_TF_SHIFT_H_ */
