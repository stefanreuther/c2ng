/**
  *  \file game/interface/basetaskpredictor.cpp
  *  \brief Class game::interface::BaseTaskPredictor
  */

#include "game/interface/basetaskpredictor.hpp"
#include "game/interface/planetmethod.hpp"
#include "interpreter/arguments.hpp"

namespace {
    // Add base parts. Does not go below 0 even if num is negative.
    void addBaseParts(game::map::Planet& p, game::TechLevel area, int index, int num)
    {
        if (index != 0 && num != 0) {
            int have;
            if (p.getBaseStorage(area, index).get(have)) {
                p.setBaseStorage(area, index, std::max(0, have + num));
            }
        }
    }

    // Remove base parts.
    void consumeBaseParts(game::map::Planet& p, game::TechLevel area, int index, int num)
    {
        addBaseParts(p, area, index, -num);
    }

    // Build base parts to have a minimum amount available.
    void buildBaseParts(game::map::Planet& p, game::TechLevel area, int index, int num)
    {
        if (index != 0 && num != 0) {
            int have;
            if (p.getBaseStorage(area, index).get(have)) {
                p.setBaseStorage(area, index, std::max(have, num));
            }
        }
    }

    // Remove all parts from base storage.
    void clearBaseStorage(game::map::Planet& p, game::TechLevel area)
    {
        for (int i = 1, n = p.getBaseStorageLimit(area); i < n; ++i) {
            p.setBaseStorage(area, i, 0);
        }
    }
}


game::interface::BaseTaskPredictor::BaseTaskPredictor(const game::map::Planet& p,
                                                      const game::map::Universe& univ,
                                                      const game::spec::ShipList& shipList,
                                                      const game::config::HostConfiguration& config)
    : m_planet(p),            // copies the planet
      m_universe(univ),
      m_shipList(shipList),
      m_config(config)
{ }

void
game::interface::BaseTaskPredictor::advanceTurn()
{
    // ex WBaseTaskPredictor::advanceTurn
    // FIXME: consider merging this with PlanetPredictor
    // Ship building
    if (int hullSlot = m_planet.getBaseBuildOrder().getHullIndex()) {
        // What are we building?
        const ShipBuildOrder& order = m_planet.getBaseBuildOrder();

        // Consume components
        consumeBaseParts(m_planet, HullTech, hullSlot, 1);
        if (const game::spec::Hull* h = m_shipList.hulls().get(m_planet.getBaseBuildHull(m_config, m_shipList.hullAssignments()).orElse(0))) {
            consumeBaseParts(m_planet, EngineTech, order.getEngineType(), h->getNumEngines());
        }
        consumeBaseParts(m_planet, BeamTech, order.getBeamType(), order.getNumBeams());
        consumeBaseParts(m_planet, TorpedoTech, order.getLauncherType(), order.getNumLaunchers());

        // Mark done
        m_planet.setBaseBuildOrder(ShipBuildOrder());
    }

    // Recycling
    if (m_planet.getFriendlyCode().orElse(String_t()) == "dmp") {
        for (size_t i = 0; i < NUM_TECH_AREAS; ++i) {
            clearBaseStorage(m_planet, TechLevel(i));
        }
    }

    // Shipyard
    if (m_planet.getBaseShipyardAction().orElse(0) == RecycleShipyardAction) {
        if (const game::map::Ship* sh = m_universe.ships().get(m_planet.getBaseShipyardId().orElse(0))) {
            // Recycling a ship we know
            int hullNr = sh->getHull().orElse(0);
            addBaseParts(m_planet, HullTech, hullNr, 1);
            if (const game::spec::Hull* h = m_shipList.hulls().get(hullNr)) {
                addBaseParts(m_planet, EngineTech, sh->getEngineType().orElse(0), h->getNumEngines());
            }
            addBaseParts(m_planet, BeamTech, sh->getBeamType().orElse(0), sh->getNumBeams().orElse(0));
            addBaseParts(m_planet, TorpedoTech, sh->getTorpedoType().orElse(0), sh->getNumLaunchers().orElse(0));
        }
    }
    m_planet.setBaseShipyardOrder(0, 0);
}

game::map::Planet&
game::interface::BaseTaskPredictor::planet()
{
    // ex WBaseTaskPredictor::getPlanet
    return m_planet;
}

// TaskPredictor:
bool
game::interface::BaseTaskPredictor::predictInstruction(const String_t& name, interpreter::Arguments& args)
{
    // ex WBaseTaskPredictor::predictInstruction
    if (name == "BUILDSHIP" || name == "ENQUEUESHIP") {
        ShipBuildOrder order;
        if (parseBuildShipCommand(args, order, m_shipList)) {
            if (name == "ENQUEUESHIP" && order.getHullIndex() > 0 && m_planet.getBaseBuildOrder().getHullIndex() > 0) {
                advanceTurn();
            }
            postBuildOrder(order);
        }
        return true;
    } else if (name == "SETFCODE") {
        args.checkArgumentCount(1);
        String_t friendlyCode;
        interpreter::checkStringArg(friendlyCode, args.getNext());
        m_planet.setFriendlyCode(friendlyCode);
        return true;
    } else if (name == "SETMISSION") {
        args.checkArgumentCount(1);
        int32_t msn;
        interpreter::checkIntegerArg(msn, args.getNext(), 0, MAX_BASE_MISSION);
        m_planet.setBaseMission(msn);
        return true;
    } else if (name == "WAITONETURN") {
        advanceTurn();
        return true;
    } else if (name == "FIXSHIP" || name == "RECYCLESHIP") {
        args.checkArgumentCount(1);
        int32_t shipId;
        interpreter::checkIntegerArg(shipId, args.getNext());
        if (shipId == 0) {
            m_planet.setBaseShipyardOrder(0, 0);
        } else {
            m_planet.setBaseShipyardOrder(name == "FIXSHIP" ? FixShipyardAction : RecycleShipyardAction, shipId);
        }
        return true;
    } else {
        return true;
    }
}

void
game::interface::BaseTaskPredictor::postBuildOrder(ShipBuildOrder order)
{
    // WBaseTaskPredictor::postBuildOrder
    if (order.getHullIndex() > 0) {
        // Can we actually build this?
        int planetOwner = 0;
        m_planet.getOwner(planetOwner);

        int slot = m_shipList.hullAssignments().getIndexFromHull(m_config, planetOwner, order.getHullIndex());
        if (slot > 0) {
            // Make sure all parts we need for this build are in store.
            buildBaseParts(m_planet, HullTech, slot, 1);
            if (const game::spec::Hull* h = m_shipList.hulls().get(order.getHullIndex())) {
                buildBaseParts(m_planet, EngineTech, order.getEngineType(), h->getNumEngines());
            }
            buildBaseParts(m_planet, BeamTech, order.getBeamType(), order.getNumBeams());
            buildBaseParts(m_planet, TorpedoTech, order.getLauncherType(), order.getNumLaunchers());
            order.setHullIndex(slot);
            m_planet.setBaseBuildOrder(order);
        } else {
            // We cannot build this, so just ignore it.
            order.setHullIndex(0);
        }
    }
    m_planet.setBaseBuildOrder(order);
}
