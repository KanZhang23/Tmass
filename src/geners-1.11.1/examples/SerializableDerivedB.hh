// Example derived class for a serializable inheritance hierarchy

#ifndef GSEXAMPLES_SERIALIZABLEDERIVEDB_HH_
#define GSEXAMPLES_SERIALIZABLEDERIVEDB_HH_

#include "SerializableBase.hh"

class SerializableDerivedB : public SerializableBase
{
public:
    inline SerializableDerivedB(const double d) : SerializableBase(), b_(d)
    {
        std::cout << "SerializableDerivedB constructor" << std::endl;
    }

    inline virtual ~SerializableDerivedB()
    {
        std::cout << "SerializableDerivedB destructor" << std::endl;
    }

    inline double b() const {return b_;}

    virtual inline gs::ClassId classId() const {return gs::ClassId(*this);}
    virtual bool write(std::ostream& of) const;

    static inline const char* classname() {return "SerializableDerivedB";}
    static inline unsigned version() {return 1;}
    static SerializableDerivedB* read(const gs::ClassId& id, std::istream& in);

protected:
    virtual bool isEqual(const SerializableBase& otherBase) const
    {
        // Note the use of static_cast rather than dynamic_cast below.
        // static_cast works faster and it is guaranteed to succeed here.
        const SerializableDerivedB& r = 
            static_cast<const SerializableDerivedB&>(otherBase);
        return b_ == r.b_;
    }

private:
    double b_;
};

#endif // GSEXAMPLES_SERIALIZABLEDERIVEDB_HH_
