/**
  *  \file game/map/simpleobjectcursor.hpp
  *  \brief Class game::map::SimpleObjectCursor
  */
#ifndef C2NG_GAME_MAP_SIMPLEOBJECTCURSOR_HPP
#define C2NG_GAME_MAP_SIMPLEOBJECTCURSOR_HPP

#include "afl/base/signalconnection.hpp"
#include "game/map/objectcursor.hpp"

namespace game { namespace map {

    /** Object selection, simple version.
        This implements the simple way of selecting an object, by tracking a current index.
        This is the sufficient solution for most places.

        Structural changes are handled by trying to switch to the object given as a hint in the sig_setChange event,
        which is the right thing for fleet renamings.
        For other objects that go away there probably is no unique "right" way.

        This object can be told to point at any ObjectType.
        When not pointing at one, it correctly reports no object selected. */
    class SimpleObjectCursor : public ObjectCursor {
     public:
        /** Default constructor.
            Starts with no underlying ObjectType. */
        SimpleObjectCursor();

        /** Copy constructor.
            Copies the ObjectType and selected index from the other cursor.
            @param other Cursor to copy from */
        SimpleObjectCursor(const ObjectCursor& other);

        /** Destructor. */
        ~SimpleObjectCursor();

        // ObjectCursor:
        virtual ObjectType* getObjectType() const;
        virtual void setCurrentIndex(Id_t index);
        virtual Id_t getCurrentIndex() const;

        /** Set underlying object type.
            @param type Type; must live longer than the SimpleObjectCursor; can be null */
        void setObjectType(ObjectType* type);

     private:
        int m_currentIndex;
        ObjectType* m_objectType;

        afl::base::SignalConnection conn_setChange;
        void onSetChange(Id_t hint);
    };

} }

#endif
