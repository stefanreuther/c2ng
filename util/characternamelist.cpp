/**
  *  \file util/characternamelist.cpp
  *  \brief Class util::CharacterNameList
  */

#include "util/characternamelist.hpp"
#include "afl/base/countof.hpp"
#include "afl/charset/utf8reader.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/char.hpp"
#include "afl/string/format.hpp"

using afl::charset::Unichar_t;
using afl::string::Format;

namespace {
    /** Skip whitespace in a string.
        @param s [in] string
        @param n [in/out] current position. Updated to point to the first non-whitespace */
    void skipWhitespace(const String_t& s, String_t::size_type& n)
    {
        while (n < s.size() && afl::string::charIsSpace(s[n])) {
            ++n;
        }
    }

    /** Parse hex number. Reads hex digits from a string and assembles them into a number.
        @param s [in] string
        @param n [in/out] current position. Updated to point after the hex digits
        @param result [out] receives the hex number
        @return true iff any hex digits were read */
    bool parseHex(const String_t& s, String_t::size_type& n, Unichar_t& result)
    {
        bool did = false;
        result = 0;
        while (n < s.size()) {
            char c = afl::string::charToLower(s[n]);
            if (c >= '0' && c <= '9') {
                result = 16*result + (c-'0');
                did = true;
                ++n;
            } else if (c >= 'a' && c <= 'f') {
                result = 16*result + 10 + (c-'a');
                did = true;
                ++n;
            } else {
                break;
            }
        }
        return did;
    }

    struct CompareValues {
        bool operator()(const std::pair<Unichar_t, int>& a, const std::pair<Unichar_t, int>& b) const
            {
                return a.second > b.second
                    || (a.second == b.second
                        && a.first < b.first);
            }
    };
}

util::CharacterNameList::CharacterNameList()
    : m_names(), m_aliases()
{ }

util::CharacterNameList::~CharacterNameList()
{ }

void
util::CharacterNameList::addDefault()
{
    // ex loadCharacterNames() (part)
    /* Default names for our characters */
    for (int i = 0; i < 16; ++i) {
        m_names[0xE100 + i] = Format("[PCC2] REPLACEMENT UPPER LEFT %X", i);
        m_names[0xE110 + i] = Format("[PCC2] REPLACEMENT UPPER RIGHT %X", i);
        m_names[0xE120 + i] = Format("[PCC2] REPLACEMENT LOWER LEFT %X", i);
        m_names[0xE130 + i] = Format("[PCC2] REPLACEMENT LOWER RIGHT %X", i);
    }
    m_names[0xE140] = "[PCC2] TOP ARC CLOCKWISE ARROW";
    m_names[0xE141] = "[PCC2] NESTED RECTANGLES ORNAMENT";
    m_names[0xE142] = "[PCC2] ORNAMENT LEFT";
    m_names[0xE143] = "[PCC2] ORNAMENT RIGHT";
    m_names[0xE144] = "[PCC2] THIN VERTICAL BAR";
}

void
util::CharacterNameList::loadNames(afl::io::Stream& in)
{
    // ex loadCharacterNames() (part)
    const Unichar_t NONE = 0xFFFFFFFF;
    afl::io::TextFile tf(in);
    String_t line;
    Unichar_t current = NONE;
    while (tf.readLine(line)) {
        /* 001D    <control>
                   = INFORMATION SEPARATOR THREE
           0020    SPACE */
        Unichar_t nchar = 0;

        /* Strip "comment" */
        String_t::size_type n = line.find('(');
        if (n != line.npos) {
            while (n > 0 && line[n-1] == ' ') {
                --n;
                line.erase(n);
            }
        }

        /* Parse it */
        n = 0;
        if (line.size() != 0 && parseHex(line, n, nchar)) {
            /* Line starts with a hex code */
            skipWhitespace(line, n);
            if (n < line.size() && line[n] != '<') {
                /* It's a real name */
                m_names[nchar] = afl::string::strUCase(line.substr(n));
                current = NONE;
            } else {
                /* Not a real name. Note number for later */
                current = nchar;
            }
        } else {
            /* Line does not start with a hex code */
            skipWhitespace(line, n);
            if (current != NONE && n < line.size() && line[n] == '=') {
                /* Alternate name line for a character with bad name */
                ++n;
                skipWhitespace(line, n);
                if (n < line.size()) {
                    m_names[current] = afl::string::strUCase(line.substr(n));
                    current = NONE;
                }
            }
        }
    }
}

void
util::CharacterNameList::loadAliases(afl::io::Stream& in)
{
    // ex loadCharacterNames() (part)
    afl::io::TextFile tf(in);
    String_t line;
    while (tf.readLine(line)) {
        String_t::size_type n = line.find('=');
        if (n != String_t::npos) {
            m_aliases.push_back(std::make_pair(afl::string::strTrim(line.substr(0, n)),
                                               afl::string::strTrim(line.substr(n+1))));
        }
    }
}

String_t
util::CharacterNameList::getCharacterName(afl::charset::Unichar_t ch) const
{
    // ex getCharacterName(uint32_t ch)
    String_t result = Format("U+%04X", ch);

    NameMap_t::const_iterator i = m_names.find(ch);
    if (i != m_names.end()) {
        /* Name known from table */
        result += ' ';
        result += i->second;
    } else if (ch > ' ' && ch < 127) {
        /* No name table loaded, but it's an ASCII character */
        result += " ";
        result += char(ch);
    }

    return result;
}

afl::charset::Unichar_t
util::CharacterNameList::findCharacterByName(const String_t& name) const
{
    // getCharacterByName(string_t name)
    for (NameMap_t::const_iterator it = m_names.begin(); it != m_names.end(); ++it) {
        if (it->second == name) {
            return it->first;
        }
    }
    return 0;
}

util::CharacterNameList::CharacterList_t
util::CharacterNameList::searchCharactersByName(String_t expr, afl::charset::Unichar_t maxCharCode)
{
    // matchCharacterNames(string_t expr, const uint32_t limit, CharacterList& out)
    // Clear output. Quick exit if expression is empty
    CharacterList_t result;
    if (!expr.empty()) {
        // Collect characters and their values
        std::map<Unichar_t, int> values;

        // Check for "U+nnnn" match
        if (expr.size() >= 2) {
            String_t::size_type n = 0;
            skipWhitespace(expr, n);
            if (expr.size() - n > 3 && (expr[n] == 'U' || expr[n] == 'u') && expr[n+1] == '+') {
                n += 2;
            }
            Unichar_t result;
            if (parseHex(expr, n, result) && result <= maxCharCode) {
                skipWhitespace(expr, n);
                if (n == expr.size()) {
                    // Match
                    values[result] += 1000;
                }
            }
        }

        // Check for single character match
        afl::charset::Utf8Reader rdr(afl::string::toBytes(expr), 0);
        if (rdr.hasMore()) {
            Unichar_t ch = rdr.eat();
            if (!rdr.hasMore()) {
                values[ch] += 900;
            }
        }

        // Check for name match
        expr = afl::string::strUCase(expr);
        for (NameMap_t::const_iterator i = m_names.begin(); i != m_names.end() && i->first <= maxCharCode; ++i) {
            if (i->second.find("LETTER " + expr) != String_t::npos) {
                values[i->first] += 800;
            } else if (i->second.find("DIGIT " + expr) != String_t::npos) {
                values[i->first] += 700;
            } else if (i->second.find(" " + expr) != String_t::npos) {
                values[i->first] += 600;
            } else {
                String_t::size_type n = i->second.find(expr);
                if (n == 0) {
                    values[i->first] += 600;
                } else if (n != String_t::npos) {
                    values[i->first] += 500;
                } else {
                    // No match at all
                }
            }
        }

        // Generate output
        std::vector<std::pair<Unichar_t, int> > tmp(values.begin(), values.end());
        std::sort(tmp.begin(), tmp.end(), CompareValues());
        for (std::vector<std::pair<Unichar_t, int> >::iterator i = tmp.begin(); i != tmp.end(); ++i) {
            result.push_back(i->first);
        }
    }
    return result;
}

bool
util::CharacterNameList::generateCharacter(afl::charset::Unichar_t ch, Generator& gen) const
{
    // RHCharacterGenerator::generate(uint32_t id)
    Unichar_t result[10];
    size_t rsize = 1;

    // Figure out character name. Only with the name, we can do something sensible
    NameMap_t::const_iterator i = m_names.find(ch);
    if (i == m_names.end()) {
        return false;
    }

    // Does it have an accent?
    String_t name = i->second;
    String_t::size_type n = name.find(" WITH ");
    if (n != String_t::npos) {
        Unichar_t accent = findCharacterByName("COMBINING " + name.substr(n + 6));
        if (accent == 0) {
            accent = findCharacterByName("COMBINING " + name.substr(n + 6) + " ACCENT");
        }
        if (accent != 0) {
            result[1] = accent;
            rsize = 2;
            name.erase(n);
        }
    }

    // Check trivial character
    Unichar_t basechar = findCharacterByName(name);
    if (basechar != 0 && rsize != 1) {
        result[0] = basechar;
        if (gen.check(afl::base::Memory<Unichar_t>::unsafeCreate(result, rsize))) {
            return true;
        }
    }

    // Check aliases
    for (size_t index = 0; index < m_aliases.size(); ++index) {
        // Does it have a variant?
        String_t::size_type varPos = m_aliases[index].first.find('?');
        String_t::size_type repPos = m_aliases[index].second.find('?');
        const String_t& a = m_aliases[index].first;
        const String_t& b = m_aliases[index].second;
        if (varPos == String_t::npos || repPos == String_t::npos) {
            // Check direct alias
            if (name == a && (basechar = findCharacterByName(b)) != 0) {
                result[0] = basechar;
                if (gen.check(afl::base::Memory<Unichar_t>::unsafeCreate(result, rsize))) {
                    return true;
                }
            }
        } else {
            // Check variations
            static const char*const variations[] = {
                "CAPITAL LETTER",
                "SMALL LETTER"
            };
            for (size_t v = 0; v < countof(variations); ++v) {
                if (name == a.substr(0, varPos) + variations[v] + a.substr(varPos+1)
                    && (basechar = findCharacterByName(b.substr(0, repPos) + variations[v] + b.substr(repPos+1))) != 0)
                {
                    result[0] = basechar;
                    if (gen.check(afl::base::Memory<Unichar_t>::unsafeCreate(result, rsize))) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}
