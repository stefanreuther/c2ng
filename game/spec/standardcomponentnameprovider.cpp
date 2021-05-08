/**
  *  \file game/spec/standardcomponentnameprovider.cpp
  *  \brief Class game::spec::StandardComponentNameProvider
  *
  *  We need short names/abbreviations for all units.
  *  They are defined by a specification file "names.cc", supported by PCC since 1.1.12 / 1.99.17.
  *  This module implements short name management as a ComponentNameProvider.
  *  It could also be a home to translated unit names.
  */

#include "game/spec/standardcomponentnameprovider.hpp"
#include "afl/base/countof.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/string/format.hpp"
#include "afl/string/string.hpp"
#include "util/fileparser.hpp"

using afl::string::Format;

namespace {
    static const char*const NAMES[] = {
        "HULLS.SHORT",
        "ENGINES.SHORT",
        "BEAMS.SHORT",
        "TORPS.SHORT",
    };

    // Above array must agree with ComponentNameProvider enum values.
    static_assert(game::spec::ComponentNameProvider::Hull == 0, "game::spec::ComponentNameProvider::Hull");
    static_assert(game::spec::ComponentNameProvider::Engine == 1, "game::spec::ComponentNameProvider::Engine");
    static_assert(game::spec::ComponentNameProvider::Beam == 2, "game::spec::ComponentNameProvider::Beam");
    static_assert(game::spec::ComponentNameProvider::Torpedo == 3, "game::spec::ComponentNameProvider::Torpedo");
}

class game::spec::StandardComponentNameProvider::NameFileParser : public util::FileParser {
 public:
    static_assert(countof(NAMES) == NUM_TRANSLATIONS, "countof(NAMES)");

    typedef std::map<String_t, String_t> Map_t;

    NameFileParser(StandardComponentNameProvider& parent, afl::string::Translator& tx, afl::sys::LogListener& log)
        : util::FileParser(";#"),
          m_parent(parent),
          m_translator(tx),
          m_log(log),
          m_section(0)
        { }

    virtual void handleLine(const String_t& fileName, int lineNr, String_t line)
        {
            // ex NameFileParser::process
            String_t::size_type n = line.find_first_not_of(" \t");
            if (n == String_t::npos) {
                // cannot happen
            } else if (line[n] == '[') {
                // section
                String_t::size_type p = line.find(']');
                if (p == String_t::npos) {
                    handleError(fileName, lineNr);
                } else {
                    // new section
                    const String_t sectionName(afl::string::strUCase(String_t(line, n+1, p-n-1)));
                    m_section = 0;
                    for (size_t i = 0; i < NUM_TRANSLATIONS; ++i) {
                        if (sectionName == NAMES[i]) {
                            m_section = &m_parent.m_translations[i];
                        }
                    }
                }
            } else if (m_section != 0) {
                String_t::size_type p = line.find('=');
                if (p == String_t::npos) {
                    // syntax error
                    handleError(fileName, lineNr);
                } else {
                    // translation
                    const String_t name(afl::string::strUCase(afl::string::strRTrim(String_t(line, n, p-n))));
                    const String_t value(afl::string::strTrim(String_t(line, p+1)));

                    // FIXME: PCC2 handled numbered entries here.
                    Map_t& tx = *m_section;
                    if (tx.find(name) == tx.end()) {
                        tx.insert(std::make_pair(name, value));
                    }
                }
            } else {
                // not in a section
            }
        }
    virtual void handleIgnoredLine(const String_t& /*fileName*/, int /*lineNr*/, String_t /*line*/)
        { }

    void handleError(const String_t& fileName, int lineNr)
        {
            // ex NameFileParser::syntaxError
            m_log.write(m_log.Warn, "game.spec.componentname", fileName, lineNr, m_translator.translateString("Syntax error, line has been ignored"));
        }

 private:
    StandardComponentNameProvider& m_parent;
    afl::string::Translator& m_translator;
    afl::sys::LogListener& m_log;
    Map_t* m_section;
};


game::spec::StandardComponentNameProvider::StandardComponentNameProvider()
{ }

game::spec::StandardComponentNameProvider::~StandardComponentNameProvider()
{ }

String_t
game::spec::StandardComponentNameProvider::getName(Type /*type*/, int /*index*/, const String_t& name) const
{
    // We do not mess with normal names for now
    return name;
}

String_t
game::spec::StandardComponentNameProvider::getShortName(Type type, int /*index*/, const String_t& name, const String_t& shortName) const
{
    // ex game/specsn.cc:getShortName, but totally modified
    // PCC2 would special-case fighters here; we set their short name in the Fighter constructor.
    if (!shortName.empty()) {
        return shortName;
    } else {
        std::map<String_t, String_t>::const_iterator it = m_translations[type].find(afl::string::strUCase(name));
        if (it != m_translations[type].end()) {
            return it->second;
        } else {
            return name;
        }
    }
}

void
game::spec::StandardComponentNameProvider::clear()
{
    for (size_t i = 0; i < NUM_TRANSLATIONS; ++i) {
        m_translations[i].clear();
    }
}

void
game::spec::StandardComponentNameProvider::load(afl::io::Directory& dir, afl::string::Translator& tx, afl::sys::LogListener& log)
{
    // ex game/specsn.cc:initShortNames
    NameFileParser parser(*this, tx, log);
    afl::base::Ptr<afl::io::Stream> file;

    // Load user files. Language-specific file first
    String_t code = tx("{languageCode}");
    if (!code.empty() && code[0] != '{') {
        file = dir.openFileNT(Format("names_%s.usr", code), afl::io::FileSystem::OpenRead);
        if (file.get() != 0) {
            parser.parseFile(*file);
        }
        file = dir.openFileNT(Format("names_%s.cc", code), afl::io::FileSystem::OpenRead);
        if (file.get() != 0) {
            parser.parseFile(*file);
        }
    }

    // System file
    file = dir.openFileNT("names.usr", afl::io::FileSystem::OpenRead);
    if (file.get() != 0) {
        parser.parseFile(*file);
    }
    file = dir.openFileNT("names.cc", afl::io::FileSystem::OpenRead);
    if (file.get() != 0) {
        parser.parseFile(*file);
    }
}
