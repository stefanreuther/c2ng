/**
  *  \file server/c2talk.cpp
  *  \brief c2talk server application
  */

#include <stdexcept>
#include <cstring>
#include "afl/net/commandhandler.hpp"
#include "afl/net/name.hpp"
#include "afl/net/protocolhandler.hpp"
#include "afl/net/protocolhandlerfactory.hpp"
#include "afl/net/resp/client.hpp"
#include "afl/net/resp/protocolhandler.hpp"
#include "afl/net/server.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/thread.hpp"
#include "server/application.hpp"
#include "server/talk/commandhandler.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"
#include "afl/base/deleter.hpp"
#include "server/interface/baseclient.hpp"
#include "server/talk/configuration.hpp"

namespace {
    class TalkServerApplication;

    /** Session. This aggregates all session state into a ProtocolHandler. */
    class TalkSession : public afl::net::ProtocolHandler {
     public:
        TalkSession(server::talk::Root& root)
            : m_session(),
              m_commandHandler(root, m_session),
              m_protocolHandler(m_commandHandler)
            { }

        virtual void getOperation(Operation& op)
            { m_protocolHandler.getOperation(op); }
        virtual void advanceTime(afl::sys::Timeout_t msecs)
            { m_protocolHandler.advanceTime(msecs); }
        virtual void handleData(afl::base::ConstBytes_t bytes)
            { m_protocolHandler.handleData(bytes); }
        virtual void handleSendTimeout(afl::base::ConstBytes_t unsentBytes)
            { m_protocolHandler.handleSendTimeout(unsentBytes); }
        virtual void handleConnectionClose()
            { m_protocolHandler.handleConnectionClose(); }

     private:
        server::talk::Session m_session;
        server::talk::CommandHandler m_commandHandler;
        afl::net::resp::ProtocolHandler m_protocolHandler;
    };

    /** ProtocolHandlerFactory for c2talk. */
    class TalkProtocolHandlerFactory : public afl::net::ProtocolHandlerFactory {
     public:
        TalkProtocolHandlerFactory(server::talk::Root& root)
            : m_root(root)
            { }
        virtual TalkSession* create()
            { return new TalkSession(m_root); }
     private:
        server::talk::Root& m_root;
    };

    /** c2talk server application. */
    class TalkServerApplication : public server::Application {
     public:
        TalkServerApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net)
            : Application(env, fs, net),
              m_listenAddress("127.0.0.1", "5555"),
              m_dbAddress("127.0.0.1", "6379"),
              m_mailAddress("127.0.0.1", "21212"),
              m_keywordTableName(),
              m_config()
            { }

        virtual bool handleCommandLineOption(const String_t& /*option*/, afl::sys::CommandLineParser& /*parser*/)
            {
                return false;
            }

        virtual void serverMain()
            {
                // Connect to database
                afl::base::Deleter del;
                afl::net::CommandHandler& db(createClient(m_dbAddress, del));
                afl::net::CommandHandler& mail(createClient(m_mailAddress, del));

                // Set up root (global data)
                server::talk::Root root(db, mail, m_config);
                root.log().addListener(log());
                if (!m_keywordTableName.empty()) {
                    root.keywordTable().load(*fileSystem().openFile(m_keywordTableName, afl::io::FileSystem::OpenRead), log());
                }

                // Protocol Handler
                TalkProtocolHandlerFactory factory(root);

                // Server
                afl::net::Server server(networkStack().listen(m_listenAddress, 10), factory);
                log().write(afl::sys::LogListener::Info, "talk", afl::string::Format("Listening on %s", m_listenAddress.toString()));
                server.run();
            }

        virtual bool handleConfiguration(const String_t& key, const String_t& value)
            {
                // ex planetscentral/talk/talk.cc:processConfig
                if (key == "TALK.HOST") {
                    /* @q Talk.Host:Str (Config)
                       Listen address */
                    m_listenAddress.setName(value);
                    return true;
                } else if (key == "TALK.PORT") {
                    /* @q Talk.Port:Int (Config)
                       Port number. */
                    m_listenAddress.setService(value);
                    return true;
                } else if (key == "TALK.THREADS") {
                    /* @q Talk.Threads:Int (Config)
                       Ignored in c2ng/c2talk-server for compatibility reasons.
                       Number of threads (=maximum number of parallel connections) */
                    return true;
                } else if (key == "TALK.MSGID") {
                    /* @q Talk.MsgID:Str (Config)
                       Suffix for creating NNTP Message-IDs.
                       The value should start with a punctuator and must include a "@",
                       for example, ".talk@msgid.example.com".
                       The Id will be generated by prepending numbers (sequence number and {@type MID|posting Id}). */
                    m_config.messageIdSuffix = value;
                    return true;
                } else if (key == "TALK.PATH") {
                    /* @q Talk.Path:Str (Config)
                       Name of NNTP server, used for generating "Path" and "Xref" headers. */
                    m_config.pathHost = value;
                    return true;
                } else if (key == "TALK.WWWROOT") {
                    /* @q Talk.WWWRoot:Str (Config)
                       Root of web application, used for generating links. */
                    m_config.baseUrl = value;
                    return true;
                } else if (key == "TALK.SYNTAXDB") {
                    /* @q Talk.SyntaxDB:Str (Config)
                       Name of file with syntax database.
                       If not specified, the syntax database will be empty ({SYNTAXGET} will always fail). */
                    m_keywordTableName = value;
                    return true;
                } else if (key == "REDIS.HOST") {
                    m_dbAddress.setName(value);
                    return true;
                } else if (key == "REDIS.PORT") {
                    m_dbAddress.setService(value);
                    return true;
                } else if (key == "MAILOUT.HOST") {
                    m_mailAddress.setName(value);
                    return true;
                } else if (key == "MAILOUT.PORT") {
                    m_mailAddress.setService(value);
                    return true;
                } else if (key == "USER.KEY") {
                    m_config.userKey = value;
                    return true;
                } else {
                    return false;
                }
            }

     private:
        afl::net::Name m_listenAddress;
        afl::net::Name m_dbAddress;
        afl::net::Name m_mailAddress;

        String_t m_keywordTableName;
        server::talk::Configuration m_config;

        afl::net::CommandHandler& createClient(const afl::net::Name& name, afl::base::Deleter& del)
            {
                // ex Connection::connectRetry
                int i = 5;
                while (1) {
                    --i;
                    try {
                        afl::net::CommandHandler& result = del.addNew(new afl::net::resp::Client(networkStack(), name));
                        log().write(afl::sys::LogListener::Info, "talk", afl::string::Format("Connected to %s", name.toString()));
                        waitReady(result);
                        return result;
                    }
                    catch (std::exception& e) {
                        if (i == 0) {
                            throw;
                        }
                        afl::sys::Thread::sleep(1000);
                    }
                }
            }

        void waitReady(afl::net::CommandHandler& handler)
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
                                log().write(afl::sys::LogListener::Error, "talk", "Server fails to become ready; giving up.");
                                throw;
                            }
                        } else {
                            throw;
                        }
                    }
                    int sleepTime = count > 20 ? 20 : count > 10 ? 5 : 1;
                    log().write(afl::sys::LogListener::Trace, "talk", afl::string::Format("Server not ready yet, sleeping %d seconds...", sleepTime));
                    afl::sys::Thread::sleep(sleepTime * 1000);
                }
            }
    };
}

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    afl::net::NetworkStack& net = afl::net::NetworkStack::getInstance();
    TalkServerApplication(env, fs, net).run();
}
