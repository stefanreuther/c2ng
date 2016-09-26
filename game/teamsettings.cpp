/**
  *  \file game/teamsettings.cpp
  */

#include "game/teamsettings.hpp"
#include "afl/string/format.hpp"

game::TeamSettings::TeamSettings()
{
    clear();
}

game::TeamSettings::~TeamSettings()
{ }

void
game::TeamSettings::clear()
{
    // ex game/team.cc:doneTeams
    m_flags = 0;
    m_viewpointPlayer = 0;
    m_playerTeams.setAll(0);
    m_teamNames.setAll(String_t());
    m_sendConfig.setAll(0);
    m_receiveConfig.setAll(0);

    for (int i = 0; i <= MAX_PLAYERS; ++i) {
        m_playerTeams.set(i, i);
    }
    sig_teamChange.raise();
}

// /** Get number of team a player is in. */
int
game::TeamSettings::getPlayerTeam(int player) const
{
    // ex game/team.h:getPlayerTeam
    return m_playerTeams.get(player);
}

// /** Change number of team a player is in. */
void
game::TeamSettings::setPlayerTeam(int player, int team)
{
    // ex game/team.h:setPlayerTeam
    if (team != m_playerTeams.get(player)) {
        m_playerTeams.set(player, team);
        sig_teamChange.raise();
    }
}

// /** Remove player from his team. Moves him into a team of his own. */
void
game::TeamSettings::removePlayerTeam(int player)
{
    // ex game/team.h:removePlayerTeam
    if (getNumTeamMembers(getPlayerTeam(player)) > 1) {
        if (getNumTeamMembers(player) == 0) {
            // We can put him into the team which has the same number as the player
            setPlayerTeam(player, player);
        } else {
            // Search for an unused team
            // By the pigeonhole principle, this will never produce a team number greater than the actual number of players in the game.
            for (int i = 1; i <= MAX_PLAYERS; ++i) {
                if (getNumTeamMembers(i) == 0) {
                    setPlayerTeam(player, i);
                    break;
                }
            }
        }
    }
}

// /** Get number of team members in a team. */
int
game::TeamSettings::getNumTeamMembers(int team) const
{
    // ex game/team.h:getNumTeamMembers
    int result = 0;
    for (int i = 1; i <= MAX_PLAYERS; ++i) {
        if (m_playerTeams.get(i) == team) {
            ++result;
        }
    }
    return result;
}

// /** Get name of a team. */
String_t
game::TeamSettings::getTeamName(int team, afl::string::Translator& tx) const
{
    // ex game/team.h:getTeamName
    String_t result = m_teamNames.get(team);
    if (result.empty()) {
        result = afl::string::Format(tx.translateString("Team %d").c_str(), team);
    }
    return result;
}

// /** Set name of a team. */
void
game::TeamSettings::setTeamName(int team, const String_t& name)
{
    // ex game/team.h:setTeamName
    if (m_teamNames.get(team) != name) {
        m_teamNames.set(team, name);
        sig_teamChange.raise();
    }
}

// /** Check whether team has a name. If it has not, getTeamName() will return a default name. */
bool
game::TeamSettings::isNamedTeam(int team) const
{
    // ex game/team.h:isNamedTeam
    return !m_teamNames.get(team).empty();
}

// /** Check whether there's any team configured. */
bool
game::TeamSettings::hasAnyTeams() const
{
    // ex game/team.h:isTeamConfigured
    for (int i = 1; i <= MAX_PLAYERS; ++i) {
        if (m_playerTeams.get(i) != i || !m_teamNames.get(i).empty()) {
            return true;
        }
    }
    return false;
}

// /** Set player Ids.
//     \param eff   "effective" id. The one in whose name we're giving commands.
//     \param real  "real" id. The one which was authenticated. */
void
game::TeamSettings::setViewpointPlayer(int player)
{
    // ex game/team.h:setPlayerIds (sort-of)
    if (m_viewpointPlayer != player) {
        m_viewpointPlayer = player;
        sig_teamChange.raise();
    }
}

// /** Get current player. That's the one whose data we're looking at,
//     0 if none. */
int
game::TeamSettings::getViewpointPlayer() const
{
    // ex game/team.h:getPlayerId
    return m_viewpointPlayer;
}

// /** Get relation to player n. */
game::TeamSettings::Relation
game::TeamSettings::getPlayerRelation(int player) const
{
    if (player == m_viewpointPlayer) {
        return ThisPlayer;
    } else if (m_playerTeams.get(m_viewpointPlayer) != 0 && m_playerTeams.get(player) == m_playerTeams.get(m_viewpointPlayer)) {
        return AlliedPlayer;
    } else {
        return EnemyPlayer;
    }
}
