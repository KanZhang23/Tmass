#ifndef NPSTAT_CPP11RANDOMGEN_HH_
#define NPSTAT_CPP11RANDOMGEN_HH_

/*!
// \file CPP11RandomGen.hh
//
// \brief Wrapper for random generators in the C++11 standard
//
// Author: I. Volobouev
//
// April 2012
*/

#include <cmath>
#include <limits>

#include "npstat/rng/AbsRandomGenerator.hh"

namespace npstat {
    /**
    // Wrapper class for random number generators defined in the C++11
    // standard. This class will not make a copy of the original generator,
    // only a reference to it is stored. You will not be able to use this
    // class in a meaningful way unless you have a C++11-compliant compiler
    // which provides the <random> header.
    //
    // The template type should be one of the random engines
    // defined by C++11. Example:
    //
    // std::random_device rd;
    // std::mt19937 eng(rd());
    // npstat::CPP11RandomGen<decltype(eng)> gen(eng);
    //
    // Now, gen() will produce pseudo-random numbers.
    */
    template<typename RandomEngine>
    class CPP11RandomGen : public AbsRandomGenerator
    {
    public:
        inline explicit CPP11RandomGen(RandomEngine& fcn) : f_(fcn) {}

        inline virtual ~CPP11RandomGen() {}

        inline unsigned dim() const {return 1U;}

        inline double operator()()
        {
            const unsigned b = std::numeric_limits<double>::digits;
            const long double r = static_cast<long double>(f_.max())
                - static_cast<long double>(f_.min()) + 1.0L;
            const unsigned log2r = std::log(r)/std::log(2.0L);
            unsigned k = std::max<unsigned>(1U, (b + log2r - 1UL)/log2r);
            double sum = 0.0;
            double tmp = 1.0;
            for (; k != 0; --k)
            {
                sum += double(f_() - f_.min())*tmp;
                tmp *= r;
            }
            return sum/tmp;
        }

    private:
        CPP11RandomGen();
        RandomEngine& f_;
    };
}

#endif // NPSTAT_CPP11RANDOMGEN_HH_
