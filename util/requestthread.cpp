/**
  *  \file util/requestthread.cpp
  *  \brief Class util::RequestThread
  */

#include "util/requestthread.hpp"
#include "afl/sys/mutexguard.hpp"

// Constructor.
util::RequestThread::RequestThread(String_t name, afl::sys::LogListener& log, afl::string::Translator& tx, int delay)
    : m_thread(),
      m_taskMutex(),
      m_taskSemaphore(0),
      m_taskQueue(),
      m_name(name),
      m_log(log),
      m_translator(tx),
      m_stop(false),
      m_delay(delay)
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

    // Make sure tasks are destroyed in correct order (FIFO).
    // PtrVector would destroy them from back to front.
    // Tasks might reference temporaries (RequestSender::makeTemporary) that refer to each other,
    // so destroying them in the wrong order means a task referring to the temporary overtakes one that destroys it.
    while (!m_taskQueue.empty()) {
        afl::container::PtrVector<afl::base::Runnable> tasks;
        tasks.swap(m_taskQueue);
        for (size_t i = 0, n = tasks.size(); i < n; ++i) {
            tasks.replaceElementNew(i, 0);
        }
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
    m_log.write(m_log.Trace, m_name, "Thread started");
    while (1) {
        m_taskSemaphore.wait();

        // Fetch tasks under mutex lock. This is also a nice place to check for termination requests.
        afl::container::PtrVector<afl::base::Runnable> tasks;
        {
            afl::sys::MutexGuard g(m_taskMutex);
            if (m_stop) {
                // Do not modify m_taskQueue when stopped, to guarantee that unexecuted tasks are destroyed in order!
                break;
            }
            tasks.swap(m_taskQueue);
        }

        // Process tasks
        // FIXME: check termination requests between tasks?
        for (size_t i = 0, n = tasks.size(); i < n; ++i) {
            // Request delay. This is a testing feature, so no need to check for termination here.
            if (m_delay > 0) {
                m_thread->sleep(m_delay);
            }
            try {
                tasks[i]->run();
            }
            catch (std::exception& e) {
                m_log.write(m_log.Warn, m_name, m_translator("Exception in background thread"), e);
            }

            // Destroy in correct order
            tasks.replaceElementNew(i, 0);
        }
    }
    m_log.write(m_log.Trace, m_name, "Thread terminates");
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
