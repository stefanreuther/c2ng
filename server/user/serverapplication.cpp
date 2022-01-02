/**
  *  \file server/user/serverapplication.cpp
  */

#include "server/user/serverapplication.hpp"
#include "afl/except/commandlineexception.hpp"
#include "afl/net/protocolhandlerfactory.hpp"
#include "afl/net/resp/protocolhandler.hpp"
#include "afl/net/server.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/thread.hpp"
#include "server/common/randomidgenerator.hpp"
#include "server/ports.hpp"
#include "server/user/classicencrypter.hpp"
#include "server/user/commandhandler.hpp"
#include "server/user/multipasswordencrypter.hpp"
#include "server/user/root.hpp"
#include "server/user/saltedpasswordencrypter.hpp"
#include "version.hpp"

using afl::async::InterruptOperation;

namespace {
    const char LOG_NAME[] = "user";

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

    size_t parseSize(const String_t& key, const String_t& value)
    {
        size_t n;
        if (!afl::string::strToInteger(value, n)) {
            throw afl::except::CommandLineException(afl::string::Format("Invalid number for '%s'", key));
        }
        return n;
    }
}

server::user::ServerApplication::ServerApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net, afl::async::Interrupt& intr)
    : Application(LOG_NAME, env, fs, net),
      m_listenAddress(DEFAULT_ADDRESS, USER_PORT),
      m_dbAddress(DEFAULT_ADDRESS, DB_PORT),
      m_interrupt(intr),
      m_config()
{ }

server::user::ServerApplication::~ServerApplication()
{ }

void
server::user::ServerApplication::serverMain()
{
    // Connect to other services
    afl::base::Deleter del;
    afl::net::CommandHandler& db(createClient(m_dbAddress, del, true));

    // Id generator for generating tokens
    // Unlike for router, we don't allow this to be configured for the service:
    // our job is to generate cryptographically secure tokens. Unlike router, we cannot rely
    // on an external component to secure them, nor do we have backward compatiblity constraints.
    server::common::RandomIdGenerator gen(fileSystem());

    // Password encrypter
#if 0
    ClassicEncrypter enc(m_config.userKey);
#else
    SaltedPasswordEncrypter primary(gen);
    ClassicEncrypter secondary(m_config.userKey);
    MultiPasswordEncrypter enc(primary, secondary);
#endif

    // Set up root
    Root root(db, gen, enc, m_config);
    CommandHandler ch(root);
    ProtocolHandlerFactory factory(ch);
    root.log().addListener(log());

    // Server
    afl::net::Server server(networkStack().listen(m_listenAddress, 10), factory);
    log().write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("Listening on %s", m_listenAddress.toString()));

    // Server thread
    afl::sys::Thread serverThread("user.server", server);
    serverThread.start();

    // Wait for termination request
    afl::async::Controller ctl;
    m_interrupt.wait(ctl, InterruptOperation::Kinds_t() + InterruptOperation::Break + InterruptOperation::Terminate);

    // Stop
    log().write(afl::sys::LogListener::Info, LOG_NAME, "Received stop signal, shutting down.");
    server.stop();
    serverThread.join();
}

bool
server::user::ServerApplication::handleConfiguration(const String_t& key, const String_t& value)
{
    if (key == "REDIS.HOST") {
        m_dbAddress.setName(value);
        return true;
    } else if (key == "REDIS.PORT") {
        m_dbAddress.setService(value);
        return true;
    } else if (key == "USER.HOST") {
        /* @q User.Host:Str (Config)
           Listen address for the User instance
           @since PCC2 2.40.6 */
        m_listenAddress.setName(value);
        return true;
    } else if (key == "USER.PORT") {
        /* @q User.Port:Int (Config)
           Port number for the User instance
           @since PCC2 2.40.6 */
        m_listenAddress.setService(value);
        return true;
    } else if (key == "USER.KEY") {
        /* @q User.Key:Str (Config)
           Site-wide secret ("pepper") for encrypting passwords. */
        m_config.userKey = value;
        return true;
    } else if (key == "USER.DATA.MAXKEYSIZE") {
        /* @q User.Data.MaxKeySize:Int (Config)
           Maximum size of a key in {UGET}/{USET}.
           @since PCC2 2.40.6 */
        m_config.userDataMaxKeySize = parseSize(key, value);
        return true;
    } else if (key == "USER.DATA.MAXVALUESIZE") {
        /* @q User.Data.MaxValueSize:Int (Config)
           Maximum size of a value in {UGET}/{USET}.
           @since PCC2 2.40.6 */
        m_config.userDataMaxValueSize = parseSize(key, value);
        return true;
    } else if (key == "USER.DATA.MAXTOTALSIZE") {
        /* @q User.Data.MaxTotalSize:Int (Config)
           Maximum total size of all user data ({UGET}/{USET}).
           @since PCC2 2.40.6 */
        m_config.userDataMaxTotalSize = parseSize(key, value);
        return true;
    } else if (key == "USER.PROFILE.MAXVALUESIZE") {
        /* @q User.Profile.MaxValueSize:Int (Config)
           Maximum size of a value in {SET (User Command)}.
           @since PCC2 2.40.7 */
        m_config.profileMaxValueSize = parseSize(key, value);
        return true;
    } else {
        return false;
    }
}

bool
server::user::ServerApplication::handleCommandLineOption(const String_t& /*option*/, afl::sys::CommandLineParser& /*parser*/)
{
    return false;
}

String_t
server::user::ServerApplication::getApplicationName() const
{
    return afl::string::Format("PCC2 User Server v%s - (c) 2019-2022 Stefan Reuther", PCC2_VERSION);
}

String_t
server::user::ServerApplication::getCommandLineOptionHelp() const
{
    return String_t();
}
