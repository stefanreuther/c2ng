/**
  *  \file game/v3/passwordchecker.hpp
  *  \brief Class game::v3::PasswordChecker
  */
#ifndef C2NG_GAME_V3_PASSWORDCHECKER_HPP
#define C2NG_GAME_V3_PASSWORDCHECKER_HPP

#include "afl/base/signalconnection.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/browser/usercallback.hpp"
#include "game/task.hpp"
#include "game/turn.hpp"

namespace game { namespace v3 {

    /** Check v3 turn password.
        This class implements checking a v3 turn password.
        To use,
        - create an instance of PasswordChecker in your TurnLoader::loadCurrentTurn()
        - load the "gen" files into the turn's GenExtra
        - if you decide to proceed with loading, call checkPassword(), passing it your "then" task
          (if you do not want to proceed, call "then" yourself). */
    class PasswordChecker {
     public:
        /** Constructor.
            @param turn      Turn
            @param pCallback Browser callback. If given as null, passwords are NOT checked.
            @param log       Logger
            @param tx        Translator */
        PasswordChecker(Turn& turn, game::browser::UserCallback* pCallback, afl::sys::LogListener& log, afl::string::Translator& tx);

        /** Check password.
            Call after loading the turn data, in particular, after loading the GenExtra.

            If a password is configured for the given player,
            will exercise an askPassword sequence on the browser callback and then call the "then" callback.

            If no password is configured, will immediately call the "then" callback.

            @param player Player number
            @param then   Status callback */
        void checkPassword(int player, std::auto_ptr<StatusTask_t> then);

     private:
        void onPasswordResult(game::browser::UserCallback::PasswordResponse resp);

        Turn& m_turn;
        game::browser::UserCallback* m_pCallback;
        afl::sys::LogListener& m_log;
        afl::string::Translator& m_translator;
        int m_player;
        std::auto_ptr<StatusTask_t> m_then;
        afl::base::SignalConnection conn_passwordResult;
    };

} }

#endif
