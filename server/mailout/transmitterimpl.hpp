/**
  *  \file server/mailout/transmitterimpl.hpp
  *  \brief Class server::mailout::TransmitterImpl
  */
#ifndef C2NG_SERVER_MAILOUT_TRANSMITTERIMPL_HPP
#define C2NG_SERVER_MAILOUT_TRANSMITTERIMPL_HPP

#include <list>
#include "afl/base/ref.hpp"
#include "afl/base/stoppable.hpp"
#include "afl/base/uncopyable.hpp"
#include "afl/io/directory.hpp"
#include "afl/net/name.hpp"
#include "afl/net/smtp/client.hpp"
#include "afl/net/smtp/configuration.hpp"
#include "afl/sys/mutex.hpp"
#include "afl/sys/semaphore.hpp"
#include "afl/sys/thread.hpp"
#include "server/mailout/transmitter.hpp"

namespace server { namespace mailout {

    class Root;
    class Message;

    /** Transmitter for sending mails on SMTP.
        This is the main implementation of the Transmitter interface for production use.

        <b>Basic Operation</b>

        Messages are always stored in the redis database for persistance, although they often are short-lived.
        There are two queues in redis:
        - sending
        - preparing
        Those are managed <em>outside</em> Transmitter.
        Upon startup, we delete all messages from the preparing queue;
        these are messages that were partially prepared but not sent.

        Transmitter only deals with the sending queue which it mirrors in RAM.
        - m_workQueue
        - m_postponedMessages

        All messages are placed in m_workQueue first.
        After they are sent, they are removed.
        If they cannot be sent right now, they are moved to the m_postponedMessages and reconsidered at a later time
        by moving them back to m_workQueue.

        <b>Mutual Exclusion</b>

        TransmitterImpl spawns a thread that processes the queue.
        That thread will access the database.
        The database CommandHandler is expected to be multithread-safe.

        Explicit protection is required only for TransmitterImpl's own members. */
    class TransmitterImpl : public Transmitter,
                            private afl::base::Uncopyable,
                            private afl::base::Stoppable
    {
     public:
        /** Constructor.
            \param root Service root; must live longer than TransmitterImpl instance
            \param templateDir Template directory
            \param net Network stack; must live longer than TransmitterImpl instance
            \param smtpAddress Address of SMTP server
            \param smtpConfig SMTP configuration */
        TransmitterImpl(Root& root,
                        afl::base::Ref<afl::io::Directory> templateDir,
                        afl::net::NetworkStack& net,
                        afl::net::Name smtpAddress,
                        const afl::net::smtp::Configuration& smtpConfig);

        /** Destructor. */
        ~TransmitterImpl();

        // Transmitter:
        virtual void send(int32_t messageId);
        virtual void notifyAddress(String_t address);
        virtual void runQueue();

     private:
        virtual void run();
        virtual void stop();

        void processWork();
        bool sendMessage(Message& msg, String_t address);

        afl::sys::Thread m_thread;

        Root& m_root;
        afl::base::Ref<afl::io::Directory> m_templateDirectory;

        afl::net::smtp::Client m_smtpClient;
        afl::net::smtp::Configuration m_smtpConfig;
        afl::net::NetworkStack& m_networkStack;

        /** Protected data.
            Stuff in this class is protected by a mutex and can be accessed by the worker thread
            as well as the main service thread. */
        class Data {
         public:
            Data();

            bool isStopRequested();
            void requestStop();
            bool getNextWork(int32_t& msgId);
            void addToWork(int32_t msgId);
            void removeFromWork(int32_t msgId);
            void moveToPending(int32_t msgId);
            void movePendingToWork();
            void wait();

         private:
            afl::sys::Semaphore m_wake;             ///< Wake the worker. Posted for each element added to m_workQueue, or for stop request.
            afl::sys::Mutex m_mutex;                ///< Mutex protecting all of the following variables.
            bool m_stopRequest;                     ///< Set to true to trigger stop of the worker thread.
            std::list<int32_t> m_workQueue;         ///< List of items to process. Head is currently being worked on.
            std::list<int32_t> m_postponedMessages; ///< List of items that failed because of an unverified address.
        };
        Data m_data;
    };

} }

#endif
