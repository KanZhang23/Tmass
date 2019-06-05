#ifndef SOBOL_F_H_
#define SOBOL_F_H_

#define MAX_SOBOL_F_DIM 40

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  DIMEN : dimension
 *  ATMOST : sequence length
 *  MAXS : Maximum Digits of Scrambling Of Owen type Scrambling
 *  IFLAG: User Choice of Sequences
 *     IFLAG = 0 : No Scrambling
 *     IFLAG = 1 : Owen type Scrambling
 *     IFLAG = 2 : Faure-Tezuka type Scrambling
 *     IFLAG = 3 : Owen + Faure-Tezuka type Scrambling
 *
 *  If initialization is successful, both FLAG[0] and FLAG[1] are set to 1
 *  on exit.
 */
void insobl_(int FLAG[2], int *DIMEN, int *ATMOST, int *TAUS,
             double QUASI[MAX_SOBOL_F_DIM], int *MAXS, int *IFLAG);

/* Get the next Sobol point */
void gosobl_(double QUASI[MAX_SOBOL_F_DIM]);

/* Initialization for Niederreiter's sequence */
void sinlo2_(int *DIMEN, int *SKIP, int *IFLAG);

/* Get the next Niederreiter's point */
void sgolo2_(double QUASI[MAX_SOBOL_F_DIM]);

#ifdef __cplusplus
}
#endif

#endif /* SOBOL_F_H_ */
