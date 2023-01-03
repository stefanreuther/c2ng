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
