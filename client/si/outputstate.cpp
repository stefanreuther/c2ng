/**
  *  \file client/si/outputstate.cpp
  */

#include "client/si/outputstate.hpp"

client::si::OutputState::OutputState()
    : m_process(),
      m_target(NoChange)
{ }

void
client::si::OutputState::set(RequestLink2 p, Target t)
{
    m_process = p;
    m_target = t;
}

client::si::RequestLink2
client::si::OutputState::getProcess() const
{
    return m_process;
}

client::si::OutputState::Target
client::si::OutputState::getTarget() const
{
    return m_target;
}
