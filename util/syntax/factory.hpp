/**
  *  \file util/syntax/factory.hpp
  */
#ifndef C2NG_UTIL_SYNTAX_FACTORY_HPP
#define C2NG_UTIL_SYNTAX_FACTORY_HPP

#include "afl/base/deleter.hpp"
#include "afl/string/string.hpp"

namespace util { namespace syntax {

    class KeywordTable;
    class Highlighter;

    // FIXME: make this a virtual interface?
    class Factory {
     public:
        Factory(KeywordTable& tab);

        Highlighter& create(String_t name, afl::base::Deleter& del);

     private:
        KeywordTable& m_table;
    };

} }

#endif
