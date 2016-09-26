/**
  *  \file game/config/configurationparser.cpp
  *  \brief Class game::config::ConfigurationParser
  */

#include <stdexcept>
#include "game/config/configurationparser.hpp"
#include "game/config/configuration.hpp"
#include "util/translation.hpp"

game::config::ConfigurationParser::ConfigurationParser(afl::sys::LogListener& log, Configuration& config, ConfigurationOption::Source source)
    : util::ConfigurationFileParser(),
      m_log(log),
      m_config(config),
      m_source(source)
{
    // ex ConfigFileParser::ConfigFileParser
}

void
game::config::ConfigurationParser::handleAssignment(const String_t& fileName, int lineNr, const String_t& name, const String_t& value, const String_t& /*line*/)
{
    // ex ConfigFileParser::assign
    try {
        m_config.setOption(name, value, m_source);
    }
    catch(std::domain_error& e) {
        handleError(fileName, lineNr, e.what());
    }
    catch(std::runtime_error& e) {
        handleError(fileName, lineNr, e.what());
    }
}

void
game::config::ConfigurationParser::handleError(const String_t& fileName, int lineNr, const String_t& message)
{
    // ex ConfigFileParser::error
    m_log.write(m_log.Warn, "game.config.parser", fileName, lineNr, message + _("; line has been ignored"));
}

void
game::config::ConfigurationParser::handleIgnoredLine(const String_t& /*fileName*/, int /*lineNr*/, String_t /*line*/)
{ }
