/**
  *  \file client/si/inputstate.cpp
  */

#include "client/si/inputstate.hpp"

client::si::InputState::InputState()
    : m_process()
{ }

void
client::si::InputState::setProcess(RequestLink2 p)
{
    m_process = p;
}


client::si::RequestLink2
client::si::InputState::getProcess() const
{
    return m_process;
}
