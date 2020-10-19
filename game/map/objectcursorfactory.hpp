/**
  *  \file game/map/objectcursorfactory.hpp
  *  \brief Interface game::map::ObjectCursorFactory
  */
#ifndef C2NG_GAME_MAP_OBJECTCURSORFACTORY_HPP
#define C2NG_GAME_MAP_OBJECTCURSORFACTORY_HPP

#include "afl/base/deletable.hpp"
#include "game/session.hpp"

namespace game { namespace map {

    class ObjectCursor;

    /** Factory for ObjectCursor objects.
        Provides access to an ObjectCursor instance. */
    class ObjectCursorFactory : public afl::base::Deletable {
     public:
        /** Get cursor.
            \param session Session
            \return ObjectCursor instance. Caller will NOT take ownership. Can be null. */
        virtual ObjectCursor* getCursor(Session& session) = 0;
    };

} }

#endif
