/**
  *  \file game/ref/sortby.cpp
  *  \brief Class game::ref::SortBy - Sort Predicates
  */

#include "game/ref/sortby.hpp"
#include "afl/string/format.hpp"
#include "game/map/fleet.hpp"
#include "game/map/object.hpp"
#include "game/map/ship.hpp"
#include "game/spec/mission.hpp"
#include "util/math.hpp"

using afl::string::Format;
using game::Reference;
using game::Session;
using game::map::Planet;
using game::map::Point;
using game::map::Ship;
using game::map::Universe;

namespace {
    /* Get name for a reference */
    String_t getReferenceName(Session& session, const Reference& a)
    {
        // Resolve as plain name; if that does not work, as reference name.
        String_t name;
        if (!session.getReferenceName(a, game::PlainName, name)) {
            name = a.toString(session.translator());
        }
        return name;
    }

    /* Get owner for a reference */
    int getReferenceOwner(const Universe& univ, const Reference& a)
    {
        if (a.getType() == Reference::Player) {
            return a.getId();
        } else if (const game::map::Object* obj = univ.getObject(a)) {
            return obj->getOwner().orElse(0);
        } else {
            return 0;
        }
    }

    /* Get position from a reference */
    afl::base::Optional<Point> getReferencePosition(const Universe& univ, const Reference& a)
    {
        if (const game::map::Object* mo = univ.getObject(a)) {
            // It's a map object
            return mo->getPosition();
        } else {
            // Might be a position
            return a.getPosition();
        }
    }

    /* Get hull type from a reference */
    int getReferenceHullType(const Universe& univ, const Reference& a)
    {
        if (a.getType() == Reference::Hull) {
            return a.getId();
        } else if (a.getType() == Reference::Ship) {
            if (const Ship* pShip = univ.ships().get(a.getId())) {
                return pShip->getHull().orElse(0);
            } else {
                return 0;
            }
        } else {
            return 0;
        }
    }

    /* Get class name for a position: stringify the position */
    String_t getClassForPosition(afl::base::Optional<Point> pt, afl::string::Translator& tx)
    {
        if (const Point* p = pt.get()) {
            return p->toString();
        } else {
            return tx("not on map");
        }
    }

    /* Compare two optional positions */
    int comparePositions(afl::base::Optional<Point> a, afl::base::Optional<Point> b)
    {
        Point pa, pb;
        bool oka = a.get(pa);
        bool okb = b.get(pb);
        int result = util::compare3(oka, okb);
        if (result == 0) {
            result = pa.compare(pb);
        }
        return result;
    }

    /* Given ship, return towed ship */
    game::Id_t getShipTowId(const Ship& ship)
    {
        // Only check current ships
        // FIXME: should be done generally; loading a history ship should clear the mission
        if (ship.getShipKind() != Ship::CurrentShip) {
            return 0;
        }

        // Check for Tow mission
        int mission;
        if (!ship.getMission().get(mission)) {
            return 0;
        }
        if (mission != game::spec::Mission::msn_Tow) {
            return 0;
        }

        // Result is tow parameter
        return ship.getMissionParameter(game::TowParameter).orElse(0);
    }


    /*
     *  Helpers for SortBy::TransferTarget
     */

    /* Given a ship, get reference to transporter target */
    Reference getTransporterTarget(const Ship& ship, Ship::Transporter tr)
    {
        if (ship.isTransporterActive(tr)) {
            game::Id_t id;
            if (ship.getTransporterTargetId(tr).get(id)) {
                switch (tr) {
                 case Ship::TransferTransporter:
                    return Reference(Reference::Ship, id);
                 case Ship::UnloadTransporter:
                    return Reference(Reference::Planet, id);
                }
            }
        }
        return Reference();
    }

    /* Classify reference for sorting by transporter target */
    int classifyTransporterTarget(Reference r)
    {
        switch (r.getType()) {
         case Reference::Ship:   return 1;
         case Reference::Planet: return 2;
         default:                return 0;
        }
    }

}


/*
 *  SortBy::Id
 */

int
game::ref::SortBy::Id::compare(const Reference& a, const Reference& b) const
{
    // ex game/objl-sort.cc:sortById, sort.pas:SortById
    return util::compare3(a.getId(), b.getId());
}

String_t
game::ref::SortBy::Id::getClass(const Reference& /*a*/) const
{
    return String_t();
}


/*
 *  SortBy::Name
 */

game::ref::SortBy::Name::Name(Session& session)
    : m_session(session)
{ }

int
game::ref::SortBy::Name::compare(const Reference& a, const Reference& b) const
{
    // ex sortByName
    return afl::string::strCaseCompare(getReferenceName(m_session, a), getReferenceName(m_session, b));
}

String_t
game::ref::SortBy::Name::getClass(const Reference& /*a*/) const
{
    return String_t();
}


/*
 *  SortBy::Owner
 */

game::ref::SortBy::Owner::Owner(const game::map::Universe& univ, const PlayerList& players, afl::string::Translator& tx)
    : m_universe(univ),
      m_players(players),
      m_translator(tx)
{ }

int
game::ref::SortBy::Owner::compare(const Reference& a, const Reference& b) const
{
    // ex sortByOwner, sort.pas:SortByOwner
    return util::compare3(getReferenceOwner(m_universe, a), getReferenceOwner(m_universe, b));
}

String_t
game::ref::SortBy::Owner::getClass(const Reference& a) const
{
    // ex diviOwner
    return m_players.getPlayerName(getReferenceOwner(m_universe, a), Player::ShortName, m_translator);
}


/*
 *  SortBy::Position (formerly SortByLocation)
 */

game::ref::SortBy::Position::Position(const game::map::Universe& univ, afl::string::Translator& tx)
    : m_universe(univ),
      m_translator(tx)
{ }

int
game::ref::SortBy::Position::compare(const Reference& a, const Reference& b) const
{
    // ex sortByLocation, sort.pas:SortBy::Location
    return comparePositions(getPosition(a), getPosition(b));
}

String_t
game::ref::SortBy::Position::getClass(const Reference& a) const
{
    // ex diviLocation
    return getClassForPosition(getPosition(a), m_translator);
}

inline afl::base::Optional<game::map::Point>
game::ref::SortBy::Position::getPosition(const Reference& a) const
{
    return getReferencePosition(m_universe, a);
}


/*
 *  SortBy::NextPosition (formerly SortByNewLocation)
 */

game::ref::SortBy::NextPosition::NextPosition(const game::map::Universe& univ,
                                              const Game& game,
                                              const game::spec::ShipList& shipList,
                                              const Root& root,
                                              afl::string::Translator& tx)
    : m_universe(univ),
      m_translator(tx),
      m_predictor()
{
    m_predictor.computeMovement(univ, game, shipList, root);
}

int
game::ref::SortBy::NextPosition::compare(const Reference& a, const Reference& b) const
{
    // ex sortByNewLocation
    return comparePositions(getPosition(a), getPosition(b));
}

String_t
game::ref::SortBy::NextPosition::getClass(const Reference& a) const
{
    // ex diviNewLocation
    return getClassForPosition(getPosition(a), m_translator);
}

afl::base::Optional<game::map::Point>
game::ref::SortBy::NextPosition::getPosition(const Reference& a) const
{
    afl::base::Optional<Point> result;
    if (a.getType() == Reference::Ship) {
        // Try to resolve via predictor
        result = m_predictor.getShipPosition(a.getId());
    }

    if (!result.isValid()) {
        // Does not move, or we don't know how it moves
        result = getReferencePosition(m_universe, a);
    }

    return result;
}


/*
 *  SortBy::Damage
 */

game::ref::SortBy::Damage::Damage(const game::map::Universe& univ)
    : m_universe(univ)
{ }

int
game::ref::SortBy::Damage::compare(const Reference& a, const Reference& b) const
{
    return util::compare3(getDamage(a), getDamage(b));
}

String_t
game::ref::SortBy::Damage::getClass(const Reference& /*a*/) const
{
    return String_t();
}

int
game::ref::SortBy::Damage::getDamage(const Reference& a) const
{
    // ex sortByDamage, sort.pas:SortBy::Damage
    int result = 0;
    if (a.getType() == Reference::Ship) {
        if (const Ship* pShip = m_universe.ships().get(a.getId())) {
            result = pShip->getDamage().orElse(0);
        }
    }
    return result;
}


/*
 *  SortBy::Mass
 */

game::ref::SortBy::Mass::Mass(const game::map::Universe& univ, const game::spec::ShipList& shipList)
    : m_universe(univ),
      m_shipList(shipList)
{ }

int
game::ref::SortBy::Mass::compare(const Reference& a, const Reference& b) const
{
    // ex sortByMass, sort.pas:SortBy::Mass
    return util::compare3(getMass(a), getMass(b));
}

String_t
game::ref::SortBy::Mass::getClass(const Reference& /*a*/) const
{
    return String_t();
}

int
game::ref::SortBy::Mass::getMass(const Reference& a) const
{
    int result = 0;
    if (a.getType() == Reference::Ship) {
        if (const Ship* pShip = m_universe.ships().get(a.getId())) {
            result = pShip->getMass(m_shipList).orElse(0);
        }
    }
    return result;
}


/*
 *  SortBy::HullMass
 */

game::ref::SortBy::HullMass::HullMass(const game::map::Universe& univ, const game::spec::ShipList& shipList)
    : m_universe(univ), m_shipList(shipList)
{ }

int
game::ref::SortBy::HullMass::compare(const Reference& a, const Reference& b) const
{
    // ex sortByHullMass, sort.pas:SortByHullMass
    return util::compare3(getHullMass(a), getHullMass(b));
}

String_t
game::ref::SortBy::HullMass::getClass(const Reference& /*a*/) const
{
    return String_t();
}

int
game::ref::SortBy::HullMass::getHullMass(const Reference& a) const
{
    if (const game::spec::Hull* pHull = m_shipList.hulls().get(getReferenceHullType(m_universe, a))) {
        return pHull->getMass();
    } else {
        return 0;
    }
}


/*
 *  SortBy::HullType
 */

game::ref::SortBy::HullType::HullType(const game::map::Universe& univ, const game::spec::ShipList& shipList, afl::string::Translator& tx)
    : m_universe(univ),
      m_shipList(shipList),
      m_translator(tx)
{ }

int
game::ref::SortBy::HullType::compare(const Reference& a, const Reference& b) const
{
    // ex sortByHull, sort.pas:SortByHull
    return util::compare3(getReferenceHullType(m_universe, a),
                          getReferenceHullType(m_universe, b));
}

String_t
game::ref::SortBy::HullType::getClass(const Reference& a) const
{
    // ex diviHull
    if (const game::spec::Hull* pHull = m_shipList.hulls().get(getReferenceHullType(m_universe, a))) {
        return pHull->getName(m_shipList.componentNamer());
    } else {
        return m_translator("unknown");
    }
}


/*
 *  SortBy::BattleOrder
 */

game::ref::SortBy::BattleOrder::BattleOrder(const game::map::Universe& univ, BattleOrderRule rule, afl::string::Translator& tx)
    : m_universe(univ),
      m_rule(rule),
      m_translator(tx)
{ }

int
game::ref::SortBy::BattleOrder::compare(const Reference& a, const Reference& b) const
{
    return util::compare3(getBattleOrderValue(a), getBattleOrderValue(b));
}

String_t
game::ref::SortBy::BattleOrder::getClass(const Reference& a) const
{
    // ex diviBattleOrder
    int n = getBattleOrderValue(a);
    if (n < 0) {
        return "< 0";
    } else if (n < 1000) {
        int level = n/100;
        return Format("%d .. %d", 100*level, 100*level+99);
    } else if (n < BattleOrderRule::UNKNOWN) {
        return UTF_GEQ " 1000";
    } else {
        return m_translator("unknown");
    }

}

int
game::ref::SortBy::BattleOrder::getBattleOrderValue(const Reference& a) const
{
    // ex sortByBattleOrder
    if (const game::map::Object* obj = m_universe.getObject(a)) {
        return m_rule.get(*obj);
    } else {
        return BattleOrderRule::UNKNOWN;
    }
}


/*
 *  SortBy::Fleet
 */

game::ref::SortBy::Fleet::Fleet(const game::map::Universe& univ, afl::string::Translator& tx)
    : m_universe(univ),
      m_translator(tx)
{ }

int
game::ref::SortBy::Fleet::compare(const Reference& a, const Reference& b) const
{
    // ex sortByFleet, sort.pas:SortByFleet
    return util::compare3(getFleetNumberKey(a), getFleetNumberKey(b));
}

String_t
game::ref::SortBy::Fleet::getClass(const Reference& a) const
{
    // ex diviFleet
    int key = getFleetNumberKey(a) >> 1;
    if (key != 0) {
        if (const Ship* pLeader = m_universe.ships().get(key)) {
            return game::map::Fleet::getTitle(*pLeader, m_translator);
        } else {
            // This is an error: a fleet number that does not exist
            return m_translator("unknown");
        }
    } else {
        return m_translator("not in a fleet");
    }
}

int
game::ref::SortBy::Fleet::getFleetNumberKey(const Reference& a) const
{
    int result = 0;
    if (a.getType() == Reference::Ship) {
        if (const Ship* pShip = m_universe.ships().get(a.getId())) {
            int fleetNumber = pShip->getFleetNumber();
            if (fleetNumber != 0) {
                if (pShip->isFleetLeader()) {
                    result = 2*fleetNumber;
                } else {
                    result = 2*fleetNumber + 1;
                }
            }
        }
    }
    return result;
}


/*
 *  SortBy::TowGroup
 */

game::ref::SortBy::TowGroup::TowGroup(const game::map::Universe& univ, afl::string::Translator& tx)
    : m_universe(univ),
      m_translator(tx)
{ }

int
game::ref::SortBy::TowGroup::compare(const Reference& a, const Reference& b) const
{
    return util::compare3(getTowGroupKey(a), getTowGroupKey(b));
}

String_t
game::ref::SortBy::TowGroup::getClass(const Reference& a) const
{
    // ex diviTowGroup
    int key = getTowGroupKey(a) >> 1;
    if (const Ship* pShip = m_universe.ships().get(key)) {
        return Format(m_translator("towing %s"), pShip->getName());
    } else {
        return m_translator("not in a tow group");
    }
}

int
game::ref::SortBy::TowGroup::getTowGroupKey(const Reference& a) const
{
    // ex sortByTowGroup, sort.pas:SortByTowGroup
    if (a.getType() == Reference::Ship) {
        if (const Ship* pShip = m_universe.ships().get(a.getId())) {
            // Check whether we are towing someone.
            if (int towee = getShipTowId(*pShip)) {
                return 2*towee;
            }

            // Check if we are being towed
            // (This makes many algorithms O(n^2), but our n usually is small.)
            for (int i = 1, n = m_universe.ships().size(); i <= n; ++i) {
                if (const Ship* pTower = m_universe.ships().get(i)) {
                    if (getShipTowId(*pTower) == a.getId()) {
                        return 2*a.getId() + 1;
                    }
                }
            }
        }
    }
    return 0;
}


/*
 *  SortBy::TransferTarget
 */

game::ref::SortBy::TransferTarget::TransferTarget(const game::map::Universe& univ,
                                                  game::map::Ship::Transporter transporterId,
                                                  bool checkOther,
                                                  afl::string::Translator& tx)
    : m_universe(univ),
      m_transporterId(transporterId),
      m_checkOther(checkOther),
      m_translator(tx)
{ }

int
game::ref::SortBy::TransferTarget::compare(const Reference& a, const Reference& b) const
{
    const Reference ta = getTarget(a);
    const Reference tb = getTarget(b);

    int result = util::compare3(classifyTransporterTarget(ta), classifyTransporterTarget(tb));
    if (result == 0) {
        result = util::compare3(ta.getId(), tb.getId());
    }
    if (result == 0) {
        result = util::compare3(classifyTransporterTarget(a), classifyTransporterTarget(b));
    }
    if (result == 0) {
        result = util::compare3(a.getId(), b.getId());
    }
    return result;
}

String_t
game::ref::SortBy::TransferTarget::getClass(const Reference& a) const
{
    const Reference ta = getTarget(a);
    switch (ta.getType()) {
     case Reference::Ship: {
        String_t shipName;
        if (const Ship* pShip = m_universe.ships().get(ta.getId())) {
            shipName = pShip->getName();
        }
        if (shipName.empty()) {
            shipName = Format("#%d", ta.getId());
        }
        return Format(m_translator("Transferring to %s"), shipName);
     }

     case Reference::Planet:
        if (ta.getId() == 0) {
            return m_translator("Jettison");
        } else {
            String_t planetName;
            if (const Planet* pPlanet = m_universe.planets().get(ta.getId())) {
                planetName = pPlanet->getName(m_translator);
            }
            if (planetName.empty()) {
                planetName = Format("#%d", ta.getId());
            }
            return Format(m_translator("Unloading to %s"), planetName);
        }

     default:
        return String_t();
    }
}

game::Reference
game::ref::SortBy::TransferTarget::getTarget(const Reference a) const
{
    Reference result;
    if (const Ship* pShip = dynamic_cast<const Ship*>(m_universe.getObject(a))) {
        // Check requested transporter
        result = getTransporterTarget(*pShip, m_transporterId);

        // Check other transporter if desired
        if (!result.isSet() && m_checkOther) {
            switch (m_transporterId) {
             case Ship::TransferTransporter:
                result = getTransporterTarget(*pShip, Ship::UnloadTransporter);
                break;
             case Ship::UnloadTransporter:
                result = getTransporterTarget(*pShip, Ship::TransferTransporter);
                break;
            }
        }
    }
    return result;
}
