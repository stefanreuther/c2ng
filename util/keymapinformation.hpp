/**
  *  \file util/keymapinformation.hpp
  *  \brief Class util::KeymapInformation
  */
#ifndef C2NG_UTIL_KEYMAPINFORMATION_HPP
#define C2NG_UTIL_KEYMAPINFORMATION_HPP

#include <vector>
#include <utility>
#include "afl/string/string.hpp"

namespace util {

    /** Preformatted information about a keymap's inheritance hierarchy.
        Represents a list of
        - keymap names
        - level (=depth in inheritance tree)

        This is a data object that can be passed between threads. */
    class KeymapInformation {
     public:
        typedef size_t Index_t;

        static const Index_t nil = Index_t(-1);

        /** Constructor.
            Make an empty object. */
        KeymapInformation();

        /** Destructor. */
        ~KeymapInformation();

        /** Clear.
            @post size() == 0 */
        void clear();

        /** Get number of items.
            @return number */
        size_t size() const;

        /** Add an item.
            @param level Level
            @param name  Name */
        void add(size_t level, const String_t& name);

        /** Get an item, given an index.
            @param [in]  index   Index [0,size())
            @param [out] level   Level
            @param [out] name    Name
            @return true on success, false if index is out of range */
        bool get(Index_t index, size_t& level, String_t& name) const;

        /** Find an item, given a name.
            @param name  Name
            @return index; nil if not found */
        Index_t find(const String_t& name) const;

     private:
        std::vector<std::pair<size_t, String_t> > m_data;
    };

}

#endif
