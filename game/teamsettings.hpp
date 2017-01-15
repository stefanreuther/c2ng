/**
  *  \file game/teamsettings.hpp
  */
#ifndef C2NG_GAME_TEAMSETTINGS_HPP
#define C2NG_GAME_TEAMSETTINGS_HPP

#include "game/playerarray.hpp"
#include "afl/string/string.hpp"
#include "afl/base/signal.hpp"
#include "afl/string/translator.hpp"

namespace game {

    class TeamSettings {
     public:
        enum Relation {
            ThisPlayer,         // ex is_Me
            AlliedPlayer,       // ex is_Ally
            EnemyPlayer         // ex is_Enemy
        };

        TeamSettings();
        ~TeamSettings();

        void clear();

        int getPlayerTeam(int player) const;
        void setPlayerTeam(int player, int team);

        void removePlayerTeam(int player);
        int getNumTeamMembers(int team) const;

        String_t getTeamName(int team, afl::string::Translator& tx) const;
        void setTeamName(int team, const String_t& name);
        bool isNamedTeam(int team) const;

        bool hasAnyTeams() const;

        void setViewpointPlayer(int player);
        int getViewpointPlayer() const;
        Relation getPlayerRelation(int player) const;

        // ex game/team.h:sig_team_change (and sig_player_change)
        afl::base::Signal<void()> sig_teamChange;

     private:
        int m_flags;
        int m_viewpointPlayer;
        PlayerArray<int> m_playerTeams;
        PlayerArray<String_t> m_teamNames;
        PlayerArray<int> m_sendConfig;
        PlayerArray<int> m_receiveConfig;
    };

}

#endif
