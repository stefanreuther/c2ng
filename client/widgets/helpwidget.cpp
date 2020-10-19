/**
  *  \file client/widgets/helpwidget.cpp
  *  \brief Class client::widgets::HelpWidget
  */

#include "client/widgets/helpwidget.hpp"
#include "client/dialogs/helpdialog.hpp"

client::widgets::HelpWidget::HelpWidget(ui::Root& root, util::RequestSender<game::Session> gameSender, String_t pageName)
    : InvisibleWidget(),
      m_root(root),
      m_gameSender(gameSender),
      m_pageName(pageName),
      m_flags()
{
    // ex WHelpWidget::WHelpWidget
    m_flags += AcceptH;
    m_flags += AcceptF1;
}

client::widgets::HelpWidget::~HelpWidget()
{ }

client::widgets::HelpWidget&
client::widgets::HelpWidget::setFlag(Flag flag, bool value)
{
    m_flags.set(flag, value);
    return *this;
}

bool
client::widgets::HelpWidget::handleKey(util::Key_t key, int prefix)
{
    // ex WHelpWidget::handleEvent
    if (key == util::KeyMod_Alt + 'h'
        || key == util::KeyMod_Alt + 'H'
        || (m_flags.contains(AcceptH)
            && (key == 'h' || key == 'H'))
        || (m_flags.contains(AcceptF1)
            && key == util::Key_F1))
    {
        client::dialogs::doHelpDialog(m_root, m_gameSender, m_pageName);
        return true;
    } else {
        return defaultHandleKey(key, prefix);
    }
}
