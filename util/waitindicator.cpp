/**
  *  \file util/waitindicator.cpp
  *  \brief Class util::WaitIndicator
  */

#include "util/waitindicator.hpp"

// Constructor.
util::WaitIndicator::WaitIndicator(RequestDispatcher& disp)
    : m_receiver(disp, *this)
{ }

// Destructor.
util::WaitIndicator::~WaitIndicator()
{ }

void
util::WaitIndicator::confirm(RequestSender<WaitIndicator>& sender, bool success)
{
    // Note that this function is called from a destructor.
    // If it throws (e.g. out of memory), life as you know it will be over.
    // However, if we'd protect against this and catch/ignore the exception,
    // the confirmation will not get back to the UI thread, causing it to hang forever
    // (but still reacting to UI events and thus not being killable using the window manager).
    // Thus, crashing is the better alternative.
    class Confirmer : public Request<WaitIndicator> {
     public:
        Confirmer(bool success)
            : m_success(success)
            { }
        virtual void handle(WaitIndicator& ind)
            { ind.post(m_success); }
     private:
        const bool m_success;
    };
    sender.postNewRequest(new Confirmer(success));
}
