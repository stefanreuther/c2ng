/**
  *  \file game/map/shipinfo.cpp
  */

#include "game/map/shipinfo.hpp"
#include "afl/base/countof.hpp"
#include "afl/string/format.hpp"
#include "game/map/chunnelmission.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"

namespace {
    using game::Element;
    using afl::string::Format;

    // FIXME: code duplication: planetinfo.cpp
    String_t formatAge(int currentTurn, int historyTurn, afl::string::Translator& tx)
    {
        // ex formatTurnNumber
        int age = currentTurn - historyTurn;
        if (age == 0) {
            return tx("current turn");
        } else if (age == 1) {
            return tx("previous turn");
        } else if (age < 0) {
            return Format(tx("turn %d"), historyTurn);
        } else {
            return Format(tx("%d turns ago"), age);
        }
    }

    void addHeading(game::map::ShipCargoInfos_t& result, String_t heading)
    {
        result.push_back(game::map::ShipCargoInfo(heading, String_t(), String_t(), true, !result.empty()));
    }

}


/*
 *  ShipMovementInfo
 */

bool
game::map::ShipMovementInfo::operator==(const ShipMovementInfo& other) const
{
    return action == other.action
        && status == other.status
        && partner == other.partner
        && from == other.from
        && to == other.to;
}

bool
game::map::ShipMovementInfo::operator!=(const ShipMovementInfo& other) const
{
    return !operator==(other);
}

/*
 *  Public Methods
 */

void
game::map::packShipMovementInfo(ShipMovementInfos_t& result,
                                const Ship& ship,
                                const Universe& univ,
                                const UnitScoreDefinitionList& scoreDefinitions,
                                const game::spec::ShipList& shipList,
                                const Root& root)
{
    // ex WShipScannerChartWidget::drawPost (part)
    // Ship position. We cannot do anything without it.
    Point pos;
    if (!ship.getPosition(pos)) {
        return;
    }

    // Waypoint
    Point waypoint;
    if (ship.getWaypoint().get(waypoint)) {
        result.push_back(ShipMovementInfo(ShipMovementInfo::Movement, ShipMovementInfo::Success, 0, pos, waypoint));
    }

    // Chunnel.
    // FIXME: PCC 1.x also parses when this ship goes through another ship's chunnel.
    // To do that, it computes turn movement.
    const Configuration& config = univ.config();
    ChunnelMission ch;
    if (ch.check(ship, univ, scoreDefinitions, shipList, root)) {
        const Ship* target = univ.ships().get(ch.getTargetId());
        Point targetPos;
        if (target != 0 && target->getPosition(targetPos)) {
            ShipMovementInfo::Status st =
                ch.getFailureReasons() == 0
                ? ShipMovementInfo::Success
                : (ch.getFailureReasons() & ~ChunnelMission::chf_MateAny) != 0
                ? ShipMovementInfo::InitiatorFails
                : ShipMovementInfo::MateFails;

            result.push_back(ShipMovementInfo(ShipMovementInfo::Chunnel, st, ch.getTargetId(), pos, config.getSimpleNearestAlias(targetPos, pos)));
        }
    }

    // Tow
    for (Id_t i = univ.findShipTowing(ship.getId(), 0); i != 0; i = univ.findShipTowing(ship.getId(), i)) {
        // How much do we want to validate here?
        // Additional conditions for towing are same X/Y and appropriate engines of the tower.
        // We try to prevent users from setting those, so we don't need to filter here.
        // If users manage to set them anyway, let them have it.
        // PCC 1.x does the same.
        const Ship* tower = univ.ships().get(i);
        Point towerWaypoint;
        if (tower != 0 && tower->getWaypoint().get(towerWaypoint)) {
            result.push_back(ShipMovementInfo(ShipMovementInfo::Tow, ShipMovementInfo::Success, i, pos, config.getSimpleNearestAlias(towerWaypoint, pos)));
        }
    }
}



void
game::map::packShipLastKnownCargo(ShipCargoInfos_t& result, const Ship& ship, int currentTurn, util::NumberFormatter fmt, const game::spec::ShipList& shipList, afl::string::Translator& tx)
{
    // ex client/dialogs/cargohist.cc:addLastKnownCargo
    // ex histscan.pas:ShowLastKnownCargo (part)

    class Helper {
     public:
        Helper(ShipCargoInfos_t& result, String_t turnLabel, util::NumberFormatter& fmt, const game::spec::ShipList& shipList, afl::string::Translator& tx)
            : m_result(result), m_turnLabel(turnLabel), m_hadHeading(false), m_total(0),
              m_formatter(fmt), m_shipList(shipList), m_translator(tx)
            { }

        void add(Element::Type type, int32_t amount)
            {
                bool addSpaceBefore = false;
                if (!m_hadHeading) {
                    addHeading(m_result, m_translator("Last known cargo"));
                    m_result.push_back(ShipCargoInfo(m_turnLabel, String_t(), String_t(), false, false));
                    addSpaceBefore = true;
                    m_hadHeading = true;
                }
                m_result.push_back(ShipCargoInfo(Element::getName(type, m_translator, m_shipList),
                                                 m_formatter.formatNumber(amount),
                                                 Element::getUnit(type, m_translator, m_shipList),
                                                 false, addSpaceBefore));
                if (type != Element::Money) {
                    m_total += amount;
                }
            }

        bool hadAnything() const
            { return m_hadHeading; }

        int32_t getTotal() const
            { return m_total; }
     private:
        ShipCargoInfos_t& m_result;
        String_t m_turnLabel;
        bool m_hadHeading;
        int32_t m_total;
        util::NumberFormatter& m_formatter;
        const game::spec::ShipList& m_shipList;
        afl::string::Translator& m_translator;
    };

    // Set up helper
    String_t turnLabel;
    int infoTurn = ship.getHistoryTimestamp(Ship::RestTime);
    if (infoTurn > 0) {
        turnLabel = Format("(%s)", formatAge(currentTurn, infoTurn, tx));
    } else {
        turnLabel = tx("(age of this information is unknown)");
    }
    Helper h(result, turnLabel, fmt, shipList, tx);

    // List cargo
    static const Element::Type list[] = {
        Element::Neutronium, Element::Tritanium, Element::Duranium, Element::Molybdenum,
        Element::Supplies, Element::Colonists, Element::Money
    };
    for (size_t i = 0; i < countof(list); ++i) {
        Element::Type el = list[i];
        int n;
        if (ship.getCargo(el).get(n)) {
            h.add(el, n);
        }
    }

    int ammo;
    if (ship.getAmmo().get(ammo)) {
        // We know its ammo
        const game::spec::Hull* hull = shipList.hulls().get(ship.getHull().orElse(0));
        if (hull != 0 && hull->getNumBays() != 0) {
            // We know it's a carrier
            h.add(Element::Fighters, ammo);
        } else {
            // Check whether we know the torpedo type
            int torpType = 0, launcherCount = 0;
            if (ship.getTorpedoType().get(torpType)
                && shipList.launchers().get(torpType) != 0
                && ((ship.getNumLaunchers().get(launcherCount) && launcherCount > 0)
                    || (hull != 0 && hull->getMaxLaunchers() > 0)))
            {
                h.add(Element::fromTorpedoType(torpType), ammo);
            } else {
                // Can this happen? PCC 1.x will display a naked "Torpedoes" line here.
            }
        }
    }

    // Summary/footer
    if (h.hadAnything()) {
        result.push_back(ShipCargoInfo(tx("\xE2\x96\xB6 Total"), fmt.formatNumber(h.getTotal()), tx("kt"), false, true));
    }
}

void
game::map::packShipMassRanges(ShipCargoInfos_t& result, const Ship& ship, util::NumberFormatter fmt, const game::spec::ShipList& shipList, afl::string::Translator& tx)
{
    // ex client/dialogs/cargohist.cc:addMassRanges
    // ex histscan.pas:ShowLastKnownCargo (part)

    // Helper to simplify code
    class Helper {
     public:
        Helper(ShipCargoInfos_t& result, util::NumberFormatter fmt)
            : m_result(result),
              m_formatter(fmt),
              m_addSpace(true)
            { }
        void addLine(String_t name, int32_t value, String_t unit)
            {
                m_result.push_back(ShipCargoInfo(name, m_formatter.formatNumber(value), unit, false, m_addSpace));
                m_addSpace = false;
            }
     private:
        ShipCargoInfos_t& m_result;
        util::NumberFormatter m_formatter;
        bool m_addSpace;
    };

    // Must know mass and hull to proceed
    int shipMass = 0;
    if (!ship.getMass(shipList).get(shipMass)) {
        return;
    }

    const game::spec::Hull* pHull = shipList.hulls().get(ship.getHull().orElse(0));
    if (!pHull) {
        return;
    }

    // Heading
    Helper h(result, fmt);
    addHeading(result, Format(tx("Current mass: %d kt"), shipMass));

    // We know the ship's mass and hull. Do we know its arms?
    bool armsKnown = true;
    int knownArmsTotal = 0;
    int tubeMass;
    h.addLine(tx("Hull"), pHull->getMass(), tx("kt"));
    if (pHull->getNumBays() > 0) {
        // It has bays, which don't have a weight
        tubeMass = 0;
    } else {
        int numLaunchers = 0;
        game::spec::TorpedoLauncher* pLauncher = shipList.launchers().get(ship.getTorpedoType().orElse(0));
        if (ship.getNumLaunchers().get(numLaunchers) && numLaunchers != 0 && pLauncher != 0) {
            // We know its torpedo type
            tubeMass = pLauncher->getMass() * numLaunchers;
            knownArmsTotal += tubeMass;
            h.addLine(Format(tx("%s launchers"), pLauncher->getName(shipList.componentNamer())), tubeMass, tx("kt"));
        } else if (pHull->getMaxLaunchers() != 0 && !ship.getNumLaunchers().isValid()) {
            // It might have torps, but we don't know which one
            tubeMass = 0;
            armsKnown = false;
            for (int i = 1; i <= shipList.launchers().size(); ++i) {
                if (const game::spec::TorpedoLauncher* p = shipList.launchers().get(i)) {
                    tubeMass = std::max(tubeMass, p->getMass());
                }
            }
            tubeMass *= pHull->getMaxLaunchers();
        } else {
            // No torpedoes
            tubeMass = 0;
        }
    }

    int beamMass;
    int numBeams = 0;
    const game::spec::Beam* pBeam = shipList.beams().get(ship.getBeamType().orElse(0));
    if (ship.getNumBeams().get(numBeams) && numBeams > 0 && pBeam != 0) {
        // We know its beam type
        beamMass = pBeam->getMass() * numBeams;
        knownArmsTotal += beamMass;
        h.addLine(Format(tx("%s beams"), pBeam->getName(shipList.componentNamer())), beamMass, tx("kt"));
    } else if (pHull->getMaxBeams() > 0 && !ship.getNumBeams().isValid()) {
        // It might have beams but we don't know which ones
        armsKnown = false;
        beamMass = 0;
        for (int i = 1; i <= shipList.beams().size(); ++i) {
            if (const game::spec::Beam* p = shipList.beams().get(i)) {
                beamMass = std::max(beamMass, p->getMass());
            }
        }
        beamMass *= pHull->getMaxBeams();
    } else {
        // No beams
        beamMass = 0;
    }

    /* Now add derived information. We know that
           Hull_mass   == h.getMass()
           Weapon_mass <= beamMass + tubeMass
           Fuel_mass   <= h.getFuel()
           Cargo_mass  <= h.getCargo()
       Hence,
           Fuel_mass == Mass - Hull_mass - Weapon_mass - Cargo_mass
       and therefore
           Fuel_mass >= Mass - Hull_mass - (beamMass+tubeMass) - h.getCargo()
       giving a range for fuel content. Same reasoning holds for cargo. */
    int minCargo = 0;
    int minFuel = 0;
    int mass = shipMass - pHull->getMass() - beamMass - tubeMass;
    if (mass > pHull->getMaxCargo()) {
        minFuel = mass - pHull->getMaxCargo();
    }
    if (mass > pHull->getMaxFuel()) {
        minCargo = mass - pHull->getMaxFuel();
    }

    /* Mass is
           Mass := Hull_mass + Weapon_mass + Fuel_mass + Cargo_mass
       We know that
           Hull_mass   == h.getMass()
           Weapon_mass >= knownArmsTotal
           Fuel_mass   >= minFuel
           Cargo_mass  >= minCargo
       Hence,
           Fuel_mass   == Mass - Hull_mass - Weapon_mass - Cargo_mass
           Fuel_mass   <= Mass - Hull_mass - knownArmsTotal - minCargo */
    mass = shipMass - knownArmsTotal - pHull->getMass();
    h.addLine(armsKnown ? tx("Cargo+Fuel") : tx("Cargo+Fuel+Weapons"), mass, tx("kt"));
    if (mass >= minCargo && mass - minCargo < pHull->getMaxFuel()) {
        h.addLine(tx("\xE2\x96\xB6 Max. Fuel"), mass - minCargo, tx("kt"));
    }
    if (minFuel > 0) {
        h.addLine(tx("\xE2\x96\xB6 Min. Fuel"), minFuel, tx("kt"));
    }
    if (mass >= minFuel && mass - minFuel < pHull->getMaxCargo()) {
        h.addLine(armsKnown ? tx("\xE2\x96\xB6 Max. Cargo") : tx("\xE2\x96\xB6 Max. Cargo+Weapons"), mass - minFuel, tx("kt"));
    }
    if (minCargo > 0) {
        h.addLine(tx("\xE2\x96\xB6 Min. Cargo"), minCargo, tx("kt"));
    }
}
