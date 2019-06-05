#ifndef PERMUTATIONS_H
#define PERMUTATIONS_H

#define N_JET_PERMUTATIONS 24

#ifdef __cplusplus
extern "C" {
#endif

/* Jet numbering */
enum {
    Q = 0,
    QBAR,
    BLEP,
    BHAD
};

typedef struct {
    int index[4];
} Permutation;

extern const Permutation allPermutations[N_JET_PERMUTATIONS];

int parse_jet_number(const char *jet_name);

#ifdef __cplusplus
}
#endif

#endif /* PERMUTATIONS_H */
