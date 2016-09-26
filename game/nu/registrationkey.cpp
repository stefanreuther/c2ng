/**
  *  \file game/nu/registrationkey.cpp
  */

#include "game/nu/registrationkey.hpp"
#include "afl/string/format.hpp"

game::nu::RegistrationKey::RegistrationKey(afl::data::Access playerObject)
    : m_status(Unknown),
      m_line1(playerObject("username").toString()),
      m_line2(afl::string::Format("Account #%d", playerObject("accountid").toString()))
{
    String_t email = playerObject("email").toString();
    if (!email.empty()) {
        if (!m_line1.empty()) {
            m_line1 += ", ";
        }
        m_line1 += email;
    }
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

void
game::nu::RegistrationKey::setStatus(Status st)
{
    m_status = st;
}
