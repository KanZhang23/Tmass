#include "f_math.h"
#include <string.h>
#include <math.h>
#include <limits.h>

#define SQR2 1.414213562373095

struct f_funct_table_u
{
  char *name;
  Float_function_u f;
};

struct f_funct_table_b
{
  char *name;
  Float_function_b f;
};

static struct f_funct_table_u function_table_u[] = {
  {"~", f_not},
  {"abs", f_abs},
  {"sign", f_sign},
  {"sqrt", f_sqrt},
  {"ssqrt", f_ssqrt},
  {"exp", f_exp},
  {"log", f_log},
  {"slog", f_slog},
  {"erf", f_erf},
  {"gerf", f_gerf}
};

static struct f_funct_table_b function_table_b[] = {
  {">", f_gt},
  {"<", f_lt},
  {">=", f_ge},
  {"<=", f_le},
  {"==", f_eq},
  {"!=", f_ne},
  {"**", f_pow},
  {"^", f_pow},
  {"||", f_or},
  {"&&", f_and},
  {"|", f_bwor},
  {"&", f_bwand},
  {"min", f_min},
  {"max", f_max},
  {"hypot", f_hypot}
};

Float_function_u
find_unary_funct(char *name)
{
  int i, n;
  
  n = sizeof(function_table_u)/sizeof(function_table_u[0]);
  for (i=0; i<n; i++)
    if (strcmp(name, function_table_u[i].name) == 0)
      return function_table_u[i].f;
  
  return NULL;
}

Float_function_b
find_binary_funct(char *name)
{
  int i, n;
  
  n = sizeof(function_table_b)/sizeof(function_table_b[0]);
  for (i=0; i<n; i++)
    if (strcmp(name, function_table_b[i].name) == 0)
      return function_table_b[i].f;
  
  return NULL;
}

float f_not(float x, float dx)
{
  return (x == 0.f ? 1.f : 0.f);
}

float f_abs(float x, float dx)
{
  return (x < 0.f ? -x : x);
}

float f_sign(float x, float dx)
{
  return (x < 0.f ? -1.f : (x > 0.f ? 1.f : 0.f));
}

float f_sqrt(float x, float dx)
{
  return (float)sqrt((double)x);
}

float f_ssqrt(float x, float dx)
{
  if (x > 0.f)
    return (float)sqrt((double)x);
  else
    return 0.f;
}

float f_exp(float x, float dx)
{
  return (float)exp((double)x);
}

float f_log(float x, float dx)
{
  return (float)log((double)x);
}

float f_erf(float x, float dx)
{
  return (float)erf((double)x);
}

float f_gerf(float x, float dx)
{
  return (float)((1.0 + erf((double)x/SQR2))/2.0);
}

float f_slog(float x, float dx)
{
  if (x > 0.f)
    return (float)log((double)x);
  else
    return -1000.f;
}

float f_gt(float x, float dx, float y, float dy)
{
  return (float)(x > y);
}

float f_lt(float x, float dx, float y, float dy)
{
  return (float)(x < y);
}

float f_ge(float x, float dx, float y, float dy)
{
  return (float)(x >= y);
}

float f_le(float x, float dx, float y, float dy)
{
  return (float)(x <= y);
}

float f_eq(float x, float dx, float y, float dy)
{
  return (float)(x == y);
}

float f_ne(float x, float dx, float y, float dy)
{
  return (float)(x != y);
}

float f_pow(float x, float dx, float y, float dy)
{
  return (float)pow((double)x, (double)y);
}

float f_or(float x, float dx, float y, float dy)
{
  return (float)(x != 0.f || y != 0.f);
}

float f_and(float x, float dx, float y, float dy)
{
  return (float)(x != 0.f && y != 0.f);
}

float f_bwor(float x, float dx, float y, float dy)
{
  return (float)((int)x | (int)y);
}

float f_bwand(float x, float dx, float y, float dy)
{
  return (float)((int)x & (int)y);
}

float f_min(float x, float dx, float y, float dy)
{
  return (x < y ? x : y);
}

float f_max(float x, float dx, float y, float dy)
{
  return (x > y ? x : y);
}

float f_hypot(float x, float dx, float y, float dy)
{
  return (float)hypot((double)x, (double)y);
}
