// This example works only if C++11 is supported
#include "geners/CPP11_config.hh"
#ifdef CPP11_STD_AVAILABLE

#include "ExtDerivedAIO.hh"
#include "geners/IOException.hh"

bool ExtDerivedAIO::write(std::ostream& of, const wrapped_base& base,
                          const bool dumpId) const
{
    // If necessary, write out the class id
    const bool status = dumpId ? wrappedClassId().write(of) : true;

    // Write the object data out
    if (status)
    {
        const wrapped_type& w = dynamic_cast<const wrapped_type&>(base);
        gs::write_pod(of, w.getValue());
    }

    // Return "true" on success
    return status && !of.fail();
}

ExtDerivedA* ExtDerivedAIO::read(const gs::ClassId& id, std::istream& in) const
{
    // Validate the class id. You might want to implement
    // class versioning here.
    wrappedClassId().ensureSameId(id);

    // Read in the object data
    int i;
    gs::read_pod(in, &i);

    // Check that the stream is in a valid state
    if (in.fail()) throw gs::IOReadFailure(
        "In ExtDerivedAIO::read: input stream failure");

    // Return new object
    return new ExtDerivedA(i);
}

const gs::ClassId& ExtDerivedAIO::wrappedClassId()
{
    static const gs::ClassId wrapId(gs::ClassId::makeId<wrapped_type>());
    return wrapId;
}

#endif // CPP11_STD_AVAILABLE
