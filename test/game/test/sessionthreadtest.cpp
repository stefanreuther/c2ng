/**
  *  \file test/game/test/sessionthreadtest.cpp
  *  \brief Test for game::test::SessionThread
  */

#include "game/test/sessionthread.hpp"

#include "afl/io/internalfilesystem.hpp"
#include "afl/sys/semaphore.hpp"
#include "afl/test/testrunner.hpp"

/** Test SessionThread.
    A: prepare a SessionThread. Fetch session pointer.
    E: task in gameSender() accesses same session as session(). */
AFL_TEST("game.test.SessionThread:basics", a)
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
    a.checkEqual("01. result", &testee.session(), result);
}

/** Test file system passing.
    A: prepare a SessionThread with a custom FileSystem instance.
    E: file system is published in session */
AFL_TEST("game.test.SessionThread:fileSystem", a)
{
    afl::io::InternalFileSystem fs;
    fs.openFile("/x", afl::io::FileSystem::Create);
    game::test::SessionThread testee(fs);
    AFL_CHECK_SUCCEEDS(a("01. fileSystem"), testee.session().world().fileSystem().openFile("/x", afl::io::FileSystem::OpenRead));
}
