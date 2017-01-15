/**
  *  \file util/string.cpp
  */

#include <cstring>
#include <algorithm>
#include "util/string.hpp"
#include "afl/string/char.hpp"
#include "afl/string/parse.hpp"
#include "util/stringparser.hpp"

bool
util::stringMatch(const char* pattern, const char* tester)
{
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

bool
util::stringMatch(const char* pattern, const String_t& tester)
{
    return stringMatch(pattern, tester.c_str());
}



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
    int minResult, maxResult;

    String_t::size_type p = s.find('-');
    if (p != s.npos) {
        // two parts
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
            if (sp.parseChar('\t')) {
                // Tab: this is an option
                maxOption = std::max(maxOption, component.size());
            } else if (sp.parseChar('\n')) {
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
            if (sp.parseChar('\t')) {
                // Tab: this is an option
                result.append(2, ' ');
                result += component;
                result.append(maxOption - component.size(), ' ');
            } else if (sp.parseChar('\n')) {
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
    for (String_t::size_type i = 0, n = name.size(); i < name.size(); ++i) {
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
