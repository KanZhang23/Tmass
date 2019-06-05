#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "convolute_breit_wigner.h"
#include "fftw3.h"
#include "pi.h"

#ifdef __cplusplus
extern "C" {
#endif

/*  #define fftw_complex fftwf_complex */
/*  #define fftw_plan fftwf_plan */
/*  #define fftw_malloc fftwf_malloc */
/*  #define fftw_free fftwf_free */
/*  #define fftw_plan_dft_r2c_1d fftwf_plan_dft_r2c_1d */
/*  #define fftw_plan_dft_c2r_1d fftwf_plan_dft_c2r_1d */
/*  #define fftw_destroy_plan fftwf_destroy_plan */
/*  #define fftw_execute fftwf_execute */

typedef double REAL;

void convolute_breit_wigner(double *data,
                            const size_t npoints,
                            const double hwhm)
{
    static REAL *in_f = NULL;
    static fftw_complex *out_f = NULL;
    static fftw_plan p_f;

    static fftw_complex *in_b = NULL;
    static REAL *out_b = NULL;
    static fftw_plan p_b;

    static fftw_complex *stored_bw = NULL;
    static size_t oldpoints = 0;
    static double oldhwhm = -1.0;

    const size_t size = npoints*2;
    const REAL normfactor = 1.0/(REAL)size;
    size_t i;

    assert(data);
    assert(npoints > 0);
    assert(hwhm > 0.0);

    if (npoints != oldpoints)
    {
        if (oldpoints)
        {
            fftw_destroy_plan(p_f);
            fftw_destroy_plan(p_b);
            fftw_free(in_f);
            fftw_free(out_f);
            fftw_free(in_b);
            fftw_free(out_b);
            fftw_free(stored_bw);
        }

        in_f  = (REAL *)fftw_malloc(sizeof(REAL) * size);
        out_f = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * (npoints+1));
        in_b  = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * (npoints+1));
        out_b = (REAL *)fftw_malloc(sizeof(REAL) * size);
        stored_bw = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * (npoints+1));
        if (in_f == NULL || out_f == NULL ||
            in_b == NULL || out_b == NULL ||
            stored_bw == NULL)
        {
            fprintf(stderr, "Fatal error in convolute_breit_wigner: "
                    "out of memory. Aborting.\n");
            fflush(stderr);
            abort();
        }
        p_f = fftw_plan_dft_r2c_1d(size, in_f, out_f, FFTW_MEASURE);
        p_b = fftw_plan_dft_c2r_1d(size, in_b, out_b, FFTW_MEASURE);
    }

    if (npoints != oldpoints || oldhwhm != hwhm)
    {
        oldpoints = npoints;
        oldhwhm = hwhm;

        /* Transform the Breit-Wigner into the forward direction */
        for (i=0; i<npoints; ++i)
            in_f[i] = hwhm/PI/(i*i + hwhm*hwhm);
        for (i=npoints; i<size; ++i)
            in_f[i] = hwhm/PI/((size-i)*(size-i) + hwhm*hwhm);
        fftw_execute(p_f);
        memcpy(stored_bw, out_f, sizeof(fftw_complex) * (npoints+1));
        memset(in_f, 0, sizeof(REAL) * size);
    }

    /* Transform the input data into the forward direction */
    for (i=0; i<npoints; ++i)
        in_f[i] = data[i];
    fftw_execute(p_f);

    /* Multiply the stored transformed Breit-Wigner by the transformed data */
    for (i=0; i<npoints+1; ++i)
    {
        in_b[i][0] = out_f[i][0]*stored_bw[i][0] - out_f[i][1]*stored_bw[i][1];
        in_b[i][1] = out_f[i][0]*stored_bw[i][1] + out_f[i][1]*stored_bw[i][0];
    }

    /* Transform the data back */
    fftw_execute(p_b);
    for (i=0; i<npoints; ++i)
        data[i] = normfactor*out_b[i];
}

#ifdef __cplusplus
}
#endif
