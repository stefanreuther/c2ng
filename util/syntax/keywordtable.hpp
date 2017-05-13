/**
  *  \file util/syntax/keywordtable.hpp
  */
#ifndef C2NG_UTIL_SYNTAX_KEYWORDTABLE_HPP
#define C2NG_UTIL_SYNTAX_KEYWORDTABLE_HPP

#include <map>
#include "afl/io/stream.hpp"
#include "afl/sys/loglistener.hpp"
#include "afl/string/string.hpp"

namespace util { namespace syntax {

    class KeywordTable {
     public:
        KeywordTable();
        ~KeywordTable();

        void load(afl::io::Stream& in, afl::sys::LogListener& log);

        void add(const String_t& key, const String_t& value);
        const String_t* get(const String_t& key) const;

     private:
        std::map<String_t, String_t> m_data;
    };

} }

#endif
