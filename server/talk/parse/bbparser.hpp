/**
  *  \file server/talk/parse/bbparser.hpp
  *  \brief Class server::talk::parse::BBParser
  */
#ifndef C2NG_SERVER_TALK_PARSE_BBPARSER_HPP
#define C2NG_SERVER_TALK_PARSE_BBPARSER_HPP

#include "afl/container/ptrvector.hpp"
#include "server/talk/inlinerecognizer.hpp"
#include "server/talk/linkparser.hpp"
#include "server/talk/parse/bblexer.hpp"
#include "server/talk/textnode.hpp"

namespace server { namespace talk { namespace parse {

    /** BBCode parser.
        Uses a BBLexer to parse a BBCode document.

        Because a BBParser parses the entire content of a BBLexer,
        it can be used once only. */
    class BBParser {
     public:
        /** Type for a parser warning. */
        enum WarningType {
            SuspiciousText,     ///< Suspicious text: [token].
            MissingClose,       ///< Missing closing tag: [text] before [token].
            TagNotOpen,         ///< Tag not open: [text].
            BadLink,            ///< Bad link: [text].
            NoOwnText           ///< No own text in document.
        };

        /** Parser warning. */
        struct Warning {
            WarningType type;   ///< Type.
            String_t token;     ///< Token at which the warning was detected.
            String_t extra;     ///< Extra information.
            size_t pos;         ///< Position of token in text.

            Warning(WarningType type, const String_t& token, const String_t& extra, size_t pos)
                : type(type), token(token), extra(extra), pos(pos)
                { }
        };

        /** Vector of warnings. */
        typedef std::vector<Warning> Warnings_t;


        /** Constructor.
            \param lex     Lexer
            \param recog   InlineRecognizer instance
            \param options Items to recognize by InlineRecognizer
            \param lp      LinkParser instance */
        BBParser(BBLexer& lex, InlineRecognizer& recog, InlineRecognizer::Kinds_t options, const LinkParser& lp);

        /** Parse document.
            \return newly-allocated document */
        TextNode* parse();

        /** Access warnings.
            \return Array of warnings. */
        const Warnings_t& warnings() const;

        /** Check for known tag.
            \param tag Tag name
            \return true if this tag is known to BBParser */
        static bool isKnownTag(const String_t& tag);

     private:
        typedef size_t depth_t;

        BBLexer& lex;
        InlineRecognizer m_recognizer;
        const LinkParser& m_linkParser;
        InlineRecognizer::Kinds_t options;
        BBLexer::Token current;
        afl::container::PtrVector<TextNode> stack;
        Warnings_t m_warnings;

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
        void closeAndWarn();
        bool inLink() const;

        void checkLink(TextNode& node);
        void addWarning(WarningType type, const String_t& extra);
    };

} } }

inline const server::talk::parse::BBParser::Warnings_t&
server::talk::parse::BBParser::warnings() const
{
    return m_warnings;
}

#endif
