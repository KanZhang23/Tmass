#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>

#include "n_d_mask.h"

#define MAX_ULONGLONG (~0ULL)
#define CHECK_BOUNDS

#ifdef __cplusplus
extern "C" {
#endif

N_d_mask* n_d_mask_create(const unsigned *dim, const unsigned ndim)
{
    N_d_mask *mask = 0;
    unsigned *buf;
    unsigned i, idim, nwords;
    unsigned long long ull_nwords, stride = 1;

    assert(dim);
    assert(ndim);

    mask = (N_d_mask *)calloc(1, sizeof(N_d_mask));
    if (mask == 0)
        goto out_of_mem;
    mask->strides = (unsigned long long *)malloc(ndim*sizeof(unsigned long long));
    if (mask->strides == 0)
        goto out_of_mem;
    mask->dim = (unsigned *)malloc(ndim*sizeof(unsigned));
    if (mask->dim == 0)
        goto out_of_mem;
    for (idim=0; idim<ndim; ++idim)
    {
        assert(dim[idim]);
        mask->dim[idim] = dim[idim];
        mask->strides[idim] = stride;
        assert(stride < MAX_ULONGLONG/dim[idim]);
        stride *= dim[idim];
    }
    ull_nwords = stride/(8*sizeof(unsigned));
    if (stride % (8*sizeof(unsigned)))
        ++ull_nwords;
    assert(ull_nwords*sizeof(unsigned) < UINT_MAX);
    nwords = (unsigned)ull_nwords;
    buf = (unsigned *)malloc(nwords*sizeof(unsigned));
    if (buf == 0)
        goto out_of_mem;
    for (i=0; i<nwords; ++i)
        buf[i] = 0;
    mask->data = buf;
    mask->ndim = ndim;
    return mask;

 out_of_mem:
    fprintf(stderr, "Fatal error in n_d_mask_create: not enough memory. Exiting.\n");
    exit(EXIT_FAILURE);
}

void n_d_mask_delete(N_d_mask* mask)
{
    if (mask)
    {
        if (mask->data)
            free(mask->data);
        if (mask->dim)
            free(mask->dim);
        if (mask->strides)
            free(mask->strides);
        free(mask);
    }
}

void n_d_mask_set(N_d_mask *mask, const unsigned *cell, const int onoff)
{
    unsigned idim, wordnum, ibit;
    unsigned long long bitnum = 0;

    for (idim=0; idim<mask->ndim; ++idim)
    {
#ifdef CHECK_BOUNDS
        assert(cell[idim] < mask->dim[idim]);
#endif
        bitnum += mask->strides[idim]*cell[idim];
    }
    wordnum = (unsigned)(bitnum/(8*sizeof(unsigned)));
    ibit = (unsigned)(bitnum % (8*sizeof(unsigned)));
    if (onoff)
        mask->data[wordnum] |= (1 << ibit);
    else
        mask->data[wordnum] &= ~(1 << ibit);
}

int n_d_mask_get(const N_d_mask *mask, const unsigned *cell)
{
    unsigned idim, wordnum, ibit;
    unsigned long long bitnum = 0;

    for (idim=0; idim<mask->ndim; ++idim)
    {
#ifdef CHECK_BOUNDS
        assert(cell[idim] < mask->dim[idim]);
#endif
        bitnum += mask->strides[idim]*cell[idim];
    }
    wordnum = (unsigned)(bitnum/(8*sizeof(unsigned)));
    ibit = (unsigned)(bitnum % (8*sizeof(unsigned)));
    if (mask->data[wordnum] & (1 << ibit))
        return 1;
    else
        return 0;
}

unsigned long long n_d_mask_num_cells(const N_d_mask *mask)
{
    assert(mask);
    {
        const unsigned maxdim = mask->ndim - 1;
        return mask->strides[maxdim] * mask->dim[maxdim];
    }
}

#ifdef __cplusplus
}
#endif
