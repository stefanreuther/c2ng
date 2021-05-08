/**
  *  \file game/sim/session.cpp
  */

#include "game/sim/session.hpp"

game::sim::Session::Session()
    : RefCounted(),
      m_setup(),
      m_config(),
      m_gameInterface()
{ }

game::sim::Session::~Session()
{ }

game::sim::Setup&
game::sim::Session::setup()
{
    return m_setup;
}

const game::sim::Setup&
game::sim::Session::setup() const
{
    return m_setup;
}

game::sim::Configuration&
game::sim::Session::configuration()
{
    return m_config;
}

const game::sim::Configuration&
game::sim::Session::configuration() const
{
    return m_config;
}

void
game::sim::Session::setNewGameInterface(GameInterface* gi)
{
    m_gameInterface.reset(gi);
}

game::sim::GameInterface*
game::sim::Session::getGameInterface() const
{
    return m_gameInterface.get();
}
