/**
  *  \file game/browser/optionalusercallback.cpp
  *  \brief Class game::browser::OptionalUserCallback
  */

#include "game/browser/optionalusercallback.hpp"

game::browser::OptionalUserCallback::OptionalUserCallback()
    : m_pInstance(0),
      conn_passwordResult()
{ }

game::browser::OptionalUserCallback::~OptionalUserCallback()
{ }

void
game::browser::OptionalUserCallback::setInstance(UserCallback* pInstance)
{
    conn_passwordResult.disconnect();
    m_pInstance = pInstance;
    if (pInstance != 0) {
        conn_passwordResult = pInstance->sig_passwordResult.add(&sig_passwordResult, &afl::base::Signal<void(PasswordResponse)>::raise);
    }
}

void
game::browser::OptionalUserCallback::askPassword(const PasswordRequest& req)
{
    if (m_pInstance != 0) {
        m_pInstance->askPassword(req);
    } else {
        PasswordResponse resp;
        resp.canceled = true;
        sig_passwordResult.raise(resp);
    }
}
