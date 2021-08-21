/**
  *  \file game/teamsettings.cpp
  *  \brief Class game::TeamSettings
  */

#include "game/teamsettings.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/value.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/format.hpp"
#include "util/io.hpp"
#include "util/skincolor.hpp"
#include "util/updater.hpp"

namespace {
    /*
     *  Definition of team file format
     *
     *  This file is shared between all PCC versions since 1.0.2.
     *  - TeamHeader
     *  - 12 team names
     *  - TransferSettings
     */
    typedef afl::bits::Value<afl::bits::Int16LE> Int16_t;

    const int NUM_HEADER_TEAMS = 12;
    const int NUM_DATA_PLAYERS = 11;

    static const char TEAM_MAGIC[] = { 'C', 'C', 't', 'e', 'a', 'm', '0', 26 };
    static_assert(sizeof(TEAM_MAGIC) == 8, "sizeof TEAM_MAGIC");

    struct TeamHeader {
        char signature[sizeof(TEAM_MAGIC)];
        Int16_t flags;
        uint8_t playerTeams[NUM_HEADER_TEAMS];
        uint8_t playerColors[NUM_HEADER_TEAMS];
    };
    static_assert(sizeof(TeamHeader) == 34, "sizeof TeamHeader");

    struct TransferSettings {
        uint8_t sendConfig[NUM_DATA_PLAYERS];
        uint8_t receiveConfig[NUM_DATA_PLAYERS];
        Int16_t passcode;
    };
    static_assert(sizeof(TransferSettings) == 24, "sizeof TransferSettings");
}


// Constructor.
game::TeamSettings::TeamSettings()
{
    clear();
}

// Destructor.
game::TeamSettings::~TeamSettings()
{ }

// Reset to default settings.
void
game::TeamSettings::clear()
{
    // ex game/team.cc:doneTeams
    // ex phost.pas:DefaultTeams (note: default flags in PCC1: tf_AutoSync(1) + tf_XferSync(2)
    m_flags = 0;
    m_viewpointPlayer = 0;
    m_passcode = 0;
    m_playerTeams.setAll(0);
    m_teamNames.setAll(String_t());
    m_sendConfig.setAll(0);
    m_receiveConfig.setAll(0);

    for (int i = 0; i <= MAX_PLAYERS; ++i) {
        m_playerTeams.set(i, i);
    }
    sig_teamChange.raise();
}

// Get team number for a player.
int
game::TeamSettings::getPlayerTeam(int player) const
{
    // ex game/team.h:getPlayerTeam
    return m_playerTeams.get(player);
}

// Set team number for a player.
void
game::TeamSettings::setPlayerTeam(int player, int team)
{
    // ex game/team.h:setPlayerTeam
    if (team != m_playerTeams.get(player)) {
        m_playerTeams.set(player, team);
        sig_teamChange.raise();
    }
}

// Remove player from their team.
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

// Get number of team members in a team.
int
game::TeamSettings::getNumTeamMembers(int team) const
{
    // ex game/team.h:getNumTeamMembers
    // ex phost.pas:IsFreeTeam
    int result = 0;
    for (int i = 1; i <= MAX_PLAYERS; ++i) {
        if (m_playerTeams.get(i) == team) {
            ++result;
        }
    }
    return result;
}

// Get name of a team.
String_t
game::TeamSettings::getTeamName(int team, afl::string::Translator& tx) const
{
    // ex game/team.h:getTeamName
    String_t result = m_teamNames.get(team);
    if (result.empty()) {
        result = afl::string::Format(tx("Team %d"), team);
    }
    return result;
}

// Set name of a team.
void
game::TeamSettings::setTeamName(int team, const String_t& name)
{
    // ex game/team.h:setTeamName
    if (m_teamNames.get(team) != name) {
        m_teamNames.set(team, name);
        sig_teamChange.raise();
    }
}

// Check for named team.
bool
game::TeamSettings::isNamedTeam(int team) const
{
    // ex game/team.h:isNamedTeam
    return !m_teamNames.get(team).empty();
}

// Get players in a team.
game::PlayerSet_t
game::TeamSettings::getTeamPlayers(int team) const
{
    PlayerSet_t result;
    for (int i = 1; i <= MAX_PLAYERS; ++i) {
        if (m_playerTeams.get(i) == team) {
            result += i;
        }
    }
    return result;
}

// Check for team configuration.
bool
game::TeamSettings::hasAnyTeams() const
{
    // ex game/team.h:isTeamConfigured
    // FIXME: this does not consider m_passcode, m_sendConfig, m_receiveConfig.
    // '!hasAnyTeams()' therefore is not sufficient to determine whether to save team.cc.
    for (int i = 1; i <= MAX_PLAYERS; ++i) {
        if (m_playerTeams.get(i) != i || !m_teamNames.get(i).empty()) {
            return true;
        }
    }
    return false;
}

// Set viewpoint player.
void
game::TeamSettings::setViewpointPlayer(int player)
{
    // ex game/team.h:setPlayerIds (sort-of)
    if (m_viewpointPlayer != player) {
        m_viewpointPlayer = player;
        sig_teamChange.raise();
    }
}

// Get viewpoint player.
int
game::TeamSettings::getViewpointPlayer() const
{
    // ex game/team.h:getPlayerId
    return m_viewpointPlayer;
}

// Get relation from viewpoint to a player.
game::TeamSettings::Relation
game::TeamSettings::getPlayerRelation(int player) const
{
    // ex global.pas:FriendOrFoe
    if (player == m_viewpointPlayer) {
        return ThisPlayer;
    } else if (m_playerTeams.get(m_viewpointPlayer) != 0 && m_playerTeams.get(player) == m_playerTeams.get(m_viewpointPlayer)) {
        return AlliedPlayer;
    } else {
        return EnemyPlayer;
    }
}

// Get player color.
util::SkinColor::Color
game::TeamSettings::getPlayerColor(int player) const
{
    // ex game/chart.h:getPlayerColor
    return getRelationColor(getPlayerRelation(player));
}

// Get color for a relation.
util::SkinColor::Color
game::TeamSettings::getRelationColor(Relation relation)
{
    // ex getPlayerColor, sort-of
    switch (relation) {
     case TeamSettings::ThisPlayer:
        return util::SkinColor::Green;
     case TeamSettings::AlliedPlayer:
        return util::SkinColor::Yellow;
     case TeamSettings::EnemyPlayer:
        return util::SkinColor::Red;
    }
    return util::SkinColor::Static;
}

// Load from file.
void
game::TeamSettings::load(afl::io::Directory& dir, int player, afl::charset::Charset& cs, afl::string::Translator& tx)
{
    // ex game/team.cc:loadTeams, initTeams
    // ex phost.pas:LoadTeams
    // Start empty
    clear();

    // Load file if exists
    afl::base::Ptr<afl::io::Stream> in = dir.openFileNT(afl::string::Format("team%d.cc", player), afl::io::FileSystem::OpenRead);
    if (in.get() == 0) {
        return;
    }

    TeamHeader header;
    in->fullRead(afl::base::fromObject(header));
    if (std::memcmp(header.signature, TEAM_MAGIC, sizeof(TEAM_MAGIC)) != 0) {
        throw afl::except::FileFormatException(*in, tx("File is missing required signature"));
    }

    // Remember header data
    m_flags = header.flags;
    for (int i = 1; i <= NUM_HEADER_TEAMS; ++i) {
        int thisTeam = header.playerTeams[i-1];
        if (thisTeam >= 0 && thisTeam <= MAX_PLAYERS) {
            m_playerTeams.set(i, thisTeam);
        }
    }

    // Read names
    for (int i = 1; i <= NUM_HEADER_TEAMS; ++i) {
        try {
            m_teamNames.set(i, util::loadPascalString(*in, cs));
        }
        catch (std::exception&) {
            // Silently ignore problems; in particular, file truncation
        }
    }

    // Read data transfer settings
    TransferSettings settings;
    if (in->read(afl::base::fromObject(settings)) == sizeof(settings)) {
        for (int i = 1; i <= NUM_DATA_PLAYERS; ++i) {
            m_sendConfig.set(i, settings.sendConfig[i-1]);
            m_receiveConfig.set(i, settings.receiveConfig[i-1]);
        }
        m_passcode = settings.passcode;
    }
    in.reset();

    sig_teamChange.raise();
}

// Save to file.
void
game::TeamSettings::save(afl::io::Directory& dir, int player, afl::charset::Charset& cs) const
{
    // ex phost.pas:SaveTeams
    afl::base::Ref<afl::io::Stream> out = dir.openFile(afl::string::Format("team%d.cc", player), afl::io::FileSystem::Create);

    // Header
    TeamHeader header;
    std::memcpy(header.signature, TEAM_MAGIC, sizeof(TEAM_MAGIC));
    header.flags = static_cast<int16_t>(m_flags);
    for (int i = 1; i <= NUM_HEADER_TEAMS; ++i) {
        // Fill in team assignments. Team colors are not used by
        // anything, so fill in some defaults (similar to PCC 1.x).
        header.playerTeams[i-1]  = static_cast<uint8_t>(m_playerTeams.get(i));
        header.playerColors[i-1] = static_cast<uint8_t>(m_playerTeams.get(i) == m_playerTeams.get(player) ? 3 : 4);
    }
    out->fullWrite(afl::base::fromObject(header));

    // Names
    for (int i = 1; i <= NUM_HEADER_TEAMS; ++i) {
        util::storePascalStringTruncate(*out, m_teamNames.get(i), cs);
    }

    // Data transfer
    TransferSettings settings;
    for (int i = 1; i <= NUM_DATA_PLAYERS; ++i) {
        settings.sendConfig[i-1]    = static_cast<int8_t>(m_sendConfig.get(i));
        settings.receiveConfig[i-1] = static_cast<int8_t>(m_receiveConfig.get(i));
    }
    settings.passcode = static_cast<int16_t>(m_passcode);
    out->fullWrite(afl::base::fromObject(settings));
}

// Copy from other settings object.
void
game::TeamSettings::copyFrom(const TeamSettings& other)
{
    util::Updater u;
    u.set(m_flags,           other.m_flags);
    u.set(m_viewpointPlayer, other.m_viewpointPlayer);
    u.set(m_passcode,        other.m_passcode);
    u.set(m_playerTeams,     other.m_playerTeams);
    u.set(m_teamNames,       other.m_teamNames);
    u.set(m_sendConfig,      other.m_sendConfig);
    u.set(m_receiveConfig,   other.m_receiveConfig);
    if (u) {
        sig_teamChange.raise();
    }
}
