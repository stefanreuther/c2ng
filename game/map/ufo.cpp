/**
  *  \file game/map/ufo.cpp
  *
  *  PCC2 Comment:
  *
  *  Ufo loading mechanism: the most important thing Ufos are used for
  *  wormholes. We handle these specially. Other than that, UFO.HST
  *  Ufos and PHost's General Objects (GOs) all end up here. We ought
  *  to identify an object by its Id/Type-Code combination as we
  *  cannot assume that all add-ons that generate GOs coordinate on an
  *  Id range. This isn't yet implemented, though, making PCC2 behave
  *  identical to PCC 1.x. In any case, Ufos with an Id in the range
  *  of 1..1000 need not have the same type code, and multiple Ufos
  *  with the same Id/Type are not supported either.
  *
  *  For Ufos and GOs, merging is trivial. We assume both data sources
  *  contain equivalent information; the last seen instance survives.
  *
  *  For wormholes, we have three possible sources:
  *  - the Ufo from KORE.DAT
  *  - the UTIL.DAT entry
  *  - the WORMHOLE.TXT file
  *  Unfortunately, there is no 1:1 mapping between wormhole Ids and
  *  Ufo Ids (each WH consumes two WH Id slots, but whereas a
  *  bidirectional WH consumes two Ufo slots, an unidirectional one
  *  consumes only one). We therefore queue all UTIL.DAT wormholes
  *  first, and merge them later upon postprocess() time. In the
  *  single player case, we can simply match the Ufo and wormhole
  *  sequences: if KORE.DAT contains Ufos 51,53,54, and UTIL.DAT
  *  contains Ufos 0,6,7, we can therefore derive the mapping. It's a
  *  little harder if we have multiple players with different
  *  registration status.
  *
  *  Finally, WORMHOLE.TXT can be used to "fill in the blanks", mainly
  *  intended to be used in games where wormholes are static and known
  *  to everyone in the game.
  */

#include "game/map/ufo.hpp"
#include "afl/string/format.hpp"
#include "util/math.hpp"

game::map::Ufo::Ufo(Id_t id)
    : m_id(id),
      m_colorCode(),
      m_position(),
      m_speed(),
      m_heading(),
      m_planetRange(),
      m_shipRange(),
      m_radius(),
      m_typeCode(),
      m_name(),
      m_info1(),
      m_info2(),
      m_realId(0),
      m_turnLastSeen(0),
      m_posLastSeen(),
      m_movementVector(),
      m_flags(),
      m_otherEnd(0)
{
    // ex GUfo::GUfo
}

game::map::Ufo::~Ufo()
{
    // ex GUfo::~GUfo
    disconnect();
}

// Object:
String_t
game::map::Ufo::getName(Name which, afl::string::Translator& tx, InterpreterInterface& /*iface*/) const
{
    // ex GUfo::getName
    switch (which) {
     case PlainName:
        return m_name;
     case LongName:
     case DetailedName:
        return afl::string::Format(tx.translateString("Ufo #%d: %s").c_str(), m_id, m_name);
    }
    return String_t();
}

game::Id_t
game::map::Ufo::getId() const
{
    // ex GUfo::getId
    return m_id;
}

bool
game::map::Ufo::getOwner(int& result) const
{
    // ex GUfo::getOwner
    result = 0;
    return true;
}

// MapObject:
bool
game::map::Ufo::getPosition(Point& result) const
{
    // ex GUfo::getPos
    if (isValid()) {
        result = m_position;
        return true;
    } else {
        return false;
    }
}


// CircularObject:
bool
game::map::Ufo::getRadius(int& result) const
{
    // ex GUfo::getRadius
    if (isValid()) {
        return m_radius.get(result);
    } else {
        return false;
    }
}

bool
game::map::Ufo::getRadiusSquared(int32_t& result) const
{
    // ex GUfo::getRadiusSquared
    int r;
    if (getRadius(r)) {
        result = util::squareInteger(r);
        return true;
    } else {
        return false;
    }
}

bool
game::map::Ufo::isValid() const
{
    // ex GUfo::isExistant
    return m_colorCode != 0;
}

int
game::map::Ufo::getColorCode() const
{
    return m_colorCode;
}

void
game::map::Ufo::setColorCode(int n)
{
    if (m_colorCode != n) {
        m_colorCode = n;
        markDirty();
    }
}

game::IntegerProperty_t
game::map::Ufo::getSpeed() const
{
    // ex GUfo::getSpeed
    return m_speed;
}

void
game::map::Ufo::setSpeed(IntegerProperty_t speed)
{
    if (!speed.isSame(m_speed)) {
        m_speed = speed;
        markDirty();
    }
}

game::IntegerProperty_t
game::map::Ufo::getHeading() const
{
    // ex GUfo::getHeading
    return m_heading;
}

void
game::map::Ufo::setHeading(IntegerProperty_t heading)
{
    if (!heading.isSame(m_heading)) {
        m_heading = heading;
        markDirty();
    }
}

game::IntegerProperty_t
game::map::Ufo::getPlanetRange() const
{
    // ex GUfo::getPlanetRange
    return m_planetRange;
}

void
game::map::Ufo::setPlanetRange(IntegerProperty_t range)
{
    if (!range.isSame(m_planetRange)) {
        m_planetRange = range;
        markDirty();
    }
}

game::IntegerProperty_t
game::map::Ufo::getShipRange() const
{
    // ex GUfo::getShipRange
    return m_shipRange;
}

void
game::map::Ufo::setShipRange(IntegerProperty_t range)
{
    if (!range.isSame(m_shipRange)) {
        m_shipRange = range;
        markDirty();
    }
}

game::IntegerProperty_t
game::map::Ufo::getTypeCode() const
{
    // ex GUfo::getTypeCode
    return m_typeCode;
}

void
game::map::Ufo::setTypeCode(IntegerProperty_t typeCode)
{
    if (!typeCode.isSame(m_typeCode)) {
        m_typeCode = typeCode;
        markDirty();
    }
}

String_t
game::map::Ufo::getInfo1() const
{
    // ex GUfo::getInfo1
    return m_info1;
}

void
game::map::Ufo::setInfo1(String_t info)
{
    if (info != m_info1) {
        m_info1 = info;
        markDirty();
    }
}

String_t
game::map::Ufo::getInfo2() const
{
    // ex GUfo::getInfo2
    return m_info2;
}

void
game::map::Ufo::setInfo2(String_t info)
{
    if (info != m_info2) {
        m_info2 = info;
        markDirty();
    }
}

int32_t
game::map::Ufo::getRealId() const
{
    // ex GUfo::getRealId
    return m_realId;
}

void
game::map::Ufo::setRealId(int32_t id)
{
    // ex GUfo::setRealId
    if (id != m_realId) {
        m_realId = id;
        markDirty();
    }
}

void
game::map::Ufo::setName(String_t name)
{
    if (name != m_name) {
        m_name = name;
        markDirty();
    }
}

void
game::map::Ufo::setPosition(Point pt)
{
    if (pt != m_position) {
        m_position = pt;
        markDirty();
    }
}

void
game::map::Ufo::setRadius(IntegerProperty_t r)
{
    if (!r.isSame(m_radius)) {
        m_radius = r;
        markDirty();
    }
}

int
game::map::Ufo::getLastTurn() const
{
    // ex GUfo::getLastTurn
    return m_turnLastSeen;
}

void
game::map::Ufo::setLastTurn(int n)
{
    if (m_turnLastSeen != n) {
        m_turnLastSeen = n;
        markDirty();
    }
}

game::map::Point
game::map::Ufo::getMovementVector() const
{
    // ex GUfo::getMovementVector
    return m_movementVector;
}

void
game::map::Ufo::setMovementVector(Point vec)
{
    // ex GUfo::setMovementVector
    // FIXME: PCC 1.x updates speed and heading when a movement vector is set.
    // It also fakes speed and heading for wormholes, but not consistently.
    // Make up some rules.
    if (vec != m_movementVector) {
        m_movementVector = vec;
        markDirty();
    }
}

// /** Disconnect this Ufo from its other end. */
void
game::map::Ufo::disconnect()
{
    // ex GUfo::disconnect
    if (m_otherEnd != 0) {
        m_otherEnd->m_otherEnd = 0;
        m_otherEnd = 0;
    }
}

// /** Connect this Ufo with another one. Used to establish wormhole end links. */
void
game::map::Ufo::connectWith(Ufo& other)
{
    // ex GUfo::connectWith
    disconnect();
    other.disconnect();
    m_otherEnd = &other;
    other.m_otherEnd = this;
}

game::map::Ufo*
game::map::Ufo::getOtherEnd() const
{
    // ex GUfo::getOtherEnd
    return m_otherEnd;
}

// /** Postprocess after loading. */
void
game::map::Ufo::postprocess(int turn)
{
    // ex GUfo::postprocess
    if (!isSeenThisTurn() && m_turnLastSeen > 0) {
        // Ufo from database, not seen this turn. Estimate movement.
        m_position = Point(m_posLastSeen.getX() + m_movementVector.getX() * (turn - m_turnLastSeen),
                           m_posLastSeen.getY() + m_movementVector.getY() * (turn - m_turnLastSeen));
    }
    if (isSeenThisTurn() && m_turnLastSeen < turn) {
        // Ufo was seen, and previous sighting was earlier.
        m_turnLastSeen = turn;
        m_posLastSeen = m_position;
    }
}

bool
game::map::Ufo::isStoredInHistory() const
{
    // ex GUfo::isStoredInHistory
    return m_flags.contains(UfoStoredInDatabase);
}

void
game::map::Ufo::setIsStoredInHistory(bool value)
{
    // ex GUfo::setIsStoredInHistory
    if (value != isStoredInHistory()) {
        m_flags ^= UfoStoredInDatabase;
        markDirty();
    }
}

bool
game::map::Ufo::isSeenThisTurn() const
{
    // ex GUfo::isSeenThisTurn
    return m_flags.contains(UfoSeenThisTurn);
}

void
game::map::Ufo::setIsSeenThisTurn(bool value)
{
    if (value != isSeenThisTurn()) {
        m_flags ^= UfoSeenThisTurn;
        markDirty();
    }
}
