/**
  *  \file gfx/timerqueue.cpp
  */

#include <algorithm>
#include "gfx/timerqueue.hpp"

class gfx::TimerQueue::TimerImpl : public gfx::Timer {
 public:
    TimerImpl(TimerQueue& owner)
        : m_pOwner(&owner),
          m_timeout(afl::sys::INFINITE_TIMEOUT),
          m_elapsed(0)
        { }

    ~TimerImpl()
        {
            if (m_pOwner != 0) {
                m_pOwner->remove(*this);
            }
        }

    virtual void setInterval(afl::sys::Timeout_t timeout)
        {
            m_timeout = timeout;
            m_elapsed = 0;
        }

    void disconnect()
        {
            m_pOwner = 0;
        }

    bool handleElapsedTime(afl::sys::Timeout_t time)
        {
            bool did = false;
            if (m_timeout != afl::sys::INFINITE_TIMEOUT) {
                afl::sys::Timeout_t remain = m_timeout - m_elapsed;
                if (remain <= time) {
                    // ok, timer fires
                    m_timeout = afl::sys::INFINITE_TIMEOUT;
                    sig_fire.raise();
                    did = true;
                } else {
                    // timer still running
                    m_elapsed += time;
                }
            }
            return did;
        }

    afl::sys::Timeout_t getNextTimeout() const
        {
            if (m_timeout != afl::sys::INFINITE_TIMEOUT) {
                return m_timeout - m_elapsed;
            } else {
                return afl::sys::INFINITE_TIMEOUT;
            }
        }

 private:
    TimerQueue* m_pOwner;
    afl::sys::Timeout_t m_timeout;
    afl::sys::Timeout_t m_elapsed;
};

gfx::TimerQueue::TimerQueue()
    : m_timers(),
      m_dirty(false)
{ }

gfx::TimerQueue::~TimerQueue()
{
    for (size_t i = 0, n = m_timers.size(); i < n; ++i) {
        if (m_timers[i] != 0) {
            m_timers[i]->disconnect();
        }
    }
}

afl::base::Ref<gfx::Timer>
gfx::TimerQueue::createTimer()
{
    afl::base::Ref<TimerImpl> t = *new TimerImpl(*this);
    m_timers.push_back(&t.get());
    return t;
}

afl::sys::Timeout_t
gfx::TimerQueue::getNextTimeout()
{
    cleanup();
    afl::sys::Timeout_t result = afl::sys::INFINITE_TIMEOUT;
    for (size_t i = 0, n = m_timers.size(); i < n; ++i) {
        result = std::min(result, m_timers[i]->getNextTimeout());
    }
    return result;
}

bool
gfx::TimerQueue::handleElapsedTime(afl::sys::Timeout_t time)
{
    bool did = false;
    for (size_t i = 0, n = m_timers.size(); i < n; ++i) {
        try {
            if (m_timers[i] != 0) {
                if (m_timers[i]->handleElapsedTime(time)) {
                    did = true;
                }
            }
        }
        catch (...) {
            // FIXME: log?
        }
    }
    cleanup();
    return did;
}

void
gfx::TimerQueue::cleanup()
{
    if (m_dirty) {
        size_t in = 0, out = 0, n = m_timers.size();
        while (in < n) {
            if (TimerImpl* p = m_timers[in++]) {
                m_timers[out++] = p;
            }
        }
        m_timers.resize(out);
        m_dirty = false;
    }
}

void
gfx::TimerQueue::remove(TimerImpl& who)
{
    for (size_t i = 0, n = m_timers.size(); i < n; ++i) {
        if (&who == m_timers[i]) {
            m_timers[i] = 0;
            m_dirty = true;
            break;
        }
    }
}
