/**
  *  \file util/requestthread.cpp
  *  \brief Class util::RequestThread
  */

#include "util/requestthread.hpp"
#include "afl/sys/mutexguard.hpp"
#include "util/translation.hpp"

// Constructor.
util::RequestThread::RequestThread(String_t name, afl::sys::LogListener& log)
    : m_thread(),
      m_taskMutex(),
      m_taskSemaphore(0),
      m_taskQueue(),
      m_name(name),
      m_log(log),
      m_stop(false)
{
    m_thread.reset(new afl::sys::Thread(name, *this));
    m_thread->start();
}

// Destructor.
util::RequestThread::~RequestThread()
{
    if (m_thread.get() != 0) {
        stop();
        m_thread->join();
    }
}

// Post new Runnable.
void
util::RequestThread::postNewRunnable(afl::base::Runnable* p)
{
    afl::sys::MutexGuard g(m_taskMutex);
    m_taskQueue.pushBackNew(p);
    if (m_taskQueue.size() == 1) {
        m_taskSemaphore.post();
    }
}

// Thread entry point.
void
util::RequestThread::run()
{
    m_log.write(m_log.Trace, m_name, _("Thread started"));
    while (1) {
        m_taskSemaphore.wait();

        // Fetch tasks under mutex lock. This is also a nice place to check for termination requests.
        afl::container::PtrVector<afl::base::Runnable> tasks;
        {
            afl::sys::MutexGuard g(m_taskMutex);
            tasks.swap(m_taskQueue);
            if (m_stop) {
                break;
            }
        }

        // Process tasks
        // FIXME: check termination requests between tasks?
        for (size_t i = 0, n = tasks.size(); i < n; ++i) {
            // for testing: m_thread->sleep(1000);
            try {
                tasks[i]->run();
            }
            catch (std::exception& e) {
                m_log.write(m_log.Error, m_name, _("Exception in background thread"), e);
            }
        }
    }
    m_log.write(m_log.Trace, m_name, _("Thread terminates"));
}

void
util::RequestThread::stop()
{
    {
        afl::sys::MutexGuard g(m_taskMutex);
        m_stop = true;
    }
    m_taskSemaphore.post();
}
