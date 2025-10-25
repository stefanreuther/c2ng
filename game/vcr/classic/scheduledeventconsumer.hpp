/**
  *  \file game/vcr/classic/scheduledeventconsumer.hpp
  *  \interface game::vcr::classic::ScheduledEventConsumer
  */
#ifndef C2NG_GAME_VCR_CLASSIC_SCHEDULEDEVENTCONSUMER_HPP
#define C2NG_GAME_VCR_CLASSIC_SCHEDULEDEVENTCONSUMER_HPP

#include "game/vcr/classic/eventlistener.hpp"
#include "game/vcr/classic/scheduledevent.hpp"
#include "game/vcr/classic/types.hpp"

namespace game { namespace vcr { namespace classic {

    /** Consumer for ScheduledEvent.
        An event scheduler produces a sequence of ScheduledEvent objects,
        that are played back in sequence to render a fight. */
    class ScheduledEventConsumer {
     public:
        /** Virtual destructor. */
        virtual ~ScheduledEventConsumer()
            { }

        /** Place an object.
            Sets an object's details.
            @param side Side
            @param info Information about the object */
        virtual void placeObject(Side side, const EventListener::UnitInfo& info) = 0;

        /** Process an event.
            @param e Event */
        virtual void pushEvent(ScheduledEvent e) = 0;

        /** Remove animations by Id.
            For use by implementation of EventListener::removeAnimations(),
            to remove animations placed by pushEvent().
            @param from First Id
            @param to Last Id */
        virtual void removeAnimations(int32_t from, int32_t to) = 0;
    };

} } }

#endif
