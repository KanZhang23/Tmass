#include <assert.h>

#include "parameter_grid.h"
#include "topmass_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

static ParameterGridPoint *param_grid = NULL;
static unsigned n_param_grid = 0;
static unsigned last_n_points = 0;
static double last_pmin = 0.0;
static double last_pmax = 0.0;

void flat_parameter_grid(const ParameterGridPoint **grid,
                         const unsigned n_points,
                         const double pmin, const double pmax)
{
    assert(grid && n_points);
    if (n_points != last_n_points || pmin != last_pmin || pmax != last_pmax)
    {
        last_n_points = n_points;
        last_pmin = pmin;
        last_pmax = pmax;
	get_static_memory((void **)&param_grid, sizeof(ParameterGridPoint),
			  &n_param_grid, n_points);
        {
            const double step = (pmax - pmin)/n_points;
            const double weight = 1.0/n_points;
            unsigned i;
            for (i=0; i<n_points; ++i)
            {
                param_grid[i].x = pmin + (i + 0.5)*step;
                param_grid[i].weight = weight;
            }
        }
    }
    *grid = param_grid;
}

void print_parameter_grid(const ParameterGridPoint *grid,
                          const unsigned n_points, FILE *stream)
{
    unsigned i;

    assert(grid);
    assert(n_points);
    assert(stream);
    
    fprintf(stream, "{");
    for (i=0; i<n_points; ++i)
        fprintf(stream, "%s{%g %g}", i ? " " : "", grid[i].x, grid[i].weight);
    fprintf(stream, "}");
}

#ifdef __cplusplus
}
#endif
