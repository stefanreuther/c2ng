/**
  *  \file ui/widgets/quit.cpp
  */

#include "ui/widgets/quit.hpp"

ui::widgets::Quit::Quit(Root& root, EventLoop& loop)
    : InvisibleWidget(),
      m_root(root),
      m_loop(loop)
{ }

bool
ui::widgets::Quit::handleKey(util::Key_t key, int prefix)
{
    // ex UIQuit::handleEvent
    if (key == util::Key_Quit) {
        m_loop.stop(0);
        m_root.ungetKeyEvent(key, prefix);
        return true;
    } else {
        return false;
    }
}
