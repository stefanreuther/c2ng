/**
  *  \file client/downlink.cpp
  *  \brief Class client::Downlink
  */

#include "client/downlink.hpp"
#include "util/translation.hpp"

// Constructor.
client::Downlink::Downlink(ui::Root& root)
    : m_root(root),
      m_indicator(root, _("Working")),
      m_busy(false),
      m_loop(root),
      m_receiver(root.engine().dispatcher(), *this)
{ }

// Destructor.
client::Downlink::~Downlink()
{ }

void
client::Downlink::setBusy(bool flag)
{
    if (flag != m_busy) {
        m_busy = flag;
        if (m_busy) {
            m_indicator.setExtent(gfx::Rectangle(gfx::Point(), m_indicator.getLayoutInfo().getPreferredSize()));
            m_root.moveWidgetToEdge(m_indicator, 1, 2, 10);
            m_root.add(m_indicator);
        } else {
            m_root.remove(m_indicator);
            m_indicator.replayEvents();
        }
    }
}

void
client::Downlink::confirm(util::RequestSender<Downlink>& sender, bool success)
{
    // Note that this function is called from a destructor.
    // If it throws (e.g. out of memory), life as you know it will be over.
    // However, if we'd protect against this and catch/ignore the exception,
    // the confirmation will not get back to the UI thread, causing it to hang forever
    // (but still reacting to UI events and thus not being killable using the window manager).
    // Thus, crashing is the better alternative.
    class Confirmer : public util::Request<Downlink> {
     public:
        Confirmer(bool success)
            : m_success(success)
            { }
        virtual void handle(Downlink& dl)
            { dl.m_loop.stop(m_success); }
     private:
        bool m_success;
    };
    sender.postNewRequest(new Confirmer(success));
}
