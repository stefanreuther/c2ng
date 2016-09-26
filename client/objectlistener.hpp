/**
  *  \file client/objectlistener.hpp
  */
#ifndef C2NG_CLIENT_OBJECTLISTENER_HPP
#define C2NG_CLIENT_OBJECTLISTENER_HPP

#include "afl/base/deletable.hpp"
#include "game/session.hpp"
#include "game/map/object.hpp"

namespace client {

    class ObjectListener : public afl::base::Deletable {
     public:
        virtual void handle(game::Session& s, game::map::Object* obj) = 0;
    };
}

#endif
