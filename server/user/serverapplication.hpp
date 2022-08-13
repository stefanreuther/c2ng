/**
  *  \file server/user/serverapplication.hpp
  *  \brief Class server::user::ServerApplication
  */
#ifndef C2NG_SERVER_USER_SERVERAPPLICATION_HPP
#define C2NG_SERVER_USER_SERVERAPPLICATION_HPP

#include "afl/async/interrupt.hpp"
#include "server/application.hpp"
#include "server/user/configuration.hpp"

namespace server { namespace user {

    /** User Server main entry point. */
    class ServerApplication : public server::Application {
     public:
        /** Constructor.
            \param env  Environment
            \param fs   File system
            \param net  Network stack
            \param intr Operating system interrupts */
        ServerApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net, afl::async::Interrupt& intr);
        ~ServerApplication();

        // Application:
        virtual void serverMain();
        virtual bool handleConfiguration(const String_t& key, const String_t& value);
        virtual bool handleCommandLineOption(const String_t& option, afl::sys::CommandLineParser& parser);
        virtual String_t getApplicationName() const;
        virtual String_t getCommandLineOptionHelp() const;

     private:
        afl::net::Name m_listenAddress;
        afl::net::Name m_dbAddress;
        afl::async::Interrupt& m_interrupt;
        Configuration m_config;
    };

} }

#endif
