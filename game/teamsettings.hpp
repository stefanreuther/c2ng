/**
  *  \file game/teamsettings.hpp
  *  \brief Class game::TeamSettings
  */
#ifndef C2NG_GAME_TEAMSETTINGS_HPP
#define C2NG_GAME_TEAMSETTINGS_HPP

#include "afl/base/signal.hpp"
#include "afl/base/uncopyable.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/charset/charset.hpp"
#include "afl/io/directory.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "game/playerarray.hpp"
#include "game/playerset.hpp"
#include "util/skincolor.hpp"

namespace game {

    /** Team settings.
        Defines informal alliances and therefore the colors being used.
        Players are assigned to teams.
        By default, each player is in their own team with team Id = player Id.
        By setting multiple players' team Ids identical, players become teammates.
        Teams can also be assigned names.

        This is an entirely client-side concept. */
    class TeamSettings : private afl::base::Uncopyable {
     public:
        /** Player relations. */
        enum Relation {
            ThisPlayer,         // ex is_Me
            AlliedPlayer,       // ex is_Ally
            EnemyPlayer         // ex is_Enemy
        };

        /** Values for send/receive configuration.
            These values are part of the binary representation, do not change. */
        enum MessageType {
            PlanetList,              // ex tx_Informer
            PlanetInformation,       // ex tx_VPAplan
            MinefieldInformation,    // ex tx_VPAmine
            ResultAccess,            // ex tx_RSTcookie
            DrawingInformation       // @since 2.41
        };
        typedef afl::bits::SmallSet<MessageType> MessageTypes_t;


        /** Constructor.
            Makes default team settings. */
        TeamSettings();

        /** Destructor. */
        ~TeamSettings();

        /** Reset to default settings. */
        void clear();

        /** Get team number for a player.
            \param player Player number
            \return team number */
        int getPlayerTeam(int player) const;

        /** Set team number for a player.
            \param player Player
            \param team Team */
        void setPlayerTeam(int player, int team);

        /** Remove player from their team.
            Moves them into a team of their own.
            \param player Player */
        void removePlayerTeam(int player);

        /** Get number of team members in a team.
            \param team Team number
            \return number of members */
        int getNumTeamMembers(int team) const;

        /** Get name of a team.
            \param team Team number
            \param tx Translator (for formatting defaults)
            \return name or default; never empty */
        String_t getTeamName(int team, afl::string::Translator& tx) const;

        /** Set name of a team.
            \param team Team number
            \param name Name */
        void setTeamName(int team, const String_t& name);

        /** Check for named team.
            \param team Team
            \return true if this team has a nonempty name assigned (getTeamName will not return default) */
        bool isNamedTeam(int team) const;

        /** Get players in a team.
            \param team Team
            \return Number of players in this team */
        PlayerSet_t getTeamPlayers(int team) const;

        /** Check for team configuration.
            \return true if any setting differs from the default (team numbers, names) */
        bool hasAnyTeams() const;

        /** Set viewpoint player.
            This is the player we're giving commands as, which will be reported as ThisPlayer.
            \param player Player number */
        void setViewpointPlayer(int player);

        /** Get viewpoint player.
            \return player number
            \see setViewpointPlayer */
        int getViewpointPlayer() const;

        /** Get relation from viewpoint to a player.
            \param player Player number
            \return relation */
        Relation getPlayerRelation(int player) const;

        /** Get player color.
            \param player Player
            \return color */
        util::SkinColor::Color getPlayerColor(int player) const;

        /** Get color for a relation.
            \param relation Relation to check
            \return color */
        static util::SkinColor::Color getRelationColor(Relation relation);

        /** Set send configuration for a player.
            \param player Player
            \param config New configuration */
        void setSendConfiguration(int player, MessageTypes_t config);

        /** Get send configuration for a player.
            \param player Player
            \return configuration */
        MessageTypes_t getSendConfiguration(int player) const;

        /** Get set of all supported send configurations.
            Only values in this set have an effect when set in getSendConfiguration/setSendConfiguration.
            \return set */
        static MessageTypes_t getAllSendConfigurations();

        /** Set receive configuration for a player.
            \param player Player
            \param config New configuration */
        void setReceiveConfiguration(int player, MessageTypes_t config);

        /** Get receive configuration for a player.
            \param player Player
            \return configuration */
        MessageTypes_t getReceiveConfiguration(int player) const;

        /** Get set of all supported receive configurations.
            Only values in this set have an effect when set in getReceiveConfiguration/setReceiveConfiguration.
            \return set */
        static MessageTypes_t getAllReceiveConfigurations();

        /** Synchronize data transfer configuration (send/receive settings) from team settings. */
        void synchronizeDataTransferConfigurationFromTeams();

        /** Set passcode for PlanetList transmissions.
            \param code New passcode (0..9999) */
        void setPasscode(int code);

        /** Get passcode for PlanetList transmissions.
            \return passcode */
        int getPasscode() const;


        /** Load from file.
            \param dir Directory
            \param player Player number
            \param cs Game character set
            \param tx Translator (for exception message) */
        void load(afl::io::Directory& dir, int player, afl::charset::Charset& cs, afl::string::Translator& tx);

        /** Save to file.
            \param dir Directory
            \param player Player number
            \param cs Game character set */
        void save(afl::io::Directory& dir, int player, afl::charset::Charset& cs) const;

        /** Copy from other settings object.
            Use instead of an assignment operator; this will raise signals accordingly.
            \param other Other object */
        void copyFrom(const TeamSettings& other);

        /** Signal: player/team configuration changed.
            Raised when any configuration in this object changes. */
        // ex game/team.h:sig_team_change (and sig_player_change)
        afl::base::Signal<void()> sig_teamChange;

     private:
        int m_flags;
        int m_viewpointPlayer;
        int m_passcode;
        PlayerArray<int> m_playerTeams;
        PlayerArray<String_t> m_teamNames;
        PlayerArray<int> m_sendConfig;
        PlayerArray<int> m_receiveConfig;
    };

}

#endif
