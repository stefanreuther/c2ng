/**
  *  \file game/vcr/classic/interleavedscheduler.cpp
  *  \brief Class game::vcr::classic::InterleavedScheduler
  */

#include "game/vcr/classic/interleavedscheduler.hpp"
#include "game/vcr/classic/scheduledeventconsumer.hpp"

namespace {
    const size_t NOW = 2;
    const int FIRST_ANIMATION_ID = 1;
}

/*
 *  FIXME: this definitely still needs some tuning.
 *  As of 20180408, this is experimental.
 *
 *  FIXME: a problem this has to solve is that if we fire beams earlier,
 *  the fighters they hit will not yet be present.
 *  Possible ideas:
 *  - make virtual fighter tracks by alternatingly adding +57 to the tracks, and launch fighters earlier
 *  - track whether we have launched a fighter, and do not fire the beam earlier
 */

game::vcr::classic::InterleavedScheduler::InterleavedScheduler(ScheduledEventConsumer& parent)
    : m_consumer(parent),
      m_animationCounter(FIRST_ANIMATION_ID),
      m_finished(false)
{ }

void
game::vcr::classic::InterleavedScheduler::placeObject(Side side, const UnitInfo& info)
{
    m_finished = false;
    m_consumer.placeObject(side, info);
}

void
game::vcr::classic::InterleavedScheduler::updateTime(Time_t time, int32_t distance)
{
    m_queue[NOW].post.push_back(ScheduledEvent(ScheduledEvent::UpdateTime, LeftSide, time));
    m_queue[NOW].post.push_back(ScheduledEvent(ScheduledEvent::UpdateDistance, LeftSide, distance));
    m_queue[NOW].post.push_back(ScheduledEvent(ScheduledEvent::WaitTick, LeftSide));
    shift();
}

void
game::vcr::classic::InterleavedScheduler::startFighter(Side side, int track, int position, int distance, int fighterDiff)
{
    m_queue[NOW].pre.push_back(ScheduledEvent(ScheduledEvent::StartFighter, side, track, position, distance));
    m_queue[NOW].pre.push_back(ScheduledEvent(ScheduledEvent::UpdateNumFighters, side, fighterDiff));
}

void
game::vcr::classic::InterleavedScheduler::landFighter(Side side, int track, int fighterDiff)
{
    m_queue[NOW].pre.push_back(ScheduledEvent(ScheduledEvent::RemoveFighter, side, track));
    m_queue[NOW].pre.push_back(ScheduledEvent(ScheduledEvent::UpdateNumFighters, side, fighterDiff));
}

void
game::vcr::classic::InterleavedScheduler::killFighter(Side side, int track)
{
    int id = m_animationCounter++;
    m_queue[NOW].pre.push_back(ScheduledEvent(ScheduledEvent::ExplodeFighter, side, track, id));
    m_queue[NOW].pre.push_back(ScheduledEvent(ScheduledEvent::RemoveFighter, side, track));
    m_queue[0].pre.push_back(ScheduledEvent(ScheduledEvent::WaitAnimation, side, id));
}

void
game::vcr::classic::InterleavedScheduler::fireBeam(Side side, int track, int target, int hit, int /*damage*/, int /*kill*/, const HitEffect& effect)
{
    if (track < 0) {
        int beamSlot = -1-track;
        if (target < 0) {
            // Ship/Ship
            int id = m_animationCounter++;
            m_queue[NOW+2].pre.push_back(ScheduledEvent(ScheduledEvent::FireBeamShipShip, side, beamSlot, id));
            m_queue[NOW+2].pre.push_back(ScheduledEvent(ScheduledEvent::BlockBeam, side, beamSlot));
            m_queue[NOW].pre.push_back(ScheduledEvent(ScheduledEvent::WaitAnimation, side, id));
            m_queue[NOW].pre.push_back(ScheduledEvent(ScheduledEvent::UnblockBeam, side, beamSlot));
            if (hit >= 0) {
                renderHit(flipSide(side), effect);
            }
        } else {
            // Ship/Fighter
            int id = m_animationCounter++;
            m_queue[NOW+2].pre.push_back(ScheduledEvent(ScheduledEvent::FireBeamShipFighter, side, target, beamSlot, id));
            m_queue[NOW+2].pre.push_back(ScheduledEvent(ScheduledEvent::BlockBeam, side, beamSlot));
            m_queue[NOW].pre.push_back(ScheduledEvent(ScheduledEvent::WaitAnimation, side, id));
            m_queue[NOW].pre.push_back(ScheduledEvent(ScheduledEvent::UnblockBeam, side, beamSlot));
        }
    } else {
        if (target < 0) {
            // Fighter/Ship
            int id = m_animationCounter++;
            m_queue[NOW+1].pre.push_back(ScheduledEvent(ScheduledEvent::FireBeamFighterShip, side, track, id));
            m_queue[NOW].pre.push_back(ScheduledEvent(ScheduledEvent::WaitAnimation, side, id));
            if (hit >= 0) {
                renderHit(flipSide(side), effect);
            }
        } else {
            // Fighter/Fighter
            int id = m_animationCounter++;
            m_queue[NOW+1].pre.push_back(ScheduledEvent(ScheduledEvent::FireBeamFighterFighter, side, track, target, id));
            m_queue[NOW].pre.push_back(ScheduledEvent(ScheduledEvent::WaitAnimation, side, id));
        }
    }
}

void
game::vcr::classic::InterleavedScheduler::fireTorpedo(Side side, int hit, int launcher, int torpedoDiff, const HitEffect& effect)
{
    int id = m_animationCounter++;
    m_queue[NOW+3].pre.push_back(ScheduledEvent(ScheduledEvent::FireTorpedo, side, launcher, hit, id, 6 /* FIXME: compute from distance! */));
    m_queue[NOW+3].pre.push_back(ScheduledEvent(ScheduledEvent::UpdateNumTorpedoes, side, torpedoDiff));
    m_queue[NOW+3].pre.push_back(ScheduledEvent(ScheduledEvent::BlockLauncher, side, launcher));
    m_queue[NOW].pre.push_back(ScheduledEvent(ScheduledEvent::WaitAnimation, side, id));
    m_queue[NOW].pre.push_back(ScheduledEvent(ScheduledEvent::UnblockLauncher, side, launcher));
    renderHit(flipSide(side), effect);
}

void
game::vcr::classic::InterleavedScheduler::updateBeam(Side side, int id, int value)
{
    m_queue[NOW].pre.push_back(ScheduledEvent(ScheduledEvent::UpdateBeam, side, id, value));
}

void
game::vcr::classic::InterleavedScheduler::updateLauncher(Side side, int id, int value)
{
    m_queue[NOW].pre.push_back(ScheduledEvent(ScheduledEvent::UpdateLauncher, side, id, value));
}

void
game::vcr::classic::InterleavedScheduler::moveObject(Side side, int position)
{
    m_queue[NOW].pre.push_back(ScheduledEvent(ScheduledEvent::MoveObject, side, position));
}

void
game::vcr::classic::InterleavedScheduler::moveFighter(Side side, int track, int position, int distance, FighterStatus status)
{
    m_queue[NOW].pre.push_back(ScheduledEvent(ScheduledEvent::MoveFighter, side, track, position, distance, status));
}

void
game::vcr::classic::InterleavedScheduler::killObject(Side /*side*/)
{
    // FIXME: implement
}

void
game::vcr::classic::InterleavedScheduler::updateObject(Side side, int damage, int crew, int shield)
{
    m_finished = false;
    m_queue[NOW].pre.push_back(ScheduledEvent(ScheduledEvent::UpdateObject, side, damage, crew, shield));
}

void
game::vcr::classic::InterleavedScheduler::updateAmmo(Side side, int numTorpedoes, int numFighters)
{
    m_queue[NOW].pre.push_back(ScheduledEvent(ScheduledEvent::UpdateAmmo, side, numTorpedoes, numFighters));
}

void
game::vcr::classic::InterleavedScheduler::updateFighter(Side side, int track, int position, int distance, FighterStatus status)
{
    m_queue[NOW].pre.push_back(ScheduledEvent(ScheduledEvent::UpdateFighter, side, track, position, distance, status));
}

void
game::vcr::classic::InterleavedScheduler::setResult(BattleResult_t result)
{
    m_finished = true;
    m_queue[NOW].pre.push_back(ScheduledEvent(ScheduledEvent::SetResult, LeftSide, result.toInteger()));
}

void
game::vcr::classic::InterleavedScheduler::removeAnimations()
{
    // FIXME: this is inefficient. Can we do better?
    for (int i = FIRST_ANIMATION_ID; i < m_animationCounter; ++i) {
        m_consumer.removeAnimations(i);
    }
    m_animationCounter = FIRST_ANIMATION_ID;
}

void
game::vcr::classic::InterleavedScheduler::renderHit(Side side, const HitEffect& effect)
{
    int id = m_animationCounter++;
    m_queue[NOW].pre.push_back(ScheduledEvent(ScheduledEvent::HitObject, side, effect.damageDone, effect.crewKilled, effect.shieldLost, id));
    m_queue[0].pre.push_back(ScheduledEvent(ScheduledEvent::WaitAnimation, LeftSide, id));
}

void
game::vcr::classic::InterleavedScheduler::shift()
{
    int count = m_finished ? NUM_FRAMES : 1;
    for (int i = 0; i < count; ++i) {
        // Send final list
        Frame& final = m_queue[NUM_FRAMES-1];
        for (size_t i = 0, n = final.pre.size(); i < n; ++i) {
            m_consumer.pushEvent(final.pre[i]);
        }
        for (size_t i = 0, n = final.post.size(); i < n; ++i) {
            m_consumer.pushEvent(final.post[i]);
        }
        final.pre.clear();
        final.post.clear();

        // Shift everything
        for (size_t i = NUM_FRAMES-1; i > 0; --i) {
            m_queue[i].pre.swap(m_queue[i-1].pre);
            m_queue[i].post.swap(m_queue[i-1].post);
        }
    }
}
