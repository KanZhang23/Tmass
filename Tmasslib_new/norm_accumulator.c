#include <math.h>
#include <string.h>
#include <assert.h>

#include "norm_accumulator.h"

#ifdef __cplusplus
extern "C" {
#endif

static long double ld_norm_error(const norm_accumulator *acc)
{
    if (acc->ntries > 1)
    {
        const long double mean = acc->wsum/acc->ntries;
        const long double var = acc->wsumsq/acc->ntries - mean*mean;
        if (var > 0.0L)
            return sqrtl(var/(acc->ntries - 1));
        else
            return 0.0L;
    }
    else
        return 0.0L;
}

double norm_value(const norm_accumulator *acc)
{
    if (acc->ntries > 0)
        return acc->wsum/acc->ntries;
    else
        return 0.0;
}

void norm_accumulate(norm_accumulator *acc, const double weight)
{
    const long double w = weight;
    assert(weight >= 0.0);
    acc->wsum += w;
    acc->wsumsq += w*w;
    if (weight > acc->wmax)
        acc->wmax = weight;
    ++acc->ntries;
}

double norm_error(const norm_accumulator *acc)
{
    if (acc->ntries > 1)
    {
        const long double mean = acc->wsum/acc->ntries;
        const long double var = acc->wsumsq/acc->ntries - mean*mean;
        if (var > 0.0L)
            return sqrt((double)var/(acc->ntries - 1));
        else
            return 0.0;
    }
    else
        return 0.0;
}

void norm_reset(norm_accumulator *acc)
{
    memset(acc, 0, sizeof(norm_accumulator));
}

void norm_scale(norm_accumulator *acc, const double factor)
{
    acc->wsum *= factor;
    acc->wsumsq *= (factor * factor);
    acc->wmax *= factor;
}

void norm_add_maxcorr(norm_accumulator *acc,
                      const norm_accumulator *added)
{
    if (acc->ntries == 0)
    {
        memcpy(acc, added, sizeof(norm_accumulator));
    }
    else if (added->ntries)
    {
        const unsigned new_n = acc->ntries + added->ntries;
        const long double new_mean = acc->wsum/acc->ntries +
                                     added->wsum/added->ntries;
        const long double new_error = ld_norm_error(acc) + 
                                      ld_norm_error(added);
        const long double new_var = new_error*new_error*(new_n - 1);

        acc->ntries = new_n;
        acc->wsum   = new_mean*new_n;
        acc->wsumsq = (new_var + new_mean*new_mean)*new_n;

        if (added->wmax > acc->wmax)
            acc->wmax = added->wmax;
    }
}

void norm_add_nocorr(norm_accumulator *acc,
                     const norm_accumulator *added)
{
    if (acc->ntries == 0)
    {
        memcpy(acc, added, sizeof(norm_accumulator));
    }
    else if (added->ntries)
    {
        long double left_var = 0.0, right_var = 0.0, new_mean, new_var;
        const unsigned new_n = acc->ntries + added->ntries;
        const long double left_mean = acc->wsum/acc->ntries;
        const long double right_mean = added->wsum/added->ntries;
        if (acc->ntries > 1)
            left_var = (acc->wsumsq/acc->ntries - left_mean*left_mean)/
                (acc->ntries - 1);
        if (added->ntries > 1)
            right_var = (added->wsumsq/added->ntries - right_mean*right_mean)/
                (added->ntries - 1);
        new_mean = left_mean + right_mean;
        new_var = (left_var + right_var)*(new_n - 1);

        acc->ntries = new_n;
        acc->wsum   = new_mean*new_n;
        acc->wsumsq = (new_var + new_mean*new_mean)*new_n;

        if (added->wmax > acc->wmax)
            acc->wmax = added->wmax;
    }
}

#ifdef __cplusplus
}
#endif
