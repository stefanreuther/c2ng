/**
  *  \file interpreter/tokenizer.cpp
  *  \brief Class interpreter::Tokenizer
  */

#include "interpreter/tokenizer.hpp"
#include "interpreter/error.hpp"
#include "afl/string/char.hpp"
#include "util/math.hpp"

// Constructor.
interpreter::Tokenizer::Tokenizer(const String_t& str)
    : m_line(str),
      m_pos(0),
      m_currentToken(tEnd),
      m_currentString(),
      m_currentInteger(0),
      m_currentFloat(0)
{
    read();
}

// Destructor.
interpreter::Tokenizer::~Tokenizer()
{ }

// Check for token type, read next on success.
bool
interpreter::Tokenizer::checkAdvance(TokenType t)
{
    // ex IntTokenizer::checkAdvance
    if (getCurrentToken() == t) {
        read();
        return true;
    } else {
        return false;
    }
}

// Check for identifier, and read next token if succeeded.
bool
interpreter::Tokenizer::checkAdvance(const char* keyword)
{
    // ex IntTokenizer::checkAdvance
    // ex ccexpr.pas:ChompWord
    if (getCurrentToken() == tIdentifier && getCurrentString() == keyword) {
        read();
        return true;
    } else {
        return false;
    }
}

/** Read a token.
    Advances \c m_pos and sets \c m_currentToken to the next token encountered within the input line. */
void
interpreter::Tokenizer::read()
{
    // ex IntTokenizer::checkAdvance
    // ex ccexpr.pas:GetSymbol

    // Skip whitespace
    const String_t::size_type limit = m_line.size();
    while (m_pos < limit && (m_line[m_pos] == ' ' || m_line[m_pos] == '\r' || m_line[m_pos] == '\t' || m_line[m_pos] == '\f')) {
        ++m_pos;
    }

    // End reached?
    if (m_pos >= limit) {
        m_currentToken = tEnd;
        return;
    }

    // Check current character
    switch (m_line[m_pos]) {
     case '%':
        // Comment
        m_currentToken = tEnd;
        return;
     case '&':
        m_currentToken = tAmpersand;
        ++m_pos;
        return;
     case '#':
        m_currentToken = tHash;
        ++m_pos;
        return;
     case '+':
        m_currentToken = tPlus;
        ++m_pos;
        return;
     case '-':
        ++m_pos;
        if (m_pos < limit && m_line[m_pos] == '>') {
            m_currentToken = tArrow;
            ++m_pos;
        } else {
            m_currentToken = tMinus;
        }
        return;
     case '*':
        m_currentToken = tMultiply;
        ++m_pos;
        return;
     case '/':
        m_currentToken = tSlash;
        ++m_pos;
        return;
     case '\\':
        m_currentToken = tBackslash;
        ++m_pos;
        return;
     case '^':
        m_currentToken = tCaret;
        ++m_pos;
        return;
     case '(':
        m_currentToken = tLParen;
        ++m_pos;
        return;
     case ')':
        m_currentToken = tRParen;
        ++m_pos;
        return;
     case ',':
        m_currentToken = tComma;
        ++m_pos;
        return;
     case '=':
        m_currentToken = tEQ;
        ++m_pos;
        return;
     case '<':
        ++m_pos;
        if (m_pos < limit && m_line[m_pos] == '=') {
            m_currentToken = tLE;
            ++m_pos;
        } else if (m_pos < limit && m_line[m_pos] == '>') {
            m_currentToken = tNE;
            ++m_pos;
        } else {
            m_currentToken = tLT;
        }
        return;
     case '>':
        ++m_pos;
        if (m_pos < limit && m_line[m_pos] == '=') {
            m_currentToken = tGE;
            ++m_pos;
        } else {
            m_currentToken = tGT;
        }
        return;
     case ':':
        ++m_pos;
        if (m_pos < limit && m_line[m_pos] == '=') {
            m_currentToken = tAssign;
            ++m_pos;
        } else {
            m_currentToken = tColon;
        }
        return;
     case ';':
        m_currentToken = tSemicolon;
        ++m_pos;
        return;
     case '.':
        if (m_pos+1 < limit && m_line[m_pos+1] >= '0' && m_line[m_pos+1] <= '9') {
            readNumber();
        } else {
            m_currentToken = tDot;
            ++m_pos;
        }
        return;
     case '\'':
        // Simple string
        // FIXME: sanitize against broken UTF-8 on input? Let TextFile do that?
        {
            String_t::size_type p2 = m_pos+1;
            while (p2 < limit && m_line[p2] != '\'') {
                ++p2;
            }
            if (p2 >= limit) {
                throw Error::expectSymbol("'");
            }
            m_currentString.assign(m_line, m_pos+1, p2-m_pos-1);
            m_currentToken = tString;
            m_pos = p2+1;
        }
        return;

     case '"':
        // String with quotes
        // FIXME: sanitize against broken UTF-8 on input? Let TextFile do that?
        {
            ++m_pos;
            bool quoted = false;
            m_currentString.clear();
            while (m_pos < limit && (m_line[m_pos] != '"' || quoted)) {
                if (m_line[m_pos] == '\\' && !quoted) {
                    quoted = true;
                } else if (m_line[m_pos] == 'n' && quoted) {
                    m_currentString += "\n";
                    quoted = false;
                } else if (m_line[m_pos] == 't' && quoted) {
                    m_currentString += "\t";
                    quoted = false;
                } else {
                    m_currentString += m_line[m_pos];
                    quoted = false;
                }
                ++m_pos;
            }
            if (m_pos >= limit) {
                throw Error::expectSymbol("\"");
            }
            ++m_pos;
            m_currentToken = tString;
        }
        return;

     case '0': case '1': case '2': case '3': case '4':
     case '5': case '6': case '7': case '8': case '9':
        readNumber();
        return;

     default:
        char c = afl::string::charToUpper(m_line[m_pos]);
        ++m_pos;
        m_currentString.assign(1, c);
        if ((c >= 'A' && c <= 'Z') || c == '$' || c == '_') {
            // Identifier
            while (m_pos < limit) {
                c = afl::string::charToUpper(m_line[m_pos]);
                if (isIdentifierCharacter(c)) {
                    m_currentString += c;
                    ++m_pos;
                } else {
                    break;
                }
            }

            // Special case: if identifier "ends" in '.', strip it.
            if (m_currentString[m_currentString.size()-1] == '.') {
                m_currentString.erase(m_currentString.size()-1);
                --m_pos;
            }

            // Check length
            if (m_currentString.size() > 255) {
                throw Error("Identifier too long");
            }

            // Handle special keywords
            if (m_currentString == "TRUE") {
                m_currentInteger = 1;
                m_currentToken = tBoolean;
            } else if (m_currentString == "FALSE") {
                m_currentInteger = 0;
                m_currentToken = tBoolean;
            } else if (m_currentString == "PI") {
                m_currentToken = tFloat;
                m_currentFloat = util::PI;
            } else if (m_currentString == "AND") {
                m_currentToken = tAND;
            } else if (m_currentString == "OR") {
                m_currentToken = tOR;
            } else if (m_currentString == "XOR") {
                m_currentToken = tXOR;
            } else if (m_currentString == "NOT") {
                m_currentToken = tNOT;
            } else if (m_currentString == "MOD") {
                m_currentToken = tMOD;
            } else {
                // Identifier
                m_currentToken = tIdentifier;
            }
        } else {
            // Invalid
            m_currentToken = tInvalid;
        }
        return;
    }
}

/** Read number.
    Assumes that \c m_pos points to a digit or period.
    Reads the number into m_currentFloat / m_currentInteger, and sets m_currentToken appropriately. */
void
interpreter::Tokenizer::readNumber()
{
    // Initialize
    bool isFloat = false;
    m_currentInteger = 0;
    m_currentFloat = 0;

    // Read places before period
    while (m_pos < m_line.size() && m_line[m_pos] >= '0' && m_line[m_pos] <= '9') {
        int digit = m_line[m_pos] - '0';

        // Advance float
        m_currentFloat = 10*m_currentFloat + digit;

        // Advance int. Maximum int is 0x7FFFFFFF (yes, this means you cannot enter -0x80000000 in CCScript)
        if (!isFloat) {
            if (m_currentInteger > (0x7FFFFFFF - digit) / 10) {
                isFloat = true;
            } else {
                m_currentInteger = 10*m_currentInteger + digit;
            }
        }

        ++m_pos;
    }

    // Check for period
    if (m_pos < m_line.size() && m_line[m_pos] == '.') {
        isFloat = true;
        ++m_pos;

        // Read places after period
        double divide = 1.0;
        while (m_pos < m_line.size() && m_line[m_pos] >= '0' && m_line[m_pos] <= '9') {
            int digit = m_line[m_pos] - '0';
            m_currentFloat = 10*m_currentFloat + digit;
            divide *= 10;
            ++m_pos;
        }

        m_currentFloat /= divide;
    }

    // Output
    if (isFloat) {
        m_currentToken = tFloat;
    } else {
        m_currentToken = tInteger;
    }
}

// Test for identifier character.
bool
interpreter::Tokenizer::isIdentifierCharacter(char c)
{
    // ex ccexpr.pas:IsID
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '$' || c == '_' || (c >= '0' && c <= '9') || c == '.';
}

// Test for valid uppercase identifier.
bool
interpreter::Tokenizer::isValidUppercaseIdentifier(const String_t& candidate)
{
    if (candidate.empty()) {
        return false;
    }
    for (size_t i = 0, n = candidate.size(); i < n; ++i) {
        if ((candidate[i] >= 'A' && candidate[i] <= 'Z')
            || candidate[i] == '_'
            || (i > 0
                && ((candidate[i] >= '0' && candidate[i] <= '9')
                    || candidate[i] == '.' || candidate[i] == '$')))
        {
            // ok
        } else {
            return false;
        }
    }
    return true;
}
