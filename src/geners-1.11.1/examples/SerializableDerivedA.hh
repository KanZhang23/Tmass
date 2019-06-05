// Example derived class for a serializable inheritance hierarchy

#ifndef GSEXAMPLES_SERIALIZABLEDERIVEDA_HH_
#define GSEXAMPLES_SERIALIZABLEDERIVEDA_HH_

#include "SerializableBase.hh"

class SerializableDerivedA : public SerializableBase
{
public:
    inline SerializableDerivedA(const int i) : SerializableBase(), a_(i)
    {
        std::cout << "SerializableDerivedA constructor" << std::endl;
    }

    inline virtual ~SerializableDerivedA()
    {
        std::cout << "SerializableDerivedA destructor" << std::endl;
    }

    inline int a() const {return a_;}
    
    virtual inline gs::ClassId classId() const {return gs::ClassId(*this);}
    virtual bool write(std::ostream& of) const;

    static inline const char* classname() {return "SerializableDerivedA";}
    static inline unsigned version() {return 1;}
    static SerializableDerivedA* read(const gs::ClassId& id, std::istream& in);

protected:
    virtual bool isEqual(const SerializableBase& otherBase) const
    {
        // Note the use of static_cast rather than dynamic_cast below.
        // static_cast works faster and it is guaranteed to succeed here.
        const SerializableDerivedA& r = 
            static_cast<const SerializableDerivedA&>(otherBase);
        return a_ == r.a_;
    }

private:
    int a_;
};

#endif // GSEXAMPLES_SERIALIZABLEDERIVEDA_HH_
