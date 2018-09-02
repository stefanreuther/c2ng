/**
  *  \file server/monitor/serverapplication.hpp
  *  \brief Class server::monitor::ServerApplication
  */
#ifndef C2NG_SERVER_MONITOR_SERVERAPPLICATION_HPP
#define C2NG_SERVER_MONITOR_SERVERAPPLICATION_HPP

#include "server/application.hpp"
#include "afl/sys/environment.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/async/interrupt.hpp"
#include "afl/sys/commandlineparser.hpp"
#include "afl/net/name.hpp"
#include "afl/container/ptrvector.hpp"
#include "server/monitor/status.hpp"

namespace server { namespace monitor {

    /** c2monitor server application.
        c2monitor-server's main function consists of an instantiation of this object. */
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
        afl::async::Interrupt& m_interrupt;
        String_t m_templateFileName;
        String_t m_statusFileName;
        int32_t m_updateInterval;
        int32_t m_saveInterval;

        Status m_status;

        void doSave();
    };

} }

#endif
