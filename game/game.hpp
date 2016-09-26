/**
  *  \file game/game.hpp
  */
#ifndef C2NG_GAME_GAME_HPP
#define C2NG_GAME_GAME_HPP

#include "game/unitscoredefinitionlist.hpp"
#include "afl/base/refcounted.hpp"
#include "game/score/turnscorelist.hpp"
#include "game/map/cursors.hpp"
#include "game/teamsettings.hpp"
#include "game/historyturnlist.hpp"
#include "afl/base/ptr.hpp"
#include "afl/base/signal.hpp"

namespace game {

    class Turn;

    class Game : public afl::base::RefCounted {
     public:
        Game();
        ~Game();

        Turn& currentTurn();
        const Turn& currentTurn() const;

        HistoryTurnList& previousTurns();
        const HistoryTurnList& previousTurns() const;

        UnitScoreDefinitionList& planetScores();
        const UnitScoreDefinitionList& planetScores() const;
        UnitScoreDefinitionList& shipScores();
        const UnitScoreDefinitionList& shipScores() const;

        int getViewpointPlayer() const;
        void setViewpointPlayer(int pid);

        afl::base::Ptr<Turn> getViewpointTurn() const;
        int getViewpointTurnNumber() const;
        void setViewpointTurnNumber(int nr);

        TeamSettings& teamSettings();
        const TeamSettings& teamSettings() const;

        game::score::TurnScoreList& scores();
        const game::score::TurnScoreList& scores() const;

        game::map::Cursors& cursors();

        void notifyListeners();

        afl::base::Signal<void()> sig_viewpointTurnChange;

     private:
        afl::base::Ptr<Turn> m_currentTurn;
        HistoryTurnList m_previousTurns;

        UnitScoreDefinitionList m_planetScores;
        UnitScoreDefinitionList m_shipScores;

        TeamSettings m_teamSettings;
        int m_viewpointTurnNumber;

        game::score::TurnScoreList m_scores;

        game::map::Cursors m_cursors;
    };

}

#endif
