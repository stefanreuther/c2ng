/**
  *  \file client/downlink.cpp
  *  \brief Class client::Downlink
  */

#include "client/downlink.hpp"

client::Downlink::Downlink(ui::Root& root, afl::string::Translator& tx)
    : WaitIndicator(root.engine().dispatcher()),
      m_root(root),
      m_indicator(root, tx("Working...")),
      m_busy(false),
      m_loop(root)
{ }

client::Downlink::Downlink(client::si::UserSide& us)
    : WaitIndicator(us.root().engine().dispatcher()),
      m_root(us.root()),
      m_indicator(us.root(), us.translator()("Working...")),
      m_busy(false),
      m_loop(us.root())
{
    m_indicator.sig_interrupt.add(&us, &client::si::UserSide::interruptRunningProcesses);
}

client::Downlink::~Downlink()
{ }

void
client::Downlink::post(bool success)
{
    m_loop.stop(success);
}

bool
client::Downlink::wait()
{
    setBusy(true);
    bool success = (m_loop.run() != 0);
    setBusy(false);
    return success;
}

void
client::Downlink::setBusy(bool flag)
{
    if (flag != m_busy) {
        m_busy = flag;
        if (m_busy) {
            m_indicator.setExtent(gfx::Rectangle(gfx::Point(), m_indicator.getLayoutInfo().getPreferredSize()));
            m_root.moveWidgetToEdge(m_indicator, gfx::CenterAlign, gfx::BottomAlign, 10);
            m_root.add(m_indicator);
        } else {
            m_root.remove(m_indicator);
            m_indicator.replayEvents();
        }
    }
}
