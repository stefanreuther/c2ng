/**
  *  \file server/talk/parse/bblexer.hpp
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

        BBLexer(const String_t& text);

        Token read();

        Token    getTokenType() const;
        String_t getTokenString() const;
        String_t getTag() const;
        String_t getAttribute() const;

        void skipBlanks();

     private:
        bool findNewline();

        String_t text;
        String_t::size_type cursor;
        String_t::size_type token_start;
        String_t::size_type token_length;
        String_t::size_type attr_start;
        String_t::size_type attr_length;
        String_t tag;
        Token token;
    };

} } }

#endif
