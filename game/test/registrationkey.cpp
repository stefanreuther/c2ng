/**
  *  \file game/test/registrationkey.cpp
  *  \brief Class game::test::RegistrationKey
  */

#include "game/test/registrationkey.hpp"

game::test::RegistrationKey::RegistrationKey(Status status, int maxTech)
    : m_status(status),
      m_maxTech(maxTech)
{ }

game::test::RegistrationKey::~RegistrationKey()
{ }

game::RegistrationKey::Status
game::test::RegistrationKey::getStatus() const
{
    return m_status;
}

String_t
game::test::RegistrationKey::getLine(Line /*which*/) const
{
    return "<Test>";
}

bool
game::test::RegistrationKey::setLine(Line /*which*/, String_t /*value*/)
{
    return false;
}

int
game::test::RegistrationKey::getMaxTechLevel(game::TechLevel /*area*/) const
{
    return m_maxTech;
}
