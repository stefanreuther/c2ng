/**
  *  \file util/configurationfileparser.cpp
  */

#include "util/configurationfileparser.hpp"
#include "afl/string/string.hpp"
#include "util/translation.hpp"

// /** Configuration File Parser (pconfig.src alike)

//     This class encapsulates the logic for parsing PCONFIG.SRC-alike
//     configuration files. In a nutshell:
//     - sections separated by `% sectionname'
//     - comments starting with `\#'
//     - assignments `key = value' in each section

//     To parse a configuration file, derive a class from this and implement
//     at least the assign() method. */

util::ConfigurationFileParser::ConfigurationFileParser()
    : FileParser("#"),
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
        handleError(fileName, lineNr, _("Syntax error"));
        return;
    }

    String_t key = afl::string::strRTrim(ppline.substr(0, eqpos));
    if (key.empty()) {
        handleError(fileName, lineNr, _("Syntax error"));
        return;
    }

    handleAssignment(fileName, lineNr, key, afl::string::strLTrim(ppline.substr(eqpos+1)), line);
}


// /** Syntax error. This function is called when the parser encounters
//     a syntax error.

//     \param line_id line number
//     \param s       error message

//     \default Throws a FileFormatException. */
// void
// ConfigParser::error(int line_id, string_t s)
// {
//     throw FileFormatException(getFileName(), itoa(line_id) + ": " + s);
// }
