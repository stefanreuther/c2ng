/**
  *  \file game/nu/turnloader.hpp
  */
#ifndef C2NG_GAME_NU_TURNLOADER_HPP
#define C2NG_GAME_NU_TURNLOADER_HPP

#include "game/turnloader.hpp"
#include "game/nu/gamestate.hpp"
#include "afl/base/ptr.hpp"
#include "game/map/universe.hpp"
#include "afl/data/access.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"

namespace game { namespace nu {

    class TurnLoader : public game::TurnLoader {
     public:
        TurnLoader(afl::base::Ref<GameState> gameState,
                   afl::string::Translator& tx,
                   afl::sys::LogListener& log);
        ~TurnLoader();

        virtual PlayerStatusSet_t getPlayerStatus(int player, String_t& extra, afl::string::Translator& tx) const;
        virtual void loadCurrentTurn(Turn& turn, Game& game, int player, Root& root, Session& session);
        virtual void saveCurrentTurn(const Turn& turn, const Game& game, int player, const Root& root, Session& session);
        virtual void getHistoryStatus(int player, int turn, afl::base::Memory<HistoryStatus> status, const Root& root);
        virtual void loadHistoryTurn(Turn& turn, Game& game, int player, int turnNumber, Root& root);
        virtual String_t getProperty(Property p);

     private:
        afl::base::Ref<GameState> m_gameState;
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;

        void loadPlanets(game::map::Universe& univ, afl::data::Access planets, PlayerSet_t players);
        void loadStarbases(game::map::Universe& univ, afl::data::Access bases, PlayerSet_t players);
        void loadShips(game::map::Universe& univ, afl::data::Access ships, PlayerSet_t players);
        void loadMinefields(game::map::Universe& univ, afl::data::Access p);
        void loadVcrs(Turn& turn, afl::data::Access p);
    };

} }

#endif
