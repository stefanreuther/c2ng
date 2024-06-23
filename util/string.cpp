/**
  *  \file util/string.cpp
  *  \brief String Utilities
  */

#include <cstring>
#include <algorithm>
#include "util/string.hpp"
#include "afl/charset/base64.hpp"
#include "afl/charset/utf8.hpp"
#include "afl/charset/utf8reader.hpp"
#include "afl/string/char.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "util/stringparser.hpp"

namespace {
    bool mustEncode(const String_t& word)
    {
        for (String_t::size_type i = 0; i < word.size(); ++i) {
            uint8_t u = word[i];
            if (u >= 0x80 || u < 0x20 || u == '?') {
                return true;
            }
        }
        return false;
    }

    size_t countZeroes(const String_t& a, size_t& pos)
    {
        size_t n = 0;
        while (pos < a.size() && a[pos] == '0') {
            ++pos, ++n;
        }
        return n;
    }

    size_t countDigits(const String_t& a, size_t& pos)
    {
        size_t n = 0;
        while (pos < a.size() && a[pos] >= '0' && a[pos] <= '9') {
            ++pos, ++n;
        }
        return n;
    }
}

// String Match, PHost way.
bool
util::stringMatch(const char* pattern, const char* tester)
{
    // ex phost.pas:PMatch
    while (*pattern) {
        char p = *pattern++;
        if (!*tester) {
            // tester ends before pattern. Permitted?
            return afl::string::charIsLower(p);
        } else {
            // character must match exactly otherwise
            if (afl::string::charToLower(p) != afl::string::charToLower(*tester)) {
                return false;
            }
        }
        ++tester;
    }
    // pattern ends -- tester must end, too
    return *tester == 0;
}

// String Match, PHost way.
bool
util::stringMatch(const char* pattern, const String_t& tester)
{
    return stringMatch(pattern, tester.c_str());
}

// Consume word from comma-separated list.
bool
util::eatWord(const char*& tpl, String_t& word)
{
    if (tpl == 0) {
        // error in invocation, pretend end
        return false;
    } else if (*tpl == '\0') {
        // end reached
        return false;
    } else if (const char* p = std::strchr(tpl, ',')) {
        // intermediate word
        word.assign(tpl, p - tpl);
        tpl = p+1;
        return true;
    } else {
        // final word
        word.assign(tpl);
        tpl += strlen(tpl);
        return true;
    }
}

bool
util::parseRange(const String_t& s, int& min, int& max, String_t::size_type& pos)
{
    String_t::size_type p = s.find('-');
    if (p != s.npos) {
        // two parts
        int minResult, maxResult;
        if (!afl::string::strToInteger(s.substr(0, p), minResult, pos)) {
            return false;
        }

        // do we have a maximum?
        ++p;
        while (p < s.size() && afl::string::charIsSpace(s[p])) {
            ++p;
        }

        if (p < s.size()) {
            // yes, case three
            if (!afl::string::strToInteger(s.substr(p), maxResult, pos)) {
                pos += p;
                return false;
            } else {
                min = minResult;
                max = maxResult;
                return true;
            }
        } else {
            // no, case two
            min = minResult;
            return true;
        }
    } else {
        // one part, case one
        int minResult;
        if (!afl::string::strToInteger(s, minResult, pos)) {
            return false;
        } else {
            min = max = minResult;
            return true;
        }
    }
}

// Parse a player character.
bool
util::parsePlayerCharacter(const char ch, int& number)
{
    // ex game/player.h:getPlayerIdFromChar
    if (ch >= '0' && ch <= '9') {
        number = ch - '0';
        return true;
    } else if (ch >= 'A' && ch <= 'Z') {
        number = ch - 'A' + 10;
        return true;
    } else if (ch >= 'a' && ch <= 'z') {
        number = ch - 'a' + 10;
        return true;
    } else {
        return false;
    }
}

// Parse a boolean value.
bool
util::parseBooleanValue(const String_t& s, bool& result)
{
    if (stringMatch("No", s) || stringMatch("False", s)) {
        result = false;
        return true;
    } else if (stringMatch("Yes", s) || stringMatch("True", s)) {
        result = true;
        return true;
    } else {
        int32_t tmp;
        if (afl::string::strToInteger(s, tmp) && (tmp == 0 || tmp == 1)) {
            result = (tmp != 0);
            return true;
        } else {
            return false;
        }
    }
}

// Parse a zoom level.
bool
util::parseZoomLevel(const String_t& s, int& mul, int& div)
{
    int m = 1, d = 1;
    String_t::size_type pos = s.find_first_of(":/");
    bool ok;
    if (pos == String_t::npos) {
        d = 1;
        ok = afl::string::strToInteger(s, m);
    } else {
        ok = afl::string::strToInteger(s.substr(0, pos), m)
            && afl::string::strToInteger(s.substr(pos+1), d);
    }
    if (ok && m > 0 && d > 0) {
        mul = m;
        div = d;
        return true;
    } else {
        return false;
    }
}

String_t
util::formatOptions(String_t s)
{
    // Pass 1: figure out length of "options" part
    size_t maxOption = 0;
    {
        StringParser sp(s);
        while (1) {
            String_t component;
            sp.parseDelim("\t\n", component);
            if (sp.parseCharacter('\t')) {
                // Tab: this is an option
                maxOption = std::max(maxOption, component.size());
            } else if (sp.parseCharacter('\n')) {
                // Newline: nothing to do
            } else {
                // End reached
                break;
            }
        }
    }

    // Add room
    maxOption += 3;

    // Pass 2: build result
    String_t result;
    {
        StringParser sp(s);
        while (1) {
            String_t component;
            sp.parseDelim("\t\n", component);
            if (sp.parseCharacter('\t')) {
                // Tab: this is an option
                result.append(2, ' ');
                result += component;
                result.append(maxOption - component.size(), ' ');
            } else if (sp.parseCharacter('\n')) {
                // Newline: nothing to do
                result += component;
                result += '\n';
            } else {
                // End reached
                result += component;
                break;
            }
        }
    }
    return result;
}

// Beautify variable name.
String_t
util::formatName(String_t name)
{
    // ex int/propenum.h:beautifyName
    bool hadUC = false;
    for (String_t::size_type i = 0, n = name.size(); i < n; ++i) {
        if (afl::string::charIsUpper(name[i])) {
            if (hadUC) {
                name[i] = afl::string::charToLower(name[i]);
            } else {
                hadUC = true;
            }
        } else {
            hadUC = false;
        }
    }
    return name;
}

// Format age of an information.
String_t
util::formatAge(int currentTurn, int historyTurn, afl::string::Translator& tx)
{
    // ex formatTurnNumber
    int age = currentTurn - historyTurn;
    if (age == 0) {
        return tx("current turn");
    } else if (age == 1) {
        return tx("previous turn");
    } else if (age < 0) {
        return afl::string::Format(tx("turn %d"), historyTurn);
    } else {
        return afl::string::Format(tx("%d turns ago"), age);
    }
}

// Format a zoom level.
String_t
util::formatZoomLevel(int mul, int div)
{
    if (div == 1) {
        return afl::string::Format("%d", mul);
    } else {
        return afl::string::Format("%d/%d", mul, div);
    }
}

// Encode MIME header.
String_t
util::encodeMimeHeader(String_t input, String_t charsetName)
{
    // FIXME: RFC 2047 places some pretty tight limits on the format of lines containing
    // encoded words:
    // - max 75 chars per encoded word
    // - max 76 chars per line containing an encoded word
    // Since we don't see complete lines, we only try to enforce the per-word limit.
    // "65" is 75 minus "=" and "?", minus roundoff errors.
    String_t::size_type maxCharsPerWord = (65 - charsetName.size()) * 3/4;
    String_t::size_type n = 0;
    String_t::size_type p;
    String_t result;
    while ((p = input.find_first_not_of(" \t\r\n", n)) != String_t::npos) {
        // Copy run of space characters
        result.append(input, n, p-n);

        // Find next word
        n = input.find_first_of(" \t\r\n", p);
        if (n == String_t::npos) {
            n = input.size();
        }
        String_t word(input, p, n-p);
        if (mustEncode(word)) {
            String_t::size_type i = 0;
            while (i < word.size()) {
                String_t::size_type n = word.size() - i;
                if (n > maxCharsPerWord) {
                    n = maxCharsPerWord;
                }
                result += "=?";
                result += charsetName;
                result += "?B?";
                result += afl::string::fromBytes(afl::charset::Base64().encode(afl::string::toMemory(String_t(word, i, n))));
                result += "?=";
                i += n;
                if (i < word.size()) {
                    result += "\r\n ";
                }
            }
        } else {
            result += word;
        }
    }
    return result;
}

// Encode as HTML.
String_t
util::encodeHtml(const String_t& input, bool rawUnicode)
{
    String_t escaped;
    afl::charset::Utf8Reader rdr(afl::string::toBytes(input), 0);
    afl::charset::Utf8 u8;
    while (rdr.hasMore()) {
        afl::charset::Unichar_t ch = rdr.eat();
        if (ch == '&') {
            escaped.append("&amp;");
        } else if (ch == '"') {
            escaped.append("&quot;");
        } else if (ch == '<') {
            escaped.append("&lt;");
        } else if (ch == '>') {
            escaped.append("&gt;");
        } else if (ch == 39 || (ch > 127 && !rawUnicode)) {
            // 39 = single quote, always encoded numerically because &apos; is not in XML.
            escaped.append(afl::string::Format("&#%d;", ch));
        } else {
            u8.append(escaped, ch);
        }
    }
    return escaped;
}

void
util::addTrailingCharacter(String_t& s, char ch)
{
    if (s.empty() || s[s.size()-1] != ch) {
        s += ch;
    }
}

void
util::removeTrailingCharacter(String_t& s, char ch)
{
    if (!s.empty() && s[s.size()-1] == ch) {
        s.erase(s.size()-1);
    }
}

void
util::addListItem(String_t& s, String_t delim, String_t ele)
{
    if (!ele.empty()) {
        if (!s.empty()) {
            s += delim;
        }
        s += ele;
    }
}

String_t
util::sanitizeString(String_t str)
{
    String_t result;
    afl::charset::Utf8Reader rdr(afl::string::toBytes(str), 0);
    while (rdr.hasMore()) {
        afl::charset::Unichar_t ch = rdr.eat();
        if (ch < ' ' || ch >= 0x7F) {
            result += '?';
        } else {
            result += char(ch);
        }
    }
    return result;
}

int
util::strCollate(const String_t& a, const String_t& b)
{
    size_t posA = 0, posB = 0;
    while (1) {
        size_t zeroA = countZeroes(a, posA);
        size_t zeroB = countZeroes(b, posB);
        size_t digitsA = countDigits(a, posA);
        size_t digitsB = countDigits(b, posB);

        // Shorter number is smaller
        if (digitsA != digitsB) {
            return digitsA < digitsB ? -1 : +1;
        }

        // Equal number compares lexically
        if (digitsA != 0) {
            if (int n = a.compare(posA - digitsA, digitsA, b, posB - digitsB, digitsB)) {
                return n;
            }
        }

        // More leading zeroes goes in front
        if (zeroA != zeroB) {
            return zeroA > zeroB ? -1 : +1;
        }

        // If string ends, compare as tie-breaker
        bool endA = (posA == a.size());
        bool endB = (posB == b.size());
        if (endA) {
            if (endB) {
                // Both end; compare as tie-breakse
                return a.compare(b);
            } else {
                // A ends, B doesn't
                return -1;
            }
        } else {
            if (endB) {
                // B ends, A doesn't
                return +1;
            } else {
                // We have one more character
                uint8_t charA = uint8_t(afl::string::charToUpper(a[posA++]));
                uint8_t charB = uint8_t(afl::string::charToUpper(b[posB++]));
                if (charA != charB) {
                    return charA < charB ? -1 : +1;
                }
            }
        }
    }
}

const char*
util::strStartsWith(const String_t& str, const char* pfx)
{
    size_t len = std::strlen(pfx);
    if (str.compare(0, len, pfx, len) == 0) {
        return str.data() + len;
    } else {
        return 0;
    }
}
