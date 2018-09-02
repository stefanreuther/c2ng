/**
  *  \file game/ref/listobserver.hpp
  */
#ifndef C2NG_GAME_REF_LISTOBSERVER_HPP
#define C2NG_GAME_REF_LISTOBSERVER_HPP

#include "game/ref/list.hpp"
#include "game/ref/userlist.hpp"
#include "game/session.hpp"
#include "afl/base/signal.hpp"

namespace game { namespace ref {

    class Configuration;

    class ListObserver {
     public:
        ListObserver();

        void setList(const List& list);
        void setExtra(const UserList& list);
        void setSession(Session& session);

        Configuration getConfig() const;
        void setConfig(const Configuration& config);

        const UserList& getList();

        afl::base::Signal<void()> sig_listChange;

     private:
        List m_mainList;
        UserList m_extraList;
        UserList m_resultList;

        Session* m_pSession;

        afl::base::SignalConnection conn_viewpointTurnChange;
        afl::base::SignalConnection conn_universeChange;

        void updateResultList();
        void onViewpointTurnChange();
        void onUniverseChange();
    };

} }

#endif
