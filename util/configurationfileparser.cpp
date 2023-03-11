/**
  *  \file util/configurationfileparser.cpp
  *  \brief Class util::ConfigurationFileParser
  */

#include "util/configurationfileparser.hpp"
#include "afl/string/string.hpp"

util::ConfigurationFileParser::ConfigurationFileParser(afl::string::Translator& tx)
    : FileParser("#"),
      m_translator(tx),
      m_sectionName(),
      m_inSection(true)
{
    // ex ConfigParser::ConfigParser
}

void
util::ConfigurationFileParser::setSection(String_t sectionName, bool inSection)
{
    // ex ConfigParser::configure
    m_sectionName = sectionName;
    m_inSection = inSection;
}

bool
util::ConfigurationFileParser::isInSection() const
{
    // ex ConfigParser::isInSection
    return m_inSection;
}

const String_t&
util::ConfigurationFileParser::getSectionName() const
{
    // ex ConfigParser::getSectionName
    return m_sectionName;
}

void
util::ConfigurationFileParser::handleLine(const String_t& fileName, int lineNr, String_t line)
{
    // ex ConfigParser::process

    // Remove whitespace
    String_t ppline = afl::string::strTrim(line);

    // blank line?
    if (ppline.empty()) {
        handleIgnoredLine(fileName, lineNr, line);
        return;
    }

    // Section delimiter?
    if (ppline[0] == '%') {
        if (afl::string::strCaseCompare(m_sectionName, afl::string::strLTrim(ppline.substr(1))) == 0) {
            m_inSection = true;
        } else {
            m_inSection = false;
        }
        handleIgnoredLine(fileName, lineNr, line);
        return;
    }

    // Process this line further?
    if (!m_inSection) {
        handleIgnoredLine(fileName, lineNr, line);
        return;
    }

    // It's an assignment
    String_t::size_type eqpos = ppline.find('=');
    if (eqpos == String_t::npos) {
        handleError(fileName, lineNr, m_translator("Syntax error"));
        return;
    }

    String_t key = afl::string::strRTrim(ppline.substr(0, eqpos));
    if (key.empty()) {
        handleError(fileName, lineNr, m_translator("Syntax error"));
        return;
    }

    handleAssignment(fileName, lineNr, key, afl::string::strLTrim(ppline.substr(eqpos+1)), line);
}
