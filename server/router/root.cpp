/**
  *  \file server/router/root.cpp
  */

#include <memory>
#include <stdexcept>
#include "server/router/root.hpp"
#include "server/router/session.hpp"
#include "server/errors.hpp"
#include "afl/string/format.hpp"

namespace {
    const char*const LOG_NAME = "router";
}

server::router::Root::Root(util::process::Factory& factory,
                           server::common::IdGenerator& gen,
                           const Configuration& config,
                           server::interface::FileBase* pFileBase)
    : m_log(),
      m_factory(factory),
      m_generator(gen),
      m_pFileBase(pFileBase),
      m_config(config),
      m_sessions()
{ }

server::router::Root::~Root()
{ }

afl::sys::Log&
server::router::Root::log()
{
    return m_log;
}

server::router::Session&
server::router::Root::createSession(afl::base::Memory<const String_t> args)
{
    // ex router.cc:handleNewSessionRequest
    // Create session object
    std::auto_ptr<Session> p(new Session(m_factory, args, m_generator.createId(), m_log, m_pFileBase));

    // Check and resolve conflicts
    bool stopped = false;
    for (size_t i = 0, n = m_sessions.size(); i < n; ++i) {
        if (m_sessions[i] != 0 && m_sessions[i]->isActive()) {
            if (p->checkConflict(*m_sessions[i])) {
                // We have a conflict
                if (m_config.newSessionsWin) {
                    // New sessions wins
                    m_sessions[i]->stop();
                    stopped = true;
                } else {
                    // Old sessions wins
                    throw std::runtime_error(SESSION_CONFLICT);
                }
            }
        }
    }

    // Check for overload
    if (m_sessions.size() >= m_config.maxSessions || stopped) {
        removeExpiredSessions();
    }
    if (m_sessions.size() >= m_config.maxSessions) {
        throw std::runtime_error(TOO_MANY_SESSIONS);
    }

    // Start the session
    if (!p->start(m_config.serverPath)) {
        throw std::runtime_error(CANNOT_START_SESSION);
    }
    return *m_sessions.pushBackNew(p.release());
}

void
server::router::Root::restartSession(Session& s)
{
    // ex RouterSession::restart
    s.stop();
    if (!s.start(m_config.serverPath)) {
        throw std::runtime_error(CANNOT_START_SESSION);
    }
}

const server::router::Configuration&
server::router::Root::config() const
{
    return m_config;
}

const server::router::Root::Sessions_t&
server::router::Root::sessions() const
{
    return m_sessions;
}

server::router::Session*
server::router::Root::getSessionById(String_t id) const
{
    for (size_t i = 0, n = m_sessions.size(); i < n; ++i) {
        if (Session* p = m_sessions[i]) {
            if (p->getId() == id) {
                return p;
            }
        }
    }
    return 0;
}

void
server::router::Root::removeExpiredSessions()
{
    // ex router.cc:handleTimeouts
    try {
        // Pass 1: close all timed-out sessions
        const afl::sys::Time now = afl::sys::Time::getCurrentTime();
        for (size_t i = 0, n = m_sessions.size(); i < n; ++i) {
            if (Session* ps = m_sessions[i]) {
                if (ps->isActive()) {
                    const int32_t timeout = (ps->isUsed() ? m_config.normalTimeout : m_config.virginTimeout);
                    const int64_t expired = (now - ps->getLastAccessTime()).getMilliseconds() / 1000;
                    if (expired >= timeout) {
                        m_log.write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("session %d timed out", ps->getId()));
                        ps->stop();
                    }
                }
            }
        }

        // Pass 2: remove objects of terminated sessions (this includes sessions terminated for other reasons)
        size_t in = 0, out = 0;
        while (in < m_sessions.size()) {
            if (m_sessions[in] != 0 && m_sessions[in]->isActive()) {
                m_sessions.swapElements(in, out);
                ++out;
            } else {
            }
            ++in;
        }
        m_sessions.resize(out);
    }
    catch (std::exception& e)
    {
        // Exceptions must not cause this function to report an error to the user.
        m_log.write(afl::sys::LogListener::Error, LOG_NAME, afl::string::Format("Error during session cleanup: %s", e.what()));
    }
}

void
server::router::Root::stopAllSessions()
{
    for (size_t i = 0, n = m_sessions.size(); i < n; ++i) {
        if (Session* ps = m_sessions[i]) {
            ps->stop();
        }
    }
    m_sessions.clear();
}
