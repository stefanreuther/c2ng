/**
  *  \file game/map/ufo.cpp
  *  \brief Class game::map::Ufo
  */

#include "game/map/ufo.hpp"
#include "afl/string/format.hpp"
#include "game/map/configuration.hpp"
#include "util/math.hpp"

game::map::Ufo::Ufo(Id_t id)
    : CircularObject(id),
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
game::map::Ufo::getName(ObjectName which, afl::string::Translator& tx, const InterpreterInterface& /*iface*/) const
{
    // ex GUfo::getName
    switch (which) {
     case PlainName:
        return m_name;
     case LongName:
     case DetailedName:
        return afl::string::Format(tx.translateString("Ufo #%d: %s").c_str(), getId(), m_name);
    }
    return String_t();
}

afl::base::Optional<int>
game::map::Ufo::getOwner() const
{
    // ex GUfo::getOwner
    return 0;
}

afl::base::Optional<game::map::Point>
game::map::Ufo::getPosition() const
{
    // ex GUfo::getPos
    if (isValid()) {
        return m_position;
    } else {
        return afl::base::Nothing;
    }
}


// CircularObject:
afl::base::Optional<int>
game::map::Ufo::getRadius() const
{
    // ex GUfo::getRadius
    if (isValid()) {
        return m_radius;
    } else {
        return afl::base::Nothing;
    }
}

afl::base::Optional<int32_t>
game::map::Ufo::getRadiusSquared() const
{
    // ex GUfo::getRadiusSquared
    int r;
    if (getRadius().get(r)) {
        return util::squareInteger(r);
    } else {
        return afl::base::Nothing;
    }
}

bool
game::map::Ufo::isValid() const
{
    // ex GUfo::isExistant
    return m_colorCode != 0;
}

const String_t&
game::map::Ufo::getName() const
{
    return m_name;
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
game::map::Ufo::getWarpFactor() const
{
    // ex GUfo::getSpeed
    return m_speed;
}

void
game::map::Ufo::setWarpFactor(IntegerProperty_t speed)
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

String_t
game::map::Ufo::getPlainName() const
{
    return m_name;
}

game::map::Point
game::map::Ufo::getLastPosition() const
{
    return m_posLastSeen;
}

int
game::map::Ufo::getLastTurn() const
{
    // ex GUfo::getLastTurn
    return m_turnLastSeen;
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
    if (vec != m_movementVector) {
        m_movementVector = vec;
        markDirty();
    }
}

void
game::map::Ufo::disconnect()
{
    // ex GUfo::disconnect
    if (m_otherEnd != 0) {
        m_otherEnd->m_otherEnd = 0;
        m_otherEnd = 0;
    }
}

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

void
game::map::Ufo::addMessageInformation(const game::parser::MessageInformation& info)
{
    // ex GUfo::addHistoryData (sort-of), GUfo::addWormholeData, GUfo::addObjectData
    namespace gp = game::parser;
    assert(info.getObjectId() == getId());
    if (info.getTurnNumber() >= m_turnLastSeen) {
        // FIXME: limit to !isSeenThisTurn()?
        // FIXME: some cleverer merging (accept old value if existing value is unknown? does this happen?)

        m_turnLastSeen = info.getTurnNumber();

        // -- Scalars --
        // Real ID
        int32_t iv;
        if (info.getValue(gp::mi_UfoRealId).get(iv)) {
            m_realId = iv;
        }

        // Color
        if (info.getValue(gp::mi_Color).get(iv)) {
            m_colorCode = iv;
        }

        // Speed
        if (info.getValue(gp::mi_WarpFactor).get(iv)) {
            m_speed = iv;
        }

        // Heading
        if (info.getValue(gp::mi_Heading).get(iv)) {
            m_heading = iv;
        }

        // Ranges
        if (info.getValue(gp::mi_UfoShipRange).get(iv)) {
            m_shipRange = iv;
        }
        if (info.getValue(gp::mi_UfoPlanetRange).get(iv)) {
            m_planetRange = iv;
        }

        // Radius
        if (info.getValue(gp::mi_Radius).get(iv)) {
            m_radius = iv;
        }

        // Type
        if (info.getValue(gp::mi_Type).get(iv)) {
            m_typeCode = iv;
        }

        // -- Strings --
        // (parse directly into subject variables; no type conversion needed)
        info.getValue(gp::ms_Name).get(m_name);
        info.getValue(gp::ms_UfoInfo1).get(m_info1);
        info.getValue(gp::ms_UfoInfo2).get(m_info2);

        // -- Pairs --
        int32_t x, y;
        if (info.getValue(gp::mi_X).get(x) && info.getValue(gp::mi_Y).get(y)) {
            m_position = m_posLastSeen = Point(x, y);
        }
        if (info.getValue(gp::mi_UfoSpeedX).get(x) && info.getValue(gp::mi_UfoSpeedY).get(y)) {
            m_movementVector = Point(x, y);
        }
    }
}

void
game::map::Ufo::postprocess(int turn, const Configuration& mapConfig)
{
    // ex GUfo::postprocess, ccmain.pas:UpdateUfoMemory
    if (!isSeenThisTurn() && m_turnLastSeen > 0) {
        // Ufo from database, not seen this turn. Estimate movement.
        m_position = mapConfig.getCanonicalLocation(Point(m_posLastSeen.getX() + m_movementVector.getX() * (turn - m_turnLastSeen),
                                                          m_posLastSeen.getY() + m_movementVector.getY() * (turn - m_turnLastSeen)));
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
