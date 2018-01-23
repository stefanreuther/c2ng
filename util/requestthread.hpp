/**
  *  \file util/requestthread.hpp
  *  \brief Class util::RequestThread
  */
#ifndef C2NG_UTIL_REQUESTTHREAD_HPP
#define C2NG_UTIL_REQUESTTHREAD_HPP

#include <memory>
#include "afl/base/stoppable.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/string.hpp"
#include "afl/sys/loglistener.hpp"
#include "afl/sys/mutex.hpp"
#include "afl/sys/semaphore.hpp"
#include "afl/sys/thread.hpp"
#include "util/requestdispatcher.hpp"

namespace util {

    /** Worker thread.
        This implements RequestDispatcher and executes all posted Runnable's in a separate thread. */
    class RequestThread : public RequestDispatcher, private afl::base::Stoppable {
     public:
        /** Constructor.
            This starts the thread.
            \param name Name of the thread */
        RequestThread(String_t name, afl::sys::LogListener& log);

        /** Destructor.
            This stops the thread. */
        ~RequestThread();

        // RequestDispatcher:
        virtual void postNewRunnable(afl::base::Runnable* p);

     private:
        // Runnable:
        virtual void run();
        virtual void stop();

        /** Underlying thread. Created in constructor, shut down in destructor. */
        std::auto_ptr<afl::sys::Thread> m_thread;

        /** Mutex protecting m_taskQueue and m_stop. */
        afl::sys::Mutex m_taskMutex;

        /** Semaphore that signals availability of new tasks.
            The semaphore is increased for every empty->nonempty transition of m_taskQueue,
            not for every individual task. */
        afl::sys::Semaphore m_taskSemaphore;

        /** Tasks. Protected by m_taskMutex. */
        afl::container::PtrVector<afl::base::Runnable> m_taskQueue;

        /** Name. */
        String_t m_name;

        /** Logger. */
        afl::sys::LogListener& m_log;

        /** Stop flag. Protected by m_taskMutex. */
        bool m_stop;
    };

}

#endif
