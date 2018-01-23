/**
  *  \file server/interface/hostcronserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTCRONSERVER_HPP
#define C2NG_SERVER_INTERFACE_HOSTCRONSERVER_HPP

#include "server/interface/composablecommandhandler.hpp"
#include "server/interface/hostcron.hpp"

namespace server { namespace interface {

    class HostCronServer : public ComposableCommandHandler {
     public:
        explicit HostCronServer(HostCron& impl);
        ~HostCronServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        HostCron& m_implementation;

        static Value_t* packEvent(const HostCron::Event& event);
    };

} }

#endif
