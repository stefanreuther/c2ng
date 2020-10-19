/**
  *  \file util/syntax/inihighlighter.hpp
  *  \brief Class util::syntax::IniHighlighter
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
        /** Constructor.
            \param tab Keyword table. Must live as long as this highlighter.
            \param defaultSection Default section name */
        IniHighlighter(const KeywordTable& tab, String_t defaultSection);

        /** Destructor. */
        ~IniHighlighter();

        // Highlighter:
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
