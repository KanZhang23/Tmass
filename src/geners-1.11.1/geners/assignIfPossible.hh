#ifndef GENERS_ASSIGNIFPOSSIBLE_HH_
#define GENERS_ASSIGNIFPOSSIBLE_HH_

// This header will work only with C++11

#include <type_traits>

namespace gs {
    namespace Private {
        template<
            typename A,
            typename B,
            bool assignable = std::is_assignable<A&, const B&>::value
        >
        struct AssignmentHelper
        {
            inline static bool assign(A&, const B&) {return false;}
        };

        template<typename A, typename B>
        struct AssignmentHelper<A, B, true>
        {
            inline static bool assign(A& a, const B& b) {a = b; return true;}
        };
    }

    template <typename A, typename B>
    inline bool assignIfPossible(A& a, const B& b)
    {
        return Private::AssignmentHelper<A,B>::assign(a, b);
    }
}

#endif // GENERS_ASSIGNIFPOSSIBLE_HH_
