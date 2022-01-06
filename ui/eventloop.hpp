/**
  *  \file ui/eventloop.hpp
  *  \brief Class ui::EventLoop
  */
#ifndef C2NG_UI_EVENTLOOP_HPP
#define C2NG_UI_EVENTLOOP_HPP

#include "afl/base/closure.hpp"
#include "afl/base/uncopyable.hpp"

namespace ui {

    class Root;

    /** User Interface Event Loop.
        The event loop performs event dispatch until a stop signal is generated.
        The stop signal can be given from widget callbacks.

        Usage:
        - create EventLoop
        - set up some widgets
        - call run() to start event dispatching
        - call stop() from an event callback to have run() return

        An EventLoop can be used multiple times in sequence.

        Limitations:
        - stop() does not stack; calling stop() twice does not cause two run() calls to return.
          However, stop() CAN be called before run() and will cause run() to immediately exit.
        - like all UI stuff, this is single-threaded and stop() must be called from the UI thread */
    class EventLoop : public afl::base::Uncopyable {
     public:
        /** Constructor.
            \param root UI root */
        explicit EventLoop(Root& root);

        /** Dispatch events.
            \return Value passed to stop(). */
        int run();

        /** Stop.
            Causes run() to return the given value.
            \param n Return value */
        void stop(int n);

        /** Check whether this loop is already stopped.
            This can enable optimisations occasionally. */
        bool isStopped() const;

        /** Make a closure that calls stop().
            The closure can directly be attached to UI callback signals, e.g. Button::sig_click.
            \param n Return value
            \return newly-allocated closure */
        afl::base::Closure<void(int)>* makeStop(int n);

     private:
        Root& m_root;
        bool m_stopped;
        int m_result;
    };

}

#endif
