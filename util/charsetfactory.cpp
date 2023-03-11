/**
  *  \file util/charsetfactory.cpp
  *  \brief Class util::CharsetFactory
  */

#include "util/charsetfactory.hpp"
#include "afl/base/countof.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/utf8charset.hpp"
#include "util/translation.hpp"

namespace {
    /*
     *  Factory Functions
     */
    template<typename T>
    afl::charset::Charset* createInstance()
    {
        return new T();
    }

    template<const afl::charset::Codepage& CP>
    afl::charset::Charset* createCodepage()
    {
        return new afl::charset::CodepageCharset(CP);
    }

    /*
     *  Definitions
     */
    struct CharsetDefinition {
        const char* primaryKey;
        const char* secondaryKey;
        const char* tertiaryKey;
        const char* englishName;
        const char* englishDescription;
        afl::charset::Charset* (*create)();
    };

    extern const afl::charset::Codepage PCC1_CODEPAGE = {{
            0x00c7, 0x00fc, 0x00e9, 0x00e2, 0x00e4, 0x00e0, 0x00e5, 0x00e7,
            0x00ea, 0x00eb, 0x00e8, 0x00ef, 0x00ee, 0x00ec, 0x00c4, 0x00c5,
            0x00c9, 0x00e6, 0x00c6, 0x00f4, 0x00f6, 0x00f2, 0x00fb, 0x00f9,
            0x00ff, 0x00d6, 0x00dc, 0x00a2, 0x00a3, 0x00a5, 0x2259, 0x00d7,
            0x00e1, 0x00ed, 0x00f3, 0x00fa, 0x00f1, 0x00d1, 0x00aa, 0x00ba,
            0x00bf, 0x00ae, 0x2122, 0x00bd, 0x00bc, 0x00a1, 0x00ab, 0x00bb,
            0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2525, 0x2528, 0x2512,
            0x2511, 0x252b, 0x2503, 0x2513, 0x251b, 0x251a, 0x2519, 0x2510,
            0x2514, 0x2534, 0x252c, 0x251c, 0x2500, 0x253c, 0x251d, 0x2520,
            0x2517, 0x250f, 0x253b, 0x2533, 0x2523, 0x2501, 0x254b, 0x2537,
            0x2538, 0x252f, 0x2530, 0x2516, 0x2515, 0x250d, 0x250e, 0x2542,
            0x253f, 0x2518, 0x250c, 0x2588, 0x2584, 0x25c0, 0x25b6, 0x2580,
            0x03b1, 0x00df, 0x0393, 0x03c0, 0x03a3, 0x03c3, 0x03bc, 0x03c4,
            0x03a6, 0x0398, 0x03a9, 0x03b4, 0x221e, 0x03c6, 0x03b5, 0x2229,
            0x2261, 0x00b1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00f7, 0x2248,
            0x00b0, 0x2219, 0x00b7, 0x221a, 0x207f, 0x00b2, 0x25a0, 0x00a0,
        }};

    const CharsetDefinition DEFINITIONS[] = {
        // primary  secondary  tertiary short name     long name                                              creator
        { "utf8",   0,             0,   N_("UTF-8"),   N_("Unicode (UTF-8)"),                                 &createInstance<afl::charset::Utf8Charset> },
        { "cp1250", "windows1250", 0,   N_("CP1250"),  N_("Windows Codepage 1250 (Eastern Europe)"),          &createCodepage<afl::charset::g_codepage1250> },
        { "cp1251", "windows1251", 0,   N_("CP1251"),  N_("Windows Codepage 1251 (Cyrillic)"),                &createCodepage<afl::charset::g_codepage1251> },
        { "cp1252", "windows1252", 0,   N_("CP1252"),  N_("Windows Codepage 1252 (Western Europe, Latin-1)"), &createCodepage<afl::charset::g_codepage1252> },
        { "cp437",  "ibm437",      0,   N_("CP437"),   N_("MS-DOS Codepage 437"),                             &createCodepage<afl::charset::g_codepage437> },
        { "cp850",  "ibm850",      0,   N_("CP850"),   N_("MS-DOS Codepage 850"),                             &createCodepage<afl::charset::g_codepage850> },
        { "cp852",  "ibm852",      0,   N_("CP852"),   N_("MS-DOS Codepage 852"),                             &createCodepage<afl::charset::g_codepage852> },
        { "cp866",  "ibm866",      0,   N_("CP866"),   N_("MS-DOS Codepage 866 (\"Alternative\" cyrillic)"),  &createCodepage<afl::charset::g_codepage866> },
        { "koi8r",  0,             0,   N_("KOI8-R"),  N_("Cyrillic (KOI8-R)"),                               &createCodepage<afl::charset::g_codepageKOI8R> },
        { "latin1", "iso88591", "ansi", N_("Latin-1"), N_("ISO 8859-1 (Latin-1, Windows/Unix)"),              &createCodepage<afl::charset::g_codepageLatin1> },
        { "latin2", "iso88592",    0,   N_("Latin-2"), N_("ISO 8859-2 (Latin-2)"),                            &createCodepage<afl::charset::g_codepageLatin2> },
        { "pcc1",   0,             0,   N_("PCC1"),    N_("PCC 1.x (almost Codepage 437)"),                   &createCodepage<PCC1_CODEPAGE> },
    };

    /*
     *  Helpers
     */

    bool match(const String_t& str, const char* compareTo)
    {
        return compareTo != 0 && afl::string::strCaseCompare(str, compareTo) == 0;
    }

    const CharsetDefinition* get(util::CharsetFactory::Index_t index)
    {
        return index < countof(DEFINITIONS) ? &DEFINITIONS[index] : 0;
    }
}

// Index for Unicode character set.
const util::CharsetFactory::Index_t util::CharsetFactory::UNICODE_INDEX;


// Get number of known character sets.
util::CharsetFactory::Index_t
util::CharsetFactory::getNumCharsets() const
{
    return countof(DEFINITIONS);
}

// Create character set.
afl::charset::Charset*
util::CharsetFactory::createCharsetByIndex(Index_t index) const
{
    if (const CharsetDefinition* def = get(index)) {
        return def->create();
    } else {
        return 0;
    }
}

// Get key for a character set.
String_t
util::CharsetFactory::getCharsetKey(Index_t index) const
{
    if (const CharsetDefinition* def = get(index)) {
        return def->primaryKey;
    } else {
        return String_t();
    }
}

// Get name for a character set.
String_t
util::CharsetFactory::getCharsetName(Index_t index, afl::string::Translator& tx) const
{
    if (const CharsetDefinition* def = get(index)) {
        return tx.translateString(def->englishName);
    } else {
        return String_t();
    }
}

// Get description for a character set.
String_t
util::CharsetFactory::getCharsetDescription(Index_t index, afl::string::Translator& tx) const
{
    if (const CharsetDefinition* def = get(index)) {
        return tx.translateString(def->englishDescription);
    } else {
        return String_t();
    }
}

// Look up a key, producing an index.
bool
util::CharsetFactory::findIndexByKey(String_t name, Index_t& result) const
{
    // Remove dashes. "ISO-8859-1" is the same as "iso88591".
    String_t::size_type n;
    while ((n = name.find('-')) != String_t::npos) {
        name.erase(n, 1);
    }

    // Find
    for (size_t i = 0; i < countof(DEFINITIONS); ++i) {
        if (match(name, DEFINITIONS[i].primaryKey) || match(name, DEFINITIONS[i].secondaryKey) || match(name, DEFINITIONS[i].tertiaryKey)) {
            result = i;
            return true;
        }
    }
    return false;
}

// CharsetFactory:
afl::charset::Charset*
util::CharsetFactory::createCharset(String_t name)
{
    Index_t index;
    if (findIndexByKey(name, index)) {
        return createCharsetByIndex(index);
    } else {
        return 0;
    }
}
