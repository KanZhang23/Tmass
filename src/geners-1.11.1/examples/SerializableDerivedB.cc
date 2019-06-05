#include <cassert>

#include "SerializableDerivedB.hh"

#include "geners/binaryIO.hh"
#include "geners/CPP11_auto_ptr.hh"
#include "geners/IOException.hh"

bool SerializableDerivedB::write(std::ostream& of) const
{
    gs::write_pod(of, b_);
    return !of.fail();
}

SerializableDerivedB* SerializableDerivedB::read(const gs::ClassId& id,
                                                 std::istream& in)
{
    static const gs::ClassId myId(gs::ClassId::makeId<SerializableDerivedB>());

    if (id.name() == myId.name())
    {
        // We are reading an object of this particular class
        myId.ensureSameVersion(id);

        CPP11_auto_ptr<SerializableDerivedB> obj(new SerializableDerivedB(0.0));
        gs::read_pod(in, &obj->b_);
        if (in.fail())
            throw gs::IOReadFailure("In SerializableDerivedB::read: "
                                    "input stream failure");
        return obj.release();
    }
    else
    {
        // We are reading an object of some other class (normally,
        // it must be derived from this one). Do this via the read
        // function of the top-level base class.
        SerializableBase* base = SerializableBase::read(id, in);
        SerializableDerivedB* obj = dynamic_cast<SerializableDerivedB*>(base);

        // The above dynamic_cast must always succeed unless this function
        // was called with a completely inappropriate class id. Normally,
        // such a call should never happen and should be considered a bug.
        // Hence the assert below.
        assert(obj);
        return obj;
    }
}
