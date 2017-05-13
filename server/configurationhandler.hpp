/**
  *  \file server/configurationhandler.hpp
  *  \brief Class server::ConfigurationHandler
  */
#ifndef C2NG_SERVER_CONFIGURATIONHANDLER_HPP
#define C2NG_SERVER_CONFIGURATIONHANDLER_HPP

#include <set>
#include "afl/io/filesystem.hpp"
#include "afl/string/string.hpp"
#include "afl/sys/commandlineparser.hpp"
#include "afl/sys/environment.hpp"
#include "afl/sys/loglistener.hpp"

namespace server {

    /** Configuration handling for server infrastructure.
        This implements the common handling of configuration:
        - "-Dkey=value" option
        - "--config=file" option and C2CONFIG environment variable, and associated config file

        <b>Usage:</b>
        - instantiate a descentdant that implements handleConfiguration().
        - call handleConfiguration() during command-line parsing
        - call loadConfigurationFile() to load the configuration file

        The ConfigurationHandler descendant need not stay around for longer than just that configuration parsing.
        ConfigurationHandler keeps internal state for one parsing run. */
    class ConfigurationHandler {
     public:
        /** Constructor.
            \param log Logger
            \param logName Name of log channel */
        ConfigurationHandler(afl::sys::LogListener& log, String_t logName);

        /** Destructor. */
        ~ConfigurationHandler();

        /** Process a command-line option ("-D", "--config").
            \param option Option name as obtained from CommandLineParser::getNext
            \param parser The CommandLineParser, used to obtain parameters
            \retval true Option has been processed
            \retval false Option not recognized
            \throw std::runtime_error the option was recognized, but its parameters were invalid */
        bool handleCommandLineOption(const String_t& option, afl::sys::CommandLineParser& parser);

        /** Load configuration file.
            \param env Environment instance
            \param fs File system instance
            \throw std::runtime_error as reported by file system
            \throw afl::except::FileProblemException as reported by file system */
        void loadConfigurationFile(afl::sys::Environment& env, afl::io::FileSystem& fs);

        /** Handle a configuration option.
            \param key Name of configuration option
            \param value Value
            \retval true Option recognized
            \retval false Option not recognized. If given on the command line, this is a hard error; in a config file, this is ignored. */
        virtual bool handleConfiguration(const String_t& key, const String_t& value) = 0;

     private:
        void logConfiguration(const String_t& key, const String_t& value);

        afl::sys::LogListener& m_log;
        const String_t m_logName;
        std::set<String_t> m_commandLineKeys;
        String_t m_configFileName;
    };

}

#endif
