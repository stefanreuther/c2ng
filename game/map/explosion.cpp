/**
  *  \file game/map/explosion.cpp
  *
  *  PCC2 Comment:
  *
  *  In PCC2, explosions are a separate type. In PCC 1.x, they were
  *  markers with a special tag. Implementing them as separate type
  *  allows to attach regular information in a more meaningful way
  *  than by recycling and abusing drawings. The disadvantage is that
  *  we need to process two info sources when iterating over markers
  *  and explosions. This will mainly become an issue when we want to
  *  provide a uniform interface for scripts.
  *
  *  \todo make GExplosion and GExplosionContainer implement GObject
  *  and GObjectType, respectively.
  */

#include "game/map/explosion.hpp"
#include "afl/string/format.hpp"

// /** Create explosion.
//     \param id  Explosion Id (NOT ship Id!), 0 if not known.
//     \param pos Explosion position. */
game::map::Explosion::Explosion(Id_t id, Point pos)
    : m_id(id),
      m_position(pos),
      m_shipName(),
      m_shipId(0)
{
    // ex GExplosion::GExplosion
}

game::map::Explosion::Explosion(const Explosion& ex)
    : m_id(ex.m_id),
      m_position(ex.m_position),
      m_shipName(ex.m_shipName),
      m_shipId(ex.m_shipId)
{ }

game::map::Explosion::~Explosion()
{ }

// MapObject:
// /** Get explosion position. */
bool
game::map::Explosion::getPosition(Point& result) const
{
    // ex GExplosion::getPos
    result = m_position;
    return true;
}


// Object:
String_t
game::map::Explosion::getName(ObjectName /*which*/, afl::string::Translator& tx, InterpreterInterface& /*iface*/) const
{
    if (!m_shipName.empty()) {
        return afl::string::Format(tx.translateString("Explosion of %s%!d%!0{ (#%1$d)%}").c_str(), m_shipName, m_shipId);
    } else if (m_shipId != 0) {
        return afl::string::Format(tx.translateString("Explosion of ship #%d").c_str(), m_shipId);
    } else {
        return "Explosion";
    }
}

// /** Get explosion Id. This is the sequence number of the explosion
//     when dealing with Winplan RSTs. */
game::Id_t
game::map::Explosion::getId() const
{
    // GExplosion::getId
    return m_id;
}

bool
game::map::Explosion::getOwner(int& result) const
{
    // Explosions always report unowned
    result = 0;
    return true;
}

// Explosion:
// /** Get name of ship that exploded here. Empty if unknown. */
String_t
game::map::Explosion::getShipName() const
{
    // ex GExplosion::getShipName
    return m_shipName;
}

// /** Get Id of ship that exploded here. 0 if unknown. */
game::Id_t
game::map::Explosion::getShipId() const
{
    // ex GExplosion::getShipId
    return m_shipId;
}

// /** Set name of ship that exploded here. */
void
game::map::Explosion::setShipName(String_t name)
{
    // ex GExplosion::setShipName
    if (name != m_shipName) {
        m_shipName = name;
        markDirty();
    }
}

// /** Set Id of ship that exploded here. */
void
game::map::Explosion::setShipId(Id_t id)
{
    // ex GExplosion::setShipId
    if (id != m_shipId) {
        m_shipId = id;
        markDirty();
    }
}

// /** Merge information of other explosion record. This tests whether
//     these records potentially describe the same explosion and, if yes,
//     merges them.
//     \param other other explosion record
//     \return true if merge successful */
bool
game::map::Explosion::merge(const Explosion& other)
{
    // ex GExplosion::merge
    /* not same position? Cannot match. */
    if (m_position != other.m_position) {
        return false;
    }

    /* different explosion Id? */
    if (m_id != 0 && other.m_id != 0 && m_id != other.m_id) {
        return false;
    }

    /* different ship name? */
    // FIXME: deal with dummy ship names produced by AllowShipNames=No
    if (m_shipName.size() && other.m_shipName.size() && m_shipName != other.m_shipName) {
        return false;
    }

    /* If we have a name, the ship Ids must match. */
    if (other.m_shipName.size()) {
        if (m_shipId != other.m_shipId) {
            return false;
        }
    } else {
        if (m_shipId != 0 && other.m_shipId != 0 && m_shipId != other.m_shipId)
            return false;
    }

    /* ok, it will work. do it. */
    if (other.m_id != 0) {
        m_id = other.m_id;
    }
    if (other.m_shipName.size()) {
        m_shipName = other.m_shipName;
        m_shipId   = other.m_shipId;
    } else if (other.m_shipId) {
        m_shipId   = other.m_shipId;
    } else {
        // nothing copied
    }
    markDirty();
    return true;
}



// FIXME: retire (not needed?)
// /** Set explosion position. */
// inline void
// GExplosion::setPos(GPoint pos)
// {
//     this->pos = pos;
// }
