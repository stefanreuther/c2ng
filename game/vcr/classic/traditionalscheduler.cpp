/**
  *  \file game/vcr/classic/traditionalscheduler.cpp
  *  \brief Class game::vcr::classic::TraditionalScheduler
  */

#include "game/vcr/classic/traditionalscheduler.hpp"
#include "game/vcr/classic/scheduledeventconsumer.hpp"

namespace {
    /* All animations are created with this ID, and immediately waited-upon. */
    const int32_t ANIMATION_ID = 99;
}

game::vcr::classic::TraditionalScheduler::TraditionalScheduler(ScheduledEventConsumer& parent)
    : m_consumer(parent)
{ }

void
game::vcr::classic::TraditionalScheduler::placeObject(Side side, const UnitInfo& info)
{
    m_consumer.placeObject(side, info);
}

void
game::vcr::classic::TraditionalScheduler::updateTime(Time_t time, int32_t distance)
{
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::UpdateTime, LeftSide, time));
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::UpdateDistance, LeftSide, distance));
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::WaitTick, LeftSide));
}

void
game::vcr::classic::TraditionalScheduler::startFighter(Side side, int track, int position, int distance, int fighterDiff)
{
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::StartFighter, side, track, position, distance));
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::UpdateNumFighters, side, fighterDiff));
}

void
game::vcr::classic::TraditionalScheduler::landFighter(Side side, int track, int fighterDiff)
{
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::RemoveFighter, side, track));
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::UpdateNumFighters, side, fighterDiff));
}

void
game::vcr::classic::TraditionalScheduler::killFighter(Side side, int track)
{
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::ExplodeFighter, side, track, ANIMATION_ID));
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::RemoveFighter, side, track));
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::WaitAnimation, side, ANIMATION_ID));
}

void
game::vcr::classic::TraditionalScheduler::fireBeam(Side side, int track, int target, int hit, int /*damage*/, int /*kill*/, const HitEffect& effect)
{
    if (track < 0) {
        int beamSlot = -1-track;
        if (target < 0) {
            // Ship/Ship
            m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::FireBeamShipShip, side, beamSlot, ANIMATION_ID));
            m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::WaitAnimation, side, ANIMATION_ID));
            if (hit >= 0) {
                renderHit(flipSide(side), effect);
            }
        } else {
            // Ship/Fighter
            m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::FireBeamShipFighter, side, target, beamSlot, ANIMATION_ID));
            m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::WaitAnimation, side, ANIMATION_ID));
        }
    } else {
        if (target < 0) {
            // Fighter/Ship
            m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::FireBeamFighterShip, side, track, ANIMATION_ID));
            m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::WaitAnimation, side, ANIMATION_ID));
            if (hit >= 0) {
                renderHit(flipSide(side), effect);
            }
        } else {
            // Fighter/Fighter
            m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::FireBeamFighterFighter, side, track, target, ANIMATION_ID));
            m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::WaitAnimation, side, ANIMATION_ID));
        }
    }
}

void
game::vcr::classic::TraditionalScheduler::fireTorpedo(Side side, int hit, int launcher, int torpedoDiff, const HitEffect& effect)
{
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::FireTorpedo, side, launcher, hit, ANIMATION_ID, 6 /* FIXME: compute from distance! */));
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::UpdateNumTorpedoes, side, torpedoDiff));
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::WaitAnimation, side, ANIMATION_ID));
    renderHit(flipSide(side), effect);
}

void
game::vcr::classic::TraditionalScheduler::updateBeam(Side side, int id, int value)
{
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::UpdateBeam, side, id, value));
}

void
game::vcr::classic::TraditionalScheduler::updateLauncher(Side side, int id, int value)
{
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::UpdateLauncher, side, id, value));
}

void
game::vcr::classic::TraditionalScheduler::moveObject(Side side, int position)
{
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::MoveObject, side, position));
}

void
game::vcr::classic::TraditionalScheduler::moveFighter(Side side, int track, int position, int distance, FighterStatus status)
{
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::MoveFighter, side, track, position, distance, status));
}

void
game::vcr::classic::TraditionalScheduler::killObject(Side /*side*/)
{
    // FIXME: implement
}

void
game::vcr::classic::TraditionalScheduler::updateObject(Side side, int damage, int crew, int shield)
{
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::UpdateObject, side, damage, crew, shield));
}

void
game::vcr::classic::TraditionalScheduler::updateAmmo(Side side, int numTorpedoes, int numFighters)
{
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::UpdateAmmo, side, numTorpedoes, numFighters));
}

void
game::vcr::classic::TraditionalScheduler::updateFighter(Side side, int track, int position, int distance, FighterStatus status)
{
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::UpdateFighter, side, track, position, distance, status));
}

void
game::vcr::classic::TraditionalScheduler::setResult(BattleResult_t result)
{
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::SetResult, LeftSide, result.toInteger()));
}

void
game::vcr::classic::TraditionalScheduler::removeAnimations()
{
    m_consumer.removeAnimations(ANIMATION_ID, ANIMATION_ID);
}

void
game::vcr::classic::TraditionalScheduler::renderHit(Side side, const HitEffect& effect)
{
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::HitObject, side, effect.damageDone, effect.crewKilled, effect.shieldLost, ANIMATION_ID));
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::WaitAnimation, side, ANIMATION_ID));
}
