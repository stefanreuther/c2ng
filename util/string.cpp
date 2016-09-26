/**
  *  \file util/string.cpp
  */

#include <cstring>
#include "util/string.hpp"
#include "afl/string/char.hpp"
#include "afl/string/parse.hpp"

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
