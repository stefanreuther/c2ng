/**
  *  \file ui/waitindicator.cpp
  *  \brief Class ui::WaitIndicator
  */

#include "ui/waitindicator.hpp"

ui::WaitIndicator::WaitIndicator(Root& root, afl::string::Translator& tx)
    : util::WaitIndicator(root.engine().dispatcher()),
      m_root(root),
      m_indicator(root, tx("Working...")),
      m_busy(false),
      m_loop(root)
{ }

ui::WaitIndicator::~WaitIndicator()
{ }

void
ui::WaitIndicator::post(bool success)
{
    m_loop.stop(success);
}

bool
ui::WaitIndicator::wait()
{
    setBusy(true);
    bool success = (m_loop.run() != 0);
    setBusy(false);
    return success;
}

void
ui::WaitIndicator::setBusy(bool flag)
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
