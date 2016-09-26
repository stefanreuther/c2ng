/**
  *  \file client/objectobserver.hpp
  */
#ifndef C2NG_CLIENT_OBJECTOBSERVER_HPP
#define C2NG_CLIENT_OBJECTOBSERVER_HPP

#include <memory>
#include "util/slaveobject.hpp"
#include "game/session.hpp"
#include "afl/base/signalconnection.hpp"
#include "afl/container/ptrvector.hpp"
#include "game/map/objectobserver.hpp"
#include "client/objectcursorfactory.hpp"

namespace client {

    class ObjectListener;

    class ObjectObserver : public util::SlaveObject<game::Session> {
     public:
        ObjectObserver(std::auto_ptr<ObjectCursorFactory> f);
        ~ObjectObserver();

        virtual void init(game::Session& s);
        virtual void done(game::Session& s);

        void addNewListener(game::Session& s, ObjectListener* pl);

     private:
        void onObjectChange();

        std::auto_ptr<ObjectCursorFactory> m_factory;
        std::auto_ptr<game::map::ObjectObserver> m_observer;
        game::Session* m_pSession;
        afl::base::SignalConnection conn_objectChange;
        afl::container::PtrVector<ObjectListener> m_listeners;
    };

}

#endif
