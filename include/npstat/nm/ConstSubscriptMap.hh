#ifndef NPSTAT_CONSTSUBSCRIPTMAP_HH_
#define NPSTAT_CONSTSUBSCRIPTMAP_HH_

/*!
// \file ConstSubscriptMap.hh
//
// \brief A variation of std::map with const subscripting operator
//
// Author: I. Volobouev
//
// August 2012
*/

#include <map>
#include <stdexcept>

namespace npstat {
    template <class Key, class T,
              class Compare = std::less<Key>,
              class Allocator = std::allocator<std::pair<const Key,T> > >
    struct ConstSubscriptMap : public std::map<Key,T,Compare,Allocator>
    {
        inline T& operator[](const Key&);
        inline const T& operator[](const Key&) const;
    };

    template<class Key,class T,class Compare,class Allocator>
    inline T& ConstSubscriptMap<Key,T,Compare,Allocator>::operator[](const Key& key)
    {
        typename ConstSubscriptMap<Key,T,Compare,Allocator>::const_iterator it = this->find(key);
        if (it == std::map<Key,T,Compare,Allocator>::end()) throw std::invalid_argument(
            "In npstat::ConstSubscriptMap::operator[]: key not found");
        return const_cast<T&>(it->second);
    }

    template<class Key,class T,class Compare,class Allocator>
    inline const T& ConstSubscriptMap<Key,T,Compare,Allocator>::operator[](const Key& key) const
    {
        typename ConstSubscriptMap<Key,T,Compare,Allocator>::const_iterator it = this->find(key);
        if (it == std::map<Key,T,Compare,Allocator>::end()) throw std::invalid_argument(
            "In npstat::ConstSubscriptMap::operator[]: key not found");
        return it->second;
    }
}

#endif // NPSTAT_CONSTSUBSCRIPTMAP_HH_
