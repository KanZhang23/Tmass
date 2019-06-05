#ifndef NPSTAT_STORABLENONE_HH_
#define NPSTAT_STORABLENONE_HH_

#include "geners/binaryIO.hh"
#include "geners/IOException.hh"

namespace npstat {
    struct StorableNone
    {
        inline StorableNone() {}

        inline bool operator==(const StorableNone& r) const
            {return true;}
        inline bool operator!=(const StorableNone& r) const
            {return false;}

        // I/O methods needed for writing
        inline gs::ClassId classId() const {return gs::ClassId(*this);}
        inline bool write(std::ostream& of) const
        {
            unsigned char c = 0;
            gs::write_pod(of, c);
            return !of.fail();
        }

        // I/O methods needed for reading
        static inline const char* classname() {return "NoneType";}
        static inline unsigned version() {return 1;}
        static inline StorableNone* read(
            const gs::ClassId& id, std::istream& in)
        {
            static const gs::ClassId myClassId(gs::ClassId::makeId<StorableNone>());
            myClassId.ensureSameId(id);
            unsigned char c = 1;
            gs::read_pod(in, &c);
            if (in.fail()) throw gs::IOReadFailure("In npstat::StorableNone::read: "
                                                   "input stream failure");
            if (c) throw gs::IOInvalidData("In npstat::StorableNone::read: "
                                           "input stream corruption detected");
            return new StorableNone();
        }
    };
}

#endif // NPSTAT_STORABLENONE_HH_
