/**
  *  \file util/rich/parser.hpp
  */
#ifndef C2NG_UTIL_RICH_PARSER_HPP
#define C2NG_UTIL_RICH_PARSER_HPP

#include "afl/io/xml/basereader.hpp"

namespace util { namespace rich {

    class Text;

    /** Rich text parser, basic version.
        This implements parsing of flow-text markup from XML. */
    class Parser {
     public:
        explicit Parser(afl::io::xml::BaseReader& rdr);

        // Utilities
        void readNext();
        bool isOpeningTag(const char* what);
        void skipTag();

        // Parser
        Text parseText(bool keepFormat);
        Text parseTextItem(bool keepFormat);
        Text parse();

        static void appendText(Text& out, bool& haveSpace, const String_t& in);
        static Text renderKeys(const String_t& name);

        static Text parseXml(String_t source);

     private:
        afl::io::xml::BaseReader& m_reader;
        afl::io::xml::BaseReader::Token m_currentToken;
    };

} }

#endif
