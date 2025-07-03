/**
  *  \file server/application.hpp
  *  \brief Base class server::Application
  */
#ifndef C2NG_SERVER_APPLICATION_HPP
#define C2NG_SERVER_APPLICATION_HPP

#include "afl/base/deleter.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/net/commandhandler.hpp"
#include "afl/net/name.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/net/tunnel/tunnelablenetworkstack.hpp"
#include "afl/sys/commandlineparser.hpp"
#include "afl/sys/environment.hpp"
#include "afl/sys/loglistener.hpp"
#include "util/consolelogger.hpp"

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
        The standard command line accepts:
        - "-log" to configure the logger
        - "-D", "--config" to set configuration variables
        - "-proxy" to configure a proxy/tunnel to make outgoing connections
        - "-h", "--help" for help

        You derive from Application and implement serverMain().
        Your main() function looks like
        <code>
          return MyServerApplication(env, fs, net).run();
        </code>
        This will process the configuration and command line and call your handleConfiguration(), handleCommandLineOption() functions.
        It will then call your appMain(), with exception protection.
        Exceptions will be logged and cause the program to terminate unsuccessfully. */
    class Application : public afl::base::Deletable {
     public:
        /** Constructor.
            \param logName Logger name
            \param instanceName Default instance name
            \param env Environment instance
            \param fs File system instance
            \param net Network stack instance */
        Application(const String_t& logName, const String_t& instanceName, afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net);

        /** Run the server.
            Invokes serverMain() with exception protection.
            \return return code (exit code) */
        int run();

        /** Access environment.
            \return environment */
        afl::sys::Environment& environment();

        /** Access file system.
            \return file system */
        afl::io::FileSystem& fileSystem();

        /** Access network stack (to use for listening).
            \return network stack */
        afl::net::NetworkStack& networkStack();

        /** Access network stack (to use for connecting to other services)
            \return network stack */
        afl::net::NetworkStack& clientNetworkStack();

        /** Access logger.
            \return logger */
        afl::sys::LogListener& log();

        /** Access standard output channel.
            \return TextWriter */
        afl::io::TextWriter& standardOutput();

        /** Exit the application.
            \param n return code (exit code).

            Note that this function is implemented by throwing an exception.
            It will only work from the thread that called run().
            It will not work if called inside a block that catches all exceptions (catch(...)). */
        void exit(int n);

        /** Create a client to another microservice.
            \param name Network name
            \param del Deleter for created objects
            \param stateless true if this is a stateless connection (database, format).
                             In this case, it will be set to auto-reconnect.
                             In other cases, you have to deal with reconnections.
            \return CommandHandler to access the microservice, allocated in the deleter. */
        afl::net::CommandHandler& createClient(const afl::net::Name& name, afl::base::Deleter& del, bool stateless);

        /** Check for instance option.
            \param name   Option given by user
            \param expect Expected local option
            \return true if name is "<instance>.<expect>" */
        bool isInstanceOption(const String_t& name, const String_t& expect) const;

     protected:
        /** Application.
            This contains what normally would be your main() function.
            This function should contain the network loop. */
        virtual void serverMain() = 0;

        /** Handle configuration value.
            \param key Key (upper-case)
            \param value Value
            \retval true Key was known and processed
            \retval false Key not known
            \throw std::exception if key was known but value was invalid */
        virtual bool handleConfiguration(const String_t& key, const String_t& value) = 0;

        /** Handle command-line option.
            \param option Option name
            \param parser CommandLineParser instance to access potential option values
            \return true if option was understood */
        virtual bool handleCommandLineOption(const String_t& option, afl::sys::CommandLineParser& parser) = 0;

        /** Get application name.
            \return one-line banner (application name, version, copyright notice) */
        virtual String_t getApplicationName() const = 0;

        /** Get command-line option help.
            \return help in format for util::formatOptions(), ending in "\n" if nonempty; empty if no help available */
        virtual String_t getCommandLineOptionHelp() const = 0;

     private:
        class ConfigurationHandler;
        friend class ConfigurationHandler;

        String_t m_logName;
        String_t m_instanceName;
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
