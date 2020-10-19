/**
  *  \file util/simplerequestdispatcher.hpp
  *  \brief Class util::SimpleRequestDispatcher
  */
#ifndef C2NG_UTIL_SIMPLEREQUESTDISPATCHER_HPP
#define C2NG_UTIL_SIMPLEREQUESTDISPATCHER_HPP

#include <queue>
#include "afl/base/runnable.hpp"
#include "afl/sys/mutex.hpp"
#include "afl/sys/semaphore.hpp"
#include "util/requestdispatcher.hpp"

namespace util {

    /** Simple RequestDispatcher.
        This class collects posted Runnables and allows to execute them upon request.
        This is useful for testing, but can also be useful elsewhere. */
    class SimpleRequestDispatcher : public RequestDispatcher {
     public:
        /** Constructor. */
        SimpleRequestDispatcher();

        /** Destructor. */
        ~SimpleRequestDispatcher();

        /** Wait until a task is posted, then execute it. */
        void wait();

        /** Wait until a task is posted, then execute it; with timeout.
            \param timeout Timeout
            \return true if task was processed, false on timeout */
        bool wait(uint32_t timeout);

        // RequestDispatcher implementation:
        virtual void postNewRunnable(afl::base::Runnable* p);

     private:
        /** Mutex covering m_queue. */
        afl::sys::Mutex m_mutex;

        /** Semaphore counting the number of unprocessed tasks in m_queue. */
        afl::sys::Semaphore m_queueSemaphore;

        /** Queue containing unprocessed tasks.
            Tasks are owned by this object. */
        std::queue<afl::base::Runnable*> m_queue;

        void processTask();
    };

}

#endif
