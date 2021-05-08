/**
  *  \file game/map/objectcursor.hpp
  *  \brief Class game::map::ObjectCursor
  */
#ifndef C2NG_GAME_MAP_OBJECTCURSOR_HPP
#define C2NG_GAME_MAP_OBJECTCURSOR_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/signal.hpp"
#include "game/types.hpp"

namespace game { namespace map {

    class Object;
    class ObjectType;

    /** Object selection, base class.
        This provides functions to manage selection of an single object from a set (ObjectType),
        used to denote the "current" object in a context.

        The object is identified by an index that is the same as the index of the underlying ObjectType,
        most of the time, the Id of the object.

        ex GObjectSelection */
    class ObjectCursor : public afl::base::Deletable {
     public:
        /** Browsing mode. */
        enum Mode {
            Next,               ///< Pick next object (larger index).
            Previous,           ///< Pick previous object (smaller index).
            First,              ///< Pick first object (lowest index).
            Last,               ///< Pick last object (highest index).
            NextHere,           ///< Pick next object here (larger index).
            PreviousHere        ///< Pick previous object here (smaller index).
        };

        /** Get underlying object type.
            This is used to find out possible indexes to select.
            \return object type, can be null. */
        virtual ObjectType* getObjectType() const = 0;

        /** Set current index.
            \param index New index */
        virtual void setCurrentIndex(Id_t index) = 0;

        /** Get currently-selected index.
            \return index */
        virtual Id_t getCurrentIndex() const = 0;

        /** Get current object.
            \return object, null if none selected */
        Object* getCurrentObject() const;

        /** Browse.
            Picks a new index according to the parameters, and selects it using setCurrentIndex().
            \param mode Browsing mode
            \param marked true to accept only marked objects */
        void browse(Mode mode, bool marked);

        /** Signal: index change.
            Called whenever the index (getCurrentIndex()) changes. */
        // ex sig_current_changed
        afl::base::Signal<void()> sig_indexChange;
    };

} }

#endif
