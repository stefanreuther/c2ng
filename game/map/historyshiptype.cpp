/**
  *  \file game/map/historyshiptype.cpp
  *  \brief Class game::map::HistoryShipType
  */

#include "game/map/historyshiptype.hpp"

game::map::HistoryShipType::HistoryShipType(ObjectVector<Ship>& vec)
    : ObjectVectorType<Ship>(vec)
{ }

bool
game::map::HistoryShipType::isValid(const Ship& s) const
{
    // ex GHistoryShipType::isValidIndex (sort-of)
    return s.getShipKind() != Ship::NoShip;
}

game::Id_t
game::map::HistoryShipType::findNextShipAtNoWrap(Point pos, Id_t id, bool marked, int& turn)
{
    // ex WHistoryObjectSelection::browse (part)
    for (Id_t i = findNextIndex(id); i != 0; i = findNextIndex(i)) {
        if (acceptShip(pos, i, marked, turn)) {
            return i;
        }
    }
    return 0;
}

game::Id_t
game::map::HistoryShipType::findPreviousShipAtNoWrap(Point pos, Id_t id, bool marked, int& turn)
{
    // ex WHistoryObjectSelection::browse (part)
    for (Id_t i = getPreviousIndex(id); i != 0; i = getPreviousIndex(i)) {
        if (acceptShip(pos, i, marked, turn)) {
            return i;
        }
    }
    return 0;
}

game::Id_t
game::map::HistoryShipType::findNextShipAtWrap(Point pos, Id_t id, bool marked, int& turn)
{
    // ex WHistoryObjectSelection::browse (part)
    Id_t n = findNextShipAtNoWrap(pos, id, marked, turn);
    if (n == 0) {
        n = findNextShipAtNoWrap(pos, 0, marked, turn);
    }
    return n;
}

game::Id_t
game::map::HistoryShipType::findPreviousShipAtWrap(Point pos, Id_t id, bool marked, int& turn)
{
    // ex WHistoryObjectSelection::browse (part)
    Id_t n = findPreviousShipAtNoWrap(pos, id, marked, turn);
    if (n == 0) {
        n = findPreviousShipAtNoWrap(pos, 0, marked, turn);
    }
    return n;
}

bool
game::map::HistoryShipType::acceptShip(Point pos, Id_t id, bool marked, int& turn)
{
    if (const Ship* sh = getObjectByIndex(id)) {
        if (!marked || sh->isMarked()) {
            int t = sh->getHistoryNewestLocationTurn();
            while (const ShipHistoryData::Track* p = sh->getHistoryLocation(t)) {
                int x, y;
                if (p->x.get(x) && p->y.get(y) && pos.getX() == x && pos.getY() == y) {
                    turn = t;
                    return true;
                }
                --t;
            }
        }
    }
    return false;
}
