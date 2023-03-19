/**
  *  \file game/config/expressionlists.cpp
  *  \brief Class game::config::ExpressionLists
  */

#include "game/config/expressionlists.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/string/format.hpp"
#include "util/fileparser.hpp"

using game::config::ExpressionLists;

namespace {
    /*
     *  Constants
     */
    const char*const LOG_NAME = "game.config";

    const char*const LRU_FILE = "lru.ini";

    static const char*const AREA_NAMES[game::config::ExpressionLists::NUM_AREAS] = {
        "SHIPLABELS",
        "PLANETLABELS",
        "FIND",
    };

    const size_t LRU_LIMIT = 5;


    /*
     *  ListFileParser - Parse expr.cc or lru.ini
     */

    class ListFileParser : public util::FileParser {
     public:
        ListFileParser(ExpressionLists::Kind k, ExpressionLists& container, afl::sys::LogListener& log, afl::string::Translator& tx);

        virtual void handleLine(const String_t& fileName, int lineNr, String_t line);
        virtual void handleIgnoredLine(const String_t& fileName, int lineNr, String_t line);

     private:
        void syntaxError(const String_t& fileName, int lineNr);

        ExpressionLists::Kind m_kind;
        ExpressionLists& m_container;
        util::ExpressionList* m_section;
        afl::sys::LogListener& m_log;
        afl::string::Translator& m_translator;
    };

    void saveListFile(afl::io::TextFile& tf, const char* name, const util::ExpressionList& list)
    {
        // ex saveLRUList
        if (!list.empty()) {
            tf.writeLine(afl::string::Format("[%s]", name));
            for (size_t index = 0, n = list.size(); index < n; ++index) {
                if (const util::ExpressionList::Item* it = list.get(index)) {
                    // Sanitize the name so we'll be able to re-parse it
                    // FIXME: this will fail if the line starts with '#', ';' or '[', but is good enough for now.
                    String_t name = it->name;
                    String_t::size_type i = 0;
                    bool deleting = true;
                    while (i < name.size()) {
                        uint8_t ch = static_cast<uint8_t>(name[i]);
                        if ((deleting && ch <= ' ') || ch < ' ') {
                            name.erase(i, 1);
                        } else {
                            ++i;
                        }
                        deleting = (ch <= ' ');
                    }

                    // Save
                    tf.writeText(name);
                    tf.writeText("  ");
                    tf.writeText(it->flags);
                    tf.writeLine(it->value);
                }
            }
            tf.writeLine();
        }
    }
}

ListFileParser::ListFileParser(ExpressionLists::Kind k, ExpressionLists& container, afl::sys::LogListener& log, afl::string::Translator& tx)
    : FileParser(";#"),
      m_kind(k),
      m_container(container),
      m_section(0),
      m_log(log),
      m_translator(tx)
{
    // ex client/lrupredef.cc:ListFileParser::ListFileParser
    setCharsetNew(new afl::charset::Utf8Charset());
}

void
ListFileParser::handleLine(const String_t& fileName, int lineNr, String_t line)
{
    // ex ListFileParser::process
    String_t::size_type n = line.find_first_not_of(" \t");
    if (n == String_t::npos) {
        // cannot happen
    } else if (line[n] == '[') {
        // section
        String_t::size_type p = line.find(']', n);
        if (p == String_t::npos) {
            syntaxError(fileName, lineNr);
        } else {
            ExpressionLists::Area a;
            if (ExpressionLists::parseArea(afl::string::strUCase(String_t(line, n+1, p-n-1)), a)) {
                m_section = m_container.get(a, m_kind);
            } else {
                m_section = 0;
            }
        }
    } else if (m_section != 0) {
        // line in section
        String_t::size_type p = line.find("  ", n);
        if (p == String_t::npos) {
            syntaxError(fileName, lineNr);
        } else {
            String_t name(line, n, p-n);
            String_t value = afl::string::strTrim(line.substr(p+1));
            String_t flags;
            if (!value.empty() && value[0] == '[') {
                String_t::size_type pos = value.find(']');
                if (pos != String_t::npos) {
                    flags = value.substr(0, pos+1);
                    value = afl::string::strLTrim(value.substr(pos+1));
                }
            }

            m_section->pushBackNew(new util::ExpressionList::Item(name, flags, value));
        }
    } else {
        // unknown section
    }
}

void
ListFileParser::handleIgnoredLine(const String_t& /*fileName*/, int /*lineNr*/, String_t /*line*/)
{ }

void
ListFileParser::syntaxError(const String_t& fileName, int lineNr)
{
    // ex ListFileParser::syntaxError
    m_log.write(afl::sys::LogListener::Warn, LOG_NAME, afl::string::Format(m_translator("%s:%d: file format error -- line ignored"), fileName, lineNr));
}


/*
 *  ExpressionList
 */

game::config::ExpressionLists::ExpressionLists()
{ }

game::config::ExpressionLists::~ExpressionLists()
{ }

util::ExpressionList*
game::config::ExpressionLists::get(Area a, Kind k)
{
    // ex client/lrupredef.cc:getListByName [part]
    return &m_data[k][a];
}

const util::ExpressionList*
game::config::ExpressionLists::get(Area a, Kind k) const
{
    return &m_data[k][a];
}

void
game::config::ExpressionLists::loadRecentFiles(util::ProfileDirectory& profile, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    // ex loadLRULists
    clearAll(Recent);

    afl::base::Ptr<afl::io::Stream> file = profile.openFileNT(LRU_FILE);
    if (file.get() != 0) {
        ListFileParser(Recent, *this, log, tx).parseFile(*file);
    }
}

void
game::config::ExpressionLists::saveRecentFiles(util::ProfileDirectory& profile, afl::sys::LogListener& log, afl::string::Translator& tx) const
{
    // ex saveLRULists
    try {
        afl::base::Ref<afl::io::Stream> file = profile.createFile(LRU_FILE);
        afl::io::TextFile tf(*file);
        tf.setCharsetNew(new afl::charset::Utf8Charset());
        for (size_t i = 0; i < NUM_AREAS; ++i) {
            saveListFile(tf, AREA_NAMES[i], *get(Area(i), Recent));
        }
        tf.flush();
    }
    catch (std::exception& e) {
        log.write(afl::sys::LogListener::Warn, LOG_NAME, tx("Unable to create file"), e);
    }
}

void
game::config::ExpressionLists::loadPredefinedFiles(util::ProfileDirectory& profile, afl::io::Directory& dir, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    // ex loadPredefinedLists
    clearAll(Predefined);

    ListFileParser p(Predefined, *this, log, tx);
    afl::base::Ptr<afl::io::Stream> file = profile.openFileNT("expr.ini");
    if (file.get() != 0) {
        p.parseFile(*file);
    }
    p.parseOptionalFile(dir, "expr.cc");
    p.parseOptionalFile(dir, "expr.usr");
}

void
game::config::ExpressionLists::clear()
{
    clearAll(Predefined);
    clearAll(Recent);
}

void
game::config::ExpressionLists::pack(Items_t& out, Area a, afl::string::Translator& tx) const
{
    // ex WLRUPopup::doPopup (part)
    // Figure out whether we have to use headings
    size_t numNonEmptyLists = 0;
    for (size_t k = 0; k < NUM_KINDS; ++k) {
        if (!get(a, Kind(k))->empty()) {
            ++numNonEmptyLists;
        }
    }
    if (numNonEmptyLists == 0) {
        // Nothing to show
        return;
    }

    // Build the list
    for (size_t k = 0; k < NUM_KINDS; ++k) {
        const util::ExpressionList& list = *get(a, Kind(k));
        if (!list.empty()) {
            if (numNonEmptyLists != 1) {
                out.push_back(Item(getHeading(a, Kind(k), tx)));
            }
            for (size_t j = 0; j < list.size(); ++j) {
                if (const util::ExpressionList::Item* p = list.get(j)) {
                    out.push_back(*p);
                }
            }
        }
    }
}

void
game::config::ExpressionLists::pushRecent(Area a, String_t flags, String_t expr)
{
    if (!get(a, Predefined)->findIndexForValue(expr).isValid()) {
        get(a, Recent)->pushFrontNew(new util::ExpressionList::Item(expr, flags, expr), LRU_LIMIT);
    }
}

bool
game::config::ExpressionLists::parseArea(const String_t& area, Area& result)
{
    // ex client/lrupredef.cc:getListByName [part]
    for (size_t i = 0; i < NUM_AREAS; ++i) {
        if (AREA_NAMES[i] == area) {
            result = static_cast<Area>(i);
            return true;
        }
    }
    return false;
}

void
game::config::ExpressionLists::clearAll(Kind k)
{
    for (size_t i = 0; i < NUM_AREAS; ++i) {
        get(Area(i), k)->clear();
    }
}

String_t
game::config::ExpressionLists::getHeading(Area a, Kind k, afl::string::Translator& tx)
{
    switch (a) {
     case ShipLabels:
     case PlanetLabels:
        switch (k) {
         case Recent:
            return tx("Last expressions");
         case Predefined:
            return tx("Predefined expressions");
        }
        break;

     case Search:
        switch (k) {
         case Recent:
            return tx("Last queries");
         case Predefined:
            return tx("Predefined queries");
        }
    }
    return String_t();
}
