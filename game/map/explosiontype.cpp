/**
  *  \file game/map/explosiontype.cpp
  */

#include "game/map/explosiontype.hpp"

game::map::ExplosionType::ExplosionType(Universe& univ)
    : m_universe(univ),
      m_explosions()
{ }

game::map::ExplosionType::~ExplosionType()
{ }

// /** Add explosion. If this explosion matches one we already know,
//     merges the information.
//     \param x Explosion to add
//     \return reference to added/modified explosion record */
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

game::map::Universe*
game::map::ExplosionType::getUniverseByIndex(Id_t /*index*/)
{
    return &m_universe;
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
