/**
  *  \file server/user/multipasswordencrypter.cpp
  *  \brief Class server::user::MultiPasswordEncrypter
  */

#include "server/user/multipasswordencrypter.hpp"

server::user::MultiPasswordEncrypter::MultiPasswordEncrypter(PasswordEncrypter& primary, PasswordEncrypter& secondary)
    : m_primary(primary),
      m_secondary(secondary)
{ }

String_t
server::user::MultiPasswordEncrypter::encryptPassword(String_t password, String_t userId)
{
    return m_primary.encryptPassword(password, userId);
}

server::user::PasswordEncrypter::Result
server::user::MultiPasswordEncrypter::checkPassword(String_t password, String_t hash, String_t userId)
{
    Result r = m_primary.checkPassword(password, hash, userId);
    if (r == Invalid) {
        r = m_secondary.checkPassword(password, hash, userId);
        if (r == ValidCurrent) {
            r = ValidNeedUpdate;
        }
    }
    return r;
}
