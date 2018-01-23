/**
  *  \file server/monitor/observer.hpp
  *  \brief Interface server::monitor::Observer
  */
#ifndef C2NG_SERVER_MONITOR_OBSERVER_HPP
#define C2NG_SERVER_MONITOR_OBSERVER_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"

namespace server { namespace monitor {

    /** Base class for a service status observer. */
    class Observer : public afl::base::Deletable {
     public:
        /** Status of the service.
            The status can be the status of a service (Running/Broken/Down)
            or a simple value such as CPU usage. */
        enum Status {
            Unknown,             ///< Unknown/not yet determined.
            Running,             ///< Service is running. Value is latency in ms.
            Broken,              ///< Service seems running, but not working correctly.
            Down,                ///< Service is down.
            Value                ///< Just a value, no running/broken indication.
        };

        /** Result of status check. */
        struct Result {
            Status status;
            int32_t value;

            Result()
                : status(Unknown), value(0)
                { }
            Result(Status st, int32_t value)
                : status(st), value(value)
                { }
        };

        /** Get user-readable name of service.
            \return name */
        virtual String_t getName() = 0;

        /** Get machine-readable identifier of service.
            \return identifier */
        virtual String_t getId() = 0;

        /** Get unit of result value.
            \return unit; can be empty string */
        virtual String_t getUnit() = 0;

        /** Handle configuration item.
            All Observer instances get to see all configuration.
            \param key Configuration key
            \param value Value
            \return true if value processed */
        virtual bool handleConfiguration(const String_t& key, const String_t& value) = 0;

        /** Determine result.
            \return result */
        virtual Result check() = 0;
    };

} }

#endif
