#ifndef ABSCONVERGENCECHECKER_HH_
#define ABSCONVERGENCECHECKER_HH_

#include <vector>
#include "npstat/stat/StatAccumulator.hh"

enum ConvergenceStatus
{
    CS_CONTINUE = 0,
    CS_CONVERGED,
    CS_MAX_CYCLES,
    CS_OUT_OF_TIME
};

class AbsConvergenceChecker
{
public:
    inline virtual ~AbsConvergenceChecker() {}

    // "history" has indices [setNumber][jesNumber], where
    // "setNumber" is a checkpoint number in the integration
    // and "jesNumber" is the jet energy scale incertainty index.
    virtual ConvergenceStatus check(
        double timeElapsed,
        const std::vector<std::vector<npstat::StatAccumulator> >& history) const=0;

    // Mathod for accumulating function values. The first
    // argument is the identifier of the point in the
    // parameter space and the second argument is the
    // value of the integrated function.
    virtual void accumulate(unsigned i, double value)=0;

    // Method for retrieving the relative uncertainty estimate
    // for the point with the given index (i.e., identifier)
    // in the parameter space.
    virtual double getRelUncertainty(unsigned i) const=0;
};

class WallClockTimeLimit : public AbsConvergenceChecker
{
public:
    inline explicit WallClockTimeLimit(const double dt) : tmax_(dt) {}

    inline virtual ~WallClockTimeLimit() {}

    inline virtual ConvergenceStatus check(
        const double timeElapsed,
        const std::vector<std::vector<npstat::StatAccumulator> >&) const
    {
        if (timeElapsed < tmax_)
            return CS_CONTINUE;
        else
            return CS_OUT_OF_TIME;
    }

    inline virtual void accumulate(unsigned, double) {}

    // This class does not return a meaningful uncertainty
    inline virtual double getRelUncertainty(unsigned) const {return -1.0;}

private:
    WallClockTimeLimit();

    double tmax_;
};

#endif // ABSCONVERGENCECHECKER_HH_
