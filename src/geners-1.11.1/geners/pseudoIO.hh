#ifndef GENERS_PSEUDOIO_HH_
#define GENERS_PSEUDOIO_HH_

#include "geners/GenericIO.hh"

//
// Use the "gs_enable_pseudo_io" macro to enable "pseudo I/O" for
// stateless classes which do not need I/O but which, nevertheless,
// can be used as template parameters in places where I/O-enabled
// classes are expected. The class needs to have the default constructor
// and the assignment operator.
//
// On "write" operation the class id will be handled in the usual
// manner (written to disk as necessary). The object itself is,
// naturally, not written out.
//
// On "read" operation the I/O system will recreate the object using
// its default constructor.
//
#define gs_enable_pseudo_io_write_helper(QUAL, classname) /**/               \
  namespace gs {                                                             \
    template <class Stream, class State>                                     \
    struct GenericWriter<Stream, State, QUAL classname,                      \
                         Int2Type<IOTraits<int>::ISEXTERNAL> >               \
    {                                                                        \
      inline static bool process(const QUAL classname &, Stream& os,         \
                                 State*, const bool processClassId)          \
      {                                                                      \
        static const ClassId current(ClassId::makeId< classname >());        \
        return processClassId ? current.write(os) : true;                    \
      }                                                                      \
    };                                                                       \
  }

#define gs_enable_pseudo_io(classname) /**/                                  \
  gs_specialize_class_id(classname, 0U)                                      \
  gs_declare_type_external(classname)                                        \
  gs_enable_pseudo_io_write_helper(GENERS_EMPTY_TYPE_QUALIFYER_, classname)  \
  gs_enable_pseudo_io_write_helper(volatile, classname)                      \
  namespace gs {                                                             \
    template <class Stream, class State>                                     \
    struct GenericReader<Stream, State, classname,                           \
                         Int2Type<IOTraits<int>::ISEXTERNAL> >               \
    {                                                                        \
      inline static bool readIntoPtr(classname*& ptr, Stream& is,            \
                                     State* st, const bool processClassId)   \
      {                                                                      \
        static const ClassId current(ClassId::makeId< classname >());        \
        const ClassId& stored = processClassId ? ClassId(is,1) : st->back(); \
        current.ensureSameId(stored);                                        \
        if (ptr)                                                             \
            *ptr = classname ();                                             \
        else                                                                 \
            ptr = new classname ();                                          \
        return true;                                                         \
      }                                                                      \
      inline static bool process(classname& s, Stream& is,                   \
                                 State* st, const bool processClassId)       \
      {                                                                      \
        classname * ps = &s;                                                 \
        return readIntoPtr(ps, is, st, processClassId);                      \
      }                                                                      \
    };                                                                       \
  }

#endif // GENERS_PSEUDOIO_HH_
