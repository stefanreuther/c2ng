/**
  *  \file util/simplerequestdispatcher.cpp
  *  \brief Class util::SimpleRequestDispatcher
  */

#include <memory>
#include <cassert>
#include "util/simplerequestdispatcher.hpp"
#include "afl/sys/mutexguard.hpp"

util::SimpleRequestDispatcher::SimpleRequestDispatcher()
    : m_mutex(),
      m_queueSemaphore(0),
      m_queue()
{ }

util::SimpleRequestDispatcher::~SimpleRequestDispatcher()
{
    while (!m_queue.empty()) {
        delete m_queue.front();
        m_queue.pop();
    }
}

void
util::SimpleRequestDispatcher::wait()
{
    m_queueSemaphore.wait();
    processTask();
}

bool
util::SimpleRequestDispatcher::wait(uint32_t timeout)
{
    bool ok = m_queueSemaphore.wait(timeout);
    if (ok) {
        processTask();
    }
    return ok;
}

void
util::SimpleRequestDispatcher::postNewRunnable(afl::base::Runnable* p)
{
    if (p) {
        afl::sys::MutexGuard g(m_mutex);
        m_queue.push(p);
        m_queueSemaphore.post();
    }
}

void
util::SimpleRequestDispatcher::processTask()
{
    std::auto_ptr<afl::base::Runnable> r;
    {
        afl::sys::MutexGuard g(m_mutex);
        assert(!m_queue.empty());
        r.reset(m_queue.front());
        m_queue.pop();
    }
    assert(r.get());
    r->run();
}
