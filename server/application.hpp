/**
  *  \file server/application.hpp
  */
#ifndef C2NG_SERVER_APPLICATION_HPP
#define C2NG_SERVER_APPLICATION_HPP

#include "afl/base/deleter.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/sys/commandlineparser.hpp"
#include "afl/sys/environment.hpp"
#include "afl/sys/loglistener.hpp"
#include "util/consolelogger.hpp"
#include "afl/net/commandhandler.hpp"
#include "afl/net/name.hpp"
#include "afl/net/tunnel/tunnelablenetworkstack.hpp"

namespace server {

    class ConfigurationHandler;

    /** Base class for server application.
        A server application's primary objective is to run for a long time, unattended, on a network interface
        (unlike a util::Application, which typically runs short-lived with a rich command line interface).

        This aggregates a few common objects:
        - afl::sys::Environment instance
        - afl::io::FileSystem instance
        - afl::net::NetworkStack instance
        - a logger (currently: ConsoleLogger)

        It implements a standard command line and configuration file parser.

        You derive from Application and implement serverMain().
        Your main() function looks like
        <code>
          return MyServerApplication(env, fs, net).run();
        </code>

        This will also catch and log all exceptions your appMain() throws. */
    class Application : public afl::base::Deletable {
     public:
        /** Constructor.
            \param logName Logger name
            \param env Environment instance
            \param fs File system instance
            \param net Network stack instance */
        Application(const String_t& logName, afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net);

        /** Run the server.
            Invokes serverMain() with exception protection.
            \return return code (exit code) */
        int run();

        String_t getHelp() const;

        afl::sys::Environment& environment();
        afl::io::FileSystem& fileSystem();
        afl::net::NetworkStack& networkStack();
        afl::net::NetworkStack& clientNetworkStack();
        afl::sys::LogListener& log();

        afl::io::TextWriter& standardOutput();

        void exit(int n);

        /** Create a client to another microservice.
            \param name Network name
            \param del Deleter for created objects
            \param stateless true if this is a stateless connection (database, format).
                             In this case, it will be set to auto-reconnect.
                             In other cases, you have to deal with it.
            \return CommandHandler to access the microservice, allocated in the deleter. */
        afl::net::CommandHandler& createClient(const afl::net::Name& name, afl::base::Deleter& del, bool stateless);

     protected:
        /** Application.
            This contains what normally would be your main() function. */
        virtual void serverMain() = 0;

        virtual bool handleConfiguration(const String_t& key, const String_t& value) = 0;

        virtual bool handleCommandLineOption(const String_t& option, afl::sys::CommandLineParser& parser) = 0;

     private:
        class ConfigurationHandler;
        friend class ConfigurationHandler;

        String_t m_logName;
        afl::sys::Environment& m_environment;
        afl::io::FileSystem& m_fileSystem;
        afl::net::NetworkStack& m_networkStack;
        afl::base::Deleter m_deleter;
        afl::net::tunnel::TunnelableNetworkStack m_clientNetworkStack;
        util::ConsoleLogger m_logger;
        afl::base::Ref<afl::io::TextWriter> m_errorOutput;
        afl::base::Ref<afl::io::TextWriter> m_standardOutput;
        String_t m_configFileName;

        void reportError(String_t str);

        void parseCommandLine(ConfigurationHandler& handler);

        void waitReady(afl::net::CommandHandler& handler);
    };

}

#endif
