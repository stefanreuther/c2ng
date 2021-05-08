/**
  *  \file ui/eventloop.cpp
  *  \brief Class ui::EventLoop
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

bool
ui::EventLoop::isStopped() const
{
    return m_stopped;
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
        void call(int)
            { m_me.stop(m_n); }
     private:
        EventLoop& m_me;
        int m_n;
    };
    return new Stopper(*this, n);
}
