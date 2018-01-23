/**
  *  \file server/interface/hostscheduleserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTSCHEDULESERVER_HPP
#define C2NG_SERVER_INTERFACE_HOSTSCHEDULESERVER_HPP

#include "server/interface/composablecommandhandler.hpp"
#include "server/interface/hostschedule.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    class HostScheduleServer : public ComposableCommandHandler {
     public:
        explicit HostScheduleServer(HostSchedule& impl);
        ~HostScheduleServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        static Value_t* packSchedule(const HostSchedule::Schedule& sch);
        static HostSchedule::Schedule parseSchedule(interpreter::Arguments& args);

     private:
        HostSchedule& m_implementation;
    };

} }

#endif
