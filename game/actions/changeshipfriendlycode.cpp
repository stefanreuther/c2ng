/**
  *  \file game/actions/changeshipfriendlycode.cpp
  */

#include "game/actions/changeshipfriendlycode.hpp"
#include "game/map/ship.hpp"
#include "game/map/reverter.hpp"

using game::map::Ship;

game::actions::ChangeShipFriendlyCode::ChangeShipFriendlyCode(game::map::Universe& univ)
    : m_universe(univ),
      m_info()
{
    // ex GFleetFCodeChanger::GFleetFCodeChanger [part]
}

game::actions::ChangeShipFriendlyCode::~ChangeShipFriendlyCode()
{ }

void
game::actions::ChangeShipFriendlyCode::addShip(Id_t shipId, game::spec::FriendlyCodeList& fcl, util::RandomNumberGenerator& rng)
{
    if (Ship* p = m_universe.ships().get(shipId)) {
        m_info.push_back(Info(shipId, p->getFriendlyCode().orElse(""), fcl.generateRandomCode(rng, fcl.Pessimistic)));
    }
}

void
game::actions::ChangeShipFriendlyCode::addFleet(Id_t fleetId, game::spec::FriendlyCodeList& fcl, util::RandomNumberGenerator& rng)
{
    // ex GFleetFCodeChanger::init
    for (Id_t shipId = m_universe.playedShips().findNextIndex(0); shipId != 0; shipId = m_universe.playedShips().findNextIndex(shipId)) {
        if (Ship* p = m_universe.ships().get(shipId)) {
            // Also check shipId, which means if we pass the Id of a lone ship,
            // that ship will be added.
            if (shipId == fleetId || p->getFleetNumber() == fleetId) {
                addShip(shipId, fcl, rng);
            }
        }
    }
}

// /** Set friendly code on all fleet members to the same value.
//     \param fcode New fcode */
void
game::actions::ChangeShipFriendlyCode::setFriendlyCode(String_t fc)
{
    // ex GFleetFCodeChanger::setFCode
    for (size_t i = 0, n = m_info.size(); i < n; ++i) {
        if (Ship* p = m_universe.ships().get(m_info[i].shipId)) {
            p->setFriendlyCode(fc);
        }
    }
}

// /** Set friendly code on all fleet members to avoid a same value.
//     Attempts to set the fcodes such that no fleet member has the specified code.
//     \param avoid Code to avoid */
void
game::actions::ChangeShipFriendlyCode::unsetFriendlyCode(String_t avoidFC)
{
    for (size_t i = 0, n = m_info.size(); i < n; ++i) {
        Id_t shipId = m_info[i].shipId;
        if (Ship* p = m_universe.ships().get(shipId)) {
            String_t newFC = m_info[i].oldFriendlyCode;
            if (newFC == avoidFC) {
                if (game::map::Reverter* pReverter = m_universe.getReverter()) {
                    pReverter->getPreviousShipFriendlyCode(shipId).get(newFC);
                }
            }
            if (newFC == avoidFC) {
                newFC = m_info[i].randomFriendlyCode;
            }
            p->setFriendlyCode(newFC);
        }
    }
}

// /** Undo all friendly code changes.
//     Restores the fcodes that were set before this object was constructed. */
void
game::actions::ChangeShipFriendlyCode::undo()
{
    // GFleetFCodeChanger::undo
    for (size_t i = 0, n = m_info.size(); i < n; ++i) {
        if (Ship* p = m_universe.ships().get(m_info[i].shipId)) {
            p->setFriendlyCode(m_info[i].oldFriendlyCode);
        }
    }
}
