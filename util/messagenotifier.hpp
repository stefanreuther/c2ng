/**
  *  \file util/messagenotifier.hpp
  *  \brief Class util::MessageNotifier
  */
#ifndef C2NG_UTIL_MESSAGENOTIFIER_HPP
#define C2NG_UTIL_MESSAGENOTIFIER_HPP

#include "afl/base/signal.hpp"
#include "afl/sys/loglistener.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "afl/sys/mutexguard.hpp"
#include "afl/sys/mutex.hpp"

namespace util {

    /** Log message notifier.
        Log messages are generated from multiple threads.
        This class provides a single-threaded signal to receive notifications when a log message was written.
        The object is associated with a thread using a RequestDispatcher.
        Only that thread is allowed to hook the change signal, and only that thread will receive information.

        MessageNotifier does not provide access to the actual messages; use MessageCollector for that.
        (This means you may get notifications for messages the MessageCollector filtered out.)

        MessageNotifier performs simple debouncing.
        If more messages arrive while you're still processing the previous callback,
        another callback is scheduled, no matter how many messages arrive.
        (This means each notification may see multiple messages arrived.) */
    class MessageNotifier : public afl::sys::LogListener {
     public:
        /** Constructor.
            \param dispatcher RequestDispatcher for the thread that receives notifications */
        explicit MessageNotifier(RequestDispatcher& dispatcher);

        /** Destructor. */
        virtual ~MessageNotifier();

        // LogListener
        virtual void handleMessage(const Message& msg);

        /** Change signal. */
        afl::base::Signal<void()> sig_change;

     private:
        afl::sys::Mutex m_mutex;

        // Signalisation state
        enum SignalState {
            Idle,
            Pending,
            Retriggered
        };
        SignalState m_signalState;

        // Message receiver
        RequestReceiver<MessageNotifier> m_receiver;

        /** Trigger an update.
            This function is called with the mutex held.
            \param g MutexGuard, passed in to prove that you hold the mutex */
        void triggerUpdate(afl::sys::MutexGuard& g);

        /** Confirm an update.
            Called after a callback has been processed.
            Schedules another callback if needed. */
        void confirmUpdate();
    };

}

#endif
