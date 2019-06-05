//
// The following program illustrates usage of the archive I/O
// in the "geners" package with templates whose code can not be
// modified to play nicely with this I/O facility (e.g., they
// are members of external libraries owned by somebody else).
// To make the I/O of such templates meaningful, interfaces to such
// templates must expose sufficient amount of detail so that the
// complete object state can be captured and later restored.
//
// The template which will be serialized is called "ExternalTemplate".
// The header which provides serialization facilities for this template
// is called "ExternalTemplateIO.hh". This header must be included here
// in order for us to work with such a template.
//

#include "ExternalTemplateIO.hh"

// We will use the class "SimpleSerializable2" as one of the
// template parameters.
#include "SimpleSerializable2.hh"

// Include Geners headers for archive I/O
#include "geners/StringArchive.hh"
#include "geners/Record.hh"
#include "geners/Reference.hh"

using namespace gs;


// An example class not known to "geners"
class Unknown
{
public:
    inline Unknown() {}
};


int main(int, char**)
{
    // The types we we want to store in the archive
    typedef ExternalTemplate<int, SimpleSerializable2> MyTemplate;
    typedef ExternalTemplate2<Unknown> MyTemplate2;

    // The archive to use
    StringArchive ar;

    // The objects which we want to store in the archive
    int i = 10;
    SimpleSerializable2 s2(3.0);
    MyTemplate object(i, s2);
    MyTemplate2 object2(5);

    // Store the objects
    ar << Record(object, "Example template", "")
       << Record(object2, "Template with one template argument", "");

    // Get the reference to this object in the archive
    Reference<MyTemplate> ref(ar, "Example template", "");
    assert(ref.unique());

    // Read the object back and check that the retrived object is
    // identical to the original one. First, restore it on the stack...
    MyTemplate readback(0, s2);
    assert(readback != object);
    ref.restore(0, &readback);
    assert(readback == object);

    // and now fetch it on the heap
    CPP11_auto_ptr<MyTemplate> pread = ref.get(0);
    assert(*pread == object);

    // Now, get "MyTemplate2" object
    Reference<MyTemplate2> ref2(ar, "Template with one template argument", "");
    assert(ref2.unique());
    MyTemplate2 readback2(3);
    assert(readback2 != object2);
    ref2.restore(0, &readback2);
    assert(readback2 == object2);
    
    // We are done
    return 0;
}
