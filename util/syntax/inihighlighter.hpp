/**
  *  \file util/syntax/inihighlighter.hpp
  */
#ifndef C2NG_UTIL_SYNTAX_INIHIGHLIGHTER_HPP
#define C2NG_UTIL_SYNTAX_INIHIGHLIGHTER_HPP

#include "util/syntax/highlighter.hpp"

namespace util { namespace syntax {

    class KeywordTable;

    /** Syntax highlighter for INI files.
        This highlights files like pconfig.src, explmap.cfg, amaster.src, ... */
    class IniHighlighter : public Highlighter {
     public:
        IniHighlighter(const KeywordTable& tab, String_t defaultSection);
        ~IniHighlighter();

        virtual void init(afl::string::ConstStringMemory_t text);
        virtual bool scan(Segment& result);

     private:
        const KeywordTable& m_table;
        String_t m_section;
        afl::string::ConstStringMemory_t m_text;

        enum State {
            sBOL,
            sAfterSection,
            sAfterName
        };
        State m_state;
    };

} }

#endif
