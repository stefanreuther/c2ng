/**
  *  \file gfx/timer.hpp
  *  \brief Base class gfx::Timer
  */
#ifndef C2NG_GFX_TIMER_HPP
#define C2NG_GFX_TIMER_HPP

#include "afl/base/refcounted.hpp"
#include "afl/base/signal.hpp"
#include "afl/sys/types.hpp"

namespace gfx {

    /** Timer.
        This is the base class for a UI timer implementation.
        The main factory function is Engine::createTimer.

        Timers operate single-threaded in the UI thread.
        That is,
        - only call setInterval from the UI thread
        - the callback sig_fire will happen from the UI thread

        Timers always are single-shot timers.
        To get a cyclic timer, start it again from the callback.

        These timers are intended for user-interface purposes and are thus not expected to be very precise.
        One assumption is that the UI thread never blocks for a measurable amount, which means that if it does block,
        the block time may or may not be accounted for timer expiration depending on the implementation. */
    class Timer : public afl::base::RefCounted {
     public:
        /** Virtual destructor. */
        virtual ~Timer()
            { }

        /** Set interval.
            \param timeout New timeout in milliseconds. INFINITE_TIMEOUT to disable the timer */
        virtual void setInterval(afl::sys::Timeout_t timeout) = 0;

        /** Signal: interval elapsed, timer fires. */
        afl::base::Signal<void()> sig_fire;
    };

}

#endif
