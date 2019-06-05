#ifndef POLY_3_2D_H_
#define POLY_3_2D_H_

#ifdef __cplusplus
extern "C" {
#endif

double poly_3_2d(double x, double y,
                 double xmin, double xmax,
                 double ymin, double ymax,
                 const double coeffs[10]);

double poly_3_2d_explicit(double x, double y,
                          double xmin, double xmax,
                          double ymin, double ymax,
                          double a, double b,
                          double c, double d,
                          double e, double f,
                          double g, double h,
                          double i, double j);

#ifdef __cplusplus
}
#endif

#endif /* POLY_3_2D_H_ */
