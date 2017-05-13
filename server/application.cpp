/**
  *  \file server/application.cpp
  */

#include "server/application.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/nulltextwriter.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/longcommandlineparser.hpp"
#include "server/configurationhandler.hpp"

using afl::string::Format;

namespace {
    const char LOG_NAME[] = "server";

    afl::base::Ref<afl::io::TextWriter> getWriter(afl::sys::Environment& env, afl::sys::Environment::Channel ch)
    {
        try {
            return env.attachTextWriter(ch);
        }
        catch (...) {
            return *new afl::io::NullTextWriter();
        }
    }
}

class server::Application::ConfigurationHandler : public server::ConfigurationHandler {
 public:
    ConfigurationHandler(server::Application& app)
        : server::ConfigurationHandler(app.log(), LOG_NAME),
          m_app(app)
        { }
    bool handleConfiguration(const String_t& key, const String_t& value)
        { return m_app.handleConfiguration(key, value); }
 private:
    Application& m_app;
};

server::Application::Application(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net)
    : m_environment(env),
      m_fileSystem(fs),
      m_networkStack(net),
      m_logger(),
      m_errorOutput(getWriter(env, env.Error)),
      m_standardOutput(getWriter(env, env.Output)),
      m_configFileName()
{
    m_logger.attachWriter(false, m_standardOutput.asPtr());
    m_logger.attachWriter(true, m_errorOutput.asPtr());
}

int
server::Application::run()
{
    try {
        {
            ConfigurationHandler handler(*this);
            parseCommandLine(handler);
            handler.loadConfigurationFile(environment(), fileSystem());
        }
        serverMain();
        m_standardOutput->flush();
        m_errorOutput->flush();
        return 0;
    }
    catch (afl::except::FileProblemException& e) {
        reportError(e.getFileName() + ": " + e.what());
        return 1;
    }
    catch (std::exception& e) {
        reportError(e.what());
        return 1;
    }
    catch (...) {
        reportError("Uncaught exception");
        return 1;
    }
}

afl::sys::Environment&
server::Application::environment()
{
    return m_environment;
}

afl::io::FileSystem&
server::Application::fileSystem()
{
    return m_fileSystem;
}

afl::net::NetworkStack&
server::Application::networkStack()
{
    return m_networkStack;
}

afl::sys::LogListener&
server::Application::log()
{
    return m_logger;
}

void
server::Application::reportError(String_t str)
{
    m_errorOutput->writeLine(Format("%s: %s", m_environment.getInvocationName(), str));
    m_standardOutput->flush();
    m_errorOutput->flush();
}

void
server::Application::parseCommandLine(ConfigurationHandler& handler)
{
    afl::sys::LongCommandLineParser parser(environment().getCommandLine());
    bool option;
    String_t text;
    while (parser.getNext(option, text)) {
        if (!option) {
            throw std::runtime_error(Format("Unexpected parameter: \"%s\"", text));
        }
        if (handler.handleCommandLineOption(text, parser)) {
            // ok, "-D" or "--config"
        } else if (text == "log") {
            m_logger.setConfiguration(parser.getRequiredParameter(text));
        } else {
            if (!handleCommandLineOption(text, parser)) {
                throw std::runtime_error(Format("Unrecognized command line option: \"-%s\"", text));
            }
        }
    }
}
