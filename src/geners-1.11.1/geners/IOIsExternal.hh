#ifndef GENERS_IOISEXTERNAL_HH_
#define GENERS_IOISEXTERNAL_HH_

namespace gs {
    template <class T>
    struct IOIsExternal
    {
        enum {value = 0};
    };
}

// Use the following macro (outside of any namespace)
// to declare some type as external for I/O purposes
//
#define gs_declare_type_external(T) /**/                                   \
namespace gs {                                                             \
    template <> struct IOIsExternal<T> {enum {value = 1};};                \
    template <> struct IOIsExternal<T const> {enum {value = 1};};          \
    template <> struct IOIsExternal<T volatile> {enum {value = 1};};       \
    template <> struct IOIsExternal<T const volatile> {enum {value = 1};}; \
}

// Use the following macro (outside of any namespace)
// to declare some template parameterized by one argument
// as external for I/O purposes
//
#define gs_declare_template_external_T(name) /**/                          \
namespace gs {                                                             \
    template <class T> struct IOIsExternal< name <T> >                     \
       {enum {value = 1};};                                                \
    template <class T> struct IOIsExternal<const name <T> >                \
       {enum {value = 1};};                                                \
    template <class T> struct IOIsExternal<volatile name <T> >             \
       {enum {value = 1};};                                                \
    template <class T> struct IOIsExternal<const volatile name <T> >       \
       {enum {value = 1};};                                                \
}

// Use the following macro (outside of any namespace)
// to declare some template parameterized by two arguments
// as external for I/O purposes
//
#define gs_declare_template_external_TT(name) /**/                            \
namespace gs {                                                                \
    template <class T,class U> struct IOIsExternal< name <T,U> >              \
       {enum {value = 1};};                                                   \
    template <class T,class U> struct IOIsExternal<const name <T,U> >         \
       {enum {value = 1};};                                                   \
    template <class T,class U> struct IOIsExternal<volatile name <T,U> >      \
       {enum {value = 1};};                                                   \
    template <class T,class U> struct IOIsExternal<const volatile name <T,U> >\
       {enum {value = 1};};                                                   \
}

// Use the following macro (outside of any namespace)
// to declare some template parameterized by three arguments
// as external for I/O purposes
//
#define gs_declare_template_external_TTT(name) /**/   \
namespace gs {                                        \
    template <class T,class U,class V>                \
    struct IOIsExternal< name <T,U,V> >               \
       {enum {value = 1};};                           \
    template <class T,class U,class V>                \
    struct IOIsExternal<const name <T,U,V> >          \
       {enum {value = 1};};                           \
    template <class T,class U,class V>                \
    struct IOIsExternal<volatile name <T,U,V> >       \
       {enum {value = 1};};                           \
    template <class T,class U,class V>                \
    struct IOIsExternal<const volatile name <T,U,V> > \
       {enum {value = 1};};                           \
}

// Use the following macro (outside of any namespace)
// to declare some template parameterized by four arguments
// as external for I/O purposes
//
#define gs_declare_template_external_TTTT(name) /**/    \
namespace gs {                                          \
    template <class T,class U,class V,class X>          \
    struct IOIsExternal< name <T,U,V,X> >               \
       {enum {value = 1};};                             \
    template <class T,class U,class V,class X>          \
    struct IOIsExternal<const name <T,U,V,X> >          \
       {enum {value = 1};};                             \
    template <class T,class U,class V,class X>          \
    struct IOIsExternal<volatile name <T,U,V,X> >       \
       {enum {value = 1};};                             \
    template <class T,class U,class V,class X>          \
    struct IOIsExternal<const volatile name <T,U,V,X> > \
       {enum {value = 1};};                             \
}

// Use the following macro (outside of any namespace)
// to declare some template parameterized by five arguments
// as external for I/O purposes
//
#define gs_declare_template_external_TTTTT(name) /**/     \
namespace gs {                                            \
    template <class T,class U,class V,class X,class Y>    \
    struct IOIsExternal< name <T,U,V,X,Y> >               \
       {enum {value = 1};};                               \
    template <class T,class U,class V,class X,class Y>    \
    struct IOIsExternal<const name <T,U,V,X,Y> >          \
       {enum {value = 1};};                               \
    template <class T,class U,class V,class X,class Y>    \
    struct IOIsExternal<volatile name <T,U,V,X,Y> >       \
       {enum {value = 1};};                               \
    template <class T,class U,class V,class X,class Y>    \
    struct IOIsExternal<const volatile name <T,U,V,X,Y> > \
       {enum {value = 1};};                               \
}

// Use the following macro (outside of any namespace)
// to declare some template parameterized by six arguments
// as external for I/O purposes
//
#define gs_declare_template_external_TTTTTT(name) /**/         \
namespace gs {                                                 \
    template <class T,class U,class V,class X,class Y,class Z> \
    struct IOIsExternal< name <T,U,V,X,Y,Z> >                  \
       {enum {value = 1};};                                    \
    template <class T,class U,class V,class X,class Y,class Z> \
    struct IOIsExternal<const name <T,U,V,X,Y,Z> >             \
       {enum {value = 1};};                                    \
    template <class T,class U,class V,class X,class Y,class Z> \
    struct IOIsExternal<volatile name <T,U,V,X,Y,Z> >          \
       {enum {value = 1};};                                    \
    template <class T,class U,class V,class X,class Y,class Z> \
    struct IOIsExternal<const volatile name <T,U,V,X,Y,Z> >    \
       {enum {value = 1};};                                    \
}

#endif // GENERS_IOISEXTERNAL_HH_
