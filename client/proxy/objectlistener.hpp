/**
  *  \file client/proxy/objectlistener.hpp
  *  \brief Interface client::proxy::ObjectListener
  */
#ifndef C2NG_CLIENT_PROXY_OBJECTLISTENER_HPP
#define C2NG_CLIENT_PROXY_OBJECTLISTENER_HPP

#include "afl/base/deletable.hpp"
#include "game/map/object.hpp"
#include "game/session.hpp"

namespace client { namespace proxy {

    /** Interface for a listener to a map object.
        The ObjectListener lives in the game thread.
        It needs to generate signalisations to the UI thread as needed. */
    class ObjectListener : public afl::base::Deletable {
     public:
        /** Handle object change.
            Called when either the object changes, or a different (or no) object is selected.
            \param s Session
            \param obj Object (can be null) */
        virtual void handle(game::Session& s, game::map::Object* obj) = 0;
    };

} }

#endif
