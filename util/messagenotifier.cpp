/**
  *  \file util/messagenotifier.cpp
  */

#include "util/messagenotifier.hpp"

// Constructor.
util::MessageNotifier::MessageNotifier(RequestDispatcher& dispatcher)
    : sig_change(),
      m_mutex(),
      m_signalState(Idle),
      m_receiver(dispatcher, *this)
{ }      

// Destructor.
util::MessageNotifier::~MessageNotifier()
{ }

void
util::MessageNotifier::handleMessage(const Message& /*msg*/)
{
    afl::sys::MutexGuard g(m_mutex);
    triggerUpdate(g);
}

// Trigger an update.
void
util::MessageNotifier::triggerUpdate(afl::sys::MutexGuard& /*g*/)
{
    // Note that this function is called with the mutex held.
    // Be careful not to do anything that fetches a conflicting mutex.
    // The mutex inside m_receiver ought to be safe.
    switch (m_signalState) {
     case Idle:
        class Updater : public Request<MessageNotifier> {
         public:
            virtual void handle(MessageNotifier& mc)
                {
                    mc.sig_change.raise();
                    mc.confirmUpdate();
                }
        };
        m_signalState = Pending;
        m_receiver.getSender().postNewRequest(new Updater());
        break;

     case Pending:
        m_signalState = Retriggered;
        break;

     case Retriggered:
        break;
    }
}

// Confirm an update.
void
util::MessageNotifier::confirmUpdate()
{
    afl::sys::MutexGuard g(m_mutex);

    // Reset state
    SignalState oldState = m_signalState;
    m_signalState = Idle;

    // Retrigger if needed
    switch (oldState) {
     case Idle:                 // cannot happen
     case Pending:
        break;

     case Retriggered:
        triggerUpdate(g);
        break;
    }
}
