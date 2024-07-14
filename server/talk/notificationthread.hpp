/**
  *  \file server/talk/notificationthread.hpp
  *  \brief Class server::talk::NotificationThread
  */
#ifndef C2NG_SERVER_TALK_NOTIFICATIONTHREAD_HPP
#define C2NG_SERVER_TALK_NOTIFICATIONTHREAD_HPP

#include "afl/base/stoppable.hpp"
#include "afl/sys/thread.hpp"
#include "afl/sys/semaphore.hpp"
#include "afl/sys/atomicinteger.hpp"
#include "server/talk/notifier.hpp"
#include "server/interface/mailqueue.hpp"

namespace server { namespace talk {

    class Root;

    /** Implementation of Notifier with background thread.

        Notifications for forum posts (notifyMessage) are queued (Root::messageNotificationQueue())
        and published with delay given in configuration (Configuration::notificationDelay).

        Notifications for PMs (notifyPM) are immediately sent.

        This implements a background thread that starts immediately on construction,
        and stops on deletion of the object. */
    class NotificationThread : public Notifier, private afl::base::Stoppable {
     public:
        /** Constructor.
            @param root Service root
            @param mq   Mail queue */
        NotificationThread(Root& root, server::interface::MailQueue& mq);

        /** Destructor. */
        ~NotificationThread();

        // Notifier:
        virtual void notifyMessage(Message& msg);
        virtual void notifyPM(UserPM& msg, const afl::data::StringList_t& notifyIndividual, const afl::data::StringList_t& notifyGroup);

        // Stoppable:
        virtual void stop();
        virtual void run();

     private:
        Root& m_root;                                ///< Service root
        server::interface::MailQueue& m_mailQueue;   ///< Mail queue.
        afl::sys::Semaphore m_semaphore;             ///< Semaphore to wake background thread.
        afl::sys::AtomicInteger m_shutdown;          ///< Signal for shutdown.
        afl::sys::Thread m_thread;                   ///< Background thread.

        void wake();
        uint32_t tick();
    };

} }

#endif
