/**
  *  \file game/actions/buildparts.cpp
  *  \brief Class game::actions::BuildParts
  */

#include "game/actions/buildparts.hpp"
#include "game/actions/basebuildexecutor.hpp"
#include "game/map/reverter.hpp"
#include "game/map/universe.hpp"

// Constructor.
game::actions::BuildParts::BuildParts(game::map::Planet& planet,
                                      CargoContainer& container,
                                      game::spec::ShipList& shipList,
                                      Root& root)
    : BaseBuildAction(planet, container, shipList, root),
      m_elements(),
      m_pUniverse(0),
      conn_universeChange()
{
    update();
}

// Destructor.
game::actions::BuildParts::~BuildParts()
{ }

// Set undo information.
void
game::actions::BuildParts::setUndoInformation(game::map::Universe& univ)
{
    m_pUniverse = &univ;
    conn_universeChange = univ.sig_universeChange.add(this, &BuildParts::update);
    update();
}

// Get minimum number of parts that must remain.
int
game::actions::BuildParts::getMinParts(TechLevel area, int slot) const
{
    // ex GStarbaseBuildPartsAction::getMinimumParts
    // Find minimum
    int min = computeMinParts(area, slot);

    // Ensure we're not disturbing a build order
    const ShipBuildOrder buildOrder = planet().getBaseBuildOrder();
    if (buildOrder.getHullIndex() != 0) {
        int occupiedSlot = 0;
        int requiredAmount = 0;
        switch (area) {
         case HullTech:
            occupiedSlot = buildOrder.getHullIndex();
            requiredAmount = 1;
            break;
         case EngineTech:
            occupiedSlot = buildOrder.getEngineType();
            if (const game::spec::Hull* pHull = shipList().hulls().get(planet().getBaseBuildHull(hostConfiguration(), shipList().hullAssignments()).orElse(0))) {
                requiredAmount = pHull->getNumEngines();
            }
            break;
         case BeamTech:
            occupiedSlot = buildOrder.getBeamType();
            requiredAmount = buildOrder.getNumBeams();
            break;
         case TorpedoTech:
            occupiedSlot = buildOrder.getLauncherType();
            requiredAmount = buildOrder.getNumLaunchers();
            break;
        }

        if (occupiedSlot == slot && requiredAmount > min) {
            min = requiredAmount;
        }
    }

    return min;
}

// Get number of existing parts.
int
game::actions::BuildParts::getNumExistingParts(TechLevel area, int slot) const
{
    // ex GStarbaseBuildPartsAction::getExistingParts
    return planet().getBaseStorage(area, slot).orElse(0);
}

// Get current target number of parts.
int
game::actions::BuildParts::getNumParts(TechLevel area, int slot) const
{
    // ex GStarbaseBuildPartsAction::getTotalParts
    int numParts;
    if (const Element* p = find(area, slot)) {
        numParts = p->target;
    } else {
        numParts = getNumExistingParts(area, slot);
    }
    return numParts;
}

// Add parts.
int
game::actions::BuildParts::add(TechLevel area, int slot, int amount, bool partial)
{
    // ex GStarbaseBuildPartsAction::addParts
    if (amount == 0) {
        // No change
        return 0;
    } else {
        Element& p = findCreate(area, slot);
        const int have = p.target;
        int change;
        if (amount > 0) {
            // Build
            const int LIMIT = MAX_NUMBER;
            const int maxBuildable = have > LIMIT ? 0 : LIMIT - have;
            change = std::min(amount, maxBuildable);
        } else {
            // Scrap
            const int limit = getMinParts(area, slot);
            const int maxScrapable = have < limit ? 0 : limit - have;
            change = std::max(amount, maxScrapable);
        }

        if (change != amount && !partial) {
            change = 0;
        }

        if (change != 0) {
            p.target += change;
            update();
        }
        return change;
    }
}

// Perform all changes.
void
game::actions::BuildParts::perform(BaseBuildExecutor& exec)
{
    // ex GStarbaseBuildPartsAction::perform
    int neededTech[NUM_TECH_AREAS] = {};

    // Update, in case anything changed
    updateUndoInformation();

    // Build everything and register required tech
    for (size_t i = 0, n = m_elements.size(); i < n; ++i) {
        const Element& ele = m_elements[i];
        const int numExistingParts = planet().getBaseStorage(ele.area, ele.slot).orElse(0);
        if (ele.target != numExistingParts) {
            const game::spec::Component* comp = 0;
            exec.setBaseStorage(ele.area, ele.slot, ele.target, 0);
            switch (ele.area) {
             case HullTech: {
                int owner;
                if (planet().getOwner(owner)) {
                    comp = shipList().hulls().get(shipList().hullAssignments().getHullFromIndex(hostConfiguration(), owner, ele.slot));
                }
                break;
             }

             case EngineTech:
                comp = shipList().engines().get(ele.slot);
                break;

             case BeamTech:
                comp = shipList().beams().get(ele.slot);
                break;

             case TorpedoTech:
                comp = shipList().launchers().get(ele.slot);
                break;
            }

            if (ele.target > numExistingParts && comp != 0) {
                neededTech[ele.area] = std::max(neededTech[ele.area], comp->getTechLevel());
            }
        }
    }

    // Build tech levels
    for (size_t i = 0; i < NUM_TECH_AREAS; ++i) {
        if (neededTech[i] > planet().getBaseTechLevel(TechLevel(i)).orElse(1)) {
            exec.setBaseTechLevel(TechLevel(i), neededTech[i]);
        }
    }
}

const game::actions::BuildParts::Element*
game::actions::BuildParts::find(TechLevel area, int slot) const
{
    for (size_t i = 0, n = m_elements.size(); i < n; ++i) {
        if (m_elements[i].area == area && m_elements[i].slot == slot) {
            return &m_elements[i];
        }
    }
    return 0;
}

inline game::actions::BuildParts::Element&
game::actions::BuildParts::findCreate(TechLevel area, int slot)
{
    if (const Element* p = find(area, slot)) {
        return *const_cast<Element*>(p);
    }

    m_elements.push_back(Element(area, slot, getNumExistingParts(area, slot)));
    return m_elements.back();
}

inline int
game::actions::BuildParts::computeMinParts(TechLevel area, int slot) const
{
    // Take minimum parts from reverter, if known; otherwise, don't go below what we currently have.
    if (m_pUniverse != 0) {
        if (game::map::Reverter* pRev = m_pUniverse->getReverter()) {
            if (const int* p = pRev->getMinBaseStorage(planet().getId(), area, slot).get()) {
                return std::max(0, *p);
            }
        }
    }
    return getNumExistingParts(area, slot);
}

void
game::actions::BuildParts::updateUndoInformation()
{
    // A universe change may cause our target to become out of range. Fix it.
    for (size_t i = 0, n = m_elements.size(); i < n; ++i) {
        Element& ele = m_elements[i];
        int min = getMinParts(ele.area, ele.slot);
        if (ele.target < min) {
            ele.target = min;
        }
    }
}
