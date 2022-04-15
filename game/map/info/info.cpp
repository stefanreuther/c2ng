/**
  *  \file game/map/info/info.cpp
  *  \brief Information Summary Rendering
  */

#include <vector>
#include <algorithm>
#include "game/map/info/info.hpp"
#include "afl/base/countof.hpp"
#include "afl/io/xml/tagnode.hpp"
#include "afl/io/xml/textnode.hpp"
#include "afl/string/format.hpp"
#include "game/cargospec.hpp"
#include "game/map/anyplanettype.hpp"
#include "game/map/anyshiptype.hpp"
#include "game/map/info/linkbuilder.hpp"
#include "game/map/minefield.hpp"
#include "game/map/planetformula.hpp"
#include "game/spec/basichullfunction.hpp"
#include "game/tables/nativeracename.hpp"
#include "game/tables/temperaturename.hpp"
#include "game/types.hpp"
#include "util/string.hpp"
#include "util/translation.hpp"
#include "util/vector.hpp"

using afl::io::xml::TagNode;
using afl::io::xml::TextNode;
using afl::string::Format;
using game::CargoSpec;
using game::Element;
using game::Id_t;
using game::SearchQuery;
using game::config::HostConfiguration;
using game::map::Planet;
using game::map::Ship;
using game::map::info::LinkBuilder;
using game::spec::BasicHullFunction;
using game::spec::Beam;
using game::spec::Hull;
using game::spec::TorpedoLauncher;

namespace {
    // Format for history counts: " (+n)" if nonzero, empty otherwise
    const char*const HISTORY_FMT = "%!d%!0{(+%0$d)%}";

    // Indentation (&nbsp; + space)
    const char*const INDENT = "\xC2\xA0 ";

    // Prefix for filtering own units
    // FIXME: this is used the same way as in PCC1/2: not consistently, and should probably by 'Played And '.
    const char*const LINK_PREFIX = "Owner$=My.Race$ And ";



    /*
     *  XML Utils
     */

    TagNode& makeTag(TagNode& out, const String_t& tagName)
    {
        TagNode* p = new TagNode(tagName);
        out.addNewChild(p);
        return *p;
    }

    void makeText(TagNode& out, const String_t& text)
    {
        out.addNewChild(new TextNode(text));
    }

    TagNode& makeRow(TagNode& tab)
    {
        return makeTag(tab, "tr");
    }

    TagNode& makeLeftCell(TagNode& out)
    {
        return makeTag(out, "td");
    }

    TagNode& makeLeftCell(TagNode& out, int width)
    {
        TagNode& p = makeLeftCell(out);
        p.setAttribute("width", Format("%d", width));
        return p;
    }

    TagNode& makeRightCell(TagNode& out)
    {
        TagNode& p = makeTag(out, "td");
        p.setAttribute("align", "right");
        return p;
    }

    TagNode& makeRightCell(TagNode& out, int width)
    {
        TagNode& p = makeRightCell(out);
        p.setAttribute("width", Format("%d", width));
        return p;
    }

    TagNode& makeWhite(TagNode& out)
    {
        TagNode& node = makeTag(out, "font");
        node.setAttribute("color", "white");
        return node;
    }

    TagNode& makeGreen(TagNode& out)
    {
        TagNode& node = makeTag(out, "font");
        node.setAttribute("color", "green");
        return node;
    }

    void makeLink(TagNode& out, const String_t& text, const String_t& target)
    {
        if (text.empty() || target.empty()) {
            makeText(out, text);
        } else {
            TagNode& a = makeTag(out, "a");
            a.setAttribute("href", target);
            makeText(a, text);
        }
    }

    void makeOptionalLink(TagNode& out, const String_t& text, const String_t& target, bool flag)
    {
        if (flag) {
            makeLink(out, text, target);
        } else {
            makeText(out, text);
        }
    }

    void makeTwoColumnRow(TagNode& tab, const String_t& name, const String_t& value)
    {
        TagNode& row = makeRow(tab);
        makeText(makeLeftCell(row), name);
        makeText(makeGreen(makeRightCell(row)), value);
    }

    void makeThreeColumnRow(TagNode& tab, const String_t& name, const String_t& value, const String_t& unit)
    {
        TagNode& row = makeRow(tab);
        makeText(makeLeftCell(row), name);
        makeText(makeGreen(makeRightCell(row)), value);
        makeText(makeGreen(makeLeftCell(row)), unit);
    }

    void makeTwoColumnTextRow(TagNode& tab, const String_t& name, const String_t& value)
    {
        TagNode& row = makeRow(tab);
        makeText(makeLeftCell(row), name);
        makeText(makeGreen(makeLeftCell(row)), value);
    }

    void makeTwoColumnTextRowIfNonzero(TagNode& tab, const String_t& name, int32_t value, util::NumberFormatter& fmt)
    {
        // ex showIfNonzero
        if (value != 0) {
            makeTwoColumnTextRow(tab, name, fmt.formatNumber(value));
        }
    }

    /*
     *  Data acquisition
     */

    struct Pair {
        game::Id_t id;
        int32_t value;

        Pair(game::Id_t id, int32_t value)
            : id(id), value(value)
            { }
    };
    typedef std::vector<Pair> Pairs_t;

    struct SortDescending {
        bool operator()(const Pair& a, const Pair& b) const
            { return a.value > b.value; }
    };

    void sort(Pairs_t& pairs)
    {
        std::sort(pairs.begin(), pairs.end(), SortDescending());
    }

    /*
     *  Totals Page
     */

    struct TypeMap {
        CargoSpec::Type csType : 8;
        Element::Type eleType : 8;
    };
    static const TypeMap CARGO_TYPES[] = {
        // ex ctypes - first four are minerals
        CargoSpec::Neutronium, Element::Neutronium,
        CargoSpec::Tritanium,  Element::Tritanium,
        CargoSpec::Duranium,   Element::Duranium,
        CargoSpec::Molybdenum, Element::Molybdenum,
        CargoSpec::Supplies,   Element::Supplies,
        CargoSpec::Colonists,  Element::Colonists,
        CargoSpec::Money,      Element::Money,
    };
    const size_t NUM_MINERALS = 4;

    /** Count minerals from all our planets. */
    void sumMinedMinerals(CargoSpec& min, const game::map::PlayedPlanetType& type)
    {
        for (Id_t i = type.findNextIndex(0); i != 0; i = type.findNextIndex(i)) {
            if (const Planet* pl = type.getObjectByIndex(i)) {
                for (size_t j = 0; j < countof(CARGO_TYPES); ++j) {
                    min.add(CARGO_TYPES[j].csType, pl->getCargo(CARGO_TYPES[j].eleType).orElse(0));
                }
            }
        }
    }

    /* Count minerals from all our ships. */
    void sumShipMinerals(CargoSpec& min, const game::map::PlayedShipType& type)
    {
        for (Id_t i = type.findNextIndex(0); i != 0; i = type.findNextIndex(i)) {
            if (const Ship* sh = type.getObjectByIndex(i)) {
                for (size_t j = 0; j < countof(CARGO_TYPES); ++j) {
                    Element::Type eleType = CARGO_TYPES[j].eleType;
                    int32_t amount = sh->getCargo(eleType).orElse(0);
                    if (sh->isTransporterActive(Ship::UnloadTransporter)) {
                        amount += sh->getTransporterCargo(Ship::UnloadTransporter, eleType).orElse(0);
                    }
                    if (sh->isTransporterActive(Ship::TransferTransporter)) {
                        amount += sh->getTransporterCargo(Ship::TransferTransporter, eleType).orElse(0);
                    }
                    min.add(CARGO_TYPES[j].csType, amount);
                }
            }
        }
    }

    /* Count ground minerals, production, and max production. */
    void sumGroundMineralsAndProduction(CargoSpec& ground,
                                        CargoSpec& produced,
                                        CargoSpec& maxProduced,
                                        const game::map::PlayedPlanetType& type,
                                        const HostConfiguration& config,
                                        const game::HostVersion& host)
    {
        for (Id_t i = type.findNextIndex(0); i != 0; i = type.findNextIndex(i)) {
            if (const Planet* pl = type.getObjectByIndex(i)) {
                int owner = 0;
                pl->getOwner(owner);

                /* Mining */
                for (size_t i = 0; i < NUM_MINERALS; ++i) {
                    const Element::Type eleType = CARGO_TYPES[i].eleType;
                    const CargoSpec::Type csType = CARGO_TYPES[i].csType;
                    const int32_t gnd = pl->getOreGround(eleType).orElse(0);
                    ground.add(csType, gnd);

                    const int32_t capacity = getMiningCapacity(*pl, config, host, eleType, pl->getNumBuildings(game::MineBuilding).orElse(0)).orElse(0);
                    maxProduced.add(csType, capacity);

                    produced.add(csType, std::min(capacity, gnd));
                }

                /* Bovinoids */
                produced.add(CargoSpec::Supplies, getBovinoidSupplyContributionLimited(*pl, config, host).orElse(0));
                maxProduced.add(CargoSpec::Supplies, getBovinoidSupplyContribution(*pl, config, host).orElse(0));

                /* Factories */
                int32_t fact = pl->getNumBuildings(game::FactoryBuilding).orElse(0);
                if (host.isPHost()) {
                    fact = fact * config[HostConfiguration::ProductionRate](owner) / 100;
                }
                produced.add(CargoSpec::Supplies, fact);
                maxProduced.add(CargoSpec::Supplies, fact);

                /* Taxes */
                int ctax = pl->getColonistTax().orElse(0);
                int32_t due = getColonistDue(*pl, config, host, ctax).orElse(0);
                int32_t rem;
                int32_t pay = getColonistDueLimited(*pl, config, host, ctax, rem).orElse(0);
                if (pl->getNatives().orElse(0) > 0) {
                    int ntax = pl->getNativeTax().orElse(0);
                    due += getNativeDue(*pl, config, host, ntax).orElse(0);
                    pay += getNativeDueLimited(*pl, config, host, ntax, rem).orElse(0);
                }
                produced.add(CargoSpec::Money, pay);
                maxProduced.add(CargoSpec::Money, due);
            }
        }
    }

    /* Count structures. */
    void sumStructures(game::map::info::TotalsInfo& t, const game::map::PlayedPlanetType& type)
    {
        for (Id_t i = type.findNextIndex(0); i != 0; i = type.findNextIndex(i)) {
            if (const Planet* pl = type.getObjectByIndex(i)) {
                t.totalMines     += pl->getNumBuildings(game::MineBuilding).orElse(0);
                t.totalFactories += pl->getNumBuildings(game::FactoryBuilding).orElse(0);
                t.totalDefense   += pl->getNumBuildings(game::DefenseBuilding).orElse(0);
            }
        }
    }

    /* Show a line containing two cargo values. */
    void renderCargoPair(TagNode& tab, Element::Type eleType, CargoSpec::Type csType, const CargoSpec& a, const CargoSpec& b,
                         const game::spec::ShipList& shipList, util::NumberFormatter fmt, afl::string::Translator& tx)
    {
        // ex addCargoPair(RichDocument& d, GCargoType el, const GCargoSpec& a, const GCargoSpec& b)
        TagNode& row = makeRow(tab);
        makeText(makeLeftCell(row), Element::getName(eleType, tx, shipList) + ":");
        makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(a.get(csType)));
        makeText(makeGreen(makeLeftCell(row)), Element::getUnit(eleType, tx, shipList));
        makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(b.get(csType)));
        makeText(makeGreen(makeLeftCell(row)), Element::getUnit(eleType, tx, shipList));
    }

    /*
     *  Hull List
     */

    bool compareHulls(const int order, const util::Vector<int, int>& counts, const int a, const int b, const game::spec::ShipList& shipList)
    {
        const Hull* ha = shipList.hulls().get(a);
        const Hull* hb = shipList.hulls().get(b);
        if (ha == 0 || hb == 0) {
            return a < b;
        } else {
            switch (order) {
             case game::map::info::Ships_SortByAmount:
                return counts.get(a) > counts.get(b);
             case game::map::info::Ships_SortByName:
                return ha->getName(shipList.componentNamer()) < hb->getName(shipList.componentNamer());
             case game::map::info::Ships_SortById:
                return a < b;
             case game::map::info::Ships_SortByMass:
                return ha->getMass() > hb->getMass();
             case game::map::info::Ships_SortByTech:
                return ha->getTechLevel() > hb->getTechLevel();
             case game::map::info::Ships_SortByCargo:
                return ha->getMaxCargo() > hb->getMaxCargo();
             case game::map::info::Ships_SortByEngines:
                return ha->getNumEngines() > hb->getNumEngines();
             case game::map::info::Ships_SortByBeams:
                return ha->getMaxBeams() > hb->getMaxBeams();
             case game::map::info::Ships_SortByLaunchers:
                return ha->getMaxLaunchers() > hb->getMaxLaunchers();
             case game::map::info::Ships_SortByBays:
                return ha->getNumBays() > hb->getNumBays();
             default:
                return a < b;
            }
        }
    }

    void renderHullList(TagNode& tab, util::Vector<int, int>& counts, int order, const char* linkFormat, const game::spec::ShipList& shipList, util::NumberFormatter& fmt,
                        const LinkBuilder& link, SearchQuery::SearchObjects_t searchObj)
    {
        // ex drawHullList(RichDocument& d, std::vector<int>& counts, const int order, const char* query_fmt)
        // Same stupid algorithm as in PCC 1.x - selection sort.
        while (1) {
            int hid = 0;
            for (int i = 1, n = counts.size(); i < n; ++i) {
                if (counts.get(i) > 0) {
                    if (hid == 0 || compareHulls(order, counts, i, hid, shipList)) {
                        hid = i;
                    }
                }
            }
            if (hid != 0) {
                if (const Hull* h = shipList.hulls().get(hid)) {
                    const int n = counts.get(hid);
                    TagNode& row = makeRow(tab);
                    makeOptionalLink(makeLeftCell(row), h->getName(shipList.componentNamer()),
                                     link.makeSearchLink(SearchQuery(SearchQuery::MatchTrue, searchObj, Format(linkFormat, hid))),
                                     n > 0);
                    makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(n));
                }
                counts.set(hid, 0);
            } else {
                break;
            }
        }
    }

    String_t makeQuerySuffix(bool withFreighters)
    {
        String_t querySuffix = " And Owner$=My.Race$";
        if (!withFreighters) {
            querySuffix += " And Type.Short<>\"F\"";
        }
        return querySuffix;
    }


    /*
     *  Starbase Summary
     */

    bool hasAnyStorage(const Planet& pl, game::TechLevel level)
    {
        // TODO: this could be a method of BaseStorage/Planet
        for (int i = 1, n = pl.getBaseStorageLimit(level); i < n; ++i) {
            if (pl.getBaseStorage(level, i).orElse(0) > 0) {
                return true;
            }
        }
        return false;
    }

    /*
     *  Starchart Summary
     */

    void computeSize(std::vector<int>& xs, int wrap, int (&size)[2])
    {
        if (xs.empty()) {
            /* no planets */
            size[0] = size[1] = -1;
        } else {
            /* sort for initial (and, for non-wrap, only) estimate */
            std::sort(xs.begin(), xs.end());
            size[0] = xs.front();
            size[1] = xs.back();

            if (wrap != 0) {
                /* With wrap, we probably have a cluster yielding maximum size
                   in the above computation:
                         . . .. . ..     . .
                         |                 |
                         |                 |
                         min               max
                   We're now looking for a new set
                         . . .. . ..     . .
                                   |     |
                                   |     |
                                   max   min
                   giving a better estimate about empire size. */
                int est = size[1] - size[0];
                for (std::vector<int>::size_type i = 1; i < xs.size(); ++i) {
                    int t = xs[i-1] - xs[i] + wrap;
                    if (t < est) {
                        size[0] = xs[i];
                        size[1] = xs[i-1];
                        est = t;
                    }
                }
            }
        }
    }

    void showRange(TagNode& tab, String_t ttl, int (&size)[2], int wrap, afl::string::Translator& tx)
    {
        String_t left = INDENT;
        String_t right;
        if (size[0] == size[1]) {
            left += Format(tx("%s Location:"), ttl);
            right = Format(tx("at %d"), size[0]);
        } else {
            left += Format(tx("%s Range:"), ttl);
            if (size[0] < size[1]) {
                right = Format(tx("%d ly from %d to %d"), size[1]-size[0]+1, size[0], size[1]);
            } else {
                right = Format(tx("%d ly from %d to %d"), wrap+size[1]-size[0]+1, size[0], size[1]);
            }
        }
        makeTwoColumnTextRow(tab, left, right);
    }
}

/*
 *  Public Entry Points - Computation
 */

// Compute TotalsInfo structure.
game::map::info::TotalsInfo
game::map::info::computeTotalsInfo(const Universe& univ, const game::config::HostConfiguration& config, const game::HostVersion& host)
{
    TotalsInfo out;
    sumMinedMinerals(out.availableResources, univ.playedPlanets());
    sumShipMinerals(out.availableResources, univ.playedShips());
    sumGroundMineralsAndProduction(out.groundResources, out.producedResources, out.maxProducedResources, univ.playedPlanets(), config, host);
    sumStructures(out, univ.playedPlanets());
    return out;
}

// Compute StarchartInfo structure.
game::map::info::StarchartInfo
game::map::info::computeStarchartInfo(const Turn& turn, const TeamSettings& teams)
{
    // ex drawStarchartPage(RichDocument& d, GUniverse& univ), part
    StarchartInfo out;
    const Universe& univ = turn.universe();

    // Planets
    AnyPlanetType ptype(const_cast<Universe&>(univ).planets());
    for (Id_t pid = ptype.findNextIndex(0); pid != 0; pid = ptype.findNextIndex(pid)) {
        if (const Planet* pl = ptype.getObjectByIndex(pid)) {
            // Accout total
            ++out.totalPlanets;

            // Account owners
            int owner;
            if (pl->getOwner(owner)) {
                if (pl->getHistoryTimestamp(Planet::ColonistTime) == turn.getTurnNumber()) {
                    out.numCurrentPlanets.set(owner, out.numCurrentPlanets.get(owner) + 1);
                } else {
                    out.numOldPlanets.set(owner, out.numOldPlanets.get(owner) + 1);
                }
            }
        }
    }

    // Ships
    AnyShipType stype(const_cast<Universe&>(univ).ships());
    for (Id_t sid = stype.findNextIndex(0); sid != 0; sid = stype.findNextIndex(sid)) {
        const Ship* sh = stype.getObjectByIndex(sid);
        int owner;
        if (sh != 0 && sh->getOwner(owner)) {
            if (sh->getShipKind() == Ship::HistoryShip || sh->getShipKind() == Ship::GuessedShip) {
                out.numOldShips.set(owner, out.numOldShips.get(owner) + 1);
            } else {
                out.numCurrentShips.set(owner, out.numCurrentShips.get(owner) + 1);
                if (sh->getShipKind() == Ship::CurrentTarget) {
                    ++out.totalTargets;
                }
            }
        }
    }

    // Minefields
    const MinefieldType& mfs = univ.minefields();
    for (Id_t mid = mfs.findNextIndex(0); mid != 0; mid = mfs.findNextIndex(mid)) {
        const Minefield* mf = mfs.getObjectByIndex(mid);
        int owner;
        if (mf != 0 && mf->getOwner(owner)) {
            out.numMinefields.set(owner, out.numMinefields.get(owner) + 1);
            switch (teams.getPlayerRelation(owner)) {
             case TeamSettings::ThisPlayer:
                ++out.numOwnMinefields;
                break;
             case TeamSettings::AlliedPlayer:
                ++out.numTeamMinefields;
                break;
             case TeamSettings::EnemyPlayer:
                ++out.numEnemyMinefields;
                break;
            }
        }
    }

    return out;
}

/*
 *  Public Entry Points - Rendering
 */

// Render unit totals (part of TotalsPage).
void
game::map::info::renderUnitTotals(TagNode& tab, const Universe& univ, util::NumberFormatter fmt, afl::string::Translator& tx)
{
    // ex drawTotalsPage(RichDocument& d, GUniverse& univ), part
    {
        TagNode& row = makeRow(tab);
        makeText(makeLeftCell(row, 10), tx("Planets:"));
        makeText(makeGreen(makeRightCell(row, 6)), fmt.formatNumber(univ.playedPlanets().countObjects()));
    }
    makeTwoColumnRow(tab, tx("Starbases:"), fmt.formatNumber(univ.playedBases().countObjects()));
    makeTwoColumnRow(tab, tx("Starships:"), fmt.formatNumber(univ.playedShips().countObjects()));
    makeTwoColumnRow(tab, INDENT + tx("Capital ships:"), fmt.formatNumber(univ.playedShips().countCapitalShips()));
}

// Render mineral totals (part of TotalsPage).
void
game::map::info::renderMineralTotals(TagNode& tab, const TotalsInfo& t, const game::spec::ShipList& shipList, util::NumberFormatter fmt, afl::string::Translator& tx)
{
    // ex drawTotalsPage(RichDocument& d, GUniverse& univ), part
    // Minerals table (5-column table: type, available+unit, ground+unit)
    {
        TagNode& row = makeRow(tab);
        makeText(makeWhite(makeLeftCell(row, 10)), tx("Minerals"));
        makeText(makeRightCell(row, 6), tx("(available)"));
        makeText(makeLeftCell(row, 2), "");
        makeText(makeRightCell(row, 6), tx("(ground)"));
        makeText(makeLeftCell(row, 2), "");
    }
    renderCargoPair(tab, Element::Neutronium, CargoSpec::Neutronium, t.availableResources, t.groundResources, shipList, fmt, tx);
    renderCargoPair(tab, Element::Tritanium,  CargoSpec::Tritanium,  t.availableResources, t.groundResources, shipList, fmt, tx);
    renderCargoPair(tab, Element::Duranium,   CargoSpec::Duranium,   t.availableResources, t.groundResources, shipList, fmt, tx);
    renderCargoPair(tab, Element::Molybdenum, CargoSpec::Molybdenum, t.availableResources, t.groundResources, shipList, fmt, tx);
}

// Render colonies totals (part of TotalsPage).
void
game::map::info::renderColoniesTotals(TagNode& tab, const TotalsInfo& t, util::NumberFormatter fmt, afl::string::Translator& tx)
{
    // ex drawTotalsPage(RichDocument& d, GUniverse& univ), part
    // Colonies table (3-column table: type, amount+unit)
    {
        TagNode& row = makeRow(tab);
        makeText(makeWhite(makeLeftCell(row, 10)), tx("Colonies"));
        makeText(makeRightCell(row, 6), "");
        makeText(makeLeftCell(row, 2), "");
    }
    makeThreeColumnRow(tab, tx("Colonists:"),     fmt.formatPopulation(t.availableResources.get(CargoSpec::Colonists)), "");
    makeThreeColumnRow(tab, tx("Money:"),         fmt.formatNumber(t.availableResources.get(CargoSpec::Money)),         tx("mc"));
    makeThreeColumnRow(tab, tx("Supplies:"),      fmt.formatNumber(t.availableResources.get(CargoSpec::Supplies)),      tx("kt"));
    makeThreeColumnRow(tab, tx("Mineral Mines:"), fmt.formatNumber(t.totalMines),                                       "");
    makeThreeColumnRow(tab, tx("Factories:"),     fmt.formatNumber(t.totalFactories),                                   "");
    makeThreeColumnRow(tab, tx("Defense Posts:"), fmt.formatNumber(t.totalDefense),                                     "");
}

// Render production totals (part of TotalsPage).
void
game::map::info::renderProductionTotals(TagNode& tab, const TotalsInfo& t, const game::spec::ShipList& shipList, util::NumberFormatter fmt, afl::string::Translator& tx)
{
    // ex drawTotalsPage(RichDocument& d, GUniverse& univ), part
    // Production table (5-column table: type, produced+unit, max+unit)
    {
        TagNode& row = makeRow(tab);
        makeText(makeWhite(makeLeftCell(row, 10)), tx("Production"));
        makeText(makeRightCell(row, 6), "");
        makeText(makeLeftCell(row, 2), "");
        makeText(makeRightCell(row, 6), tx("(max)"));
        makeText(makeLeftCell(row, 2), "");
    }
    renderCargoPair(tab, Element::Neutronium, CargoSpec::Neutronium, t.producedResources, t.maxProducedResources, shipList, fmt, tx);
    renderCargoPair(tab, Element::Tritanium,  CargoSpec::Tritanium,  t.producedResources, t.maxProducedResources, shipList, fmt, tx);
    renderCargoPair(tab, Element::Duranium,   CargoSpec::Duranium,   t.producedResources, t.maxProducedResources, shipList, fmt, tx);
    renderCargoPair(tab, Element::Molybdenum, CargoSpec::Molybdenum, t.producedResources, t.maxProducedResources, shipList, fmt, tx);
    renderCargoPair(tab, Element::Money,      CargoSpec::Money,      t.producedResources, t.maxProducedResources, shipList, fmt, tx);
    renderCargoPair(tab, Element::Supplies,   CargoSpec::Supplies,   t.producedResources, t.maxProducedResources, shipList, fmt, tx);
}

// Render table of top-mineral planets.
void
game::map::info::renderTopMineralPlanets(TagNode& tab, const Universe& univ, bool sortByTotal, size_t limit, Element::Type el, const game::spec::ShipList& shipList, util::NumberFormatter fmt, afl::string::Translator& tx, const LinkBuilder& link)
{
    // ex showTopMinerals(RichDocument& d, GUniverse& univ, const int opts, const int count, const GCargoType el)
    // Table:
    //     16 em       8 em  2em    8 em  2em
    //     Title      (total)       (mined)
    //     Name/Link   amount kt     amount kt

    // Header
    TagNode& firstRow = makeRow(tab);
    makeText(makeWhite(makeLeftCell(firstRow, 16)), Format(tx("Top %d %s Planets"), limit, Element::getName(el, tx, shipList)));
    makeText(makeRightCell(firstRow, 8), tx("(total)"));
    makeText(makeLeftCell(firstRow, 2), "");
    makeText(makeRightCell(firstRow, 8), tx("(mined)"));
    makeText(makeLeftCell(firstRow, 2), "");

    // Acquire data
    Pairs_t data;
    const PlayedPlanetType& type = univ.playedPlanets();
    for (Id_t i = type.findNextIndex(0); i != 0; i = type.findNextIndex(i)) {
        if (const Planet* pl = univ.planets().get(i)) {
            int32_t value = pl->getCargo(el).orElse(0);
            if (sortByTotal) {
                value += pl->getOreGround(el).orElse(0);
            }
            data.push_back(Pair(i, value));
        }
    }
    sort(data);

    // Render
    size_t n = std::min(limit, data.size());
    for (size_t i = 0; i < n; ++i) {
        Id_t pid = data[i].id;
        if (const Planet* pl = univ.planets().get(pid)) {
            int32_t mined = pl->getCargo(el).orElse(0);
            int32_t total = mined + pl->getOreGround(el).orElse(0);

            TagNode& row = makeRow(tab);
            makeLink(makeLeftCell(row),
                     Format("Planet #%d: %s", pid, pl->getName(tx)),     // This is getName(LongName) but we don't have an InterpreterInterface here
                     link.makePlanetLink(*pl));
            makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(total));
            makeText(makeGreen(makeLeftCell(row)), Element::getUnit(el, tx, shipList));
            makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(mined));
            makeText(makeGreen(makeLeftCell(row)), Element::getUnit(el, tx, shipList));
        }
    }
}

// Render table of top-resource planets.
void
game::map::info::renderTopResourcePlanets(TagNode& tab,
                                          const Universe& univ,
                                          size_t limit,
                                          Element::Type el,
                                          const game::spec::ShipList& shipList,
                                          util::NumberFormatter fmt,
                                          afl::string::Translator& tx,
                                          const LinkBuilder& link)
{
    // showTopResources(RichDocument& d, GUniverse& univ, const int opts, const int count, const GCargoType el)
    // Table:
    //     16 em        8 em
    //     Title       (unit)
    //     Name/Link   amount

    // Header
    TagNode& firstRow = makeRow(tab);
    makeText(makeWhite(makeLeftCell(firstRow, 16)), Format(tx("Top %d %s Planets"), limit, Element::getName(el, tx, shipList)));
    makeText(makeRightCell(firstRow, 8), Format("(%s)", Element::getUnit(el, tx, shipList)));

    // Acquire data
    // @diff PCC2 will not show a planet with 0 of any but I actually don't see a reason why not.
    Pairs_t data;
    const PlayedPlanetType& type = univ.playedPlanets();
    for (Id_t i = type.findNextIndex(0); i != 0; i = type.findNextIndex(i)) {
        if (const Planet* pl = univ.planets().get(i)) {
            int32_t value = pl->getCargo(el).orElse(0);
            data.push_back(Pair(i, value));
        }
    }
    sort(data);

    // Render
    size_t n = std::min(limit, data.size());
    for (size_t i = 0; i < n; ++i) {
        Id_t pid = data[i].id;
        if (const Planet* pl = univ.planets().get(pid)) {
            TagNode& row = makeRow(tab);
            makeLink(makeLeftCell(row),
                     Format("Planet #%d: %s", pid, pl->getName(tx)),     // This is getName(LongName) but we don't have an InterpreterInterface here
                     link.makePlanetLink(*pl));
            makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(data[i].value));
        }
    }
}

// Render number of planets (part of PlanetsPage).
void
game::map::info::renderPlanetNumber(TagNode& tab, const Universe& univ,
                                    util::NumberFormatter fmt,
                                    afl::string::Translator& tx)
{
    // ex drawPlanetsPage(RichDocument& d, GUniverse& univ, int opts), part
    TagNode& row = makeRow(tab);
    makeText(makeLeftCell(row, 15), tx("Total:"));
    makeText(makeGreen(makeRightCell(row, 3)), fmt.formatNumber(univ.playedPlanets().countObjects()));
}

// Render planet natives summary (part of PlanetsPage).
void
game::map::info::renderPlanetNativeSummary(TagNode& tab, const Universe& univ,
                                           uint8_t sortOrder,
                                           util::NumberFormatter fmt,
                                           afl::string::Translator& tx,
                                           const LinkBuilder& link)
{
    // ex drawPlanetsPage(RichDocument& d, GUniverse& univ, int opts), part
    // Data
    const int MAX_NATIVE_RACE = SiliconoidNatives;
    int nativePlanets[MAX_NATIVE_RACE+1];         // ex nativePlanets
    int32_t nativePop[MAX_NATIVE_RACE+1];     // ex nativePop
    std::fill_n(nativePlanets, countof(nativePlanets), 0);
    std::fill_n(nativePop, countof(nativePop), 0);

    // Count natives
    const PlayedPlanetType& type = univ.playedPlanets();
    for (Id_t pid = type.findNextIndex(0); pid != 0; pid = type.findNextIndex(pid)) {
        if (const Planet* pl = type.getObjectByIndex(pid)) {
            const int nr = pl->getNativeRace().orElse(0);
            if (nr >= 0 && nr <= MAX_NATIVE_RACE) {
                ++nativePlanets[nr];
                nativePop[nr] += pl->getNatives().orElse(0);
            }
        }
    }


    // Build sorted native list
    Pairs_t nativeIndex;
    for (int i = 0; i <= MAX_NATIVE_RACE; ++i) {
        if (sortOrder == Planets_SortByRace || nativePlanets[i] > 0) {
            nativeIndex.push_back(Pair(i, sortOrder == Planets_SortByNumber ? nativePlanets[i] : nativePop[i]));
        }
    }
    if (sortOrder != Planets_SortByRace) {
        sort(nativeIndex);
    }

    // Render
    {
        TagNode& row = makeRow(tab);
        makeText(makeWhite(makeLeftCell(row, 12)), tx("Natives"));
        makeText(makeRightCell(row, 6), tx("Planets"));
        makeText(makeRightCell(row, 8), tx("Natives"));
    }
    for (size_t i = 0; i < nativeIndex.size(); ++i) {
        const int race = nativeIndex[i].id;
        TagNode& row = makeRow(tab);
        makeOptionalLink(makeLeftCell(row),
                         game::tables::NativeRaceName(tx).get(race),
                         link.makeSearchLink(SearchQuery(SearchQuery::MatchTrue,
                                                         SearchQuery::SearchObjects_t(SearchQuery::SearchPlanets),
                                                         Format("Owner$=My.Race$ And Natives.Race$=%d", race))),
                         nativePlanets[race] > 0);
        makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(nativePlanets[race]));
        makeText(makeGreen(makeRightCell(row)), fmt.formatPopulation(nativePop[race]));
    }
}

// Render planet climate summary (part of PlanetsPage).
void
game::map::info::renderPlanetClimateSummary(TagNode& tab, const Universe& univ,
                                            util::NumberFormatter fmt,
                                            afl::string::Translator& tx,
                                            const LinkBuilder& link)
{
    // ex drawPlanetsPage(RichDocument& d, GUniverse& univ, int opts), part
    // Data
    const size_t NUM_CLIMATE_LEVELS = 5;
    static const int CLIMATE_LIMITS[NUM_CLIMATE_LEVELS] = { 14, 39, 64, 84, 100 };

    int climatePlanets[NUM_CLIMATE_LEVELS];
    std::fill_n(climatePlanets, countof(climatePlanets), 0);

    // Count planets
    const PlayedPlanetType& type = univ.playedPlanets();
    for (Id_t pid = type.findNextIndex(0); pid != 0; pid = type.findNextIndex(pid)) {
        if (const Planet* pl = type.getObjectByIndex(pid)) {
            const int temp = pl->getTemperature().orElse(0);
            size_t level = 0;
            while (level < NUM_CLIMATE_LEVELS-1 && temp > CLIMATE_LIMITS[level]) {
                ++level;
            }
            ++climatePlanets[level];
        }
    }

    // Render
    {
        TagNode& row = makeRow(tab);
        makeText(makeWhite(makeLeftCell(row, 12)), tx("Climate"));
        makeText(makeRightCell(row, 6), tx("Planets"));
    }
    for (size_t i = 0; i < NUM_CLIMATE_LEVELS; ++i) {
        TagNode& row = makeRow(tab);
        makeOptionalLink(makeLeftCell(row),
                         game::tables::TemperatureName(tx).get(CLIMATE_LIMITS[i]),
                         link.makeSearchLink(SearchQuery(SearchQuery::MatchTrue,
                                                         SearchQuery::SearchObjects_t(SearchQuery::SearchPlanets),
                                                         Format("Temp$>=%d And Temp$<=%d And Owner$=My.Race$",
                                                                i == 0 ? 0 : CLIMATE_LIMITS[i-1]+1,
                                                                CLIMATE_LIMITS[i]))),
                         climatePlanets[i] > 0);
        makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(climatePlanets[i]));
    }
}

// Render planet defense summary (part of PlanetsPage).
void
game::map::info::renderPlanetDefenseSummary(TagNode& tab, const Universe& univ, const game::config::HostConfiguration& config, util::NumberFormatter fmt, afl::string::Translator& tx,
                                            const LinkBuilder& link)
{
    const int dfu = config[HostConfiguration::DefenseForUndetectable]();

    // Data
    int nUndefended = 0, nVisible = 0;

    // Count stuff
    const PlayedPlanetType& type = univ.playedPlanets();
    for (Id_t pid = type.findNextIndex(0); pid != 0; pid = type.findNextIndex(pid)) {
        if (const Planet* pl = type.getObjectByIndex(pid)) {
            if (pl->getNumBuildings(DefenseBuilding).orElse(0) < dfu) {
                ++nVisible;
            }
            if (pl->getNumBuildings(DefenseBuilding).orElse(0) < 10) {
                ++nUndefended;
            }
        }
    }

    // Render
    {
        TagNode& row = makeRow(tab);
        makeOptionalLink(makeLeftCell(row, 15), tx("Nearly undefended:"),
                         link.makeSearchLink(SearchQuery(SearchQuery::MatchTrue,
                                                         SearchQuery::SearchObjects_t(SearchQuery::SearchPlanets),
                                                         "Defense<10 And Owner$=My.Race$")),
                         nUndefended > 0);
        makeText(makeGreen(makeRightCell(row, 3)), fmt.formatNumber(nUndefended));
    }
    {
        TagNode& row = makeRow(tab);
        makeOptionalLink(makeLeftCell(row, 15), tx("Visible by sensor scan:"),
                         link.makeSearchLink(SearchQuery(SearchQuery::MatchTrue,
                                                         SearchQuery::SearchObjects_t(SearchQuery::SearchPlanets),
                                                         Format("Defense<%d And Owner$=My.Race$", dfu))),
                         nVisible > 0);
        makeText(makeGreen(makeRightCell(row, 3)), fmt.formatNumber(nVisible));
    }
}

// Render starbase summary (part of StarbasePage).
void
game::map::info::renderStarbaseSummary(TagNode& tab,
                                       const Universe& univ,
                                       util::NumberFormatter fmt,
                                       afl::string::Translator& tx,
                                       const LinkBuilder& link)
{
    // drawStarbasePage(RichDocument& d, GUniverse& univ, int opts), part
    const size_t ITEMS = 8;
    static const char*const names[ITEMS] = {
        N_("Tech 10 Hulls"),
        N_("Tech 10 Engines"),
        N_("Tech 10 Beams"),
        N_("Tech 10 Torpedoes"),
        N_("Building a ship"),
        N_("Recycling a ship"),
        N_("Repairing a ship"),
        N_("Have parts in storage"),
    };
    static const char*const expr[ITEMS] = {
        "Tech.Hull=10",
        "Tech.Engine=10",
        "Tech.Beam=10",
        "Tech.Torpedo=10",
        "Build",
        "Shipyard.Action=\"Recycle\"",
        "Shipyard.Action=\"Fix\"",
        "Storage.Hulls(0)+Storage.Engines(0)+Storage.Beams(0)+Storage.Launchers(0)"
    };

    // Acquire data
    int counts[ITEMS];
    int nbases = 0;
    std::fill_n(counts, ITEMS, 0);

    const game::map::PlayedBaseType& type = univ.playedBases();
    for (Id_t pid = type.findNextIndex(0); pid != 0; pid = type.findNextIndex(pid)) {
        if (const Planet* pl = type.getObjectByIndex(pid)) {
            ++nbases;
            if (pl->getBaseTechLevel(HullTech).orElse(0) == 10) {
                ++counts[0];
            }
            if (pl->getBaseTechLevel(EngineTech).orElse(0) == 10) {
                ++counts[1];
            }
            if (pl->getBaseTechLevel(BeamTech).orElse(0) == 10) {
                ++counts[2];
            }
            if (pl->getBaseTechLevel(TorpedoTech).orElse(0) == 10) {
                ++counts[3];
            }

            const int hullType = pl->getBaseBuildOrderHullIndex().orElse(0);
            if (hullType > 0) {
                ++counts[4];
            }

            const int order = pl->getBaseShipyardAction().orElse(0);
            if (order == RecycleShipyardAction) {
                ++counts[5];
            }
            if (order == FixShipyardAction) {
                ++counts[6];
            }

            if (hasAnyStorage(*pl, HullTech) || hasAnyStorage(*pl, BeamTech) || hasAnyStorage(*pl, EngineTech) || hasAnyStorage(*pl, TorpedoTech)) {
                ++counts[7];
            }
        }
    }

    // Render
    {
        TagNode& row = makeRow(tab);
        makeText(makeLeftCell(row, 17), tx("Total:"));
        makeText(makeGreen(makeRightCell(row, 3)), fmt.formatNumber(nbases));
    }
    if (nbases != 0) {
        for (size_t i = 0; i < ITEMS; ++i) {
            TagNode& row = makeRow(tab);
            makeOptionalLink(makeLeftCell(row),
                             tx(names[i]) + ":",
                             link.makeSearchLink(SearchQuery(SearchQuery::MatchTrue,
                                                             SearchQuery::SearchObjects_t(SearchQuery::SearchBases),
                                                             expr[i])),
                             counts[i] > 0);
            makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(counts[i]));
        }
    }
}

// Render starbase ship building summary (part of StarbasePage).
void
game::map::info::renderStarbaseShipBuildSummary(TagNode& tab,
                                                const Universe& univ,
                                                uint8_t sortOrder,
                                                const game::spec::ShipList& shipList,
                                                const game::config::HostConfiguration& config,
                                                util::NumberFormatter fmt,
                                                afl::string::Translator& tx,
                                                const LinkBuilder& link)
{
    // drawStarbasePage(RichDocument& d, GUniverse& univ, int opts), part

    // Acquire data
    util::Vector<int, int> builds;
    bool any = false;
    const game::map::PlayedBaseType& type = univ.playedBases();
    for (Id_t pid = type.findNextIndex(0); pid != 0; pid = type.findNextIndex(pid)) {
        if (const Planet* pl = type.getObjectByIndex(pid)) {
            int hullType = pl->getBaseBuildHull(config, shipList.hullAssignments()).orElse(0);
            if (hullType > 0 && hullType <= shipList.hulls().size()) {
                any = true;
                builds.set(hullType, builds.get(hullType) + 1);
            } else {
                builds.set(0, builds.get(0) + 1);
            }
        }
    }

    // Render ship list
    {
        TagNode& row = makeRow(tab);
        makeText(makeWhite(makeLeftCell(row, 20)), tx("Ships Being Built"));
        makeRightCell(row, 4);
    }
    renderHullList(tab, builds, sortOrder, "Build.Hull$=%d", shipList, fmt, link, SearchQuery::SearchObjects_t(SearchQuery::SearchBases));
    if (!any) {
        TagNode& row = makeRow(tab);
        makeText(makeLeftCell(row), tx("(none)"));
        makeRightCell(row);
    }
}

// Render starship summary (part of StarshipPage).
void
game::map::info::renderShipSummary(TagNode& tab,
                                   const Universe& univ,
                                   bool withFreighters,
                                   const UnitScoreDefinitionList& shipScores,
                                   const game::spec::ShipList& shipList,
                                   const game::config::HostConfiguration& config,
                                   util::NumberFormatter fmt,
                                   afl::string::Translator& tx,
                                   const LinkBuilder& link)
{
    // ex drawStarshipPage(RichDocument& d, GUniverse& univ, const int opts, const bool withFreighters), part
    static const int ITEMS = 8;
    static const char*const names[ITEMS] = {
        N_("In free space:"),
        N_("Carriers:"),
        N_("Torpedo Ships:"),
        N_("Ships w/o fuel:"),
        N_("Damaged Ships:"),
        N_("Gravitonic accel:"),
        N_("Alchemy Ships:"),
        N_("Cloakable Ships:"),
    };
    static const char*const exprs[ITEMS] = {
        "Orbit$=0",
        "Type.Short=\"C\"",
        "Type.Short=\"T\"",
        "Cargo.N=0",
        "Damage<>0",
        "InStr(Hull.Special,\"G\")",
        "InStr(Hull.Special,\"A\")",
        "InStr(Hull.Special,\"C\")",
    };

    /* Acquire data */
    int32_t counts[ITEMS];
    int32_t count = 0;
    std::fill_n(counts, countof(counts), 0);

    const PlayedShipType& type = univ.playedShips();
    for (Id_t sid = type.findNextIndex(0); sid != 0; sid = type.findNextIndex(sid)) {
        if (const Ship* sh = type.getObjectByIndex(sid)) {
            if (withFreighters || sh->getNumBeams().orElse(0) > 0 || sh->getNumBays().orElse(0) > 0 || sh->getNumLaunchers().orElse(0) > 0) {
                ++count;
                if (sh->getNumLaunchers().orElse(0) > 0) {
                    ++counts[2];
                }
                if (sh->getNumBays().orElse(0) > 0) {
                    ++counts[1];
                }

                if (sh->getCargo(Element::Neutronium).orElse(0) == 0) {
                    ++counts[3];
                }
                if (sh->getDamage().orElse(0) > 0) {
                    ++counts[4];
                }
                if (sh->hasSpecialFunction(BasicHullFunction::Cloak, shipScores, shipList, config)
                    || sh->hasSpecialFunction(BasicHullFunction::AdvancedCloak, shipScores, shipList, config)
                    || sh->hasSpecialFunction(BasicHullFunction::HardenedCloak, shipScores, shipList, config))
                {
                    ++counts[7];
                }
                if (sh->hasSpecialFunction(BasicHullFunction::Gravitonic, shipScores, shipList, config)) {
                    ++counts[5];
                }
                if (sh->hasSpecialFunction(BasicHullFunction::MerlinAlchemy, shipScores, shipList, config)
                    || sh->hasSpecialFunction(BasicHullFunction::AriesRefinery, shipScores, shipList, config)
                    || sh->hasSpecialFunction(BasicHullFunction::NeutronicRefinery, shipScores, shipList, config))
                {
                    ++counts[6];
                }

                Point pos;
                if (sh->getPosition(pos) && univ.findPlanetAt(pos) == 0) {
                    ++counts[0];
                }
            }
        }
    }

    // Top part
    {
        TagNode& row = makeRow(tab);
        makeText(makeLeftCell(row, 17), tx("Total:"));
        makeText(makeGreen(makeRightCell(row, 3)), fmt.formatNumber(count));
    }
    for (int i = 0; i < ITEMS; ++i) {
        if (counts[i] > 0) {
            TagNode& row = makeRow(tab);
            makeLink(makeLeftCell(row), tx(names[i]),
                     link.makeSearchLink(SearchQuery(SearchQuery::MatchTrue,
                                                     SearchQuery::SearchObjects_t(SearchQuery::SearchShips),
                                                     exprs[i] + makeQuerySuffix(withFreighters))));
            makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(counts[i]));
        }
    }
}

// Render starship experience level summary (part of StarshipPage).
void
game::map::info::renderShipExperienceSummary(TagNode& tab,
                                             const Universe& univ,
                                             bool withFreighters,
                                             const UnitScoreDefinitionList& shipScores,
                                             const game::config::HostConfiguration& config,
                                             util::NumberFormatter fmt,
                                             afl::string::Translator& tx,
                                             const LinkBuilder& link)
{
    // ex drawStarshipPage(RichDocument& d, GUniverse& univ, const int opts, const bool withFreighters), part
    // Acquire data
    util::Vector<int, int> levelCounts;
    const PlayedShipType& type = univ.playedShips();
    for (Id_t sid = type.findNextIndex(0); sid != 0; sid = type.findNextIndex(sid)) {
        if (const Ship* sh = type.getObjectByIndex(sid)) {
            if (withFreighters || sh->hasWeapons()) {
                int level;
                if (sh->getScore(ScoreId_ExpLevel, shipScores).get(level) && level >= 0 && level <= MAX_EXPERIENCE_LEVELS) {
                    levelCounts.set(level, levelCounts.get(level) + 1);
                }
            }
        }
    }

    // Render
    {
        TagNode& row = makeRow(tab);
        makeText(makeWhite(makeLeftCell(row, 17)), tx("Ships by Experience Level"));
        makeRightCell(row, 3);
    }
    for (int i = 0; i <= MAX_EXPERIENCE_LEVELS; ++i) {
        int n = levelCounts.get(i);
        if (n > 0) {
            TagNode& row = makeRow(tab);
            makeLink(makeLeftCell(row), config.getExperienceLevelName(i, tx),
                     link.makeSearchLink(SearchQuery(SearchQuery::MatchTrue,
                                                     SearchQuery::SearchObjects_t(SearchQuery::SearchShips),
                                                     Format("Level=%d%s", i, makeQuerySuffix(withFreighters)))));
            makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(n));
        }
    }
}

// Render starship type summary (part of StarshipPage).
void
game::map::info::renderShipTypeSummary(TagNode& tab,
                                       const Universe& univ,
                                       uint8_t sortOrder,
                                       bool withFreighters,
                                       const game::spec::ShipList& shipList,
                                       util::NumberFormatter fmt,
                                       afl::string::Translator& tx,
                                       const LinkBuilder& link)
{
    util::Vector<int, int> hullCounts;
    const PlayedShipType& type = univ.playedShips();
    for (Id_t sid = type.findNextIndex(0); sid != 0; sid = type.findNextIndex(sid)) {
        if (const Ship* sh = type.getObjectByIndex(sid)) {
            if (withFreighters || sh->hasWeapons()) {
                int hullId = sh->getHull().orElse(0);
                hullCounts.set(hullId, hullCounts.get(hullId) + 1);
            }
        }
    }

    // Hull list
    {
        TagNode& row = makeRow(tab);
        makeText(makeWhite(makeLeftCell(row, 20)), tx("Ships by Hull Type"));
        makeRightCell(row, 4);
    }
    renderHullList(tab, hullCounts, sortOrder, "Owner$=My.Race$ And Hull$=%d", shipList, fmt, link, SearchQuery::SearchObjects_t(SearchQuery::SearchShips));
}

// Render starchart summary, own empire (part of StarchartPage).
void
game::map::info::renderStarchartEmpireSummary(TagNode& tab,
                                              const StarchartInfo& t,
                                              const Universe& univ,
                                              const TeamSettings& teams,
                                              util::NumberFormatter fmt,
                                              afl::string::Translator& tx)
{
    // ex drawStarchartPage(RichDocument& d, GUniverse& univ), part
    std::vector<int> planetXs;
    std::vector<int> planetYs;

    // Planets
    const PlayedPlanetType& ptype = univ.playedPlanets();
    for (Id_t pid = ptype.findNextIndex(0); pid != 0; pid = ptype.findNextIndex(pid)) {
        if (const Planet* pl = ptype.getObjectByIndex(pid)) {
            Point pos;
            if (pl->getPosition(pos)) {
                planetXs.push_back(pos.getX());
                planetYs.push_back(pos.getY());
            }
        }
    }

    // Compute size
    const Configuration& chartConf = univ.config();
    const Point chartSize = chartConf.getMode() == Configuration::Wrapped
        ? chartConf.getSize()
        : Point();
    int sizeX[2], sizeY[2];
    computeSize(planetXs, chartSize.getX(), sizeX);
    computeSize(planetYs, chartSize.getY(), sizeY);


    // Now, draw it
    {
        TagNode& row = makeRow(tab);
        makeText(makeWhite(makeLeftCell(row, 18)), tx("Your Empire"));
        makeLeftCell(row, 22);     // must specify size here because it also sizes the "X ly from A to B" text
    }

    makeTwoColumnTextRow(tab, tx("Planets:"), fmt.formatNumber(t.numCurrentPlanets.get(teams.getViewpointPlayer())));
    if (sizeX[0] >= 0) {
        showRange(tab, tx("East-West"),   sizeX, chartSize.getX(), tx);
        showRange(tab, tx("North-South"), sizeY, chartSize.getY(), tx);
    }
    makeTwoColumnTextRow(tab, tx("Starships:"), fmt.formatNumber(t.numCurrentShips.get(teams.getViewpointPlayer())));
    makeTwoColumnTextRowIfNonzero(tab, tx("Unowned Planets:"), t.numCurrentPlanets.get(0) + t.numOldPlanets.get(0), fmt);
    makeTwoColumnTextRow(tab, tx("Total Planets:"), fmt.formatNumber(t.totalPlanets));
    makeTwoColumnTextRowIfNonzero(tab, tx("Ion Storms:"), univ.ionStormType().countObjects(), fmt);
    makeTwoColumnTextRowIfNonzero(tab, tx("Own Minefields:"), t.numOwnMinefields, fmt);
    makeTwoColumnTextRowIfNonzero(tab, tx("Team Minefields:"), t.numTeamMinefields, fmt);
    makeTwoColumnTextRowIfNonzero(tab, tx("Enemy Minefields:"), t.numEnemyMinefields, fmt);
}

// Render starchart summary, foreign units (part of StarchartPage).
void
game::map::info::renderStarchartForeignSummary(TagNode& tab,
                                               const StarchartInfo& t,
                                               const TeamSettings& teams,
                                               const PlayerList& players,
                                               util::NumberFormatter fmt,
                                               afl::string::Translator& tx,
                                               const LinkBuilder& link)
{
    // ex drawStarchartPage(RichDocument& d, GUniverse& univ), part
    // Slightly different layout from PCC2 because we cannot do multi-column cells.
    {
        TagNode& row = makeRow(tab);
        makeText(makeWhite(makeLeftCell(row, 10)), tx("Foreign Units"));
        makeText(makeRightCell(row, 7), tx("Ships"));
        makeText(makeLeftCell(row, 3), tx("(hist.)"));
        makeText(makeRightCell(row, 7), tx("Planets"));
        makeText(makeLeftCell(row, 3), tx("(hist.)"));
        makeText(makeRightCell(row, 7), tx("Minefields"));
    }

    int totalCurrentPlanets = 0;
    int totalOldPlanets = 0;
    int totalCurrentShips = 0;
    int totalOldShips = 0;
    int totalMinefields = 0;
    for (int pl = 1; pl <= MAX_PLAYERS; ++pl) {
        const int thisCurrentPlanets = t.numCurrentPlanets.get(pl);
        const int thisOldPlanets = t.numOldPlanets.get(pl);
        const int thisCurrentShips = t.numCurrentShips.get(pl);
        const int thisOldShips = t.numOldShips.get(pl);
        const int thisMinefields = t.numMinefields.get(pl);
        if (pl != teams.getViewpointPlayer()
            && (thisCurrentPlanets != 0
                || thisOldPlanets != 0
                || thisCurrentShips != 0
                || thisOldShips != 0
                || thisMinefields != 0))
        {
            TagNode& row = makeRow(tab);
            makeLink(makeLeftCell(row), players.getPlayerName(pl, Player::ShortName),
                     link.makeSearchLink(SearchQuery(SearchQuery::MatchTrue,
                                                     SearchQuery::SearchObjects_t() + SearchQuery::SearchShips + SearchQuery::SearchPlanets + SearchQuery::SearchOthers,
                                                     Format("Owner$=%d", pl))));
            makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(thisCurrentShips));
            makeText(makeGreen(makeLeftCell(row)), Format(HISTORY_FMT, fmt.formatNumber(thisOldShips)));
            makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(thisCurrentPlanets));
            makeText(makeGreen(makeLeftCell(row)), Format(HISTORY_FMT, fmt.formatNumber(thisOldPlanets)));
            makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(thisMinefields));
            totalCurrentPlanets += thisCurrentPlanets;
            totalOldPlanets += thisOldPlanets;
            totalCurrentShips += thisCurrentShips;
            totalOldShips += thisOldShips;
            totalMinefields += thisMinefields;
        }
    }

    {
        TagNode& row = makeRow(tab);
        makeText(makeLeftCell(row), tx("Total:"));
        makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(totalCurrentShips));
        makeText(makeGreen(makeLeftCell(row)), Format(HISTORY_FMT, fmt.formatNumber(totalOldShips)));
        makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(totalCurrentPlanets));
        makeText(makeGreen(makeLeftCell(row)), Format(HISTORY_FMT, fmt.formatNumber(totalOldPlanets)));
        makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(totalMinefields));
    }
    {
        TagNode& row = makeRow(tab);
        makeText(makeLeftCell(row), INDENT + tx("Visual Contacts:"));
        makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(t.totalTargets));
        makeRightCell(row);
        makeLeftCell(row);
        makeRightCell(row);
        makeLeftCell(row);
    }
}

// Render universal minefield friendly code (part of StarchartPage).
void
game::map::info::renderUniversalFriendlyCode(TagNode& tab,
                                             const Universe& univ,
                                             const TeamSettings& teams,
                                             afl::string::Translator& tx,
                                             const LinkBuilder& link)
{
    // This used to be a <p>, but using a table provides a more uniform interface.
    TagNode& row = makeRow(tab);
    makeText(makeLeftCell(row), tx("Universal Minefield FCode:"));

    Id_t umfPlanet = univ.findUniversalMinefieldFriendlyCodePlanetId(teams.getViewpointPlayer());
    if (const Planet* pl = univ.planets().get(umfPlanet)) {
        makeLink(makeLeftCell(row), pl->getFriendlyCode().orElse(""), link.makePlanetLink(*pl));
    } else {
        makeText(makeGreen(makeLeftCell(row)), tx("none"));
    }
}

// Render beam weapon summary (part of WeaponsPage).
void
game::map::info::renderBeamWeaponSummary(TagNode& tab,
                                         const Universe& univ,
                                         bool showAll,
                                         const game::spec::ShipList& shipList,
                                         util::NumberFormatter fmt,
                                         afl::string::Translator& tx,
                                         const LinkBuilder& link)
{
    // ex drawWeaponPage(RichDocument& d, GUniverse& univ, int opts), part
    util::Vector<int32_t, int> numBeamShips, numBeams;

    const PlayedShipType& type = univ.playedShips();
    for (Id_t sid = type.findNextIndex(0); sid != 0; sid = type.findNextIndex(sid)) {
        if (const Ship* sh = type.getObjectByIndex(sid)) {
            if (!sh->hasWeapons()) {
                // Freighter
                numBeamShips.set(0, numBeamShips.get(0) + 1);
            } else {
                // Primary weapon
                const int thisBeams = sh->getNumBeams().orElse(0);
                const int beamType = sh->getBeamType().orElse(0);
                if (thisBeams > 0 && beamType > 0 && beamType <= shipList.beams().size()) {
                    numBeamShips.set(beamType, numBeamShips.get(beamType) + 1);
                    numBeams.set(beamType, numBeams.get(beamType) + thisBeams);
                } else {
                    numBeamShips.set(0, numBeamShips.get(0) + 1);
                }
            }
        }
    }

    // Render
    {
        TagNode& row = makeRow(tab);
        makeText(makeWhite(makeLeftCell(row, 16)), tx("Beams"));
        makeText(makeRightCell(row, 4), tx("Ships"));
        makeText(makeRightCell(row, 8), tx("Weapons"));
    }
    if (showAll || numBeamShips.get(0) != 0) {
        TagNode& row = makeRow(tab);
        makeOptionalLink(makeLeftCell(row), tx("No beams"),
                         link.makeSearchLink(SearchQuery(SearchQuery::MatchTrue,
                                                         SearchQuery::SearchObjects_t(SearchQuery::SearchShips),
                                                         Format("%sBeam$=0", LINK_PREFIX))),
                         numBeamShips.get(0) != 0);
        makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(numBeamShips.get(0)));
        makeRightCell(row);
    }
    for (int i = 1, n = shipList.beams().size(); i <= n; ++i) {
        const Beam* b = shipList.beams().get(i);
        if (b != 0 && (showAll || numBeamShips.get(i) != 0)) {
            TagNode& row = makeRow(tab);
            makeOptionalLink(makeLeftCell(row), b->getName(shipList.componentNamer()),
                             link.makeSearchLink(SearchQuery(SearchQuery::MatchTrue,
                                                             SearchQuery::SearchObjects_t(SearchQuery::SearchShips),
                                                             Format("%sBeam$=%d", LINK_PREFIX, i))),
                             numBeamShips.get(i));
            makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(numBeamShips.get(i)));
            makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(numBeams.get(i)));
        }
    }
}

// Render torpedo weapon summary (part of WeaponsPage).
void
game::map::info::renderTorpedoWeaponSummary(TagNode& tab,
                                            const Universe& univ,
                                            bool showAll,
                                            const game::spec::ShipList& shipList,
                                            util::NumberFormatter fmt,
                                            afl::string::Translator& tx,
                                            const LinkBuilder& link)
{
    // ex drawWeaponPage(RichDocument& d, GUniverse& univ, int opts), part
    util::Vector<int32_t, int> numTorpedoShips, numTorpedoes;

    const PlayedShipType& type = univ.playedShips();
    for (Id_t sid = type.findNextIndex(0); sid != 0; sid = type.findNextIndex(sid)) {
        if (const Ship* sh = type.getObjectByIndex(sid)) {
            const int thisBays = sh->getNumBays().orElse(0);
            const int thisBeams = sh->getNumBeams().orElse(0);
            const int thisLaunchers = sh->getNumLaunchers().orElse(0);

            if (thisBeams == 0 && thisBeams == 0 && thisLaunchers == 0) {
                // Freighter
                numTorpedoShips.set(0, numTorpedoShips.get(0) + 1);
            } else {
                // Secondary weapon
                const int torpType = sh->getTorpedoType().orElse(0);
                const int ammo = sh->getAmmo().orElse(0);
                if (thisLaunchers > 0 && torpType > 0 && torpType <= shipList.launchers().size()) {
                    // Torper
                    numTorpedoShips.set(torpType, numTorpedoShips.get(torpType) + 1);
                    numTorpedoes.set(torpType, numTorpedoes.get(torpType) + ammo);
                } else if (thisBays > 0) {
                    // Carrier
                } else {
                    // No torps/fighters
                    numTorpedoShips.set(0, numTorpedoShips.get(0) + 1);
                }
            }
        }
    }

    // Render
    {
        TagNode& row = makeRow(tab);
        makeText(makeWhite(makeLeftCell(row, 16)), tx("Torpedoes"));
        makeText(makeRightCell(row, 4), tx("Ships"));
        makeText(makeRightCell(row, 8), tx("Torpedoes"));
    }
    if (showAll || numTorpedoShips.get(0) != 0) {
        TagNode& row = makeRow(tab);
        makeOptionalLink(makeLeftCell(row), tx("No torps/fighters"),
                         link.makeSearchLink(SearchQuery(SearchQuery::MatchTrue,
                                                         SearchQuery::SearchObjects_t(SearchQuery::SearchShips),
                                                         Format("%sIsEmpty(Aux)", LINK_PREFIX))),
                         numTorpedoShips.get(0) != 0);
        makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(numTorpedoShips.get(0)));
        makeRightCell(row);
    }
    for (int i = 1, n = shipList.launchers().size(); i <= n; ++i) {
        const TorpedoLauncher* tl = shipList.launchers().get(i);
        if (tl != 0 && (showAll || numTorpedoShips.get(i) != 0)) {
            TagNode& row = makeRow(tab);
            makeOptionalLink(makeLeftCell(row), tl->getName(shipList.componentNamer()),
                             link.makeSearchLink(SearchQuery(SearchQuery::MatchTrue,
                                                             SearchQuery::SearchObjects_t(SearchQuery::SearchShips),
                                                             Format("%sTorp$=%d", LINK_PREFIX, i))),
                             numTorpedoShips.get(i));
            makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(numTorpedoShips.get(i)));
            makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(numTorpedoes.get(i)));
        }
    }
}

// Render misc weapon summary (part of WeaponsPage).
void
game::map::info::renderOtherWeaponSummary(TagNode& tab,
                                          const Universe& univ,
                                          util::NumberFormatter fmt,
                                          afl::string::Translator& tx)
{
    // ex drawWeaponPage(RichDocument& d, GUniverse& univ, int opts)
    int32_t unarmed = 0;
    int32_t carriers = 0;
    int32_t fighters = 0;

    const PlayedShipType& type = univ.playedShips();
    for (Id_t sid = type.findNextIndex(0); sid != 0; sid = type.findNextIndex(sid)) {
        if (const Ship* sh = type.getObjectByIndex(sid)) {
            if (!sh->hasWeapons()) {
                // Freighter
                ++unarmed;
            } else if (sh->getNumBays().orElse(0) > 0) {
                // Carrier
                ++carriers;
                fighters += sh->getAmmo().orElse(0);
            }
        }
    }

    // Render
    {
        TagNode& row = makeRow(tab);
        makeText(makeWhite(makeLeftCell(row, 16)), tx("Others"));
        makeRightCell(row, 4);
    }
    {
        TagNode& row = makeRow(tab);
        makeOptionalLink(makeLeftCell(row), tx("Carriers"), Format("q:UI.Search '%sFighter.Bays','2s'", LINK_PREFIX), carriers != 0);
        makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(carriers));
    }
    if (carriers != 0) {
        TagNode& row = makeRow(tab);
        makeText(makeLeftCell(row), INDENT + tx("Fighters"));
        makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(fighters));
    }
    {
        TagNode& row = makeRow(tab);
        makeOptionalLink(makeLeftCell(row), tx("Unarmed ships"), Format("q:UI.Search '%sType.Short=\"F\"','2s'", LINK_PREFIX), carriers != 0);
        makeText(makeGreen(makeRightCell(row)), fmt.formatNumber(unarmed));
    }
}
