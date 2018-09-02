/**
  *  \file server/host/game.hpp
  *  \brief Class server::host::Game
  */
#ifndef C2NG_SERVER_HOST_GAME_HPP
#define C2NG_SERVER_HOST_GAME_HPP

#include "afl/net/redis/subtree.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/stringlistkey.hpp"
#include "afl/net/commandhandler.hpp"
#include "game/playerset.hpp"
#include "server/interface/hostgame.hpp"
#include "server/interface/hostplayer.hpp"
#include "server/common/racenames.hpp"
#include "afl/net/redis/stringfield.hpp"

namespace server { namespace host {

    class TalkListener;
    class Root;
    class Cron;

    /** Handle to a game.
        Provides operations on games in the database and implements the database schema. */
    class Game {
     public:
        typedef server::interface::HostGame::State State_t;
        typedef server::interface::HostGame::Type Type_t;

        static const int NUM_PLAYERS = 11;

        /** Handle to a slot.
            See Game::getSlot(). */
        class Slot {
         public:
            /** Constructor.
                \param tree Base */
            explicit Slot(afl::net::redis::Subtree tree);

            /** Access list of all users.
                \return key. Primary user is first, replacements at end. */
            afl::net::redis::StringListKey players();

            /** Access slot status.
                \return key. 0=unavailable slot, 1=slot exists in game. */
            afl::net::redis::IntegerField slotStatus();

            /** Access turn status.
                \return key. Values are TurnXxx (TurnMissing etc.) */
            afl::net::redis::IntegerField turnStatus();

            /** Access ranks.
                \return key. After end of game, 1=first, 2=second place etc.; 0=dead */
            afl::net::redis::IntegerField rank();
         private:
            afl::net::redis::Subtree m_tree;
        };

        /** Handle to a turn's "header" information.
            \see Game::Turn::info() */
        class TurnInfo {
         public:
            /** Constructor.
                \param key Base */
            explicit TurnInfo(afl::net::redis::HashKey key);

            /** Access turn time (integer format).
                \return key. Content is the same as Root::getTime(). */
            afl::net::redis::IntegerField time();

            /** Access turn time (VGAP string format).
                \return key. Content is the same as game::Timestamp. */
            afl::net::redis::StringField timestamp();

            /** Access turn status.
                \return key. Content is array of packed Int16LE. */
            afl::net::redis::StringField turnStatus();

            // /** Access relative directory name.
            //     \return key. Content is name of directory relative to game's directory. */
            // afl::net::redis::StringField relativeDirectory();

         private:
            afl::net::redis::HashKey m_key;
        };

        /** Handle to a turn's "backup files" information.
            \see Game::Turn::files() */
        class TurnFiles {
         public:
            /** Constructor.
                \param tree Base */
            TurnFiles(afl::net::redis::Subtree tree);

            /** Access a player's files.
                \param slot Slot number
                \return key. Content is set of file names */
            afl::net::redis::StringSetKey playerFiles(int slot);

            /** Access global files (e.g. specification).
                \param slot Slot number
                \return key. Content is set of file names */
            afl::net::redis::StringSetKey globalFiles();

         private:
            afl::net::redis::Subtree m_tree;
        };


        /** Handle to a turn.
            \see Game::turn() */
        // FIXME: the interface is subject to debate
        class Turn {
         public:
            /** Constructor.
                \param tree Base */
            explicit Turn(afl::net::redis::Subtree tree);

            /** Access scores.
                \return key. Field names are score names, content is scores in Int32LE format. */
            afl::net::redis::HashKey scores();

            /** Access turn information.
                \return key. Contains time, timestamp, turnstatus. */
            TurnInfo info();

            /** Access turn's backup file names.
                \return key. */
            TurnFiles files();

            /** Access player status.
                \return key. Field names are player numbers, content is primary player's user Id */
            afl::net::redis::HashKey playerId();

         private:
            afl::net::redis::Subtree m_tree;
        };


        /** Permission levels. */
        enum PermissionLevel {
            ReadPermission,             ///< Read (see) the game.
            ConfigPermission,           ///< Configure the game.
            AdminPermission             ///< Super-powers (currently equivalent to ConfigPermission).
        };

        /** Unit type to disable existence check. */
        enum NoExistanceCheck_t {
            NoExistanceCheck            ///< Disable existence check when passed to Game's constructor.
        };

        // FIXME: where to put these? HostGame interface?
        // FIXME: the HostTurn interface has its own set.
        // FIXME: turn into integers
        enum HostTurnState {
            TurnMissing     = 0,        /* No turn submitted */
            TurnGreen       = 1,        /* Turn was submitted and OK */
            TurnYellow      = 2,        /* Turn was submitted and yellow*/
            TurnRed         = 3,        /* Turn was submitted and red*/
            TurnBad         = 4,        /* Turn was submitted and damaged */
            TurnStale       = 5,        /* Turn was stale */
            TurnDead        = 6,        /* No turn submitted, but player was dead */

            TurnStateMask   = 15,
            TurnIsTemporary = 16
        };



        /** Constructor.
            \param root Service root
            \param gameId Game Id
            \throw std::exception if game does not exist */
        Game(Root& root, int32_t gameId);

        /** Constructor.
            This constructor does not verify that the game exists.
            Use in places where you know that the game exists, to save a database roundtrip.
            \param root Service root
            \param gameId Game Id
            \param n Pass \c NoExistanceCheck */
        Game(Root& root, int32_t gameId, NoExistanceCheck_t n);

        /** Destructor. */
        ~Game();

        /** Get game Id.
            \return Id as passed to constructor */
        int32_t getId() const;

        /** Get game state.
            \return state
            \throw std::exception if database value cannot be interpreted */
        State_t getState();

        /** Set game state.
            Updates all respective lists.
            \param newState New state
            \param talk TalkListener to notify
            \param root Service root */
        void setState(State_t newState, TalkListener* talk, Root& root);

        /** Get game type.
            \return type
            \throw std::exception if database value cannot be interpreted */
        Type_t getType();

        /** Set game type.
            \param newType New type
            \param talk TalkListener to notify
            \param root Service root */
        void setType(Type_t newType, TalkListener* talk, Root& root);

        /** Get game name.
            \return name */
        String_t getName();

        /** Set game name.
            \param newName New name
            \param talk TalkListener to notify */
        void setName(String_t newName, TalkListener* talk);

        /** Get game owner.
            \return owner user Id (can be "") */
        String_t getOwner();

        /** Set game owner.
            Updates all respective lists.
            \param newOwner new state
            \param root Service root */
        void setOwner(String_t newOwner, Root& root);

        /** Get configuration string value.
            \param name Configuration key
            \return value */
        String_t getConfig(String_t name);

        /** Set configuration string value.
            This sets the raw value and does not check any interactions / consistency rules.
            \param name Configuration key
            \param value Value */
        void setConfig(String_t name, String_t value);

        /** Get configuration integer value.
            \param name Configuration key
            \return value */
        int32_t getConfigInt(String_t name);

        /** Set configuration inter value.
            This sets the raw value and does not check any interactions / consistency rules.
            \param name Configuration key
            \param value Value */
        void setConfigInt(String_t name, int32_t value);

        /** Remove game configuration property.
            \param name Configuration key */
        void removeConfig(String_t name);

        /** Get game directory.
            \return directory (in hostfile service) */
        String_t getDirectory();

        /** Add a history item to the game history.
            The history item will have the form "time:what:gameId:args".
            It will be added to the game's history and optionally the global history.
            \param root Service root
            \param what Message name (e.g. "game-state")
            \param args Message arguments
            \param global true to add the item to the global history as well */
        void addGameHistoryItem(Root& root, String_t what, String_t args, bool global);

        /** Add a history item to user history.
            The history item will have the form "time:what:gameId:args".
            It will be added to the game's histroy and the user's history.
            \param root Service root
            \param what Message name (e.g. "game-state")
            \param args Message arguments
            \param player User Id */
        void addUserHistoryItem(Root& root, String_t what, String_t args, String_t player);

        /** Get per-user string configuration value.
            \param player User Id
            \param name Configuration key
            \return value */
        String_t getPlayerConfig(String_t player, String_t name);

        /** Set per-user string configuration value.
            \param player User Id
            \param name Configuration key
            \param value Value */
        void setPlayerConfig(String_t player, String_t name, String_t value);

        /** Get per-user integer configuration value.
            \param player User Id
            \param name Configuration key
            \return value */
        int32_t getPlayerConfigInt(String_t player, String_t name);

        /** Set per-user integer configuration value.
            \param player User Id
            \param name Configuration key
            \param value Value */
        void setPlayerConfigInt(String_t player, String_t name, int32_t value);

        /** Get name of score used to determine the game end.
            Unless explicitly configured, this value is not known before the first host run.
            \return name of score */
        String_t getRefereeScoreName();

        /** Access a slot (player position).
            \param slot Slot number [1,NUM_PLAYERS]
            \return slot handle */
        Slot getSlot(int32_t slot);

        /** Check whether slot exists in the game.
            \param slot Slot number. Out-of-range values are correctly recognized
            \return true if slot is exists (i.e. a player can play it) */
        bool isSlotInGame(int32_t slot);

        /** Check whether a slot is played.
            \param slot Slot number
            \return true if a player is subscribed to this slot */
        bool isSlotPlayed(int32_t slot);

        /** Check whether game has any open slots.
            \return true There exists a slot that is not played */
        bool hasAnyOpenSlot();

        /** Add player to a slot.
            If the slot was unplayed, the player becomes primary player; otherwise, the player becomes a replacement.
            This will update the database, host file permissions, and user file installations.
            It will not start a game that becomes full; this must be done by the caller.
            \param slot Slot number
            \param player User Id
            \param root Service root */
        void pushPlayerSlot(int32_t slot, String_t player, Root& root);

        /** Remove player from a slot.
            Dropping the last player from a slot makes it unplayed.
            This will update the database, host file permissions, and user file installations.
            It will not affect the scheduler.
            \param slot Slot number
            \param root Service root */
        String_t popPlayerSlot(int32_t slot, Root& root);

        /** Get all players in a slot.
            \param slot    [in] Slot number
            \param players [out] User Ids returned here. First is primary player
            \see server::host::Game::Slot::players() */
        void listPlayers(int32_t slot, afl::data::StringList_t& players);

        /** Get all slots played by a player.
            \param player User Id
            \return slot set */
        game::PlayerSet_t getSlotsByPlayer(String_t player);

        /** Get all slots.
            \return set of all slots in this game */
        game::PlayerSet_t getGameSlots();

        /** Clear cache.
            Removes the cache element of the game object. */
        void clearCache();

        /** Get difficulty.
            If it is not yet known, it is computed and cached.
            \param root Service root
            \return Difficulty [1,100] */
        int32_t getDifficulty(Root& root);

        /** Mark game broken.
            This function is intended to be called by the scheduler when it detects a problem.
            The game is marked broken, but not removed from scheduler lists;
            the scheduler will not pick it up after restarting.
            \param message Message for admin
            \param root Service root */
        void markBroken(String_t message, Root& root);

        /** Get schedule subtree.
            \return subtree */
        afl::net::redis::Subtree getSchedule();

        /** Access tools by kind.
            \return key. Field names are tool kinds, content is tool Id */
        afl::net::redis::HashKey toolsByKind();

        /** Access tool data.
            \param toolId Tool Id (name)
            \return key */
        afl::net::redis::Subtree toolData(const String_t& toolId);

        /** Access tools.
            \return key. Elements are tool Ids */
        afl::net::redis::StringSetKey tools();

        /** Access user reference counters.
            \return key. Field names are user Ids, content is reference counts */
        afl::net::redis::HashKey userReferenceCounters();

        /** Access score descriptions.
            \return key. Field names are score Ids, content is descriptions */
        afl::net::redis::HashKey scoreDescriptions();

        /** Access settings.
            \return key. Field names are config keys, content is values */
        afl::net::redis::HashKey settings();

        /** Access rank points.
            \return key. Field names are user Ids, content is rank points already awarded for this game */
        afl::net::redis::HashKey rankPoints();

        /** Access turn.
            \param nr Turn number
            \return handle */
        Turn turn(int32_t nr);

        /** Check whether user is or was on a game.
            \param user User Id
            \return true if user is or was on a game (has a reference counter) */
        bool isUserOnGame(String_t user);

        /** Check whether user is on this game as primary player.
            \param user User Id
            \return true if user currently is a primary player */
        bool isUserOnGameAsPrimary(String_t user);

        /** Check whether ranking is disabled in this game.
            \return value */
        bool isRankingDisabled();

        /** Check whether joining as multiple races is allowed in this game.
            \return value */
        bool isMultiJoinAllowed();

        /** Describe this game.
            This function creates a user-dependant view (turn states, joinability),
            but otherwise assumes the user has read access.
            \param verbose true to fill the structure completely; false to produce essential information only
            \param forUser user who is requesting this information (for viewpoint-dependant values)
            \param root Service root
            \return description */
        server::interface::HostGame::Info describe(bool verbose, String_t forUser, Root& root);

        /** Describe a slot.
            This function creates a user-dependant view (joinability),
            but otherwise assumes the user has read access.
            \param slot Slot to describe
            \param forUser user who is requesting this information (for viewpoint-dependant values)
            \param raceNames race names (see loadRaceNames())
            \return description */
        server::interface::HostPlayer::Info describeSlot(int32_t slot, String_t forUser, const server::common::RaceNames& raceNames);

        /** Describe victory condition.
            \param root Service root
            \return description */
        server::interface::HostGame::VictoryCondition describeVictoryCondition(Root& root);

        /** Check permissions.
            \param user User Id to check for (can be empty)
            \param level Level to check
            \return true if permission is granted */
        bool hasPermission(String_t user, PermissionLevel level);

        /** Load race names.
            If the game has been mastered and has its own race.nm file, loads that from the host filer.
            Otherwise, checks files provided by shiplist/master/host/defaults.
            \param raceNames [out] Result
            \param root Service root */
        void loadRaceNames(server::common::RaceNames& raceNames, Root& root);


        /*
         *  Settings accessors
         */

        /** Access "configuration changed" settings value.
            \return field. */
        afl::net::redis::IntegerField configChanged();

        /** Access "schedule changed" settings value.
            \return field. */
        afl::net::redis::IntegerField scheduleChanged();

        /** Access "end condition changed" settings value.
            \return field. */
        afl::net::redis::IntegerField endChanged();

        /** Access turn number.
            \return field. */
        afl::net::redis::IntegerField turnNumber();

        /** Access time of last schedule change.
            \return field. */
        afl::net::redis::IntegerField lastScheduleChangeTime();

        /** Access time of last host.
            \return field. */
        afl::net::redis::IntegerField lastHostTime();

        /** Access time of last turn submission.
            \return field. */
        afl::net::redis::IntegerField lastTurnSubmissionTime();

        /** Access forum number.
            \return field */
        afl::net::redis::IntegerField forumId();

        /** Access "forum disabled" status.
            \return field */
        afl::net::redis::IntegerField forumDisabled();

        /** Access "kick after missed turns" value.
            \return field */
        afl::net::redis::IntegerField numMissedTurnsForKick();

     private:
        afl::net::redis::Subtree m_game;
        const int32_t m_gameId;
    };
} }

#endif
