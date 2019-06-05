#ifndef NPSTAT_NTUPLERECORDTYPESFWD_HH_
#define NPSTAT_NTUPLERECORDTYPESFWD_HH_

//=========================================================================
// NtupleRecordTypesFwd.hh
//
// Forwarding declarations for the NtupleRecordTypes.hh header.
// Applications should never use this header directly.
//
// Author: I. Volobouev
//
// November 2010
//=========================================================================

namespace npstat {
    namespace Private {
        template<class Ntuple> class NtupleHeaderRecord;
        template<class Ntuple> class NtupleBufferRecord;
        template<class Ntuple> class NtupleFooterRecord;
        template<class Ntuple> class NtupleBufferReference;
        template<class Ntuple> class NtupleColumnReference;
    }
}

#endif // NPSTAT_NTUPLERECORDTYPESFWD_HH_
