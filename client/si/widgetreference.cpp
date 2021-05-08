/**
  *  \file client/si/widgetreference.cpp
  *  \brief Class client::si::WidgetReference
  */

#include "client/si/widgetreference.hpp"
#include "client/si/widgetholder.hpp"

client::si::WidgetReference::WidgetReference(afl::base::Ref<WidgetHolder> holder, size_t slot)
    : m_holder(holder),
      m_slot(slot)
{ }

client::si::WidgetReference::WidgetReference(const WidgetReference& other)
    : m_holder(other.m_holder),
      m_slot(other.m_slot)
{ }

client::si::WidgetReference::~WidgetReference()
{ }

ui::Widget*
client::si::WidgetReference::get(Control& ctl) const
{
    return m_holder->get(ctl, m_slot);
}

client::si::WidgetHolder&
client::si::WidgetReference::getHolder() const
{
    return *m_holder;
}

size_t
client::si::WidgetReference::getSlot() const
{
    return m_slot;
}

client::si::WidgetReference
client::si::WidgetReference::makePeer(size_t peerSlot) const
{
    return WidgetReference(m_holder, peerSlot);
}
