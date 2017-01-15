/**
  *  \file util/stringlist.hpp
  */
#ifndef C2NG_UTIL_STRINGLIST_HPP
#define C2NG_UTIL_STRINGLIST_HPP

#include <vector>
#include <utility>
#include "afl/string/string.hpp"

namespace util {

    // /** \class StringList
    //     \brief Ordered key/string pair list

    //     This provides a container of pairs of keys (integers) and strings,
    //     to be used for list boxes and similar things. */
    class StringList {
     public:
        StringList();
        ~StringList();

        // Adding
        void add(int32_t key, const String_t& s);

        // Manipulation
        void sortAlphabetically();
        void swap(StringList& other);

        // Random access
        size_t size() const;
        bool get(size_t index, int32_t& key, String_t& s) const;
        bool find(int32_t key, size_t& index) const;

     private:
        typedef std::pair<int32_t, String_t> Element_t;
        std::vector<Element_t> m_data;
    };

}

#endif
