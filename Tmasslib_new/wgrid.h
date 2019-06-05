#ifndef WGRID_H_
#define WGRID_H_

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WGRID_RECTANGULAR = 0,
    WGRID_TRAPEZOID,
    WGRID_SIMPSON,
    N_WGRID_TYPES
} Wgrid_type;

typedef struct
{
    double mW;         /* Center value of the W mass Breit-Wigner   */
    double widthW;     /* Width of the W mass Breit-Wigner          */
    double coverage;   /* Fraction of the Breit-Wigner which
                          should be covered by the grid. Should
                          be a bit smaller than 1 (exactly 1
                          would include the tail which extends
                          to the negative mass values) */
    double mWmin;      /* Range covered. Corresponds to "coverage". */
    double mWmax;
    char *filename;    /* Set to the name of file used to initialize
			  the grid (0 if not from file)             */
    double *points;    /* An array of W mass values in the grid.
                          They will be equidistant in the cumulative
                          Breit-Wigner probability. */
    unsigned nbins;    /* Number of intervals in the grid           */
    Wgrid_type wtype;  /* Type of the integration for which this
                          grid is intended                          */
} wgrid;

/* One of the functions below should be called to initialize the grid.
 * The "read_w_grid" function returns 0 on success, something else
 * on failure.
 */
void init_w_grid(wgrid *g, size_t n_intervals, double mW,
                 double widthW, double coverage, Wgrid_type wtype);
void init_w_grid_byrange(wgrid *g, size_t n_intervals, double mW,
			 double widthW, double mWmin, double mWmax,
			 Wgrid_type wtype);
int read_w_grid(wgrid *g, char *filename);

/* This function should be called to release the memory 
 * when you are done using the W grid.
 */
void cleanup_w_grid(wgrid *g);

/* The following function returns 0 if the grids are identical,
 * 1 if they are not.
 */
int compare_w_grids(const wgrid *, const wgrid *);

/* Reposition the grid edges */
void set_w_grid_edges(wgrid *g, double mWmin, double mWmax);

/* The following function returns the number of points in the grid.
 * Note that it is usually larger than the number of intervals.
 */
size_t w_grid_npoints(const wgrid *g);

/* The following function returns the bin range */
void w_grid_bin_edges(const wgrid *g, size_t binnum, double *min, double *max);

/* Print the grid (useful for debugging) */
void print_w_grid(const wgrid *g, FILE *stream);

/* Find grid bin number for the given mass */
int w_grid_bin_num(const wgrid *g, double wmass);

/* Breit-Wigner density for the given mass */
double w_grid_bw_value(const wgrid *g, double wmass);

#ifdef __cplusplus
}
#endif

#endif /* WGRID_H_ */
