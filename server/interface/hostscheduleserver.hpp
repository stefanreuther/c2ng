/**
  *  \file server/interface/hostscheduleserver.hpp
  *  \brief Class server::interface::HostScheduleServer
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTSCHEDULESERVER_HPP
#define C2NG_SERVER_INTERFACE_HOSTSCHEDULESERVER_HPP

#include "server/interface/composablecommandhandler.hpp"
#include "server/interface/hostschedule.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    /** Server for host schedule access.
        Implements a ComposableCommandHandler and dispatches received commands to a HostSchedule implementation. */
    class HostScheduleServer : public ComposableCommandHandler {
     public:
        /** Constructor.
            @param impl Implementation; must live sufficiently long. */
        explicit HostScheduleServer(HostSchedule& impl);

        /** Destructor. */
        ~HostScheduleServer();

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        /** Pack schedule into a Value tree.
            @param sch Schedule
            @return newly allocated Value; caller assumes ownership */
        static Value_t* packSchedule(const HostSchedule::Schedule& sch);

        /** Parse schedule from a command line.
            @param args Command line
            @return Schedule/schedule modification */
        static HostSchedule::Schedule parseSchedule(interpreter::Arguments& args);

     private:
        HostSchedule& m_implementation;
    };

} }

#endif
