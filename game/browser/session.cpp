/**
  *  \file game/browser/session.cpp
  *  \brief Class game::browser::Session
  */

#include "game/browser/session.hpp"

using afl::sys::LogListener;

namespace {
    const char*const LOG_NAME = "game.browser";
}

game::browser::Session::Session(afl::string::Translator& tx, afl::sys::LogListener& log)
    : m_translator(tx),
      m_log(log),
      m_browser(),
      m_accountManager(),
      m_userCallbackProxy(tx, log),
      m_tasks()
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

void
game::browser::Session::addTask(std::auto_ptr<Task_t> task)
{
    m_tasks.pushBackNew(task.release());
    if (m_tasks.size() == 1) {
        m_log.write(LogListener::Trace, LOG_NAME, "(new task)");
        m_tasks.front()->call();
    }
}

void
game::browser::Session::finishTask()
{
    m_tasks.popFront();
    if (!m_tasks.empty()) {
        m_log.write(LogListener::Trace, LOG_NAME, "(queued task)");
        m_tasks.front()->call();
    }
}
