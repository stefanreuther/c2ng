/**
  *  \file server/router/serverapplication.hpp
  */
#ifndef C2NG_SERVER_ROUTER_SERVERAPPLICATION_HPP
#define C2NG_SERVER_ROUTER_SERVERAPPLICATION_HPP

#include "afl/async/interrupt.hpp"
#include "server/application.hpp"
#include "server/common/idgenerator.hpp"
#include "server/router/configuration.hpp"
#include "util/process/factory.hpp"

    class IdGenerator;

namespace server { namespace router {

    class ServerApplication : public server::Application {
     public:
        /** Constructor.
            \param env  Environment
            \param fs   File system
            \param net  Network stack
            \param intr Operating system interrupts
            \param factory Process factory */
        ServerApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net, afl::async::Interrupt& intr, util::process::Factory& factory);

        virtual void serverMain();
        virtual bool handleConfiguration(const String_t& key, const String_t& value);
        virtual bool handleCommandLineOption(const String_t& option, afl::sys::CommandLineParser& parser);
        virtual String_t getApplicationName() const;
        virtual String_t getCommandLineOptionHelp() const;

     private:
        afl::net::Name m_listenAddress;
        afl::net::Name m_fileAddress;
        afl::async::Interrupt& m_interrupt;
        util::process::Factory& m_factory;
        std::auto_ptr<server::common::IdGenerator> m_generator;
        Configuration m_config;
    };

} }

#endif
