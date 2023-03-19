/**
  *  \file util/stringlist.hpp
  *  \brief Class util::StringList
  */
#ifndef C2NG_UTIL_STRINGLIST_HPP
#define C2NG_UTIL_STRINGLIST_HPP

#include <vector>
#include <utility>
#include "afl/base/optional.hpp"
#include "afl/string/string.hpp"

namespace util {

    /** Ordered key/string pair list
        This provides a container of pairs of keys (integers) and strings,
        to be used for list boxes and similar things. */
    class StringList {
     public:
        /** Constructor.
            Makes an empty list. */
        StringList();

        /** Destructor. */
        ~StringList();

        /** Add a key/string pair.
            \param key Key
            \param s   String */
        void add(int32_t key, const String_t& s);

        /** Sort contents alphabetically. */
        void sortAlphabetically();

        /** Swap content with another list.
            \param other Other list */
        void swap(StringList& other);

        /** Clear list. */
        void clear();

        /** Get number of key/string pairs.
            \return number */
        size_t size() const;

        /** Check emptiness.
            \return true if empty */
        bool empty() const;

        /** Get key/string pair, given an index.
            \param [in]  index  Index [0, size())
            \param [out] key    Key
            \param [out] s      String
            \retval true Index valid; key/s have been set
            \retval false Index out of range; key/s not modified */
        bool get(size_t index, int32_t& key, String_t& s) const;

        /** Find key.
            \param key    Key to find
            \return Found index, if any
            \retval true Key found; index has been set
            \retval false Key not found; index unchanged */
        afl::base::Optional<size_t> find(int32_t key) const;

     private:
        typedef std::pair<int32_t, String_t> Element_t;
        std::vector<Element_t> m_data;
    };

}

#endif
