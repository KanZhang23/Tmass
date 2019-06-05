#ifndef F_MATH_H
#define F_MATH_H

typedef float (*Float_function_u)(float, float);
typedef float (*Float_function_b)(float, float, float, float);

Float_function_u find_unary_funct(char *name);
Float_function_b find_binary_funct(char *name);

float f_not(float, float);
float f_abs(float, float);
float f_sign(float, float);
float f_sqrt(float, float);
float f_ssqrt(float, float);
float f_exp(float, float);
float f_log(float, float);
float f_slog(float, float);
float f_erf(float, float);
float f_gerf(float, float);

float f_ge(float, float, float, float);
float f_le(float, float, float, float);
float f_gt(float, float, float, float);
float f_lt(float, float, float, float);
float f_eq(float, float, float, float);
float f_ne(float, float, float, float);
float f_pow(float, float, float, float);
float f_or(float, float, float, float);
float f_and(float, float, float, float);
float f_bwor(float, float, float, float);
float f_bwand(float, float, float, float);
float f_min(float, float, float, float);
float f_max(float, float, float, float);
float f_hypot(float, float, float, float);

#endif /* not FMATH_H */
