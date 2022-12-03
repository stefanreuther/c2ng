/**
  *  \file game/actions/buildship.cpp
  *  \brief Class game::actions::BuildShip
  */

#include "game/actions/buildship.hpp"
#include "game/actions/preconditions.hpp"
#include "game/exception.hpp"
#include "game/actions/basebuildexecutor.hpp"
#include "afl/except/assertionfailedexception.hpp"

namespace {
    int findFirstHull(const game::config::HostConfiguration& config,
                      const game::spec::ShipList& shipList,
                      const game::map::Planet& pl)
    {
        const int owner = pl.getOwner().orElse(0);
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
        int e = 0;
        int maxTech = pl.getBaseTechLevel(game::EngineTech).orElse(1);
        while (const game::spec::Engine* p = shipList.engines().findNext(e)) {
            if (e == 0 || p->getTechLevel() <= maxTech) {
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
        int b = 0;
        int maxTech = pl.getBaseTechLevel(game::BeamTech).orElse(1);
        while (const game::spec::Beam* p = shipList.beams().findNext(b)) {
            if (b == 0 || p->getTechLevel() <= maxTech) {
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
        int t = 0;
        int maxTech = pl.getBaseTechLevel(game::TorpedoTech).orElse(1);
        while (const game::spec::TorpedoLauncher* p = shipList.launchers().findNext(t)) {
            if (t == 0 || p->getTechLevel() <= maxTech) {
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
    const int owner = planet.getOwner().orElse(0);
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

// Get current build order as ShipQuery.
game::ShipQuery
game::actions::BuildShip::getQuery() const
{
    // ex WShipBuildDialog::handleEvent (part)
    const int owner = planet().getOwner().orElse(0);

    ShipQuery q;
    q.setHullType(m_order.getHullIndex());
    q.setPlayerDisplaySet(PlayerSet_t(owner));
    q.setEngineType(m_order.getEngineType());
    q.setOwner(owner);
    q.complete(shipList(), hostConfiguration(), owner, 0);
    return q;
}

void
game::actions::BuildShip::setPart(TechLevel area, int id)
{
    // Refuse setting a component that would fail otherwise.
    afl::except::checkAssertion(shipList().getComponent(area, id) != 0, "<BuildShip::setPart>");
    switch (area) {
     case HullTech:
        m_order.setHullIndex(id);
        if (const game::spec::Hull* h = shipList().hulls().get(id)) {
            m_order.setNumBeams(h->getMaxBeams());
            m_order.setNumLaunchers(h->getMaxLaunchers());
        }
        break;
     case EngineTech:
        m_order.setEngineType(id);
        break;
     case BeamTech:
        m_order.setBeamType(id);
        break;
     case TorpedoTech:
        m_order.setLauncherType(id);
        break;
    }
    update();
}

void
game::actions::BuildShip::setNumParts(Weapon area, int amount)
{
    const game::spec::Hull* h = shipList().hulls().get(m_order.getHullIndex());
    switch (area) {
     case BeamWeapon:
        if (h != 0) {
            amount = std::min(amount, h->getMaxBeams());
        }
        m_order.setNumBeams(std::max(0, amount));
        break;

     case TorpedoWeapon:
        if (h != 0) {
            amount = std::min(amount, h->getMaxLaunchers());
        }
        m_order.setNumLaunchers(std::max(0, amount));
        break;
    }
    update();
}

void
game::actions::BuildShip::addParts(Weapon area, int amount)
{
    switch (area) {
     case BeamWeapon:
        setNumParts(area, m_order.getNumBeams() + amount);
        break;

     case TorpedoWeapon:
        setNumParts(area, m_order.getNumLaunchers() + amount);
        break;
    }
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
    // @change This performs the tech upgrades before the actual parts.
    // Tech upgrades therefore appear before the parts in Detailed Bill (like PCC2, unlike PCC1).
    const int owner = planet().getOwner().orElse(0);

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

    const int numBays = (pHull != 0 ? pHull->getNumBays() : 0);
    exec.accountFighterBay(numBays);
}

/** Prepare a build order.
    \retval true we're re-using the base's build order
    \retval false this is a new build order */
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
        // If a field refers to a nonexistant component, update() will throw (assertion in CountingExecutor).
        // We therefore try to fix those here.
        if (shipList.engines().get(o.getEngineType()) == 0) {
            o.setEngineType(findBestEngine(shipList, pl));
        }
        if (o.getNumBeams() == 0 || shipList.beams().get(o.getBeamType()) == 0) {
            o.setBeamType(findBestBeam(shipList, pl));
            o.setNumBeams(0);
        }
        if (o.getNumLaunchers() == 0 || shipList.launchers().get(o.getLauncherType()) == 0) {
            o.setLauncherType(findBestLauncher(shipList, pl));
            o.setNumLaunchers(0);
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

    // Executor will handle that we need a tech level but may not be allowed to use it
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
    if (!planet().getOwner().get(owner)) {
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
