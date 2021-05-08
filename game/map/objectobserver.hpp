/**
  *  \file game/map/objectobserver.hpp
  *  \brief Class game::map::ObjectObserver
  */
#ifndef C2NG_GAME_MAP_OBJECTOBSERVER_HPP
#define C2NG_GAME_MAP_OBJECTOBSERVER_HPP

#include "afl/base/signal.hpp"
#include "afl/base/signalconnection.hpp"

namespace game { namespace map {

    class Object;
    class ObjectCursor;
    class ObjectType;

    /** Observe an ObjectCursor.
        Provides a sig_objectChange event which is called whenever the object selected into the ObjectCursor changes,
        either by a change on the object, or by selecting a different object.
        It also provides methods to access that object. */
    class ObjectObserver {
     public:
        explicit ObjectObserver(ObjectCursor& cursor);
        ~ObjectObserver();

        /** Get current object.
            \return pointer to object, can be null
            \see ObjectCursor::getCurrentObject() */
        Object* getCurrentObject() const;

        /** Get cursor.
            \return reference to the ObjectCursor this object was constructed with. */
        ObjectCursor& cursor() const;

        /** Get object type observed by cursor.
            \return ObjectType; can be null
            \see ObjectCursor::getObjectType() */
        ObjectType* getObjectType() const;

        /** Signal: object change.
            Called when the object changes, or a different object is selected.
            \see Object::sig_change
            \see ObjectCursor::sig_indexChange */
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
