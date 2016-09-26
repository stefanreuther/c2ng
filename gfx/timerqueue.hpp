/**
  *  \file gfx/timerqueue.hpp
  *  \brief Class gfx::TimerQueue
  */
#ifndef C2NG_GFX_TIMERQUEUE_HPP
#define C2NG_GFX_TIMERQUEUE_HPP

#include <vector>
#include "afl/base/ptr.hpp"
#include "gfx/timer.hpp"

namespace gfx {

    /** Simple implementation of timers.
        This is a simple, basic implementation of timers.
        It is intended to be used by Engine implementations if the underlying framework has no UI timers.

        To use, let your createTimer() function call TimerQueue::createTimer().
        In your event waiting routine,
        - limit the time to getNextTimeout(), if any
        - call handleElapsedTime() with the elapsed time if the timeout expires or an event arrives

        This class must entirely be used from the UI thread only. */
    class TimerQueue {
     public:
        /** Constructor. */
        TimerQueue();

        /** Destructor. */
        ~TimerQueue();

        /** Create a timer.

            Can be called from users; must be called from UI thread.

            \return new timer */
        afl::base::Ptr<Timer> createTimer();

        /** Get next timeout.

            Call from user-interface event acquisition method; see class description.

            \return Time until next timeout in milliseconds; INFINITE_TIMEOUT if none */
        afl::sys::Timeout_t getNextTimeout();

        /** Handle elapsed time.
            If any timer fires within that interval, process its callback.
            \param time Elapsed time (milliseconds, getTickCounter() difference)

            Call from user-interface event acquisition method; see class description.

            \retval true Some timers fired
            \retval false No progress */
        bool handleElapsedTime(afl::sys::Timeout_t time);

     private:
        /*
         *  Data model: we have a vector of TimerImpl pointers.
         *  If a timer dies, we set it to null and mark the vector dirty.
         *  That is, all code accessing m_timers must either handle that m_timers contains null elements,
         *  or call cleanup() to remove the null pointers.
         *
         *  We call cleanup() only from infrastructure methods (getNextTimeout(), handleElapsedTime()),
         *  not from user methods (createTimer()) which could be called from users' callbacks.
         */
        class TimerImpl;
        friend class TimerImpl;

        std::vector<TimerImpl*> m_timers;

        bool m_dirty;

        void cleanup();
        void remove(TimerImpl& who);
    };

}

#endif
