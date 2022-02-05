/**
  *  \file game/browser/session.cpp
  *  \brief Class game::browser::Session
  */

#include "game/browser/session.hpp"

using afl::sys::LogListener;

namespace {
    const char*const LOG_NAME = "game.browser";
}

game::browser::Session::Session(afl::io::FileSystem& fileSystem,
                                afl::string::Translator& tx,
                                afl::sys::LogListener& log,
                                util::ProfileDirectory& profile)
    : m_translator(tx),
      m_log(log),
      m_accountManager(profile, tx, log),
      m_callback(),
      m_browser(fileSystem, tx, log, m_accountManager, profile, m_callback),
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

game::browser::Browser&
game::browser::Session::browser()
{
    return m_browser;
}

game::browser::AccountManager&
game::browser::Session::accountManager()
{
    return m_accountManager;
}

game::browser::OptionalUserCallback&
game::browser::Session::callback()
{
    return m_callback;
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
