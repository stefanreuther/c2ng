/**
  *  \file client/objectcursorfactory.hpp
  */
#ifndef C2NG_CLIENT_OBJECTCURSORFACTORY_HPP
#define C2NG_CLIENT_OBJECTCURSORFACTORY_HPP

#include "afl/base/deletable.hpp"
#include "game/map/objectcursor.hpp"
#include "game/session.hpp"

namespace client {

    class ObjectCursorFactory : public afl::base::Deletable {
     public:
        virtual game::map::ObjectCursor* getCursor(game::Session& session) = 0;
    };

}

#endif
