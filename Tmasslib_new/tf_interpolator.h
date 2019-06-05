#ifndef TF_INTERPOLATOR_H_
#define TF_INTERPOLATOR_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double (*convert)(double);
    float *data;
    unsigned nxpoints;
    unsigned nypoints;
    unsigned n_fcn_points;
    float xmin;
    float xmax;
    float ymin;
    float ymax;
    float fcn_arg_min;
    float fcn_arg_max;
    float fcn_low;
    float fcn_high;

    float bwx;
    float bwy;
    float bwarg;
} Tf_interpolator;

/* The following function assumes ownership of the "data" array */
const Tf_interpolator* create_tf_interpolator(
    double (*convert)(double), float *data,
    unsigned nxpoints, unsigned nypoints, unsigned n_fcn_points,
    float xmin, float xmax, float ymin, float ymax,
    float fcn_arg_min, float fcn_arg_max, float fcn_low, float fcn_high);

void destroy_tf_interpolator(const Tf_interpolator *tf);

float tf_interpolate(const Tf_interpolator *tf, float x, float y, float arg);

#ifdef __cplusplus
}
#endif

#endif /* TF_INTERPOLATOR_H_ */
