#ifndef RANGE_LOOKUP_FLT_H_
#define RANGE_LOOKUP_FLT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* The number of ranges is larger than the number of bounds
 * by one (ranges at both ends are open). Each range includes
 * the left bound and excludes the right bound. Bounds must
 * be given in the strictly increasing order.
 */
unsigned range_lookup_flt(const float *bounds, unsigned nbounds, float x);

int is_strictly_increasing_flt(const float *bounds, unsigned nbounds);

#ifdef __cplusplus
}
#endif

#endif /* RANGE_LOOKUP_FLT_H_ */
