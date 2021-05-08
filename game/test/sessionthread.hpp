/**
  *  \file game/test/sessionthread.hpp
  *  \brief Class game::test::SessionThread
  */
#ifndef C2NG_GAME_TEST_SESSIONTHREAD_HPP
#define C2NG_GAME_TEST_SESSIONTHREAD_HPP

#include "game/session.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "util/requestthread.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"
#include "afl/sys/log.hpp"

namespace game { namespace test {

    /** A game::Session with an associated thread.
        This is useful for multi-threaded tests.
        - set up the test using session()
        - perform test using gameSender()
        - verify results using session() */
    class SessionThread {
     public:
        /** Constructor. This will start the thread. */
        SessionThread();

        /** Constructor with file system. This will start the thread.
         \param fs File System to use */
        SessionThread(afl::io::FileSystem& fs);

        /** Destructor. */
        ~SessionThread();

        /** Access session.
            Use only when there is no request in flight on the gameSender().
            \return session */
        Session& session();

        /** Access sender for game session.
            \return sender */
        util::RequestSender<Session> gameSender();

        /** Synchronize.
            Perform a dummy synchronous call to make sure previous asynchronous requests completed. */
        void sync();

     private:
        afl::string::NullTranslator m_translator;
        afl::io::NullFileSystem m_fileSystem;
        game::Session m_session;

        // - Game thread
        afl::sys::Log m_log;
        util::RequestThread m_thread;
        util::RequestReceiver<Session> m_receiver;
    };
    
} }

#endif
