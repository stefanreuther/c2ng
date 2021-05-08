/**
  *  \file game/vcr/classic/eventvisualizer.cpp
  *  \brief Class game::vcr::classic::EventVisualizer
  */

#include "game/vcr/classic/eventvisualizer.hpp"
#include "afl/string/format.hpp"
#include "game/vcr/classic/algorithm.hpp"
#include "game/vcr/classic/battle.hpp"
#include "game/vcr/classic/utils.hpp"

namespace {
    int update(int& value, int newValue)
    {
        int oldValue = value;
        value = newValue;
        return newValue - oldValue;
    }
}


game::vcr::classic::EventVisualizer::EventVisualizer(EventListener& listener)
    : m_listener(listener)
{ }

game::vcr::classic::EventVisualizer::~EventVisualizer()
{ }

void
game::vcr::classic::EventVisualizer::init(Algorithm& algo,
                                          const Battle& battle,
                                          const game::spec::ShipList& shipList,
                                          const PlayerList& players,
                                          const TeamSettings* teams,
                                          const game::config::HostConfiguration& config)
{
    // Start up the algorithm
    algo.setCapabilities(battle.getCapabilities());
    algo.initBattle(battle.left(), battle.right(), battle.getSeed());

    // Initialize events
    initSide(LeftSide, algo, battle.left(), shipList, players, teams, config);
    initSide(RightSide, algo, battle.right(), shipList, players, teams, config);

    // Finalize by setting the time
    m_listener.updateTime(algo.getTime(), algo.getDistance());
}

bool
game::vcr::classic::EventVisualizer::playCycle(Algorithm& algo)
{
    // Play cycle on algorithm. This will generate events.
    bool ok = algo.playCycle();

    // Update everything.
    updateSide(LeftSide, algo);
    updateSide(RightSide, algo);

    // Report finish
    if (!ok) {
        Object left, right;
        algo.doneBattle(left, right);
        m_listener.setResult(algo.getResult());
    }

    // Finalize by setting the time
    m_listener.updateTime(algo.getTime(), algo.getDistance());

    return ok;
}

void
game::vcr::classic::EventVisualizer::refresh(Algorithm& algo, bool done)
{
    // Refresh sides
    refreshSide(LeftSide, algo);
    refreshSide(RightSide, algo);

    // Report finish
    if (done) {
        m_listener.setResult(algo.getResult());
    }

    // Finalize by setting the time
    m_listener.updateTime(algo.getTime(), algo.getDistance());
}


void
game::vcr::classic::EventVisualizer::startFighter(Algorithm& algo, Side side, int track)
{
    int fighterX = algo.getFighterX(side, track);
    m_listener.startFighter(side, track, fighterX, std::abs(algo.getObjectX(side) - fighterX), update(m_unitState[side].numFighters, algo.getNumFighters(side)));
    if (track > m_unitState[side].maxFighterTrack) {
        m_unitState[side].maxFighterTrack = track;
    }
}

void
game::vcr::classic::EventVisualizer::landFighter(Algorithm& algo, Side side, int track)
{
    m_listener.landFighter(side, track, update(m_unitState[side].numFighters, algo.getNumFighters(side)));
}

void
game::vcr::classic::EventVisualizer::killFighter(Algorithm& /*algo*/, Side side, int track)
{
    m_listener.killFighter(side, track);
}
void
game::vcr::classic::EventVisualizer::fireBeam(Algorithm& algo, Side side, int track, int target, int hit, int damage, int kill)
{
    m_listener.fireBeam(side, track, target, hit, damage, kill, getHitEffect(algo, flipSide(side)));
}

void
game::vcr::classic::EventVisualizer::fireTorpedo(Algorithm& algo, Side side, int hit, int launcher)
{
    m_listener.fireTorpedo(side, hit, launcher, update(m_unitState[side].numTorpedoes, algo.getNumTorpedoes(side)), getHitEffect(algo, flipSide(side)));
}

void
game::vcr::classic::EventVisualizer::updateBeam(Algorithm& algo, Side side, int id)
{
    m_listener.updateBeam(side, id, algo.getBeamStatus(side, id));
}

void
game::vcr::classic::EventVisualizer::updateLauncher(Algorithm& algo, Side side, int id)
{
    m_listener.updateLauncher(side, id, algo.getLauncherStatus(side, id));
}

void
game::vcr::classic::EventVisualizer::killObject(Algorithm& /*algo*/, Side side)
{
    m_listener.killObject(side);
}

void
game::vcr::classic::EventVisualizer::initSide(Side side, Algorithm& algo,
                                              const Object& obj,
                                              const game::spec::ShipList& shipList,
                                              const PlayerList& players,
                                              const TeamSettings* teams,
                                              const game::config::HostConfiguration& config)
{
    // Initialize UnitInfo.
    EventListener::UnitInfo ui;
    ui.object          = obj;
    ui.position        = algo.getObjectX(side);
    ui.ownerName       = players.getPlayerName(obj.getOwner(), Player::ShortName);
    ui.relation        = teams != 0 ? teams->getPlayerRelation(obj.getOwner()) : TeamSettings::EnemyPlayer;
    if (const game::spec::Component* beam = shipList.beams().get(obj.getBeamType())) {
        ui.beamName = beam->getName(shipList.componentNamer());
    }
    if (const game::spec::Component* tl = shipList.launchers().get(obj.getTorpedoType())) {
        ui.launcherName = tl->getName(shipList.componentNamer());
    }

    // Initialize UnitState.
    UnitState& us = m_unitState[side];
    us.damage       = algo.getDamage(side);
    us.crew         = algo.getCrew(side);
    us.shield       = algo.getShield(side);
    us.position     = algo.getObjectX(side);
    us.numTorpedoes = algo.getNumTorpedoes(side);
    us.numFighters  = algo.getNumFighters(side);
    us.numBeams     = obj.getNumBeams();
    us.numLaunchers = obj.getNumLaunchers();

    // The algorithm may already have modified the object. Update it.
    ui.object.setDamage(us.damage);
    ui.object.setCrew(us.crew);
    ui.object.setShield(us.shield);
    ui.object.setNumTorpedoes(us.numTorpedoes);
    ui.object.setNumFighters(us.numFighters);

    // Update shadow copies of specification values
    ui.object.setPicture(ui.object.getGuessedShipPicture(shipList.hulls()));
    ui.object.setRace(config.getPlayerRaceNumber(ui.object.getOwner()));

    // Place the object.
    m_listener.placeObject(side, ui);

    // Charge all weapons.
    for (int i = 0, n = ui.object.getNumBeams(); i < n; ++i) {
        m_listener.updateBeam(side, i, algo.getBeamStatus(side, i));
    }
    for (int i = 0, n = ui.object.getNumLaunchers(); i < n; ++i) {
        m_listener.updateLauncher(side, i, algo.getLauncherStatus(side, i));
    }
}

void
game::vcr::classic::EventVisualizer::updateSide(Side side, Algorithm& algo)
{
    // Move object
    int newPos = algo.getObjectX(side);
    if (newPos != m_unitState[side].position) {
        m_unitState[side].position = newPos;
        m_listener.moveObject(side, newPos);
    }

    // Move fighters
    for (int i = 0, n = m_unitState[side].maxFighterTrack; i <= n; ++i) {
        FighterStatus st = algo.getFighterStatus(side, i);
        if (st != FighterIdle) {
            int fighterX = algo.getFighterX(side, i);
            m_listener.moveFighter(side, i, fighterX, std::abs(fighterX - newPos), st);
        }
    }
}

void
game::vcr::classic::EventVisualizer::refreshSide(Side side, Algorithm& algo)
{
    UnitState& us = m_unitState[side];

    // Refresh status
    us.damage       = algo.getDamage(side);
    us.crew         = algo.getCrew(side);
    us.shield       = algo.getShield(side);
    us.numTorpedoes = algo.getNumTorpedoes(side);
    us.numFighters  = algo.getNumFighters(side);
    m_listener.updateObject(side, us.damage, us.crew, us.shield);
    m_listener.updateAmmo(side, us.numTorpedoes, us.numFighters);

    // Refresh position
    const int newPos = algo.getObjectX(side);
    us.position = newPos;
    m_listener.moveObject(side, newPos);

    // Refresh fighter positions
    // We need to iterate through all possible fighter tracks because we may have missed the launch of a fighter
    // on a track beyond maxFighterTrack.
    for (int i = 0; i < Algorithm::MAX_FIGHTER_TRACKS; ++i) {
        FighterStatus st = algo.getFighterStatus(side, i);
        int fighterX = (st != FighterIdle ? algo.getFighterX(side, i) : newPos);
        if (st != FighterIdle || i <= us.maxFighterTrack) {
            m_listener.updateFighter(side, i, fighterX, std::abs(fighterX - newPos), st);
        }
        if (st != FighterIdle && i > us.maxFighterTrack) {
            us.maxFighterTrack = i;
        }
    }

    // Refresh beam status
    for (int i = 0, n = us.numBeams; i < n; ++i) {
        m_listener.updateBeam(side, i, algo.getBeamStatus(side, i));
    }

    // Refresh launcher status
    for (int i = 0, n = us.numLaunchers; i < n; ++i) {
        m_listener.updateLauncher(side, i, algo.getLauncherStatus(side, i));
    }
}

game::vcr::classic::EventListener::HitEffect
game::vcr::classic::EventVisualizer::getHitEffect(Algorithm& algo, Side side)
{
    EventListener::HitEffect eff;
    eff.damageDone =  update(m_unitState[side].damage, algo.getDamage(side));
    eff.crewKilled = -update(m_unitState[side].crew,   algo.getCrew(side));
    eff.shieldLost = -update(m_unitState[side].shield, algo.getShield(side));
    return eff;
}
