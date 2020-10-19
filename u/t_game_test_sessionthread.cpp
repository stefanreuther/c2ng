/**
  *  \file u/t_game_test_sessionthread.cpp
  *  \brief Test for game::test::SessionThread
  */

#include "game/test/sessionthread.hpp"

#include "t_game_test.hpp"
#include "afl/sys/semaphore.hpp"

/** Test SessionThread.
    A: prepare a SessionThread. Fetch session pointer.
    E: task in gameSender() accesses same session as session(). */
void
TestGameTestSessionThread::testIt()
{
    game::test::SessionThread testee;

    // Task to fetch the session address
    class Task : public util::Request<game::Session> {
     public:
        Task(afl::sys::Semaphore& sem, game::Session*& result)
            : m_sem(sem), m_result(result)
            { }
        virtual void handle(game::Session& session)
            {
                m_result = &session;
                m_sem.post();
            }
     private:
        afl::sys::Semaphore& m_sem;
        game::Session*& m_result;
    };

    // Invoke it
    afl::sys::Semaphore semDone(0);
    game::Session* result = 0;
    testee.gameSender().postNewRequest(new Task(semDone, result));

    // Wait for completion and check result
    semDone.wait();
    TS_ASSERT_EQUALS(&testee.session(), result);
}

