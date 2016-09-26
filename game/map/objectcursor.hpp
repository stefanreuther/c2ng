/**
  *  \file game/map/objectcursor.hpp
  */
#ifndef C2NG_GAME_MAP_OBJECTCURSOR_HPP
#define C2NG_GAME_MAP_OBJECTCURSOR_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/signal.hpp"
#include "game/types.hpp"

namespace game { namespace map {

    class Object;
    class ObjectType;
    class Universe;

    /** Object selection, base class.
        This provides functions to manage selection of an single object from a set (ObjectType),
        used to denote the "current" object in a context.

        ex GObjectSelection */
    class ObjectCursor : public afl::base::Deletable {
     public:
        /** Get underlying object type.
            This is used to find out possible indexes to select.
            \return object type, can be null. */
        virtual ObjectType* getObjectType() const = 0;

        /** Set current index. */
        virtual void setCurrentIndex(Id_t index) = 0;

        /** Get currently-selected index. */
        virtual Id_t getCurrentIndex() const = 0;

        Object* getCurrentObject() const;
        Universe* getCurrentUniverse() const;

        // ex sig_current_changed
        afl::base::Signal<void()> sig_indexChange;
    };

} }

#endif
