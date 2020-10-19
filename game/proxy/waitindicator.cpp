/**
  *  \file game/proxy/waitindicator.cpp
  *  \brief Class game::proxy::WaitIndicator
  */

#include "game/proxy/waitindicator.hpp"

// Constructor.
game::proxy::WaitIndicator::WaitIndicator(util::RequestDispatcher& disp)
    : m_receiver(disp, *this)
{ }

// Destructor.
game::proxy::WaitIndicator::~WaitIndicator()
{ }

void
game::proxy::WaitIndicator::confirm(util::RequestSender<WaitIndicator>& sender, bool success)
{
    // Note that this function is called from a destructor.
    // If it throws (e.g. out of memory), life as you know it will be over.
    // However, if we'd protect against this and catch/ignore the exception,
    // the confirmation will not get back to the UI thread, causing it to hang forever
    // (but still reacting to UI events and thus not being killable using the window manager).
    // Thus, crashing is the better alternative.
    class Confirmer : public util::Request<WaitIndicator> {
     public:
        Confirmer(bool success)
            : m_success(success)
            { }
        virtual void handle(WaitIndicator& ind)
            { ind.post(m_success); }
     private:
        bool m_success;
    };
    sender.postNewRequest(new Confirmer(success));
}
