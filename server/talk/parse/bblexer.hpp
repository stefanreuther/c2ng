/**
  *  \file server/talk/parse/bblexer.hpp
  *  \brief Class server::talk::parse::BBLexer
  */
#ifndef C2NG_SERVER_TALK_PARSE_BBLEXER_HPP
#define C2NG_SERVER_TALK_PARSE_BBLEXER_HPP

#include "afl/string/string.hpp"

namespace server { namespace talk { namespace parse {

    /** BBCode lexer. This splits a BBCode document into tokens, i.e. tags and text.
        Specifically,
        - "[foo]" and "[foo=bar]" are tags, where "foo" must entirely consist of letters,
          with the special exception of "[*]" for list items. Letter tags are normalized
          to lower-case. For letter tags, a single attribute can be specified with an
          equals sign; if none is given, it is reported as the empty string. The attribute
          can optionally be put inside double-quotes.
        - "[/foo]" is a closing tag.
        - double-newlines are paragraph breaks; there are no provisions for leaving
          more vertical room.
        - "@foo" is parsed as a special token type, AtLink, to allow converting user
          names into links. "foo" must consist entirely of identifier characters, i.e.
          letters, digits, underscores, as permitted in PCc user names.
        - anything else is text, including partial forms of syntax (e.g. "[foo", "[/]",
          or "[*foo*]"). BBCode cannot produce syntax errors. Note that text can be
          arbitrarily split into tokens, not necessarily only at word boundaries. */
    class BBLexer {
     public:
        enum Token {
            Eof,                    ///< End of input
            TagStart,               ///< "[foo]" or "[foo=bar]"
            TagEnd,                 ///< "[/foo]"
            Smiley,                 ///< "[:smile:]" (string is "smile")
            Paragraph,              ///< Paragraph break, i.e.\ double-newline
            AtLink,                 ///< "@foo"
            Text                    ///< anything else
        };

        /** Constructor.
            \param text Text to parse */
        BBLexer(const String_t& text);

        /** Read a token.
            \return token type. */
        Token read();

        /** Get current token.
            \return current token type, i.e. last value returned by read() */
        Token getTokenType() const;

        /** Get current token string.
            \return complete token text */
        String_t getTokenString() const;

        /** Get current tag. For TagStart and TagEnd only.
            \return tag, in lower-case */
        String_t getTag() const;

        /** Get current attribute. For TagStart only.
            \return attribute */
        String_t getAttribute() const;

        /** Skip blanks.
            Advances cursor until it sits at the end or a non-blank. */
        void skipBlanks();

     private:
        bool findNewline();

        String_t m_text;
        String_t::size_type m_cursor;
        String_t::size_type m_tokenStart;
        String_t::size_type m_tokenLength;
        String_t::size_type m_attributeStart;
        String_t::size_type m_attributeLength;
        String_t m_tag;
        Token m_token;
    };

} } }

#endif
