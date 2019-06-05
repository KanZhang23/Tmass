// Example base class for a serializable inheritance hierarchy

#ifndef GSEXAMPLES_SERIALIZABLEBASE_HH_
#define GSEXAMPLES_SERIALIZABLEBASE_HH_

#include <iostream>
#include <typeinfo>

#include "geners/ClassId.hh"

class SerializableBase
{
public:
    inline SerializableBase()
        : number_(nextObjectNumber())
    {
        std::cout << "SerializableBase constructor for object # "
                  << number_ << std::endl;
    }

    inline virtual ~SerializableBase()
    {
        std::cout << "SerializableBase destructor for object # "
                  << number_ << std::endl;
    }

    // Comparison operators. Note that they are not virtual and should
    // not be overriden by derived classes. These operators are very
    // useful for I/O testing.
    inline bool operator==(const SerializableBase& r) const
        {return (typeid(*this) == typeid(r)) && this->isEqual(r);}
    inline bool operator!=(const SerializableBase& r) const
        {return !(*this == r);}

    // I/O methods needed for writing. Must be overriden by derived classes.
    virtual gs::ClassId classId() const = 0;
    virtual bool write(std::ostream& of) const = 0;

    // I/O methods needed for reading
    static inline const char* classname() {return "SerializableBase";}
    static inline unsigned version() {return 1;}
    static SerializableBase* read(const gs::ClassId& id, std::istream& in);

protected:
    // Method needed to compare objects for equality.
    // Must be overriden by derived classes.
    virtual bool isEqual(const SerializableBase&) const = 0;

private:
    static unsigned long nextObjectNumber();
    unsigned long number_;
};

#endif // GSEXAMPLES_SERIALIZABLEBASE_HH_
