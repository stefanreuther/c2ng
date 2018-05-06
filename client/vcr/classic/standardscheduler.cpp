/**
  *  \file client/vcr/classic/standardscheduler.cpp
  */

#include "client/vcr/classic/standardscheduler.hpp"
#include "client/vcr/classic/eventconsumer.hpp"

namespace {
    using game::vcr::classic::LeftSide;
    const int32_t ANIMATION_ID = 99;
}

client::vcr::classic::StandardScheduler::StandardScheduler(client::vcr::classic::EventConsumer& parent)
    : m_consumer(parent)
{ }

void
client::vcr::classic::StandardScheduler::placeObject(Side_t side, const UnitInfo& info)
{
    m_consumer.placeObject(side, info);
}

void
client::vcr::classic::StandardScheduler::updateTime(Time_t time, int32_t distance)
{
    for (size_t i = 0; i < m_pre.size(); ++i) {
        m_consumer.pushEvent(m_pre[i]);
    }
    if (!m_pre.empty()) {
        m_consumer.pushEvent(Event(Event::WaitAnimation, LeftSide, ANIMATION_ID));
    }
    for (size_t i = 0; i < m_post.size(); ++i) {
        m_consumer.pushEvent(m_post[i]);
    }
    if (!m_post.empty()) {
        m_consumer.pushEvent(Event(Event::WaitAnimation, LeftSide, ANIMATION_ID));
    }
    m_pre.clear();
    m_post.clear();

    m_consumer.pushEvent(Event(Event::UpdateTime, LeftSide, time));
    m_consumer.pushEvent(Event(Event::UpdateDistance, LeftSide, distance));
    m_consumer.pushEvent(Event(Event::WaitTick, LeftSide));
}

void
client::vcr::classic::StandardScheduler::startFighter(Side_t side, int track, int position, int distance, int fighterDiff)
{
    m_pre.push_back(Event(Event::StartFighter, side, track, position, distance));
    m_pre.push_back(Event(Event::UpdateNumFighters, side, fighterDiff));
}

void
client::vcr::classic::StandardScheduler::landFighter(Side_t side, int track, int fighterDiff)
{
    m_pre.push_back(Event(Event::RemoveFighter, side, track));
    m_pre.push_back(Event(Event::UpdateNumFighters, side, fighterDiff));
}

void
client::vcr::classic::StandardScheduler::killFighter(Side_t side, int track)
{
    m_post.push_back(Event(Event::ExplodeFighter, side, track, ANIMATION_ID));
    m_post.push_back(Event(Event::RemoveFighter, side, track));
}

void
client::vcr::classic::StandardScheduler::fireBeam(Side_t side, int track, int target, int hit, int /*damage*/, int /*kill*/, const HitEffect& effect)
{
    if (track < 0) {
        int beamSlot = -1-track;
        if (target < 0) {
            // Ship/Ship
            m_pre.push_back(Event(Event::FireBeamShipShip, side, beamSlot, ANIMATION_ID));
            if (hit >= 0) {
                renderHit(flipSide(side), effect);
            }
        } else {
            // Ship/Fighter
            m_pre.push_back(Event(Event::FireBeamShipFighter, side, target, beamSlot, ANIMATION_ID));
        }
    } else {
        if (target < 0) {
            // Fighter/Ship
            m_pre.push_back(Event(Event::FireBeamFighterShip, side, track, ANIMATION_ID));
            if (hit >= 0) {
                renderHit(flipSide(side), effect);
            }
        } else {
            // Fighter/Fighter
            m_pre.push_back(Event(Event::FireBeamFighterFighter, side, track, target, ANIMATION_ID));
        }
    }
}

void
client::vcr::classic::StandardScheduler::fireTorpedo(Side_t side, int hit, int launcher, int torpedoDiff, const HitEffect& effect)
{
    m_pre.push_back(Event(Event::FireTorpedo, side, launcher, hit, ANIMATION_ID, 6 /* FIXME: compute from distance! */));
    m_pre.push_back(Event(Event::UpdateNumTorpedoes, side, torpedoDiff));
    renderHit(flipSide(side), effect);
}

void
client::vcr::classic::StandardScheduler::updateBeam(Side_t side, int id, int value)
{
    m_post.push_back(Event(Event::UpdateBeam, side, id, value));
}

void
client::vcr::classic::StandardScheduler::updateLauncher(Side_t side, int id, int value)
{
    m_post.push_back(Event(Event::UpdateLauncher, side, id, value));
}

void
client::vcr::classic::StandardScheduler::moveObject(Side_t side, int position)
{
    m_pre.push_back(Event(Event::MoveObject, side, position));
}

void
client::vcr::classic::StandardScheduler::moveFighter(Side_t side, int track, int position, int distance, FighterStatus_t status)
{
    m_pre.push_back(Event(Event::MoveFighter, side, track, position, distance, status));
}

void
client::vcr::classic::StandardScheduler::killObject(Side_t /*side*/)
{
    // FIXME: implement
}

void
client::vcr::classic::StandardScheduler::updateObject(Side_t side, int damage, int crew, int shield)
{
    m_pre.push_back(Event(Event::UpdateObject, side, damage, crew, shield));
}

void
client::vcr::classic::StandardScheduler::updateAmmo(Side_t side, int numTorpedoes, int numFighters)
{
    m_pre.push_back(Event(Event::UpdateAmmo, side, numTorpedoes, numFighters));
}

void
client::vcr::classic::StandardScheduler::updateFighter(Side_t side, int track, int position, int distance, FighterStatus_t status)
{
    m_pre.push_back(Event(Event::UpdateFighter, side, track, position, distance, status));
}

void
client::vcr::classic::StandardScheduler::setResult(game::vcr::classic::BattleResult_t result)
{
    m_post.push_back(Event(Event::SetResult, LeftSide, result.toInteger()));
}

void
client::vcr::classic::StandardScheduler::removeAnimations()
{
    m_consumer.removeAnimations(ANIMATION_ID);
}

void
client::vcr::classic::StandardScheduler::renderHit(Side_t side, const HitEffect& effect)
{
    m_post.push_back(Event(Event::HitObject, side, effect.damageDone, effect.crewKilled, effect.shieldLost, ANIMATION_ID));
}
