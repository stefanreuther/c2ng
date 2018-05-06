/**
  *  \file client/vcr/classic/eventconsumer.hpp
  */
#ifndef C2NG_CLIENT_VCR_CLASSIC_EVENTCONSUMER_HPP
#define C2NG_CLIENT_VCR_CLASSIC_EVENTCONSUMER_HPP

#include "game/vcr/classic/eventlistener.hpp"
#include "game/vcr/classic/types.hpp"
#include "client/vcr/classic/event.hpp"

namespace client { namespace vcr { namespace classic {

    class EventConsumer {
     public:
        virtual ~EventConsumer()
            { }

        virtual void placeObject(game::vcr::classic::Side side, const game::vcr::classic::EventListener::UnitInfo& info) = 0;

        virtual void pushEvent(Event e) = 0;

        virtual void removeAnimations(int32_t id) = 0;
    };

} } }

#endif
