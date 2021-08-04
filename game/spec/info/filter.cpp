/**
  *  \file game/spec/info/filter.cpp
  *  \brief Class game::spec::info::Filter
  */

#include "game/spec/info/filter.hpp"
#include "afl/string/format.hpp"
#include "game/spec/info/browser.hpp"
#include "game/spec/info/utils.hpp"

using afl::string::Format;
using game::spec::info::IntRange_t;
namespace gsi = game::spec::info;

namespace {
    String_t toString(IntRange_t range, IntRange_t maxRange, afl::string::Translator& tx)
    {
        return util::toString(range, maxRange, true, util::NumberFormatter(false, false), tx);
    }

    gsi::FilterInfo makeDefaultAttribute(String_t name, const gsi::FilterElement& elem, const gsi::Browser& browser)
    {
        const IntRange_t maxRange = getAttributeRange(elem.att);
        return gsi::FilterInfo(name, toString(elem.range, maxRange, browser.translator()), gsi::EditRange, maxRange, elem);
    }

    gsi::FilterInfo makeAbilityAttribute(const gsi::FilterElement& e, String_t label, const gsi::Browser& browser)
    {
        afl::string::Translator& tx = browser.translator();
        String_t value;
        if (const game::spec::BasicHullFunction* p = browser.shipList().basicHullFunctions().getFunctionById(e.value)) {
            value = p->getDescription();
        }
        String_t levelName = game::formatExperienceLevelSet(gsi::convertRangeToSet(e.range), browser.root().hostVersion(), browser.root().hostConfiguration(), tx);
        if (!levelName.empty()) {
            value += " (";
            value += levelName;
            value += ")";
        }

        IntRange_t levelRange = gsi::getLevelRange(browser.root());
        return gsi::FilterInfo(label,
                               value,
                               levelRange.isUnit() ? gsi::NotEditable : gsi::EditRangeLevel,
                               levelRange,
                               e);
    }
}

// Constructor.
game::spec::info::Filter::Filter()
    : m_content(),
      m_nameFilter()
{ }

// Destructor.
game::spec::info::Filter::~Filter()
{ }

// Add filter element.
void
game::spec::info::Filter::add(const FilterElement& e)
{
    if (FilterElement* pe = find(e.att)) {
        // FIXME: ValueRange_ShipAbility can sensibly appear multiple times (ships that can Cloak AND Hyperjump).
        // (The same reasoning can also be applied for Value_Player (ships that can be built by Feds AND Lizards),
        // but we're already rightfully using that specially.)
        pe->value = e.value;
        pe->range = e.range;
    } else {
        m_content.push_back(e);
    }
}

// Describe entire filter.
void
game::spec::info::Filter::describe(FilterInfos_t& result, const Browser& browser) const
{
    for (Iterator_t it = begin(); it != end(); ++it) {
        result.push_back(describe(*it, browser));
    }
    if (!m_nameFilter.empty()) {
        result.push_back(FilterInfo(browser.translator()("Name"), m_nameFilter, EditString, IntRange_t(), FilterElement(String_Name, 0, IntRange_t())));
    }
}

// Describe single element.
game::spec::info::FilterInfo
game::spec::info::Filter::describe(const FilterElement& e, const Browser& browser) const
{
    afl::string::Translator& tx = browser.translator();
    switch (e.att) {
     case Range_CostD:
     case Range_CostM:
     case Range_CostMC:
     case Range_CostT:
     case Range_HitOdds:
     case Range_KillPower:
     case Range_Mass:
     case Range_MaxBeams:
     case Range_MaxCargo:
     case Range_MaxCrew:
     case Range_MaxEfficientWarp:
     case Range_MaxFuel:
     case Range_MaxLaunchers:
     case Range_NumBays:
     case Range_NumEngines:
     case Range_NumMinesSwept:
     case Range_RechargeTime:
     case Range_Tech:
     case Range_TorpCost:
     case Range_Id:
     case Range_DamagePower:
        return makeDefaultAttribute(toString(e.att, tx), e, browser);

     case Range_IsArmed:
        if (e.range == IntRange_t::fromValue(0)) {
            return FilterInfo(tx("Armed"), tx("no"), SetValueRange, IntRange_t(), FilterElement(Range_IsArmed, 0, IntRange_t::fromValue(1)));
        } else if (e.range == IntRange_t::fromValue(1)) {
            return FilterInfo(tx("Armed"), tx("yes"), SetValueRange, IntRange_t(), FilterElement(Range_IsArmed, 0, IntRange_t::fromValue(0)));
        } else {
            return makeDefaultAttribute(tx("Armed"), e, browser);
        }

     case Range_IsDeathRay:
        if (e.range == IntRange_t::fromValue(0)) {
            return FilterInfo(tx("Type"), tx("normal"), SetValueRange, IntRange_t(), FilterElement(Range_IsArmed, 0, IntRange_t::fromValue(1)));
        } else if (e.range == IntRange_t::fromValue(1)) {
            return FilterInfo(tx("Type"), tx("death ray"), SetValueRange, IntRange_t(), FilterElement(Range_IsArmed, 0, IntRange_t::fromValue(0)));
        } else {
            return makeDefaultAttribute(tx("Type"), e, browser);
        }

     case Value_Hull: {
        const ShipList& shipList = browser.shipList();
        String_t name;
        if (const Hull* p = shipList.hulls().get(e.value)) {
            name = p->getName(shipList.componentNamer());
        }
        return FilterInfo(tx("Hull"), name, EditValueHull, getHullRange(shipList), e);
     }

     case Value_Player:
        return FilterInfo(tx("Player"), browser.root().playerList().getPlayerName(e.value, Player::ShortName), EditValuePlayer, getPlayerRange(browser.root()), e);

     case Value_Category:
        return FilterInfo(tx("Category"), toString(RacialAbilityList::Category(e.value), tx), NotEditable, IntRange_t(), e);

     case Value_Origin:
        return FilterInfo(tx("From"), toString(RacialAbilityList::Origin(e.value), tx), NotEditable, IntRange_t(), e);

     case ValueRange_ShipAbility:
        return makeAbilityAttribute(e, tx("Has"), browser);

     case String_Name:
        // Not handled here
        break;
    }
    return FilterInfo(String_t(), String_t(), NotEditable, IntRange_t(), FilterElement(FilterAttribute(), 0, IntRange_t()));
}

// Get player filter.
int
game::spec::info::Filter::getPlayerFilter() const
{
    for (size_t i = 0, n = m_content.size(); i < n; ++i) {
        if (m_content[i].att == Value_Player) {
            return m_content[i].value;
        }
    }
    return 0;
}

// Erase element by index.
void
game::spec::info::Filter::erase(size_t index)
{
    if (index < m_content.size()) {
        m_content.erase(m_content.begin() + index);
    } else {
        // describe() renders the name filter after the content
        m_nameFilter.clear();
    }
}

// Get number of elements.
size_t
game::spec::info::Filter::size() const
{
    return m_content.size();
}

// Set 'range' in an element.
void
game::spec::info::Filter::setRange(size_t index, IntRange_t range)
{
    if (index < m_content.size()) {
        m_content[index].range = range;
    }
}

// Set 'value' in an element.
void
game::spec::info::Filter::setValue(size_t index, int32_t value)
{
    if (index < m_content.size()) {
        m_content[index].value = value;
    }
}

// Set name filter.
void
game::spec::info::Filter::setNameFilter(const String_t& value)
{
    m_nameFilter = value;
}

// Get name filter.
const String_t&
game::spec::info::Filter::getNameFilter() const
{
    return m_nameFilter;
}

game::spec::info::FilterElement*
game::spec::info::Filter::find(FilterAttribute a)
{
    for (size_t i = 0; i < m_content.size(); ++i) {
        if (m_content[i].att == a) {
            return &m_content[i];
        }
    }
    return 0;
}
