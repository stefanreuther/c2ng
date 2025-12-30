/**
  *  \file ui/reshack/zoomselector.cpp
  *  \brief Class ui::reshack::ZoomSelector
  */

#include "ui/reshack/zoomselector.hpp"
#include "afl/string/format.hpp"
#include "ui/reshack/painter.hpp"

ui::reshack::ZoomSelector::ZoomSelector(Root& root, Painter& p, size_t nslots)
    : ModeSelector(root), m_painter(p), m_numSlots(nslots)
{ }

ui::reshack::ZoomSelector::~ZoomSelector()
{ }

bool
ui::reshack::ZoomSelector::isActive(size_t slot) const
{
    // RHZoomSelector::isActive(int slot)
    return size_t(m_painter.getZoom()) == slot;
}

bool
ui::reshack::ZoomSelector::isUsable(size_t /*slot*/) const
{
    // RHZoomSelector::isUsable(int /*slot*/)
    return true;
}

void
ui::reshack::ZoomSelector::activate(size_t slot)
{
    // RHZoomSelector::activate(int slot)
    m_painter.setZoom(int(slot));
}

String_t
ui::reshack::ZoomSelector::getName(size_t slot) const
{
    // RHZoomSelector::getName(int slot)
    return afl::string::Format("%dx", 1 << slot);
}

size_t
ui::reshack::ZoomSelector::getNumSlots() const
{
    // RHZoomSelector::getNumSlots()
    return m_numSlots;
}

size_t
ui::reshack::ZoomSelector::getSlotFromKey(util::Key_t k) const
{
    // RHZoomSelector::getSlotFromKey(UIKey k)
    if (k >= '0' && k < '0' + m_numSlots) {
        return size_t(k - '0');
    } else {
        return nil;
    }
}
