#ifndef NPSTAT_NTUPLEREFERENCE_HH_
#define NPSTAT_NTUPLEREFERENCE_HH_

/*!
// \file NtupleReference.hh
//
// \brief Special reference class for ArchivedNtuple objects
//
// Author: I. Volobouev
//
// November 2010
*/

#include "geners/CPP11_auto_ptr.hh"
#include "geners/CPP11_shared_ptr.hh"
#include "geners/AbsReference.hh"
#include "geners/ClassId.hh"

namespace npstat {
    /**
    // Reference for finding objects of class ArchivedNtuple in the binary
    // archives. Interface is similar to the standard utility class Reference
    // in Reference.hh, but there is no "restore" method. "restore" would not
    // make sense here as we can not put the complete archive onto the heap.
    */
    template <typename Ntuple>
    class NtupleReference : public gs::AbsReference
    {
    public:
        /** Find item by its unique id in the archive */
        inline NtupleReference(gs::AbsArchive& ar,
                               const unsigned long long itemId)
            : gs::AbsReference(ar, gs::ClassId::makeId<Ntuple>(),
                               "npstat::NtupleHeader", itemId) {}

        /** Find (possibly more than one) item by its name and category */
        inline NtupleReference(gs::AbsArchive& ar,
                               const char* name, const char* category)
            : gs::AbsReference(ar, gs::ClassId::makeId<Ntuple>(),
                               "npstat::NtupleHeader", name, category) {}

        /** Search for items of ArchivedNtuple type */
        inline NtupleReference(gs::AbsArchive& ar,
                               const gs::SearchSpecifier& namePattern,
                               const gs::SearchSpecifier& categPattern)
            : gs::AbsReference(ar, gs::ClassId::makeId<Ntuple>(),
                               "npstat::NtupleHeader",
                               namePattern, categPattern) {}

        inline virtual ~NtupleReference() {}

        /** Retrieve ArchivedNtuple header record and build the ntuple */
        inline CPP11_auto_ptr<Ntuple> get(const unsigned long index) const
            {return CPP11_auto_ptr<Ntuple>(getPtr(index));}

        /** Retrieve ArchivedNtuple header record and build the ntuple */
        inline CPP11_shared_ptr<Ntuple> getShared(
            const unsigned long index) const
            {return CPP11_shared_ptr<Ntuple>(getPtr(index));}

    private:
        inline Ntuple* getPtr(const unsigned long number) const
        {
            Ntuple* readout = 0;
            const unsigned long long itemId = id(number);
            if (itemId)
                readout = Ntuple::read(archive(),
                                       this->positionInputStream(itemId),
                                       itemId);
            return readout;
        }
    };
}

#endif // NPSTAT_NTUPLEREFERENCE_HH_
