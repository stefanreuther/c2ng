/**
  *  \file server/monitor/statusobserver.hpp
  *  \brief Interface server::monitor::StatusObserver
  */
#ifndef C2NG_SERVER_MONITOR_STATUSOBSERVER_HPP
#define C2NG_SERVER_MONITOR_STATUSOBSERVER_HPP

#include "server/monitor/observer.hpp"

namespace server { namespace monitor {

    /** Base class for a service status observer. */
    class StatusObserver : public Observer {
     public:
        // Observer methods:
        virtual String_t getName() = 0;
        virtual String_t getId() = 0;
        virtual String_t getUnit();
        virtual bool handleConfiguration(const String_t& key, const String_t& value) = 0;
        virtual Result check();

        /** Determine service status.
            Called periodically to update our view of the environment.
            This function should not perform any caching or similar.
            If it reports Running, the runtime of this function doubles as the latency measurement.
            \return status */
        virtual Status checkStatus() = 0;
    };

} }

#endif
