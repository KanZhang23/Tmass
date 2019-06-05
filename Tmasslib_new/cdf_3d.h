#ifndef CDF_3D_H_
#define CDF_3D_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Size of the "data" array is (nxbins+1)*(nybins+1)*(nzbins+1) */
typedef struct cdf_3d_data_ {
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    double zmin;
    double zmax;
    double bwx;
    double bwy;
    double bwz;
    double *data;
    int nxbins;
    int nybins;
    int nzbins;
} Cdf_3d_data;

double cdf_3d_value(const Cdf_3d_data *cdf, double x, double y, double z);

double cdf_3d_rectangle_coverage(const Cdf_3d_data *cdf,
                                 double xmin, double ymin, double zmin,
                                 double xmax, double ymax, double zmax);

double cdf_3d_invcdf_x(const Cdf_3d_data *cdf, double cdfvalue, double eps);
double cdf_3d_invcdf_y(const Cdf_3d_data *cdf, double cdfvalue, double eps);
double cdf_3d_invcdf_z(const Cdf_3d_data *cdf, double cdfvalue, double eps);

void  cdf_3d_optimal_window(const Cdf_3d_data *cdf,
                            double x_center, double y_center, double z_center,
                            double coverage, double eps,
                            double *xmin, double *ymin, double *zmin,
                            double *xmax, double *ymax, double *zmax);

#ifdef __cplusplus
}
#endif

#endif /* CDF_3D_H_ */
