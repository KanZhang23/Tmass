#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "histo_utils.h"
#include "cernlib.h"

typedef struct
{
    char *name;
    int n;
} Numbered_name;

static int fcompare(const float *i, const float *j);
static float fmed_sorted(float *array, int n);
static int wcompare(const struct weighted_point *i,
		    const struct weighted_point *j);
static int numbered_name_comparator(const Numbered_name *i,
				    const Numbered_name *j);

double find_cdf_value(double x, double i0, double xmin, double xmax,
		      int nbins, struct simpson_bin *cdfdata)
{
    int binnum;
    double h, dbin, dxfrac;

    assert(nbins > 0);
    assert(xmax > xmin);
    if (x <= xmin)
	return i0;
    if (x >= xmax)
	return cdfdata[nbins-1].integ;
    h      = (xmax-xmin)/(double)nbins;
    dbin   = (x-xmin)/h;
    binnum = (int)dbin;
    if (binnum == nbins)
	return cdfdata[nbins-1].integ;
    if (binnum > 0)
	i0 = cdfdata[binnum-1].integ;
    dxfrac = dbin - (double)binnum;
    if (dxfrac <= 0.0)
        return i0;
    else if (dxfrac >= 1.0)
        return cdfdata[binnum].integ;
    else
        return dxfrac*(h*h*(6.0*cdfdata[binnum].d1*(dxfrac - 1.0) + 
            cdfdata[binnum].d2*(1.0 - dxfrac*(3.0 - 2.0*dxfrac))*h) +
            12.0*(cdfdata[binnum].integ - i0))/12.0 + i0;
}

int find_inverse_cdf_value_linear(double fvalue, double i0,
                                  double xmin, double xmax, int nbins,
                                  struct simpson_bin *cdfdata, double *x)
{
    int binnum;
    double h, dxfrac, df, integ;

    assert(nbins > 0);
    assert(xmax > xmin);
    if (fvalue <= i0)
    {
        *x = xmin;
        return (fvalue < i0);
    }
    if (fvalue >= cdfdata[nbins-1].integ)
    {
        *x = xmax;
        return (fvalue > cdfdata[nbins-1].integ);
    }

    /* Find the bin number */
    if (fvalue < cdfdata[0].integ)
        binnum = 0;
    else
    {
        int minbin, maxbin, newbin;
        minbin = 1;
        maxbin = nbins-1;
        while (minbin < maxbin-1)
        {
            newbin = (maxbin + minbin)/2;
            if (fvalue < cdfdata[newbin].integ)
                maxbin = newbin;
            else
                minbin = newbin;
        }
        if (fvalue < cdfdata[minbin].integ)
            binnum = minbin;
        else
            binnum = maxbin;
        i0 = cdfdata[binnum-1].integ;
    }

    /* Bin area */
    h = (xmax-xmin)/(double)nbins;
    integ = cdfdata[binnum].integ - i0;
    if (integ == 0.0)
    {
        *x = xmin + h*((double)binnum);
        return 0;
    }

    /* Find the x value within this bin */
    df = fvalue - i0;
    if (df <= 0.0)
        dxfrac = 0.0;
    else
    {
        if (cdfdata[binnum].d1 == 0.0)
        {
            dxfrac = df/integ;
        }
        else
        {
            /* Solve quadratic equation */
            double x1, x2;
            int status;

            status = hs_solve_quadratic_eq(
                2.0*integ/cdfdata[binnum].d1/h/h-1.0,
                -df/cdfdata[binnum].d1/h/h*2.0, &x1, &x2);
            assert(status == 0);
            if (0.0 <= x1 && x1 <= 1.0)
                dxfrac = x1;
            else if (0.0 <= x2 && x2 <= 1.0)
                dxfrac = x2;
            else
                assert(0);
        }
    }
    *x = xmin + h*((double)binnum + dxfrac);
    return 0;
}

int find_inverse_cdf_value(double fvalue, double i0, double xmin, double xmax,
                           int nbins, struct simpson_bin *cdfdata, double *x)
{
    int binnum;
    double h, dxfrac, df, integ;

    assert(nbins > 0);
    assert(xmax > xmin);
    if (fvalue <= i0)
    {
        *x = xmin;
        return (fvalue < i0);
    }
    if (fvalue >= cdfdata[nbins-1].integ)
    {
        *x = xmax;
        return (fvalue > cdfdata[nbins-1].integ);
    }

    /* Find the bin number */
    if (fvalue < cdfdata[0].integ)
        binnum = 0;
    else
    {
        int minbin, maxbin, newbin;
        minbin = 1;
        maxbin = nbins-1;
        while (minbin < maxbin-1)
        {
            newbin = (maxbin + minbin)/2;
            if (fvalue < cdfdata[newbin].integ)
                maxbin = newbin;
            else
                minbin = newbin;
        }
        if (fvalue < cdfdata[minbin].integ)
            binnum = minbin;
        else
            binnum = maxbin;
        i0 = cdfdata[binnum-1].integ;
    }

    /* Bin area */
    h = (xmax-xmin)/(double)nbins;
    integ = cdfdata[binnum].integ - i0;
    if (integ == 0.0)
    {
        *x = xmin + h*((double)binnum);
        return 0;
    }

    /* Find the x value within this bin */
    df = fvalue - i0;
    if (df <= 0.0)
        dxfrac = 0.0;
    else
    {
        if (cdfdata[binnum].d2 == 0.0)
        {
            if (cdfdata[binnum].d1 == 0.0)
            {
                dxfrac = df/integ;
            }
            else
            {
                /* Solve quadratic equation */
                double x1, x2;
                int status;
                status = hs_solve_quadratic_eq(
                    2.0*integ/cdfdata[binnum].d1/h/h-1.0,
                    -df/cdfdata[binnum].d1/h/h*2.0, &x1, &x2);
                assert(status == 0);
                if (0.0 <= x1 && x1 <= 1.0)
                    dxfrac = x1;
                else if (0.0 <= x2 && x2 <= 1.0)
                    dxfrac = x2;
                else
                    assert(0);
            }
        }
        else
        {
            /* Solve cubic equation */
            double X[3], R, S, T, D;
            R = -1.5 + 3.0*cdfdata[binnum].d1/cdfdata[binnum].d2/h;
            S = 0.5 - (3.0*cdfdata[binnum].d1-6.0*integ/h/h)/cdfdata[binnum].d2/h;
            T = -6.0*df/cdfdata[binnum].d2/h/h/h;
            drteq3_(&R, &S, &T, X, &D);
            if (D > 0.0)
                dxfrac = X[0];
            else
            {
                if (0.0 <= X[0] && X[0] <= 1.0)
                    dxfrac = X[0];
                else if (0.0 <= X[1] && X[1] <= 1.0)
                    dxfrac = X[1];
                else if (0.0 <= X[2] && X[2] <= 1.0)
                    dxfrac = X[2];
                else
                    assert(0);
            }
        }
    }
    *x = xmin + h*((double)binnum + dxfrac);
    return 0;
}

int closestint(double f, double epsilon, int *veryclose)
{
    int ilow, ihi;
    double lodiff, hidiff;
  
    if (f < 0.0)
    {
	ihi = (int)f;
	ilow = ihi - 1;
    }
    else if (f == 0.0)
    {
	if (veryclose)
	    *veryclose = 1;
	return 0;
    }
    else
    {
	ilow = (int)f;
	ihi = ilow + 1;
    }

    lodiff = f - (double)ilow;
    hidiff = (double)ihi - f;
  
    if (hidiff > lodiff)
    {
	if (veryclose)
	    *veryclose = (lodiff <= epsilon);
	return(ilow);
    } 
    else
    {
	if (veryclose)
	    *veryclose = (hidiff <= epsilon);
	return(ihi);
    }
}


float find_percentile(const float *coords, const float *cdf, int n, float q)
{
    int i, imin, imax;
    
    if (n <= 0)
	return 0.f;
    else if (n == 1)
	return coords[0];
    else
    {
	if (q <= cdf[0])
	{
	    for (i=1; cdf[i] == cdf[0] && i < n; i++);
	    return coords[i-1];
	}
	else if (q >= cdf[n-1])
	{
	    for (i=n-2; cdf[i] == cdf[n-1] && i >= 0; i--);
	    return coords[i+1];
	}
	else
	{
	    imin = 0;
	    imax = n-1;
	    while (imax-imin > 1)
	    {
		i = (imax+imin)/2;
		if (cdf[i] > q)
		    imax = i;
		else if (cdf[i] < q)
		    imin = i;
		else
		{
		    for (imax = i; cdf[imax+1] == cdf[i]; imax++);
		    for (imin = i; cdf[imin-1] == cdf[i]; imin--);
		    return (coords[imin] + coords[imax])/2.f;
		}    
	    }
	    return coords[imin] + (coords[imax]-coords[imin])*
		(q-cdf[imin])/(cdf[imax]-cdf[imin]);
	}
    }
}


int arr_stats_weighted(const struct weighted_point *array, int n,
		       float *fmean, float *ferr)
{
    /* This function assumes that all weights are non-negative */
    double fsum = 0.0, fsumsq = 0.0, wsum = 0.0, wsumsq = 0.0;
    double mean, eff_n;
    int i;

    if (n > 0)
    {
	for (i=0; i<n; ++i)
	{
	    register double weight = (double)(array[i].w);
	    fsum += weight*(double)(array[i].x);
	    wsum += weight;
	    wsumsq += weight*weight;
	    if (weight < 0.0)
	    {
		*fmean = 0.f;
		*ferr = 0.f;
		return -1;
	    }
	}
	if (wsum <= 0.0)
	{
	    *fmean = 0.f;
	    *ferr = 0.f;
	    return -1;
	}
	else
	{
	    mean = fsum / wsum;
	    *fmean = (float)mean;
	    if (n > 1)
	    {
		for (i=0; i<n; ++i)
		{
		    register double datum = (double)(array[i].x) - mean;
		    fsumsq += datum*datum*(double)(array[i].w);
		}
		eff_n = wsum*wsum/wsumsq;
		if (eff_n > 1.0)
		    *ferr = (float)sqrt(fsumsq/wsum/(eff_n-1.0)*eff_n);
		else
		    *ferr = 0.f;
	    }
	    else
	    {
		*ferr = 0.f;
	    }
	}
    }
    else
    {
	*fmean = 0.f;
	*ferr = 0.f;
	return -1;
    }
    return 0;
}


int arr_stats(const float *array, const int n, const int key,
              float *fmean, float *ferr)
{
    double datum = 0.0, fsum = 0.0, fsumsq = 0.0;
    int i, nno0 = 0;

    if (n > 0)
    {
	for (i=0; i<n; ++i)
	    if (array[i] || key)
	    {
		fsum += array[i];
		nno0++;
	    }
	if (nno0 == 0)
	{
	    *fmean = 0.f;
	    *ferr = 0.f;
	    return -1;
	}
	else if (nno0 == 1)
	{
	    *fmean = fsum;
	    *ferr = 0.f;
	}
	else
	{
	    *fmean = fsum/(double)nno0;
	    for (i=0; i<n; ++i)
		if (array[i] || key) 
		{
		    datum = array[i] - *fmean;
		    fsumsq += datum*datum;
		}
            {
                double err = fsumsq/(double)(nno0-1);
                if (err > 0.0)
                    err = sqrt(err);
                else
                    err = 0.0;
                *ferr = err;
            }
	}    
    }
    else
    {
	*fmean = 0.f;
	*ferr = 0.f;
	return -1;
    }
    return 0;
}


int arr_medirange_weighted(const struct weighted_point *array, int n,
	float *qmin, float *q25, float *qmed, float *q75, float *qmax)
{
    /* This function assumes that all weights are non-negative */
    struct weighted_point *ptmp;
    float *coords, *cdf;
    float wsum;
    int i;

    if (n > 0)
    {
	if (n == 1)
	{
	    *qmin = array[0].x;
	    *q25  = array[0].x;
	    *qmed = array[0].x;
	    *q75  = array[0].x;
	    *qmax = array[0].x;
	}
	else
	{
	    if ((ptmp = (struct weighted_point *)malloc(
		n*sizeof(struct weighted_point))) == NULL)
	    {
		fprintf(stderr, "ERROR in arr_medirange_weighted: not enough memory\n");
		return -1;
	    }
	    memcpy(ptmp, array, n*sizeof(struct weighted_point));
	    qsort(ptmp, n, sizeof(struct weighted_point),
		  (int (*)(const void *, const void *))wcompare);
	    if ((coords = (float *)malloc(n*sizeof(float))) == NULL)
	    {
		free(ptmp);
		fprintf(stderr, "ERROR in arr_medirange_weighted: not enough memory\n");
		return -1;
	    }
	    if ((cdf = (float *)malloc(n*sizeof(float))) == NULL)
	    {
		free(coords);
		free(ptmp);
		fprintf(stderr, "ERROR in arr_medirange_weighted: not enough memory\n");
		return -1;
	    }
	    wsum = 0.f;
	    for (i=0; i<n; ++i)
	    {
		coords[i] = ptmp[i].x;
		wsum += ptmp[i].w;
		cdf[i] = wsum;
	    }
	    if (wsum <= 0.f)
	    {
		*qmin = 0.f;
		*q25  = 0.f;
		*qmed = 0.f;
		*q75  = 0.f;
		*qmax = 0.f;
		free(cdf);
		free(coords);
		free(ptmp);
		return -1;
	    }
	    *qmin = coords[0];
	    *qmax = coords[n-1];
	    *q25 = find_percentile(coords, cdf, n, 0.25f*wsum);
	    *qmed = find_percentile(coords, cdf, n, 0.5f*wsum);
	    *q75 = find_percentile(coords, cdf, n, 0.75f*wsum);
	    free(cdf);
	    free(coords);
	    free(ptmp);
	}
    }
    else
    {
	*qmin = 0.f;
	*q25  = 0.f;
	*qmed = 0.f;
	*q75  = 0.f;
	*qmax = 0.f;
	return -1;
    }
    return 0;
}


int arr_medirange(const float *array, int n, int key,
	float *qmin, float *q25, float *qmed, float *q75, float *qmax)
{
    float *ptmp;
    int i, nhalf, nno0, i0above;

    if (n > 0)
    {
	if ((ptmp = (float *)malloc(n*sizeof(float))) == NULL)
	{
	    fprintf(stderr, "ERROR in arr_medirange: not enough memory\n");
	    return -1;
	}

	nno0 = 0;
	for(i=0; i<n; i++)
	    if (array[i] || key)
		ptmp[nno0++] = array[i];

	if (nno0 == 0)
	{
	    *qmin = 0.f;
	    *q25  = 0.f;
	    *qmed = 0.f;
	    *q75  = 0.f;
	    *qmax = 0.f;
	    free(ptmp);
	    return -1;
	}
	else if (nno0 == 1)
	{
	    *qmin = ptmp[0];
	    *q25  = ptmp[0];
	    *qmed = ptmp[0];
	    *q75  = ptmp[0];
	    *qmax = ptmp[0];
	}
	else
	{
	    qsort(ptmp, nno0, sizeof(float),
		  (int (*)(const void *, const void *))fcompare);
	    nhalf = nno0/2;
	    if (nno0%2)
		i0above = nhalf+1;
	    else
		i0above = nhalf;

	    *qmin = ptmp[0];
	    *q25  = fmed_sorted(ptmp, nhalf);
	    *qmed = fmed_sorted(ptmp, nno0);
	    *q75  = fmed_sorted(ptmp+i0above, nhalf);
	    *qmax = ptmp[nno0-1];
	}

	free(ptmp);
    }
    else
    {
	*qmin = 0.f;
	*q25  = 0.f;
	*qmed = 0.f;
	*q75  = 0.f;
	*qmax = 0.f;
	return -1;
    }
    return 0;
}


int arr_percentiles(float *array, int n, int key,
		    float *percentages, int len, float *answer)
{
    float *ptmp, find;
    int i, ibelow, iabove, nno0;

    if (n > 0)
    {
	if ((ptmp = (float *)malloc(n*sizeof(float))) == NULL)
	{
	    fprintf(stderr, "ERROR in arr_percentiles: not enough memory\n");
	    return -1;
	}

	nno0 = 0;
	for(i=0; i<n; i++)
	    if (array[i] || key)
		ptmp[nno0++] = array[i];

	if (nno0 == 0)
	{
	    memset(answer, 0, len*sizeof(float));
	    free(ptmp);
	    return -1;
	}
	else if (nno0 == 1)
	{
	    for (i=0; i<len; i++)
		answer[i] = ptmp[0];
	}
	else
	{
	    qsort(ptmp, nno0, sizeof(float),
		  (int (*)(const void *, const void *))fcompare);
	    for (i=0; i<len; i++)
	    {
		if (percentages[i] <= 0.f)
		    answer[i] = ptmp[0];
		else if (percentages[i] >= 100.f)
		    answer[i] = ptmp[nno0-1];
		else
		{
		    find = (percentages[i]/100.f)*(float)(nno0-1);
		    ibelow = (int)find;
		    iabove = ibelow+1;
		    if (iabove == nno0)
			answer[i] = ptmp[ibelow];
		    else
			answer[i] = ptmp[ibelow] +
			    (ptmp[iabove] - ptmp[ibelow]) *
			    (find - (float)ibelow);
		}
	    }
	}

	free(ptmp);
    }
    else
    {
	memset(answer, 0, len*sizeof(float));
	return -1;
    }
    return 0;
}


int arr_2d_weighted_covar(const float *data, const float *weights,
			  const size_t ncols, const size_t nrows,
			  const size_t *column_list, const size_t nused,
			  float *averages, float *covar)
{
    /* *data points to a 2d array of size ncols*nrows.
     * *weights must be at least as long as nrows. It also may be
     *     a null pointer which means that all weights are equal to 1.
     *     If it is not a null pointer, every weight must be non-negative.
     * *column_list must be at least as long as nused, 
     *     and each element must be a valid column number (< ncols).
     * *averages must be at least as long as nused.
     * *covar must be at least as long nused*nused.
     *
     * The function returns 0 on success and an error code on failure:
     *     1  -- one of the size arguments is 0.
     *     2  -- one of the pointers which can not be NULL is NULL.
     *     3  -- bad column number in the column_list.
     *     4  -- found a negative weight.
     *     5  -- all weights are 0.
     * Whenever an error code is returned, the contents of averages
     * and covar arrays are undefined.
     */
    size_t i, j, imap, row, npoints = 0;
    const float *prow;
    double wsum = 0.0, wsumsq = 0.0;

    if (ncols == 0 || nrows == 0 || nused == 0)
	return 1;
    if (data == NULL || column_list == NULL ||
	averages == NULL || covar == NULL)
	return 2;
    for (imap = 0; imap<nused; ++imap)
	if (column_list[imap] >= ncols)
	    return 3;

    /* Reset the accumulators */
    for (i=0; i<nused; ++i)
    {
	averages[i] = 0.0f;
	for (j=0; j<nused; ++j)
	    covar[i*nused+j] = 0.0f;
    }

    /* First pass: calculate averages */
    for (row = 0,prow = data; row < nrows; ++row,prow += ncols)
    {
	if (weights)
	{
	    register float w = weights[row];
	    if (w < 0.0f)
		return 4;
	    if (w > 0.0f)
	    {
		++npoints;
		wsum += w;
                wsumsq += w*w;
		for (i=0; i<nused; ++i)
		    averages[i] += w*prow[column_list[i]];
	    }
	}
	else
	{
	    ++npoints;
	    for (i=0; i<nused; ++i)
		averages[i] += prow[column_list[i]];
	}
    }
    if (npoints == 0)
	return 5;
    const float effCount = weights ? wsum*wsum / wsumsq : (float)npoints;
    if (!weights)
	wsum = (float)npoints;
    for (i=0; i<nused; ++i)
	averages[i] /= wsum;

    /* Second pass: calculate variances */
    if (npoints > 1)
    {
	float w = 1.f;
	for (row = 0,prow = data; row < nrows; ++row,prow += ncols)
	{
	    if (weights)
		w = weights[row];
	    for (i=0; i<nused; ++i)
		for (j=i; j<nused; ++j)
		    covar[i*nused+j] += w*(prow[column_list[i]]-averages[i])*
			(prow[column_list[j]]-averages[j]);
	}
	for (i=0; i<nused; ++i)
	    for (j=i; j<nused; ++j)
	    {
		covar[i*nused+j] /= effCount;
		covar[j*nused+i] = covar[i*nused+j];
	    }
    }
    return 0;
}


static float fmed_sorted(float *array, int n)
{
  float fmed;
 
  if (n%2)
    fmed = array[n/2];
  else
    fmed = (array[n/2-1]+array[n/2])/2.f;

  return(fmed);
}


static int fcompare(const float *i, const float *j)
{
  if (*i < *j)
    return -1;
  else if (*i > *j)
    return 1;
  else
    return 0;
}


static int wcompare(const struct weighted_point *i,
		    const struct weighted_point *j)
{
  if (i->x < j->x)
    return -1;
  else if (i->x > j->x)
    return 1;
  else
    return 0;
}


static int numbered_name_comparator(const Numbered_name *i, const Numbered_name *j)
{
    return strcmp(i->name, j->name);
}


int find_duplicate_name(char **names, int count)
{
    Numbered_name *localnames;
    int i, maxitems;

    if (names == NULL)
	return -1;
    if (count <= 1)
	return -1;
    localnames = (Numbered_name *)malloc(count*sizeof(Numbered_name));
    if (localnames == NULL)
    {
	fprintf(stderr, "Fatal error: out of memory. Aborting.\n");
	fflush(stderr);
	abort();
    }
    maxitems = 0;
    for (i=0; i<count; ++i)
    {
	if (names[i])
	{
	    localnames[maxitems].name = names[i];
	    localnames[maxitems++].n = i;
	}
    }
    if (maxitems <= 1)
    {
	free(localnames);
	return -1;
    }
    qsort(localnames, maxitems, sizeof(Numbered_name),
	  (int (*)(const void *, const void *))numbered_name_comparator);
    count = maxitems-1;
    for (i=0; i<count; ++i)
	if (strcmp(localnames[i].name, localnames[i+1].name) == 0)
	{
	    i = localnames[i].n;
	    free(localnames);
	    return i;
	}
    free(localnames);
    return -1;
}

int hs_solve_quadratic_eq(double b, double c, double *x1, double *x2)
{
    double d;
    d = b*b - 4.0*c;
    if (d < 0.0)
        return 1;
    else if (d == 0)
    {
        *x1 = -b/2.0;
        *x2 = *x1;
        return 0;
    }
    else if (b == 0.0)
    {
        *x1 = sqrt(-c);
        *x2 = -(*x1);
        return 0;
    }
    else
    {
        *x1 = -(b + (b < 0.0 ? -1.0 : 1.0)*sqrt(d))/2.0;
        *x2 = c/(*x1);
        return 0;
    }
}

int hs_find_value_index_in_sorted_array(float *arr, int n, float value)
{
    int imin = 0, imax, ind;
    imax = n-1;
    if (value <= arr[imin])
        return imin;
    if (value >= arr[imax])
        return imax;
    while (imax - imin > 1)
    {
        ind = (imax + imin)/2;
        if (arr[ind] < value)
            imin = ind;
        else if (arr[ind] > value)
            imax = ind;
        else
            return ind;
    }
    return imin;
}

void classic_unbinned_ks_probability(float *a1, int n1, float *a2, int n2,
				     float *ks_distance, float *ksprob)
{
    double d, maxdist = 0.0;
    float z;
    int i, j;

    if (n1 > 0)
    {
	assert(a1);
	qsort(a1, n1, sizeof(float),
	      (int (*)(const void *, const void *))fcompare);
    }
    if (n2 > 0)
    {
	assert(a2);
	qsort(a2, n2, sizeof(float),
	      (int (*)(const void *, const void *))fcompare);
    }
    for (i=0, j=0; i<n1 && j<n2; )
    {
	if (a1[i] < a2[j])
	    ++i;
	else
	    ++j;
	d = fabs((double)i/(double)n1 - (double)j/(double)n2);
	if (d > maxdist)
	    maxdist = d;
    }
    if (n1 > 0 && n2 > 0)
    {
	*ks_distance = (float)maxdist;
	z = (float)(maxdist * sqrt((double)n1*(double)n2/(double)(n1+n2)));
	*ksprob = probkl_(&z);
    }
    else
    {
	*ks_distance = 1.f;
	*ksprob = 0.f;
    }
}

void weighted_unbinned_ks_probability(
    struct weighted_point *a1, int n1, int neff1,
    struct weighted_point *a2, int n2, int neff2,
    float *ks_distance, float *ksprob)
{
    double d, maxdist = 0.0, wsum1 = 0.0, wsum2 = 0.0, w1 = 0.0, w2 = 0.0;
    float z;
    int i, j;

    if (n1 > 0)
    {
	assert(a1);
	qsort(a1, n1, sizeof(struct weighted_point),
	      (int (*)(const void *, const void *))wcompare);
        for (i=0; i<n1; ++i)
            wsum1 += (double)a1[i].w;
        assert(wsum1 >= 0.0);
        if (wsum1 == 0.0)
            n1 = 0;
    }
    if (n2 > 0)
    {
	assert(a2);
	qsort(a2, n2, sizeof(struct weighted_point),
	      (int (*)(const void *, const void *))wcompare);
        for (i=0; i<n2; ++i)
            wsum2 += (double)a2[i].w;
        assert(wsum2 >= 0.0);
        if (wsum2 == 0.0)
            n2 = 0;
    }
    for (i=0, j=0; i<n1 && j<n2; )
    {
	if (a1[i].x < a2[j].x)
            w1 += (double)a1[i++].w;
	else
            w2 += (double)a2[j++].w;
	d = fabs(w1/wsum1 - w2/wsum2);
	if (d > maxdist)
	    maxdist = d;
    }
    if (n1 > 0 && n2 > 0 && neff1 > 0 && neff2 > 0)
    {
	*ks_distance = (float)maxdist;
	z = (float)(maxdist * sqrt((double)neff1*(double)neff2/(double)(neff1+neff2)));
	*ksprob = probkl_(&z);
    }
    else
    {
	*ks_distance = 1.f;
	*ksprob = 0.f;
    }
}
