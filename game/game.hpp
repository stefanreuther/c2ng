/**
  *  \file game/game.hpp
  */
#ifndef C2NG_GAME_GAME_HPP
#define C2NG_GAME_GAME_HPP

#include "afl/base/ptr.hpp"
#include "afl/base/refcounted.hpp"
#include "afl/base/signal.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/historyturnlist.hpp"
#include "game/map/cursors.hpp"
#include "game/map/markings.hpp"
#include "game/msg/configuration.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/score/turnscorelist.hpp"
#include "game/teamsettings.hpp"
#include "game/unitscoredefinitionlist.hpp"

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

        game::map::Markings& markings();

        game::msg::Configuration& messageConfiguration();
        const game::msg::Configuration& messageConfiguration() const;

        /** Add message information.
            This is the general "I got some information somewhere" call.
            It will handle all sorts of information and add it to the current turn, treating it as scanner results.

            Restrictions:
            - ship information will be treated as unreliable (that is, this cannot create interceptable ships).
            - it will only add to the current turn, even if it's dated at an older turn.
            - future information will be discarded.

            \param info Information
            \param config Host configuration (can be updated with message information) */
        void addMessageInformation(const game::parser::MessageInformation& info, game::config::HostConfiguration& config);

        void synchronizeTeamsFromAlliances();

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
        game::map::Markings m_markings;

        game::msg::Configuration m_messageConfiguration;
    };

}

#endif
