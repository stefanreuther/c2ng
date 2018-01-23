/**
  *  \file game/v3/reverter.cpp
  */

#include "game/v3/reverter.hpp"
#include "game/map/planet.hpp"
#include "game/v3/undoinformation.hpp"
#include "game/root.hpp"
#include "game/element.hpp"

using game::map::Planet;
using game::map::PlanetData;
using game::map::BaseData;

game::v3::Reverter::Reverter(game::map::Universe& univ, Session& session)
    : m_universe(univ),
      m_session(session)
{ }

afl::base::Optional<int>
game::v3::Reverter::getMinBuildings(int planetId, PlanetaryBuilding building) const
{
    switch (building) {
     case MineBuilding:
        if (const PlanetData* previousData = m_oldPlanetData.get(planetId)) {
            return previousData->numMines;
        }
        break;

     case FactoryBuilding:
        if (const PlanetData* previousData = m_oldPlanetData.get(planetId)) {
            return previousData->numFactories;
        }
        break;

     case DefenseBuilding:
        if (const PlanetData* previousData = m_oldPlanetData.get(planetId)) {
            return previousData->numDefensePosts;
        }
        break;

     case BaseDefenseBuilding:
        if (const BaseData* previousData = m_oldBaseData.get(planetId)) {
            return previousData->numBaseDefensePosts;
        }
        break;
    }
    return afl::base::Nothing;
}

int
game::v3::Reverter::getSuppliesAllowedToBuy(int planetId) const
{
    UndoInformation u;
    if (prepareUndoInformation(u, planetId)) {
        return u.getSuppliesAllowedToBuy();
    } else {
        return 0;
    }
}

afl::base::Optional<int>
game::v3::Reverter::getMinTechLevel(int planetId, TechLevel techLevel) const
{
    UndoInformation u;
    if (prepareUndoInformation(u, planetId)) {
        return u.getMinTechLevel(techLevel);
    } else {
        return afl::base::Nothing;
    }
}

afl::base::Optional<int>
game::v3::Reverter::getMinBaseStorage(int planetId, TechLevel area, int slot) const
{
    if (const BaseData* previousData = m_oldBaseData.get(planetId)) {
        if (const game::map::BaseStorage* previousStorage = game::map::getBaseStorage(*previousData, area)) {
            return previousStorage->get(slot);
        }
    }
    return afl::base::Nothing;
}

int
game::v3::Reverter::getNumTorpedoesAllowedToSell(int planetId, int slot) const
{
    UndoInformation u;
    if (prepareUndoInformation(u, planetId)) {
        return u.getNumTorpedoesAllowedToSell(slot);
    } else {
        return afl::base::Nothing;
    }
}

int
game::v3::Reverter::getNumFightersAllowedToSell(int planetId) const
{
    UndoInformation u;
    if (prepareUndoInformation(u, planetId)) {
        return u.getNumFightersAllowedToSell();
    } else {
        return afl::base::Nothing;
    }
}

void
game::v3::Reverter::addShipData(int id, const game::map::ShipData& data)
{
    if (game::map::ShipData* p = m_oldShipData.create(id)) {
        *p = data;
    }
}

void
game::v3::Reverter::addPlanetData(int id, const game::map::PlanetData& data)
{
    if (game::map::PlanetData* p = m_oldPlanetData.create(id)) {
        *p = data;
    }
}

void
game::v3::Reverter::addBaseData(int id, const game::map::BaseData& data)
{
    if (game::map::BaseData* p = m_oldBaseData.create(id)) {
        *p = data;
    }
}

const game::map::ShipData*
game::v3::Reverter::getShipData(int id) const
{
    return m_oldShipData.get(id);
}

const game::map::PlanetData*
game::v3::Reverter::getPlanetData(int id) const
{
    return m_oldPlanetData.get(id);
}

const game::map::BaseData*
game::v3::Reverter::getBaseData(int id) const
{
    return m_oldBaseData.get(id);
}

bool
game::v3::Reverter::prepareUndoInformation(UndoInformation& u, int planetId) const
{
    if (m_session.getShipList().get() != 0 && m_session.getRoot().get() != 0) {
        u.set(m_universe, *m_session.getShipList(), m_session.getRoot()->hostConfiguration(), *this, planetId);
        return true;
    } else {
        return false;
    }
}
