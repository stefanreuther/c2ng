/**
  *  \file ui/widgets/cardtabbar.cpp
  */

#include "ui/widgets/cardtabbar.hpp"


ui::widgets::CardTabBar::CardTabBar(Root& root, CardGroup& g)
    : TabBar(root),
      m_root(root),
      m_group(g),
      m_tabs(),
      conn_focusChange(g.sig_handleFocusChange.add(this, &CardTabBar::onFocusChange))
{
    // ex UICardGroupTab::UICardGroupTab
    sig_tabClick.add(this, &CardTabBar::setFocusedPage);
}

ui::widgets::CardTabBar::~CardTabBar()
{
    // ex UICardGroupTab::~UICardGroupTab
    conn_focusChange.disconnect();
}

void
ui::widgets::CardTabBar::addPage(const String_t& name, util::Key_t key, Widget& w)
{
    // ex UICardGroupTab::addTab
    size_t id = m_tabs.size();
    m_tabs.push_back(&w);
    TabBar::addPage(id, name, key);
}

void
ui::widgets::CardTabBar::addPage(const util::KeyString& name, Widget& w)
{
    // ex UICardGroupTab::addTab
    addPage(name.getString(), name.getKey(), w);
}

void
ui::widgets::CardTabBar::setFocusedPage(size_t index)
{
    if (index < m_tabs.size()) {
        m_tabs[index]->requestFocus();
        TabBar::setFocusedTab(index);
    }
}

void
ui::widgets::CardTabBar::onFocusChange()
{
    for (size_t i = 0, n = m_tabs.size(); i < n; ++i) {
        if (m_tabs[i]->hasState(FocusedState)) {
            TabBar::setFocusedTab(i);
        }
    }
}
