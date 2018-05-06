/**
  *  \file client/vcr/classic/interleavedscheduler.cpp
  */

#include "client/vcr/classic/interleavedscheduler.hpp"
#include "client/vcr/classic/eventconsumer.hpp"

namespace {
    using game::vcr::classic::LeftSide;
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


client::vcr::classic::InterleavedScheduler::InterleavedScheduler(EventConsumer& parent)
    : m_consumer(parent),
      m_animationCounter(FIRST_ANIMATION_ID),
      m_finished(false)
{ }

void
client::vcr::classic::InterleavedScheduler::placeObject(Side_t side, const UnitInfo& info)
{
    m_finished = false;
    m_consumer.placeObject(side, info);
}

void
client::vcr::classic::InterleavedScheduler::updateTime(Time_t time, int32_t distance)
{
    m_queue[NOW].post.push_back(Event(Event::UpdateTime, LeftSide, time));
    m_queue[NOW].post.push_back(Event(Event::UpdateDistance, LeftSide, distance));
    m_queue[NOW].post.push_back(Event(Event::WaitTick, LeftSide));
    shift();
}

void
client::vcr::classic::InterleavedScheduler::startFighter(Side_t side, int track, int position, int distance, int fighterDiff)
{
    m_queue[NOW].pre.push_back(Event(Event::StartFighter, side, track, position, distance));
    m_queue[NOW].pre.push_back(Event(Event::UpdateNumFighters, side, fighterDiff));
}

void
client::vcr::classic::InterleavedScheduler::landFighter(Side_t side, int track, int fighterDiff)
{
    m_queue[NOW].pre.push_back(Event(Event::RemoveFighter, side, track));
    m_queue[NOW].pre.push_back(Event(Event::UpdateNumFighters, side, fighterDiff));
}

void
client::vcr::classic::InterleavedScheduler::killFighter(Side_t side, int track)
{
    int id = m_animationCounter++;
    m_queue[NOW].pre.push_back(Event(Event::ExplodeFighter, side, track, id));
    m_queue[NOW].pre.push_back(Event(Event::RemoveFighter, side, track));
    m_queue[0].pre.push_back(Event(Event::WaitAnimation, side, id));
}

void
client::vcr::classic::InterleavedScheduler::fireBeam(Side_t side, int track, int target, int hit, int /*damage*/, int /*kill*/, const HitEffect& effect)
{
    if (track < 0) {
        int beamSlot = -1-track;
        if (target < 0) {
            // Ship/Ship
            int id = m_animationCounter++;
            m_queue[NOW+2].pre.push_back(Event(Event::FireBeamShipShip, side, beamSlot, id));
            m_queue[NOW+2].pre.push_back(Event(Event::BlockBeam, side, beamSlot));
            m_queue[NOW].pre.push_back(Event(Event::WaitAnimation, side, id));
            m_queue[NOW].pre.push_back(Event(Event::UnblockBeam, side, beamSlot));
            if (hit >= 0) {
                renderHit(flipSide(side), effect);
            }
        } else {
            // Ship/Fighter
            int id = m_animationCounter++;
            m_queue[NOW+2].pre.push_back(Event(Event::FireBeamShipFighter, side, target, beamSlot, id));
            m_queue[NOW+2].pre.push_back(Event(Event::BlockBeam, side, beamSlot));
            m_queue[NOW].pre.push_back(Event(Event::WaitAnimation, side, id));
            m_queue[NOW].pre.push_back(Event(Event::UnblockBeam, side, beamSlot));
        }
    } else {
        if (target < 0) {
            // Fighter/Ship
            int id = m_animationCounter++;
            m_queue[NOW].pre.push_back(Event(Event::FireBeamFighterShip, side, track, id));
            m_queue[NOW+1].pre.push_back(Event(Event::WaitAnimation, side, id));
            if (hit >= 0) {
                renderHit(flipSide(side), effect);
            }
        } else {
            // Fighter/Fighter
            int id = m_animationCounter++;
            m_queue[NOW].pre.push_back(Event(Event::FireBeamFighterFighter, side, track, target, id));
            m_queue[NOW+1].pre.push_back(Event(Event::WaitAnimation, side, id));
        }
    }
}

void
client::vcr::classic::InterleavedScheduler::fireTorpedo(Side_t side, int hit, int launcher, int torpedoDiff, const HitEffect& effect)
{
    int id = m_animationCounter++;
    m_queue[NOW+3].pre.push_back(Event(Event::FireTorpedo, side, launcher, hit, id, 6 /* FIXME: compute from distance! */));
    m_queue[NOW+3].pre.push_back(Event(Event::UpdateNumTorpedoes, side, torpedoDiff));
    m_queue[NOW+3].pre.push_back(Event(Event::BlockLauncher, side, launcher));
    m_queue[NOW].pre.push_back(Event(Event::WaitAnimation, side, id));
    m_queue[NOW].pre.push_back(Event(Event::UnblockLauncher, side, launcher));
    renderHit(flipSide(side), effect);
}

void
client::vcr::classic::InterleavedScheduler::updateBeam(Side_t side, int id, int value)
{
    m_queue[NOW].pre.push_back(Event(Event::UpdateBeam, side, id, value));
}

void
client::vcr::classic::InterleavedScheduler::updateLauncher(Side_t side, int id, int value)
{
    m_queue[NOW].pre.push_back(Event(Event::UpdateLauncher, side, id, value));
}

void
client::vcr::classic::InterleavedScheduler::moveObject(Side_t side, int position)
{
    m_queue[NOW].pre.push_back(Event(Event::MoveObject, side, position));
}

void
client::vcr::classic::InterleavedScheduler::moveFighter(Side_t side, int track, int position, int distance, FighterStatus_t status)
{
    m_queue[NOW].pre.push_back(Event(Event::MoveFighter, side, track, position, distance, status));
}

void
client::vcr::classic::InterleavedScheduler::killObject(Side_t /*side*/)
{
    // FIXME: implement
}

void
client::vcr::classic::InterleavedScheduler::updateObject(Side_t side, int damage, int crew, int shield)
{
    m_finished = false;
    m_queue[NOW].pre.push_back(Event(Event::UpdateObject, side, damage, crew, shield));
}

void
client::vcr::classic::InterleavedScheduler::updateAmmo(Side_t side, int numTorpedoes, int numFighters)
{
    m_queue[NOW].pre.push_back(Event(Event::UpdateAmmo, side, numTorpedoes, numFighters));
}

void
client::vcr::classic::InterleavedScheduler::updateFighter(Side_t side, int track, int position, int distance, FighterStatus_t status)
{
    m_queue[NOW].pre.push_back(Event(Event::UpdateFighter, side, track, position, distance, status));
}

void
client::vcr::classic::InterleavedScheduler::setResult(game::vcr::classic::BattleResult_t result)
{
    m_finished = true;
    m_queue[NOW].pre.push_back(Event(Event::SetResult, LeftSide, result.toInteger()));
}

void
client::vcr::classic::InterleavedScheduler::removeAnimations()
{
    // FIXME: this is inefficient. Can we do better?
    for (int i = FIRST_ANIMATION_ID; i < m_animationCounter; ++i) {
        m_consumer.removeAnimations(i);
    }
    m_animationCounter = FIRST_ANIMATION_ID;
}

void
client::vcr::classic::InterleavedScheduler::renderHit(Side_t side, const HitEffect& effect)
{
    int id = m_animationCounter++;
    m_queue[NOW].pre.push_back(Event(Event::HitObject, side, effect.damageDone, effect.crewKilled, effect.shieldLost, id));
    m_queue[0].pre.push_back(Event(Event::WaitAnimation, LeftSide, id));
}

void
client::vcr::classic::InterleavedScheduler::shift()
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
