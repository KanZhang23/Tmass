#ifndef NPSTAT_ABSRANDOMGENERATOR_HH_
#define NPSTAT_ABSRANDOMGENERATOR_HH_

/*!
// \file AbsRandomGenerator.hh
//
// \brief Interface definition for pseudo- and quasi-random number generators
//
// Author: I. Volobouev
//
// March 2010
*/

#include <cassert>
#include <stdexcept>

namespace npstat {
    /**
    // Interface class for pseudo- and quasi-random number generators.
    //
    // While implementing this interface, multivariate generators
    // should override "run" and "operator()" while 1-d generators
    // should just override "operator()". In multivariate contexts,
    // one should always use "run" rather than "operator()".
    */
    struct AbsRandomGenerator
    {
        inline virtual ~AbsRandomGenerator() {}

        /**
        // Dimensionality of the generated vectors (or points).
        // It is expected that "dim()" will always be 1 for
        // pseudo-random (as opposed to quasi-random) generators.
        */
        virtual unsigned dim() const = 0;

        /**
        // Maximum number of points which can be meaningfully produced
        // by this generator (this is usually the generator period).
        // If 0 is returned, it means that this number is larger than
        // the maximum value of unsigned long long (which is typically
        // 2^64 - 1). Override as necessary.
        */
        virtual unsigned long long maxPoints() const {return 0;}

        /**
        // Standard 1-d generator function. It should generate
        // a run-time error for essentially multivariate generators
        // whose dimensionality is larger than 1.
        */
        virtual double operator()() = 0;

        /**
        // Generate a bunch of points. Multivariate generators
        // must override this. Here, the effective dimensionality
        // of the generator is bufLen/nPt.
        */
        virtual void run(double* buf, const unsigned bufLen, const unsigned nPt)
        {
            if (nPt)
            {
                if (this->dim() != 1U) throw std::invalid_argument(
                    "In npstat::AbsRandomGenerator::run: this method "
                    "must be overriden by multivariate generators");
                assert(buf);
                for (unsigned i=0; i<bufLen; ++i)
                    buf[i] = this->operator()();
            }
        }
    };

    /**
    // Wrapper for functions which look like "double generate()",
    // for example, "drand48" from cstdlib on Linux.
    */
    class WrappedRandomGen : public AbsRandomGenerator
    {
    public:
        inline explicit WrappedRandomGen(double (*fcn)()) : f_(fcn) {}
        inline virtual ~WrappedRandomGen() {}

        inline unsigned dim() const {return 1U;}
        inline double operator()() {return f_();}

    private:
        WrappedRandomGen();
        double (*f_)();
    };
}

#endif // NPSTAT_ABSRANDOMGENERATOR_HH_
