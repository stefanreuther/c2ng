/**
  *  \file util/syntax/scripthighlighter.hpp
  */
#ifndef C2NG_UTIL_SYNTAX_SCRIPTHIGHLIGHTER_HPP
#define C2NG_UTIL_SYNTAX_SCRIPTHIGHLIGHTER_HPP

#include "util/syntax/highlighter.hpp"

namespace util { namespace syntax {

    class KeywordTable;

    // /** Highlighter for CCScript (*.q files). */
    class ScriptHighlighter : public Highlighter {
     public:
        // FIXME: what do we need the KeywordTable for? PlanetsCentral references it, but does not use it
        ScriptHighlighter(const KeywordTable& table);
        ~ScriptHighlighter();

        virtual void init(afl::string::ConstStringMemory_t text);
        virtual bool scan(Segment& result);

     private:
        const KeywordTable& m_table;
        afl::string::ConstStringMemory_t m_text;

        enum State {
            sDefaultBOL,            // standard highlighting, beginning of line (=detection of keywords)
            sDefault,               // standard highlighting, within statement
            sAfterSub,              // I have seen "Sub" or "Function"
            sAfterSubDef,           // I have seen a parameter name, and expect a comma for the next one
            sAfterDim,              // I have seen "Dim" or "Local"
            sAfterDimDef,           // I have seen "Dim" or "Local", and expect a comma for the next definition
            sAfterFor,              // I have seen "For", "To" now is a keyword
            sAfterLoop,             // I have seen "Do" or "Loop", "Until" now is a keyword
            sAfterIf,               // I have seen "If", "Then" now is a keyword
            sAfterCase,             // I have seen "Case", "Is" now is a keyword
            sAfterWith              // I have seen "With" or "On"
        };
        State m_state;
        int m_parenLevel;

        void leaveDefault();
    };

} }

#endif
