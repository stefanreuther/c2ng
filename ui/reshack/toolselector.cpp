/**
  *  \file ui/reshack/toolselector.cpp
  *  \brief Class ui::reshack::ToolSelector
  */

#include "ui/reshack/toolselector.hpp"
#include "ui/reshack/tool.hpp"
#include "ui/reshack/painter.hpp"

ui::reshack::ToolSelector::ToolSelector(Root& root, Painter& p)
    : ModeSelector(root),
      m_tools(),
      m_keys(),
      m_painter(p)
{ }

ui::reshack::ToolSelector::~ToolSelector()
{ }

void
ui::reshack::ToolSelector::addNewTool(util::Key_t key, Tool* t)
{
    // RHToolSelector::addNewTool(UIKey key, RHTool* t)
    if (t != 0) {
        m_tools.pushBackNew(t);
        m_keys.push_back(key);
        if (t->isUsable() && m_painter.getTool() == 0) {
            m_painter.setTool(t);
        }
    }
}

bool
ui::reshack::ToolSelector::isActive(size_t slot) const
{
    // RHToolSelector::isActive(int slot)
    return slot < m_tools.size() && m_painter.getTool() == m_tools[slot];
}

bool
ui::reshack::ToolSelector::isUsable(size_t slot) const
{
    // RHToolSelector::isUsable(int slot)
    return slot < m_tools.size() && m_tools[slot]->isUsable();
}

void
ui::reshack::ToolSelector::activate(size_t slot)
{
    // RHToolSelector::activate(int slot)
    if (slot < m_tools.size()) {
        m_painter.setTool(m_tools[slot]);
    }
}

String_t
ui::reshack::ToolSelector::getName(size_t slot) const
{
    // RHToolSelector::getName(int slot)
    if (slot < m_tools.size()) {
        return m_tools[slot]->getName();
    } else {
        return "?";
    }
}

size_t
ui::reshack::ToolSelector::getNumSlots() const
{
    // RHToolSelector::getNumSlots()
    return m_tools.size();
}

size_t
ui::reshack::ToolSelector::getSlotFromKey(util::Key_t k) const
{
    // RHToolSelector::getSlotFromKey(UIKey k)
    for (size_t i = 0; i < m_keys.size(); ++i) {
        if (k == m_keys[i]) {
            return i;
        }
    }
    return nil;
}
