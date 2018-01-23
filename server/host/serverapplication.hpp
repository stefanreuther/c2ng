/**
  *  \file server/host/serverapplication.hpp
  *  \brief Class server::host::ServerApplication
  */
#ifndef C2NG_SERVER_HOST_SERVERAPPLICATION_HPP
#define C2NG_SERVER_HOST_SERVERAPPLICATION_HPP

#include "afl/async/interrupt.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/net/name.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/sys/environment.hpp"
#include "server/application.hpp"
#include "server/host/configuration.hpp"

namespace server { namespace host {

    /** c2host server application.
        c2host-server's main function consists of an instantiation of this object. */
    class ServerApplication : public Application {
     public:
        /** Constructor.
            \param env  Environment
            \param fs   File system
            \param net  Network stack
            \param intr Operating system interrupts*/
        ServerApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net, afl::async::Interrupt& intr);

        /** Destructor. */
        ~ServerApplication();

        // server::Application:
        virtual bool handleCommandLineOption(const String_t& option, afl::sys::CommandLineParser& parser);
        virtual void serverMain();
        virtual bool handleConfiguration(const String_t& key, const String_t& value);

     private:
        afl::net::Name m_listenAddress;
        afl::net::Name m_dbAddress;
        afl::net::Name m_userFileAddress;
        afl::net::Name m_talkAddress;
        afl::net::Name m_mailAddress;
        afl::net::Name m_routerAddress;

        Configuration m_config;
        afl::async::Interrupt& m_interrupt;

        void setupWorkDirectory();
    };

} }

#endif
