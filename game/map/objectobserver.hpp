/**
  *  \file game/map/objectobserver.hpp
  */
#ifndef C2NG_GAME_MAP_OBJECTOBSERVER_HPP
#define C2NG_GAME_MAP_OBJECTOBSERVER_HPP

#include "afl/base/signal.hpp"
#include "afl/base/signalconnection.hpp"

namespace game { namespace map {

    class Object;
    class ObjectCursor;
    class ObjectType;
    class Universe;

    // /** Observe a GObjectSelection.
    // This class observes a GObjectSelection.
    // It provides a virtual onObjectChange() method which is called
    // whenever the object selected into the GObjectSelection changes.
    // It also provides methods to access that object. */
    class ObjectObserver {
     public:
        ObjectObserver(ObjectCursor& cursor);
        ~ObjectObserver();

        Object* getCurrentObject();

        ObjectCursor& cursor();
        ObjectType* getObjectType();

        afl::base::Signal<void()> sig_objectChange;

     private:
        ObjectCursor& m_cursor;
        afl::base::SignalConnection conn_cursorChange;
        afl::base::SignalConnection conn_objectChange;

        void onCurrentChange();
        void attachObject();
        void attachCursor();
    };

} }

#endif
