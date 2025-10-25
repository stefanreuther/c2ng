/**
  *  \file test/game/vcr/classic/scheduledeventtest.cpp
  *  \brief Test for game::vcr::classic::ScheduledEvent
  */

#include "game/vcr/classic/scheduledevent.hpp"

#include "afl/test/testrunner.hpp"

using game::vcr::classic::RightSide;
using game::vcr::classic::ScheduledEvent;

AFL_TEST("game.vcr.classic.ScheduledEvent", a)
{
    // Constructor signatures
    a.checkEqual("01", ScheduledEvent(ScheduledEvent::UpdateTime, RightSide).type,             ScheduledEvent::UpdateTime);
    a.checkEqual("02", ScheduledEvent(ScheduledEvent::UpdateTime, RightSide).side,             RightSide);
    a.checkEqual("03", ScheduledEvent(ScheduledEvent::UpdateTime, RightSide, 1).a,             1);
    a.checkEqual("04", ScheduledEvent(ScheduledEvent::UpdateTime, RightSide, 1, 2).b,          2);
    a.checkEqual("05", ScheduledEvent(ScheduledEvent::UpdateTime, RightSide, 1, 2, 3).c,       3);
    a.checkEqual("06", ScheduledEvent(ScheduledEvent::UpdateTime, RightSide, 1, 2, 3, 4).d,    4);
    a.checkEqual("07", ScheduledEvent(ScheduledEvent::UpdateTime, RightSide, 1, 2, 3, 4, 5).e, 5);

    // toString
    a.checkEqual("11", ScheduledEvent::toString(ScheduledEvent::UpdateTime), String_t("UpdateTime"));
    a.checkEqual("12", ScheduledEvent::toString(ScheduledEvent::WaitTick),   String_t("WaitTick"));
}
