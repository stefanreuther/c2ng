/**
  *  \file client/map/overlay.cpp
  */

#include "client/map/overlay.hpp"
#include "client/map/callback.hpp"

client::map::Overlay::Overlay()
    : m_pCallback(0)
{ }

client::map::Overlay::~Overlay()
{
    if (m_pCallback != 0) {
        m_pCallback->removeOverlay(*this);
    }
}

void
client::map::Overlay::setCallback(Callback* p)
{
    m_pCallback = p;
}

client::map::Callback*
client::map::Overlay::getCallback()
{
    return m_pCallback;
}
