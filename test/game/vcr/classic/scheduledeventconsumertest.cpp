/**
  *  \file test/game/vcr/classic/scheduledeventconsumertest.cpp
  *  \brief Test for game::vcr::classic::ScheduledEventConsumer
  */

#include "game/vcr/classic/scheduledeventconsumer.hpp"

#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.vcr.classic.ScheduledEventConsumer")
{
    class Tester : public game::vcr::classic::ScheduledEventConsumer {
     public:
        virtual void placeObject(game::vcr::classic::Side /*side*/, const game::vcr::classic::EventListener::UnitInfo& /*info*/)
            { }
        virtual void pushEvent(game::vcr::classic::ScheduledEvent /*e*/)
            { }
        virtual void removeAnimations(int32_t /*from*/, int32_t /*to*/)
            { }
    };
    Tester t;
}
