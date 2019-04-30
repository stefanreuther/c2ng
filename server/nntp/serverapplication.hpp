/**
  *  \file server/nntp/serverapplication.hpp
  *  \brief Class server::nntp::ServerApplication
  */
#ifndef C2NG_SERVER_NNTP_SERVERAPPLICATION_HPP
#define C2NG_SERVER_NNTP_SERVERAPPLICATION_HPP

#include "server/application.hpp"
#include "afl/async/interrupt.hpp"
#include "afl/string/string.hpp"

namespace server { namespace nntp {

    /** c2nntp server application.
        c2nntp-server's main function consists of an instantiation of this object. */
    class ServerApplication : public server::Application {
     public:
        /** Constructor.
            \param env  Environment
            \param fs   File system
            \param net  Network stack
            \param intr Operating system interrupts */
        ServerApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net, afl::async::Interrupt& intr);

        /** Destructor. */
        ~ServerApplication();

        // server::Application
        virtual bool handleCommandLineOption(const String_t& option, afl::sys::CommandLineParser& parser);
        virtual void serverMain();
        virtual bool handleConfiguration(const String_t& key, const String_t& value);
        virtual String_t getApplicationName() const;
        virtual String_t getCommandLineOptionHelp() const;

     private:
        afl::net::Name m_listenAddress;
        afl::net::Name m_talkAddress;
        afl::net::Name m_userAddress;

        String_t m_baseUrl;   // ex www_prefix

        afl::async::Interrupt& m_interrupt;

    };

} }

#endif
