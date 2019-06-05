#include <math.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "wgrid.h"
#include "range_lookup.h"
#include "topmass_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

static const char *typenames[N_WGRID_TYPES] = {
    "rectangular", "trapezoidal", "simpson"
};

static int parse_grid_type(const char *chartype, Wgrid_type *wtype)
{
    int i;
    assert(chartype);
    assert(wtype);
    for (i=0; i<N_WGRID_TYPES; ++i)
	if (strcasecmp(chartype, typenames[i]) == 0)
	{
	    *wtype = (Wgrid_type)i;
	    return 0;
	}
    *wtype = N_WGRID_TYPES;
    return -1;
}

static int parse_double(const char *input, double *value)
{
    char *endptr;
    double d;

    assert(input);
    assert(value);
    d = strtod(input, &endptr);
    if (*input == '\0' || *endptr != '\0')
	return -1;
    *value = d;
    return 0;
}

static int parse_size(const char *input, unsigned *value)
{
    char *endptr;
    long tmplong;

    assert(input);
    assert(value);
    tmplong = strtol(input, &endptr, 10);
    if (*input == '\0' || *endptr != '\0' || tmplong < 0)
	return -1;
    *value = (size_t)tmplong;
    return 0;
}

static size_t w_grid_npoints_internal(size_t n_intervals, Wgrid_type wtype)
{
    switch (wtype)
    {
    case WGRID_RECTANGULAR:
        return n_intervals;
    case WGRID_TRAPEZOID:
        return n_intervals+1;
    case WGRID_SIMPSON:
        return 2*n_intervals+1;
    default:
        assert(0);
    }
}

size_t w_grid_npoints(const wgrid *g)
{
    assert(g);
    return w_grid_npoints_internal(g->nbins, g->wtype);
}

void w_grid_bin_edges(const wgrid *g, size_t binnum, double *min, double *max)
{
    assert(g);
    assert(binnum < g->nbins);

    switch (g->wtype)
    {
    case WGRID_RECTANGULAR:
        {
            const double Pi       = atan2(0.0, -1.0);
            const double peak     = g->mW*g->mW;
            const double hwhm     = g->mW*g->widthW;
            const double probmin  = (1.0 - g->coverage)/2.0;
            const double probStep = g->coverage/g->nbins;
            if (binnum == 0)
                *min = g->mWmin;
            else
            {
                double mWsq = hwhm*tan(Pi*(probmin + 
                        probStep*binnum - 0.5)) + peak;
                *min = sqrt(mWsq);
            }
            if (binnum == g->nbins-1)
                *max = g->mWmax;
            else
            {
                double mWsq = hwhm*tan(Pi*(probmin + 
                        probStep*(binnum+1) - 0.5)) + peak;
                *max = sqrt(mWsq);
            }
        }
        break;

    case WGRID_TRAPEZOID:
        *min = g->points[binnum];
        *max = g->points[binnum+1];
        break;

    case WGRID_SIMPSON:
        *min = g->points[2*binnum];
        *max = g->points[2*binnum+2];
        break;

    default:
        assert(0);
    }
}

#define fill_grid_points do {\
    size_t i, nb;\
    double prob0, probStep;\
    g->points = (double *)malloc(npoints * sizeof(double));\
    assert(g->points);\
    switch (wtype)\
    {\
    case WGRID_RECTANGULAR:\
        probStep = coverage/n_intervals;\
        prob0 = probmin + probStep/2.0;\
        nb = n_intervals;\
        break;\
    case WGRID_TRAPEZOID:\
        probStep = coverage/n_intervals;\
        prob0 = probmin;\
        nb = n_intervals;\
        g->points[nb] = g->mWmax;\
        break;\
    case WGRID_SIMPSON:\
        probStep = coverage/n_intervals/2.0;\
        prob0 = probmin;\
        nb = 2*n_intervals;\
        g->points[nb] = g->mWmax;\
        break;\
    default:\
        assert(0);\
    }\
    for (i=0; i<nb; ++i)\
    {\
        double mWsq = hwhm*tan(Pi*(prob0 + probStep*i - 0.5)) + peak;\
        assert(mWsq > 0.0);\
        g->points[i] = sqrt(mWsq);\
    }\
} while(0);

void init_w_grid_byrange(wgrid *g, const size_t n_intervals, const double mW,
			 const double widthW, const double mWmin,
			 const double mWmax, const Wgrid_type wtype)
{
    const double Pi      = atan2(0.0, -1.0);
    const double peak    = mW*mW;
    const double hwhm    = mW*widthW;
    const double probmin = atan((mWmin*mWmin-peak)/hwhm)/Pi + 0.5;
    const double probmax = atan((mWmax*mWmax-peak)/hwhm)/Pi + 0.5;
    const double coverage = probmax - probmin;
    const size_t npoints = w_grid_npoints_internal(n_intervals, wtype);

    assert(g);
    assert(n_intervals > 0);
    assert(mW > 0.0);
    assert(widthW > 0.0);
    assert(coverage > 0.0);
    assert(coverage < 1.0);
    assert(mWmin > 0.0);
    assert(mWmax > mWmin);

    g->mW = mW;
    g->widthW = widthW;
    g->coverage = coverage;
    g->nbins = n_intervals;
    g->wtype = wtype;
    g->filename = 0;
    g->mWmin = mWmin;
    g->mWmax = mWmax;

    fill_grid_points;
}

void init_w_grid(wgrid *g, const size_t n_intervals, const double mW,
                 const double widthW, const double coverage,
		 const Wgrid_type wtype)
{
    const double Pi      = atan2(0.0, -1.0);
    const double peak    = mW*mW;
    const double hwhm    = mW*widthW;
    const double probmin = (1.0 - coverage)/2.0;
    const double probmax = (1.0 + coverage)/2.0;
    const size_t npoints = w_grid_npoints_internal(n_intervals, wtype);

    assert(g);
    assert(n_intervals > 0);
    assert(mW > 0.0);
    assert(widthW > 0.0);
    assert(coverage > 0.0);
    assert(coverage < 1.0);

    g->mW = mW;
    g->widthW = widthW;
    g->coverage = coverage;
    g->nbins = n_intervals;
    g->wtype = wtype;
    g->filename = 0;

    {
	double dtmp = hwhm*tan(Pi*(probmin - 0.5)) + peak;
	assert(dtmp >= 0.0);
	g->mWmin = sqrt(dtmp);
	dtmp = hwhm*tan(Pi*(probmax - 0.5)) + peak;
	g->mWmax = sqrt(dtmp);
    }

    fill_grid_points;
}

void set_w_grid_edges(wgrid *g, const double mWmin, const double mWmax)
{
    assert(g);
    if (g->mWmin != mWmin || g->mWmax != mWmax)
    {
        const double Pi   = atan2(0.0, -1.0);
        const double peak = g->mW*g->mW;
        const double hwhm = g->mW*g->widthW;
        double probmin, probmax, mWsq, prob0, probStep;
        size_t i, nb;

	/* Can't redefine edges of an arbitrary grid, Breit-Wigner only */
	assert(g->filename == 0);
        assert(mWmin > 0.0);
        assert(mWmax > mWmin);

        g->mWmin = mWmin;
        g->mWmax = mWmax;
        probmin  = atan((mWmin*mWmin - peak)/hwhm)/Pi + 0.5;
        probmax  = atan((mWmax*mWmax - peak)/hwhm)/Pi + 0.5;
        g->coverage = probmax - probmin;

        switch (g->wtype)
        {
        case WGRID_RECTANGULAR:
            probStep = g->coverage/g->nbins;
            prob0 = probmin + probStep/2.0;
            nb = g->nbins;
            break;

        case WGRID_TRAPEZOID:
            probStep = g->coverage/g->nbins;
            prob0 = probmin;
            nb = g->nbins;
            g->points[nb] = g->mWmax;
            break;

        case WGRID_SIMPSON:
            probStep = g->coverage/g->nbins/2.0;
            prob0 = probmin;
            nb = 2*g->nbins;
            g->points[nb] = g->mWmax;
            break;

        default:
            assert(0);
        }

        for (i=0; i<nb; ++i)
        {
            mWsq = hwhm*tan(Pi*(prob0 + probStep*i - 0.5)) + peak;
            assert(mWsq > 0.0);
            g->points[i] = sqrt(mWsq);
        }
    }
}

void cleanup_w_grid(wgrid *g)
{
    if (g)
    {
        g->nbins = 0;
        g->coverage = 0.0;
        g->mWmin = g->mW;
        g->mWmax = g->mW;
        if (g->points)
        {
            free(g->points);
            g->points = 0;
        }
	if (g->filename)
	{
	    free(g->filename);
	    g->filename = 0;
	}
    }
}

#define BUFSIZE 80

int read_w_grid(wgrid *g, char *filename)
{
    FILE *stream;
    char buf[BUFSIZE];
    char *c;
    size_t i, linenum = 0, ntoread = 0, nread = 0, state = 0;

    if (g == NULL || filename == NULL)
    {
	fprintf(stderr, "Error in read_w_grid: bad argument\n");
	return 1;
    }
    if ((stream = fopen(filename, "r")) == NULL)
    {
	fprintf(stderr, "Error in read_w_grid: failed to open file %s\n",
		filename);
	return 2;
    }
    g->points = 0;
    g->nbins = 0;
    g->filename = strdup(filename);
    assert(g->filename);

    while (fgets(buf, BUFSIZE, stream))
    {
	++linenum;
	buf[BUFSIZE - 1] = '\0';
	/* Skip white space at the beginning of the line */
	i = strspn(buf, " \f\n\r\t\v");
	/* Skip empty lines and comments */
	if (buf[i] == '\0' || buf[i] == '#')
	    continue;
	/* Remove white space at the end */
	c = buf + i + strlen(buf+i) - 1;
	while (isspace(*c))
	    *c-- = '\0';
	switch (state)
	{
	case 0:
	    if (parse_grid_type(buf+i, &g->wtype))
		goto read_error;
	    break;
	case 1:
	    if (parse_double(buf+i, &g->mW))
		goto read_error;
	    if (g->mW <= 0.0)
		goto read_error;
	    break;
	case 2:
	    if (parse_double(buf+i, &g->widthW))
		goto read_error;
	    if (g->widthW <= 0.0)
		goto read_error;
	    break;
	case 3:
	    if (parse_double(buf+i, &g->coverage))
		goto read_error;
	    if (g->coverage < 0.0 || g->coverage > 1.0)
		goto read_error;
	    break;
	case 4:
	    if (parse_double(buf+i, &g->mWmin))
		goto read_error;
	    break;
	case 5:
	    if (parse_double(buf+i, &g->mWmax))
		goto read_error;
	    if (g->mWmax <= g->mWmin)
		goto read_error;
	    break;
	case 6:
	    if (parse_size(buf+i, &g->nbins))
		goto read_error;
	    if (g->nbins == 0)
		goto read_error;
	    ntoread = w_grid_npoints_internal(g->nbins, g->wtype);
	    g->points = (double *)malloc(ntoread*sizeof(double));
	    assert(g->points);
	    break;
	case 7:
	    if (parse_double(buf+i, g->points + nread))
		goto read_error;
	    /* Check that grid coordinates are within limits */
	    if (g->points[nread] < g->mWmin || g->points[nread] > g->mWmax)
		goto read_error;
	    /* Check that grid coordinates are increasing */
	    if (nread > 0)
		if (g->points[nread] <= g->points[nread-1])
		    goto read_error;
	    if (++nread == ntoread)
		state = 1000;
	    break;
	default:
	    assert(0);
	}
	if (state == 1000)
	    break;
	if (state < 7)
	    ++state;
    }
    fclose(stream);
    if (state == 1000)
	return 0;
    else
    {
	fprintf(stderr, "Error in read_w_grid: incomplete data "
		"in file %s\n", filename);
	cleanup_w_grid(g);
	return 4;
    }

 read_error:
    fprintf(stderr, "Error in read_w_grid: bad input on "
	    "line %u in file %s\n", (unsigned)linenum, filename);
    fclose(stream);
    cleanup_w_grid(g);
    return 3;
}

void print_w_grid(const wgrid *g, FILE *stream)
{
    assert(g);
    if (stream)
    {
        int i, npoints;
        fprintf(stream, "mW = %g, GW = %g, mW min = %g, mW max = %g\n",
                g->mW, g->widthW, g->mWmin, g->mWmax);
        fprintf(stream, "coverage = %g, type %s, n = %u\n",
                g->coverage, typenames[g->wtype], g->nbins);
	if (g->filename)
	    fprintf(stream, "file: %s\n", g->filename);
        fprintf(stream, "points:");
        npoints = w_grid_npoints_internal(g->nbins, g->wtype);
        for (i=0; i<npoints; ++i)
            fprintf(stream, " %g", g->points[i]);
    }
}

int w_grid_bin_num(const wgrid *g, const double mwmin_mW)
{
    assert(g);
    if (mwmin_mW >= g->mWmax)
	return g->nbins;
    else if (mwmin_mW >= g->mWmin)
    {
	switch (g->wtype)
	{
	case WGRID_RECTANGULAR:
	{
	    int minbin = -1;
	    unsigned ibin;
	    double mwleft, mwright;
	    for (ibin=0; ibin<g->nbins; ++ibin)
	    {
		w_grid_bin_edges(g, ibin, &mwleft, &mwright);
		if (mwmin_mW >= mwleft && mwmin_mW < mwright)
		{
		    minbin = ibin;
		    break;
		}
	    }
	    assert(minbin >= 0);
	    return minbin;
	}
	case WGRID_TRAPEZOID:
	{
	    unsigned i2 = range_lookup(g->points, 2*g->nbins+1, mwmin_mW);
	    assert(i2);
	    return i2-1;
	}
	case WGRID_SIMPSON:
	{
	    unsigned i2 = range_lookup(g->points, 2*g->nbins+1, mwmin_mW);
	    assert(i2);
	    return (i2-1)/2;
	}
	default:
	    assert(0);
	}
    }
    else
	return -1;
}

double w_grid_bw_value(const wgrid *g, const double wmass)
{
    if (wmass < g->mWmin || wmass > g->mWmax)
        return 0.0;
    else
        return cauchy_density(wmass*wmass, g->mW*g->mW, g->mW*g->widthW);
}

int compare_w_grids(const wgrid *g1, const wgrid *g2)
{
    assert(g1);
    assert(g1->points);
    assert(g2);
    assert(g2->points);

    if (g1->mW != g2->mW)
	return 1;
    if (g1->widthW != g2->widthW)
	return 1;
    if (g1->coverage != g2->coverage)
	return 1;
    if (g1->mWmin != g2->mWmin)
	return 1;
    if (g1->mWmax != g2->mWmax)
	return 1;
    if (g1->nbins != g2->nbins)
	return 1;
    if (g1->wtype != g2->wtype)
	return 1;
    {
	unsigned i;
	const unsigned imax = w_grid_npoints(g1);
	for (i=0; i<imax; ++i)
	    if (g1->points[i] != g2->points[i])
		return 1;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif
