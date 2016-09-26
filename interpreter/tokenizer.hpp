/**
  *  \file interpreter/tokenizer.hpp
  */
#ifndef C2NG_INTERPRETER_TOKENIZER_HPP
#define C2NG_INTERPRETER_TOKENIZER_HPP

#include "afl/string/string.hpp"
#include "afl/base/types.hpp"

namespace interpreter {
    /** Tokenizer.
        This class can split a line into CCScript tokens.
        It roughly corresponds to PCC 1.1.17's tokenizer with the following exceptions:
        - unterminated strings are errors
        - identifiers can start with '_' or '$' (in PCC 1.x, they can only start with letters), but cannot end with dots.
        - 2147483647 is a valid integer constant

        Summarized, rules are:
        - whitespace separates tokens but is otherwise ignored
        - '%' starts a comment to end of line
        - decimal numeric literals are permitted, with decimal point, but no fancy stuff like hex or '1.0e+12'.
          Integer literals are automatically turned into floats if they leave 32-bit range.
        - identifiers can contain letters, digits, '_', '$' and '.', but cannot start with a digit or '.', and cannot end with '.'.
          They are converted into upper-case.
          The special keywords 'TRUE' and 'FALSE' turn into boolean literals, 'PI' is a float literal, and 'AND', 'OR', 'XOR', 'MOD', 'NOT' are operators.
        - strings can be delimited by apostrophes or double-quotes.
          Within a double-quoted string, the backslash can be used to quote the next character, to include a double-quote or backslash in a string.
        - other characters or character pairs are operator tokens, or invalid. */
    class Tokenizer {
     public:
        /// Token type.
        enum TokenType {
            // Specials:
            tEnd,                   // End of input
            tInvalid,               // Invalid character (character obtainable in String Value)

            // Literals:
            tInteger,               // Integer literal
            tFloat,                 // Float literal
            tString,                // String literal
            tBoolean,               // Boolean literal

            // Identifiers and Reserved Words:
            tIdentifier,            // Identifier
            tAND,
            tOR,
            tXOR,
            tNOT,
            tMOD,

            // Character Pairs:
            tNE,                    // <>
            tGE,                    // >=
            tLE,                    // <=
            tAssign,                // :=
            tArrow,                 // ->

            // Single-Character Tokesn:
            tAmpersand,
            tHash,
            tPlus,
            tMinus,
            tMultiply,
            tSlash,                 // real division
            tBackslash,             // integer division
            tCaret,                 // power
            tLParen,
            tRParen,
            tComma,
            tEQ,
            tLT,
            tGT,
            tColon,
            tSemicolon,
            tDot
        };

        /** Constructor.
            \param str String to tokenize
            \throw Error if the string immediately fails to parse */
        Tokenizer(const String_t& str);

        /** Destructor. */
        ~Tokenizer();

        TokenType       getCurrentToken() const;
        int32_t         getCurrentInteger() const;
        const String_t& getCurrentString() const;
        double          getCurrentFloat() const;

        String_t        getRemainingLine() const;

        TokenType readNextToken();

        bool checkAdvance(TokenType t);
        bool checkAdvance(const char* keyword);

     private:
        String_t             m_line;
        String_t::size_type  m_pos;

        TokenType m_currentToken;
        String_t  m_currentString;
        int32_t   m_currentInteger;
        double    m_currentFloat;

        void read();
        void readNumber();
    };

}

// /** Get type of current (most recently read) token. */
inline interpreter::Tokenizer::TokenType
interpreter::Tokenizer::getCurrentToken() const
{
    return m_currentToken;
}

// /** Get integer value of current (most recently read) token.
//     This is the value associated with a tInteger or tBoolean token. */
inline int32_t
interpreter::Tokenizer::getCurrentInteger() const
{
    return m_currentInteger;
}

// /** Get string value of current (most recently read) token.
//     This is the value associated with a tString token, or the name of a tIdentifier token. */
inline const String_t&
interpreter::Tokenizer::getCurrentString() const
{
    return m_currentString;
}

// /** Get float value of current (most recently read) token.
//     This is the value associated with a tFloat token. */
inline double
interpreter::Tokenizer::getCurrentFloat() const
{
    return m_currentFloat;
}

// /** Get remaining unparsed line. */
inline String_t
interpreter::Tokenizer::getRemainingLine() const
{
    return m_line.substr(m_pos);
}

// /** Read next token.
//     \return type of token read */
inline interpreter::Tokenizer::TokenType
interpreter::Tokenizer::readNextToken()
{
    read();
    return getCurrentToken();
}

#endif
