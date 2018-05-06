/**
  *  \file game/map/planeteffectors.cpp
  */

#include "game/map/planeteffectors.hpp"

game::map::PlanetEffectors::PlanetEffectors()
{
    clear();
}

void
game::map::PlanetEffectors::clear()
{
    // ex GPlanetEffectors::clear
    for (size_t i = 0; i < NUM_EFFECTS; ++i) {
        m_effectors[i] = 0;
    }
}

void
game::map::PlanetEffectors::add(Kind eff, int count)
{
    // ex GPlanetEffectors::add
    m_effectors[eff] += count;
}

void
game::map::PlanetEffectors::set(Kind eff, int count)
{
    // ex GPlanetEffectors::set
    m_effectors[eff] = count;
}

int
game::map::PlanetEffectors::get(Kind eff) const
{
    // ex GPlanetEffectors::get
    return m_effectors[eff];
}

int
game::map::PlanetEffectors::getNumTerraformers() const
{
    // ex GPlanetEffectors::getNumTerraformers
    return m_effectors[HeatsTo50] + m_effectors[CoolsTo50] + m_effectors[HeatsTo100];
}
