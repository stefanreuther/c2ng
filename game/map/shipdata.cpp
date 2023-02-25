/**
  *  \file game/map/shipdata.cpp
  *  \brief Structure game::map::ShipData
  */

#include "game/map/shipdata.hpp"

game::map::ShipData::ShipData(int)
{ }

game::IntegerProperty_t
game::map::getShipMass(const ShipData& data, const game::spec::ShipList& shipList)
{
    // ex game/struct-util.cc:getShipMass, global.pas:ShipMassOf
    int hull, ammo, n, t, d, m, col, sup, nl, lt, nb, bt;
    if (data.hullType.get(hull) && data.ammo.get(ammo)
        && data.neutronium.get(n) && data.tritanium.get(t) && data.duranium.get(d) && data.molybdenum.get(m)
        && data.colonists.get(col) && data.supplies.get(sup)
        && data.numLaunchers.get(nl) && data.torpedoType.get(lt)
        && data.numBeams.get(nb) && data.beamType.get(bt))
    {
        const game::spec::Hull* pHull = shipList.hulls().get(hull);
        if (!pHull) {
            return IntegerProperty_t();
        }
        int mass = pHull->getMass() + ammo + n + t + d + m + col + sup;
        if (nl > 0) {
            const game::spec::TorpedoLauncher* pLauncher = shipList.launchers().get(lt);
            if (pLauncher == 0) {
                return IntegerProperty_t();
            }
            mass += pLauncher->getMass() * nl;
        }
        if (nb > 0) {
            const game::spec::Beam* pBeam = shipList.beams().get(bt);
            if (pBeam == 0) {
                return IntegerProperty_t();
            }
            mass += pBeam->getMass() * nb;
        }
        return mass;
    } else {
        return IntegerProperty_t();
    }
}

game::IntegerProperty_t
game::map::getShipCargo(const ShipData& data, Element::Type type)
{
    // ex GShip::getCargoRaw, GShip::getCargo
    int numBays, expectedType, numLaunchers, torpedoType;
    switch (type) {
     case Element::Neutronium:
        return data.neutronium;
     case Element::Tritanium:
        return data.tritanium;
     case Element::Duranium:
        return data.duranium;
     case Element::Molybdenum:
        return data.molybdenum;
     case Element::Fighters:
        if (data.numBays.get(numBays)) {
            if (numBays > 0) {
                // I know it has bays, so ammo is number of fighters
                return data.ammo;
            } else {
                // I know it has no fighters
                return 0;
            }
        } else {
            // I don't know whether it has fighters
            return afl::base::Nothing;
        }
     case Element::Colonists:
        return data.colonists;
     case Element::Supplies:
        return data.supplies;
     case Element::Money:
        return data.money;
     default:
        if (Element::isTorpedoType(type, expectedType)) {
            if (data.torpedoType.get(torpedoType)) {
                if (torpedoType == expectedType) {
                    // Asking correct torpedo type
                    return data.ammo;
                } else {
                    // Asking wrong torpedo type
                    return 0;
                }
            } else if (data.numLaunchers.get(numLaunchers) && numLaunchers == 0) {
                // Asking any type, and we know we don't have torpedoes
                return 0;
            } else {
                // Nothing known
                return afl::base::Nothing;
            }
        } else {
            // I don't know what cargo type this is, but I don't have it.
            return 0;
        }
    }
}

void
game::map::setShipCargo(ShipData& data, Element::Type type, IntegerProperty_t amount)
{
    int numBays, expectedType, torpedoType;
    switch (type) {
     case Element::Neutronium:
        data.neutronium = amount;
        break;
     case Element::Tritanium:
        data.tritanium = amount;
        break;
     case Element::Duranium:
        data.duranium = amount;
        break;
     case Element::Molybdenum:
        data.molybdenum = amount;
        break;
     case Element::Fighters:
        if (data.numBays.get(numBays) && numBays > 0) {
            // I know it has bays, so ammo is number of fighters
            data.ammo = amount;
        }
        break;
     case Element::Colonists:
        data.colonists = amount;
        break;
     case Element::Supplies:
        data.supplies = amount;
        break;
     case Element::Money:
        data.money = amount;
        break;
     default:
        if (Element::isTorpedoType(type, expectedType) && data.torpedoType.get(torpedoType) && torpedoType == expectedType) {
            data.ammo = amount;
        }
        break;
    }
}

bool
game::map::isTransferActive(const ShipData::Transfer& tr)
{
    return tr.neutronium.orElse(0) != 0
        || tr.tritanium.orElse(0) != 0
        || tr.duranium.orElse(0) != 0
        || tr.molybdenum.orElse(0) != 0
        || tr.colonists.orElse(0) != 0
        || tr.supplies.orElse(0) != 0
        || tr.targetId.orElse(0) != 0;
}
