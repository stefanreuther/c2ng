/**
  *  \file interpreter/tokenizer.hpp
  *  \brief Class interpreter::Tokenizer
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
        - other characters or character pairs are operator tokens, or invalid.

        Tokens are identified by a TokenType and a parameter depending on the type. */
    class Tokenizer {
     public:
        /// Token type.
        enum TokenType {
            // Specials:
            tEnd,                   ///< End of input.
            tInvalid,               ///< Invalid character (character obtainable in String Value).

            // Literals:
            tInteger,               ///< Integer literal. \see getCurrentInteger().
            tFloat,                 ///< Float literal. \see getCurrentFloat().
            tString,                ///< String literal. \see getCurrentString().
            tBoolean,               ///< Boolean literal. \see getCurrentInteger().

            // Identifiers and Reserved Words:
            tIdentifier,            ///< Identifier. \see getCurrentString().
            tAND,                   ///< "AND" keyword.
            tOR,                    ///< "OR" keyword.
            tXOR,                   ///< "XOR" keyword.
            tNOT,                   ///< "NOT" keyword.
            tMOD,                   ///< "MOD" keyword.

            // Character Pairs:
            tNE,                    ///< "<>" digraph.
            tGE,                    ///< ">=" digraph.
            tLE,                    ///< "<=" digraph.
            tAssign,                ///< ":=" digraph.
            tArrow,                 ///< "->" digraph.

            // Single-Character Tokens:

            tAmpersand,             ///< "&" (concatenation).
            tHash,                  ///< "#" (concatenation, file handle).
            tPlus,                  ///< "+" (addition).
            tMinus,                 ///< "-" (subtraction).
            tMultiply,              ///< "*" (multiplication).
            tSlash,                 ///< "/" (real division).
            tBackslash,             ///< "\" (integer division).
            tCaret,                 ///< "^" (power).
            tLParen,                ///< "("
            tRParen,                ///< ")"
            tComma,                 ///< ","
            tEQ,                    ///< "="
            tLT,                    ///< "<"
            tGT,                    ///< ">"
            tColon,                 ///< ":" (not currently in use, but part of ":=")
            tSemicolon,             ///< ";" (sequence)
            tDot                    ///< "." (mostly synonymous to "->")
        };

        /** Constructor.
            This will immediately read the first token.
            \param str String to tokenize
            \throw Error if the string immediately fails to parse */
        explicit Tokenizer(const String_t& str);

        /** Destructor. */
        ~Tokenizer();

        /** Get most recently read token type.
            \return token type */
        TokenType getCurrentToken() const;

        /** Get integer value of current (most recently read) token.
            This is the value associated with a tInteger or tBoolean token.
            \return value */
        int32_t getCurrentInteger() const;

        /** Get string value of the current (most recently read) token.
            This is the value associated with a tString token, or the name of a tIdentifier token.
            \return value */
        const String_t& getCurrentString() const;

        /** Get float value of current (most recently read) token.
            This is the value associated with a tFloat token.
            \return value */
        double getCurrentFloat() const;

        /** Get remaining unparsed line.
            \return unparsed line, starting exactly after the most recently read token */
        String_t getRemainingLine() const;

        /** Read next token.
            \return type of token read */
        TokenType readNextToken();

        /** Check for token type, read next on success.
            If the current token type is \c t, reads the next token and returns true.
            Otherwise, keep the current token unchanged and return false.
            \param t Token type to check for
            \return status */
        bool checkAdvance(TokenType t);

        /** Check for identifier, and read next token if succeeded.
            If the current token is identifier \c keyword, reads the next token and returns true.
            Otherwise, keep the current token and return false.
            \param keyword Keyword to check for (in upper case)
            \return status */
        bool checkAdvance(const char* keyword);

        /** Test for identifier character.
            \param c Character to test
            \return true on success */
        static bool isIdentifierCharacter(char c);

        /** Test for valid uppercase identifier.
            \param c Candidate to test
            \return true on success */
        static bool isValidUppercaseIdentifier(const String_t& candidate);

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

// Get most recently read token type.
inline interpreter::Tokenizer::TokenType
interpreter::Tokenizer::getCurrentToken() const
{
    return m_currentToken;
}

// Get integer value of current (most recently read) token.
inline int32_t
interpreter::Tokenizer::getCurrentInteger() const
{
    return m_currentInteger;
}

// Get string value of the current (most recently read) token.
inline const String_t&
interpreter::Tokenizer::getCurrentString() const
{
    return m_currentString;
}

// Get float value of current (most recently read) token.
inline double
interpreter::Tokenizer::getCurrentFloat() const
{
    return m_currentFloat;
}

// Get remaining unparsed line.
inline String_t
interpreter::Tokenizer::getRemainingLine() const
{
    return m_line.substr(m_pos);
}

// Read next token.
inline interpreter::Tokenizer::TokenType
interpreter::Tokenizer::readNextToken()
{
    read();
    return getCurrentToken();
}

#endif
