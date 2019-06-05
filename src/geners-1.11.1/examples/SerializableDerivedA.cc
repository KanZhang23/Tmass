#include <cassert>

#include "SerializableDerivedA.hh"

#include "geners/binaryIO.hh"
#include "geners/CPP11_auto_ptr.hh"
#include "geners/IOException.hh"

bool SerializableDerivedA::write(std::ostream& of) const
{
    gs::write_pod(of, a_);
    return !of.fail();
}

SerializableDerivedA* SerializableDerivedA::read(const gs::ClassId& id,
                                                 std::istream& in)
{
    // Class id for the current version of this class
    static const gs::ClassId myId(gs::ClassId::makeId<SerializableDerivedA>());

    if (id.name() == myId.name())
    {
        // We are reading an object of this particular class
        myId.ensureSameVersion(id);

        CPP11_auto_ptr<SerializableDerivedA> obj(new SerializableDerivedA(0));
        gs::read_pod(in, &obj->a_);
        if (in.fail())
            throw gs::IOReadFailure("In SerializableDerivedA::read: "
                                    "input stream failure");
        return obj.release();
    }
    else
    {
        // We are reading an object of some other class (normally,
        // it must be derived from this one). Do this via the read
        // function of the top-level base class.
        SerializableBase* base = SerializableBase::read(id, in);
        SerializableDerivedA* obj = dynamic_cast<SerializableDerivedA*>(base);

        // The above dynamic_cast must always succeed unless this function
        // was called with a completely inappropriate class id. Normally,
        // such a call should never happen and should be considered a bug.
        // Hence the assert below.
        assert(obj);
        return obj;
    }
}
