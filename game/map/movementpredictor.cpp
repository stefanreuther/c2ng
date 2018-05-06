/**
  *  \file game/map/movementpredictor.cpp
  */

#include <cassert>
#include "game/map/movementpredictor.hpp"
#include "game/map/anyshiptype.hpp"
#include "game/map/objectvector.hpp"
#include "game/map/ship.hpp"
#include "game/map/shippredictor.hpp"
#include "game/map/universe.hpp"
#include "game/spec/mission.hpp"

using game::spec::Mission;

game::map::MovementPredictor::MovementPredictor()
    : m_info()
{ }

game::map::MovementPredictor::~MovementPredictor()
{ }

// /** Compute movement and fill in ships' predicted position fields.
//     Loosely based upon shipacc.pas::InitMovementPrediction */
void
game::map::MovementPredictor::computeMovement(const Universe& univ,
                                              const Game& game,
                                              const game::spec::ShipList& shipList,
                                              const Root& root)
{
    // ex GMovementPredictor::computeMovement
    init(univ);
    resolveTows(univ);
    while (moveShips(univ, game, shipList, root)) {
        // nix
    }
}

// /** Get position of a ship. */
bool
game::map::MovementPredictor::getShipPosition(Id_t sid, Point& out) const
{
    // ex GMovementPredictor::getShipPosition
    if (Info* p = m_info.get(sid)) {
        out = p->pos;
        return true;
    } else {
        return false;
    }
}

// /** Get cargo of a ship. */
bool
game::map::MovementPredictor::getShipCargo(Id_t sid, Cargo_t& out) const
{
    // ex GMovementPredictor::getShipCargo
    if (Info* p = m_info.get(sid)) {
        out = p->cargo;
        return true;
    } else {
        return false;
    }
}


// /** Initialize movement info.
//     Initialize all ship's status and waypoint. */
void
game::map::MovementPredictor::init(const Universe& univ)
{
    // ex GMovementPredictor::init
    AnyShipType ty(const_cast<Universe&>(univ));
    for (Id_t i = ty.findNextIndex(0); i != 0; i = ty.findNextIndex(i)) {
        if (const Ship* pShip = ty.getObjectByIndex(i)) {
            if (Info* pInfo = m_info.create(i)) {
                pInfo->status = Normal;
                if (pShip->isPlayable(Ship::ReadOnly)) {
                    pShip->getWaypoint().get(pInfo->pos);
                } else {
                    pShip->getPosition(pInfo->pos);
                }
                copyCargo(*pInfo, *pShip, Cargo_t::Tritanium,  Element::Tritanium);
                copyCargo(*pInfo, *pShip, Cargo_t::Duranium,   Element::Duranium);
                copyCargo(*pInfo, *pShip, Cargo_t::Molybdenum, Element::Molybdenum);
                copyCargo(*pInfo, *pShip, Cargo_t::Supplies,   Element::Supplies);
                copyCargo(*pInfo, *pShip, Cargo_t::Money,      Element::Money);
            }
        }
    }
}

// /** Tow resolution.
//     Set all towing/towed ships' status. */
void
game::map::MovementPredictor::resolveTows(const Universe& univ)
{
    // Assume any tow succeeds.
    // Anyway, be careful not to make tow groups with more than two ships.
    AnyShipType ty(const_cast<Universe&>(univ));
    for (Id_t i = ty.findNextIndex(0); i != 0; i = ty.findNextIndex(i)) {
        const Ship* pShip = ty.getObjectByIndex(i);
        Info* pInfo = m_info.get(i);
        if (pShip != 0 && pInfo != 0 && pShip->getMission().orElse(0) == Mission::msn_Tow) {
            int towId = pShip->getMissionParameter(TowParameter).orElse(0);
            if (Info* pToweeInfo = m_info.get(towId)) {
                if (towId != i && pInfo->status == Normal && pToweeInfo->status == Normal) {
                    // Ship is trying to tow a valid ship,
                    // and neither already has a different role in towing.
                    pInfo->status = Towing;
                    pToweeInfo->status = Towed;
                }
            }
        }
    }
}

bool
game::map::MovementPredictor::moveShips(const Universe& univ,
                                        const Game& game,
                                        const game::spec::ShipList& shipList,
                                        const Root& root)
{
    // ex GMovementPredictor::moveShips
    bool moved = false;             // true if we moved a ship
    int unresolved_sid = 0;         // ID of a ship with unresolved intercept

    // Try moving all ships.
    // - not ours: just mark it moved.
    // - ours, Normal, not intercepting: move and mark Moved
    // - ours, Towing: move, move towee, mark both Moved
    // - ours, Normal, intercepting, towee Moved: move, mark Moved
    // - ours, Normal, intercepting, towee not Moved: wait for next iteration.
    //   For a normal, non-cyclic intercept, the next iteration will ultimately move it.
    //   For a cyclic intercept, we need special handling; see below.
    AnyShipType ty(const_cast<Universe&>(univ));
    for (Id_t sid = ty.findNextIndex(0); sid != 0; sid = ty.findNextIndex(sid)) {
        const Ship* pShip = ty.getObjectByIndex(sid);
        Info* pInfo = m_info.get(sid);
        if (pShip != 0 && pInfo != 0) {
            const Status ost = pInfo->status;
            if (pShip->isPlayable(Object::ReadOnly) && (ost == Normal || ost == Towing)) {
                // We could possibly move this ship. Intercept?
                if (Info* pTarget = getInterceptTarget(*pShip)) {
                    if (pTarget->status != Moved) {
                        // This is an unresolved intercept. Save it for later.
                        if (unresolved_sid == 0) {
                            unresolved_sid = sid;
                        }
                        continue;
                    }
                    pInfo->pos = pTarget->pos;
                }

                // Work on a copy of the ship
                ShipPredictor pred(univ, sid, game.shipScores(), shipList, root.hostConfiguration(), root.hostVersion(), root.registrationKey());
                pred.setWaypoint(pInfo->pos);
                pred.computeTurn();
                pInfo->pos = pred.getPosition();
                pInfo->status = Moved;
                moved = true;
                copyCargo(*pInfo, pred);
                if (ost == Towing) {
                    // Move towee
                    int toweeId = pShip->getMissionParameter(TowParameter).orElse(0);
                    Info* pToweeInfo = m_info.get(toweeId);
                    const Ship* pTowee = univ.ships().get(toweeId);
                    if (pTowee != 0 && pToweeInfo != 0) {
                        pToweeInfo->pos = pInfo->pos;
                        pToweeInfo->status = Moved;

                        // Compute towee's turn.
                        // Normally, we'd have to use the combined ShipPredictor to compute both turns at once.
                        // However, since we're not interested in fuel usage, we can also compute the towee separately.
                        if (pShip->isPlayable(Object::ReadOnly)) {
                            ShipPredictor toweePred(univ, toweeId, game.shipScores(), shipList, root.hostConfiguration(), root.hostVersion(), root.registrationKey());
                            toweePred.setWaypoint(toweePred.getPosition());
                            toweePred.setWarpFactor(0);
                            toweePred.computeTurn();
                            copyCargo(*pToweeInfo, toweePred);
                        }
                    } else {
                        // Error
                        return false;
                    }
                }
            }

            // Mark ship done if possible.
            // If it is being towed, we cannot mark it here;
            // it'll be marked by the above code during a later iteration.
            // Make sure to not mark it done too early, otherwise intercepting towed ships won't work.
            if (pInfo->status != Towed && pInfo->status != Moved) {
                pInfo->status = Moved;
                moved = true;
            }
        }
    }

    // Resolve cyclic intercepts.
    // If we did not move a ship in the above loop, but found one that is intercepting,
    // this means the ship is the entry into an intercept loop
    // (but possibly not part of the actual loop).
    // NOTE: unresolved_sid was set to the Id of a ship whose intercept could not be resolved
    // when it was encountered. That intercept can now be resolvable by an action in the
    // above loop after setting unresolved_sid. Therefore, it is important to only enter this
    // loop if the above loop did not modify anything [#371].
    if (!moved && unresolved_sid != 0) {
        // Find all ships that are part of the loop
        // FIXME: we can probably simplify this by using an 'Info*' instead of an 'int sid'.
        int sid = unresolved_sid;
        while (1) {
            Info* pInfo = m_info.get(sid);
            Ship* pShip = univ.ships().get(sid);
            if (pInfo == 0 || pShip == 0) {
                // Error
                return false;
            }
            if (pInfo->status != Normal) {
                // Found loop
                break;
            }
            if (getInterceptTarget(*pShip) == 0) {
                // This is an error and cannot (should not) happen.
                // If it happens anyway, stop to avoid infinite loop.
                return false;
            }
            pInfo->status = ResolvingLoop;
            sid = pShip->getMissionParameter(InterceptParameter).orElse(0);
        }

        // Now, %sid points to a ship which is part of the loop.
        // Go around once more, collecting the coordinates.
        const int loop_start = sid;
        int32_t sum_x = 0;
        int32_t sum_y = 0;
        int num_ships = 0;
        do {
            Ship* pShip = univ.ships().get(sid);
            assert(pShip != 0);
            assert(getInterceptTarget(*pShip) != 0);
            Point pos;
            pShip->getPosition(pos);
            sum_x += pos.getX();
            sum_y += pos.getY();
            ++num_ships;
            sid = pShip->getMissionParameter(InterceptParameter).orElse(0);
        } while (sid != loop_start);

        // Resolve the PHost way, very simple: move everyone to the geometric center.
        // We do not care for wrap for now.
        sum_x /= num_ships;
        sum_y /= num_ships;
        do {
            Info* pInfo = m_info.get(sid);
            assert(pInfo != 0);
            Ship* pShip = univ.ships().get(sid);
            assert(pShip != 0);
            ShipPredictor pred(univ, sid, game.shipScores(), shipList, root.hostConfiguration(), root.hostVersion(), root.registrationKey());
            pred.setWaypoint(Point(sum_x, sum_y));
            pred.computeTurn();
            pInfo->pos = pred.getPosition();
            pInfo->status = Moved;
            copyCargo(*pInfo, pred);
            moved = true;
            sid = pShip->getMissionParameter(InterceptParameter).orElse(0);
        } while (sid != loop_start);

        // Now, clear all other ResolvingLoop ships. These are the entry into the loop;
        // the next iteration will move them because they now have fulfilled preconditions.
        for (Id_t i = 1, n = m_info.size(); i <= n; ++i) {
            if (Info* p = m_info.get(i)) {
                if (p->status == ResolvingLoop) {
                    p->status = Normal;
                }
            }
        }
    }

    return moved;
}

// /** Check valid intercept.
//     The intercept must be in a status that allows us to resolve it,
//     i.e. it must not target a nonexisting ship. That is:
//     - valid Id
//     - not referring to a nonexistant ship
//     - not referring to itself (this condition is actually redundant with the current implementation) */
game::map::MovementPredictor::Info*
game::map::MovementPredictor::getInterceptTarget(const Ship& sh) const
{
    // ex GMovementPredictor::isValidIntercept
    int msn = sh.getMission().orElse(0);
    int i = sh.getMissionParameter(InterceptParameter).orElse(0);
    if (msn == Mission::msn_Intercept && i != sh.getId()) {
        return m_info.get(i);
    } else {
        return 0;
    }
}


void
game::map::MovementPredictor::copyCargo(Info& info, const Ship& sh, Cargo_t::Type infoElement, Element::Type shipElement)
{
    // ex GMovementPredictor::copyCargo
    info.cargo.set(infoElement, sh.getCargo(shipElement).orElse(0));
}

void
game::map::MovementPredictor::copyCargo(Info& info, const ShipPredictor& pred)
{
    // ex GMovementPredictor::copyCargo
    info.cargo.set(Cargo_t::Tritanium,  pred.getCargo(Element::Tritanium));
    info.cargo.set(Cargo_t::Duranium,   pred.getCargo(Element::Duranium));
    info.cargo.set(Cargo_t::Molybdenum, pred.getCargo(Element::Molybdenum));
    info.cargo.set(Cargo_t::Supplies,   pred.getCargo(Element::Supplies));
    info.cargo.set(Cargo_t::Money,      pred.getCargo(Element::Money));
}
