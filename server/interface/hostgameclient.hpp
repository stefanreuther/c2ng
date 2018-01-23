/**
  *  \file server/interface/hostgameclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTGAMECLIENT_HPP
#define C2NG_SERVER_INTERFACE_HOSTGAMECLIENT_HPP

#include "server/interface/hostgame.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    class HostGameClient : public HostGame {
     public:
        explicit HostGameClient(afl::net::CommandHandler& commandHandler);

        virtual int32_t createNewGame();
        virtual int32_t cloneGame(int32_t gameId, afl::base::Optional<State> newState);
        virtual void setType(int32_t gameId, Type type);
        virtual void setState(int32_t gameId, State state);
        virtual void setOwner(int32_t gameId, String_t user);
        virtual void setName(int32_t gameId, String_t name);
        virtual Info getInfo(int32_t gameId);
        virtual void getInfos(afl::base::Optional<State> requiredState,
                              afl::base::Optional<Type> requiredType,
                              afl::base::Optional<String_t> requiredUser,
                              bool verbose,
                              std::vector<Info>& result);
        virtual void getGames(afl::base::Optional<State> requiredState,
                              afl::base::Optional<Type> requiredType,
                              afl::base::Optional<String_t> requiredUser,
                              afl::data::IntegerList_t& result);
        virtual void setConfig(int32_t gameId, const afl::data::StringList_t& keyValues);
        virtual String_t getConfig(int32_t gameId, String_t key);
        virtual void getConfig(int32_t gameId, const afl::data::StringList_t& keys, afl::data::StringList_t& values);
        virtual String_t getComputedValue(int32_t gameId, String_t key);
        virtual State getState(int32_t gameId);
        virtual Type getType(int32_t gameId);
        virtual String_t getOwner(int32_t gameId);
        virtual String_t getName(int32_t gameId);
        virtual String_t getDirectory(int32_t gameId);
        virtual Permissions_t getPermissions(int32_t gameId, String_t userId);
        virtual bool addTool(int32_t gameId, String_t toolId);
        virtual bool removeTool(int32_t gameId, String_t toolId);
        virtual void getTools(int32_t gameId, std::vector<HostTool::Info>& result);
        virtual Totals getTotals();
        virtual VictoryCondition getVictoryCondition(int32_t gameId);
        virtual void updateGames(const afl::data::IntegerList_t& gameIds);

        static Info unpackInfo(const Value_t* value);

     private:
        afl::net::CommandHandler& m_commandHandler;

        static void buildGameListCommand(afl::data::Segment& cmd,
                                         const afl::base::Optional<State>& requiredState,
                                         const afl::base::Optional<Type>& requiredType,
                                         const afl::base::Optional<String_t>& requiredUser);
    };

} }

#endif
