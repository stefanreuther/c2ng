/**
  *  \file game/actions/basebuildaction.cpp
  *  \brief Class game::actions::BaseBuildAction
  */

#include "game/actions/basebuildaction.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "game/actions/basebuildexecutor.hpp"
#include "game/actions/preconditions.hpp"
#include "game/exception.hpp"
#include "game/map/planetformula.hpp"
#include "game/registrationkey.hpp"
#include "util/translation.hpp"

using game::spec::Cost;

namespace {
    /** Counter. This guy will just account costs. */
    class CountingExecutor : public game::actions::BaseBuildExecutor {
     public:
        CountingExecutor(const game::map::Planet& planet,
                         const game::spec::ShipList& shipList,
                         const game::Root& root);
        virtual void setBaseTechLevel(game::TechLevel area, int value);
        virtual void setBaseStorage(game::TechLevel area, int index, int value, int free);
        virtual void accountHull(int number, int count, int free);

        const Cost& getCost() const
            { return m_cost; }

        bool hasInaccessibleTech() const
            { return m_needInaccessibleTech; }

     private:
        const game::map::Planet& m_planet;
        const game::spec::ShipList& m_shipList;
        const game::Root& m_root;
        Cost m_cost;
        bool m_needInaccessibleTech;
    };

    /** Executor. This guy will write changes to the underlying planet. */
    class ExecutingExecutor : public game::actions::BaseBuildExecutor {
     public:
        explicit ExecutingExecutor(game::map::Planet& planet);
        virtual void setBaseTechLevel(game::TechLevel area, int value);
        virtual void setBaseStorage(game::TechLevel area, int index, int value, int free);
        virtual void accountHull(int number, int count, int free);

     private:
        game::map::Planet& m_planet;
    };

    int getPlanetOwner(const game::map::Planet& planet)
    {
        int owner = 0;
        afl::except::checkAssertion(planet.getOwner(owner), "no owner", "<BaseBuildAction>");
        return owner;
    }

    const game::spec::Hull& getHull(const game::spec::ShipList& shipList, int hullNr)
    {
        const game::spec::Hull* pHull = shipList.hulls().get(hullNr);
        afl::except::checkAssertion(pHull != 0, "invalid hull", "<BaseBuildAction>");
        return *pHull;
    }
}

/***************************** CountingExecutor *****************************/

CountingExecutor::CountingExecutor(const game::map::Planet& planet,
                                   const game::spec::ShipList& shipList,
                                   const game::Root& root)
    : m_planet(planet),
      m_shipList(shipList),
      m_root(root),
      m_cost(),
      m_needInaccessibleTech(false)
{ }

void
CountingExecutor::setBaseTechLevel(game::TechLevel area, int value)
{
    // ex CountingActor::setTech
    // If tech increases, check permission
    const int currentValue = m_planet.getBaseTechLevel(area).orElse(1);
    if (value > currentValue && value > m_root.registrationKey().getMaxTechLevel(area)) {
        m_needInaccessibleTech = true;
    }

    // Do it
    m_cost.add(Cost::Money, game::map::getBaseTechCost(getPlanetOwner(m_planet), currentValue, value, m_root.hostConfiguration()));
}

void
CountingExecutor::setBaseStorage(game::TechLevel area, int index, int value, int /*free*/)
{
    // ex CountingActor::setHullSlot, CountingActor::setEngine, CountingActor::setBeam, CountingActor::setLauncher
    int diff = value - m_planet.getBaseStorage(area, index).orElse(0);
    if (diff != 0) {
        switch (area) {
         case game::HullTech: {
            // Only validate other parameters if there is a difference, so users can try null operations.
            int hullNr = m_shipList.hullAssignments().getHullFromIndex(m_root.hostConfiguration(), getPlanetOwner(m_planet), index);
            m_cost += getHull(m_shipList, hullNr).cost() * diff;
            break;
         }

         case game::EngineTech: {
            const game::spec::Engine* pEngine = m_shipList.engines().get(index);
            afl::except::checkAssertion(pEngine != 0, "invalid engine", "<BaseBuildAction>");
            m_cost += pEngine->cost() * diff;
            break;
         }
         case game::BeamTech: {
            const game::spec::Beam* pBeam = m_shipList.beams().get(index);
            afl::except::checkAssertion(pBeam != 0, "invalid beam", "<BaseBuildAction>");
            m_cost += pBeam->cost() * diff;
            break;
         }
         case game::TorpedoTech: {
            const game::spec::TorpedoLauncher* pTL = m_shipList.launchers().get(index);
            afl::except::checkAssertion(pTL != 0, "invalid launcher", "<BaseBuildAction>");
            m_cost += pTL->cost() * diff;
            break;
         }
        }
    }
}

void
CountingExecutor::accountHull(int number, int count, int /*free*/)
{
    // ex CountingActor::accountHull
    if (count != 0) {
        m_cost += getHull(m_shipList, number).cost() * count;
    }
}

/***************************** ExecutingExecutor ****************************/

ExecutingExecutor::ExecutingExecutor(game::map::Planet& planet)
    : m_planet(planet)
{ }

void
ExecutingExecutor::setBaseTechLevel(game::TechLevel area, int value)
{
    // ex ExecutingActor::setTech
    m_planet.setBaseTechLevel(area, value);
}

void
ExecutingExecutor::setBaseStorage(game::TechLevel area, int index, int value, int /*free*/)
{
    // ex ExecutingActor::setHullSlot, ExecutingActor::setEngine, ExecutingActor::setBeam, ExecutingActor::setLauncher
    m_planet.setBaseStorage(area, index, value);
}

void
ExecutingExecutor::accountHull(int /*number*/, int count, int /*free*/)
{
    afl::except::checkAssertion(count == 0, "inaccessible hull", "<BaseBuildAction>");
}


/**************************** BaseBuildAction ****************************/

game::actions::BaseBuildAction::BaseBuildAction(game::map::Planet& planet,
                                                CargoContainer& container,
                                                game::spec::ShipList& shipList,
                                                Root& root)
    : m_planet(planet),
      m_shipList(shipList),
      m_root(root),
      m_costAction(container),
      m_needInaccessibleTech(false),
      conn_planetChange(),
      conn_shipListChange(),
      conn_configChange()
{
    // ex GStarbaseBuildTransaction::GStarbaseBuildTransaction
    // FIXME: reconsider this constraint check. Move to commit to allow people asking for costs without a played base?
    mustHavePlayedBase(planet);
}

game::actions::BaseBuildAction::~BaseBuildAction()
{ }

void
game::actions::BaseBuildAction::update()
{
    // ex GStarbaseBuildAction::getCost [well... sort-of]
    // Connect signals on first call.
    // That this function is called indicates the object is alive at its most-derived type.
    // (Connecting from the constructor would risk this method to be called when we're still constructing,
    // causing the perform() call go into neverland.)
    if (!conn_planetChange.isConnected()) {
        conn_planetChange   = m_planet.sig_change.add(this, &BaseBuildAction::update);
        conn_shipListChange = m_shipList.sig_change.add(this, &BaseBuildAction::update);
        conn_configChange   = m_root.hostConfiguration().sig_change.add(this, &BaseBuildAction::update);
    }

    // Perform action
    CountingExecutor ca(m_planet, m_shipList, m_root);
    perform(ca);
    m_costAction.setCost(ca.getCost());
    m_needInaccessibleTech = ca.hasInaccessibleTech();

    // Tell observers
    sig_change.raise();
}

game::actions::BaseBuildAction::Status
game::actions::BaseBuildAction::getStatus()
{
    update();
    if (m_needInaccessibleTech) {
        return DisallowedTech;
    } else if (!m_costAction.isValid()) {
        return MissingResources;
    } else {
        return Success;
    }
}

bool
game::actions::BaseBuildAction::isValid()
{
    return getStatus() == Success;
}

void
game::actions::BaseBuildAction::commit()
{
    // Update to get most recent status
    update();

    // Status check
    switch (getStatus()) {
     case MissingResources:
        throw Exception(Exception::eNoResource, _("Not enough resources to perform this action"));

     case DisallowedTech:
        throw Exception(Exception::ePerm, _("Tech level not accessible"));

     case Success:
        break;
    }

    // Disconnect signals to avoid re-triggering ourselves
    conn_planetChange.disconnect();
    conn_shipListChange.disconnect();
    conn_configChange.disconnect();

    // Do it
    ExecutingExecutor xa(m_planet);
    perform(xa);
    m_costAction.commit();
}

