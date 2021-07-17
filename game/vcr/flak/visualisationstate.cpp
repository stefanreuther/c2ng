/**
  *  \file game/vcr/flak/visualisationstate.cpp
  *  \brief Class game::vcr::flak::VisualisationState
  */

#include <cmath>
#include "game/vcr/flak/visualisationstate.hpp"
#include "util/math.hpp"

namespace {
    const int SMOKE_SIZE = 200;
    const int SMOKE_SIZE_HALF = 100;
    const int ZSCALE = 25;

    template<typename T>
    T& makeSlot(std::vector<T>& vec, size_t slot)
    {
        while (vec.size() <= slot) {
            vec.push_back(T());
        }
        return vec[slot];
    }

    template<typename T>
    T* getSlot(std::vector<T>& vec, size_t slot)
    {
        if (slot < vec.size()) {
            return &vec[slot];
        } else {
            return 0;
        }
    }

    float getAngle(const game::vcr::flak::Position& a, const game::vcr::flak::Position& b)
    {
        if (a.x == b.x && a.y == b.y) {
            return 0;
        } else {
            return float(std::atan2(b.y - a.y, b.x - a.x));
        }
    }

    float updateAngle(float current, float target, float speed) {
        float delta = current - target;
        if (delta < -util::PI) {
            delta += float(2*util::PI);
        }
        if (delta > util::PI) {
            delta -= float(2*util::PI);
        }
        if (std::abs(delta) < speed) {
            return target;
        } else if (delta < 0) {
            return current + speed;
        } else {
            return current - speed;
        }
    }
}

game::vcr::flak::VisualisationState::VisualisationState()
    : m_objects(), m_ships(), m_fleets(), m_smoke(), m_beams(),
      m_rng(0),
      m_maxBeamAge(5), m_maxSmokeAge(10), m_gridSize(2000),
      m_time(0)
{ }

game::vcr::flak::VisualisationState::~VisualisationState()
{ }

afl::base::Memory<const game::vcr::flak::VisualisationState::Object>
game::vcr::flak::VisualisationState::objects() const
{
    return m_objects;
}

afl::base::Memory<const game::vcr::flak::VisualisationState::Ship>
game::vcr::flak::VisualisationState::ships() const
{
    return m_ships;
}

afl::base::Memory<const game::vcr::flak::VisualisationState::Fleet>
game::vcr::flak::VisualisationState::fleets() const
{
    return m_fleets;
}

afl::base::Memory<const game::vcr::flak::VisualisationState::Smoke>
game::vcr::flak::VisualisationState::smoke() const
{
    return m_smoke;
}

afl::base::Memory<const game::vcr::flak::VisualisationState::Beam>
game::vcr::flak::VisualisationState::beams() const
{
    return m_beams;
}

int32_t
game::vcr::flak::VisualisationState::getTime() const
{
    return m_time;
}

bool
game::vcr::flak::VisualisationState::animate()
{
    // ex Client.FlakVis.animate

    // Update ship angles
    for (Ship_t i = 0; i < m_ships.size(); ++i) {
        Ship& sh = m_ships[i];
        if (sh.isAlive && !sh.isPlanet) {
            if (Ship* ene = getSlot(m_ships, sh.enemy)) {
                sh.heading = updateAngle(sh.heading, getAngle(sh.pos, ene->pos), 0.1f);
            }
        }
    }

    // Update beams
    {
        size_t out = 0;
        for (size_t in = 0; in < m_beams.size(); ++in) {
            if (++m_beams[in].age < m_maxBeamAge) {
                m_beams[out++] = m_beams[in];
            }
        }
        while (m_beams.size() > out) {
            m_beams.pop_back();
        }
    }

    // Update smoke/explosions
    {
        size_t out = 0;
        for (size_t in = 0; in < m_smoke.size(); ++in) {
            if (++m_smoke[in].age < m_maxSmokeAge) {
                Smoke& s = m_smoke[in];
                s.pos.x += s.dx;
                s.pos.y += s.dy;
                s.pos.z += s.dz;
                m_smoke[out++] = s;
            }
        }
        while (m_smoke.size() > out) {
            m_smoke.pop_back();
        }
    }

    // Return true to keep playing animations (slightly different than JS version).
    return !m_beams.empty() || !m_smoke.empty();
}

float
game::vcr::flak::VisualisationState::getArenaSize() const
{
    // ex fvGetArenaSize
    int32_t size = 2000*2000;
    for (Fleet_t i = 0; i < m_fleets.size(); ++i) {
        const Fleet& f = m_fleets[i];
        if (f.isAlive) {
            size = std::max(size, util::squareInteger(f.x) + util::squareInteger(f.y));
        }
    }
    return std::sqrt(float(size));
}

int32_t
game::vcr::flak::VisualisationState::getGridSize() const
{
    return m_gridSize;
}

void
game::vcr::flak::VisualisationState::setMaxBeamAge(int n)
{
    m_maxBeamAge = n;
}

void
game::vcr::flak::VisualisationState::setMaxSmokeAge(int n)
{
    m_maxSmokeAge = n;
}

void
game::vcr::flak::VisualisationState::updateTime(int32_t time)
{
    m_time = time;
}

void
game::vcr::flak::VisualisationState::fireBeamFighterFighter(Object_t from, Object_t to, bool /*hits*/)
{
    // ex ACTION_CALLOUTS.beamff
    const Object* f = getSlot(m_objects, from);
    const Object* t = getSlot(m_objects, to);
    if (f != 0 && t != 0) {
        addBeam(f->pos, t->pos);
    }
}

void
game::vcr::flak::VisualisationState::fireBeamFighterShip(Object_t from, Ship_t to, bool /*hits*/)
{
    // ex ACTION_CALLOUTS.beamfs
    const Object* f = getSlot(m_objects, from);
    const Ship* t = getSlot(m_ships, to);
    if (f != 0 && t != 0) {
        addBeam(f->pos, t->pos);
    }
}

void
game::vcr::flak::VisualisationState::fireBeamShipFighter(Ship_t from, int /*beamNr*/, Object_t to, bool /*hits*/)
{
    // ex ACTION_CALLOUTS.beamsf
    const Ship* f = getSlot(m_ships, from);
    const Object* t = getSlot(m_objects, to);
    if (f != 0 && t != 0) {
        addBeam(f->pos, t->pos);
    }
}

void
game::vcr::flak::VisualisationState::fireBeamShipShip(Ship_t from, int /*beamNr*/, Ship_t to, bool /*hits*/)
{
    // ex ACTION_CALLOUTS.beamss
    const Ship* f = getSlot(m_ships, from);
    const Ship* t = getSlot(m_ships, to);
    if (f != 0 && t != 0) {
        addBeam(f->pos, t->pos);
    }
}

void
game::vcr::flak::VisualisationState::createFighter(Object_t id, const Position& pos, int player, Ship_t enemy)
{
    // ex ACTION_CALLOUTS.fnew
    Object& obj = makeSlot(m_objects, id);
    obj.type = FighterObject;
    obj.pos = pos;
    obj.pos.z *= ZSCALE;
    obj.player = player;
    obj.xRotation = 0;
    obj.yRotation = 0;

    const Ship* ene = getSlot(m_ships, enemy);
    if (ene != 0) {
        obj.heading = getAngle(pos, ene->pos);
    } else {
        obj.heading = 0;
    }
}

void
game::vcr::flak::VisualisationState::killFighter(Object_t id)
{
    // ex ACTION_CALLOUTS.fkill
    if (Object* obj = getSlot(m_objects, id)) {
        addSmoke(obj->pos, 5, 0);
        obj->type = NoObject;
    }
}

void
game::vcr::flak::VisualisationState::landFighter(Object_t id)
{
    // ex ACTION_CALLOUTS.fland
    if (Object* obj = getSlot(m_objects, id)) {
        obj->type = NoObject;
    }
}

void
game::vcr::flak::VisualisationState::moveFighter(Object_t id, const Position& pos, Ship_t to)
{
    // ex ACTION_CALLOUTS.fmove
    if (Object* obj = getSlot(m_objects, id)) {
        obj->pos = pos;
        obj->pos.z *= ZSCALE;
        if (const Ship* ene = getSlot(m_ships, to)) {
            // For now, turn fighters immediately; it looks better this way.
            // Turning them smoothly means a fighter spends most of its time returning, turning.
            // As an excuse, fighters are much more maneuverable than big ships.
            obj->heading = getAngle(pos, ene->pos);
        }
    }
}

void
game::vcr::flak::VisualisationState::createFleet(Fleet_t fleetNr, int32_t x, int32_t y, int player, Ship_t firstShip, size_t numShips)
{
    // ex ACTION_CALLOUTS.gnew
    Fleet& fl = makeSlot(m_fleets, fleetNr);
    fl.player = player;
    fl.firstShip = firstShip;
    fl.numShips = numShips;
    fl.isAlive = true;
    fl.x = x;
    fl.y = y;
    fl.enemy = NO_ENEMY;

    m_gridSize = std::max(std::abs(x), std::max(std::abs(y), m_gridSize));
}

void
game::vcr::flak::VisualisationState::setEnemy(Fleet_t fleetNr, Ship_t enemy)
{
    // ex ACTION_CALLOUTS.genemy
    if (Fleet* fl = getSlot(m_fleets, fleetNr)) {
        fl->enemy = enemy;
        for (size_t i = 0; i < fl->numShips; ++i) {
            if (Ship* sh = getSlot(m_ships, fl->firstShip + i)) {
                sh->enemy = enemy;
            }
        }
    }
}

void
game::vcr::flak::VisualisationState::killFleet(Fleet_t fleetNr)
{
    // ex ACTION_CALLOUTS.gkill
    if (Fleet* fl = getSlot(m_fleets, fleetNr)) {
        fl->isAlive = false;
    }
}

void
game::vcr::flak::VisualisationState::moveFleet(Fleet_t fleetNr, int32_t x, int32_t y)
{
    // ex ACTION_CALLOUTS.gmove
    if (Fleet* fl = getSlot(m_fleets, fleetNr)) {
        fl->x = x;
        fl->y = y;
    }
}

void
game::vcr::flak::VisualisationState::createShip(Ship_t shipNr, const Position& pos, const ShipInfo& info)
{
    // ex ACTION_CALLOUTS.snew
    Ship& sh = makeSlot(m_ships, shipNr);

    static_cast<Visualizer::ShipInfo&>(sh) = info;
    sh.isAlive  = true;
    sh.heading  = getAngle(pos, Position());
    sh.pos      = pos;
    sh.pos.z   *= ZSCALE;
    sh.enemy    = NO_ENEMY;
}

void
game::vcr::flak::VisualisationState::killShip(Ship_t shipNr)
{
    // ex ACTION_CALLOUTS.skill
    if (Ship* sh = getSlot(m_ships, shipNr)) {
        addSmoke(sh->pos, 25, 0);
        sh->isAlive = false;
    }
}

void
game::vcr::flak::VisualisationState::moveShip(Ship_t shipNr, const Position& pos)
{
    // ex ACTION_CALLOUTS.smove
    if (Ship* sh = getSlot(m_ships, shipNr)) {
        sh->pos = pos;
        sh->pos.z *= ZSCALE;
    }
}

void
game::vcr::flak::VisualisationState::createTorpedo(Object_t id, const Position& pos, int player, Ship_t /*enemy*/)
{
    // ex ACTION_CALLOUTS.tnew
    Object& obj = makeSlot(m_objects, id);
    obj.type = TorpedoObject;
    obj.pos = pos;
    obj.pos.z *= ZSCALE;
    obj.player = player;
    obj.heading = 0;            // not relevant
    obj.xRotation = m_rng(256);
    obj.yRotation = m_rng(256);
}

void
game::vcr::flak::VisualisationState::hitTorpedo(Object_t id, Ship_t /*shipNr*/)
{
    // ex ACTION_CALLOUTS.thit
    if (Object* obj = getSlot(m_objects, id)) {
        obj->type = NoObject;
    }
}

void
game::vcr::flak::VisualisationState::missTorpedo(Object_t id)
{
    // ex ACTION_CALLOUTS.tmiss
    if (Object* obj = getSlot(m_objects, id)) {
        obj->type = NoObject;
    }
}

void
game::vcr::flak::VisualisationState::moveTorpedo(Object_t id, const Position& pos)
{
    // ex ACTION_CALLOUTS.tmove
    if (Object* obj = getSlot(m_objects, id)) {
        obj->pos = pos;
        obj->pos.z *= ZSCALE;
    }
}

void
game::vcr::flak::VisualisationState::addSmoke(const Position& pos, int n, int age)
{
    // ex fvAddSmoke
    for (int i = 0; i < n; ++i) {
        int dx = m_rng(SMOKE_SIZE) - SMOKE_SIZE_HALF,
            dy = m_rng(SMOKE_SIZE) - SMOKE_SIZE_HALF,
            dz = m_rng(SMOKE_SIZE) - SMOKE_SIZE_HALF;
        m_smoke.push_back(Smoke(pos, dx, dy, dz, age));
    }
}

void
game::vcr::flak::VisualisationState::addBeam(const Position& from, const Position& to)
{
    // ex fvAddBeam
    m_beams.push_back(Beam(from, to, 0));
}
