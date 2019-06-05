#include <string.h>
#include "permutations.h"

#ifdef __cplusplus
extern "C" {
#endif

int parse_jet_number(const char *jet_name)
{
    if (strcasecmp(jet_name, "qbar") == 0)
        return (int)QBAR;
    if (strcasecmp(jet_name, "q") == 0)
        return (int)Q;
    if (strcasecmp(jet_name, "blep") == 0)
        return (int)BLEP;
    if (strcasecmp(jet_name, "bhad") == 0)
        return (int)BHAD;
    return -1;
}

const Permutation allPermutations[N_JET_PERMUTATIONS] = {
    {{BHAD, BLEP, Q, QBAR}},
    {{BLEP, BHAD, Q, QBAR}},
    {{BHAD, Q, BLEP, QBAR}},
    {{BLEP, Q, BHAD, QBAR}},
    {{BHAD, Q, QBAR, BLEP}},
    {{BLEP, Q, QBAR, BHAD}},
    {{Q, BHAD, BLEP, QBAR}},
    {{Q, BLEP, BHAD, QBAR}},
    {{Q, BHAD, QBAR, BLEP}},
    {{Q, BLEP, QBAR, BHAD}},
    {{Q, QBAR, BHAD, BLEP}},
    {{Q, QBAR, BLEP, BHAD}},
    {{BHAD, BLEP, QBAR, Q}},
    {{BLEP, BHAD, QBAR, Q}},
    {{BHAD, QBAR, BLEP, Q}},
    {{BLEP, QBAR, BHAD, Q}},
    {{BHAD, QBAR, Q, BLEP}},
    {{BLEP, QBAR, Q, BHAD}},
    {{QBAR, BHAD, BLEP, Q}},
    {{QBAR, BLEP, BHAD, Q}},
    {{QBAR, BHAD, Q, BLEP}},
    {{QBAR, BLEP, Q, BHAD}},
    {{QBAR, Q, BHAD, BLEP}},
    {{QBAR, Q, BLEP, BHAD}}
};

#ifdef __cplusplus
}
#endif
