#ifndef RANGE_LOOKUP_H_
#define RANGE_LOOKUP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* The number of ranges is larger than the number of bounds
 * by one (ranges at both ends are open). Each range includes
 * the left bound and excludes the right bound. Bounds must
 * be given in the strictly increasing order.
 */
unsigned range_lookup(const double *bounds, unsigned nbounds, double x);

int is_strictly_increasing(const double *bounds, unsigned nbounds);

#ifdef __cplusplus
}
#endif

#endif /* RANGE_LOOKUP_H_ */
