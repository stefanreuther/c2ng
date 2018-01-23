/**
  *  \file game/actions/buildstructures.cpp
  *  \brief Class game::actions::BuildStructures
  */

#include "game/actions/buildstructures.hpp"
#include "game/actions/preconditions.hpp"
#include "game/exception.hpp"
#include "game/map/planetformula.hpp"
#include "game/map/reverter.hpp"
#include "game/map/universe.hpp"
#include "game/spec/cost.hpp"

// Constructor.
game::actions::BuildStructures::BuildStructures(game::map::Planet& planet, CargoContainer& container,
                                                const game::config::HostConfiguration& config)
    : sig_change(),
      m_planet(planet),
      m_costAction(container),
      m_hostConfiguration(config),
      m_planetChangeConnection(planet.sig_change.add(this, &BuildStructures::updatePlanet)),
      m_costChangeConnection(m_costAction.sig_change.add(&sig_change, &afl::base::Signal<void()>::raise))
{
    // ex GPlanetBuildStructuresAction::GPlanetBuildStructuresAction
    mustBePlayed(m_planet);

    // Initialize everything to "unmodifieable"
    for (size_t i = 0; i < NUM_PLANETARY_BUILDING_TYPES; ++i) {
        m_data[i].order = m_data[i].min = m_data[i].max = planet.getNumBuildings(PlanetaryBuilding(i)).orElse(0);
    }

    // Set upper limits
    updateUpperLimits();

    // Set costs (should not be necessary, will set the cost to 0)
    updateCost();
}


// Destructor.
game::actions::BuildStructures::~BuildStructures()
{ }

// Set undo information.
void
game::actions::BuildStructures::setUndoInformation(const game::map::Universe& univ)
{
    // ex GPlanetBuildStructuresAction::setUndoInformation [sort-of]
    if (const game::map::Reverter* pRev = univ.getReverter()) {
        for (size_t i = 0; i < NUM_PLANETARY_BUILDING_TYPES; ++i) {
            int min;
            if (pRev->getMinBuildings(m_planet.getId(), PlanetaryBuilding(i)).get(min)) {
                if (min >= 0 && min < m_data[i].min) {
                    m_data[i].min = min;
                }
            }
        }
    }
}

// Add structures.
int
game::actions::BuildStructures::add(PlanetaryBuilding type, int count, bool partial)
{
    // ex GPlanetBuildStructuresAction::addStructures
    if (count > 0) {
        // add
        if (m_data[type].order + count > m_data[type].max) {
            // request may exceed limit
            if (!partial) {
                return 0;
            }
            count = m_data[type].max - m_data[type].order;
        }
    } else {
        // remove
        if (m_data[type].order + count < m_data[type].min) {
            if (!partial) {
                return 0;
            }
            count = m_data[type].min - m_data[type].order;
        }
    }

    if (count != 0) {
        m_data[type].order += count;
        updateCost();
    }
    return count;
}

// Add structures, limited by resources.
int
game::actions::BuildStructures::addLimitCash(PlanetaryBuilding type, int count)
{
    // ex GPlanetBuildStructuresAction::addStructuresLimitCash
    // check how much we can add according to the building limit rules
    int limitedCount = add(type, count, true);

    // do we have enough cash? if not, back out
    // FIXME: this calls the listener a lot
    while (limitedCount > 0 && !m_costAction.isValid()) {
        add(type, -1, true);
        --limitedCount;
    }
    return limitedCount;
}

// Autobuild.
void
game::actions::BuildStructures::doStandardAutoBuild()
{
    // ex GPlanetBuildStructuresAction::doStandardAutoBuild
    // If planet does not have a factory, but wants some, start by building one first, independant of orders.
    if (m_data[FactoryBuilding].order == 0 && m_planet.getAutobuildGoal(FactoryBuilding) != 0) {
        if (addLimitCash(FactoryBuilding, 1) == 0) {
            // We're unable to build anything
            return;
        }
    }

    // Figure out order in which to build */
    PlanetaryBuilding order[NUM_PLANETARY_BUILDING_TYPES];
    for (size_t i = 0; i < NUM_PLANETARY_BUILDING_TYPES; ++i) {
        order[i] = PlanetaryBuilding(i);
    }
    for (size_t i = 0; i < NUM_PLANETARY_BUILDING_TYPES; ++i) {
        for (size_t j = 0; j < NUM_PLANETARY_BUILDING_TYPES-1; ++j) {
            if (m_planet.getAutobuildSpeed(order[j]) < m_planet.getAutobuildSpeed(order[j+1])) {
                std::swap(order[j], order[j+1]);
            }
        }
    }

    // Do it
    bool haveBuilt;
    do {
        haveBuilt = false;
        size_t index = 0;
        while (index < NUM_PLANETARY_BUILDING_TYPES) {
            // Find out how many items have the same speed
            int speed = m_planet.getAutobuildSpeed(order[index]);
            size_t group = 1;
            while (index + group < NUM_PLANETARY_BUILDING_TYPES && speed == m_planet.getAutobuildSpeed(order[index+group])) {
                ++group;
            }

            // Build these
            for (int n = 0; n < speed; ++n) {
                bool did = false;
                for (size_t i = 0; i < group; ++i) {
                    const PlanetaryBuilding what = order[index+i];
                    if (m_data[what].order < m_planet.getAutobuildGoal(what)) {
                        if (addLimitCash(order[index+i], 1) != 0) {
                            did = true;
                            haveBuilt = true;
                        }
                    }
                }
                if (!did) {
                    break;
                }
            }

            index += group;
        }
    } while (haveBuilt);
}

// Get minimum number of buildings permitted in this transaction.
int
game::actions::BuildStructures::getMinBuildings(PlanetaryBuilding type) const
{
    return m_data[type].min;
}

// Get maximum number of buildings permitted in this transaction.
int
game::actions::BuildStructures::getMaxBuildings(PlanetaryBuilding type) const
{
    return m_data[type].max;
}

// Get current target number of buildings.
int
game::actions::BuildStructures::getNumBuildings(PlanetaryBuilding type) const
{
    return m_data[type].order;
}

// Commit transaction.
void
game::actions::BuildStructures::commit()
{
    // ex GPlanetBuildStructuresAction::commit
    // Update cost in case something changed behind our back
    updateCost();
    if (!isValid()) {
        throw Exception(Exception::eNoResource);
    }

    // Commit everything
    for (size_t i = 0; i < NUM_PLANETARY_BUILDING_TYPES; ++i) {
        m_planet.setNumBuildings(PlanetaryBuilding(i), m_data[i].order);
    }
    m_costAction.commit();
}

// Check validity.
bool
game::actions::BuildStructures::isValid() const
{
    return m_costAction.isValid();
}

// Access underlying CargoCostAction.
const game::actions::CargoCostAction&
game::actions::BuildStructures::costAction() const
{
    return m_costAction;
}

void
game::actions::BuildStructures::updateUpperLimits()
{
    // The upper limit can never be reduced, only grow.
    // If a user beams clans down to a planet, that will increase the maximum.
    // Beaming the clans away will not reduce the maximum, as host will not see in what order we did that.
    bool change = false;
    for (size_t i = 0; i < NUM_PLANETARY_BUILDING_TYPES; ++i) {
        int32_t max;
        if (game::map::getMaxBuildings(m_planet, PlanetaryBuilding(i), m_hostConfiguration).get(max)) {
            if (max > m_data[i].max) {
                m_data[i].max = max;
                change = true;
            }
        }
    }
    if (change) {
        sig_change.raise();
    }
}

void
game::actions::BuildStructures::updateCost()
{
    // ex GPlanetBuildStructuresAction::update
    // This targets fixed goals.
    // If we build 3 structures and someone outside builds 5, this ends up scrapping 2.
    int differences[NUM_PLANETARY_BUILDING_TYPES];
    for (size_t i = 0; i < NUM_PLANETARY_BUILDING_TYPES; ++i) {
        differences[i] = m_data[i].order - m_planet.getNumBuildings(PlanetaryBuilding(i)).orElse(0);
    }

    game::spec::Cost cost;
    cost.set(cost.Money,     4*differences[MineBuilding] + 3*differences[FactoryBuilding] + 10*(differences[DefenseBuilding] + differences[BaseDefenseBuilding]));
    cost.set(cost.Supplies,  differences[MineBuilding] + differences[FactoryBuilding] + differences[DefenseBuilding]);
    cost.set(cost.Duranium,  differences[BaseDefenseBuilding]);
    m_costAction.setCost(cost);

    sig_change.raise();
}

void
game::actions::BuildStructures::updatePlanet()
{
    updateUpperLimits();
    updateCost();
}
