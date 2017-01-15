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
#include "util/translation.hpp"

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
        return p.getCurrentChar(ch)
            && (ch == '('
                || ch == '\''
                || ch == '"'
                || ch == '#'
                || afl::string::charIsAlphanumeric(ch));
    }



    /*
     *  Parser
     */

    const Filter& parseFilter1(util::StringParser& p, afl::base::Deleter& h);

    /** Parse string.
        Consumes everything up to and including the delimiter \c ch. */
    const Filter& parseString(util::StringParser& p, afl::base::Deleter& h, char ch)
    {
        const char ntbs[] = { ch, '\0' };
        String_t value;
        p.parseDelim(ntbs, value);
        if (!p.parseChar(ch)) {
            throw ParseException(_("unterminated string constant"));
        }
        return h.addNew(new StringFilter(value));
    }

    /** Parse upper bound.
        If there's a "-", parse the upper bound and update \c high. */
    void parseUpperBound(util::StringParser& p, int& low, int& high)
    {
        int i;
        if (p.parseChar('-')) {
            parseWhitespace(p);
            if (!p.parseInt(i) || i < low) {
                throw ParseException(_("Invalid upper bound after \"#\""));
            }
            high = i;
        }
    }

    /** Parse elementary expression.
        <pre>parseSingle := '(' parseFilter1 ')' | string | '#'? number ('-' number)? | identifier '*'?</pre> */
    const Filter& parseSingle(util::StringParser& p, afl::base::Deleter& h)
    {
        // ex game/un-trnflt.cc:parseSingle
        int i;
        parseWhitespace(p);
        if (p.parseChar('(')) {
            // parenized expression
            const Filter& result = parseFilter1(p, h);
            parseWhitespace(p);
            if (!p.parseChar(')')) {
                throw ParseException(_("\")\" expected"));
            }
            return result;
        } else if (p.parseChar('\'')) {
            // single-quoted string
            return parseString(p, h, '\'');
        } else if (p.parseChar('"')) {
            // double-quoted string
            return parseString(p, h, '"');
        } else if (p.parseChar('#')) {
            // index / index range
            parseWhitespace(p);
            if (!p.parseInt(i) || i <= 0) {
                throw ParseException(_("Command index expected after \"#\""));
            }
            parseWhitespace(p);
            int low = i, high = i;
            parseUpperBound(p, low, high);
            return h.addNew(new IndexFilter(low, high));
        } else if (p.parseInt(i)) {
            // Id / Id range
            parseWhitespace(p);
            int low = i, high = i;
            parseUpperBound(p, low, high);
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
                    bool wild = p.parseChar('*');
                    return h.addNew(new NameFilter(text, wild));
                }
            } else {
                throw ParseException(_("Invalid expression"));
            }
        }
    }


    /** Parse almost-elementary expression.
        <pre>parseEx ::= parseSingle+</pre> */
    const Filter& parseEx(util::StringParser& p, afl::base::Deleter& h)
    {
        const Filter* result = &parseSingle(p, h);
        while (1) {
            parseWhitespace(p);
            if (!isSingle(p)) {
                break;
            }
            result = &h.addNew(new AndFilter(*result, parseSingle(p, h)));
        }
        return *result;
    }

    /** Parse "and" expression.
        <pre>parseAnd ::= '!'? parseEx</pre> */
    const Filter& parseAnd(util::StringParser& p, afl::base::Deleter& h)
    {
        // ex game/un-trnflt.cc:parseAnd
        parseWhitespace(p);
        if (p.parseChar('!')) {
            return h.addNew(new NegateFilter(parseAnd(p, h)));
        } else {
            return parseEx(p, h);
        }
    }

    /** Parse "or" expression.
        <pre>parseOr ::= parseAnd ('&' parseAnd)*</pre> */
    const Filter& parseOr(util::StringParser& p, afl::base::Deleter& h)
    {
        // ex game/un-trnflt.cc:parseOr
        const Filter* result = &parseAnd(p, h);
        while (1) {
            parseWhitespace(p);
            if (!p.parseChar('&')) {
                break;
            }
            result = &h.addNew(new AndFilter(*result, parseAnd(p, h)));
        }
        return *result;
    }

    /** Parse filter expression, back-end.
        <pre>parseFilter1 ::= parseOr ('|' parseOr)*</pre> */
    const Filter& parseFilter1(util::StringParser& p, afl::base::Deleter& h)
    {
        // ex game/un-trnflt.cc:parseFilter1
        const Filter* result = &parseOr(p, h);
        while (1) {
            parseWhitespace(p);
            if (!p.parseChar('|')) {
                break;
            }
            result = &h.addNew(new OrFilter(*result, parseOr(p, h)));
        }
        return *result;
    }


} } } }


// Parse filter expression.
const game::v3::trn::Filter&
game::v3::trn::Filter::parse(String_t text, afl::base::Deleter& deleter)
{
    // ex game/un-trnflt.cc:parseFilter
    util::StringParser p(text);
    const Filter& result = parseFilter1(p, deleter);
    parseWhitespace(p);
    if (!p.parseEnd()) {
        throw ParseException(_("Expression incorrectly terminated"));
    }
    return result;
}
