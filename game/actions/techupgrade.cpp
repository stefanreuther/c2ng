/**
  *  \file game/actions/techupgrade.cpp
  *  \brief Class game::actions::TechUpgrade
  */

#include "game/actions/techupgrade.hpp"
#include "game/actions/basebuildexecutor.hpp"
#include "game/actions/preconditions.hpp"
#include "game/map/reverter.hpp"
#include "game/map/universe.hpp"
#include "game/registrationkey.hpp"

// Constructor.
game::actions::TechUpgrade::TechUpgrade(game::map::Planet& planet,
                                        CargoContainer& container,
                                        game::spec::ShipList& shipList,
                                        Root& root)
    : BaseBuildAction(planet, container, shipList, root),
      conn_undoChange(),
      m_pUniverse(0)
{
    // ex GStarbaseTechUpgradeAction::GStarbaseTechUpgradeAction
    // Preconditions (redundant, actually)
    mustHavePlayedBase(planet);

    for (size_t i = 0; i < NUM_TECH_AREAS; ++i) {
        m_newTechLevels[i] = m_minTechLevels[i] = planet.getBaseTechLevel(TechLevel(i)).orElse(1);
    }
    update();
}

// Set undo information.
void
game::actions::TechUpgrade::setUndoInformation(game::map::Universe& univ)
{
    // ex GStarbaseTechUpgradeAction::setUndoInformation
    m_pUniverse = &univ;
    conn_undoChange = univ.sig_universeChange.add(this, &TechUpgrade::onUndoChange);
    onUndoChange();
}

// Get current minimum tech level.
int
game::actions::TechUpgrade::getMinTechLevel(TechLevel area) const
{
    // ex GStarbaseTechUpgradeAction::getMinTech
    return m_minTechLevels[area];
}

// Get maximum tech level.
int
game::actions::TechUpgrade::getMaxTechLevel(TechLevel area) const
{
    // ex GStarbaseTechUpgradeAction::getMaxTech
    return registrationKey().getMaxTechLevel(area);
}

// Get current target tech level.
int
game::actions::TechUpgrade::getTechLevel(TechLevel area) const
{
    // ex GStarbaseTechUpgradeAction::getTech
    return m_newTechLevels[area];
}

// Set new target tech level.
bool
game::actions::TechUpgrade::setTechLevel(TechLevel area, int level)
{
    // ex GStarbaseTechUpgradeAction::setTech
    if (level == m_newTechLevels[area]) {
        // No change, ok
        return true;
    } else if (level >= getMinTechLevel(area) && level <= getMaxTechLevel(area)) {
        // Valid change
        m_newTechLevels[area] = level;
        update();
        return true;
    } else {
        // Invalid change
        return false;
    }
}

// Upgrade to new target tech level.
bool
game::actions::TechUpgrade::upgradeTechLevel(TechLevel area, int level)
{
    if (level <= m_newTechLevels[area]) {
        return true;
    } else {
        return setTechLevel(area, level);
    }
}

void
game::actions::TechUpgrade::perform(BaseBuildExecutor& exec)
{
    // ex GStarbaseTechUpgradeAction::perform
    updateUndoInformation();
    for (size_t i = 0; i < NUM_TECH_AREAS; ++i) {
        exec.setBaseTechLevel(TechLevel(i), m_newTechLevels[i]);
    }
}

void
game::actions::TechUpgrade::onUndoChange()
{
    updateUndoInformation();
    update();
}

void
game::actions::TechUpgrade::updateUndoInformation()
{
    if (m_pUniverse != 0) {
        if (const game::map::Reverter* pRev = m_pUniverse->getReverter()) {
            for (size_t i = 0; i < NUM_TECH_AREAS; ++i) {
                const TechLevel level = TechLevel(i);
                int value;
                if (pRev->getMinTechLevel(planet().getId(), level).get(value)) {
                    if (value < planet().getBaseTechLevel(level).orElse(1)) {
                        m_minTechLevels[i] = value;
                    }
                    if (m_newTechLevels[i] < m_minTechLevels[i]) {
                        m_newTechLevels[i] = m_minTechLevels[i];
                    }
                }
            }
        }
    }
}
