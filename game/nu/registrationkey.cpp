/**
  *  \file game/nu/registrationkey.cpp
  *  \brief Class game::nu::RegistrationKey
  */

#include "game/nu/registrationkey.hpp"
#include "afl/string/format.hpp"
#include "util/string.hpp"

game::nu::RegistrationKey::RegistrationKey(afl::data::Access accountObject)
    : m_status(accountObject("isregistered").toInteger() ? Registered : Unregistered),
      m_line1(accountObject("account")("username").toString()),
      m_line2(afl::string::Format("Account #%d", accountObject("account")("id").toString()))
{
    util::addListItem(m_line1, ", ", accountObject("account")("email").toString());
}

game::nu::RegistrationKey::~RegistrationKey()
{ }

game::RegistrationKey::Status
game::nu::RegistrationKey::getStatus() const
{
    return m_status;
}

String_t
game::nu::RegistrationKey::getLine(Line which) const
{
    switch (which) {
     case Line1:
        return m_line1;
     case Line2:
        return m_line2;
     case Line3:
     case Line4:
        return String_t();
    }
    return String_t();
}

bool
game::nu::RegistrationKey::setLine(Line /*which*/, String_t /*value*/)
{
    return false;
}

int
game::nu::RegistrationKey::getMaxTechLevel(TechLevel /*area*/) const
{
    return (getStatus() == Registered ? 10 : 7);
}
