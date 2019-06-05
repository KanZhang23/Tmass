#ifndef GENERS_CHARBUFFER_HH_
#define GENERS_CHARBUFFER_HH_

#include <iostream>

#include "geners/ClassId.hh"
#include "geners/CStringBuf.hh"

namespace gs {
    class CharBuffer : private CStringBuf, public std::basic_iostream<char>
    {
    public:
        CharBuffer();

        unsigned long size() const;

        inline ClassId classId() const {return ClassId(*this);}
        bool write(std::ostream& of) const;

        static inline const char* classname() {return "gs::CharBuffer";}
        static inline unsigned version() {return 1;}
        static void restore(const ClassId& id, std::istream& in,
                            CharBuffer* buf);

        bool operator==(const CharBuffer& r) const;
        inline bool operator!=(const CharBuffer& r) const
            {return !(*this == r);}

    private:
        CharBuffer(const CharBuffer&);
        CharBuffer& operator=(const CharBuffer&);
    };
}

#endif // GENERS_CHARBUFFER_HH_
