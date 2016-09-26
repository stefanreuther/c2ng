/**
  *  \file ui/eventloop.cpp
  */

#include "ui/eventloop.hpp"
#include "ui/root.hpp"

ui::EventLoop::EventLoop(Root& root)
    : m_root(root),
      m_stopped(false),
      m_result(0)
{ }

int
ui::EventLoop::run()
{
    // ex UIBaseWidget::run, UIRoot::run (sort-of)
    while (!m_stopped) {
        m_root.handleEvent();
    }
    m_stopped = false;
    return m_result;
}

void
ui::EventLoop::stop(int n)
{
    // ex UIBaseWidget::stop, UIRoot::stop (sort-of)
    m_result = n;
    m_stopped = true;
}

afl::base::Closure<void(int)>*
ui::EventLoop::makeStop(int n)
{
    class Stopper : public afl::base::Closure<void(int)> {
     public:
        Stopper(EventLoop& me, int n)
            : m_me(me),
              m_n(n)
            { }
        Stopper* clone() const
            { return new Stopper(*this); }
        void call(int)
            { m_me.stop(m_n); }
     private:
        EventLoop& m_me;
        int m_n;
    };
    return new Stopper(*this, n);
}

// FIXME: port prefix argument handling
// int
// UIRoot::run()
// {
//     while (pending_stops.size() == 0) {
//         UIEvent e;
//         if (!haveQueuedEvent())
//             performDeferredRedraws();
//         getEvent(e, hasState(st_IdleNotify));

//         /* Process event. If nobody wants it, simplify it. */
//         bool ok = false;
//         do {
//             if (handleEvent(e, false) || handleEvent(e, true))
//                 ok = true;
//         } while (!ok && simplifyEvent(e));

//         /* If it is an unhandled user event, clear the prefix
//            argument. User events are keypresses and mouse button
//            releases. Since prefix argument entry is terminated
//            by either a keypress, or a pressed mouse button, it
//            suffices to test for b==0. This either means a button
//            release as required, or we don't have an argument. */
//         if (!ok)
//             if (e.type == ev_Key || (e.type == ev_Mouse && e.mouse.b == 0))
//                 consumePrefixArg();
//     }
//     int result = pending_stops.front();
//     pending_stops.pop();
//     return result;
// }
