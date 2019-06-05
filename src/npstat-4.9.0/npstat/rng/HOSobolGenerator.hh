#ifndef NPSTAT_HOSOBOLGENERATOR_HH_
#define NPSTAT_HOSOBOLGENERATOR_HH_

/*!
// \file HOSobolGenerator.hh
//
// \brief Generator of higher-order scrambled Sobol sequences
//
// Author: I. Volobouev
//
// October 2010
*/

#include "npstat/rng/SobolGenerator.hh"

namespace npstat {
    /**
    // Generator of higher order scrambled Sobol sequences. The theory
    // behind this method is described in http://arxiv.org/abs/1007.0842
    //
    // Sampling according to the points produced by this generator is
    // supposed to achieve convergence rate of O(N^(-gamma-1/2+eps)),
    // where "gamma" equals to min(interlacingFactor, alpha). Here, alpha is
    // the maximum order of square-integrable continuous partial derivatives
    // in the integrand, and "interlacingFactor" is a parameter of the
    // scrambling algorithm. "eps" in the convergence rate stands for terms
    // which look like log(N)^dim. For large N, "eps" becomes vanishingly
    // small.
    //
    // The "interlacingFactor" parameter can not be made very high due to
    // certain limitations in the algorithm implementation. The main problem
    // is that, to construct scrambled random sequences with 2^k points,
    // the algorithm is using integers with magnitudes up to
    // 2^(interlacingFactor*k). This typically means that the product
    // interlacingFactor*k can not be easily made larger than 64 on 64-bit
    // systems. Therefore, if one uses interlacingFactor of 2, one can
    // produce 2^32 quasi-random numbers, for interlacingFactor of 3 this
    // becomes 2^21, for interlacingFactor of 4 this becomes 2^16, etc.
    // Also, the dimensionality of the underlying Sobol sequence becomes
    // larger than the dimensionality of the generated higher order sequence
    // by the interlacing factor.
    //
    // If this generator is used with the "interlacingFactor" parameter of 1
    // then it reproduces the original underlying Sobol sequence (which can
    // have up to 2^62 points).
    //
    // The theoretical convergence rate claims should be taken with a grain
    // of salt. In a simple numerical study which I did using multivariate
    // exponentials, "interlacingFactor" value of 2 did provide significant
    // improvement over the original Sobol sequence. "interlacingFactor"
    // value of 3 worked significantly better than "interlacingFactor" of 2
    // in 1 dimension, marginally better in 2 dimensions, and did not improve
    // anything in 3 dimensions. "interlacingFactor" value of 4 worked
    // marginally better than "interlacingFactor" of 3 in 1 dimension and
    // did not improve anything in higher dimensions. Since big values of N
    // can not be generated with this code for large interlacing factors,
    // perhaps this just means that one needs to reach higher N in order to
    // see the improvement due to scrambling. However, one can expect that
    // in this case the limited precision of the integrand evaluation will
    // become a dominant source of uncertainty and will spoil things anyway.
    */
    class HOSobolGenerator : public SobolGenerator
    {
    public:
        HOSobolGenerator(unsigned dim, unsigned interlacingFactor,
                         unsigned maxPowerOfTwo, unsigned nSkip=0U);
        inline virtual ~HOSobolGenerator() {}

        inline virtual unsigned dim() const {return dim_;}
        virtual void run(double* buf, unsigned bufSize, unsigned nPoints);

    private:
        long double normfactor_;
        long long bitset_[DIM_MAX];
        unsigned long long newset_[DIM_MAX];
        unsigned dim_;
        unsigned d_;
    };
}

#endif // NPSTAT_HOSOBOLGENERATOR_HH_
