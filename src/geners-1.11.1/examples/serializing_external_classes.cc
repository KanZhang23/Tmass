//
// The following program illustrates usage of the archive I/O
// in the "geners" package with classes whose code can not be
// modified to play nicely with this I/O facility (e.g., they
// are members of external libraries owned by somebody else).
// To make the I/O of such classes meaningful, interfaces to such
// classes must expose sufficient amount of detail so that the
// complete object state can be captured and later restored.
//
// The technique illustrated here will not work for members of
// external inheritance hierarchies which can be addressed by
// pointer or reference to the base (slicing will occur).
//
#include <cmath>

// We will pretend that the following class belongs to some other library
namespace other {
    class Point2d
    {
    public:
        inline Point2d(double x, double y) : x_(x), y_(y) {}

        inline double x() const {return x_;}
        inline double y() const {return y_;}
        inline double distanceToOrigin() const {return sqrt(x_*x_ + y_*y_);}

        inline bool operator==(const Point2d& r) const
            {return x_ == r.x_ && y_ == r.y_;}
        inline bool operator!=(const Point2d& r) const
            {return !(*this == r);}

    private:
        Point2d();
        double x_;
        double y_;
    };
}

// The following header has to be included in order to wrap external
// classes or templates
#include "geners/GenericIO.hh"

// The next thing to do is to specialize the ClassIdSpecialization template
// so that a ClassId object can be associated with the class we want to
// serialize. The easiest way to do this is to use the "gs_specialize_class_id"
// macro (other similar macros exist which work for templates). The first
// argument of the macro is the class name (which will be stringized) and
// the second argument is the version number.
//
gs_specialize_class_id(other::Point2d, 1)

// The following macro tells the serialization system that the type is
// "external" for the I/O purposes so that the system will not try to
// apply its default serialization methods
//
gs_declare_type_external(other::Point2d)

// The most arcane part is to specialize the behavior of the two
// template classes at the heart of the serialization facility:
// gs::GenericWriter and gs::GenericReader.
//
namespace gs {
    //
    // This is how the specialization of GenericWriter should look like
    //
    template <class Stream, class State>
    struct GenericWriter<Stream, State, other::Point2d,
                         Int2Type<IOTraits<int>::ISEXTERNAL> >
    {
        inline static bool process(const other::Point2d& point, Stream& os,
                                   State*, const bool processClassId)
        {
            // If necessary, serialize the class id
            static const ClassId current(ClassId::makeId<other::Point2d>());
            const bool status = processClassId ? current.write(os) : true;

            // Serialize object data if the class id was successfully
            // written out
            if (status)
            {
                write_pod(os, point.x());
                write_pod(os, point.y());
            }

            // Return "true" on success, "false" on failure
            return status && !os.fail();
        }
    };

    // And this is the specialization of GenericReader
    //
    template <class Stream, class State>
    struct GenericReader<Stream, State, other::Point2d,
                         Int2Type<IOTraits<int>::ISEXTERNAL> >
    {
        inline static bool readIntoPtr(other::Point2d*& ptr, Stream& is,
                                       State* st, const bool processClassId)
        {
            // Make sure that the serialized class id is consistent with
            // the current one
            static const ClassId current(ClassId::makeId<other::Point2d>());
            const ClassId& stored = processClassId ? ClassId(is,1) : st->back();
            current.ensureSameId(stored);

            // Deserialize object data. This part, of course, has to be
            // consistent with the corresponding GenericWriter code and
            // can depend on what stored.version() returns. If there can
            // be several I/O versions of the serialized class, replace 
            // the "ensureSameId" call used above with "ensureSameName".
            double x=0, y=0;
            read_pod(is, &x);
            read_pod(is, &y);
            if (is.fail())
                // Return "false" on failure
                return false;

            // Build the object from the stored data
            if (ptr)
                *ptr = other::Point2d(x, y);
            else
                ptr = new other::Point2d(x, y);
            return true;
        }

        inline static bool process(other::Point2d& s, Stream& is,
                                   State* st, const bool processClassId)
        {
            // Simply convert reading by reference into reading by pointer
            other::Point2d* ps = &s;
            return readIntoPtr(ps, is, st, processClassId);
        }
    };
}

//
// Now, we can use other::Point2d class with the archives
// just as if it had the read/write methods, as illustrated
// by the code below
//
#include "geners/StringArchive.hh"
#include "geners/Record.hh"
#include "geners/Reference.hh"

using namespace gs;
using namespace other;

int main(int, char**)
{
    // The archive to use
    StringArchive ar;

    // The objects which we want to store in the archive
    Point2d p1(0, 1);
    Point2d p2(1, 2);

    // Store the objects
    ar << Record(p1, "First point", "Top")
       << Record(p2, "Second point", "Top");

    // Read the objects back and check that the retrived objects
    // are identical to the original ones. First, restore an object
    // on the stack...
    Point2d p2_2(0, 0);
    Reference<Point2d>(ar, "Second point", "Top").restore(0, &p2_2);
    assert(p2 == p2_2);

    // and now fetch another on the heap
    CPP11_auto_ptr<Point2d> p1_2 = Reference<Point2d>(
        ar, "First point", "Top").get(0);
    assert(p1 == *p1_2);

    // We are done
    return 0;
}
