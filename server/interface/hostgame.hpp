/**
  *  \file server/interface/hostgame.hpp
  *  \brief Interface server::interface::HostGame
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTGAME_HPP
#define C2NG_SERVER_INTERFACE_HOSTGAME_HPP

#include <vector>
#include "afl/base/deletable.hpp"
#include "afl/base/optional.hpp"
#include "afl/base/types.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/data/integerlist.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/string/string.hpp"
#include "server/interface/hostschedule.hpp"
#include "server/interface/hosttool.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    /** Host game interface.
        This interface allows creation and manipulation of games. */
    class HostGame : public afl::base::Deletable {
     public:
        /** Game state. */
        enum State {
            Preparing,
            Joining,
            Running,
            Finished,
            Deleted
        };

        /** Gane type. */
        enum Type {
            PrivateGame,
            UnlistedGame,
            PublicGame,
            TestGame
        };

        /** Permission level. */
        enum Permission {
            UserIsOwner,              ///< User is owner of the game.
            UserIsPrimary,            ///< User is primary player on the game.
            UserIsActive,             ///< User is active replacement player.
            UserIsInactive,           ///< User is player on game, but neigher primary nor active.
            GameIsPublic              ///< User is not on game, but game is public.
        };
        typedef afl::bits::SmallSet<Permission> Permissions_t;

        /** Status of a slot. */
        enum SlotState {
            OpenSlot,                 ///< Slot is open.
            SelfSlot,                 ///< User is playing this slot as primary or replacement.
            OccupiedSlot,             ///< Slot is played by someone else.
            DeadSlot                  ///< Slot is dead.
        };

        /** Game information. */
        struct Info {
            int32_t gameId;                                                ///< Game Id.
            State state;                                                   ///< Game status.
            Type type;                                                     ///< Game type.
            String_t name;                                                 ///< Game name.
            afl::base::Optional<String_t> description;                     ///< Game description (subtitle).
            int32_t difficulty;                                            ///< Game difficulty.
            afl::base::Optional<HostSchedule::Schedule> currentSchedule;   ///< Currently-active schedule.
            afl::base::Optional<std::vector<SlotState> > slotStates;       ///< States of all slots.
            afl::base::Optional<std::vector<int32_t> > turnStates;         ///< States of all turns for all slots.
            afl::base::Optional<bool> joinable;                            ///< true if player can join an OpenSlot.
            afl::base::Optional<bool> userPlays;                           ///< true if player is active on this game.
            afl::base::Optional<std::vector<int32_t> > scores;             ///< Scores for all slots.
            afl::base::Optional<String_t> scoreName;                       ///< Name of score given in @c scores.
            afl::base::Optional<String_t> scoreDescription;                ///< Description (subtitle) of score given in @c scores.
            afl::base::Optional<int32_t> minRankLevelToJoin;               ///< Minimum rank level (rank) to join.
            afl::base::Optional<int32_t> maxRankLevelToJoin;               ///< Maximum rank level (rank) to join.
            afl::base::Optional<int32_t> minRankPointsToJoin;              ///< Minimum rank points (skill) to join.
            afl::base::Optional<int32_t> maxRankPointsToJoin;              ///< Maximum rank points (skill) to join.
            String_t hostName;                                             ///< Machine-readable name of host program.
            String_t hostDescription;                                      ///< Human-readable description of host program.
            String_t hostKind;                                             ///< Machine-readable kind of host program.
            String_t shipListName;                                         ///< Machine-readable name of ship list.
            String_t shipListDescription;                                  ///< Human-readable description of ship list.
            String_t shipListKind;                                         ///< Machine-readable kind of ship list.
            afl::base::Optional<String_t> masterName;                      ///< Machine-readable name of master program.
            afl::base::Optional<String_t> masterDescription;               ///< Human-readable description of master program.
            afl::base::Optional<String_t> masterKind;                      ///< Machine-readable kind of master program.
            int32_t turnNumber;                                            ///< Current turn number. 0 for game that is still joining.
            afl::base::Optional<Time_t> lastHostTime;                      ///< Time of last host.
            afl::base::Optional<Time_t> nextHostTime;                      ///< Estimated time of next host.
            afl::base::Optional<int32_t> forumId;                          ///< Forum Id if nonzero.
            afl::base::Optional<int32_t> userRank;                         ///< Rank of current user in this game.
            afl::base::Optional<int32_t> otherRank;                        ///< Rank of other user in this game. See Filter::requiredUser.

            Info();
            ~Info();
        };

        /** Game count summary. */
        struct Totals {
            int32_t numJoiningGames;
            int32_t numRunningGames;
            int32_t numFinishedGames;

            Totals()
                : numJoiningGames(0), numRunningGames(0), numFinishedGames(0)
                { }
            Totals(int32_t numJoiningGames, int numRunningGames, int numFinishedGames)
                : numJoiningGames(numJoiningGames), numRunningGames(numRunningGames), numFinishedGames(numFinishedGames)
                { }
        };

        /** Victory condition. */
        struct VictoryCondition {
            String_t endCondition;                                         ///< Type of ending condition.
            afl::base::Optional<int32_t> endTurn;                          ///< For "turn" condition, ending turn; for "score" condition, number of turns to hold a score.
            afl::base::Optional<int32_t> endProbability;                   ///< For "turn" condition, probability that game ends at that turn.
            afl::base::Optional<int32_t> endScore;                         ///< For "score" condition: score to reach and hold
            afl::base::Optional<String_t> endScoreName;                    ///< For "score" condition: machine-readable name of score.
            afl::base::Optional<String_t> endScoreDescription;             ///< For "score" condition: human-readable description of score
            afl::base::Optional<String_t> referee;                         ///< For no condition: machine-readable name of referee add-on.
            afl::base::Optional<String_t> refereeDescription;              ///< For no condition: human-readable description of referee add-on.

            VictoryCondition();
            ~VictoryCondition();
        };

        /** Filter for list of games. */
        struct Filter {
            afl::base::Optional<State> requiredState;                      ///< Return only games with this state.
            afl::base::Optional<Type> requiredType;                        ///< Return only games with this type.
            afl::base::Optional<String_t> requiredUser;                    ///< Return only games involving this user.

            afl::base::Optional<String_t> requiredHost;                    ///< Return only games running this host.
            afl::base::Optional<String_t> requiredTool;                    ///< Return only games running this tool.
            afl::base::Optional<String_t> requiredShipList;                ///< Return only games using this ship list.
            afl::base::Optional<String_t> requiredMaster;                  ///< Return only games created using this master.
            afl::base::Optional<int32_t> requiredCopyOf;                   ///< Return only games copied from this game.

            Filter()
                : requiredState(), requiredType(), requiredUser(), requiredHost(), requiredTool(), requiredShipList(), requiredMaster(), requiredCopyOf()
                { }
        };

        /** Create new game (NEWGAME).
            @return new game Id */
        virtual int32_t createNewGame() = 0;

        /** Clone a game (CLONEGAME).
            @param gameId   Original game Id
            @param newState Target game state
            @return new game Id */
        virtual int32_t cloneGame(int32_t gameId, afl::base::Optional<State> newState) = 0;

        /** Set game type (GAMESETTYPE).
            @param gameId   Game Id
            @param type     New type */
        virtual void setType(int32_t gameId, Type type) = 0;

        /** Set game state (GAMESETSTATE).
            @param gameId   Game Id
            @param state    New state */
        virtual void setState(int32_t gameId, State state) = 0;

        /** Set game owner (GAMESETOWNER).
            @param gameId   Game Id
            @param user     User Id */
        virtual void setOwner(int32_t gameId, String_t user) = 0;

        /** Set game name (GAMESETNAME).
            @param gameId   Game Id
            @param name     New name */
        virtual void setName(int32_t gameId, String_t name) = 0;

        /** Get information about one game (GAMESTAT).
            @param gameId   Game Id
            @return verbose information */
        virtual Info getInfo(int32_t gameId) = 0;

        /** Get information about a list of games (GAMELIST).
            @param [in]  filter  Filter
            @param [in]  verbose true to request verbose output; default is abbreviated
            @param [out] result  Result */
        virtual void getInfos(const Filter& filter, bool verbose, std::vector<Info>& result) = 0;

        /** Get list of games (GAMELIST ID).
            @param [in]  filter  Filter
            @param [out] result  List of game Ids */
        virtual void getGames(const Filter& filter, afl::data::IntegerList_t& result) = 0;

        /** Set game properties (GAMESET).
            @param gameId     Game Id
            @param keyValues  List of key/value pairs */
        virtual void setConfig(int32_t gameId, const afl::data::StringList_t& keyValues) = 0;

        /** Get game property (GAMEGET).
            @param gameId   Game Id
            @param key      Property name
            @return property value */
        virtual String_t getConfig(int32_t gameId, String_t key) = 0;

        /** Get multiple game properties (GAMEMGET).
            @param [in]  gameId   Game Id
            @param [in]  keys     Property names
            @param [out] values   Property values */
        // FIXME: do we need this guy? It is not used anywhere.
        virtual void getConfig(int32_t gameId, const afl::data::StringList_t& keys, afl::data::StringList_t& values) = 0;

        /** Get computed/caches value (GAMEGETCC).
            @param gameId   Game Id
            @param key      Cache-value name
            @return value */
        virtual String_t getComputedValue(int32_t gameId, String_t key) = 0;

        /** Get game state (GAMEGETSTATE).
            @param gameId   Game Id
            @return state */
        virtual State getState(int32_t gameId) = 0;

        /** Get game type (GAMEGETTYPE).
            @param gameId   Game Id
            @return type */
        virtual Type getType(int32_t gameId) = 0;

        /** Get game owner (GAMEGETOWNER).
            @param gameId   Game Id
            @return user Id */
        virtual String_t getOwner(int32_t gameId) = 0;

        /** Get game name (GAMEGETNAME).
            @param gameId   Game Id
            @return name */
        virtual String_t getName(int32_t gameId) = 0;

        /** Get game directory name in host filer (GAMEGETDIR).
            @param gameId   Game Id
            @return directory name */
        virtual String_t getDirectory(int32_t gameId) = 0;

        /** Get game permissions (GAMECHECKPERM).
            @param gameId   Game Id
            @param userId   User Id
            @return set of permissions */
        // FIXME: do we need this guy? It is not used anywhere.
        virtual Permissions_t getPermissions(int32_t gameId, String_t userId) = 0;

        /** Add a tool to a game (GAMEADDTOOL).
            @param gameId   Game Id
            @param toolId   Tool name
            @return true if tool was added, false if tool was already present */
        virtual bool addTool(int32_t gameId, String_t toolId) = 0;

        /** Remove a tool from a game (GAMERMTOOL).
            @param gameId   Game Id
            @param toolId   Tool name
            @return true if tool was removed, false if tool was not present */
        virtual bool removeTool(int32_t gameId, String_t toolId) = 0;

        /** List tools used on a game (GAMELSTOOLS).
            @param [in]  gameId  Game Id
            @param [out] result  Tools and descriptions */
        virtual void getTools(int32_t gameId, std::vector<HostTool::Info>& result) = 0;

        /** Get host statistics (GAMETOTALS).
            @return totals */
        virtual Totals getTotals() = 0;

        /** Get victory condition (GAMEGETVC).
            @param gameId   Game Id
            @return victory condition */
        virtual VictoryCondition getVictoryCondition(int32_t gameId) = 0;

        /** Update game to latest specs (GAMEUPDATE).
            @param gameIds  List of game Ids */
        // FIXME: do we need this guy? Better auto-upgrade games on startup.
        virtual void updateGames(const afl::data::IntegerList_t& gameIds) = 0;

        /** Reset game to specified turn (GAMERESET).
            @param gameId  Game Id
            @param turnNr  Turn number */
        virtual void resetToTurn(int32_t gameId, int turnNr) = 0;


        /** Format a State into a string.
            @param state State
            @return string representation */
        static String_t formatState(State state);

        /** Parse a string into a State.
            @param [in]  str     String
            @param [out] result  Result
            @return true on success, false if string does not match a State value. */
        static bool parseState(const String_t& str, State& result);

        /** Format a Type into a string.
            @param type Type
            @return string representation */
        static String_t formatType(Type type);

        /** Parse a string into a Type.
            @param [in]  str     String
            @param [out] result  Result
            @return true on success, false if string does not match a Type value. */
        static bool parseType(const String_t& str, Type& result);

        /** Format a SlotState into a string.
            @param state State
            @return string representation */
        static String_t formatSlotState(SlotState state);

        /** Parse a string into a SlotState.
            @param [in]  str     String
            @param [out] result  Result
            @return true on success, false if string does not match a SlotState value. */
        static bool parseSlotState(const String_t& str, SlotState& result);
    };

} }

#endif
