/**
  *  \file game/actions/buildship.cpp
  *  \brief Class game::actions::BuildShip
  */

#include "game/actions/buildship.hpp"
#include "game/actions/preconditions.hpp"
#include "game/exception.hpp"
#include "game/actions/basebuildexecutor.hpp"

namespace {

    int findFirstHull(const game::config::HostConfiguration& config,
                      const game::spec::ShipList& shipList,
                      const game::map::Planet& pl)
    {
        int owner = 0;
        pl.getOwner(owner);
        for (int i = 1, n = shipList.hullAssignments().getMaxIndex(config, owner); i <= n; ++i) {
            if (int hullId = shipList.hullAssignments().getHullFromIndex(config, owner, i)) {
                return hullId;
            }
        }
        return 0;
    }

    int findBestEngine(const game::spec::ShipList& shipList,
                       const game::map::Planet& pl)
    {
        int e = 1;
        int maxTech = pl.getBaseTechLevel(game::EngineTech).orElse(1);
        while (const game::spec::Engine* p = shipList.engines().findNext(e)) {
            if (p->getTechLevel() <= maxTech) {
                e = p->getId();
            } else {
                break;
            }
        }
        return e;
    }

    int findBestBeam(const game::spec::ShipList& shipList,
                     const game::map::Planet& pl)
    {
        int b = 1;
        int maxTech = pl.getBaseTechLevel(game::BeamTech).orElse(1);
        while (const game::spec::Beam* p = shipList.beams().findNext(b)) {
            if (p->getTechLevel() <= maxTech) {
                b = p->getId();
            } else {
                break;
            }
        }
        return b;
    }

    int findBestLauncher(const game::spec::ShipList& shipList,
                         const game::map::Planet& pl)
    {
        int t = 1;
        int maxTech = pl.getBaseTechLevel(game::TorpedoTech).orElse(1);
        while (const game::spec::TorpedoLauncher* p = shipList.launchers().findNext(t)) {
            if (p->getTechLevel() <= maxTech) {
                t = p->getId();
            } else {
                break;
            }
        }
        return t;
    }
}

// Constructor.
game::actions::BuildShip::BuildShip(game::map::Planet& planet,
                                    CargoContainer& container,
                                    game::spec::ShipList& shipList,
                                    Root& root)
    : BaseBuildAction(planet, container, shipList, root),
      m_order(),
      m_usePartsFromStorage(true)
{
    // ex GStarbaseBuildShipAction::GStarbaseBuildShipAction
    // Must have a base (redundant, BaseBuildAction also checks it).
    mustHavePlayedBase(planet);

    // Fetch build order from planet.
    // The planet will have a ship INDEX, not TYPE.
    int owner = 0;
    planet.getOwner(owner);
    m_order = planet.getBaseBuildOrder();
    m_order.setHullIndex(shipList.hullAssignments().getHullFromIndex(root.hostConfiguration(), owner, m_order.getHullIndex()));

    // Fix it up
    m_usePartsFromStorage = prepareBuildOrder(m_order, planet, root.hostConfiguration(), shipList);

    // Start up
    update();
}

// Destructor.
game::actions::BuildShip::~BuildShip()
{ }

// Choose whether parts from storage will be used.
void
game::actions::BuildShip::setUsePartsFromStorage(bool flag)
{
    // ex GStarbaseBuildShipAction::setUsePartsFromStorage
    if (flag != m_usePartsFromStorage) {
        m_usePartsFromStorage = flag;
        update();
    }
}

// /** Choose whether tech level upgrades are allowed. If not, we'll
//     refuse to build any ship that needs an upgrade. */
// void
// GStarbaseBuildShipAction::setUseTechUpgrade(bool b)
// {
//     if (b != use_tech_upgrade) {
//         use_tech_upgrade = b;
//         sig_changed.raise();
//     }
// }

// Check whether usage of stored parts is enabled.
bool
game::actions::BuildShip::isUsePartsFromStorage() const
{
    // ex GStarbaseBuildShipAction::isUsePartsFromStorage
    return m_usePartsFromStorage;
}

// Get current build order.
game::ShipBuildOrder
game::actions::BuildShip::getBuildOrder() const
{
    // ex GStarbaseBuildShipAction::getBuildOrder
    return m_order;
}

// Set build order.
void
game::actions::BuildShip::setBuildOrder(const ShipBuildOrder& o)
{
    // ex GStarbaseBuildShipAction::setBuildOrder (sort-of)
    m_order = o;
    update();
}

// Check whether this action is a change to an existing build order.
bool
game::actions::BuildShip::isChange() const
{
    // ex GStarbaseBuildShipAction::isChange
    // Get old order. If that one is empty, this is NOT a change.
    ShipBuildOrder oldOrder = planet().getBaseBuildOrder();
    oldOrder.canonicalize();
    if (oldOrder.getHullIndex() == 0) {
        return false;
    }
    
    // Get new order. If that one is not obtainable, this is a change.
    ShipBuildOrder newOrder;
    if (!getNewOrder(newOrder)) {
        return true;
    }

    return oldOrder != newOrder;
}

void
game::actions::BuildShip::commit()
{
    // Get build order we'll be writing to the base
    ShipBuildOrder newOrder;
    if (!getNewOrder(newOrder)) {
        throw Exception(Exception::ePerm);
    }

    // Commit the transaction
    BaseBuildAction::commit();

    // Now, write the build order
    planet().setBaseBuildOrder(newOrder);
}

void
game::actions::BuildShip::perform(BaseBuildExecutor& exec)
{
    // ex GStarbaseBuildShipAction::perform
    int owner;
    planet().getOwner(owner);

    // First, attempt to build one hull
    const int hullNr = m_order.getHullIndex();
    const game::spec::Hull*const pHull = shipList().hulls().get(hullNr);
    if (int slot = shipList().hullAssignments().getIndexFromHull(hostConfiguration(), owner, hullNr)) {
        // We can build it
        const int available = planet().getBaseStorage(HullTech, slot).orElse(0);
        const int n = getBuildAmount(1, available);
        if (n != 0) {
            doTechUpgrade(HullTech, exec, pHull);
        }
        exec.setBaseStorage(HullTech, slot, available + n, 1 - n);
    } else {
        // We can not build it.
        // FIXME: is_ok = false;
        doTechUpgrade(HullTech, exec, pHull);
        exec.accountHull(m_order.getHullIndex(), 1, 0);
    }

    // Build the engines
    const int engineType = m_order.getEngineType();
    const int numEngines = (pHull != 0 ? pHull->getNumEngines() : 0);
    const int availableEngines = planet().getBaseStorage(EngineTech, engineType).orElse(0);
    const int buildEngines = getBuildAmount(numEngines, availableEngines);
    if (buildEngines != 0) {
        doTechUpgrade(EngineTech, exec, shipList().engines().get(engineType));
    }
    exec.setBaseStorage(EngineTech, engineType, availableEngines + buildEngines, numEngines - buildEngines);

    // Build the arms
    const int beamType = m_order.getBeamType();
    const int numBeams = m_order.getNumBeams();
    if (beamType > 0 && numBeams > 0) {
        const int availableBeams = planet().getBaseStorage(BeamTech, beamType).orElse(0);
        const int buildBeams = getBuildAmount(numBeams, availableBeams);
        if (buildBeams != 0) {
            doTechUpgrade(BeamTech, exec, shipList().beams().get(beamType));
        }
        exec.setBaseStorage(BeamTech, beamType, availableBeams + buildBeams, numBeams - buildBeams);
    }

    const int torpType = m_order.getLauncherType();
    const int numLaunchers = m_order.getNumLaunchers();
    if (torpType > 0 && numLaunchers > 0) {
        const int availableLaunchers = planet().getBaseStorage(TorpedoTech, torpType).orElse(0);
        const int buildLaunchers = getBuildAmount(numLaunchers, availableLaunchers);
        if (buildLaunchers != 0) {
            doTechUpgrade(TorpedoTech, exec, shipList().launchers().get(torpType));
        }
        exec.setBaseStorage(TorpedoTech, torpType, availableLaunchers + buildLaunchers, numLaunchers - buildLaunchers);
    }
}

// /** Prepare a build order.
//     \param o [in/out] The build order
//     \param p [in] Planet
//     \retval true we're re-using the base's build order
//     \retval false this is a new build order */
bool
game::actions::BuildShip::prepareBuildOrder(ShipBuildOrder& o,
                                            const game::map::Planet& pl,
                                            const game::config::HostConfiguration& config,
                                            const game::spec::ShipList& shipList)
{
    // ex GStarbaseBuildShipAction::prepareBuildOrder
    if (shipList.hulls().get(o.getHullIndex()) == 0) {
        // Invalid or no build order. Invent one.
        int hullNr = findFirstHull(config, shipList, pl);
        const game::spec::Hull* p = shipList.hulls().get(hullNr);
        if (p == 0) {
            // This means our configuration does not have a hull for this player. Punt.
            throw Exception(Exception::ePerm);
        }
        o.setHullIndex(hullNr);
        o.setNumBeams(p->getMaxBeams());
        o.setNumLaunchers(p->getMaxLaunchers());
        o.setEngineType(findBestEngine(shipList, pl));
        o.setBeamType(findBestBeam(shipList, pl));
        o.setLauncherType(findBestLauncher(shipList, pl));
        return false;
    } else {
        // Use existing build order
        if (o.getNumBeams() == 0) {
            o.setBeamType(findBestBeam(shipList, pl));
        }
        if (o.getNumLaunchers() == 0) {
            o.setLauncherType(findBestLauncher(shipList, pl));
        }
        return true;
    }
}

/** Compute how many parts we have to build.
    \param need   we need this many parts...
    \param have   ...and we have this many already in storage */
int
game::actions::BuildShip::getBuildAmount(int need, int have) const
{
    // ex GStarbaseBuildShipAction::getBuildAmount
    if (!m_usePartsFromStorage) {
        return need;
    } else if (need <= have) {
        return 0;
    } else {
        return need - have;
    }
}

void
game::actions::BuildShip::doTechUpgrade(game::TechLevel area, BaseBuildExecutor& exec, const game::spec::Component* component) const
{
    // ex GStarbaseBuildShipAction::doTechUpgrade
    // Figure out required tech level. Protect against invalid component.
    const int need = component != 0 ? component->getTechLevel() : 0;

    // Do we need an upgrade?
    int haveTech = planet().getBaseTechLevel(area).orElse(1);
    if (haveTech >= need) {
        return;
    }

    // FIXME: move this to BaseBuildAction.
    // /* we need a tech upgrade but do not want to. In particular, do
    //    not include upgrades in the calculation. */
    // if (!use_tech_upgrade) {
    //     is_ok = false;
    //     return;
    // }

    exec.setBaseTechLevel(area, need);
}

/** Get new build order with truehull index.
    \param o [out] Result
    \retval true Success
    \retval false Order cannot be represented */
bool
game::actions::BuildShip::getNewOrder(ShipBuildOrder& o) const
{
    int owner;
    if (!planet().getOwner(owner)) {
        return false;
    }

    int slot = shipList().hullAssignments().getIndexFromHull(hostConfiguration(), owner, m_order.getHullIndex());
    if (slot == 0) {
        return false;
    }

    o = m_order;
    o.setHullIndex(slot);
    o.canonicalize();
    return true;
}
