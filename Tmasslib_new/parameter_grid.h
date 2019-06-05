#ifndef PARAMETER_GRID_H_
#define PARAMETER_GRID_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PARAM_GRID_FLAT = 0,
    PARAM_GRID_USER,
    N_PARAM_GRID_TYPES
} ParameterGridType;

typedef struct {
  double x;
  double weight;
} ParameterGridPoint;

void flat_parameter_grid(const ParameterGridPoint **grid,
                         unsigned n_points, double pmin, double pmax);

void print_parameter_grid(const ParameterGridPoint *grid,
                          unsigned n_points, FILE *stream);

#ifdef __cplusplus
}
#endif

#endif /* PARAMETER_GRID_H_ */
