#ifndef GSEXAMPLES_EXTERNALTEMPLATEIO_HH_
#define GSEXAMPLES_EXTERNALTEMPLATEIO_HH_

// Header file for the template we want to wrap
#include "ExternalTemplate.hh"

// The following header has to be included in order to wrap external
// classes or templates
#include "geners/GenericIO.hh"
#include "geners/IOException.hh"

// Specialize the ClassIdSpecialization template so that a ClassId object
// can be associated with the template we want to serialize. The easiest
// way to do this is to use the "gs_specialize_template_id_TT" macro.
// The first argument of the macro is the version number. The second
// argument tells how many template parameters affect the serialized
// representation (it is assumed that the parameters that do affect
// serialized representation come earlier in the parameter sequence).
// For example, if the second template parameter was not affecting the
// representation (e.g., as the allocator class for std::vector) then
// the second macro argument would be 1. The number of "T"s in _TT should
// correspond to the total number of template parameters (up to six).
// 
gs_specialize_template_id_TT(ExternalTemplate, 1, 2)
gs_specialize_template_id_T(ExternalTemplate2, 1, 0)

// The following macro tells the serialization system that the template
// is "external" for the I/O purposes so that the system will not try to
// apply its default serialization methods
//
gs_declare_template_external_TT(ExternalTemplate)
gs_declare_template_external_T(ExternalTemplate2)

// The most arcane part is to specialize the behavior of the two
// template classes at the heart of the serialization facility:
// gs::GenericWriter and gs::GenericReader.
//
namespace gs {
    // Serialization code for class ExternalTemplate
    //
    template <class Stream, class State, class A, class B>
    struct GenericWriter<Stream, State, ExternalTemplate<A,B>,
                         Int2Type<IOTraits<int>::ISEXTERNAL> >
    {
        inline static bool process(const ExternalTemplate<A,B>& s, Stream& os,
                                   State*, const bool processClassId)
        {
            // If necessary, serialize the class id
            static const ClassId current(ClassId::makeId<ExternalTemplate<A,B> >());
            const bool status = processClassId ? current.write(os) : true;

            // Serialize object data if the class id was successfully
            // written out
            if (status)
            {
                write_item(os, s.getA());
                write_item(os, s.getB());
            }

            // Return "true" on success, "false" on failure
            return status && !os.fail();
        }
    };

    template <class Stream, class State, class A, class B>
    struct GenericReader<Stream, State, ExternalTemplate<A,B>,
                         Int2Type<IOTraits<int>::ISEXTERNAL> >
    {
        inline static bool readIntoPtr(ExternalTemplate<A,B>*& ptr, Stream& is,
                                       State* st, const bool processClassId)
        {
            // Make sure that the serialized class id is consistent with
            // the current one
            static const ClassId current(ClassId::makeId<ExternalTemplate<A,B> >());
            const ClassId& stored = processClassId ? ClassId(is,1) : st->back();

            // Check that the name is consistent. Do not check for the
            // consistency of the complete id because we want to be able
            // to read different versions of classes A and B.
            current.ensureSameName(stored);

            // The following code does not assume that A or B have
            // default constructors
            CPP11_auto_ptr<A> pa = read_item<A>(is);
            CPP11_auto_ptr<B> pb = read_item<B>(is);
            if (ptr == 0)
                ptr = new ExternalTemplate<A,B>(*pa, *pb);
            else
                *ptr = ExternalTemplate<A,B>(*pa, *pb);
            return true;
        }

        inline static bool process(ExternalTemplate<A,B>& s, Stream& is,
                                   State* st, const bool processClassId)
        {
            // Simply convert reading by reference into reading by pointer
            ExternalTemplate<A,B>* ps = &s;
            return readIntoPtr(ps, is, st, processClassId);
        }
    };


    // Serialization code for class ExternalTemplate2.
    // Note that, in this case, A does not affect serialized
    // representation of "ExternalTemplate2".
    //
    template <class Stream, class State, class A>
    struct GenericWriter<Stream, State, ExternalTemplate2<A>,
                         Int2Type<IOTraits<int>::ISEXTERNAL> >
    {
        inline static bool process(const ExternalTemplate2<A>& s, Stream& os,
                                   State*, const bool processClassId)
        {
            // If necessary, serialize the class id
            static const ClassId current(ClassId::makeId<ExternalTemplate2<A> >());
            const bool status = processClassId ? current.write(os) : true;
            write_pod(os, s.getInt());

            // Return "true" on success, "false" on failure
            return status && !os.fail();
        }
    };

    template <class Stream, class State, class A>
    struct GenericReader<Stream, State, ExternalTemplate2<A>,
                         Int2Type<IOTraits<int>::ISEXTERNAL> >
    {
        inline static bool readIntoPtr(ExternalTemplate2<A>*& ptr, Stream& is,
                                       State* st, const bool processClassId)
        {
            // Make sure that the serialized class id is consistent with
            // the current one
            static const ClassId current(ClassId::makeId<ExternalTemplate2<A> >());
            const ClassId& stored = processClassId ? ClassId(is,1) : st->back();
            current.ensureSameId(stored);

            // The following code does not assume that A or B have
            // default constructors
            int i;
            read_pod(is, &i);
            if (is.fail()) throw IOReadFailure("Input stream failure "
                                               "while reading ExternalTemplate2");
            if (ptr == 0)
                ptr = new ExternalTemplate2<A>(i);
            else
                *ptr = ExternalTemplate2<A>(i);
            return true;
        }

        inline static bool process(ExternalTemplate2<A>& s, Stream& is,
                                   State* st, const bool processClassId)
        {
            // Simply convert reading by reference into reading by pointer
            ExternalTemplate2<A>* ps = &s;
            return readIntoPtr(ps, is, st, processClassId);
        }
    };
}

#endif // GSEXAMPLES_EXTERNALTEMPLATEIO_HH_
