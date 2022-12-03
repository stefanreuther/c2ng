/**
  *  \file game/v3/undoinformation.cpp
  *  \brief Class game::v3::UndoInformation
  */

#include <algorithm>
#include "game/v3/undoinformation.hpp"
#include "afl/base/countof.hpp"
#include "game/v3/reverter.hpp"
#include "game/v3/structures.hpp"
#include "game/map/playedshiptype.hpp"
#include "game/spec/hull.hpp"

using game::map::Ship;
using game::map::ShipData;
using game::spec::Cost;
using game::Element;

// Default constructor.
game::v3::UndoInformation::UndoInformation()
{
    // ex GUndoInformation::GUndoInformation
    clear();
}

// Initialize.
void
game::v3::UndoInformation::set(const game::map::Universe& univ,
                               const game::spec::ShipList& shipList,
                               const game::config::HostConfiguration& config,
                               const Reverter& reverter,
                               int planetId)
{
    // ex GUndoInformation::computeInformation
    // ex planacc.pas:GetUndoableSupplies, CountAmmo, ComputeMinimumTech
    clear();

    // Fetch planets. This function only works if we have undo information and are playing this unit.
    const game::map::Planet* dat = univ.planets().get(planetId);
    const game::map::PlanetData* dis = reverter.getPlanetData(planetId);
    const game::map::BaseData* oldBase = reverter.getBaseData(planetId);
    if (dat == 0 || dis == 0 || !dat->isPlayable(game::map::Object::Playable)) {
        return;
    }
    int planetOwner;
    game::map::Point planetPos;
    if (!dat->getOwner().get(planetOwner) || !dat->getPosition().get(planetPos)) {
        return;
    }

    // Count resources used. That is, if something consumes supplies, increase supplyDiffs.
    int32_t supplyDiffs = 0;

    // Count units built.
    int32_t fighterDiffs = 0;
    game::map::BaseStorage torpDiffs;

    // Count planet: supply diffs only
    supplyDiffs += dat->getNumBuildings(FactoryBuilding).orElse(0) - dis->numFactories.orElse(0);
    supplyDiffs += dat->getNumBuildings(MineBuilding).orElse(0)    - dis->numMines.orElse(0);
    supplyDiffs += dat->getNumBuildings(DefenseBuilding).orElse(0) - dis->numDefensePosts.orElse(0);

    if (dat->isBuildingBase() && !dis->baseFlag.orElse(0)) {
        supplyDiffs += config[config.StarbaseCost](planetOwner).get(Cost::Supplies);
    }

    // Count base: ammo diffs
    if (dat->hasBase() && oldBase != 0) {
        fighterDiffs += dat->getCargo(Element::Fighters).orElse(0) - oldBase->numFighters.orElse(0);
        for (int i = 1; i <= structures::NUM_TORPEDO_TYPES; ++i) {
            torpDiffs.set(i, dat->getCargo(Element::fromTorpedoType(i)).orElse(0) - oldBase->torpedoStorage.get(i).orElse(0));
        }
    }

    // Count ships: ammo and supply diffs
    const game::map::PlayedShipType& playedShips = univ.playedShips();
    for (int sid = playedShips.findNextIndex(0); sid != 0; sid = playedShips.findNextIndex(sid)) {
        const Ship* newShip = univ.ships().get(sid);
        const ShipData* oldShip = reverter.getShipData(sid);
        int shipOwner;
        game::map::Point shipPos;
        if (newShip != 0 && oldShip != 0
            && newShip->getOwner().get(shipOwner) && shipOwner == planetOwner
            && newShip->getPosition().get(shipPos) && shipPos == planetPos)
        {
            supplyDiffs += newShip->getCargo(Element::Supplies).orElse(0) - oldShip->supplies.orElse(0);
            int ammoDiff = newShip->getAmmo().orElse(0) - oldShip->ammo.orElse(0);
            if (newShip->getNumBays().orElse(0) > 0) {
                fighterDiffs += ammoDiff;
            } else if (newShip->getNumLaunchers().orElse(0) > 0) {
                int torpType;
                if (newShip->getTorpedoType().get(torpType)) {
                    torpDiffs.set(torpType, torpDiffs.get(torpType).orElse(0) + ammoDiff);
                }
            } else {
                // no secondary weapons
            }

            // Supplies in cargo transporters
            if (newShip->isTransporterActive(Ship::TransferTransporter)) {
                supplyDiffs += newShip->getTransporterCargo(Ship::TransferTransporter, Element::Supplies).orElse(0);
            }
            if (isTransferActive(oldShip->transfer)) {
                supplyDiffs -= oldShip->transfer.supplies.orElse(0);
            }
            if (newShip->isTransporterActive(Ship::UnloadTransporter)) {
                supplyDiffs += newShip->getTransporterCargo(Ship::UnloadTransporter, Element::Supplies).orElse(0);
            }
            if (isTransferActive(oldShip->unload)) {
                supplyDiffs -= oldShip->unload.supplies.orElse(0);
            }
        }
    }

    // If we built fighters, include them in the supply count.
    // FIXME: what if components cost supplies?
    if (fighterDiffs > 0) {
        supplyDiffs += fighterDiffs * config[config.BaseFighterCost](planetOwner).get(Cost::Supplies);
    }

    // Produce output for ammo diffs
    if (fighterDiffs > 0) {
        m_fightersAllowedToSell = fighterDiffs;
    }
    for (int i = 1; i <= structures::NUM_TORPEDO_TYPES; ++i) {
        int n = torpDiffs.get(i).orElse(0);
        if (n > 0) {
            m_torpedoesAllowedToSell.set(i, n);
        }
    }

    // Produce output for supply diffs: we started the turn with X supplies,
    // used Y and sold Z, so we end up at X' = X-Y-Z, or Z=X-Y-X'.
    m_suppliesAllowedToBuy = dis->supplies.orElse(0) - supplyDiffs - dat->getCargo(Element::Supplies).orElse(0);
    if (m_suppliesAllowedToBuy < 0) {
        m_suppliesAllowedToBuy = 0;
    }

    // Produce output for tech levels
    if (dat->hasBase() && oldBase != 0) {
        int level;

        // Hulls
        level = oldBase->techLevels[HullTech].orElse(1);
        for (int i = 1; i <= structures::NUM_HULLS_PER_PLAYER; ++i) {
            int oldCount = oldBase->hullStorage.get(i).orElse(0);
            int newCount = dat->getBaseStorage(HullTech, i).orElse(0);
            if (newCount > oldCount) {
                const game::spec::Hull* pHull = shipList.hulls().get(shipList.hullAssignments().getHullFromIndex(config, planetOwner, i));
                if (pHull != 0) {
                    level = std::max(level, pHull->getTechLevel());
                }
            }
        }
        m_minTechLevels[HullTech] = level;

        // Engines
        level = oldBase->techLevels[EngineTech].orElse(1);
        for (int i = 1; i <= structures::NUM_ENGINE_TYPES; ++i) {
            int oldCount = oldBase->engineStorage.get(i).orElse(0);
            int newCount = dat->getBaseStorage(EngineTech, i).orElse(0);
            if (newCount > oldCount) {
                if (const game::spec::Engine* pEngine = shipList.engines().get(i)) {
                    level = std::max(level, pEngine->getTechLevel());
                }
            }
        }
        m_minTechLevels[EngineTech] = level;

        // Beams
        level = oldBase->techLevels[BeamTech].orElse(1);
        for (int i = 1; i <= structures::NUM_BEAM_TYPES; ++i) {
            int oldCount = oldBase->beamStorage.get(i).orElse(0);
            int newCount = dat->getBaseStorage(game::BeamTech, i).orElse(0);
            if (newCount > oldCount) {
                if (const game::spec::Beam* pBeam = shipList.beams().get(i)) {
                    level = std::max(level, pBeam->getTechLevel());
                }
            }
        }
        m_minTechLevels[BeamTech] = level;

        // Torpedoes
        level = oldBase->techLevels[TorpedoTech].orElse(1);
        for (int i = 1; i <= structures::NUM_TORPEDO_TYPES; ++i) {
            int oldCount = oldBase->launcherStorage.get(i).orElse(0);
            int newCount = dat->getBaseStorage(TorpedoTech, i).orElse(0);
            if (newCount > oldCount || torpDiffs.get(i).orElse(0) > 0) {
                if (const game::spec::TorpedoLauncher* pTL = shipList.launchers().get(i)) {
                    level = std::max(level, pTL->getTechLevel());
                }
            }
        }
        m_minTechLevels[TorpedoTech] = level;
    }
}

// Reset.
void
game::v3::UndoInformation::clear()
{
    // ex GUndoInformation::clear
    m_torpedoesAllowedToSell.clear();
    m_fightersAllowedToSell = 0;
    m_suppliesAllowedToBuy = 0;
    for (size_t i = 0; i < countof(m_minTechLevels); ++i) {
        m_minTechLevels[i] = 1;
    }
}

// Get number of torpedoes that can be sold.
int
game::v3::UndoInformation::getNumTorpedoesAllowedToSell(int slot) const
{
    // ex GUndoInformation::getAmmoAllowedToSell
    return m_torpedoesAllowedToSell.get(slot).orElse(0);
}

// Get number of fighters that can be sold.
int
game::v3::UndoInformation::getNumFightersAllowedToSell() const
{
    // ex GUndoInformation::getAmmoAllowedToSell
    return m_fightersAllowedToSell;
}

// Get number of supplies that can be bought.
int
game::v3::UndoInformation::getSuppliesAllowedToBuy() const
{
    // ex GUndoInformation::getSuppliesAllowedToBuy
    return m_suppliesAllowedToBuy;
}

// /** Get minimum tech level permitted on base. */
int
game::v3::UndoInformation::getMinTechLevel(TechLevel level) const
{
    // ex GUndoInformation::getMinimumTech
    return m_minTechLevels[level];
}
