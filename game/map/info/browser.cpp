/**
  *  \file game/map/info/browser.cpp
  *  \brief Class game::map::info::Browser
  */

#include "game/map/info/browser.hpp"
#include "afl/base/memory.hpp"
#include "afl/io/xml/tagnode.hpp"
#include "afl/io/xml/textnode.hpp"
#include "game/actions/preconditions.hpp"
#include "game/exception.hpp"
#include "game/game.hpp"
#include "game/map/info/info.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"

using afl::io::xml::TagNode;
using afl::io::xml::TextNode;
using game::config::HostConfiguration;
using game::map::info::Nodes_t;
using game::spec::ShipList;

namespace {
    TagNode& makeTag(Nodes_t& out, const String_t& tagName)
    {
        TagNode* p = new TagNode(tagName);
        out.pushBackNew(p);
        return *p;
    }

    void addHeading(Nodes_t& nodes, String_t text)
    {
        TagNode* h = new TagNode("h1");
        nodes.pushBackNew(h);
        h->addNewChild(new TextNode(text));
    }

    TagNode& makeTable(game::map::info::Nodes_t& out)
    {
        TagNode& tab = makeTag(out, "table");
        tab.setAttribute("align", "left");
        return tab;
    }
}


game::map::info::Browser::Browser(Session& session)
    : m_session(session)
{
    afl::base::Memory<PageOptions_t>(m_options).fill(0);
}

void
game::map::info::Browser::setPageOptions(Page page, PageOptions_t opts)
{
    m_options[page] = opts;
}

game::map::info::PageOptions_t
game::map::info::Browser::getPageOptions(Page page) const
{
    return m_options[page];
}

void
game::map::info::Browser::renderPage(Page page, Nodes_t& out)
{
    // ex WImperialStatisticsWindow::drawPage(int n)
    switch (page) {
     case TotalsPage:
        renderTotalsPage(out);
        break;
     case MineralsPage:
        renderMineralsPage(out, m_options[page]);
        break;
     case PlanetsPage:
        renderPlanetsPage(out, m_options[page]);
        break;
     case ColonyPage:
        renderColonyPage(out, m_options[page]);
        break;
     case StarbasePage:
        renderStarbasePage(out, m_options[page]);
        break;
     case StarshipPage:
     case CapitalPage:
        renderStarshipPage(out, m_options[page], page == StarshipPage);
        break;
     case StarchartPage:
        renderStarchartPage(out);
        break;
     case WeaponsPage:
        renderWeaponsPage(out, m_options[page]);
        break;
    }
}

void
game::map::info::Browser::renderPageOptions(Page page, util::StringList& out)
{
    // ex WImperialStatisticsWindow::doOptions(UIBaseWidget& sender), part
    afl::string::Translator& tx = m_session.translator();
    uint8_t hi, lo;
    switch (page) {
     case TotalsPage:
        break;

     case MineralsPage:
        hi = m_options[page] & Minerals_ShowMask;
        lo = m_options[page] & Minerals_SortMask;
        out.add(hi + Minerals_SortByTotal,     tx("Sort by total amount"));
        out.add(hi + Minerals_SortByMined,     tx("Sort by mined amount"));
        out.add(lo + Minerals_ShowOnlyN,       tx("Show only Neutronium"));
        out.add(lo + Minerals_ShowOnlyT,       tx("Show only Tritanium"));
        out.add(lo + Minerals_ShowOnlyD,       tx("Show only Duranium"));
        out.add(lo + Minerals_ShowOnlyM,       tx("Show only Molybdenum"));
        out.add(lo,                            tx("Show all 4 minerals"));
        break;

     case PlanetsPage:
        out.add(Planets_SortByRace,            tx("Sort by native race"));
        out.add(Planets_SortByNumber,          tx("Sort by number of planets"));
        out.add(Planets_SortByTotalPop,        tx("Sort by total population"));
        break;

     case ColonyPage:
        lo = static_cast<uint8_t>(m_options[page] & ~Colony_ShowMask);
        out.add(lo,                            tx("Show all info"));
        out.add(lo + Colony_ShowOnlyColonists, tx("Show only Colonists"));
        out.add(lo + Colony_ShowOnlySupplies,  tx("Show only Supplies"));
        out.add(lo + Colony_ShowOnlyMoney,     tx("Show only Money"));
        break;

     case StarbasePage:
        lo = m_options[page] & Ships_SortMask;
        hi = m_options[page] & Ships_HideTop;
        addSortOrders(out, hi);
        if (hi != 0) {
            out.add(lo,                        tx("Show all info"));
        } else {
            out.add(lo + Ships_HideTop,        tx("Show only ship list"));
        }
        break;

     case StarshipPage:
     case CapitalPage:
        lo = m_options[page] & Ships_SortMask;
        hi = m_options[page] & Ships_HideTop;
        addSortOrders(out, hi);
        if (hi != 0) {
            out.add(lo,                        tx("Show all info"));
        } else {
            out.add(lo + Ships_HideTop,        tx("Show only hull list"));
        }
        break;

     case StarchartPage:
        break;

     case WeaponsPage:
        out.add(Weapons_ShowOnlyBeams,         tx("Show only beams"));
        out.add(Weapons_ShowOnlyTorpedoes,     tx("Show only torpedoes"));
        out.add(Weapons_ShowOnlyRest,          tx("Show only rest"));
        out.add(Weapons_ShowEverything,        tx("Show all info"));
        break;
    }
}

void
game::map::info::Browser::renderTotalsPage(Nodes_t& out)
{
    // Heading
    afl::string::Translator& tx = m_session.translator();
    addHeading(out, tx("Totals"));

    // Environment
    const ShipList& shipList = game::actions::mustHaveShipList(m_session);
    const Root& root = game::actions::mustHaveRoot(m_session);
    const Universe& univ = universe();
    util::NumberFormatter fmt = root.userConfiguration().getNumberFormatter();

    // Compute data
    const TotalsInfo t = computeTotalsInfo(univ, root.hostConfiguration(), root.hostVersion());

    // Render
    renderUnitTotals(makeTable(out), univ, fmt, tx);
    renderMineralTotals(makeTable(out), t, shipList, fmt, tx);
    renderColoniesTotals(makeTable(out), t, fmt, tx);
    renderProductionTotals(makeTable(out), t, shipList, fmt, tx);
}

void
game::map::info::Browser::renderMineralsPage(Nodes_t& out, PageOptions_t opts)
{
    // ex drawMineralsPage(RichDocument& d, GUniverse& univ, int opts)
    // Heading
    afl::string::Translator& tx = m_session.translator();
    addHeading(out, tx("Minerals"));

    // Environment
    const ShipList& shipList = game::actions::mustHaveShipList(m_session);
    util::NumberFormatter fmt = game::actions::mustHaveRoot(m_session).userConfiguration().getNumberFormatter();

    // Render according to options
    bool sortByTotal = (opts & Minerals_SortMask) == Minerals_SortByTotal;
    switch (opts & Minerals_ShowMask) {
     case Minerals_ShowOnlyN:
        renderTopMineralPlanets(makeTable(out), universe(), sortByTotal, 24, Element::Neutronium, shipList, fmt, tx);
        break;
     case Minerals_ShowOnlyT:
        renderTopMineralPlanets(makeTable(out), universe(), sortByTotal, 24, Element::Tritanium,  shipList, fmt, tx);
        break;
     case Minerals_ShowOnlyD:
        renderTopMineralPlanets(makeTable(out), universe(), sortByTotal, 24, Element::Duranium,   shipList, fmt, tx);
        break;
     case Minerals_ShowOnlyM:
        renderTopMineralPlanets(makeTable(out), universe(), sortByTotal, 24, Element::Molybdenum, shipList, fmt, tx);
        break;
     default:
        renderTopMineralPlanets(makeTable(out), universe(), sortByTotal,  5, Element::Neutronium, shipList, fmt, tx);
        renderTopMineralPlanets(makeTable(out), universe(), sortByTotal,  5, Element::Tritanium,  shipList, fmt, tx);
        renderTopMineralPlanets(makeTable(out), universe(), sortByTotal,  5, Element::Duranium,   shipList, fmt, tx);
        renderTopMineralPlanets(makeTable(out), universe(), sortByTotal,  5, Element::Molybdenum, shipList, fmt, tx);
        break;
    }
}

void
game::map::info::Browser::renderPlanetsPage(Nodes_t& out, PageOptions_t opts)
{
    // Heading
    afl::string::Translator& tx = m_session.translator();
    addHeading(out, tx("Planets"));

    // Environment
    const Root& root = game::actions::mustHaveRoot(m_session);
    const Universe& univ = universe();
    util::NumberFormatter fmt = root.userConfiguration().getNumberFormatter();

    // Render
    renderPlanetNumber(makeTable(out), univ, fmt, tx);
    if (!univ.playedPlanets().isEmpty()) {
        renderPlanetNativeSummary(makeTable(out), univ, opts, fmt, tx);
        renderPlanetClimateSummary(makeTable(out), univ, fmt, tx);
        renderPlanetDefenseSummary(makeTable(out), univ, root.hostConfiguration(), fmt, tx);
    }
}

void
game::map::info::Browser::renderColonyPage(Nodes_t& out, PageOptions_t opts)
{
    // ex drawColonyPage(RichDocument& d, GUniverse& univ, int opts)
    afl::string::Translator& tx = m_session.translator();
    addHeading(out, tx("Colony"));

    // Environment
    const ShipList& shipList = game::actions::mustHaveShipList(m_session);
    util::NumberFormatter fmt = game::actions::mustHaveRoot(m_session).userConfiguration().getNumberFormatter();

    // Render according to options
    switch (opts & Colony_ShowMask) {
     case Colony_ShowOnlyColonists:
        renderTopResourcePlanets(makeTable(out), universe(), 24, Element::Colonists, shipList, fmt, tx);
        break;

     case Colony_ShowOnlySupplies:
        renderTopResourcePlanets(makeTable(out), universe(), 24, Element::Supplies,  shipList, fmt, tx);
        break;

     case Colony_ShowOnlyMoney:
        renderTopResourcePlanets(makeTable(out), universe(), 24, Element::Money,     shipList, fmt, tx);
        break;

     default:
        renderTopResourcePlanets(makeTable(out), universe(),  5, Element::Colonists, shipList, fmt, tx);
        renderTopResourcePlanets(makeTable(out), universe(),  5, Element::Supplies,  shipList, fmt, tx);
        renderTopResourcePlanets(makeTable(out), universe(),  5, Element::Money,     shipList, fmt, tx);
        break;
    }
}

void
game::map::info::Browser::renderStarbasePage(Nodes_t& out, PageOptions_t opts)
{
    // Heading
    afl::string::Translator& tx = m_session.translator();
    addHeading(out, tx("Starbases"));

    // Environment
    const Root& root = game::actions::mustHaveRoot(m_session);
    const Universe& univ = universe();
    const ShipList& shipList = game::actions::mustHaveShipList(m_session);
    util::NumberFormatter fmt = root.userConfiguration().getNumberFormatter();

    // Do it
    if ((opts & Ships_HideTop) == 0) {
        renderStarbaseSummary(makeTable(out), universe(), fmt, tx);
    }
    if (!univ.playedBases().isEmpty()) {
        renderStarbaseShipBuildSummary(makeTable(out), univ, opts & Ships_SortMask, shipList, root.hostConfiguration(), fmt, tx);
    }
}

void
game::map::info::Browser::renderStarshipPage(Nodes_t& out, PageOptions_t opts, bool withFreighters)
{
    // Heading
    afl::string::Translator& tx = m_session.translator();
    addHeading(out, withFreighters ? tx("Starships") : tx("Capital Ships"));

    // Environment
    const Root& root = game::actions::mustHaveRoot(m_session);
    const Game& g = game::actions::mustHaveGame(m_session);
    const ShipList& shipList = game::actions::mustHaveShipList(m_session);
    const HostConfiguration& config = root.hostConfiguration();
    const Universe& univ = universe();
    util::NumberFormatter fmt = root.userConfiguration().getNumberFormatter();

    // Do it
    if ((opts & Ships_HideTop) == 0) {
        renderShipSummary(makeTable(out), univ, withFreighters, g.shipScores(), shipList, root.hostConfiguration(), fmt, tx);
        if (config[HostConfiguration::NumExperienceLevels]() > 0) {
            renderShipExperienceSummary(makeTable(out), univ, withFreighters, g.shipScores(), root.hostConfiguration(), fmt, tx);
        }
    }
    if (!univ.playedShips().isEmpty()) {
        renderShipTypeSummary(makeTable(out), univ, opts & Ships_SortMask, withFreighters, shipList, fmt, tx);
    }
}

void
game::map::info::Browser::renderStarchartPage(Nodes_t& out)
{
    // ex drawStarchartPage(RichDocument& d, GUniverse& univ)
    // Heading
    afl::string::Translator& tx = m_session.translator();
    addHeading(out, tx("Starchart"));

    // Environment
    const Root& root = game::actions::mustHaveRoot(m_session);
    const Game& g = game::actions::mustHaveGame(m_session);
    const Turn* turn = g.getViewpointTurn().get();
    if (turn == 0) {
        throw Exception(Exception::eUser);
    }
    const Universe& univ = turn->universe();
    util::NumberFormatter fmt = root.userConfiguration().getNumberFormatter();

    // Acquire data
    const StarchartInfo t = computeStarchartInfo(*turn, g.teamSettings());

    // Render
    renderStarchartEmpireSummary(makeTable(out), t, univ, g.teamSettings(), fmt, tx);
    renderStarchartForeignSummary(makeTable(out), t, g.teamSettings(), root.playerList(), fmt, tx);
    renderUniversalFriendlyCode(makeTable(out), univ, g.teamSettings(), tx);
}

void
game::map::info::Browser::renderWeaponsPage(Nodes_t& out, PageOptions_t opts)
{
    // Heading
    afl::string::Translator& tx = m_session.translator();
    addHeading(out, tx("Weapons"));

    // Environment
    const Root& root = game::actions::mustHaveRoot(m_session);
    const ShipList& shipList = game::actions::mustHaveShipList(m_session);
    const Universe& univ = universe();
    util::NumberFormatter fmt = root.userConfiguration().getNumberFormatter();

    // Do it
    if (opts == 0 || opts == Weapons_ShowOnlyBeams) {
        renderBeamWeaponSummary(makeTable(out), univ, opts != 0, shipList, fmt, tx);
    }
    if (opts == 0 || opts == Weapons_ShowOnlyTorpedoes) {
        renderTorpedoWeaponSummary(makeTable(out), univ, opts != 0, shipList, fmt, tx);
    }
    if (opts == 0 || opts == Weapons_ShowOnlyRest) {
        renderOtherWeaponSummary(makeTable(out), univ, fmt, tx);
    }
}

void
game::map::info::Browser::addSortOrders(util::StringList& out, uint8_t hi)
{
    // ex addSortOrders(StringList& list, int hi)
    afl::string::Translator& tx = m_session.translator();
    out.add(hi + Ships_SortById,        tx("Sort ships by hull Id"));
    out.add(hi + Ships_SortByName,      tx("Sort ships by name"));
    out.add(hi + Ships_SortByAmount,    tx("Sort ships by count"));
    out.add(hi + Ships_SortByMass,      tx("Sort ships by hull mass"));
    out.add(hi + Ships_SortByTech,      tx("Sort ships by tech level"));
    out.add(hi + Ships_SortByCargo,     tx("Sort ships by cargo room"));
    out.add(hi + Ships_SortByEngines,   tx("Sort ships by number of engines"));
    out.add(hi + Ships_SortByBeams,     tx("Sort ships by maximum beams"));
    out.add(hi + Ships_SortByLaunchers, tx("Sort ships by maximum torpedo launchers"));
    out.add(hi + Ships_SortByBays,      tx("Sort ships by number of fighter bays"));
}

game::map::Universe&
game::map::info::Browser::universe()
{
    Turn* t = game::actions::mustHaveGame(m_session).getViewpointTurn().get();
    if (t == 0) {
        throw Exception(Exception::eUser);
    }
    return t->universe();
}
