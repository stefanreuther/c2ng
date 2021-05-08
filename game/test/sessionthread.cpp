/**
  *  \file game/test/sessionthread.cpp
  *  \brief Class game::test::SessionThread
  */

#include "game/test/sessionthread.hpp"
#include "afl/sys/semaphore.hpp"

game::test::SessionThread::SessionThread()
    : m_translator(),
      m_fileSystem(),
      m_session(m_translator, m_fileSystem),
      m_log(),
      m_thread("<SessionThread>", m_log, m_translator),
      m_receiver(m_thread, m_session)
{ }

game::test::SessionThread::SessionThread(afl::io::FileSystem& fs)
    : m_translator(),
      m_fileSystem(),
      m_session(m_translator, fs),
      m_log(),
      m_thread("<SessionThread>", m_log, m_translator),
      m_receiver(m_thread, m_session)
{ }

game::test::SessionThread::~SessionThread()
{
    // Make sure all tasks posted until now are actually executed.
    // In particular, this means that shutdown tasks (RequestSender::makeTemporary)
    // which could otherwise get lost if the thread happens to die before noticing the task.
    sync();
}

game::Session&
game::test::SessionThread::session()
{
    return m_session;
}

util::RequestSender<game::Session>
game::test::SessionThread::gameSender()
{
    return m_receiver.getSender();
}

void
game::test::SessionThread::sync()
{
    struct Task : public afl::base::Runnable {
     public:
        Task(afl::sys::Semaphore& sem)
            : m_sem(sem)
            { }
        void run()
            { m_sem.post(); }
     private:
        afl::sys::Semaphore& m_sem;
    };

    afl::sys::Semaphore sem(0);
    m_thread.postNewRunnable(new Task(sem));
    sem.wait();
}
