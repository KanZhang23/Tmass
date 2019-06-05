//=========================================================================
// ClassId.hh
//
// Class identifier for I/O operations. Contains class name and version
// number. For templates, it should also contain version numbers of all
// template parameter classes.
//
// I. Volobouev
// September 2010
//=========================================================================

#ifndef GENERS_CLASSID_HH_
#define GENERS_CLASSID_HH_

#include <vector>
#include <string>
#include <iostream>

namespace gs {
    class ClassId
    {
    public:
        // Generic constructor using a prefix (which is usually
        // a class name) and a version number
        inline ClassId(const char* prefix, const unsigned version,
                       const bool isPtr=false)
            {initialize(prefix, version, isPtr);}

        // Generic constructor using a prefix (which is usually
        // a class name) and a version number
        inline ClassId(const std::string& prefix, const unsigned version,
                       const bool isPtr=false)
            {initialize(prefix.c_str(), version, isPtr);}

        // Use the following constructor in the "classId()" methods
        // of user-developed classes.
        //
        // Implementation note: it is possible to "specialize"
        // this constructor by delegating the actual job to the
        // "ClassIdSpecialization". Then we would be able to create
        // class ids for built-in and user types in a unified
        // way. This, however, would incur a performance hit
        // due to the necessity of making another ClassId and
        // copying the result into the internals of the new object.
        // This performance hit was deemed significant. If you
        // need a universal way to create class ids at some
        // point in your code, use the "itemId" method instead
        // (this may or may not incur a performance hit, depending
        // on what exactly the compiler does).
        template<class T>
        inline ClassId(const T&)
            {initialize(T::classname(), T::version(), false);}

        // Constructor from the class id represented by a string
        explicit ClassId(const std::string& id);

        // Use the following constructor in "read" functions.
        // Dummy argument "reading" is needed in order to generate
        // a distinct function signature (otherwise the templated
        // constructor can win).
        ClassId(std::istream& in, int reading);

        // Use the following pseudo-constructor in static "read"
        // methods in case a type check is desired. It has to be
        // made static because constructor without any arguments
        // can not be a template. Also, this is the way to construct
        // class ids for built-in types (there is no way to specialize
        // member methods).
        template<class T>
        static ClassId makeId();

        // "Universal" item id which also works for built-in types
        template<class T>
        static ClassId itemId(const T&);

        // Inspectors for the class name and version number
        inline const std::string& name() const {return name_;}
        inline unsigned version() const {return version_;}

        // Is this class a pointer for I/O purposes?
        inline bool isPointer() const {return isPtr_;}

        // The following function should return a unique class id string
        // which takes version number into account
        inline const std::string& id() const {return id_;}

        // The following checks if the class name corresponds to
        // a template (using the standard manner of class name forming)
        bool isTemplate() const;

        // The following function fills the vector with class template
        // parameters (if the class is not a template, the vector is
        // cleared). Due to the manner in which things are used in this
        // package, the result is actually a vector of (vectors of size 1).
        void templateParameters(std::vector<std::vector<ClassId> >* p) const;

        // Function to write this object out. Returns "true" on success.
        bool write(std::ostream& of) const;        

        // Comparison operators
        inline bool operator==(const ClassId& r) const
            {return id_ == r.id_;}
        inline bool operator!=(const ClassId& r) const
            {return !(*this == r);}
        inline bool operator<(const ClassId& r) const
            {return id_ < r.id_;}
        inline bool operator>(const ClassId& r) const
            {return id_ > r.id_;}

        // Modify the version number
        void setVersion(unsigned newVersion);

        // The following methods verify that the id/classname/version
        // of this object are equal to those of the argument and throw
        // "std::invalid_argument" exception if this is not so
        void ensureSameId(const ClassId& id) const;
        void ensureSameName(const ClassId& id) const;
        void ensureSameVersion(const ClassId& id) const;

        // The following method ensures that the version number of this
        // class id is within certain range [min, max], with both limits
        // allowed. "std::invalid_argument" exception is thrown if this
        // is not so.
        void ensureVersionInRange(unsigned min, unsigned max) const;

        // Sometimes one really needs to make a placeholder class id...
        // This is a dangerous function: the code using ClassId class
        // will normally assume that a ClassId object is always in a valid
        // state. Invalid class ids can be distinguished by their empty
        // class names (i.e., name().empty() returns "true").
        static ClassId invalidId();

    private:
        ClassId();

        void initialize(const char* prefix, unsigned version, bool isPtr);
        bool makeName();
        bool makeVersion();

        std::string name_;
        std::string id_;
        unsigned version_;
        bool isPtr_;

        // Return "true" if the prefix is valid
        static bool validatePrefix(const char* prefix);
    };


    // Simple class id compatibility checkers for use as policy classes
    // in templated code
    struct SameClassId
    {
        inline static bool compatible(const ClassId& id1, const ClassId& id2)
            {return id1.name() == id2.name();}
    };

    struct SameClassName
    {
        inline static bool compatible(const ClassId& id1, const ClassId& id2)
            {return id1 == id2;}
    };


    // Specialize the following template in order to be able to construct
    // ClassId for classes which do not implement static functions
    // "classname()" and "version()".
    template<class T, bool GenerateClassId=true>
    struct ClassIdSpecialization
    {
        inline static ClassId classId(const bool isPtr=false)
        {
            return ClassId(T::classname(), T::version(), isPtr);
        }
    };

    template<class T>
    struct ClassIdSpecialization<T, false>
    {
        inline static ClassId classId(const bool /* isPtr */=false)
        {
            return ClassId::invalidId();
        }
    };


    // Utility functions for naming template classes. The "nInclude"
    // argument tells us how many template parameters to include into
    // the generated template name. For example, use of
    //
    // template_class_name<X,Y>("myTemplate",1)
    //
    // will generate a class name which looks like myTemplate<X>, with
    // second template parameter omitted. While the result is equivalent
    // to invoking "template_class_name<X>("myTemplate")", having an
    // explicit limit is convenient for use from certain higher-level
    // functions. Note, however, that in the call with two template
    // parameters the class id specialization for Y must be available,
    // even though it is not used.
    //
    // This feature is sometimes helpful when certain template parameters
    // specify aspects of template behavior which have nothing to do
    // with object data contents and I/O. Typical example of such
    // a parameter is std::allocator of STL -- changing this to a custom
    // allocator will not affect serialized representation of an STL
    // container.
    //
    template<class T>
    std::string template_class_name(const char* templateName,
                                    unsigned nInclude=1);
    template<class T>
    std::string template_class_name(const std::string& templateName,
                                    unsigned nInclude=1);
    template<class T1, class T2>
    std::string template_class_name(const char* templateName,
                                    unsigned nInclude=2);
    template<class T1, class T2>
    std::string template_class_name(const std::string& templateName,
                                    unsigned nInclude=2);
    template<class T1, class T2, class T3>
    std::string template_class_name(const char* templateName,
                                    unsigned nInclude=3);
    template<class T1, class T2, class T3>
    std::string template_class_name(const std::string& templateName,
                                    unsigned nInclude=3);
    template<class T1, class T2, class T3, class T4>
    std::string template_class_name(const char* templateName,
                                    unsigned nInclude=4);
    template<class T1, class T2, class T3, class T4>
    std::string template_class_name(const std::string& templateName,
                                    unsigned nInclude=4);
    template<class T1, class T2, class T3, class T4, class T5>
    std::string template_class_name(const char* templateName,
                                    unsigned nInclude=5);
    template<class T1, class T2, class T3, class T4, class T5>
    std::string template_class_name(const std::string& templateName,
                                    unsigned nInclude=5);
    template<class T1, class T2, class T3, class T4, class T5, class T6>
    std::string template_class_name(const char* templateName,
                                    unsigned nInclude=6);
    template<class T1, class T2, class T3, class T4, class T5, class T6>
    std::string template_class_name(const std::string& templateName,
                                    unsigned nInclude=6);

    // Utility functions for naming stack-based containers such as std::array
    template<class T, std::size_t N>
    std::string stack_container_name(const char* templateName);

    template<class T, std::size_t N>
    std::string stack_container_name(const std::string& templateName);
}

#include "geners/ClassId.icc"

#endif // GENERS_CLASSID_HH_
