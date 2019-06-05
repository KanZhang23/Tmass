#ifndef CDF_2D_H_
#define CDF_2D_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Size of the "data" array is (nxbins+1)*(nybins+1) */
typedef struct cdf_2d_data_ {
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    double bwx;
    double bwy;
    double *data;
    int nxbins;
    int nybins;
} Cdf_2d_data;

double cdf_2d_value(const Cdf_2d_data *cdf, double x, double y);

double cdf_2d_rectangle_coverage(const Cdf_2d_data *cdf,
                                 double xmin, double ymin,
                                 double xmax, double ymax);

double cdf_2d_invcdf_x(const Cdf_2d_data *cdf, double cdfvalue, double eps);
double cdf_2d_invcdf_y(const Cdf_2d_data *cdf, double cdfvalue, double eps);

void  cdf_2d_optimal_window(const Cdf_2d_data *cdf,
                            double x_center, double y_center,
                            double coverage, double eps,
                            double *xmin, double *ymin,
                            double *xmax, double *ymax);

#ifdef __cplusplus
}
#endif

#endif /* CDF_2D_H_ */
