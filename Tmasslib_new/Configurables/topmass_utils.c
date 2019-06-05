#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "topmass_utils.h"
#include "pi.h"

#define TWOPIL 6.28318530717958647692528676656L

#ifdef __cplusplus
extern "C" {
#endif

/* An XOR function to help Sobol FORTRAN code */
int exor_(int *a, int *b);
int exor_(int *a, int *b)
{
    return *a ^ *b;
}

void covar_to_corr(double *covmat, const unsigned n_rows, double *stdevs)
{
    unsigned i, j;
    assert(covmat);
    assert(n_rows);
    assert(stdevs);
    for (i=0; i<n_rows; ++i)
    {
	assert(covmat[i*n_rows + i] >= 0.0);
	stdevs[i] = sqrt(covmat[i*n_rows + i]);
    }
    for (i=0; i<n_rows; ++i)
	for (j=0; j<n_rows; ++j)
	    if (covmat[i*n_rows + j])
	    {
		if (i == j)
		    covmat[i*n_rows + j] = 1.0;
		else
		{
		    double prod = stdevs[i]*stdevs[j];
		    assert(prod);
		    covmat[i*n_rows + j] /= prod;
		}
	    }
}

void get_static_memory(void **ptr, size_t structsize,
		       unsigned *nmemb, unsigned newmemb)
{
    if (newmemb > *nmemb)
    {
	if (*ptr)
	    free(*ptr);
	*ptr = malloc(newmemb * structsize);
	if (*ptr == 0)
	{
	    fprintf(stderr, "Fatal error: out of memory. Exiting.\n");
	    exit(EXIT_FAILURE);
	}
	*nmemb = newmemb;
    }
}

void read_n_bytes(int fileno, void *buf, size_t toread)
{
    ssize_t readcount;
    size_t nread;

    assert(buf);
    assert(toread);
    assert(fileno != -1);
    
    for (nread=0; nread<toread; nread+=readcount)
    {
        readcount = read(fileno, (char *)buf+nread, toread-nread);
        if (readcount == -1)
        {
            if (errno == EINTR)
            {
                readcount = 0;
                continue;
            }
            else
            {
                fprintf(stderr, "Fatal error in read_n_bytes:"
                        " %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
	else if (readcount == 0)
        {
            fprintf(stderr, "Fatal error in read_n_bytes:"
                    " premature end of file. Exiting.\n");
            exit(EXIT_FAILURE);
        }
    }
}

double cauchy_density(const double x, const double peak,
                      const double hwhm)
{
    const double diff = (x-peak)/hwhm;
    return 1.0/(1.0 + diff*diff)/PI/hwhm;
}


double relativistic_bw_density(const double m, const double polemass,
                               const double width)
{
    return cauchy_density(m*m, polemass*polemass, polemass*width);
}

double gauss_random(const double mean, const double sigma)
{
    return normal_random()*sigma + mean;
}

double cauchy_random(const double peak, const double hwhm)
{
    return hwhm*tan(PI*(uniform_random() - 0.5)) + peak;
}

double pick_closest_element(const double *arr, unsigned len, double value,
                            double (*distance)(double, double))
{
    assert(len);
    if (len == 1)
        return arr[0];
    else
    {
        unsigned i;
        double closest = arr[0];
        double mindistance = distance(closest, value);
        for (i=1; i<len; ++i)
        {
            const double d = distance(arr[i], value);
            if (d < mindistance)
            {
                closest = arr[i];
                mindistance = d;
            }
        }
        return closest;
    }
}

unsigned uniform_random_setseed(unsigned seed)
{
    long int seedval;
    if (!seed)
    {   
        size_t nread;
        FILE* f = fopen("/dev/urandom", "r");
        assert(f);
        nread = fread(&seed, sizeof(seed), 1, f);
        assert(nread == 1);
        fclose(f);
    }
    seedval = seed;
    srand48(seedval);
    return seed;
}

double uniform_random(void)
{
    return drand48();
}

double normal_random(void)
{
    const double r1 = uniform_random();
    const double r2 = uniform_random();
    return sqrtl(-2.0L*logl(r1))*sinl(TWOPIL*(r2-0.5L));
}

double ouni_(void)
{
    return uniform_random();
}

#ifdef __cplusplus
}
#endif
