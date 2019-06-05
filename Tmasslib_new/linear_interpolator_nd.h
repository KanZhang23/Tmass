#ifndef LINEAR_INTERPOLATOR_ND_H_
#define LINEAR_INTERPOLATOR_ND_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Important! We assume that the first point
 * is at xmin, not xmin + 0.5*binwidth.
 */
typedef struct
{
    float xmin;
    float xmax;
    unsigned npoints;
} Interpolator_axis;

typedef struct 
{
    Interpolator_axis *axes;
    const float *data;
    unsigned *strides;
    unsigned dim;
    unsigned owns_data;
} Interpolator_data_nd;

Interpolator_data_nd* create_interpolator_nd(const Interpolator_axis* axes,
                                             unsigned n_axes, const float *data,
                                             unsigned assume_data_ownership);
void destroy_interpolator_nd(Interpolator_data_nd *d);

float linear_interpolate_nd(const Interpolator_data_nd *d,
                            const float *coords);

#ifdef __cplusplus
}
#endif

#endif /* LINEAR_INTERPOLATOR_ND_H_ */
