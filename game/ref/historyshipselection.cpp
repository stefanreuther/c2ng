/**
  *  \file game/ref/historyshipselection.cpp
  *  \brief Class game::ref::HistoryShipSelection
  */

#include <cmath>
#include "game/ref/historyshipselection.hpp"
#include "afl/string/format.hpp"
#include "game/game.hpp"
#include "game/map/historyshiptype.hpp"
#include "game/ref/sortbyhulltype.hpp"
#include "game/ref/sortbyname.hpp"
#include "game/ref/sortbyowner.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "util/string.hpp"

using afl::string::Format;

const size_t game::ref::HistoryShipSelection::ModeMax;
const size_t game::ref::HistoryShipSelection::SortMax;

namespace {
    int getShipLastTurn(const game::map::Ship& sh)
    {
        // ex WHistoryShipSelection::getShipLastTurn (sort-of)
        int t = sh.getHistoryNewestLocationTurn();
        while (game::map::ShipHistoryData::Track* e = sh.getHistoryLocation(t)) {
            if (e->x.isValid() && e->y.isValid()) {
                return t;
            }
            --t;
        }
        return 0;
    }

    class SortByAge : public game::ref::HistoryShipList::SortPredicate {
     public:
        // ex AgeSort, AgeDivi
        SortByAge(afl::string::Translator& tx, int turnNumber)
            : m_translator(tx), m_turnNumber(turnNumber)
            { }

        virtual int compare(const game::ref::HistoryShipList::Item& a, const game::ref::HistoryShipList::Item& b) const
            { return b.turnNumber - a.turnNumber; }
        virtual String_t getClass(const game::ref::HistoryShipList::Item& a) const
            {
                if (a.turnNumber == 0) {
                    return m_translator("unknown");
                } else {
                    return util::formatAge(m_turnNumber, a.turnNumber, m_translator);
                }
            }

     private:
        afl::string::Translator& m_translator;
        int m_turnNumber;
    };
}


game::ref::HistoryShipSelection::HistoryShipSelection()
    : m_mode(AllShips),
      m_sortOrder(ById),
      m_positionValid(false),
      m_position()
{ }

game::ref::HistoryShipSelection::~HistoryShipSelection()
{ }

void
game::ref::HistoryShipSelection::setMode(Mode m)
{
    m_mode = m;
}

game::ref::HistoryShipSelection::Mode
game::ref::HistoryShipSelection::getMode() const
{
    return m_mode;
}

void
game::ref::HistoryShipSelection::setSortOrder(SortOrder o)
{
    m_sortOrder = o;
}

game::ref::HistoryShipSelection::SortOrder
game::ref::HistoryShipSelection::getSortOrder() const
{
    return m_sortOrder;
}

void
game::ref::HistoryShipSelection::setPosition(game::map::Point pos)
{
    m_position = pos;
    m_positionValid = true;
}

void
game::ref::HistoryShipSelection::buildList(HistoryShipList& list, const Turn& turn, Session& session) const
{
    // ex WHistoryShipSelection::buildList
    list.clear();

    // Must have a game for team settings
    const Game* pGame = session.getGame().get();
    if (pGame == 0) {
        return;
    }
    const TeamSettings& teams = pGame->teamSettings();
    const game::map::Universe& univ = turn.universe();

    game::map::HistoryShipType ty(const_cast<game::map::Universe&>(univ).ships());
    for (Id_t id = ty.findNextIndex(0); id != 0; id = ty.findNextIndex(id)) {
        if (const game::map::Ship* sh = ty.getObjectByIndex(id)) {
            bool accept = false;
            int turn = sh->getHistoryNewestLocationTurn();
            int owner;
            switch (m_mode) {
             case AllShips:
                accept = true;
                turn = getShipLastTurn(*sh);
                break;
             case LocalShips:
                while (game::map::ShipHistoryData::Track* e = sh->getHistoryLocation(turn)) {
                    int x, y;
                    if (e->x.get(x) && e->y.get(y)) {
                        if (isInRange(x, y, pGame->mapConfiguration())) {
                            accept = true;
                            break;
                        }
                    }
                    --turn;
                }
                break;
             case ExactShips:
                while (game::map::ShipHistoryData::Track* e = sh->getHistoryLocation(turn)) {
                    int x, y;
                    if (e->x.get(x) && e->y.get(y)) {
                        if (x == m_position.getX() && y == m_position.getY()) {
                            accept = true;
                            break;
                        }
                    }
                    --turn;
                }
                break;
             case ForeignShips:
                if (sh->getOwner(owner) && owner != teams.getViewpointPlayer()) {
                    accept = true;
                    turn = getShipLastTurn(*sh);
                }
                break;
             case TeamShips:
                if (sh->getOwner(owner) && teams.getPlayerRelation(owner) != TeamSettings::EnemyPlayer) {
                    accept = true;
                    turn = getShipLastTurn(*sh);
                }
                break;
             case EnemyShips:
                if (sh->getOwner(owner) && teams.getPlayerRelation(owner) == TeamSettings::EnemyPlayer) {
                    accept = true;
                    turn = getShipLastTurn(*sh);
                }
                break;
             case OwnShips:
                if (sh->getOwner(owner) && owner == teams.getViewpointPlayer()) {
                    accept = true;
                    turn = getShipLastTurn(*sh);
                }
                break;
            }
            if (accept) {
                list.add(HistoryShipList::Item(UserList::makeReferenceItem(Reference(Reference::Ship, id), session), turn));
            }
        }
    }

    const int refTurn = turn.getTurnNumber();
    switch (m_sortOrder) {
     case ById:
        break;

     case ByOwner:
        if (const Root* pRoot = session.getRoot().get()) {
            list.sort(SortByOwner(univ, pRoot->playerList()));
        }
        break;

     case ByHull:
        if (const game::spec::ShipList* pShipList = session.getShipList().get()) {
            list.sort(SortByHullType(univ, *pShipList, session.translator()));
        }
        break;

     case ByAge:
        list.sort(SortByAge(session.translator(), refTurn));
        break;

     case ByName:
        list.sort(SortByName(session));
        break;
    }
    list.setReferenceTurn(refTurn);
}

game::ref::HistoryShipSelection::Modes_t
game::ref::HistoryShipSelection::getAvailableModes(const game::map::Universe& univ, const game::map::Configuration& mapConfig, const TeamSettings& teams) const
{
    // ex WHistoryShipSelection::getVisibleModes
    Modes_t modes;
    Modes_t expect = Modes_t::allUpTo(Mode(ModeMax));

    // If we do not have a position, we do not expect LocalShips/ExactShips
    if (!m_positionValid) {
        expect -= LocalShips;
        expect -= ExactShips;
    }

    // Check all ships
    game::map::HistoryShipType ty(const_cast<game::map::Universe&>(univ).ships());
    for (Id_t id = ty.findNextIndex(0); id != 0; id = ty.findNextIndex(id)) {
        if (const game::map::Ship* sh = ty.getObjectByIndex(id)) {
            // Check owner modes
            int shipOwner;
            if (sh->getOwner(shipOwner)) {
                modes += AllShips;
                if (shipOwner == teams.getViewpointPlayer()) {
                    modes += OwnShips;
                } else {
                    modes += ForeignShips;
                }
                if (teams.getPlayerRelation(shipOwner) == TeamSettings::EnemyPlayer) {
                    modes += EnemyShips;
                } else {
                    modes += TeamShips;
                }
            }

            // Check location modes
            if (m_positionValid) {
                int t = sh->getHistoryNewestLocationTurn();
                while (game::map::ShipHistoryData::Track* e = sh->getHistoryLocation(t)) {
                    int x, y;
                    if (e->x.get(x) && e->y.get(y)) {
                        if (isInRange(x, y, mapConfig)) {
                            modes += LocalShips;
                        }
                        if (x == m_position.getX() && y == m_position.getY()) {
                            modes += ExactShips;
                        }
                        if (modes.contains(LocalShips) && modes.contains(ExactShips)) {
                            break;
                        }
                    }
                    --t;
                }
            }

            // Exit early when we saw all modes we can expect
            if (modes == expect) {
                break;
            }
        }
    }

    // Discount team modes when we don't have teams
    if (!teams.hasAnyTeams()) {
        modes -= TeamShips;
        modes -= EnemyShips;
    }

    return modes;
}

game::ref::HistoryShipSelection::Mode
game::ref::HistoryShipSelection::getInitialMode(const game::map::Universe& univ, const game::map::Configuration& mapConfig, const TeamSettings& teams) const
{
    // ex WHistoryShipSelection::getInitialMode
    Modes_t modes = getAvailableModes(univ, mapConfig, teams);
    if (modes.contains(ExactShips) && m_positionValid && univ.findPlanetAt(mapConfig.getCanonicalLocation(m_position)) != 0) {
        return ExactShips;
    } else if (modes.contains(LocalShips)) {
        return LocalShips;
    } else {
        return AllShips;
    }
}

String_t
game::ref::HistoryShipSelection::getModeName(Mode mode, afl::string::Translator& tx) const
{
    // ex WHistoryShipSelection::getModeName
    switch (mode) {
     case AllShips:
        return tx("All ships");
     case LocalShips:
        return Format(tx("Ships near %s"), m_position.toString());
     case ExactShips:
        return Format(tx("Ships exactly at %s"), m_position.toString());
     case ForeignShips:
        return tx("Foreign ships");
     case TeamShips:
        return tx("Team ships");
     case EnemyShips:
        return tx("Enemy ships");
     case OwnShips:
        return tx("Own ships");
    }
    return String_t();
}

String_t
game::ref::HistoryShipSelection::getModeName(afl::string::Translator& tx) const
{
    // ex WHistoryShipSelection::getModeName
    return getModeName(m_mode, tx);
}

String_t
game::ref::HistoryShipSelection::getSortOrderName(SortOrder sort, afl::string::Translator& tx)
{
    // ex WHistoryShipSelection::getSortName
    switch (sort) {
     case ById:
        return tx("Sort by Id");
     case ByOwner:
        return tx("Sort by Owner");
     case ByHull:
        return tx("Sort by Hull");
     case ByAge:
        return tx("Sort by Age of scan");
     case ByName:
        return tx("Sort by Name");
    }
    return String_t();
}

String_t
game::ref::HistoryShipSelection::getSortOrderName(afl::string::Translator& tx) const
{
    // ex WHistoryShipSelection::getSortName
    return getSortOrderName(m_sortOrder, tx);
}

bool
game::ref::HistoryShipSelection::isInRange(int x, int y, const game::map::Configuration& mapConfig) const
{
    game::map::Point p = mapConfig.getSimpleNearestAlias(game::map::Point(x, y), m_position);

    return (std::abs(p.getX() - m_position.getX()) < 10 && std::abs(p.getY() - m_position.getY()) < 10);
}
