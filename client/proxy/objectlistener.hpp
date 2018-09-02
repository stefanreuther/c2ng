/**
  *  \file client/proxy/objectlistener.hpp
  */
#ifndef C2NG_CLIENT_PROXY_OBJECTLISTENER_HPP
#define C2NG_CLIENT_PROXY_OBJECTLISTENER_HPP

#include "afl/base/deletable.hpp"
#include "game/map/object.hpp"
#include "game/session.hpp"

namespace client { namespace proxy {

    class ObjectListener : public afl::base::Deletable {
     public:
        virtual void handle(game::Session& s, game::map::Object* obj) = 0;
    };

} }

#endif
