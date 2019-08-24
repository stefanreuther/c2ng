/**
  *  \file game/actions/taxationaction.cpp
  *  \brief Class game::actions::TaxationAction
  */

#include <algorithm>
#include "game/actions/taxationaction.hpp"
#include "afl/string/format.hpp"
#include "game/exception.hpp"
#include "game/map/planetformula.hpp"
#include "game/tables/happinessname.hpp"

using afl::string::Format;

// Constructor.
game::actions::TaxationAction::TaxationAction(game::map::Planet& planet,
                                              game::config::HostConfiguration& config,
                                              const game::HostVersion& host)
    : m_planet(planet),
      m_config(config),
      m_hostVersion(host),
      m_numBuildings(),
      conn_planetChange(m_planet.sig_change.add(this, &TaxationAction::onChange)),
      conn_configChange(m_config.sig_change.add(this, &TaxationAction::onChange))
{ }

// Destructor.
game::actions::TaxationAction::~TaxationAction()
{ }

// Set number of buildings (mines + factories).
void
game::actions::TaxationAction::setNumBuildings(int mifa)
{
    m_numBuildings = mifa;
    update();
}

// Get tax rate for an area.
int
game::actions::TaxationAction::getTax(Area a) const
{
    const int* p = m_tax[a].get();
    return p ? *p : getOriginalTax(a);
}

// Get amount due for an area.
int32_t
game::actions::TaxationAction::getDue(Area a) const
{
    switch (a) {
     case Colonists:
        return getColonistDue(m_planet, m_config, m_hostVersion, getTax(a)).orElse(0);

     case Natives:
        return getNativeDue(m_planet, m_config, m_hostVersion, getTax(a)).orElse(0);
    }
    return 0;
}

// Get amount due, limited to amount collected for an area.
int32_t
game::actions::TaxationAction::getDueLimited(Area a) const
{
    int32_t remInc = 0;
    int32_t colTax = getColonistDueLimited(m_planet, m_config, m_hostVersion, getTax(Colonists), remInc).orElse(0);
    switch (a) {
     case Colonists:
        return colTax;

     case Natives:
        return getNativeDueLimited(m_planet, m_config, m_hostVersion, getTax(Natives), remInc).orElse(0);
    }
    return 0;
}

// Get happiness change.
int
game::actions::TaxationAction::getHappinessChange(Area a) const
{
    switch (a) {
     case Colonists:
        return getColonistChange(m_planet, m_config, m_hostVersion, getTax(Colonists), getNumBuildings()).orElse(0);

     case Natives:
        return getNativeChange(m_planet, m_hostVersion, getTax(Natives), getNumBuildings()).orElse(0);
    }
    return 0;
}

// Get bovinoid supply contribution.
int
game::actions::TaxationAction::getBovinoidSupplyContribution() const
{
    return game::map::getBovinoidSupplyContribution(m_planet, m_config, m_hostVersion).orElse(0);
}

// Get bovinoid supply contribution, limited to amount collected.
int
game::actions::TaxationAction::getBovinoidSupplyContributionLimited() const
{
    return game::map::getBovinoidSupplyContributionLimited(m_planet, m_config, m_hostVersion).orElse(0);
}

// Check whether area is modifyable.
bool
game::actions::TaxationAction::isModifyable(Area a) const
{
    return m_planet.isPlayable(game::map::Object::Playable)
        && isAvailable(a);
}

// Check whether area is available.
bool
game::actions::TaxationAction::isAvailable(Area a) const
{
    switch (a) {
     case Colonists:
        return true;

     case Natives:
        return (m_planet.getNativeRace().orElse(0) != 0
                && m_planet.getNativeGovernment().isValid()
                && m_planet.getNatives().isValid());
    }
    return false;
}

// Get minimum tax rate for an area.
int
game::actions::TaxationAction::getMinTax(Area a) const
{
    return isModifyable(a) ? 0 : getOriginalTax(a);
}

// Get maximum tax rate for an area.
int
game::actions::TaxationAction::getMaxTax(Area a) const
{
    return isModifyable(a) ? 100 : getOriginalTax(a);
}

// Describe current tax rate.
String_t
game::actions::TaxationAction::describe(Area a, afl::string::Translator& tx, const util::NumberFormatter& fmt) const
{
    String_t result;

    const int happyChange = getHappinessChange(a);
    const int32_t due = getDue(a);
    const int32_t pay = getDueLimited(a);
    int newHappy = 0;

    switch (a) {
     case Colonists: {
        // ex WColonistTaxSelector::drawContent (part)
        int oldHappy = m_planet.getColonistHappiness().orElse(0);
        newHappy = oldHappy + happyChange;
        if (oldHappy < 30 || newHappy < m_hostVersion.getPostTaxationHappinessLimit()) {
            result += Format(tx("Riots \xE2\x80\x94 Colonists do not pay %d mc."), fmt.formatNumber(due));
        } else if (due != pay) {
            result += Format(tx("Colonists pay %d of %d mc."), fmt.formatNumber(pay), fmt.formatNumber(due));
        } else {
            result += Format(tx("Colonists pay %d mc."), fmt.formatNumber(due));
        }
        break;
     }
     case Natives: {
        // ex WNativeTaxSelector::drawContent (part)
        int oldHappy = m_planet.getNativeHappiness().orElse(0);
        newHappy = oldHappy + happyChange;

        int sdue = getBovinoidSupplyContribution();
        int spay = getBovinoidSupplyContributionLimited();

        if (sdue != 0) {
            result += Format(tx("They need to pay %d mc and %d kt supplies."), fmt.formatNumber(due), fmt.formatNumber(sdue));
        } else {
            result += Format(tx("They need to pay %d mc."), fmt.formatNumber(due));
        }
        result += "\n";

        if (oldHappy < 30 || newHappy < m_hostVersion.getPostTaxationHappinessLimit()) {
            result += tx("Riots \xE2\x80\x94 Natives do not pay taxes.");
        } else if (m_planet.getNativeRace().orElse(0) == AmorphousNatives) {
            result += Format(tx("Amorphous worms don't pay taxes. They eat %d colonist clan%!1{s%}."),
                             fmt.formatNumber(game::map::getAmorphousBreakfast(m_hostVersion, newHappy)));
        } else if (sdue != 0) {
            if (spay < sdue) {
                if (pay < due) {
                    result += Format(tx("You can collect only %d mc and %d kt supplies."), fmt.formatNumber(pay), fmt.formatNumber(spay));
                } else {
                    result += Format(tx("You can collect all the money, but only %d kt supplies."), fmt.formatNumber(spay));
                }
            } else {
                if (pay < due) {
                    result += Format(tx("You can collect only %d mc, but all supplies."), fmt.formatNumber(pay));
                } else {
                    result += tx("You can collect all the money and supplies.");
                }
            }
        } else if (pay < due) {
            result += Format(tx("You can only collect %d mc."), fmt.formatNumber(pay));
        } else {
            result += tx("You can collect all the money.");
        }
        break;
     }
    }
    result += "\n";
    result += Format(tx("New happiness: %s (%d)"), game::tables::HappinessName(tx)(newHappy), newHappy);

    return result;
}

// Check validity.
bool
game::actions::TaxationAction::isValid() const
{
    int c = getTax(Colonists);
    int n = getTax(Natives);
    return c >= getMinTax(Colonists) && c <= getMaxTax(Colonists)
        && n >= getMinTax(Natives)   && n <= getMaxTax(Natives);
}

// Set tax rate, unconditionally.
void
game::actions::TaxationAction::setTax(Area a, int value)
{
    if (getTax(a) != value) {
        m_tax[a] = value;
        update();
    }
}

// Set tax rate, limit to valid range.
void
game::actions::TaxationAction::setTaxLimited(Area a, int value)
{
    setTax(a, std::max(getMinTax(a), std::min(getMaxTax(a), value)));
}

// Change tax rate for better/worse revenue.
void
game::actions::TaxationAction::changeRevenue(Area a, Direction d)
{
    // ex changeNativeTax, changeColonistTax
    const int revenue = getDueLimited(a);
    const int min = getMinTax(a);
    const int max = getMaxTax(a);
    int tax = getTax(a);
    while (1) {
        tax += (d == Up ? +1 : -1);
        if (tax < min || tax > max) {
            // Exit because range exceeded
            break;
        }
        m_tax[a] = tax;
        if (getDueLimited(a) != revenue) {
            // Exit because revenue changed
            break;
        }
    }
    update();
}

// Change tax rate.
void
game::actions::TaxationAction::changeTax(Area a, int delta)
{
    setTaxLimited(a, getTax(a) + delta);
}

// Set safe-tax for areas.
void
game::actions::TaxationAction::setSafeTax(Areas_t a)
{
    const int mifa = getNumBuildings();
    int tax;
    if (a.contains(Colonists)) {
        if (getColonistSafeTax(m_planet, m_config, m_hostVersion, mifa).get(tax)) {
            setTax(Colonists, tax);
        }
    }
    if (a.contains(Natives)) {
        if (getNativeSafeTax(m_planet, m_config, m_hostVersion, mifa).get(tax)) {
            setTax(Natives, tax);
        }
    }
}

// Revert tax rates.
void
game::actions::TaxationAction::revert(Areas_t as)
{
    if (as.contains(Colonists)) {
        setTax(Colonists, getOriginalTax(Colonists));
    }
    if (as.contains(Natives)) {
        setTax(Natives, getOriginalTax(Natives));
    }
}

// Commit transaction.
void
game::actions::TaxationAction::commit()
{
    if (!isValid()) {
        throw Exception(Exception::eRange);
    }

    int c = getTax(Colonists);
    if (c != getOriginalTax(Colonists)) {
        m_planet.setColonistTax(c);
    }

    int n = getTax(Natives);
    if (n != getOriginalTax(Natives)) {
        m_planet.setNativeTax(n);
    }
}

// Access planet being worked on.
game::map::Planet&
game::actions::TaxationAction::planet() const
{
    return m_planet;
}

int
game::actions::TaxationAction::getOriginalTax(Area a) const
{
    switch (a) {
     case Colonists:
        return m_planet.getColonistTax().orElse(0);

     case Natives:
        return m_planet.getNativeTax().orElse(0);
    }
    return 0;
}

int
game::actions::TaxationAction::getNumBuildings() const
{
    if (const int* p = m_numBuildings.get()) {
        return *p;
    } else {
        return m_planet.getNumBuildings(MineBuilding).orElse(0)
            + m_planet.getNumBuildings(FactoryBuilding).orElse(0);
    }
}

void
game::actions::TaxationAction::onChange()
{
    update();
}

void
game::actions::TaxationAction::update()
{
    // FIXME: potential optimisation: compute everything here. Send signal only if a computed value changes.
    sig_change.raise();
}
