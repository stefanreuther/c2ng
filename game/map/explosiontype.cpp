/**
  *  \file game/map/explosiontype.cpp
  *  \brief Class game::map::ExplosionType
  */

#include "game/map/explosiontype.hpp"

game::map::ExplosionType::ExplosionType()
    : m_explosions()
{ }

game::map::ExplosionType::~ExplosionType()
{ }

void
game::map::ExplosionType::add(const Explosion& ex)
{
    // ex GExplosionContainer::addExplosion
    for (size_t i = 0, n = m_explosions.size(); i < n; ++i) {
        if (m_explosions[i]->merge(ex)) {
            return;
        }
    }
    m_explosions.pushBackNew(new Explosion(ex));
}

void
game::map::ExplosionType::addMessageInformation(const game::parser::MessageInformation& info)
{
    namespace gp = game::parser;
    int32_t x, y;
    if (info.getValue(gp::mi_X).get(x) && info.getValue(gp::mi_Y).get(y)) {
        // Minimum values are X, Y.
        Explosion e(info.getObjectId(), Point(x, y));

        // Try to get ship information
        int32_t shipId;
        if (info.getValue(gp::mi_ExplodedShipId).get(shipId)) {
            e.setShipId(shipId);
        }
        String_t shipName;
        if (info.getValue(gp::ms_Name).get(shipName)) {
            e.setShipName(shipName);
        }

        // Add
        add(e);
    }
}

// ObjectType:
game::map::Explosion*
game::map::ExplosionType::getObjectByIndex(Id_t index)
{
    if (index > 0 && index <= Id_t(m_explosions.size())) {
        return m_explosions[index-1];
    } else {
        return 0;
    }
}

game::Id_t
game::map::ExplosionType::getNextIndex(Id_t index) const
{
    if (index < Id_t(m_explosions.size())) {
        return index+1;
    } else {
        return 0;
    }
}

game::Id_t
game::map::ExplosionType::getPreviousIndex(Id_t index) const
{
    if (index == 0) {
        return Id_t(m_explosions.size());
    } else {
        return index-1;
    }
}
