The code in this directory can depend on headers from the "nm" directory
of the "npstat" package but not from any other directory.

The classes implemented in this directory mainly deal with generation
of pseudo- and quasi-random multivariate sequences -- while the C++11
standard has a good coverage of pseudo-random generators, quasi-random
numbers are still not there. Several utilities related to random
numbers and combinatorics are implemented in this directory as well.


Random Number Generators
------------------------

AbsRandomGenerator.hh     -- Abstract interface class for pseudo- and
                             quasi-random number generators. All other
                             generators inherit from this interface.

CPP11RandomGen.hh         -- Wrapper for generator engines defined in the
                             C++11 standard.

EquidistantSampler1D.hh   -- Samples a 1-d interval in equidistant steps,
                             like bin centers in a histogram. There is
                             nothing random about it.

SobolGenerator.hh         -- Multivariate Sobol quasi-random sequences.
                             Note that the improved integral convergence
                             over pseudo-random sequences is achieved only
                             if the number of points used is 2^M - 2^K,
                             with some integer M and K (often K = 0).

HOSobolGenerator.hh       -- Higher order scrambled Sobol sequences. See
                             the comments in the header file if you want to
                             know more. Probably, the best sequence overall
                             for use in phase space integration.

MersenneTwister.hh        -- The famous Mersenne twister and its implementation
MersenneTwisterImpl.hh       (should be replaced by the C++11 implementation
                             in the future).

RandomSequenceRepeater.hh -- Can be used to store and repeat a sequence
                             made by other generators. This can be useful
                             when integration is performed in the same manner
                             for a number of parameter values.  

RegularSampler1D.hh       -- Regular sampler of the unit interval conforming
                             to the AbsRandomGenerator interface. Splits
                             the interval in half, then splits obtained
                             halves in half, etc. Naturally, needs 2^M - 1
                             calls (with integer M) to perform regular
                             sampling. Unlike EquidistantSampler1D, it is
                             not necessary to recalculate function values
                             for previous points if you decide to increase M
                             on the basis of some convergence check.

Utilities
---------

convertToSphericalRandom.hh -- converts random numbers generated in
                               an N-dimensional hypercube into a random
                               direction in the corresponding space (a point
                               on a hypersphere) plus one independent random
                               number on [0, 1) which can later be used to
                               generate the distance to the origin.

permutation.hh -- utilities related to permuting a set of consecutive
                  integers, iterating over permutations, and calculating
                  factorials.
