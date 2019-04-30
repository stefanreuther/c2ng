/**
  *  \file util/keymapinformation.hpp
  */
#ifndef C2NG_UTIL_KEYMAPINFORMATION_HPP
#define C2NG_UTIL_KEYMAPINFORMATION_HPP

#include <vector>
#include <utility>
#include "afl/string/string.hpp"

namespace util {

    class KeymapInformation {
     public:
        typedef size_t Index_t;

        static const Index_t nil = Index_t(-1);

        KeymapInformation();
        ~KeymapInformation();

        void clear();
        size_t size() const;
        void add(size_t level, const String_t& name);
        bool get(Index_t index, size_t& level, String_t& name) const;
        Index_t find(const String_t& name) const;

     private:
        std::vector<std::pair<size_t, String_t> > m_data;
    };

}

#endif
