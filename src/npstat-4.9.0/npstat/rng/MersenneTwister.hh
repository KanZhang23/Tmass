#ifndef NPSTAT_MERSENNETWISTER_HH_
#define NPSTAT_MERSENNETWISTER_HH_

/*!
// \file MersenneTwister.hh
//
// \brief A wrapper for the Mersenne Twister generator of pseudo-random numbers
//
// Author: I. Volobouev
//
// October 2010
*/

#include "geners/CPP11_auto_ptr.hh"
#include "npstat/rng/AbsRandomGenerator.hh"

namespace npstat {
    namespace Private {
        class MTRand;
    }

    /**
    // Generator of pseudo-random numbers (with period 2^19937-1). Based on
    // the interface written by Richard J. Wagner to the original Mersenne
    // Twister.
    //
    // Reference:
    // M. Matsumoto and T. Nishimura, "Mersenne Twister: A 623-Dimensionally
    // Equidistributed Uniform Pseudo-Random Number Generator", ACM Transactions
    // on Modeling and Computer Simulation, Vol. 8, No. 1, January 1998, pp 3-30.
    */
    class MersenneTwister : public AbsRandomGenerator
    {
    public:
        /** The default constructor will read /dev/urandom to get the seed */
        MersenneTwister();
        explicit MersenneTwister(unsigned long seed);
        MersenneTwister(const MersenneTwister&);
        virtual ~MersenneTwister();

        MersenneTwister& operator=(const MersenneTwister&);

        inline unsigned dim() const {return 1U;}
        double operator()();

    private:
        Private::MTRand* impl_;
    };

    /*
    // A convenience function which calls the default constructor of
    // the MersenneTwister class if the seed is 0 and the constructor
    // with the seed if the seed is not 0.
    */
#ifndef SWIG
    CPP11_auto_ptr<AbsRandomGenerator> make_MersenneTwister(unsigned long seed);
#endif
}

#endif // NPSTAT_MERSENNETWISTER_HH_
