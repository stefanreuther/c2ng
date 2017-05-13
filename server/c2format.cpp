/**
  *  \file server/c2format.cpp
  */

#include "afl/net/commandhandler.hpp"
#include "afl/net/name.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/net/protocolhandlerfactory.hpp"
#include "afl/net/resp/protocolhandler.hpp"
#include "afl/net/server.hpp"
#include "afl/string/format.hpp"
#include "server/application.hpp"
#include "server/format/format.hpp"
#include "server/interface/formatserver.hpp"

namespace {
    class ProtocolHandlerFactory : public afl::net::ProtocolHandlerFactory {
     public:
        ProtocolHandlerFactory(afl::net::CommandHandler& ch)
            : m_commandHandler(ch)
            { }
        virtual afl::net::ProtocolHandler* create()
            { return new afl::net::resp::ProtocolHandler(m_commandHandler); }
     private:
        afl::net::CommandHandler& m_commandHandler;
    };

    class FormatServerApplication : public server::Application {
     public:
        FormatServerApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net)
            : Application(env, fs, net),
              m_listenAddress("127.0.0.1", "6665")
            { }

        virtual void serverMain()
            {
                // Server implementation (stateless)
                server::format::Format fmt;

                // Command handler (stateless)
                server::interface::FormatServer fs(fmt);

                // Protocol Handler factory
                ProtocolHandlerFactory factory(fs);

                // Server
                afl::net::Server server(networkStack().listen(m_listenAddress, 10), factory);
                log().write(afl::sys::LogListener::Info, "format", afl::string::Format("Listening on %s", m_listenAddress.toString()));
                server.run();
            }

        virtual bool handleConfiguration(const String_t& key, const String_t& value)
            {
                // ex planetscentral/format/format.cc:processConfig
                if (key == "FORMAT.HOST") {
                    /* @q Format.Host:Str (Config)
                       Listen address for the Format instance. */
                    m_listenAddress.setName(value);
                    return true;
                } else if (key == "FORMAT.PORT") {
                    /* @q Format.Port:Int (Config)
                       Port number for the Format instance. */
                    m_listenAddress.setService(value);
                    return true;
                } else if (key == "FORMAT.THREADS") {
                    /* @q Format.Threads:Int (Config)
                       Ignored in c2ng/c2format-server for compatibility reasons.
                       Number of threads (=maximum number of parallel connections). */
                    return true;
                } else {
                    return false;
                }
            }

        virtual bool handleCommandLineOption(const String_t& /*option*/, afl::sys::CommandLineParser& /*parser*/)
            {
                return false;
            }
     private:
        afl::net::Name m_listenAddress;
    };
}

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    afl::net::NetworkStack& net = afl::net::NetworkStack::getInstance();
    FormatServerApplication(env, fs, net).run();
}
