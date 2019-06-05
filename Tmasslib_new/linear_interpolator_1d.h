#ifndef LINEAR_INTERPOLATOR_1D_H_
#define LINEAR_INTERPOLATOR_1D_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
    unsigned nxpoints;
    float xmin;
    float xmax;
    const float *data;
} Interpolator_data_1d;

float linear_interpolate_1d(const Interpolator_data_1d *d, float x);

#ifdef __cplusplus
}
#endif

#endif /* LINEAR_INTERPOLATOR_1D_H_ */
