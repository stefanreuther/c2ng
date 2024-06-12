/**
  *  \file util/syntax/factory.hpp
  *  \brief Class util::syntax::Factory
  */
#ifndef C2NG_UTIL_SYNTAX_FACTORY_HPP
#define C2NG_UTIL_SYNTAX_FACTORY_HPP

#include "afl/base/deleter.hpp"
#include "afl/string/string.hpp"

namespace util { namespace syntax {

    class KeywordTable;
    class Highlighter;

    /** Highlighter factory. */
    class Factory {
     public:
        /** Constructor.
            \param tab Keyword table. Must live as long as this factory and all highlighters derived from it. */
        explicit Factory(const KeywordTable& tab);

        /** Create highlighter.
            \param name File or language name hint
            \param del  Deleter. Will manage the new highlighter and its depending objects. */
        Highlighter& create(String_t name, afl::base::Deleter& del);

     private:
        const KeywordTable& m_table;
    };

} }

#endif
