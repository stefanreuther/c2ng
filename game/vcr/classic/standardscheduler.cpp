/**
  *  \file game/vcr/classic/standardscheduler.cpp
  *  \brief Class game::vcr::classic::StandardScheduler
  */

#include "game/vcr/classic/standardscheduler.hpp"
#include "game/vcr/classic/scheduledeventconsumer.hpp"

namespace {
    const int32_t ANIMATION_ID = 99;
}

game::vcr::classic::StandardScheduler::StandardScheduler(ScheduledEventConsumer& parent)
    : m_consumer(parent)
{ }

void
game::vcr::classic::StandardScheduler::placeObject(Side side, const UnitInfo& info)
{
    m_consumer.placeObject(side, info);
}

void
game::vcr::classic::StandardScheduler::updateTime(Time_t time, int32_t distance)
{
    for (size_t i = 0; i < m_pre.size(); ++i) {
        m_consumer.pushEvent(m_pre[i]);
    }
    if (!m_pre.empty()) {
        m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::WaitAnimation, LeftSide, ANIMATION_ID));
    }
    for (size_t i = 0; i < m_post.size(); ++i) {
        m_consumer.pushEvent(m_post[i]);
    }
    if (!m_post.empty()) {
        m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::WaitAnimation, LeftSide, ANIMATION_ID));
    }
    m_pre.clear();
    m_post.clear();

    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::UpdateTime, LeftSide, time));
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::UpdateDistance, LeftSide, distance));
    m_consumer.pushEvent(ScheduledEvent(ScheduledEvent::WaitTick, LeftSide));
}

void
game::vcr::classic::StandardScheduler::startFighter(Side side, int track, int position, int distance, int fighterDiff)
{
    m_pre.push_back(ScheduledEvent(ScheduledEvent::StartFighter, side, track, position, distance));
    m_pre.push_back(ScheduledEvent(ScheduledEvent::UpdateNumFighters, side, fighterDiff));
}

void
game::vcr::classic::StandardScheduler::landFighter(Side side, int track, int fighterDiff)
{
    m_pre.push_back(ScheduledEvent(ScheduledEvent::RemoveFighter, side, track));
    m_pre.push_back(ScheduledEvent(ScheduledEvent::UpdateNumFighters, side, fighterDiff));
}

void
game::vcr::classic::StandardScheduler::killFighter(Side side, int track)
{
    m_post.push_back(ScheduledEvent(ScheduledEvent::ExplodeFighter, side, track, ANIMATION_ID));
    m_post.push_back(ScheduledEvent(ScheduledEvent::RemoveFighter, side, track));
}

void
game::vcr::classic::StandardScheduler::fireBeam(Side side, int track, int target, int hit, int /*damage*/, int /*kill*/, const HitEffect& effect)
{
    if (track < 0) {
        int beamSlot = -1-track;
        if (target < 0) {
            // Ship/Ship
            m_pre.push_back(ScheduledEvent(ScheduledEvent::FireBeamShipShip, side, beamSlot, ANIMATION_ID));
            if (hit >= 0) {
                renderHit(flipSide(side), effect);
            }
        } else {
            // Ship/Fighter
            m_pre.push_back(ScheduledEvent(ScheduledEvent::FireBeamShipFighter, side, target, beamSlot, ANIMATION_ID));
        }
    } else {
        if (target < 0) {
            // Fighter/Ship
            m_pre.push_back(ScheduledEvent(ScheduledEvent::FireBeamFighterShip, side, track, ANIMATION_ID));
            if (hit >= 0) {
                renderHit(flipSide(side), effect);
            }
        } else {
            // Fighter/Fighter
            m_pre.push_back(ScheduledEvent(ScheduledEvent::FireBeamFighterFighter, side, track, target, ANIMATION_ID));
        }
    }
}

void
game::vcr::classic::StandardScheduler::fireTorpedo(Side side, int hit, int launcher, int torpedoDiff, const HitEffect& effect)
{
    m_pre.push_back(ScheduledEvent(ScheduledEvent::FireTorpedo, side, launcher, hit, ANIMATION_ID, 6 /* FIXME: compute from distance! */));
    m_pre.push_back(ScheduledEvent(ScheduledEvent::UpdateNumTorpedoes, side, torpedoDiff));
    renderHit(flipSide(side), effect);
}

void
game::vcr::classic::StandardScheduler::updateBeam(Side side, int id, int value)
{
    m_post.push_back(ScheduledEvent(ScheduledEvent::UpdateBeam, side, id, value));
}

void
game::vcr::classic::StandardScheduler::updateLauncher(Side side, int id, int value)
{
    m_post.push_back(ScheduledEvent(ScheduledEvent::UpdateLauncher, side, id, value));
}

void
game::vcr::classic::StandardScheduler::moveObject(Side side, int position)
{
    m_pre.push_back(ScheduledEvent(ScheduledEvent::MoveObject, side, position));
}

void
game::vcr::classic::StandardScheduler::moveFighter(Side side, int track, int position, int distance, FighterStatus status)
{
    m_pre.push_back(ScheduledEvent(ScheduledEvent::MoveFighter, side, track, position, distance, status));
}

void
game::vcr::classic::StandardScheduler::killObject(Side /*side*/)
{
    // FIXME: implement
}

void
game::vcr::classic::StandardScheduler::updateObject(Side side, int damage, int crew, int shield)
{
    m_pre.push_back(ScheduledEvent(ScheduledEvent::UpdateObject, side, damage, crew, shield));
}

void
game::vcr::classic::StandardScheduler::updateAmmo(Side side, int numTorpedoes, int numFighters)
{
    m_pre.push_back(ScheduledEvent(ScheduledEvent::UpdateAmmo, side, numTorpedoes, numFighters));
}

void
game::vcr::classic::StandardScheduler::updateFighter(Side side, int track, int position, int distance, FighterStatus status)
{
    m_pre.push_back(ScheduledEvent(ScheduledEvent::UpdateFighter, side, track, position, distance, status));
}

void
game::vcr::classic::StandardScheduler::setResult(BattleResult_t result)
{
    m_post.push_back(ScheduledEvent(ScheduledEvent::SetResult, LeftSide, result.toInteger()));
}

void
game::vcr::classic::StandardScheduler::removeAnimations()
{
    m_consumer.removeAnimations(ANIMATION_ID);
}

void
game::vcr::classic::StandardScheduler::renderHit(Side side, const HitEffect& effect)
{
    m_post.push_back(ScheduledEvent(ScheduledEvent::HitObject, side, effect.damageDone, effect.crewKilled, effect.shieldLost, ANIMATION_ID));
}
