//
// The following program illustrates the usage of "geners" package
// serialization mechanisms with an inheritance hierarchy. The following
// classes are used: SerializableBase, SerializableDerivedA (derived from
// SerializableBase), SerializableDerivedB (derived from SerializableBase),
// SerializableDerivedC (derived from SerializableDerivedB).
//
// SerializableBase (files SerializableBase.hh, SerializableBase.cc)
// is a top-level base class for a serializable hierarchy. Its "read"
// method (implemented inside SerializableBase.cc) is capable of reading
// all classes derived from SerializableBase. This is what happens inside
// that method:
//
// 1. An instance of a reader factory is created (or recalled) as a Meyers
//    singleton. This means that a reader factory should be implemented
//    as a companion to the top-level base class. See files
//    SerializableBaseReader.hh, SerializableBaseReader.cc.
//
// 2. The factory "read" method chooses the "read" function of a concrete
//    derived class using the class identifier.
//
// 3. The proper "read" function is called, object info is read from the
//    input stream and the object is created on the heap.
//
// The reader factory for SerializableBase descendants is defined inside
// the "SerializableBaseReader.hh" file. Naturally, the classes read by
// a particular factory must be registered with that factory. This
// registration can be done in two different ways:
//
// a) In the factory constructor, as in the file SerializableBaseReader.cc.
//    It is convenient to do so for all derived classes in a hierarchy
//    distributed with some library, etc.
//
// b) Anywhere else in the code but before any I/O related to that
//    hierarchy is performed. See the line of code in this file which
//    looks like
//
//    StaticSerializableBaseReader::registerClass<SerializableDerivedC>();
//
//    This must be done, for example, if an application implements new
//    classes which inherit from a base class defined in an external
//    library, and the constructor code for the associated reader factory
//    can not be modified.
//
#include "geners/vectorIO.hh"
#include "geners/IOException.hh"
#include "geners/StringArchive.hh"
#include "geners/Record.hh"
#include "geners/Reference.hh"

#include "SerializableBaseReader.hh"
#include "SerializableDerivedA.hh"
#include "SerializableDerivedB.hh"

// Create another class in the SerializableBase inheritance hierarchy
//
class SerializableDerivedC : public SerializableDerivedB
{
public:
    inline SerializableDerivedC(const unsigned u, const double d)
        : SerializableDerivedB(d), u_(u)
    {
        std::cout << "SerializableDerivedC constructor" << std::endl;
    }

    inline ~SerializableDerivedC()
    {
        std::cout << "SerializableDerivedC destructor" << std::endl;
    }

    inline unsigned u() const {return u_;}

    inline gs::ClassId classId() const {return gs::ClassId(*this);}
    inline bool write(std::ostream& of) const
    {
        // If writing out the complete object information at this point
        // would result in too much code duplication between the code
        // used here and the "write" method of the base class, it is
        // instead often possible to invoke
        //
        // SerializableDerivedB::classId().write(of);
        // SerializableDerivedB::write(of);
        //
        // This, of course, assumes that the "write" function of the base
        // is not abstract. However, this technique has several drawbacks.
        // The main drawback is that the implementation of the base class
        // "write" method may change with time. This means that we are
        // no longer in control of the exact format, so we must call the
        // "read" function of the base class in the "read" method of this
        // class. This leads to an extra copy of the base class contents
        // which must be later deleted. Another drawback is that the
        // class id of the base class must be written out for each object
        // of this class if proper evolution of the version numbers is
        // to be supported.
        //
        gs::write_pod(of, b());
        gs::write_pod(of, u_);
        return !of.fail();
    }

    static inline const char* classname() {return "SerializableDerivedC";}
    static inline unsigned version() {return 1;}
    static inline SerializableDerivedC* read(const gs::ClassId& id,
                                             std::istream& in)
    {
        // We do not expect any other class to be derived from
        // this one (note that the destructor is not virtual).
        // Because of this, we only need to handle the case
        // of correct class id.
        static const gs::ClassId myId(gs::ClassId::makeId<SerializableDerivedC>());
        myId.ensureSameId(id);

        double b(0.0);
        gs::read_pod(in, &b);
        unsigned u;
        gs::read_pod(in, &u);
        if (in.fail())
            throw gs::IOReadFailure("In SerializableDerivedC::read: "
                                    "input stream failure");
        return new SerializableDerivedC(u, b);
    }

protected:
    virtual bool isEqual(const SerializableBase& otherBase) const
    {
        // Note the call of "isEqual" method of the base class.
        // This should be done if that method is not pure virtual.
        const SerializableDerivedC& r = 
            static_cast<const SerializableDerivedC&>(otherBase);
        return SerializableDerivedB::isEqual(otherBase) && u_ == r.u_;
    }

private:
    unsigned u_;
};


int main(int, char**)
{
    typedef CPP11_shared_ptr<SerializableBase> BasePtr;
    typedef CPP11_shared_ptr<SerializableDerivedB> DerivedBPtr;

    // Register the new class with the reader factory
    StaticSerializableBaseReader::registerClass<SerializableDerivedC>();

    // Create a heterogeneous collection of objects using pointers
    // to the top-level base
    std::vector<BasePtr> coll1;
    coll1.push_back(BasePtr(new SerializableDerivedA(1)));
    coll1.push_back(BasePtr(new SerializableDerivedB(2.0)));
    coll1.push_back(BasePtr(new SerializableDerivedC(3U, 4.0)));

    // Write out this collection
    std::stringstream s;
    gs::write_item(s, coll1);

    // Read it back
    std::vector<BasePtr> coll2;
    gs::restore_item(s, &coll2);

    // Make sure the read back items are identical to the original ones
    {
        const unsigned len = coll1.size();
        assert(len == coll2.size());
        for (unsigned i=0; i<len; ++i)
            assert(*coll1[i] == *coll2[i]);
    }

    // Create and write a heterogeneous collection of objects using
    // pointers to a mid-level base
    std::vector<DerivedBPtr> coll3;
    coll3.push_back(DerivedBPtr(new SerializableDerivedB(5.0)));
    coll3.push_back(DerivedBPtr(new SerializableDerivedC(6U, 7.0)));
    gs::write_item(s, coll3);

    // Read it back
    std::vector<DerivedBPtr> coll4;
    gs::restore_item(s, &coll4);

    // Make sure the read back items are identical to the original ones
    {
        const unsigned len = coll3.size();
        assert(len == coll4.size());
        for (unsigned i=0; i<len; ++i)
            assert(*coll3[i] == *coll4[i]);
    }

    // Archive I/O example which illustrates the use of a base class
    // pointer/reference to store and retrieve the item
    gs::StringArchive ar;
    const SerializableDerivedB item(7.777777);

    // Write out the item using a reference to the base. Note explicit
    // dynamic cast to the base reference -- this reference must be used
    // if in the future we want to retrieve the item using a pointer to
    // the base class. This is because the class id stored in the archive
    // is always item's actual class id (this is what the "read" function
    // gets) while the class id in the catalog (which defines the item
    // type for all item search purposes) is the class id generated for
    // the reference used to store the item.
    //
    ar << gs::Record(dynamic_cast<const SerializableBase&>(item), "item", "");

    // Get the item back from the archive using a pointer to the base
    CPP11_auto_ptr<SerializableBase> p1 = 
        gs::Reference<SerializableBase>(ar, "item", "").get(0);

    // Make sure the retrieved item is the same as the original one
    assert(item == *p1);

    // We are done
    return 0;
}
