/**
  *  \file server/application.cpp
  *  \brief Base class server::Application
  */

#include "server/application.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/nulltextwriter.hpp"
#include "afl/net/resp/client.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "afl/sys/thread.hpp"
#include "server/configurationhandler.hpp"
#include "server/interface/baseclient.hpp"
#include "server/ports.hpp"
#include "util/string.hpp"

using afl::string::Format;

namespace {
    afl::base::Ref<afl::io::TextWriter> getWriter(afl::sys::Environment& env, afl::sys::Environment::Channel ch)
    {
        try {
            return env.attachTextWriter(ch);
        }
        catch (...) {
            return *new afl::io::NullTextWriter();
        }
    }

    class Exit {
     public:
        Exit(int n)
            : m_exitCode(n)
            { }
        int get()
            { return m_exitCode; }
     private:
        int m_exitCode;
    };
}

class server::Application::ConfigurationHandler : public server::ConfigurationHandler {
 public:
    ConfigurationHandler(server::Application& app)
        : server::ConfigurationHandler(app.log(), app.m_logName),
          m_app(app)
        { }
    bool handleConfiguration(const String_t& key, const String_t& value)
        { return m_app.handleConfiguration(key, value); }
 private:
    Application& m_app;
};

server::Application::Application(const String_t& logName, afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net)
    : m_logName(logName),
      m_environment(env),
      m_fileSystem(fs),
      m_networkStack(net),
      m_deleter(),
      m_clientNetworkStack(net),
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
    catch (Exit& n) {
        return n.get();
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

afl::net::NetworkStack&
server::Application::clientNetworkStack()
{
    return m_clientNetworkStack;
}

afl::sys::LogListener&
server::Application::log()
{
    return m_logger;
}

afl::io::TextWriter&
server::Application::standardOutput()
{
    return *m_standardOutput;
}

void
server::Application::exit(int n)
{
    m_standardOutput->flush();
    m_errorOutput->flush();
    throw Exit(n);
}

afl::net::CommandHandler&
server::Application::createClient(const afl::net::Name& name, afl::base::Deleter& del, bool stateless)
{
    // ex Connection::connectRetry
    // This used to do 5 loops x 1 second.
    // Doing the first loops in just 100 ms gives us faster automatic system tests.
    // Service initialisation might take some time now, so timing regime now is:
    //    5 x 0.1 second
    //   10 x   1 second
    //   10 x   5 seconds
    //   10 x  20 seconds
    // -> 260 seconds (similar to waitReady), 35 tries
    int count = 0;
    while (1) {
        try {
            afl::net::resp::Client& result = del.addNew(new afl::net::resp::Client(clientNetworkStack(), name));
            log().write(afl::sys::LogListener::Info, m_logName, afl::string::Format("Connected to %s", name.toString()));
            waitReady(result);
            if (stateless) {
                result.setReconnectMode(afl::net::Reconnectable::Always);
            }
            return result;
        }
        catch (std::exception& e) {
            if (count > 35) {
                throw;
            }
        }
        int sleepTime = count > 25 ? 20000 : count > 15 ? 5000 : count > 5 ? 1000 : 100;
        ++count;
        afl::sys::Thread::sleep(sleepTime);
    }
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
    afl::sys::StandardCommandLineParser parser(environment().getCommandLine());
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
        } else if (text == "proxy") {
            String_t url = parser.getRequiredParameter(text);
            if (!m_clientNetworkStack.add(url)) {
                throw std::runtime_error(Format("Unrecognized proxy URL: \"%s\"", url));
            }
        } else if (text == "h" || text == "help") {
            using afl::string::Format;
            afl::io::TextWriter& out = standardOutput();
            out.writeLine(getApplicationName());
            out.writeLine();
            out.writeLine(Format("Usage:\n"
                                 "  %s [-options]\n\n"
                                 "Options:\n"
                                 "%s"
                                 "\n"
                                 "Report bugs to <Streu@gmx.de>",
                                 environment().getInvocationName(),
                                 util::formatOptions(ConfigurationHandler::getHelp()
                                                     + "--log=CONFIG\tSet logger configuration\n"
                                                       "--proxy=URL\tAdd network proxy\n"
                                                     + getCommandLineOptionHelp())));
            exit(0);
        } else {
            if (!handleCommandLineOption(text, parser)) {
                throw std::runtime_error(Format("Unrecognized command line option: \"-%s\"", text));
            }
        }
    }
}

void
server::Application::waitReady(afl::net::CommandHandler& handler)
{
    // ex DbReadyClient::onConnect
    // This used to be done on the database only, but it doesn't hurt also doing it on other connections
    int count = 0;
    while (1) {
        try {
            server::interface::BaseClient(handler).ping();
            break;
        }
        catch (std::exception& e) {
            if (std::strncmp(e.what(), "LOADING", 7) == 0) {
                // 10 x 1 second
                // 10 x 5 seconds
                // 10 x 20 seconds
                if (count > 30) {
                    // 260 seconds should be enough
                    log().write(afl::sys::LogListener::Error, m_logName, "Server fails to become ready; giving up.");
                    throw;
                }
            } else {
                throw;
            }
        }
        int sleepTime = count > 20 ? 20 : count > 10 ? 5 : 1;
        ++count;
        log().write(afl::sys::LogListener::Trace, m_logName, afl::string::Format("Server not ready yet, sleeping %d seconds...", sleepTime));
        afl::sys::Thread::sleep(sleepTime * 1000);
    }
}
