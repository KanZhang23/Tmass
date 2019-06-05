#ifndef TAG_PROBABILITY_WEIGHT_H_
#define TAG_PROBABILITY_WEIGHT_H_

#include "jet_info.h"

#ifdef __cplusplus
extern "C" {
#endif

double tag_probability_weight(const jet_info *jets[4], int leptonCharge);

#ifdef __cplusplus
}
#endif

#endif /* TAG_PROBABILITY_WEIGHT_H_ */
