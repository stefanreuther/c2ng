/**
  *  \file server/talk/parse/bbparser.hpp
  *  \brief Class server::talk::parse::BBParser
  */
#ifndef C2NG_SERVER_TALK_PARSE_BBPARSER_HPP
#define C2NG_SERVER_TALK_PARSE_BBPARSER_HPP

#include "server/talk/textnode.hpp"
#include "server/talk/inlinerecognizer.hpp"
#include "server/talk/parse/bblexer.hpp"
#include "afl/container/ptrvector.hpp"

namespace server { namespace talk { namespace parse {

    /** BBCode parser.
        Uses a BBLexer to parse a BBCode document.

        Because a BBParser parses the entire content of a BBLexer,
        it can be used once only. */
    class BBParser {
     public:
        /** Constructor.
            \param lex     Lexer
            \param recog   InlineRecognizer instance
            \param options Items to recognize by InlineRecognizer */
        BBParser(BBLexer& lex, InlineRecognizer& recog, InlineRecognizer::Kinds_t options);

        /** Parse document.
            \return newly-allocated document */
        TextNode* parse();

        /** Check for known tag.
            \param tag Tag name
            \return true if this tag is known to BBParser */
        static bool isKnownTag(const String_t& tag);

     private:
        typedef size_t depth_t;

        BBLexer& lex;
        InlineRecognizer m_recognizer;
        InlineRecognizer::Kinds_t options;
        BBLexer::Token current;
        afl::container::PtrVector<TextNode> stack;

        void handleStart();
        void handleEnd();
        void handleSmiley();
        void handleParagraph();
        void handleAtLink();
        void handleText();

        void handleNoparse();
        void handleCode();
        void handleListItem();

        void next();
        void open(TextNode::MajorKind major, uint8_t minor);
        void openAt(depth_t n, TextNode::MajorKind major, uint8_t minor);
        void appendText(String_t what);
        void close();
        void close(depth_t n);
        void closeLinks();
        void closeInline();
        bool closeUntil(uint8_t major, uint8_t minor);
        bool inLink() const;
    };

} } }

#endif
