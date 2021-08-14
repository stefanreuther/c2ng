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
#include "afl/string/format.hpp"

using game::spec::Cost;
using game::spec::CostSummary;

namespace {
    const int NeedInaccessibleTech = 1;
    const int NeedForeignHull = 2;
    const int NeedDisabledTech = 4;

    /** Counter. This guy will just account costs. */
    class CountingExecutor : public game::actions::BaseBuildExecutor {
     public:
        CountingExecutor(const game::map::Planet& planet,
                         const game::spec::ShipList& shipList,
                         const game::Root& root,
                         bool useTechUpgrades);
        virtual void setBaseTechLevel(game::TechLevel area, int value);
        virtual void setBaseStorage(game::TechLevel area, int index, int value, int free);
        virtual void accountHull(int number, int count, int free);
        virtual void accountFighterBay(int count);

        const Cost& getCost() const
            { return m_cost; }

        int getImpediments() const
            { return m_impediments; }

     private:
        const game::map::Planet& m_planet;
        const game::spec::ShipList& m_shipList;
        const game::Root& m_root;
        Cost m_cost;
        int m_impediments;
        bool m_useTechUpgrades;
    };

    /** Executor. This guy will write changes to the underlying planet. */
    class ExecutingExecutor : public game::actions::BaseBuildExecutor {
     public:
        explicit ExecutingExecutor(game::map::Planet& planet);
        virtual void setBaseTechLevel(game::TechLevel area, int value);
        virtual void setBaseStorage(game::TechLevel area, int index, int value, int free);
        virtual void accountHull(int number, int count, int free);
        virtual void accountFighterBay(int count);

     private:
        game::map::Planet& m_planet;
    };

    /** Bill builder. */
    class BillingExecutor : public game::actions::BaseBuildExecutor {
     public:
        explicit BillingExecutor(const game::map::Planet& planet, CostSummary& result, const game::spec::ShipList& shipList,
                                 const game::config::HostConfiguration& config, bool useTechUpgrades, afl::string::Translator& tx);
        virtual void setBaseTechLevel(game::TechLevel area, int value);
        virtual void setBaseStorage(game::TechLevel area, int index, int value, int free);
        virtual void accountHull(int number, int count, int free);
        virtual void accountFighterBay(int count);

     private:
        void accountComponent(const game::spec::Component* comp, int have, int count, int free);

        const game::map::Planet& m_planet;
        CostSummary& m_result;
        const game::spec::ShipList& m_shipList;
        const game::config::HostConfiguration& m_config;
        bool m_useTechUpgrades;
        afl::string::Translator& m_translator;
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
                                   const game::Root& root,
                                   bool useTechUpgrades)
    : m_planet(planet),
      m_shipList(shipList),
      m_root(root),
      m_cost(),
      m_impediments(0),
      m_useTechUpgrades(useTechUpgrades)
{ }

void
CountingExecutor::setBaseTechLevel(game::TechLevel area, int value)
{
    // ex CountingActor::setTech
    const int currentValue = m_planet.getBaseTechLevel(area).orElse(1);
    if (m_useTechUpgrades) {
        // If tech increases, check permission
        if (value > currentValue && value > m_root.registrationKey().getMaxTechLevel(area)) {
            m_impediments |= NeedInaccessibleTech;
        }

        // Do it
        m_cost.add(Cost::Money, game::map::getBaseTechCost(getPlanetOwner(m_planet), currentValue, value, m_root.hostConfiguration()));
    } else {
        // Any tech upgrade causes transaction to fail
        if (value > currentValue) {
            m_impediments |= NeedDisabledTech;
        }
    }
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
        m_impediments |= NeedForeignHull;
    }
}

void
CountingExecutor::accountFighterBay(int /*count*/)
{ }

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

void
ExecutingExecutor::accountFighterBay(int /*count*/)
{ }

/**************************** BillingExecutor ****************************/

BillingExecutor::BillingExecutor(const game::map::Planet& planet,
                                 CostSummary& result,
                                 const game::spec::ShipList& shipList,
                                 const game::config::HostConfiguration& config,
                                 bool useTechUpgrades,
                                 afl::string::Translator& tx)
    : m_planet(planet), m_result(result),
      m_shipList(shipList),
      m_config(config),
      m_useTechUpgrades(useTechUpgrades),
      m_translator(tx)
{ }

void
BillingExecutor::setBaseTechLevel(game::TechLevel area, int value)
{
    // BillingActor::setTech
    static const char*const TECH_NAMES[] = {
        N_("Engine tech upgrade"),
        N_("Hull tech upgrade"),
        N_("Beam tech upgrade"),
        N_("Torpedo tech upgrade")
    };

    if (m_useTechUpgrades) {
        int have = m_planet.getBaseTechLevel(area).orElse(0);
        if (value > have) {
            Cost cost;
            cost.set(Cost::Money, game::map::getBaseTechCost(getPlanetOwner(m_planet), have, value, m_config));
            m_result.add(CostSummary::Item(0, value - have, m_translator(TECH_NAMES[area]), cost));
        }
    }
}

void
BillingExecutor::setBaseStorage(game::TechLevel area, int index, int value, int free)
{
    // ex BillingActor::setHullSlot, BillingActor::setEngine, BillingActor::setBeam, BillingActor::setLauncher
    int have = m_planet.getBaseStorage(area, index).orElse(0);
    if (area == game::HullTech) {
        index = m_shipList.hullAssignments().getHullFromIndex(m_config, getPlanetOwner(m_planet), index);
    }
    accountComponent(m_shipList.getComponent(area, index), have, value, free);
}

void
BillingExecutor::accountHull(int number, int count, int free)
{
    // BillingActor::accountHull(int number, int count, int free)
    accountComponent(m_shipList.hulls().get(number), 0, count, free);
}

void
BillingExecutor::accountComponent(const game::spec::Component* comp, int have, int count, int free)
{
    if (comp != 0) {
        String_t name = comp->getName(m_shipList.componentNamer());
        if (count > have) {
            int add = count - have;
            m_result.add(CostSummary::Item(0, add, name, comp->cost() * add));
        }
        if (free != 0) {
            m_result.add(CostSummary::Item(0, free, afl::string::Format(m_translator("From storage: %s"), name), Cost()));
        }
    }
}

void
BillingExecutor::accountFighterBay(int count)
{
    if (count != 0) {
        m_result.add(CostSummary::Item(0, count, m_translator("Fighter Bay"), Cost()));
    }
}


/**************************** BaseBuildAction ****************************/

game::actions::BaseBuildAction::BaseBuildAction(game::map::Planet& planet,
                                                CargoContainer& container,
                                                game::spec::ShipList& shipList,
                                                Root& root,
                                                afl::string::Translator& tx)
    : m_planet(planet),
      m_shipList(shipList),
      m_root(root),
      m_costAction(container),
      m_translator(tx),
      m_impediments(0),
      m_useTechUpgrades(true),
      m_inUpdate(false),
      conn_planetChange(),
      conn_shipListChange(),
      conn_configChange()
{
    // ex GStarbaseBuildTransaction::GStarbaseBuildTransaction
    // FIXME: reconsider this constraint check. Move to commit to allow people asking for costs without a played base?
    mustHavePlayedBase(planet, tx);
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
    CountingExecutor ca(m_planet, m_shipList, m_root, m_useTechUpgrades);
    perform(ca);
    m_costAction.setCost(ca.getCost());
    m_impediments = ca.getImpediments();

    // Tell observers.
    // Must protect against recursion here, because listeners may indirectly invoke update() again.
    if (!m_inUpdate) {
        try {
            m_inUpdate = true;
            sig_change.raise();
            m_inUpdate = false;
        }
        catch (...) {
            m_inUpdate = false;
            throw;
        }
    }
}

game::actions::BaseBuildAction::Status
game::actions::BaseBuildAction::getStatus()
{
    update();
    if ((m_impediments & NeedForeignHull) != 0) {
        return ForeignHull;
    } else if ((m_impediments & NeedDisabledTech) != 0) {
        return DisabledTech;
    } else if ((m_impediments & NeedInaccessibleTech) != 0) {
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
        throw Exception(Exception::eNoResource, m_translator("Not enough resources to perform this action"));

     case DisallowedTech:
        throw Exception(Exception::ePerm, m_translator("Tech level not accessible"));

     case ForeignHull:
        throw Exception(Exception::ePerm, m_translator("Hull not accessible"));

     case DisabledTech:
        throw Exception(Exception::ePerm, m_translator("Tech upgrade required"));

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

    // Update again; this will reconnect signals
    update();
}

bool
game::actions::BaseBuildAction::isUseTechUpgrade() const
{
    return m_useTechUpgrades;
}

void
game::actions::BaseBuildAction::setUseTechUpgrade(bool b)
{
    // ex GStarbaseBuildShipAction::setUseTechUpgrade
    if (b != m_useTechUpgrades) {
        m_useTechUpgrades = b;
        update();
    }
}

void
game::actions::BaseBuildAction::setReservedAmount(game::spec::Cost cost)
{
    m_costAction.setReservedAmount(cost);
}

void
game::actions::BaseBuildAction::getCostSummary(game::spec::CostSummary& result)
{
    BillingExecutor ex(m_planet, result, m_shipList, m_root.hostConfiguration(), m_useTechUpgrades, m_translator);
    perform(ex);
}
