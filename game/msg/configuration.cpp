/**
  *  \file game/msg/configuration.cpp
  *  \brief Class game::msg::Configuration
  */

#include "game/msg/configuration.hpp"
#include "util/configurationfile.hpp"
#include "afl/string/format.hpp"
#include "afl/io/textfile.hpp"

namespace {
    using util::ConfigurationFile;
    using afl::string::Format;
    using afl::base::Ptr;
    using afl::io::Stream;
    using afl::io::FileSystem;

    const char*const MESSAGE_CONFIG_TEMPLATE = "msg%d.ini";

    const char*const FILTER_KEY = "FILTER";

    void loadMessageConfiguration(afl::io::Directory& dir, int playerNr, ConfigurationFile& file)
    {
        // Whitespace handling:
        // - PCC1: always significant
        // - PCC2: not significant (but does not support SIG= where it would matter)
        file.setWhitespaceIsSignificant(true);

        // Load file
        Ptr<Stream> in(dir.openFileNT(Format(MESSAGE_CONFIG_TEMPLATE, playerNr), FileSystem::OpenRead));
        if (in.get() != 0) {
            afl::io::TextFile tf(*in);
            // FIXME ...setCharacterSet(getGameCharacterSet());
            file.load(tf);
        }
    }
}


game::msg::Configuration::Configuration()
    : m_filteredHeadings()
{ }

game::msg::Configuration::~Configuration()
{ }

bool
game::msg::Configuration::isHeadingFiltered(const String_t& heading) const
{
    // ex GMessageFilter::isFiltered, readmsg.pas:IsKilled
    return m_filteredHeadings.find(heading) != m_filteredHeadings.end();
}

void
game::msg::Configuration::toggleHeadingFiltered(const String_t& heading)
{
    // ex GMessageFilter::toggleFilter
    Filter_t::const_iterator it = m_filteredHeadings.find(heading);
    if (it != m_filteredHeadings.end()) {
        m_filteredHeadings.erase(it);
    } else {
        m_filteredHeadings.insert(heading);
    }
}

void
game::msg::Configuration::setHeadingFiltered(const String_t& heading, bool flag)
{
    // ex GMessageFilter::addToFilter, GMessageFilter::removeFromFilter, readmsg.pas:AddKillfile, readmsg.pas:RemoveKillfile
    Filter_t::const_iterator it = m_filteredHeadings.find(heading);
    if (it != m_filteredHeadings.end()) {
        // It's filtered. If it should not, remove it.
        if (!flag) {
            m_filteredHeadings.erase(it);
        }
    } else {
        // It's not filtered. If it should, add it.
        if (flag) {
            m_filteredHeadings.insert(heading);
        }
    }
}

void
game::msg::Configuration::clear()
{
    // ex GMessageFilter::clear
    m_filteredHeadings.clear();
}

void
game::msg::Configuration::load(afl::io::Directory& dir, int playerNr)
{
    // ex loadMessageConfig
    // Load
    ConfigurationFile file;
    loadMessageConfiguration(dir, playerNr, file);

    // Process
    // FIXME: "signature" feature from PCC1 (missing in PCC2)
    clear();
    for (size_t i = 0, n = file.getNumElements(); i < n; ++i) {
        if (const ConfigurationFile::Element* ele = file.getElementByIndex(i)) {
            if (ele->type == ConfigurationFile::Assignment) {
                if (ele->key == FILTER_KEY) {
                    m_filteredHeadings.insert(ele->value);
                }
            }
        }
    }
}

void
game::msg::Configuration::save(afl::io::Directory& dir, int playerNr) const
{
    // ex saveMessageConfig
    ConfigurationFile file;
    loadMessageConfiguration(dir, playerNr, file);

    // Update filters
    while (file.remove(FILTER_KEY)) {
        // nix
    }
    for (Filter_t::const_iterator it = m_filteredHeadings.begin(); it != m_filteredHeadings.end(); ++it) {
        file.add(FILTER_KEY, *it);
    }

    // Header
    file.addHeaderComment("# PCC2 Message Configuration File", false);

    // Rewrite file
    String_t fileName = Format(MESSAGE_CONFIG_TEMPLATE, playerNr);
    if (file.hasAssignments()) {
        Ptr<Stream> in(dir.openFileNT(fileName, FileSystem::Create));
        if (in.get() != 0) {
            afl::io::TextFile tf(*in);
            // FIXME ...setCharacterSet(getGameCharacterSet());
            file.save(tf);
            tf.flush();
        }
    } else {
        dir.eraseNT(fileName);
    }
}
