/**
  *  \file server/host/hostgame.hpp
  *  \brief Class server::host::HostGame
  */
#ifndef C2NG_SERVER_HOST_HOSTGAME_HPP
#define C2NG_SERVER_HOST_HOSTGAME_HPP

#include "server/interface/hostgame.hpp"

namespace server { namespace host {

    class Root;
    class Session;

    /** Implementation of HostGame interface.
        This interface implements GAME commands. */
    class HostGame : public server::interface::HostGame {
     public:
        /** Constructor.
            \param session Session
            \param root    Service root */
        HostGame(Session& session, Root& root);

        virtual int32_t createNewGame();
        virtual int32_t cloneGame(int32_t gameId, afl::base::Optional<State> newState);
        virtual void setType(int32_t gameId, Type type);
        virtual void setState(int32_t gameId, State state);
        virtual void setOwner(int32_t gameId, String_t user);
        virtual void setName(int32_t gameId, String_t name);
        virtual Info getInfo(int32_t gameId);
        virtual void getInfos(const Filter& filter, bool verbose, std::vector<Info>& result);
        virtual void getGames(const Filter& filter, afl::data::IntegerList_t& result);
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
        virtual void getTools(int32_t gameId, std::vector<server::interface::HostTool::Info>& result);
        virtual Totals getTotals();
        virtual VictoryCondition getVictoryCondition(int32_t gameId);
        virtual void updateGames(const afl::data::IntegerList_t& gameIds);

     private:
        Session& m_session;
        Root& m_root;

        void listGames(const Filter& filter, afl::data::IntegerList_t& result);

        bool addRemoveTool(int32_t gameId, String_t toolId, bool add);
    };

} }

#endif
