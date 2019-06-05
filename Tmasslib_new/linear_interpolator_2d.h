#ifndef LINEAR_INTERPOLATOR_2D_H_
#define LINEAR_INTERPOLATOR_2D_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
    unsigned nxpoints;
    unsigned nypoints;
    float xmin;
    float xmax;
    float ymin;
    float ymax;
    const float *data;
} Interpolator_data_2d;

float linear_interpolate_2d(const Interpolator_data_2d *d, float x, float y);

#ifdef __cplusplus
}
#endif

#endif /* LINEAR_INTERPOLATOR_2D_H_ */
