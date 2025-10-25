/**
  *  \file game/vcr/classic/mirroringeventlistener.cpp
  *  \brief Class game::vcr::classic::MirroringEventListener
  */

#include "game/vcr/classic/mirroringeventlistener.hpp"
#include "game/vcr/classic/algorithm.hpp"

game::vcr::classic::MirroringEventListener::MirroringEventListener(EventListener& listener)
    : m_listener(listener)
{ }

game::vcr::classic::MirroringEventListener::~MirroringEventListener()
{ }

void
game::vcr::classic::MirroringEventListener::placeObject(Side side, const UnitInfo& info)
{
    UnitInfo i = info;
    i.position = flipCoordinate(info.position);
    m_listener.placeObject(flipSide(side), i);
}

void
game::vcr::classic::MirroringEventListener::updateTime(Time_t time, int32_t distance)
{
    m_listener.updateTime(time, distance);
}

void
game::vcr::classic::MirroringEventListener::startFighter(Side side, int track, int position, int distance, int fighterDiff)
{
    m_listener.startFighter(flipSide(side), track, flipCoordinate(position), distance, fighterDiff);
}

void
game::vcr::classic::MirroringEventListener::landFighter(Side side, int track, int fighterDiff)
{
    m_listener.landFighter(flipSide(side), track, fighterDiff);
}

void
game::vcr::classic::MirroringEventListener::killFighter(Side side, int track)
{
    m_listener.killFighter(flipSide(side), track);
}

void
game::vcr::classic::MirroringEventListener::fireBeam(Side side, int track, int target, int hit, int damage, int kill, const HitEffect& effect)
{
    m_listener.fireBeam(flipSide(side), track, target, hit, damage, kill, effect);
}

void
game::vcr::classic::MirroringEventListener::fireTorpedo(Side side, int hit, int launcher, int torpedoDiff, const HitEffect& effect)
{
    m_listener.fireTorpedo(flipSide(side), hit, launcher, torpedoDiff, effect);
}

void
game::vcr::classic::MirroringEventListener::updateBeam(Side side, int id, int value)
{
    m_listener.updateBeam(flipSide(side), id, value);
}

void
game::vcr::classic::MirroringEventListener::updateLauncher(Side side, int id, int value)
{
    m_listener.updateLauncher(flipSide(side), id, value);
}

void
game::vcr::classic::MirroringEventListener::moveObject(Side side, int position)
{
    m_listener.moveObject(flipSide(side), flipCoordinate(position));
}

void
game::vcr::classic::MirroringEventListener::moveFighter(Side side, int track, int position, int distance, FighterStatus status)
{
    m_listener.moveFighter(flipSide(side), track, flipCoordinate(position), distance, status);
}

void
game::vcr::classic::MirroringEventListener::killObject(Side side)
{
    m_listener.killObject(flipSide(side));
}

void
game::vcr::classic::MirroringEventListener::updateObject(Side side, int damage, int crew, int shield)
{
    m_listener.updateObject(flipSide(side), damage, crew, shield);
}

void
game::vcr::classic::MirroringEventListener::updateAmmo(Side side, int numTorpedoes, int numFighters)
{
    m_listener.updateAmmo(flipSide(side), numTorpedoes, numFighters);
}

void
game::vcr::classic::MirroringEventListener::updateFighter(Side side, int track, int position, int distance, FighterStatus status)
{
    m_listener.updateFighter(flipSide(side), track, flipCoordinate(position), distance, status);
}

void
game::vcr::classic::MirroringEventListener::setResult(BattleResult_t result)
{
    BattleResult_t flipped;
    if (result.contains(LeftDestroyed)) {
        flipped += RightDestroyed;
    }
    if (result.contains(RightDestroyed)) {
        flipped += LeftDestroyed;
    }
    if (result.contains(LeftCaptured)) {
        flipped += RightCaptured;
    }
    if (result.contains(RightCaptured)) {
        flipped += LeftCaptured;
    }
    if (result.contains(Timeout)) {
        flipped += Timeout;
    }
    if (result.contains(Stalemate)) {
        flipped += Stalemate;
    }
    if (result.contains(Invalid)) {
        flipped += Invalid;
    }
    m_listener.setResult(flipped);
}

void
game::vcr::classic::MirroringEventListener::removeAnimations()
{
    m_listener.removeAnimations();
}

inline int
game::vcr::classic::MirroringEventListener::flipCoordinate(int x)
{
    return Algorithm::MAX_COORDINATE - x;
}

