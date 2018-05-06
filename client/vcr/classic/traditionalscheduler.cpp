/**
  *  \file client/vcr/classic/traditionalscheduler.cpp
  *  \brief Class client::vcr::classic::TraditionalScheduler
  */

#include "client/vcr/classic/traditionalscheduler.hpp"
#include "client/vcr/classic/eventconsumer.hpp"

namespace {
    using game::vcr::classic::LeftSide;
    const int32_t ANIMATION_ID = 99;
}

client::vcr::classic::TraditionalScheduler::TraditionalScheduler(client::vcr::classic::EventConsumer& parent)
    : m_consumer(parent)
{ }

void
client::vcr::classic::TraditionalScheduler::placeObject(Side_t side, const UnitInfo& info)
{
    m_consumer.placeObject(side, info);
}

void
client::vcr::classic::TraditionalScheduler::updateTime(Time_t time, int32_t distance)
{
    m_consumer.pushEvent(Event(Event::UpdateTime, LeftSide, time));
    m_consumer.pushEvent(Event(Event::UpdateDistance, LeftSide, distance));
    m_consumer.pushEvent(Event(Event::WaitTick, LeftSide));
}

void
client::vcr::classic::TraditionalScheduler::startFighter(Side_t side, int track, int position, int distance, int fighterDiff)
{
    m_consumer.pushEvent(Event(Event::StartFighter, side, track, position, distance));
    m_consumer.pushEvent(Event(Event::UpdateNumFighters, side, fighterDiff));
}

void
client::vcr::classic::TraditionalScheduler::landFighter(Side_t side, int track, int fighterDiff)
{
    m_consumer.pushEvent(Event(Event::RemoveFighter, side, track));
    m_consumer.pushEvent(Event(Event::UpdateNumFighters, side, fighterDiff));
}

void
client::vcr::classic::TraditionalScheduler::killFighter(Side_t side, int track)
{
    m_consumer.pushEvent(Event(Event::ExplodeFighter, side, track, ANIMATION_ID));
    m_consumer.pushEvent(Event(Event::RemoveFighter, side, track));
    m_consumer.pushEvent(Event(Event::WaitAnimation, side, ANIMATION_ID));
}

void
client::vcr::classic::TraditionalScheduler::fireBeam(Side_t side, int track, int target, int hit, int /*damage*/, int /*kill*/, const HitEffect& effect)
{
    if (track < 0) {
        int beamSlot = -1-track;
        if (target < 0) {
            // Ship/Ship
            m_consumer.pushEvent(Event(Event::FireBeamShipShip, side, beamSlot, ANIMATION_ID));
            m_consumer.pushEvent(Event(Event::WaitAnimation, side, ANIMATION_ID));
            if (hit >= 0) {
                renderHit(flipSide(side), effect);
            }
        } else {
            // Ship/Fighter
            m_consumer.pushEvent(Event(Event::FireBeamShipFighter, side, target, beamSlot, ANIMATION_ID));
            m_consumer.pushEvent(Event(Event::WaitAnimation, side, ANIMATION_ID));
        }
    } else {
        if (target < 0) {
            // Fighter/Ship
            m_consumer.pushEvent(Event(Event::FireBeamFighterShip, side, track, ANIMATION_ID));
            m_consumer.pushEvent(Event(Event::WaitAnimation, side, ANIMATION_ID));
            if (hit >= 0) {
                renderHit(flipSide(side), effect);
            }
        } else {
            // Fighter/Fighter
            m_consumer.pushEvent(Event(Event::FireBeamFighterFighter, side, track, target, ANIMATION_ID));
            m_consumer.pushEvent(Event(Event::WaitAnimation, side, ANIMATION_ID));
        }
    }
}

void
client::vcr::classic::TraditionalScheduler::fireTorpedo(Side_t side, int hit, int launcher, int torpedoDiff, const HitEffect& effect)
{
    m_consumer.pushEvent(Event(Event::FireTorpedo, side, launcher, hit, ANIMATION_ID, 6 /* FIXME: compute from distance! */));
    m_consumer.pushEvent(Event(Event::UpdateNumTorpedoes, side, torpedoDiff));
    m_consumer.pushEvent(Event(Event::WaitAnimation, side, ANIMATION_ID));
    renderHit(flipSide(side), effect);
}

void
client::vcr::classic::TraditionalScheduler::updateBeam(Side_t side, int id, int value)
{
    m_consumer.pushEvent(Event(Event::UpdateBeam, side, id, value));
}

void
client::vcr::classic::TraditionalScheduler::updateLauncher(Side_t side, int id, int value)
{
    m_consumer.pushEvent(Event(Event::UpdateLauncher, side, id, value));
}

void
client::vcr::classic::TraditionalScheduler::moveObject(Side_t side, int position)
{
    m_consumer.pushEvent(Event(Event::MoveObject, side, position));
}

void
client::vcr::classic::TraditionalScheduler::moveFighter(Side_t side, int track, int position, int distance, FighterStatus_t status)
{
    m_consumer.pushEvent(Event(Event::MoveFighter, side, track, position, distance, status));
}

void
client::vcr::classic::TraditionalScheduler::killObject(Side_t /*side*/)
{
    // FIXME: implement
}

void
client::vcr::classic::TraditionalScheduler::updateObject(Side_t side, int damage, int crew, int shield)
{
    m_consumer.pushEvent(Event(Event::UpdateObject, side, damage, crew, shield));
}

void
client::vcr::classic::TraditionalScheduler::updateAmmo(Side_t side, int numTorpedoes, int numFighters)
{
    m_consumer.pushEvent(Event(Event::UpdateAmmo, side, numTorpedoes, numFighters));
}

void
client::vcr::classic::TraditionalScheduler::updateFighter(Side_t side, int track, int position, int distance, FighterStatus_t status)
{
    m_consumer.pushEvent(Event(Event::UpdateFighter, side, track, position, distance, status));
}

void
client::vcr::classic::TraditionalScheduler::setResult(game::vcr::classic::BattleResult_t result)
{
    m_consumer.pushEvent(Event(Event::SetResult, LeftSide, result.toInteger()));
}

void
client::vcr::classic::TraditionalScheduler::removeAnimations()
{
    m_consumer.removeAnimations(ANIMATION_ID);
}

void
client::vcr::classic::TraditionalScheduler::renderHit(Side_t side, const HitEffect& effect)
{
    m_consumer.pushEvent(Event(Event::HitObject, side, effect.damageDone, effect.crewKilled, effect.shieldLost, ANIMATION_ID));
    m_consumer.pushEvent(Event(Event::WaitAnimation, side, ANIMATION_ID));
}
