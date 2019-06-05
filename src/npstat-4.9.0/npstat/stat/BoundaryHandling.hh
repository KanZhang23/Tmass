#ifndef NPSTAT_BOUNDARYHANDLING_HH_
#define NPSTAT_BOUNDARYHANDLING_HH_

/*!
// \file BoundaryHandling.hh
//
// \brief API for LOrPE boundary handling methods
//
// Author: I. Volobouev
//
// June 2015
*/

#include <vector>
#include <string>

#include "geners/ClassId.hh"

namespace npstat {
    class BoundaryHandling
    {
    public:
        /** 
        // Maximum number of parameters used by any of the LOrPE boundary
        // handling methods
        */
        enum {
            BM_MAXPARAMS = 4U
        };

        /*
        // Default constructor actually creates a usable default method
        */
        inline BoundaryHandling() {initialize(0, 0, 0);}

        inline explicit BoundaryHandling(const char* in_methodName,
                                         const double* params = 0,
                                         const unsigned nParams = 0)
            {initialize(in_methodName, params, nParams);}

        inline BoundaryHandling(const char* in_methodName,
                                const std::vector<double>& params)
        {
            const unsigned nParams = params.size();
            initialize(in_methodName, nParams?&params[0]:(double*)0, nParams);
        }

        const char* methodName() const;

        inline const double* parameters() const
            {return nParams_ ? params_ : (double*)0;}

        inline unsigned nParameters() const {return nParams_;}

        /** 
        // The id of the given boundary handling method can change from
        // one NPStat release to another, so do not store it anywhere
        */
        inline unsigned methodId() const {return id_;}

        bool operator==(const BoundaryHandling& r) const;
        inline bool operator!=(const BoundaryHandling& r) const
            {return !(*this == r);}

        bool operator<(const BoundaryHandling& r) const;

        inline bool operator>(const BoundaryHandling& r) const
            {return r < *this;}

        /**
        // Does given C-string represent a valid boundary method name?
        // Valid boundary method names are listed in the comments to the
        // "npstat/stat/BoundaryMethod.hh" header file. Nullptr corresponds
        // to a valid name with id 0.
        */
        static bool isValidMethodName(const char* name);

        /** All valid boundary method names (for use in error messages, etc) */
        static std::string validMethodNames();

        /**
        // Does given C-string represent a valid boundary method name for
        // a method which does not need additional parameters?
        */
        static bool isParameterFree(const char* name);

        /** Names of boundary methods which do not need any parameters */
        static std::string parameterFreeNames();

        /** Number of parameters needed for the method with the given name */
        static unsigned expectedNParameters(const char* name);

        // I/O methods needed for writing
        inline gs::ClassId classId() const {return gs::ClassId(*this);}
        bool write(std::ostream& of) const;

        // I/O methods needed for reading
        static inline const char* classname()
            {return "npstat::BoundaryHandling";}
        static inline unsigned version() {return 1;}
        static void restore(const gs::ClassId& id, std::istream& in,
                            BoundaryHandling* ptr);

    private:
        void initialize(const char* methodName,
                        const double* params, unsigned nParams);

        double params_[BM_MAXPARAMS];
        unsigned nParams_;
        unsigned id_;
    };
}

#endif // NPSTAT_BOUNDARYHANDLING_HH_
