#include <cmath>
#include <climits>
#include <cassert>
#include <stdexcept>

#include "npstat/nm/binomialCoefficient.hh"
#include "npstat/rng/permutation.hh"

#define MAX_LD_FACTORIAL_ARG 1754U

namespace npstat {
    unsigned long binomialCoefficient(const unsigned N, const unsigned M)
    {
        if (M > N) throw std::invalid_argument(
            "In npstat::binomialCoefficient: must have M <= N");
        if (N == 0U || M == 0U || N == M)
            return 1UL;
        const unsigned start = M > N - M ? M + 1 : N - M + 1;
        unsigned long prod = 1UL;
        for (unsigned i=start, count=1UL; i <= N; ++i, ++count)
        {
            // Try to avoid overflows
            bool overflows = false;
            if (i % count == 0U)
            {
                const unsigned long factor = i/count;
                overflows = ULONG_MAX / prod < factor + 1UL;
                prod *= factor;
            }
            else if (prod % count == 0UL)
            {
                prod /= count;
                overflows = ULONG_MAX / prod < i + 1UL;
                prod *= i;
            }
            else
            {
                overflows = ULONG_MAX / prod < i + 1UL;
                prod *= i;
                if (!overflows)
                    assert(prod % count == 0UL);
                prod /= count;
            }
            if (overflows) throw std::overflow_error(
                "In npstat::binomialCoefficient: unsigned long overflow");
        }
        return prod;
    }

    long double ldBinomialCoefficient(const unsigned N, unsigned M)
    {
        static const unsigned minMForLogs = 20U;

        if (M > N) throw std::invalid_argument(
            "In npstat::ldBinomialCoefficient: must have M <= N");
        if (N == 0U || M == 0U || N == M)
            return 1.0L;
        if (N > MAX_LD_FACTORIAL_ARG)
        {
            if (M > N - M)
                M = N - M;
            if (M > minMForLogs)
                return expl(logfactorial(N)-logfactorial(M)-logfactorial(N-M));
            else
            {
                long double prod = 1.0L;
                for (unsigned i = N-M+1U; i <= N; ++i)
                    prod *= i;
                prod /= ldfactorial(M);
                return prod;
            }
        }
        else
            return ldfactorial(N)/ldfactorial(M)/ldfactorial(N-M);
    }
}
