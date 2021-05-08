/**
  *  \file game/game.hpp
  *  \brief Class game::Game
  */
#ifndef C2NG_GAME_GAME_HPP
#define C2NG_GAME_GAME_HPP

#include "afl/base/ptr.hpp"
#include "afl/base/ref.hpp"
#include "afl/base/refcounted.hpp"
#include "afl/base/signal.hpp"
#include "game/config/expressionlists.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/historyturnlist.hpp"
#include "game/map/cursors.hpp"
#include "game/map/selections.hpp"
#include "game/msg/configuration.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/score/turnscorelist.hpp"
#include "game/spec/componentvector.hpp"
#include "game/teamsettings.hpp"
#include "game/unitscoredefinitionlist.hpp"
#include "game/vcr/object.hpp"

namespace game {

    class Turn;

    /** Game.
        Represents the status of a game, with
        - current and history turn
        - score history information
        - cross-turn configuration and status (messages, teams, selections) */
    class Game : public afl::base::RefCounted {
     public:
        /** Default constructor.
            Makes an empty Game. */
        Game();

        /** Destructor. */
        ~Game();

        /** Access current turn.
            Note that the turn is dynamically allocated, so you can initialize a Ref from it.
            \return current turn */
        Turn& currentTurn();
        const Turn& currentTurn() const;

        /** Access list of previous turns.
            \return list of previous turns */
        HistoryTurnList& previousTurns();
        const HistoryTurnList& previousTurns() const;

        /** Access planet score definitions.
            \return planet score definitions */
        UnitScoreDefinitionList& planetScores();
        const UnitScoreDefinitionList& planetScores() const;

        /** Access ship score definitions.
            \return ship score definitions */
        UnitScoreDefinitionList& shipScores();
        const UnitScoreDefinitionList& shipScores() const;

        /** Get viewpoint player.
            \return viewpoint player
            \see TeamSettings::getViewpointPlayer */
        int getViewpointPlayer() const;

        /** Set viewpoint player.
            \param playerNr viewpoint player
            \see TeamSettings::setViewpointPlayer */
        void setViewpointPlayer(int playerNr);

        /** Get viewpoint turn.
            \return turn (can be null) */
        afl::base::Ptr<Turn> getViewpointTurn() const;

        /** Get viewpoint turn number.
            \return turn number */
        int getViewpointTurnNumber() const;

        /** Set viewpoint turn number.
            If this changes the viewpoint turn, it will emit sig_viewpointTurnChange.
            \param nr Turn number */
        void setViewpointTurnNumber(int nr);

        /** Access team settings.
            \return team settings */
        TeamSettings& teamSettings();
        const TeamSettings& teamSettings() const;

        /** Access score history.
            \return score history */
        game::score::TurnScoreList& scores();
        const game::score::TurnScoreList& scores() const;

        /** Access object cursors.
            \return object cursors */
        game::map::Cursors& cursors();

        /** Access object selections.
            \return object selections */
        game::map::Selections& selections();

        /** Access message configuration.
            \return message configuration */
        game::msg::Configuration& messageConfiguration();
        const game::msg::Configuration& messageConfiguration() const;

        /** Access expression lists.
            \return expression lists */
        game::config::ExpressionLists& expressionLists();
        const game::config::ExpressionLists& expressionLists() const;

        /** Add message information.
            This is the general "I got some information somewhere" call.
            It will handle all sorts of information and add it to the current turn, treating it as scanner results.

            Restrictions:
            - ship information will be treated as unreliable (that is, this cannot create interceptable ships).
            - it will only add to the current turn, even if it's dated at an older turn.
            - future information will be discarded.

            \param info Information
            \param config Host configuration (can be updated with message information)
            \param msgNr If this information is from a message, its number */
        void addMessageInformation(const game::parser::MessageInformation& info,
                                   game::config::HostConfiguration& config,
                                   afl::base::Optional<size_t> msgNr);

        /** Synchronize teams from alliances.
            If we are allied with a player, adds them to our team;
            if we are not allied with a player, removes them. */
        void synchronizeTeamsFromAlliances();

        /** Notify listeners.
            Invokes all listeners on current and viewpoint turn. */
        void notifyListeners();

        /** Check for presence of a VCR object in game.
            \param obj VCR object
            \param hulls Hulls
            \return true if object corresponds to a game unit */
        bool isGameObject(const game::vcr::Object& obj, const game::spec::HullVector_t& hulls) const;

        /** Signal: viewpoint turn change. */
        afl::base::Signal<void()> sig_viewpointTurnChange;

     private:
        afl::base::Ref<Turn> m_currentTurn;
        HistoryTurnList m_previousTurns;

        UnitScoreDefinitionList m_planetScores;
        UnitScoreDefinitionList m_shipScores;

        TeamSettings m_teamSettings;
        int m_viewpointTurnNumber;

        game::score::TurnScoreList m_scores;

        game::map::Cursors m_cursors;
        game::map::Selections m_selections;

        game::msg::Configuration m_messageConfiguration;

        game::config::ExpressionLists m_expressionLists;
    };

}

#endif
