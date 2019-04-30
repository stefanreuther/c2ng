/**
  *  \file util/rich/parser.hpp
  *  \brief Class util::rich::Parser
  */
#ifndef C2NG_UTIL_RICH_PARSER_HPP
#define C2NG_UTIL_RICH_PARSER_HPP

#include "afl/io/xml/basereader.hpp"

namespace util { namespace rich {

    class Text;

    /** Rich text parser, basic version.
        This implements parsing of flow-text markup from XML.
        Parsers for superset markup can build on it. */
    class Parser {
     public:
        /** Constructor.
            \param rdr XML lexer */
        explicit Parser(afl::io::xml::BaseReader& rdr);

        /*
         *  Utilities
         */

        /** Advance to next tag. */
        void readNext();

        /** Check for opening tag.
            If that tag is found, skips it from the token stream.
            \param what Tag to check for (case sensitive!)
            \retval true Opening tag was found and skipped
            \retval false Mismatch; nothing read */
        bool isOpeningTag(const char* what);

        /** Skip a tag.
            Skips the tag itself and all its content up to the matching closing tag.
            Called while looking at the opening tag. */
        void skipTag();

        /*
         *  Parser
         */

        /** Parse text sequence.
            Parses a list of text elements until it encounters a closing tag or end of input.
            This parses the tags covered by parseTextItem().
            \param keepFormat true to keep format (inside <pre>), false to rewrap (outside <pre>)
            \return parsed text */
        Text parseText(bool keepFormat);

        /** Parse text element.
            Parses a single text tag.
            Must be called while looking at an opening tag.
            This parses the tags: a, b, em, u, tt, kbd/key, big, small, font, align.
            \param keepFormat true to keep format (inside <pre>), false to rewrap (outside <pre>)
            \return parsed content */
        Text parseTextItem(bool keepFormat);

        /** Parse text.
            Simple all-in-one function.
            This one parses the tags covered by parseTextItem() as well as br.
            \return parsed content */
        Text parse();

        /** Access reader.
            \return Reader this was constructed from. */
        afl::io::xml::BaseReader& reader();

        /** Get current token.
            This is the token that will be consumed next.
            \return Current token */
        afl::io::xml::BaseReader::Token getCurrentToken() const;

        /** Append string to Text.
            This collapses multiple spaces into one, and strips newlines.
            \param out       [in/out] Text to append to
            \param haveSpace [in/out] Space status tracking
            \param in        [in] Text to append */
        static void appendText(Text& out, bool& haveSpace, const String_t& in);

        /** Render keys.
            This splits the given key string ("Alt+X") into multiple StyleAttribute::Key elements.
            \param name Key string
            \return Text */
        static Text renderKeys(const String_t& name);

        /** Parse string.
            This is a simple all-in-one wrapper for parseText(true).
            \param source Input string
            \return parsed text */
        static Text parseXml(String_t source);

     private:
        afl::io::xml::BaseReader& m_reader;
        afl::io::xml::BaseReader::Token m_currentToken;
    };

} }

#endif
