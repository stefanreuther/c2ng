/**
  *  \file game/v3/trn/filter.cpp
  *  \brief Class game::v3::trn::Filter
  */

#include "game/v3/trn/filter.hpp"
#include "afl/string/char.hpp"
#include "game/v3/trn/andfilter.hpp"
#include "game/v3/trn/constantfilter.hpp"
#include "game/v3/trn/idfilter.hpp"
#include "game/v3/trn/indexfilter.hpp"
#include "game/v3/trn/namefilter.hpp"
#include "game/v3/trn/negatefilter.hpp"
#include "game/v3/trn/orfilter.hpp"
#include "game/v3/trn/parseexception.hpp"
#include "game/v3/trn/stringfilter.hpp"
#include "util/stringparser.hpp"

namespace game { namespace v3 { namespace trn { namespace {

    /*
       filter ::= or
                | or "|" filter
       or     ::= and
                | and "&" or
       and    ::= '!' and
                | ex
       ex     ::= single
                | single ex
       single ::= "(" filter ")"
                | identifier
                | identifier "*"
                | number
                | number "-" number
                | "#" number
                | "#" number "-" number
                | "'" character* "'"
                | """ character* """
    */



    /*
     *  Utilities
     */

    bool charIsLetter(char ch)
    {
        return afl::string::charIsUpper(ch) || afl::string::charIsLower(ch);
    }

    void parseWhitespace(util::StringParser& p)
    {
        // ex game/un-trnflt.cc:parseWs
        String_t tmp;
        p.parseWhile(afl::string::charIsSpace, tmp);
    }

    /** Check whether text is the start of a 'single' production. */
    bool isSingle(const util::StringParser& p)
    {
        // ex game/un-trnflt.cc:isSingle
        char ch;
        return p.getCurrentCharacter(ch)
            && (ch == '('
                || ch == '\''
                || ch == '"'
                || ch == '#'
                || afl::string::charIsAlphanumeric(ch));
    }



    /*
     *  Parser
     */

    const Filter& parseFilter1(util::StringParser& p, afl::base::Deleter& h, afl::string::Translator& tx);

    /** Parse string.
        Consumes everything up to and including the delimiter \c ch. */
    const Filter& parseString(util::StringParser& p, afl::base::Deleter& h, afl::string::Translator& tx, char ch)
    {
        const char ntbs[] = { ch, '\0' };
        String_t value;
        p.parseDelim(ntbs, value);
        if (!p.parseCharacter(ch)) {
            throw ParseException(tx("Unterminated string constant"));
        }
        return h.addNew(new StringFilter(value));
    }

    /** Parse upper bound.
        If there's a "-", parse the upper bound and update \c high. */
    void parseUpperBound(util::StringParser& p, afl::string::Translator& tx, int& low, int& high)
    {
        if (p.parseCharacter('-')) {
            int i;
            parseWhitespace(p);
            if (!p.parseInt(i) || i < low) {
                throw ParseException(tx("Invalid upper bound after \"-\""));
            }
            high = i;
        }
    }

    /** Parse elementary expression.
        <pre>parseSingle := '(' parseFilter1 ')' | string | '#'? number ('-' number)? | identifier '*'?</pre> */
    const Filter& parseSingle(util::StringParser& p, afl::base::Deleter& h, afl::string::Translator& tx)
    {
        // ex game/un-trnflt.cc:parseSingle
        int i;
        parseWhitespace(p);
        if (p.parseCharacter('(')) {
            // parenized expression
            const Filter& result = parseFilter1(p, h, tx);
            parseWhitespace(p);
            if (!p.parseCharacter(')')) {
                throw ParseException(tx("Expected \")\""));
            }
            return result;
        } else if (p.parseCharacter('\'')) {
            // single-quoted string
            return parseString(p, h, tx, '\'');
        } else if (p.parseCharacter('"')) {
            // double-quoted string
            return parseString(p, h, tx, '"');
        } else if (p.parseCharacter('#')) {
            // index / index range
            parseWhitespace(p);
            if (!p.parseInt(i) || i <= 0) {
                throw ParseException(tx("Expected command index after \"#\""));
            }
            parseWhitespace(p);
            int low = i, high = i;
            parseUpperBound(p, tx, low, high);
            return h.addNew(new IndexFilter(low, high));
        } else if (p.parseInt(i)) {
            // Id / Id range
            parseWhitespace(p);
            int low = i, high = i;
            parseUpperBound(p, tx, low, high);
            return h.addNew(new IdFilter(low, high));
        } else {
            String_t text;
            if (p.parseWhile(charIsLetter, text)) {
                if (afl::string::strCaseCompare(text, "true") == 0) {
                    return h.addNew(new ConstantFilter(true));
                } else if (afl::string::strCaseCompare(text, "false") == 0) {
                    return h.addNew(new ConstantFilter(false));
                } else {
                    parseWhitespace(p);
                    bool wild = p.parseCharacter('*');
                    return h.addNew(new NameFilter(text, wild));
                }
            } else {
                throw ParseException(tx("Invalid expression"));
            }
        }
    }


    /** Parse almost-elementary expression.
        <pre>parseEx ::= parseSingle+</pre> */
    const Filter& parseEx(util::StringParser& p, afl::base::Deleter& h, afl::string::Translator& tx)
    {
        const Filter* result = &parseSingle(p, h, tx);
        while (1) {
            parseWhitespace(p);
            if (!isSingle(p)) {
                break;
            }
            result = &h.addNew(new AndFilter(*result, parseSingle(p, h, tx)));
        }
        return *result;
    }

    /** Parse "and" expression.
        <pre>parseAnd ::= '!'? parseEx</pre> */
    const Filter& parseAnd(util::StringParser& p, afl::base::Deleter& h, afl::string::Translator& tx)
    {
        // ex game/un-trnflt.cc:parseAnd
        parseWhitespace(p);
        if (p.parseCharacter('!')) {
            return h.addNew(new NegateFilter(parseAnd(p, h, tx)));
        } else {
            return parseEx(p, h, tx);
        }
    }

    /** Parse "or" expression.
        <pre>parseOr ::= parseAnd ('&' parseAnd)*</pre> */
    const Filter& parseOr(util::StringParser& p, afl::base::Deleter& h, afl::string::Translator& tx)
    {
        // ex game/un-trnflt.cc:parseOr
        const Filter* result = &parseAnd(p, h, tx);
        while (1) {
            parseWhitespace(p);
            if (!p.parseCharacter('&')) {
                break;
            }
            result = &h.addNew(new AndFilter(*result, parseAnd(p, h, tx)));
        }
        return *result;
    }

    /** Parse filter expression, back-end.
        <pre>parseFilter1 ::= parseOr ('|' parseOr)*</pre> */
    const Filter& parseFilter1(util::StringParser& p, afl::base::Deleter& h, afl::string::Translator& tx)
    {
        // ex game/un-trnflt.cc:parseFilter1
        const Filter* result = &parseOr(p, h, tx);
        while (1) {
            parseWhitespace(p);
            if (!p.parseCharacter('|')) {
                break;
            }
            result = &h.addNew(new OrFilter(*result, parseOr(p, h, tx)));
        }
        return *result;
    }

} } } }


// Parse filter expression.
const game::v3::trn::Filter&
game::v3::trn::Filter::parse(String_t text, afl::base::Deleter& deleter, afl::string::Translator& tx)
{
    // ex game/un-trnflt.cc:parseFilter
    util::StringParser p(text);
    const Filter& result = parseFilter1(p, deleter, tx);
    parseWhitespace(p);
    if (!p.parseEnd()) {
        throw ParseException(tx("Expression incorrectly terminated"));
    }
    return result;
}
