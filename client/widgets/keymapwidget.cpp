/**
  *  \file client/widgets/keymapwidget.cpp
  */

#include "client/widgets/keymapwidget.hpp"

client::widgets::KeymapWidget::KeymapWidget(util::RequestSender<game::Session> gameSender,
                                            util::RequestDispatcher& self,
                                            client::si::Control& ctl)
    : m_proxy(gameSender, self),
      m_control(ctl),
      m_keys(),
      m_keymapName()
{
    m_proxy.setListener(*this);
}

client::widgets::KeymapWidget::~KeymapWidget()
{ }

void
client::widgets::KeymapWidget::setKeymapName(String_t keymap)
{
    if (keymap != m_keymapName) {
        m_keymapName = keymap;
        m_proxy.setKeymapName(keymap);
    }
}

bool
client::widgets::KeymapWidget::handleKey(util::Key_t key, int prefix)
{
    if (m_keys.find(key) != m_keys.end()) {
        m_control.executeKeyCommandWait(m_keymapName, key, prefix);
        return true;
    } else {
        return false;
    }
}

void
client::widgets::KeymapWidget::updateKeyList(util::KeySet_t& keys)
{
    m_keys.swap(keys);
}
