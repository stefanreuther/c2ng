/**
  *  \file game/browser/session.cpp
  */

#include "game/browser/session.hpp"

game::browser::Session::Session(afl::string::Translator& tx, afl::sys::LogListener& log)
    : m_translator(tx),
      m_log(log),
      m_browser(),
      m_accountManager(),
      m_userCallbackProxy(tx, log)
{ }

game::browser::Session::~Session()
{ }

afl::string::Translator&
game::browser::Session::translator()
{
    return m_translator;
}

afl::sys::LogListener&
game::browser::Session::log()
{
    return m_log;
}

std::auto_ptr<game::browser::Browser>&
game::browser::Session::browser()
{
    return m_browser;
}

std::auto_ptr<game::browser::AccountManager>&
game::browser::Session::accountManager()
{
    return m_accountManager;
}

game::browser::UserCallbackProxy&
game::browser::Session::userCallbackProxy()
{
    return m_userCallbackProxy;
}
