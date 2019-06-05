#ifndef GENERS_BOOLEANSTRING_HH_
#define GENERS_BOOLEANSTRING_HH_

namespace gs {
    template<bool isTrue=false>
    struct BooleanString
    {
        static const bool value = false;
        static const char* name() {return "false";}
    };

    template<>
    struct BooleanString<true>
    {
        static const bool value = true;
        static const char* name() {return "true";}
    };
}

#endif // GENERS_BOOLEANSTRING_HH_
