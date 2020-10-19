/**
  *  \file server/interface/hostgame.hpp
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
#include "server/interface/hosttool.hpp"
#include "server/types.hpp"
#include "server/interface/hostschedule.hpp"

namespace server { namespace interface {

    class HostGame : public afl::base::Deletable {
     public:
        enum State {
            Preparing,
            Joining,
            Running,
            Finished,
            Deleted
        };

        enum Type {
            PrivateGame,
            UnlistedGame,
            PublicGame,
            TestGame
        };

        enum Permission {
            UserIsOwner,
            UserIsPrimary,
            UserIsActive,
            UserIsInactive,
            GameIsPublic
        };
        typedef afl::bits::SmallSet<Permission> Permissions_t;

        enum SlotState {
            OpenSlot,
            SelfSlot,
            OccupiedSlot,
            DeadSlot
        };

        struct Info {
            // FIXME: needs revision
            int32_t gameId;
            State state;
            Type type;
            String_t name;
            afl::base::Optional<String_t> description;
            int32_t difficulty;
            afl::base::Optional<HostSchedule::Schedule> currentSchedule;
            afl::base::Optional<std::vector<SlotState> > slotStates;
            afl::base::Optional<std::vector<int32_t> > turnStates;
            afl::base::Optional<bool> joinable;
            afl::base::Optional<std::vector<int32_t> > scores;
            afl::base::Optional<String_t> scoreName;
            afl::base::Optional<String_t> scoreDescription;
            String_t hostName;
            String_t hostDescription;
            String_t shipListName;
            String_t shipListDescription;
            afl::base::Optional<String_t> masterName;
            afl::base::Optional<String_t> masterDescription;
            int32_t turnNumber;
            afl::base::Optional<Time_t> lastHostTime;
            afl::base::Optional<Time_t> nextHostTime;
            afl::base::Optional<int32_t> forumId;

            Info();
            ~Info();
        };

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

        struct VictoryCondition {
            // FIXME: needs revision
            String_t endCondition;
            afl::base::Optional<int32_t> endTurn;
            afl::base::Optional<int32_t> endProbability;
            afl::base::Optional<int32_t> endScore;
            afl::base::Optional<String_t> endScoreName;
            afl::base::Optional<String_t> endScoreDescription;
            afl::base::Optional<String_t> referee;
            afl::base::Optional<String_t> refereeDescription;

            VictoryCondition();
            ~VictoryCondition();
        };

        struct Filter {
            afl::base::Optional<State> requiredState;
            afl::base::Optional<Type> requiredType;
            afl::base::Optional<String_t> requiredUser;

            afl::base::Optional<String_t> requiredHost;
            afl::base::Optional<String_t> requiredTool;
            afl::base::Optional<String_t> requiredShipList;
            afl::base::Optional<String_t> requiredMaster;

            Filter()
                : requiredState(), requiredType(), requiredUser(), requiredHost(), requiredTool(), requiredShipList(), requiredMaster()
                { }
        };

        // NEWGAME
        virtual int32_t createNewGame() = 0;

        // CLONEGAME src:GID [state:HostGameState]
        virtual int32_t cloneGame(int32_t gameId, afl::base::Optional<State> newState) = 0;

        // GAMESETTYPE game:GID type:HostGameType
        virtual void setType(int32_t gameId, Type type) = 0;

        // GAMESETSTATE game:GID type:HostGameState
        virtual void setState(int32_t gameId, State state) = 0;

        // GAMESETOWNER game:GID user:UID
        virtual void setOwner(int32_t gameId, String_t user) = 0;

        // GAMESETNAME game:GID name:Str
        virtual void setName(int32_t gameId, String_t name) = 0;

        // GAMESTAT game:GID
        virtual Info getInfo(int32_t gameId) = 0;

        // GAMELIST [STATE state:HostGameState] [TYPE type:HostGameType] [USER user:UID] [VERBOSE|ID]
        virtual void getInfos(const Filter& filter, bool verbose, std::vector<Info>& result) = 0;
        virtual void getGames(const Filter& filter, afl::data::IntegerList_t& result) = 0;

        // GAMESET game:GID [key:Str value:Str ...]
        virtual void setConfig(int32_t gameId, const afl::data::StringList_t& keyValues) = 0;

        // GAMEGET game:GID key:Str
        virtual String_t getConfig(int32_t gameId, String_t key) = 0;

        // GAMEMGET game:GID key:Str...
        // FIXME: do we need this guy? It is not used anywhere.
        virtual void getConfig(int32_t gameId, const afl::data::StringList_t& keys, afl::data::StringList_t& values) = 0;

        // GAMEGETCC game:GID key:Str
        virtual String_t getComputedValue(int32_t gameId, String_t key) = 0;

        // GAMEGETSTATE game:GID
        virtual State getState(int32_t gameId) = 0;

        // GAMEGETTYPE game:GID
        virtual Type getType(int32_t gameId) = 0;

        // GAMEGETOWNER game:GID
        virtual String_t getOwner(int32_t gameId) = 0;

        // GAMEGETNAME game:GID
        virtual String_t getName(int32_t gameId) = 0;

        // GAMEGETDIR game:GID
        virtual String_t getDirectory(int32_t gameId) = 0;

        // GAMECHECKPERM game:GID user:UID
        // FIXME: do we need this guy? It is not used anywhere.
        virtual Permissions_t getPermissions(int32_t gameId, String_t userId) = 0;

        // GAMEADDTOOL game:GID tool:Str
        virtual bool addTool(int32_t gameId, String_t toolId) = 0;

        // GAMERMTOOL game:GID tool:Str
        virtual bool removeTool(int32_t gameId, String_t toolId) = 0;

        // GAMELSTOOLS game:GID
        virtual void getTools(int32_t gameId, std::vector<HostTool::Info>& result) = 0;

        // GAMETOTALS
        virtual Totals getTotals() = 0;

        // GAMEGETVC game:GID
        virtual VictoryCondition getVictoryCondition(int32_t gameId) = 0;

        // GAMEUPDATE game:GID...
        // FIXME: do we need this guy? Better auto-upgrade games on startup.
        virtual void updateGames(const afl::data::IntegerList_t& gameIds) = 0;

        static String_t formatState(State state);
        static bool parseState(const String_t& str, State& result);
        static String_t formatType(Type type);
        static bool parseType(const String_t& str, Type& result);
        static String_t formatSlotState(SlotState state);
        static bool parseSlotState(const String_t& str, SlotState& result);
    };

} }

#endif
