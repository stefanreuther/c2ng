/**
  *  \file game/v3/passwordchecker.cpp
  *  \brief Class game::v3::PasswordChecker
  */

#include "game/v3/passwordchecker.hpp"
#include "afl/string/format.hpp"
#include "game/v3/genextra.hpp"
#include "game/v3/genfile.hpp"

using afl::sys::LogListener;
using game::AuthCache;
using game::v3::GenFile;

namespace {
    const char*const LOG_NAME = "game.v3";

    bool matchAuthCache(int player, const AuthCache& authCache, const GenFile& gen)
    {
        AuthCache::Item q;
        q.playerNr = player;

        AuthCache::Items_t found = authCache.find(q);
        for (size_t i = 0, n = found.size(); i < n; ++i) {
            if (const String_t* p = found[i]->password.get()) {
                if (gen.isPassword(*p)) {
                    return true;
                }
            }
        }
        return false;
    }
}

game::v3::PasswordChecker::PasswordChecker(Turn& turn, game::browser::UserCallback* pCallback, afl::sys::LogListener& log, afl::string::Translator& tx)
    : m_turn(turn), m_pCallback(pCallback), m_log(log), m_translator(tx),
      m_player(), m_then()
{ }

void
game::v3::PasswordChecker::checkPassword(int player, const AuthCache& authCache, std::auto_ptr<StatusTask_t> then)
{
    const GenFile* gen = GenExtra::get(m_turn, player);
    if (gen == 0 || !gen->hasPassword()) {
        // No password or none loaded
        m_log.write(LogListener::Trace, LOG_NAME, "PasswordChecker: no password");
        then->call(true);
    } else if (matchAuthCache(player, authCache, *gen)) {
        // Match AuthCache (--password option)
        m_log.write(LogListener::Trace, LOG_NAME, "PasswordChecker: match cached");
        then->call(true);
    } else if (m_pCallback == 0) {
        // Check disabled (for console apps)
        m_log.write(LogListener::Trace, LOG_NAME, "PasswordChecker: check disabled");
        then->call(true);
    } else {
        // Must ask user
        m_log.write(LogListener::Trace, LOG_NAME, "PasswordChecker: ask user");

        // Register for results and remember parameters
        conn_passwordResult = m_pCallback->sig_passwordResult.add(this, &PasswordChecker::onPasswordResult);
        m_player = player;
        m_then = then;

        // Ask user
        // Intentionally use "player X" instead of racenames to avoid ambiguities
        game::browser::UserCallback::PasswordRequest req;
        req.accountName = afl::string::Format(m_translator("player %d's turn"), player);
        m_pCallback->askPassword(req);
    }
}

void
game::v3::PasswordChecker::onPasswordResult(game::browser::UserCallback::PasswordResponse resp)
{
    if (resp.canceled) {
        m_log.write(LogListener::Trace, LOG_NAME, "PasswordChecker: canceled");
        m_then->call(false);
    } else {
        const GenFile* gen = GenExtra::get(m_turn, m_player);
        if (gen != 0 && gen->isPassword(resp.password)) {
            m_log.write(LogListener::Trace, LOG_NAME, "PasswordChecker: succeeded");
            m_then->call(true);
        } else {
            m_log.write(LogListener::Trace, LOG_NAME, "PasswordChecker: failed");
            m_then->call(false);
        }
    }
}
