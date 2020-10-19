/**
  *  \file game/test/waitindicator.cpp
  *  \brief Class game::test::WaitIndicator
  */

#include <cassert>
#include "game/test/waitindicator.hpp"
#include "afl/sys/mutexguard.hpp"

game::test::WaitIndicator::WaitIndicator()
    : util::RequestDispatcher(),
      game::proxy::WaitIndicator(static_cast<RequestDispatcher&>(*this)),
      m_mutex(),
      m_resultSemaphore(0),
      m_result(false),
      m_queueSemaphore(0),
      m_queue()
{ }

game::test::WaitIndicator::~WaitIndicator()
{
    while (!m_queue.empty()) {
        delete m_queue.front();
        m_queue.pop();
    }
}

void
game::test::WaitIndicator::processQueue()
{
    while (m_queueSemaphore.wait(0)) {
        processTask();
    }
}

void
game::test::WaitIndicator::postNewRunnable(afl::base::Runnable* p)
{
    if (p) {
        afl::sys::MutexGuard g(m_mutex);
        m_queue.push(p);
        m_queueSemaphore.post();
    }
}

void
game::test::WaitIndicator::post(bool success)
{
    afl::sys::MutexGuard g(m_mutex);
    m_result = success;
    m_resultSemaphore.post();
}

bool
game::test::WaitIndicator::wait()
{
    while (!m_resultSemaphore.wait(0)) {
        m_queueSemaphore.wait();
        processTask();
    }

    afl::sys::MutexGuard g(m_mutex);
    return m_result;
}

void
game::test::WaitIndicator::processTask()
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
