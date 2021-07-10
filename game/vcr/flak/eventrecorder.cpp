/**
  *  \file game/vcr/flak/eventrecorder.cpp
  *  \brief Class game::vcr::flak::EventRecorder
  */

#include "game/vcr/flak/eventrecorder.hpp"

namespace {
    enum Command {
        cUpdateTime,
        cFireBeamFighterFighter,
        cFireBeamFighterShip,
        cFireBeamShipFighter,
        cFireBeamShipShip,
        cCreateFighter,
        cKillFighter,
        cLandFighter,
        cMoveFighter,
        cCreateFleet,
        cSetEnemy,
        cKillFleet,
        cMoveFleet,
        cCreateShip,
        cKillShip,
        cMoveShip,
        cCreateTorpedo,
        cHitTorpedo,
        cMissTorpedo,
        cMoveTorpedo
    };

    int32_t packIndex(size_t sz)
    {
        if (sz == game::vcr::flak::Visualizer::NO_ENEMY) {
            return -1;
        } else {
            return static_cast<int32_t>(sz);
        }
    }

    bool unpackIndex(util::StringInstructionList::Iterator& it, size_t& out)
    {
        int32_t tmp;
        if (it.readParameter(tmp)) {
            if (tmp == -1) {
                out = game::vcr::flak::Visualizer::NO_ENEMY;
            } else {
                out = static_cast<int32_t>(tmp);
            }
            return true;
        } else {
            return false;
        }
    }

    bool unpackPosition(util::StringInstructionList::Iterator& it, game::vcr::flak::Position& pos)
    {
        int32_t x, y, z;
        if (it.readParameter(x) && it.readParameter(y) && it.readParameter(z)) {
            pos = game::vcr::flak::Position(x, y, z);
            return true;
        } else {
            return false;
        }
    }

    bool unpackBool(util::StringInstructionList::Iterator& it, bool& out)
    {
        int32_t tmp;
        if (it.readParameter(tmp)) {
            out = (tmp != 0);
            return true;
        } else {
            return false;
        }
    }
}

// Constructor.
game::vcr::flak::EventRecorder::EventRecorder()
    : m_content()
{ }

// Destructor.
game::vcr::flak::EventRecorder::~EventRecorder()
{ }

// Swap content.
void
game::vcr::flak::EventRecorder::swapContent(util::StringInstructionList& content)
{
    m_content.swap(content);
}

// Replay content.
void
game::vcr::flak::EventRecorder::replay(Visualizer& vis) const
{
    util::StringInstructionList::Instruction_t insn;
    util::StringInstructionList::Iterator it(m_content);
    while (it.readInstruction(insn)) {
        switch (Command(insn)) {
         case cUpdateTime: {
            int32_t t;
            if (it.readParameter(t)) {
                vis.updateTime(t);
            }
            break;
         }
         case cFireBeamFighterFighter: {
            Object_t from, to;
            bool hits;
            if (unpackIndex(it, from) && unpackIndex(it, to) && unpackBool(it, hits)) {
                vis.fireBeamFighterFighter(from, to, hits);
            }
            break;
         }
         case cFireBeamFighterShip: {
            Object_t from;
            Ship_t to;
            bool hits;
            if (unpackIndex(it, from) && unpackIndex(it, to) && unpackBool(it, hits)) {
                vis.fireBeamFighterShip(from, to, hits);
            }
            break;
         }
         case cFireBeamShipFighter: {
            Ship_t from;
            int32_t beamNr;
            Object_t to;
            bool hits;
            if (unpackIndex(it, from) && it.readParameter(beamNr) && unpackIndex(it, to) && unpackBool(it, hits)) {
                vis.fireBeamShipFighter(from, beamNr, to, hits);
            }
            break;
         }
         case cFireBeamShipShip: {
            Ship_t from;
            int32_t beamNr;
            Ship_t to;
            bool hits;
            if (unpackIndex(it, from) && it.readParameter(beamNr) && unpackIndex(it, to) && unpackBool(it, hits)) {
                vis.fireBeamShipShip(from, beamNr, to, hits);
            }
            break;
         }
         case cCreateFighter: {
            Object_t id;
            Position pos;
            int32_t player;
            Ship_t enemy;
            if (unpackIndex(it, id) && unpackPosition(it, pos) && it.readParameter(player) && unpackIndex(it, enemy)) {
                vis.createFighter(id, pos, player, enemy);
            }
            break;
         }
         case cKillFighter: {
            Object_t id;
            if (unpackIndex(it, id)) {
                vis.killFighter(id);
            }
            break;
         }
         case cLandFighter: {
            Object_t id;
            if (unpackIndex(it, id)) {
                vis.landFighter(id);
            }
            break;
         }
         case cMoveFighter: {
            Object_t id;
            Position pos;
            Ship_t to;
            if (unpackIndex(it, id) && unpackPosition(it, pos) && unpackIndex(it, to)) {
                vis.moveFighter(id, pos, to);
            }
            break;
         }
         case cCreateFleet: {
            Fleet_t fleetNr;
            int32_t x, y;
            int32_t player;
            Ship_t firstShip;
            size_t numShips;
            if (unpackIndex(it, fleetNr) && it.readParameter(x) && it.readParameter(y) && it.readParameter(player) && unpackIndex(it, firstShip) && unpackIndex(it, numShips)) {
                vis.createFleet(fleetNr, x, y, player, firstShip, numShips);
            }
            break;
         }
         case cSetEnemy: {
            Object_t id;
            Ship_t enemy;
            if (unpackIndex(it, id) && unpackIndex(it, enemy)) {
                vis.setEnemy(id, enemy);
            }
            break;
         }
         case cKillFleet: {
            Fleet_t fleetNr;
            if (unpackIndex(it, fleetNr)) {
                vis.killFleet(fleetNr);
            }
            break;
         }
         case cMoveFleet: {
            Fleet_t fleetNr;
            int32_t x, y;
            if (unpackIndex(it, fleetNr) && it.readParameter(x) && it.readParameter(y)) {
                vis.moveFleet(fleetNr, x, y);
            }
            break;
         }
         case cCreateShip: {
            Ship_t shipNr;
            Position pos;
            ShipInfo info;
            if (unpackIndex(it, shipNr)
                && unpackPosition(it, pos)
                && it.readStringParameter(info.name)
                && unpackBool(it, info.isPlanet)
                && it.readParameter(info.player)
                && it.readParameter(info.shield)
                && it.readParameter(info.damage)
                && it.readParameter(info.crew)
                && it.readParameter(info.numBeams)
                && it.readParameter(info.numLaunchers)
                && it.readParameter(info.numTorpedoes)
                && it.readParameter(info.numBays)
                && it.readParameter(info.numFighters)
                && it.readParameter(info.torpedoType)
                && it.readParameter(info.beamType)
                && it.readParameter(info.mass)
                && it.readParameter(info.id))
            {
                vis.createShip(shipNr, pos, info);
            }
            break;
         }
         case cKillShip: {
            Ship_t shipNr;
            if (unpackIndex(it, shipNr)) {
                vis.killShip(shipNr);
            }
            break;
         }
         case cMoveShip: {
            Ship_t shipNr;
            Position pos;
            if (unpackIndex(it, shipNr) && unpackPosition(it, pos)) {
                vis.moveShip(shipNr, pos);
            }
            break;
         }
         case cCreateTorpedo: {
            Object_t id;
            Position pos;
            int player;
            Ship_t enemy;
            if (unpackIndex(it, id) && unpackPosition(it, pos) && it.readParameter(player) && unpackIndex(it, enemy)) {
                vis.createTorpedo(id, pos, player, enemy);
            }
            break;
         }
         case cHitTorpedo: {
            Object_t id;
            Ship_t shipNr;
            if (unpackIndex(it, id) && unpackIndex(it, shipNr)) {
                vis.hitTorpedo(id, shipNr);
            }
            break;
         }
         case cMissTorpedo: {
            Object_t id;
            if (unpackIndex(it, id)) {
                vis.missTorpedo(id);
            }
            break;
         }
         case cMoveTorpedo: {
            Object_t id;
            Position pos;
            if (unpackIndex(it, id) && unpackPosition(it, pos)) {
                vis.moveTorpedo(id, pos);
            }
            break;
         }
        }
    }
}

// Get approximation of size of content.
size_t
game::vcr::flak::EventRecorder::size() const
{
    return m_content.size();
}

void
game::vcr::flak::EventRecorder::updateTime(int32_t time)
{
    m_content.addInstruction(cUpdateTime)
        .addParameter(time);
}

void
game::vcr::flak::EventRecorder::fireBeamFighterFighter(Object_t from, Object_t to, bool hits)
{
    m_content.addInstruction(cFireBeamFighterFighter)
        .addParameter(packIndex(from))
        .addParameter(packIndex(to))
        .addParameter(hits);
}

void
game::vcr::flak::EventRecorder::fireBeamFighterShip(Object_t from, Ship_t to, bool hits)
{
    m_content.addInstruction(cFireBeamFighterShip)
        .addParameter(packIndex(from))
        .addParameter(packIndex(to))
        .addParameter(hits);
}

void
game::vcr::flak::EventRecorder::fireBeamShipFighter(Ship_t from, int beamNr, Object_t to, bool hits)
{
    m_content.addInstruction(cFireBeamShipFighter)
        .addParameter(packIndex(from))
        .addParameter(beamNr)
        .addParameter(packIndex(to))
        .addParameter(hits);
}

void
game::vcr::flak::EventRecorder::fireBeamShipShip(Ship_t from, int beamNr, Ship_t to, bool hits)
{
    m_content.addInstruction(cFireBeamShipShip)
        .addParameter(packIndex(from))
        .addParameter(beamNr)
        .addParameter(packIndex(to))
        .addParameter(hits);
}

void
game::vcr::flak::EventRecorder::createFighter(Object_t id, const Position& pos, int player, Ship_t enemy)
{
    m_content.addInstruction(cCreateFighter)
        .addParameter(packIndex(id))
        .addParameter(pos.x).addParameter(pos.y).addParameter(pos.z)
        .addParameter(player)
        .addParameter(packIndex(enemy));
}

void
game::vcr::flak::EventRecorder::killFighter(Object_t id)
{
    m_content.addInstruction(cKillFighter)
        .addParameter(packIndex(id));
}

void
game::vcr::flak::EventRecorder::landFighter(Object_t id)
{
    m_content.addInstruction(cLandFighter)
        .addParameter(packIndex(id));
}

void
game::vcr::flak::EventRecorder::moveFighter(Object_t id, const Position& pos, Ship_t to)
{
    m_content.addInstruction(cMoveFighter)
        .addParameter(packIndex(id))
        .addParameter(pos.x).addParameter(pos.y).addParameter(pos.z)
        .addParameter(packIndex(to));
}

void
game::vcr::flak::EventRecorder::createFleet(Fleet_t fleetNr, int32_t x, int32_t y, int player, Ship_t firstShip, size_t numShips)
{
    m_content.addInstruction(cCreateFleet)
        .addParameter(packIndex(fleetNr))
        .addParameter(x).addParameter(y)
        .addParameter(player)
        .addParameter(packIndex(firstShip))
        .addParameter(packIndex(numShips));
}

void
game::vcr::flak::EventRecorder::setEnemy(Fleet_t fleetNr, Ship_t enemy)
{
    m_content.addInstruction(cSetEnemy)
        .addParameter(packIndex(fleetNr))
        .addParameter(packIndex(enemy));
}

void
game::vcr::flak::EventRecorder::killFleet(Fleet_t fleetNr)
{
    m_content.addInstruction(cKillFleet)
        .addParameter(packIndex(fleetNr));
}

void
game::vcr::flak::EventRecorder::moveFleet(Fleet_t fleetNr, int32_t x, int32_t y)
{
    m_content.addInstruction(cMoveFleet)
        .addParameter(packIndex(fleetNr))
        .addParameter(x).addParameter(y);
}

void
game::vcr::flak::EventRecorder::createShip(Ship_t shipNr, const Position& pos, const ShipInfo& info)
{
    m_content.addInstruction(cCreateShip)
        .addParameter(packIndex(shipNr))
        .addParameter(pos.x).addParameter(pos.y).addParameter(pos.z)
        .addStringParameter(info.name)
        .addParameter(info.isPlanet)
        .addParameter(info.player)
        .addParameter(info.shield)
        .addParameter(info.damage)
        .addParameter(info.crew)
        .addParameter(info.numBeams)
        .addParameter(info.numLaunchers)
        .addParameter(info.numTorpedoes)
        .addParameter(info.numBays)
        .addParameter(info.numFighters)
        .addParameter(info.torpedoType)
        .addParameter(info.beamType)
        .addParameter(info.mass)
        .addParameter(info.id);
}

void
game::vcr::flak::EventRecorder::killShip(Ship_t shipNr)
{
    m_content.addInstruction(cKillShip)
        .addParameter(packIndex(shipNr));
}

void
game::vcr::flak::EventRecorder::moveShip(Ship_t shipNr, const Position& pos)
{
    m_content.addInstruction(cMoveShip)
        .addParameter(packIndex(shipNr))
        .addParameter(pos.x).addParameter(pos.y).addParameter(pos.z);
}

void
game::vcr::flak::EventRecorder::createTorpedo(Object_t id, const Position& pos, int player, Ship_t enemy)
{
    m_content.addInstruction(cCreateTorpedo)
        .addParameter(packIndex(id))
        .addParameter(pos.x).addParameter(pos.y).addParameter(pos.z)
        .addParameter(player)
        .addParameter(packIndex(enemy));
}

void
game::vcr::flak::EventRecorder::hitTorpedo(Object_t id, Ship_t shipNr)
{
    m_content.addInstruction(cHitTorpedo)
        .addParameter(packIndex(id))
        .addParameter(packIndex(shipNr));
}

void
game::vcr::flak::EventRecorder::missTorpedo(Object_t id)
{
    m_content.addInstruction(cMissTorpedo)
        .addParameter(packIndex(id));
}

void
game::vcr::flak::EventRecorder::moveTorpedo(Object_t id, const Position& pos)
{
    m_content.addInstruction(cMoveTorpedo)
        .addParameter(packIndex(id))
        .addParameter(pos.x).addParameter(pos.y).addParameter(pos.z);
}

