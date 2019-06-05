//=========================================================================
// ProcessItem.hh
//
// We will employ a compile-time visitor pattern. The idea
// is to replace function calls (which are difficult to develop)
// with partial template specializations (which are relatively
// easy to develop).
//
// The call signature of the visitor will be
//
// bool InspectingVisitor<Arg1, Arg2, T, Stage>::process(
//         const T&, Arg1&, Arg2*, const bool processClassId);
//
// or
//
// bool ModifyingVisitor<Arg1, Arg2, T, Stage>::process(
//         T&, Arg1&, Arg2*, const bool processClassId);
//
// The processing will be terminated as soon as any call to
// the visitor's process function returns "false".
//
// I. Volobouev
// October 2010
//=========================================================================

#ifndef GENERS_PROCESSITEM_HH_
#define GENERS_PROCESSITEM_HH_

#include "geners/IOTraits.hh"
#include "geners/Int2Type.hh"

namespace gs {
    // Special types to designate stages in container processing
    struct InContainerHeader 
    {
        static const char* stage() {return "InContainerHeader";}
    };

    struct InContainerSize
    {
        static const char* stage() {return "InContainerSize";}
    };

    struct InContainerFooter
    {
        static const char* stage() {return "InContainerFooter";}
    };

    struct InContainerCycle
    {
        static const char* stage() {return "InContainerCycle";}
    };

    struct InPODArray
    {
        static const char* stage() {return "InPODArray";}
    };
}

// I am not aware of an easy way to have both const and non-const
// version of a template defined in the same code fragment. This is
// why you see some preprocessor tricks below -- the alternative
// of maintaining separate const and non-const codes is much worse.

#ifdef GENERS_GENERATE_CONST_IO_PROCESSOR
#undef GENERS_GENERATE_CONST_IO_PROCESSOR
#endif

#include "geners/ProcessItem.icc"

#define GENERS_GENERATE_CONST_IO_PROCESSOR

#include "geners/ProcessItem.icc"

#undef GENERS_GENERATE_CONST_IO_PROCESSOR

#endif // GENERS_PROCESSITEM_HH_
