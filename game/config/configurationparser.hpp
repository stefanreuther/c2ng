/**
  *  \file game/config/configurationparser.hpp
  *  \brief Class game::config::ConfigurationParser
  */
#ifndef C2NG_GAME_CONFIG_CONFIGURATIONPARSER_HPP
#define C2NG_GAME_CONFIG_CONFIGURATIONPARSER_HPP

#include "util/configurationfileparser.hpp"
#include "game/config/configurationoption.hpp"
#include "afl/sys/loglistener.hpp"

namespace game { namespace config {

    class Configuration;

    /** Configuration Parser.
        This uses a ConfigurationFileParser to fill in a Configuration object.
        Errors will be logged as warnings on a logger.

        Change from PCC2: we do not detect the "unknown parameter" case.
        Instead, class Configuration silently creates unknown parameters as strings. */
    class ConfigurationParser : public util::ConfigurationFileParser {
     public:
        /** Constructor.
            \param log Logger
            \param tx Translator
            \param config The configuration object to fill
            \param source Source to set for received values */
        ConfigurationParser(afl::sys::LogListener& log, afl::string::Translator& tx, Configuration& config, ConfigurationOption::Source source);

        virtual void handleAssignment(const String_t& fileName, int lineNr, const String_t& name, const String_t& value, const String_t& line);
        virtual void handleError(const String_t& fileName, int lineNr, const String_t& message);
        virtual void handleIgnoredLine(const String_t& fileName, int lineNr, String_t line);

     private:
        afl::sys::LogListener& m_log;
        Configuration& m_config;
        ConfigurationOption::Source m_source;
    };

} }

#endif
