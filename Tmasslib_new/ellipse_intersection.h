#ifndef ELLIPSE_INTERSECTION_H_
#define ELLIPSE_INTERSECTION_H_

#ifdef __cplusplus
extern "C" {
#endif

/* The ellipse equations are used in the form
 *
 * a00*x^2 + 2*a01*x*y + a11*y^2 + b0*x + b1*y + c == 0
 */
typedef struct {
    double a00;
    double a01;
    double a11;
    double b0;
    double b1;
    double c;
} Ellipse_coeffs;

/* The following function returns the number of intersections (up to 4).
 * It works by finding zeros of Bezout's determinant (quartic equation).
 */
int ellipse_intersection(const Ellipse_coeffs e[2],
                         double x[4], double y[4]);

#ifdef __cplusplus
}
#endif

#endif /* ELLIPSE_INTERSECTION_H_ */
