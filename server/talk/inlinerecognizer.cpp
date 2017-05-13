/**
  *  \file server/talk/inlinerecognizer.cpp
  *  \brief Class server::talk::InlineRecognizer
  *
  *  c2ng porting note:
  *  This is a class to allow for possible later runtime configuration.
  *  As of 20170122, this is a very straight port that still uses the original static configuration.
  */

#include <cstring>
#include "server/talk/inlinerecognizer.hpp"
#include "afl/base/countof.hpp"
#include "afl/string/char.hpp"

namespace {
    const server::talk::InlineRecognizer::SmileyDefinition SMILEYS[] = {
        { "cool",       "B-)",  0,    "res/smileys/cool.png",       16, 16 },
        { "cry",        0,      0,    "res/smileys/cry.png",        16, 16 },
        { "eek",        0,      0,    "res/smileys/eek.png",        16, 16 },
        { "embarassed", 0,      0,    "res/smileys/embarassed.png", 16, 16 },
        { "evil",       ">:-)", 0,    "res/smileys/evil.png",       16, 16 },
        { "facepalm",   0,      0,    "res/smileys/facepalm.png",   16, 16 },
        { "frown",      0,      0,    "res/smileys/sad.png",        16, 16 }, /* same as sad */
        { "innocent",   0,      0,    "res/smileys/innocent.png",   16, 16 },
        { "lol",        ":-D",  ":D", "res/smileys/lol.png",        16, 16 },
        { "neutral",    ":-|",  0,    "res/smileys/neutral.png",    16, 16 },
        { "pacman",     0,      0,    "res/smileys/pacman.png",     16, 16 },
        { "rolleyes",   "8-|",  0,    "res/smileys/rolleyes.png",   16, 16 },
        { "sad",        ":-(",  ":(", "res/smileys/sad.png",        16, 16 },
        { "shrug",      0,      0,    "res/smileys/shrug.png",      32, 16 },
        { "smile",      ":-)",  ":)", "res/smileys/smile.png",      16, 16 },
        { "tongue",     ":-P",  ":P", "res/smileys/tongue.png",     16, 16 },
        { "transversalis", 0,   0,    "res/smileys/transversalis.png", 32, 16 },
        { "wink",       ";-)",  ";)", "res/smileys/wink.png",       16, 16 },
    };



    String_t findFirstCharacters()
    {
        String_t firsts = ":@";         /* colon is anchor point for URLs and :smileys:, @ is for email */
        for (size_t i = 0; i < countof(SMILEYS); ++i) {
            const server::talk::InlineRecognizer::SmileyDefinition& sm = SMILEYS[i];
            if (sm.symbol != 0 && firsts.find(sm.symbol[0]) == String_t::npos) {
                firsts += sm.symbol[0];
            }
            if (sm.symbol2 != 0 && firsts.find(sm.symbol2[0]) == String_t::npos) {
                firsts += sm.symbol2[0];
            }
        }
        return firsts;
    }

    bool findInlineURL(const String_t& text,
                       String_t::size_type start_at,
                       String_t::size_type anchor,
                       server::talk::InlineRecognizer::Info& info)
    {
        /* Protocols that are auto-detected */
        static const char*const PROTOCOLS[] = {
            "ftp",
            "http",
            "https",
            "mailto",
            "news",
            "nntp",
        };

        /* Check for protocol string */
        String_t::size_type n = anchor - start_at;
        String_t::size_type found_plen = 0;
        for (size_t i = 0; i < countof(PROTOCOLS); ++i) {
            String_t::size_type plen = std::strlen(PROTOCOLS[i]);
            if (plen <= n && text.compare(anchor - plen, plen, PROTOCOLS[i], plen) == 0) {
                found_plen = plen;
                break;
            }
        }
        if (found_plen == 0) {
            return false;
        }
        String_t::size_type url_start = anchor - found_plen;
        String_t::size_type url_end   = anchor + 1;

        /* Check whatever-before-the-string: must not be a letter */
        if (url_start > start_at && afl::string::charIsAlphanumeric(text[url_start-1])) {
            return false;
        }

        /* If it's an angle bracket, try to take that as delimiter */
        if (url_start > start_at && text[url_start-1] == '<') {
            while (url_end < text.size() && text[url_end] != '>' && text[url_end] != '\n') {
                ++url_end;
            }
            if (url_end < text.size() && text[url_end] == '>') {
                info.kind   = server::talk::InlineRecognizer::Link;
                info.start  = url_start;
                info.length = url_end - url_start;
                info.text.assign(text, url_start, url_end - url_start);
                return true;
            }
            /* reset url_end [#331], fails on "see <http://foo/That Page" test case otherwise */
            url_end = anchor + 1;
        }

        /* First character after the protocol must be slash or alphanumeric.
           This might fail some elaborate naming schemes, but will work for
           99.9% of all URLs out there, and users can still use [url] for
           those where it doesn't. */
        if (url_end >= text.size() || (text[url_end] != '/' && !afl::string::charIsAlphanumeric(text[url_end]))) {
            return false;
        }

        /* Simple heuristic: scan until whitespace found. Honor parens. */
        int paren = 0;
        bool back_out = true;
        while (url_end < text.size()) {
            char c = text[url_end];
            if (c == '\n' || c == ' ' || c == '\t' || c == '\r') {
                /* URL ends at whitespace */
                back_out = true;
                break;
            } else if (c == '"' || c == '>') {
                /* URL ends at unlikely characters */
                back_out = false;
                break;
            } else if (c == '(') {
                /* Start paren. For Wikipedia ("http://.../wiki/Foo_(Bar)") and MSDN ("...(VS=8.1).aspx") URLs */
                ++paren;
            } else if (c == ')') {
                /* End paren. */
                if (paren == 0) {
                    back_out = false;
                    break;
                } else {
                    --paren;
                }
            } else {
                /* Proceed */
            }
            ++url_end;
        }

        /* Make sure we don't include final punctuation */
        if (back_out && url_end > 0) {
            char prev = text[url_end-1];
            if (prev == '.' || prev == ',' || prev == ';' || prev == ':') {
                --url_end;
            }
        }

        /* Found. */
        info.kind   = server::talk::InlineRecognizer::Link;
        info.start  = url_start;
        info.length = url_end - url_start;
        info.text.assign(text, url_start, url_end - url_start);
        return true;
    }

    bool matchNamedSmiley(const String_t& text,
                          String_t::size_type anchor,
                          const char* name)
    {
        String_t::size_type n = std::strlen(name);
        return text.size() - anchor >= n+2
            && text[anchor] == ':'
            && text[anchor + n + 1] == ':'
            && text.compare(anchor+1, n, name, n) == 0;
    }

    bool matchSymbolSmiley(const String_t& text,
                           String_t::size_type anchor,
                           const char* name)
    {
        String_t::size_type n = std::strlen(name);

        /* Match smiley */
        if (text.size() - anchor < n || text.compare(anchor, n, name, n) != 0) {
            return false;
        }

        /* Match boundaries: a smiley starting with a letter must not immediately follow a letter,
           a smiley ending with a letter must not immediately be followed by a letter. */
        if (afl::string::charIsAlphanumeric(name[0]) && anchor > 0 &&
            afl::string::charIsAlphanumeric(text[anchor-1]))
        {
            return false;
        }

        if (afl::string::charIsAlphanumeric(name[n-1]) && text.size() - anchor > n
            && afl::string::charIsAlphanumeric(text[anchor + n]))
        {
            return false;
        }

        return true;
    }

    bool findInlineSmiley(const String_t& text,
                          String_t::size_type /*start_at*/,
                          String_t::size_type anchor,
                          server::talk::InlineRecognizer::Info& info)
    {
        /* Preinitialize info, assuming success */
        info.kind  = server::talk::InlineRecognizer::Smiley;
        info.start = anchor;

        /* Check all smileys */
        for (size_t i = 0; i < countof(SMILEYS); ++i) {
            const server::talk::InlineRecognizer::SmileyDefinition& sm = SMILEYS[i];
            if (matchNamedSmiley(text, anchor, sm.name)) {
                info.length = std::strlen(sm.name) + 2;
                info.text = sm.name;
                return true;
            }
            if (sm.symbol != 0 && matchSymbolSmiley(text, anchor, sm.symbol)) {
                info.length = std::strlen(sm.symbol);
                info.text = sm.name;
                return true;
            }
            if (sm.symbol2 != 0 && matchSymbolSmiley(text, anchor, sm.symbol2)) {
                info.length = std::strlen(sm.symbol2);
                info.text = sm.name;
                return true;
            }
        }
        return false;
    }

}

// Constructor.
server::talk::InlineRecognizer::InlineRecognizer()
    : m_firsts(findFirstCharacters())
{ }

// Destructor.
server::talk::InlineRecognizer::~InlineRecognizer()
{ }

// Get smiley definition.
const server::talk::InlineRecognizer::SmileyDefinition*
server::talk::InlineRecognizer::getSmileyDefinitionByName(const String_t& name) const
{
    // ex planetscentral/talk/smiley.cc:lookupSmiley
    for (std::size_t i = 0; i < countof(SMILEYS); ++i) {
        if (name == SMILEYS[i].name) {
            return &SMILEYS[i];
        }
    }
    return 0;
}

// Find an inline element.
bool
server::talk::InlineRecognizer::find(const String_t& text, String_t::size_type start_at, Kinds_t what, Info& info) const
{
    // ex planetscentral/talk/smiley.cc:findInlineElement
    /* Quick exit: if we're not expected to return anything, don't bother searching */
    if (what.empty()) {
        return false;
    }

    while (1) {
        /* Look for anchor */
        String_t::size_type anchor = text.find_first_of(m_firsts, start_at);
        if (anchor == String_t::npos) {
            return false;
        }

        /* Look for URLs */
        if (what.contains(Link) && text[anchor] == ':') {
            if (findInlineURL(text, start_at, anchor, info)) {
                return true;
            }
        }

        /* Look for smileys */
        if (what.contains(Smiley)) {
            if (findInlineSmiley(text, start_at, anchor, info)) {
                return true;
            }
        }

        /* Advance */
        start_at = anchor+1;
    }
}
