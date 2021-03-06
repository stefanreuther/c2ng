/**
  *  \file game/map/explosion.cpp
  *  \brief Class game::map::Explosion
  */

#include "game/map/explosion.hpp"
#include "afl/string/format.hpp"

game::map::Explosion::Explosion(Id_t id, Point pos)
    : Object(),
      m_id(id),
      m_position(pos),
      m_shipName(),
      m_shipId(0)
{
    // ex GExplosion::GExplosion
}

game::map::Explosion::Explosion(const Explosion& ex)
    : Object(),
      m_id(ex.m_id),
      m_position(ex.m_position),
      m_shipName(ex.m_shipName),
      m_shipId(ex.m_shipId)
{ }

game::map::Explosion::~Explosion()
{ }

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

bool
game::map::Explosion::getPosition(Point& result) const
{
    // ex GExplosion::getPos
    result = m_position;
    return true;
}

String_t
game::map::Explosion::getShipName() const
{
    // ex GExplosion::getShipName
    return m_shipName;
}

game::Id_t
game::map::Explosion::getShipId() const
{
    // ex GExplosion::getShipId
    return m_shipId;
}

void
game::map::Explosion::setShipName(String_t name)
{
    // ex GExplosion::setShipName
    if (name != m_shipName) {
        m_shipName = name;
        markDirty();
    }
}

void
game::map::Explosion::setShipId(Id_t id)
{
    // ex GExplosion::setShipId
    if (id != m_shipId) {
        m_shipId = id;
        markDirty();
    }
}

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

    /* different Ids? */
    if (m_shipId != 0 && other.m_shipId != 0 && m_shipId != other.m_shipId) {
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
