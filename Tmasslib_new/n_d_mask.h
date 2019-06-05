#ifndef N_D_MASK_H_
#define N_D_MASK_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned ndim;
    unsigned *data;
    unsigned *dim;
    unsigned long long *strides;
} N_d_mask;

N_d_mask* n_d_mask_create(const unsigned *dim, unsigned ndim);

void n_d_mask_delete(N_d_mask* mask);

void n_d_mask_set(N_d_mask *mask, const unsigned *cell, int onoff);

int n_d_mask_get(const N_d_mask *mask, const unsigned *cell);

unsigned long long n_d_mask_num_cells(const N_d_mask *mask);

#ifdef __cplusplus
}
#endif

#endif /* N_D_MASK_H_ */
