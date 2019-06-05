#ifndef JESINTEGRESULT_HH_
#define JESINTEGRESULT_HH_

#include "AbsConvergenceChecker.hh"

#include "geners/ClassId.hh"

struct JesIntegResult
{
    inline JesIntegResult() {clear();}

    int uid;                  // "User-assigned id" of this object. It will be
                              // set to the ntuple row number which contains
                              // the input event.
                              //
    unsigned nDeltaJes;       // These parameters specify a grid in delta JES
    double minDeltaJes;       // as if the specification was provided for
    double maxDeltaJes;       // a histogram axis and delta JES values taken
                              // from the position of bin centers.

    // History of integral values. The outer index is the historical
    // period number and the inner index corresponds to the cell number
    // in the delta JES grid mentioned above.
    std::vector<std::vector<npstat::StatAccumulator> > history;

    // History of relative QMC uncertainties (with similar indexing)
    std::vector<std::vector<double> > qmcRelUncertainties;

    double timeElapsed;       // Integration wall clock time in seconds.
    ConvergenceStatus status; // Integration convergence status.

    bool operator==(const JesIntegResult& r) const;
    inline bool operator!=(const JesIntegResult& r) const
        {return !(*this == r);}

    void clear();

    // Change the duty cycle of the result -- that is, the duty
    // cycle of the new result with be 1.0/factor
    JesIntegResult changeDutyCycle(unsigned factor) const;

    inline gs::ClassId classId() const {return gs::ClassId(*this);}
    bool write(std::ostream& of) const;

    static inline const char* classname() {return "JesIntegResult";}
    static inline unsigned version() {return 2;}
    static void restore(const gs::ClassId& id, std::istream& in,
                        JesIntegResult* ptr);
};

#endif // JESINTEGRESULT_HH_
