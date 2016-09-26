/**
  *  \file game/nu/gamestate.hpp
  */
#ifndef C2NG_GAME_NU_GAMESTATE_HPP
#define C2NG_GAME_NU_GAMESTATE_HPP

#include <memory>
#include "afl/base/refcounted.hpp"
#include "afl/data/access.hpp"
#include "game/browser/account.hpp"
#include "game/player.hpp"

namespace game { namespace nu {

    class BrowserHandler;

    class GameState : public afl::base::RefCounted {
     public:
        GameState(BrowserHandler& handler, game::browser::Account& acc, int32_t gameNr, size_t hint);

        ~GameState();

        afl::data::Access loadResult();

        afl::data::Access loadGameListEntry();

        void invalidateResult();


        static bool setRaceName(Player& pl, int race);

     private:
        BrowserHandler& m_handler;
        game::browser::Account& m_account;
        int32_t m_gameNr;
        size_t m_hint;

        bool m_resultValid;
        std::auto_ptr<afl::data::Value> m_result;
    };

} }

#endif
