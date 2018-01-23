/**
  *  \file server/monitor/status.hpp
  *  \brief Class server::monitor::Status
  */
#ifndef C2NG_SERVER_MONITOR_STATUS_HPP
#define C2NG_SERVER_MONITOR_STATUS_HPP

#include "afl/container/ptrvector.hpp"
#include "afl/sys/log.hpp"
#include "afl/sys/mutex.hpp"
#include "afl/sys/time.hpp"
#include "server/monitor/observer.hpp"
#include "server/monitor/timeseries.hpp"
#include "afl/io/stream.hpp"

namespace server { namespace monitor {

    /** Manager for multiple observers.
        Provides a thread-safe interface to access the status of a system defined as multiple Observer instances.
        This allows the server thread to obtain a valid status rendering at any time,
        while performing updates from another thread. */
    class Status {
     public:
        /** Default constructor.
            Makes an object with no observers. */
        Status();

        /** Destructor. */
        ~Status();

        /** Add a new Observer.
            The object will become owned by Status.
            The status will be reported as Unknown until update() is called for the first time.
            \param p Observer (null is ignored) */
        void addNewObserver(Observer* p);

        /** Handle configuration item.
            \param key Configuration key
            \param value Value
            \return true if value processed by any of the contained Observer's */
        bool handleConfiguration(const String_t& key, const String_t& value);

        /** Update the contained status.
            Calls all Observer's checkStatus() methods in sequence.
            Until this method completes, render() will return the old status. */
        void update();

        /** Return textual rendering of the service status.
            \param time [out] Associated timestamp
            \return HTML rendering */
        String_t render(afl::sys::Time& time) const;

        /** Render time series.
            \return HTML/SVG rendering */
        String_t renderTimeSeries() const;

        /** Load status from file.
            \param file File */
        void load(afl::io::Stream& file);

        /** Save status to file.
            \param file File */
        void save(afl::io::Stream& file) const;

        /** Access logger.
            Status has an own logger you can connect elsewhere.
            \return logger instance */
        afl::sys::Log& log();

     private:
        afl::sys::Log m_log;                                ///< Log node.

        mutable afl::sys::Mutex m_mutex;                    ///< Mutex protecting the following variables.
        afl::container::PtrVector<Observer> m_observers;    ///< List of observers.
        afl::container::PtrVector<TimeSeries> m_timeSeries; ///< List of time series, by index. May have fewer elements if status yet unknown.
        std::vector<Observer::Result> m_status;             ///< Statuses for observers, by index. May have fewer elements if status yet unknown.
        afl::sys::Time m_statusTime;                        ///< Time of status check.

        size_t m_maxTimePoints;

        /* Accessor methods, private because there is no public need so far */
        size_t getNumObservers();
        Observer* getObserverByIndex(size_t n);
    };

} }

#endif
