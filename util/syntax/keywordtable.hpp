/**
  *  \file util/syntax/keywordtable.hpp
  *  \brief Class util::syntax::KeywordTable
  */
#ifndef C2NG_UTIL_SYNTAX_KEYWORDTABLE_HPP
#define C2NG_UTIL_SYNTAX_KEYWORDTABLE_HPP

#include <map>
#include "afl/io/stream.hpp"
#include "afl/sys/loglistener.hpp"
#include "afl/string/string.hpp"

namespace util { namespace syntax {

    /** Keyword table for syntax highlighting.
        Stores a key/value mapping that describes keywords,
        and provides a method to load that from a file. */
    class KeywordTable {
     public:
        /** Default constructor.
            Makes an empty mapping. */
        KeywordTable();

        /** Destructor. */
        ~KeywordTable();

        /** Load syntaxdb.txt file.
            Each line in the file has the form
            - "key=value" (set a key/value pair)
            - "key=$key" (copy from previous key/value pair)
            - "keyprefix {" (prefix for all following "key=value" pairs)
            - "}" (cancel last keyprefix)
            Lines starting with ";" or "#" are comments.
            \param in File
            \parm log Logger */
        void load(afl::io::Stream& in, afl::sys::LogListener& log);

        /** Add or replace key.
            \param key Key
            \param value Value */
        void add(const String_t& key, const String_t& value);

        /** Get value.
            \param key Key
            \return pointer to value if exists, otherwise null */
        const String_t* get(const String_t& key) const;

     private:
        std::map<String_t, String_t> m_data;
    };

} }

#endif
