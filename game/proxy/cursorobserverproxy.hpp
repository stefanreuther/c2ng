/**
  *  \file game/proxy/cursorobserverproxy.hpp
  *  \brief Class game::proxy::CursorObserverProxy
  */
#ifndef C2NG_GAME_PROXY_CURSOROBSERVERPROXY_HPP
#define C2NG_GAME_PROXY_CURSOROBSERVERPROXY_HPP

#include <memory>
#include "game/proxy/objectobserver.hpp"
#include "game/map/objectcursorfactory.hpp"
#include "game/session.hpp"
#include "util/requestreceiver.hpp"

namespace game { namespace proxy {

    class ObjectListener;

    /** Observe an object identified by a cursor. */
    class CursorObserverProxy : public ObjectObserver {
     public:
        /** Constructor.
            \param gameSender Game sender
            \param f Cursor factory. Called in game thread to produce the cursor which refers to the object. */
        CursorObserverProxy(util::RequestSender<Session> gameSender, std::auto_ptr<game::map::ObjectCursorFactory> f);

        /** Destructor. */
        ~CursorObserverProxy();

        // ObjectObserver:
        virtual void addNewListener(ObjectListener* pl);

     private:
        class Trampoline;
        class TrampolineFromSession;

        util::RequestSender<Trampoline> m_trampoline;
    };

} }


#endif
