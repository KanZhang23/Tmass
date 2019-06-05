#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "n_d_random.h"
#include "halton.h"
#include "sobol.h"
#include "sobol_f.h"
#include "scrambled_sobol.h"
#include "topmass_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

static N_d_random_method current_method = N_D_RANDOM_INVALID;
static unsigned current_ndim = 0;
static int sobol_seed = 0;
static int sobol_f_firstcall = 1;
static double sobol_f_quazi[MAX_SOBOL_F_DIM];

N_d_random_method parse_n_d_random_method(char *cmethod)
{
    if (strcasecmp(cmethod, "drand48") == 0)
        return N_D_RANDOM_DRAND48;
    else if (strcasecmp(cmethod, "halton") == 0)
        return N_D_RANDOM_HALTON;
    else if (strcasecmp(cmethod, "sobol") == 0)
        return N_D_RANDOM_SOBOL;
    else if (strcasecmp(cmethod, "scramsobol") == 0)
        return N_D_RANDOM_SCRAM_SOBOL;
    else if (strcasecmp(cmethod, "fortsobol") == 0)
        return N_D_RANDOM_FORT_SOBOL;
    else if (strcasecmp(cmethod, "nieder") == 0)
        return N_D_RANDOM_NIEDER;
    else
        return N_D_RANDOM_INVALID;
}

unsigned current_n_d_random_ndim(void)
{
    return current_ndim;
}

N_d_random_method current_n_d_random_method(void)
{
    return current_method;
}

void init_n_d_random(const N_d_random_method rand_method,
                     const unsigned ndim, const int param)
{
    assert(ndim <= MAX_SOBOL_DIM);

    current_method = rand_method;
    current_ndim = ndim;
    sobol_seed = 0;

    if (ndim == 0)
        return;
    else if (ndim == 1)
    {
        if (rand_method == N_D_RANDOM_SOBOL ||
            rand_method == N_D_RANDOM_SCRAM_SOBOL)
        {
            printf("WARNING in init_n_d_random: Sobol sequences are not "
                   "implemented in 1d, switching to Halton\n");
            fflush(stdout);
            current_method = N_D_RANDOM_HALTON;
        }
    }

    switch (current_method)
    {
    case N_D_RANDOM_DRAND48:
        /* param argument is used as a seed if it is not 0 */
        if (param)
        {
            uniform_random_setseed(*((unsigned *)(&param)));
        }
        break;

    case N_D_RANDOM_HALTON:
        halton_dim_num_set(ndim);
        /* Skip "param" initial points */
        if (param > 0)
        {
            int i;
            double r[MAX_SOBOL_DIM];

            for (i=0; i<param; ++i)
                halton(r);
        }        
        break;

    case N_D_RANDOM_NIEDER:
        {
            int DIMEN = ndim;
            int IFLAG = 0;
            int SKIP = 0;

            if (param > 0 && param < 4)
                IFLAG = param;
            else if (param >= 4)
                SKIP = param;

            sinlo2_(&DIMEN, &SKIP, &IFLAG);
        }
        break;

    case N_D_RANDOM_SOBOL:
        /* Skip "param" initial points */
        if (param > 0)
        {
            int i;
            float r[MAX_SOBOL_DIM];

            for (i=0; i<param; ++i)
                i4_sobol((int)ndim, &sobol_seed, r);
        }
        break;

    case N_D_RANDOM_FORT_SOBOL:
        {
            int FLAG[2];
            int DIMEN = ndim;
            int ATMOST = (int)pow(2,30) - 1;
            int TAUS = -1;
            int MAXS = 30;
            int IFLAG = 0;

            if (param > 0 && param < 4)
                IFLAG = param;

            insobl_(FLAG, &DIMEN, &ATMOST, &TAUS,
                    sobol_f_quazi, &MAXS, &IFLAG);
            assert(FLAG[0] && FLAG[1]);
            sobol_f_firstcall = 1;

            /* Skip "param" initial points */
            if (param >= 4)
            {
                int i;
                for (i=1; i<param; ++i)
                    gosobl_(sobol_f_quazi);
                sobol_f_firstcall = 0;
            }
        }
        break;

    case N_D_RANDOM_SCRAM_SOBOL:
        /* Skip "param" initial points */
        if (param > 0)
        {
            int i;
            float r[MAX_SOBOL_DIM];

            for (i=0; i<param; ++i)
                i4_scrambled_sobol((int)ndim, &sobol_seed, r);
        }
        break;

    default:
        assert(0);
    }
}

void next_n_d_random(float *r)
{
    assert(current_method != N_D_RANDOM_INVALID);
    if (current_ndim == 0)
        return;

    switch (current_method)
    {
    case N_D_RANDOM_DRAND48:
        {
            unsigned i = 0;
            for (; i<current_ndim; ++i)
                r[i] = uniform_random();
        }
        break;

    case N_D_RANDOM_NIEDER:
        {
            unsigned i;
            double dr[MAX_SOBOL_F_DIM];
            sgolo2_(dr);
            for (i=0; i<current_ndim; ++i)
                r[i] = dr[i];
        }
        break;

    case N_D_RANDOM_HALTON:
        {
            unsigned i;
            double dr[MAX_SOBOL_DIM];
            halton(dr);
            for (i=0; i<current_ndim; ++i)
                r[i] = dr[i];
        }
        break;
    
    case N_D_RANDOM_FORT_SOBOL:
        {
            unsigned i;
            if (sobol_f_firstcall)
                sobol_f_firstcall = 0;
            else
                gosobl_(sobol_f_quazi);
            for (i=0; i<current_ndim; ++i)
                r[i] = sobol_f_quazi[i];
        }
        break;

    case N_D_RANDOM_SOBOL:
        i4_sobol((int)current_ndim, &sobol_seed, r);
        break;

    case N_D_RANDOM_SCRAM_SOBOL:
        i4_scrambled_sobol((int)current_ndim, &sobol_seed, r);
        break;

    default:
        assert(0);
    }
}

void cleanup_n_d_random(void)
{
    current_method = N_D_RANDOM_INVALID;
    current_ndim = 0;
    sobol_seed = 0;
    sobol_f_firstcall = 1;
}

#ifdef __cplusplus
}
#endif
