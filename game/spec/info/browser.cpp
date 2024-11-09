/**
  *  \file game/spec/info/browser.cpp
  *  \brief Class game::spec::info::Browser
  *
  *  This tries to keep the number of abstractions at an acceptable
  *  level, and therefore duplicates some information. For example,
  *  whether an attribute is filterable needs to be defined at
  *  multiple places. An alternative would have been to build a table
  *  with a bunch of closures for each (Page, Attribute) combination
  *  and derive the information from that.
  *
  *  Given that we also have a bunch of special cases (e.g. the
  *  Range_IsArmed attribute that translates into a SetValueRange
  *  filter instead of EditRange like everything else, and String_Name
  *  is totally special) such abstractions would have meant quite
  *  considerable bloat.
  */

#include <algorithm>
#include "game/spec/info/browser.hpp"
#include "afl/string/char.hpp"
#include "afl/string/format.hpp"
#include "game/spec/info/filter.hpp"
#include "game/spec/info/info.hpp"
#include "game/spec/info/picturenamer.hpp"
#include "game/spec/info/utils.hpp"

namespace {
    using afl::string::Format;

    void addAttribute(game::spec::info::PageContent& content, const String_t& name, const String_t& value)
    {
        if (!value.empty()) {
            content.attributes.push_back(game::spec::info::Attribute(name, value));
        }
    }

    bool matchAttribute(game::spec::info::OptionalInt_t value, const game::spec::info::IntRange_t& range)
    {
        int v;
        if (value.get(v)) {
            return range.contains(v);
        } else {
            return true;
        }
    }
}

/******************************** Matcher ********************************/

/*
 *  String Matcher
 *
 *  We want to support multi-word matches, i.e. "light clas" will match
 *  "whatever class light cruiser". This parses the search string into a
 *  list of words; an subject string matches if it contains all of these
 *  words. An empty search string matches everything.
 *
 *  A search implementation will take a couple of subject strings per item
 *  and throw them against our operator(); use ok() first to test for an
 *  empty search expression to avoid generating subject strings.
 */
class game::spec::info::Browser::Matcher {
 public:
    Matcher(const String_t& s);

    bool ok() const
        { return m_keys.empty(); }

    bool operator()(const String_t& name) const;

 private:
    std::vector<String_t> m_keys;
};

game::spec::info::Browser::Matcher::Matcher(const String_t& s)
{
    bool blank = true;
    for (size_t i = 0; i < s.size(); ++i) {
        if (afl::string::charIsSpace(s[i])) {
            blank = true;
        } else {
            if (blank) {
                m_keys.push_back(String_t());
            }
            m_keys.back() += afl::string::charToLower(s[i]);
            blank = false;
        }
    }
}

bool
game::spec::info::Browser::Matcher::operator()(const String_t& name) const
{
    const String_t copy(afl::string::strLCase(name));
    for (size_t i = 0, n = m_keys.size(); i < n; ++i) {
        if (copy.find(m_keys[i]) == String_t::npos) {
            return false;
        }
    }
    return true;
}

/****************************** CompareName ******************************/

class game::spec::info::Browser::CompareName {
 public:
    bool operator()(const ListEntry& a, const ListEntry& b) const
        { return afl::string::strCaseCompare(a.name, b.name) < 0; }
};

/******************************* CompareKey ******************************/

class game::spec::info::Browser::CompareKey {
 public:
    CompareKey(const Browser& browser, Page p, FilterAttribute key)
        : m_browser(browser),
          m_page(p),
          m_key(key)
        { }
    bool operator()(const ListEntry& a, const ListEntry& b) const
        {
            int32_t valA = 0, valB = 0;
            bool knownA = m_browser.getAttribute(m_page, a.id, m_key).get(valA);
            bool knownB = m_browser.getAttribute(m_page, b.id, m_key).get(valB);
            if (knownA != knownB) {
                return knownA > knownB;
            } else {
                return valA < valB;
            }
        }
 private:
    const Browser& m_browser;
    Page m_page;
    FilterAttribute m_key;
};

/******************************** Browser ********************************/

game::spec::info::Browser::Browser(const PictureNamer& picNamer, const Root& root, const ShipList& list, int viewpointPlayer, afl::string::Translator& tx)
    : m_picNamer(picNamer),
      m_root(root),
      m_shipList(list),
      m_translator(tx),
      m_viewpointPlayer(viewpointPlayer),
      m_racialAbilities()
{
    m_racialAbilities.addShipRacialAbilities(list);
    m_racialAbilities.addConfigRacialAbilities(root.hostConfiguration(), root.userConfiguration().getNumberFormatter(), tx);
    m_racialAbilities.addAdvantages(list.advantages());
    m_racialAbilities.filterPlayers(root.playerList().getAllPlayers());
}

std::auto_ptr<game::spec::info::PageContent>
game::spec::info::Browser::describeItem(Page p, Id_t id, bool withCost, int forPlayer) const
{
    std::auto_ptr<PageContent> result(new PageContent());
    switch (p) {
     case PlayerPage:          describePlayer       (*result, id);                                                                            break;
     case HullPage:            describeHull         (*result, id, m_shipList, withCost, m_picNamer, m_root, m_viewpointPlayer, m_translator); break;
     case RacialAbilitiesPage: describeRacialAbility(*result, id);                                                                            break;
     case ShipAbilitiesPage:   describeShipAbility  (*result, id, forPlayer);                                                                 break;
     case EnginePage:          describeEngine       (*result, id, m_shipList, withCost, m_picNamer, m_root, m_viewpointPlayer, m_translator); break;
     case BeamPage:            describeBeam         (*result, id, m_shipList, withCost, m_picNamer, m_root, m_viewpointPlayer, m_translator); break;
     case TorpedoPage:         describeTorpedo      (*result, id, m_shipList, withCost, m_picNamer, m_root, m_viewpointPlayer, m_translator); break;
     case FighterPage:         describeFighter      (*result, id, m_shipList, withCost, m_picNamer, m_root,                    m_translator); break;
    }
    return result;
}

std::auto_ptr<game::spec::info::ListContent>
game::spec::info::Browser::listItems(Page p, const Filter& f, FilterAttribute sort) const
{
    std::auto_ptr<ListContent> result(new ListContent());
    switch (p) {
     case PlayerPage:          listPlayers(*result, f);         break;
     case HullPage:            listHulls(*result, f);           break;
     case RacialAbilitiesPage: listRacialAbilities(*result, f); break;
     case ShipAbilitiesPage:   listShipAbilities(*result, f);   break;
     case EnginePage:          listEngines(*result, f);         break;
     case BeamPage:            listBeams(*result, f);           break;
     case TorpedoPage:         listTorpedoes(*result, f);       break;
     case FighterPage:         listFighters(*result, f);        break;
    }

    if (sort == Range_Id) {
        // Default sort, no change
    } else if (sort == String_Name) {
        std::stable_sort(result->content.begin(), result->content.end(), CompareName());
    } else {
        std::stable_sort(result->content.begin(), result->content.end(), CompareKey(*this, p, sort));
    }

    return result;
}

std::auto_ptr<game::spec::info::FilterInfos_t>
game::spec::info::Browser::describeFilters(Page p, const Filter& filter) const
{
    std::auto_ptr<FilterInfos_t> result(new FilterInfos_t());
    filter.describe(*result, *this);

    FilterAttributes_t atts = getAvailableFilterAttributes(p);
    for (size_t i = 0, n = result->size(); i != n; ++i) {
        (*result)[i].active = atts.contains((*result)[i].elem.att);
    }
    return result;
}

std::auto_ptr<game::spec::info::FilterInfos_t>
game::spec::info::Browser::getAvailableFilters(Page p, const Filter& existing) const
{
    // Determine available filters
    // - We never allow filtering by Id (sort only).
    FilterAttributes_t atts = getAvailableFilterAttributes(p)
        - Range_Id;

    // Remove existing ones
    for (Filter::Iterator_t it = existing.begin(); it != existing.end(); ++it) {
        atts -= it->att;
    }
    if (!existing.getNameFilter().empty()) {
        atts -= String_Name;
    }

    // Build result
    std::auto_ptr<FilterInfos_t> result(new FilterInfos_t());
    addFilterInfo(*result, atts, Range_CostD);
    addFilterInfo(*result, atts, Range_CostM);
    addFilterInfo(*result, atts, Range_CostMC);
    addFilterInfo(*result, atts, Range_CostT);
    addFilterInfo(*result, atts, Range_DamagePower);
    addFilterInfo(*result, atts, Range_HitOdds);
    if (atts.contains(Range_IsArmed)) {
        result->push_back(FilterInfo(m_translator("Armed"),   String_t(), SetValueRange, IntRange_t(), FilterElement(Range_IsArmed, 0, IntRange_t::fromValue(1))));
        result->push_back(FilterInfo(m_translator("Unarmed"), String_t(), SetValueRange, IntRange_t(), FilterElement(Range_IsArmed, 0, IntRange_t::fromValue(0))));
    }
    if (atts.contains(Range_IsDeathRay)) {
        result->push_back(FilterInfo(m_translator("Death Rays"),     String_t(), SetValueRange, IntRange_t(), FilterElement(Range_IsDeathRay, 0, IntRange_t::fromValue(1))));
        result->push_back(FilterInfo(m_translator("Normal Weapons"), String_t(), SetValueRange, IntRange_t(), FilterElement(Range_IsDeathRay, 0, IntRange_t::fromValue(0))));
    }
    addFilterInfo(*result, atts, Range_KillPower);
    addFilterInfo(*result, atts, Range_Mass);
    addFilterInfo(*result, atts, Range_MaxBeams);
    addFilterInfo(*result, atts, Range_MaxCargo);
    addFilterInfo(*result, atts, Range_MaxCrew);
    addFilterInfo(*result, atts, Range_MaxEfficientWarp);
    addFilterInfo(*result, atts, Range_MaxFuel);
    addFilterInfo(*result, atts, Range_MaxLaunchers);
    addFilterInfo(*result, atts, Range_NumBays);
    addFilterInfo(*result, atts, Range_NumEngines);
    addFilterInfo(*result, atts, Range_NumMinesSwept);
    addFilterInfo(*result, atts, Range_RechargeTime);
    addFilterInfo(*result, atts, Range_Tech);
    addFilterInfo(*result, atts, Range_TorpCost);
    addFilterInfo(*result, atts, Range_Id);
    if (atts.contains(Value_Hull)) {
        result->push_back(FilterInfo(m_translator("Hull"), String_t(), EditValueHull, getHullRange(m_shipList), FilterElement(Value_Hull, 1, IntRange_t())));
    }
    if (atts.contains(Value_Player)) {
        result->push_back(FilterInfo(m_translator("Player"), String_t(), EditValuePlayer, getPlayerRange(m_root), FilterElement(Value_Player, 1, IntRange_t())));
    }
    if (atts.contains(Value_Category)) {
        // FIXME
    }
    if (atts.contains(Value_Origin)) {
        // FIXME
    }
    if (atts.contains(ValueRange_ShipAbility)) {
        // FIXME
    }
    if (atts.contains(String_Name)) {
        result->push_back(FilterInfo(m_translator("Name"), String_t(), EditString, IntRange_t(), FilterElement(String_Name, 0, IntRange_t())));
    }

    return result;
}

game::spec::info::FilterAttributes_t
game::spec::info::Browser::getAvailableFilterAttributes(Page p) const
{
    FilterAttributes_t result;
    switch (p) {
     case PlayerPage:
        result = FilterAttributes_t() + Range_Id + Value_Hull + Value_Player + ValueRange_ShipAbility + String_Name;
        break;
     case HullPage:
        result = FilterAttributes_t() + Range_CostD + Range_CostM + Range_CostMC + Range_CostT
            + Range_IsArmed + Range_Mass + Range_MaxBeams + Range_MaxCargo + Range_MaxCrew
            + Range_MaxFuel + Range_MaxLaunchers + Range_NumBays + Range_NumEngines + Range_Id
            + Range_Tech + Value_Hull + Value_Player + ValueRange_ShipAbility + String_Name;
        break;
     case RacialAbilitiesPage:
        result = FilterAttributes_t() + Value_Player + Value_Category + Value_Origin + String_Name;
        break;
     case ShipAbilitiesPage:
        result = FilterAttributes_t() + Value_Player + Value_Hull + String_Name;
        break;
     case EnginePage:
        result = FilterAttributes_t() + Range_CostD + Range_CostM + Range_CostMC + Range_CostT
            + Range_MaxEfficientWarp + Range_Id + Range_Tech + String_Name;
        break;
     case BeamPage:
        result = FilterAttributes_t() + Range_CostD + Range_CostM + Range_CostMC + Range_CostT
            + Range_DamagePower + Range_HitOdds + Range_KillPower + Range_Mass
            + Range_NumMinesSwept + Range_RechargeTime + Range_Id + Range_Tech + String_Name;
        if (m_root.hostVersion().hasDeathRays()) {
            result += Range_IsDeathRay;
        }
        break;
     case TorpedoPage:
        result = FilterAttributes_t() + Range_CostD + Range_CostM + Range_CostMC + Range_CostT
            + Range_DamagePower + Range_HitOdds + Range_KillPower + Range_Mass
            + Range_RechargeTime + Range_TorpCost + Range_Id + Range_Tech + String_Name;
        if (m_root.hostVersion().hasDeathRays()) {
            result += Range_IsDeathRay;
        }
        break;
     case FighterPage:
        result = FilterAttributes_t() + Range_CostD + Range_CostM + Range_CostMC + Range_CostT
            + Range_DamagePower + Range_KillPower + Range_RechargeTime + Value_Player;
        break;
    }
    return result;
}

game::spec::info::FilterAttributes_t
game::spec::info::Browser::getAvailableSortAttributes(Page p) const
{
    FilterAttributes_t result = getAvailableFilterAttributes(p);

    result -= Value_Hull;
    result -= Value_Player;
    result -= ValueRange_ShipAbility;

    result += Range_Id;                   // Always valid: natural sort
    result += String_Name;                // Always valid
    return result;
}

void
game::spec::info::Browser::addItemFilter(Filter& f, Page p, Id_t id) const
{
    switch (p) {
     case PlayerPage:
        // Filter by player: id is player Id
        f.add(FilterElement(Value_Player, id, IntRange_t()));
        break;
     case HullPage:
        // Filter by hull: id is hull Id
        f.add(FilterElement(Value_Hull, id, IntRange_t()));
        break;
     case RacialAbilitiesPage:
        // FIXME: original design had this, but it is not very useful
        // Filter by racial ability: id is index into m_racialAbilities
        // if (const RacialAbilityList::Ability* a = m_racialAbilities.get(size_t(id))) {
        //     if (a->origin == RacialAbilityList::FromHullFunction) {
        //         f.add(FilterElement(ValueRange_RacialAbility, a->basicFunctionId, convertSetToRange(a->players)));
        //     }
        // }
        break;
     case ShipAbilitiesPage:
        if (const BasicHullFunction* hf = m_shipList.basicHullFunctions().getFunctionByIndex(size_t(id))) {
            f.add(FilterElement(ValueRange_ShipAbility, hf->getId(), getLevelRange(m_root)));
        }
        break;
     case EnginePage:
     case BeamPage:
     case TorpedoPage:
     case FighterPage:
        // No filter
        break;
    }
}

game::spec::info::OptionalInt_t
game::spec::info::Browser::getAttribute(Page p, Id_t id, FilterAttribute att) const
{
    switch (p) {
     case PlayerPage:
        // No special sorts
        break;

     case HullPage:
        if (const Hull* h = m_shipList.hulls().get(id)) {
            return getHullAttribute(*h, att);
        }
        break;

     case RacialAbilitiesPage:
        if (const RacialAbilityList::Ability* a = m_racialAbilities.get(id)) {
            return getRacialAbilityAttribute(*a, att);
        }
        break;

     case ShipAbilitiesPage:
        // No special sorts
        break;

     case EnginePage:
        if (const Engine* e = m_shipList.engines().get(id)) {
            return getEngineAttribute(*e, att);
        }
        break;

     case BeamPage:
        if (const Beam* b = m_shipList.beams().get(id)) {
            return getBeamAttribute(*b, att, m_root, m_viewpointPlayer);
        }
        break;

     case TorpedoPage:
        if (const TorpedoLauncher* tl = m_shipList.launchers().get(id)) {
            return getTorpedoAttribute(*tl, att, m_root, m_viewpointPlayer);
        }
        break;

     case FighterPage:
        return getFighterAttribute(Fighter(id, m_root.hostConfiguration(), m_root.playerList(), m_translator), att, m_root);
    }
    return OptionalInt_t();
}

void
game::spec::info::Browser::describePlayer(PageContent& content, Id_t id) const
{
    content.title = m_root.playerList().getPlayerName(id, Player::LongName, m_translator);
    if (const Player* pl = m_root.playerList().get(id)) {
        content.pictureName = m_picNamer.getPlayerPicture(*pl);
        addAttribute(content, m_translator("Short name"),    pl->getName(Player::ShortName,     m_translator));
        addAttribute(content, m_translator("Adjective"),     pl->getName(Player::AdjectiveName, m_translator));
        addAttribute(content, m_translator("User name"),     pl->getName(Player::UserName,      m_translator));
        addAttribute(content, m_translator("Email address"), pl->getName(Player::EmailAddress,  m_translator));

        for (RacialAbilityList::Iterator_t it = m_racialAbilities.begin(), e = m_racialAbilities.end(); it != e; ++it) {
            if (it->players.contains(id)) {
                content.abilities.push_back(Ability(it->name, m_picNamer.getAbilityPicture(it->pictureName, AbilityFlags_t()), AbilityFlags_t()));
            }
        }
    }

    content.pageLinks = Pages_t()
        + RacialAbilitiesPage            // "Racial abilities of this player"
        + HullPage;                      // "Hulls of this player"
}

void
game::spec::info::Browser::listPlayers(ListContent& content, const Filter& f) const
{
    for (Player* pl = m_root.playerList().getFirstPlayer(); pl != 0; pl = m_root.playerList().getNextPlayer(pl)) {
        if (matchPlayer(*pl, f)) {
            content.content.push_back(ListEntry(pl->getName(Player::ShortName, m_translator), pl->getId()));
        }
    }
}

bool
game::spec::info::Browser::matchPlayer(const Player& pl, const Filter& f) const
{
    if (!matchPlayerName(pl, f.getNameFilter())) {
        return false;
    }
    for (Filter::Iterator_t it = f.begin(), e = f.end(); it != e; ++it) {
        if (!matchPlayer(pl, *it)) {
            return false;
        }
    }
    return true;
}

bool
game::spec::info::Browser::matchPlayer(const Player& pl, const FilterElement& e) const
{
    switch (e.att) {
     case Range_CostD:
     case Range_CostM:
     case Range_CostMC:
     case Range_CostT:
     case Range_DamagePower:
     case Range_HitOdds:
     case Range_IsArmed:
     case Range_IsDeathRay:
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
        break;
     case Range_Id:
        // Check Id range
        return e.range.contains(pl.getId());
     case Value_Hull:
        // Check whether player can build the hull
        return shipList().hullAssignments().getIndexFromHull(root().hostConfiguration(), pl.getId(), e.value) != 0;
     case Value_Player:
        // Check for player Id
        return e.value == pl.getId();
     case Value_Category:
     case Value_Origin:
        break;
     case ValueRange_ShipAbility:
        // Check whether player can build any ship with this ability
        return checkPlayerShipAbility(pl.getId(), e.value, e.range);
     case String_Name:
        break;
    }
    return true;
}

bool
game::spec::info::Browser::matchPlayerName(const Player& pl, const String_t& f) const
{
    Matcher m(f);
    return m.ok()
        || m(pl.getName(Player::ShortName,     m_translator))
        || m(pl.getName(Player::LongName,      m_translator))
        || m(pl.getName(Player::AdjectiveName, m_translator))
        || m(pl.getName(Player::UserName,      m_translator))
        || m(pl.getName(Player::NickName,      m_translator));
}

void
game::spec::info::Browser::describeRacialAbility(PageContent& content, Id_t id) const
{
    if (const RacialAbilityList::Ability* a = m_racialAbilities.get(size_t(id))) {
        content.title = a->name;
        content.pictureName = m_picNamer.getAbilityPicture(a->pictureName, AbilityFlags_t());
        if (!a->explanation.empty()) {
            content.attributes.push_back(Attribute(a->explanation, String_t()));
        }
        content.attributes.push_back(Attribute(m_translator("Category"), toString(a->category, m_translator)));
        content.attributes.push_back(Attribute(m_translator("Origin"), toString(a->origin, m_translator)));
        content.players = a->players;
    }
}

void
game::spec::info::Browser::listRacialAbilities(ListContent& content, const Filter& f) const
{
    for (size_t i = 0, n = m_racialAbilities.size(); i < n; ++i) {
        if (const RacialAbilityList::Ability* a = m_racialAbilities.get(i)) {
            if (matchRacialAbility(*a, f)) {
                content.content.push_back(ListEntry(a->name, int(i)));
            }
        }
    }
}

bool
game::spec::info::Browser::matchRacialAbility(const RacialAbilityList::Ability& a, const Filter& f) const
{
    if (!matchRacialAbilityName(a, f.getNameFilter())) {
        return false;
    }
    for (Filter::Iterator_t it = f.begin(); it != f.end(); ++it) {
        if (!matchRacialAbility(a, *it)) {
            return false;
        }
    }
    return true;
}

bool
game::spec::info::Browser::matchRacialAbility(const RacialAbilityList::Ability& a, const FilterElement& e) const
{
    switch (e.att) {
     case Range_CostD:
     case Range_CostM:
     case Range_CostMC:
     case Range_CostT:
     case Range_DamagePower:
     case Range_HitOdds:
     case Range_IsArmed:
     case Range_IsDeathRay:
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
     case Value_Hull:
        break;
     case Value_Player:
        // Check player who has ability
        return a.players.contains(e.value);
     case Value_Category:
        // Check category
        return a.category == e.value;
     case Value_Origin:
        // Check origin
        return a.origin == e.value;
     case ValueRange_ShipAbility:
        break;
     case String_Name:
        break;
    }
    return true;
}

bool
game::spec::info::Browser::matchRacialAbilityName(const RacialAbilityList::Ability& a, const String_t& f) const
{
    // FIXME: for now, this matches names only. It might make sense to also look at hullfunc names
    // (not contained in Ability, need to look up basicFunctionId) and possibly description.
    Matcher m(f);
    return m.ok()
        || m(a.name);
}

game::spec::info::OptionalInt_t
game::spec::info::Browser::getRacialAbilityAttribute(const RacialAbilityList::Ability& a, FilterAttribute att) const
{
    if (att == Value_Category) {
        return a.category;
    } else if (att == Value_Origin) {
        return a.origin;
    } else {
        return OptionalInt_t();
    }
}

#include <stdio.h>
void
game::spec::info::Browser::describeShipAbility(PageContent& content, Id_t id, int forPlayer) const
{
    if (const BasicHullFunction* fcn = m_shipList.basicHullFunctions().getFunctionByIndex(size_t(id))) {
        afl::string::Translator& tx = m_translator;

        content.title = fcn->getDescription();
        content.pictureName = m_picNamer.getAbilityPicture(fcn->getPictureName(), AbilityFlags_t());
        content.attributes.push_back(Attribute(fcn->getExplanation(), String_t()));
        content.attributes.push_back(Attribute(tx("Name"), fcn->getName()));
        content.attributes.push_back(Attribute(tx("Id"), Format("%d", fcn->getId())));

        for (int pl = 1; pl <= MAX_PLAYERS; ++pl) {
            if (checkPlayerShipAbility(pl, fcn->getId(), getLevelRange(m_root))) {
                content.players += pl;
            }
        }

        PlayerSet_t specimenFilter;
        if (forPlayer != 0) {
            specimenFilter += forPlayer;
        } else {
            specimenFilter = m_root.playerList().getAllPlayers();
        }

        if (const Hull* pHull = m_shipList.findSpecimenHullForFunction(fcn->getId(), m_root.hostConfiguration(), specimenFilter, specimenFilter, false)) {
            content.attributes.push_back(Attribute(tx("Sample hull"), pHull->getName(m_shipList.componentNamer())));
        }
    }

    content.pageLinks = Pages_t()
        + PlayerPage
        + HullPage;
}

void
game::spec::info::Browser::listShipAbilities(ListContent& content, const Filter& f) const
{
    const BasicHullFunctionList& list = m_shipList.basicHullFunctions();
    for (size_t i = 0, n = list.getNumFunctions(); i < n; ++i) {
        if (const BasicHullFunction* fcn = list.getFunctionByIndex(i)) {
            if (matchShipAbility(*fcn, f)) {
                content.content.push_back(ListEntry(fcn->getDescription(), int(i)));
            }
        }
    }
}

bool
game::spec::info::Browser::matchShipAbility(const BasicHullFunction& hf, const Filter& f) const
{
    if (!matchShipAbilityName(hf, f.getNameFilter())) {
        return false;
    }
    for (Filter::Iterator_t it = f.begin(); it != f.end(); ++it) {
        if (!matchShipAbility(hf, *it)) {
            return false;
        }
    }
    return true;
}

bool
game::spec::info::Browser::matchShipAbility(const BasicHullFunction& hf, const FilterElement& e) const
{
    switch (e.att) {
     case Range_CostD:
     case Range_CostM:
     case Range_CostMC:
     case Range_CostT:
     case Range_DamagePower:
     case Range_HitOdds:
     case Range_IsArmed:
     case Range_IsDeathRay:
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
        break;
     case Value_Hull:
        if (const Hull* h = m_shipList.hulls().get(e.value)) {
            return checkShipAbility(*h, hf.getId(), getLevelRange(m_root), m_root.playerList().getAllPlayers());
        }
        break;
     case Value_Player:
        // Check whether player has any ship with this ability
        return checkPlayerShipAbility(e.value, hf.getId(), getLevelRange(m_root));
     case Value_Category:
     case Value_Origin:
     case ValueRange_ShipAbility:
     case String_Name:
        break;
    }
    return true;
}

bool
game::spec::info::Browser::matchShipAbilityName(const BasicHullFunction& hf, const String_t& f) const
{
    Matcher m(f);
    return m.ok()
        || m(hf.getDescription())
        || m(hf.getName());
}

void
game::spec::info::Browser::listHulls(ListContent& content, const Filter& f) const
{
    // Special-case the player filter for two reasons:
    // - reading truehull gives the order expected by players
    // - we need the player for correct ability filtering.
    //   Filtering Player=X, Ability=Chunnel should only return ships that can chunnel for player X.
    const int player = f.getPlayerFilter();
    if (player != 0) {
        const HullAssignmentList& hal = m_shipList.hullAssignments();
        const game::config::HostConfiguration& config = m_root.hostConfiguration();
        for (int slot = 0, n = hal.getMaxIndex(config, player); slot <= n; ++slot) {
            const Hull* h = m_shipList.hulls().get(hal.getHullFromIndex(config, player, slot));
            if (h != 0 && matchHull(*h, f, PlayerSet_t(player))) {
                content.content.push_back(ListEntry(h->getName(m_shipList.componentNamer()), h->getId()));
            }
        }
    } else {
        for (const Hull* h = m_shipList.hulls().findNext(0); h != 0; h = m_shipList.hulls().findNext(h->getId())) {
            if (matchHull(*h, f, PlayerSet_t::allUpTo(MAX_PLAYERS))) {
                content.content.push_back(ListEntry(h->getName(m_shipList.componentNamer()), h->getId()));
            }
        }
    }
}

bool
game::spec::info::Browser::matchHull(const Hull& h, const Filter& f, PlayerSet_t playerSet) const
{
    if (!matchComponentName(h, f.getNameFilter())) {
        return false;
    }
    for (Filter::Iterator_t it = f.begin(); it != f.end(); ++it) {
        if (!matchHull(h, *it, playerSet)) {
            return false;
        }
    }
    return true;
}

bool
game::spec::info::Browser::matchHull(const Hull& h, const FilterElement& e, PlayerSet_t playerSet) const
{
    switch (e.att) {
     case Range_CostD:
     case Range_CostM:
     case Range_CostMC:
     case Range_CostT:
     case Range_DamagePower:
     case Range_HitOdds:
     case Range_IsArmed:
     case Range_IsDeathRay:
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
        return matchAttribute(getHullAttribute(h, e.att), e.range);
     case Value_Hull:
        return (e.value == h.getId());
     case Value_Player:
        return (m_shipList.hullAssignments().getIndexFromHull(m_root.hostConfiguration(), e.value, h.getId()) != 0);
     case Value_Category:
     case Value_Origin:
        break;
     case ValueRange_ShipAbility:
        return checkShipAbility(h, e.value, e.range, playerSet);
     case String_Name:
        break;
    }
    return true;
}

void
game::spec::info::Browser::listEngines(ListContent& content, const Filter& f) const
{
    for (const Engine* e = m_shipList.engines().findNext(0); e != 0; e = m_shipList.engines().findNext(e->getId())) {
        if (matchEngine(*e, f)) {
            content.content.push_back(ListEntry(e->getName(m_shipList.componentNamer()), e->getId()));
        }
    }
}

inline bool
game::spec::info::Browser::matchEngine(const Engine& engine, const Filter& f) const
{
    if (!matchComponentName(engine, f.getNameFilter())) {
        return false;
    }
    for (Filter::Iterator_t it = f.begin(); it != f.end(); ++it) {
        if (!matchEngine(engine, *it)) {
            return false;
        }
    }
    return true;
}

inline bool
game::spec::info::Browser::matchEngine(const Engine& engine, const FilterElement& e) const
{
    return matchAttribute(getEngineAttribute(engine, e.att), e.range);
}

void
game::spec::info::Browser::listBeams(ListContent& content, const Filter& f) const
{
    for (const Beam* b = m_shipList.beams().findNext(0); b != 0; b = m_shipList.beams().findNext(b->getId())) {
        if (matchBeam(*b, f)) {
            content.content.push_back(ListEntry(b->getName(m_shipList.componentNamer()), b->getId()));
        }
    }
}

inline bool
game::spec::info::Browser::matchBeam(const Beam& beam, const Filter& f) const
{
    if (!matchComponentName(beam, f.getNameFilter())) {
        return false;
    }
    for (Filter::Iterator_t it = f.begin(); it != f.end(); ++it) {
        if (!matchBeam(beam, *it)) {
            return false;
        }
    }
    return true;
}

inline bool
game::spec::info::Browser::matchBeam(const Beam& beam, const FilterElement& e) const
{
    return matchAttribute(getBeamAttribute(beam, e.att, m_root, m_viewpointPlayer), e.range);
}

void
game::spec::info::Browser::listTorpedoes(ListContent& content, const Filter& f) const
{
    for (const TorpedoLauncher* p = m_shipList.launchers().findNext(0); p != 0; p = m_shipList.launchers().findNext(p->getId())) {
        if (matchTorpedo(*p, f)) {
            content.content.push_back(ListEntry(p->getName(m_shipList.componentNamer()), p->getId()));
        }
    }
}

inline bool
game::spec::info::Browser::matchTorpedo(const TorpedoLauncher& torp, const Filter& f) const
{
    if (!matchComponentName(torp, f.getNameFilter())) {
        return false;
    }
    for (Filter::Iterator_t it = f.begin(); it != f.end(); ++it) {
        if (!matchTorpedo(torp, *it)) {
            return false;
        }
    }
    return true;
}

inline bool
game::spec::info::Browser::matchTorpedo(const TorpedoLauncher& torp, const FilterElement& e) const
{
    return matchAttribute(getTorpedoAttribute(torp, e.att, m_root, m_viewpointPlayer), e.range);
}

bool
game::spec::info::Browser::matchComponentName(const Component& comp, const String_t& name) const
{
    Matcher m(name);
    return m.ok()
        || m(comp.getName(m_shipList.componentNamer()))
        || m(comp.getShortName(m_shipList.componentNamer()));
}

void
game::spec::info::Browser::listFighters(ListContent& content, const Filter& f) const
{
    for (Player* pl = m_root.playerList().getFirstPlayer(); pl != 0; pl = m_root.playerList().getNextPlayer(pl)) {
        Fighter ftr(pl->getId(), m_root.hostConfiguration(), m_root.playerList(), m_translator);
        if (matchFighter(ftr, f)) {
            content.content.push_back(ListEntry(ftr.getName(m_shipList.componentNamer()), ftr.getId()));
        }
    }
}

inline bool
game::spec::info::Browser::matchFighter(const Fighter& ftr, const Filter& f) const
{
    if (!matchComponentName(ftr, f.getNameFilter())) {
        return false;
    }
    for (Filter::Iterator_t it = f.begin(); it != f.end(); ++it) {
        if (!matchFighter(ftr, *it)) {
            return false;
        }
    }
    return true;
}

inline bool
game::spec::info::Browser::matchFighter(const Fighter& ftr, const FilterElement& e) const
{
    switch (e.att) {
     case Range_CostD:
     case Range_CostM:
     case Range_CostMC:
     case Range_CostT:
     case Range_DamagePower:
     case Range_HitOdds:
     case Range_IsArmed:
     case Range_IsDeathRay:
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
        return matchAttribute(getFighterAttribute(ftr, e.att, m_root), e.range);
     case Value_Player:
        return e.value == ftr.getId();
     case Value_Hull:
     case Value_Category:
     case Value_Origin:
     case ValueRange_ShipAbility:
     case String_Name:
        break;
    }
    return true;
}

void
game::spec::info::Browser::addFilterInfo(FilterInfos_t& result, FilterAttributes_t set, FilterAttribute att) const
{
    if (set.contains(att)) {
        const IntRange_t maxRange = getAttributeRange(att);
        result.push_back(FilterInfo(toString(att, m_translator), String_t(), EditRange, maxRange, FilterElement(att, 0, maxRange)));
    }
}

bool
game::spec::info::Browser::checkShipAbility(const Hull& h, int basicFunctionId, IntRange_t levelRange, PlayerSet_t playerSet) const
{
    const BasicHullFunctionList& basicDefs = m_shipList.basicHullFunctions();
    const ModifiedHullFunctionList& modList = m_shipList.modifiedHullFunctions();
    const game::config::HostConfiguration& config = m_root.hostConfiguration();
    const ExperienceLevelSet_t levels = convertRangeToSet(levelRange);

    return h.getHullFunctions(true).getPlayersThatCan(basicFunctionId, modList, basicDefs, config, h, levels, true).containsAnyOf(playerSet)
        || h.getHullFunctions(false).getPlayersThatCan(basicFunctionId, modList, basicDefs, config, h, levels, true).containsAnyOf(playerSet);
}

bool
game::spec::info::Browser::checkPlayerShipAbility(int player, int basicFunctionId, IntRange_t levelRange) const
{
    const HullAssignmentList& asgn = m_shipList.hullAssignments();
    const game::config::HostConfiguration& config = m_root.hostConfiguration();

    for (int i = 1, n = asgn.getMaxIndex(config, player); i <= n; ++i) {
        if (const Hull* h = m_shipList.hulls().get(asgn.getHullFromIndex(config, player, i))) {
            if (checkShipAbility(*h, basicFunctionId, levelRange, PlayerSet_t(player))) {
                return true;
            }
        }
    }
    return false;
}
