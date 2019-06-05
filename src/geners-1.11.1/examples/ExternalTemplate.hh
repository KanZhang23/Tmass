//
// Example template class without default constructor and without
// serialization facilities. It does provide access to all members
// that need serialization. In order for us to serialize this template,
// the types upon which this template is parameterized must themselves
// be serializable (and, of course, copy-constructible).
//

#ifndef GSEXAMPLES_EXTERNALTEMPLATE_HH_
#define GSEXAMPLES_EXTERNALTEMPLATE_HH_

template<class A, class B>
class ExternalTemplate
{
public:
    inline ExternalTemplate(const A& a, const B& b) : a_(a), b_(b) {}

    inline const A& getA() const {return a_;}
    inline const B& getB() const {return b_;}

    // The comparison operator is here for convenience.
    // It helps to check that the serialization was correctly
    // performed but its existence is not strictly necessary
    // to serialize/deserialize such a template. Naturally,
    // this operator restricts possible A and B classes to
    // those types for which "operator==" is defined.
    //
    inline bool operator==(const ExternalTemplate& r) const
        {return a_ == r.a_ && b_ == r.b_;}
    inline bool operator!=(const ExternalTemplate& r) const
        {return !(*this == r);}

private:
    A a_;
    B b_;
};

//
// Example template class about which we will assume that its
// serialized representation does not need to depend on the
// template parameter
//
template<class A>
class ExternalTemplate2
{
public:
    inline ExternalTemplate2(const int b) : a_(), b_(b) {}

    inline const A& getA() const {return a_;}
    inline int getInt() const {return b_;}

    // We presume that the value of a_ does not
    // affect the "interesting" state of this class.
    //
    inline bool operator==(const ExternalTemplate2& r) const
        {return b_ == r.b_;}
    inline bool operator!=(const ExternalTemplate2& r) const
        {return !(*this == r);}

private:
    A a_;
    int b_;
};

#endif // GSEXAMPLES_MYTEMPLATE_HH_
