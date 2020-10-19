/**
  *  \file game/test/waitindicator.hpp
  *  \brief Class game::test::WaitIndicator
  */
#ifndef C2NG_GAME_TEST_WAITINDICATOR_HPP
#define C2NG_GAME_TEST_WAITINDICATOR_HPP

#include <queue>
#include "afl/sys/mutex.hpp"
#include "afl/sys/semaphore.hpp"
#include "game/proxy/waitindicator.hpp"
#include "util/requestdispatcher.hpp"

namespace game { namespace test {

    /** WaitIndicator for testing use.
        Implements a minimal WaitIndicator with no additional preconditions.
        It can also serve as a simple RequestDispatcher. */
    class WaitIndicator : public util::RequestDispatcher, public game::proxy::WaitIndicator {
     public:
        /** Constructor. */
        WaitIndicator();

        /** Destructor. */
        ~WaitIndicator();

        /** Process thread queue.
            In case the WaitIndicator is used for its RequestDispatcher personality,
            call this after a couple of postNewRunnable() calls have been made to process them. */
        void processQueue();

        // RequestDispatcher implementation:
        virtual void postNewRunnable(afl::base::Runnable* p);

        // WaitIndicator implementation:
        virtual void post(bool success);
        virtual bool wait();

     private:
        /** Mutex covering m_result, m_queue. */
        afl::sys::Mutex m_mutex;

        /** Semaphore counting the number of post() that have not been wait()ed yet. */
        afl::sys::Semaphore m_resultSemaphore;

        /** Result of last post(). (There should be only one outstanding post().) */
        bool m_result;

        /** Semaphore counting the number of unprocessed tasks in m_queue. */
        afl::sys::Semaphore m_queueSemaphore;

        /** Queue containing unprocessed tasks.
            Tasks are owned by this object. */
        std::queue<afl::base::Runnable*> m_queue;

        void processTask();
    };

} }

#endif
