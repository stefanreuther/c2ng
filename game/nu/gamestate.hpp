/**
  *  \file game/nu/gamestate.hpp
  *  \brief Class game::nu::GameState
  */
#ifndef C2NG_GAME_NU_GAMESTATE_HPP
#define C2NG_GAME_NU_GAMESTATE_HPP

#include <memory>
#include "afl/base/refcounted.hpp"
#include "afl/data/access.hpp"
#include "game/browser/account.hpp"
#include "game/player.hpp"
#include "game/task.hpp"

namespace game { namespace nu {

    class BrowserHandler;

    /** Shared state for a game.
        I am not aware of a function to download game meta-data,
        so we always download the entire result file to present a GameFolder.

        This object is used to pass information from the GameFolder to the actual game,
        to avoid downloading the result a second time. */
    class GameState : public afl::base::RefCounted {
     public:
        /** Constructor.
            @param handler BrowserHandler
            @param acc     Account
            @param gameNr  Game number
            @param hint    Position hint; the game is at this index in the game list. */
        GameState(BrowserHandler& handler, const afl::base::Ref<game::browser::Account>& acc, int32_t gameNr, size_t hint);

        /** Destructor. */
        ~GameState();

        /** Load result file, pre-authenticated.
            This downloads the result file when called the first time,
            or returns the previously returned data.
            The account must have been logged in already.
            If the account is not or no longer logged in, the request will fail (return null).
            @return Handle to result JSON */
        afl::data::Access loadResultPreAuthenticated();

        /** Get game list entry for this game, pre-authenticated.
            The account must have been logged in already.
            If the account is not or no longer logged in, the request will fail (return null).
            @return Handle to result JSON */
        afl::data::Access loadGameListEntryPreAuthenticated();

        /** Log in.
            Shortcut for BrowserHandler::login().
            @param then Task to execute after logging in
            @return Task */
        std::auto_ptr<Task_t> login(std::auto_ptr<Task_t> then);

        /** Invalidate previously downloaded result.
            The next call to loadResultPreAuthenticated() will again hit the network. */
        void invalidateResult();

        /** Populate Player object with names.
            @param [out] pl   Player object
            @param [in]  race Race number
            @return true on success, false if race number is out of range */
        static bool setRaceName(Player& pl, int race);

     private:
        BrowserHandler& m_handler;
        afl::base::Ref<game::browser::Account> m_account;
        int32_t m_gameNr;
        size_t m_hint;

        bool m_resultValid;
        std::auto_ptr<afl::data::Value> m_result;
    };

} }

#endif
