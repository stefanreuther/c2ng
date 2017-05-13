/**
  *  \file server/configurationhandler.cpp
  *  \brief Class server::ConfigurationHandler
  */

#include <stdexcept>
#include "server/configurationhandler.hpp"
#include "afl/string/format.hpp"
#include "afl/io/stream.hpp"
#include "afl/io/textfile.hpp"

using afl::string::strUCase;
using afl::string::strTrim;
using afl::string::Format;

// Constructor.
server::ConfigurationHandler::ConfigurationHandler(afl::sys::LogListener& log, String_t logName)
    : m_log(log),
      m_logName(logName),
      m_commandLineKeys(),
      m_configFileName()
{ }

// Destructor.
server::ConfigurationHandler::~ConfigurationHandler()
{ }

// Process a command-line option ("-D", "--config").
bool
server::ConfigurationHandler::handleCommandLineOption(const String_t& option, afl::sys::CommandLineParser& parser)
{
    if (option.size() > 1 && option[0] == 'D') {
        String_t key(strUCase(option.substr(1)));
        String_t value(parser.getRequiredParameter(option));
        if (!handleConfiguration(key, value)) {
            throw std::runtime_error(Format("Unrecognized configuration setting: \"%s\"", key));
        }
        logConfiguration(key, value);
        m_commandLineKeys.insert(key);
        return true;
    } else if (option == "config") {
        m_configFileName = parser.getRequiredParameter(option);
        return true;
    } else {
        return false;
    }
}

// Load configuration file.
void
server::ConfigurationHandler::loadConfigurationFile(afl::sys::Environment& env, afl::io::FileSystem& fs)
{
    // ex server/config.cc:loadConfiguration (sort-of)
    // Get file name
    String_t fileName = m_configFileName;
    if (fileName.empty()) {
        fileName = env.getEnvironmentVariable("C2CONFIG");
    }
    if (fileName.empty()) {
        m_log.write(afl::sys::LogListener::Warn, m_logName, "Environment variable C2CONFIG not set, using defaults");
        return;
    }

    // Read the file
    afl::base::Ref<afl::io::Stream> file(fs.openFile(fileName, afl::io::FileSystem::OpenRead));
    afl::io::TextFile tf(*file);
    String_t line;
    while (tf.readLine(line)) {
        String_t::size_type i = line.find_first_of("#=");
        if (i != line.npos && line[i] == '=') {
            const String_t key(strUCase(strTrim(String_t(line, 0, i))));
            if (m_commandLineKeys.find(key) == m_commandLineKeys.end()) {
                const String_t value(strTrim(String_t(line, i+1)));
                if (handleConfiguration(key, value)) {
                    logConfiguration(key, value);
                }
            }
        }
    }
}

void
server::ConfigurationHandler::logConfiguration(const String_t& key, const String_t& value)
{
    if (key.find(".KEY") == String_t::npos) {
        m_log.write(afl::sys::LogListener::Trace, m_logName, Format("Configuration: %s = %s", key, value));
    } else {
        // Avoid writing credentials to the log file
        m_log.write(afl::sys::LogListener::Trace, m_logName, Format("Configuration: %s = <redacted>", key));
    }
}
