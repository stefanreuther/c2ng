/**
  *  \file game/map/shipdata.hpp
  */
#ifndef C2NG_GAME_MAP_SHIPDATA_HPP
#define C2NG_GAME_MAP_SHIPDATA_HPP

#include "game/types.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace map {

    struct ShipData {
        struct Transfer {
            IntegerProperty_t neutronium;
            IntegerProperty_t tritanium;
            IntegerProperty_t duranium;
            IntegerProperty_t molybdenum;
            IntegerProperty_t colonists;
            IntegerProperty_t supplies;
            IntegerProperty_t targetId;
        };

        IntegerProperty_t     owner;                                      ///< Ship owner.
        StringProperty_t      friendlyCode;                               ///< Friendly code.
        IntegerProperty_t     warpFactor;                                 ///< Warp factor.
        NegativeProperty_t    waypointDX;
        NegativeProperty_t    waypointDY;                     ///< Waypoint displacement.
        IntegerProperty_t     x;
        IntegerProperty_t     y;                                       ///< Position.
        IntegerProperty_t     engineType;                                 ///< Engine type.
        IntegerProperty_t     hullType;                                   ///< Hull type.
        IntegerProperty_t     beamType;                                   ///< Beam type.
        IntegerProperty_t     numBeams;                                   ///< Number of beams.
        IntegerProperty_t     numBays;                                    ///< Number of fighter bays.
        IntegerProperty_t     launcherType;                               ///< Torpedo type.
        IntegerProperty_t     ammo;                                       ///< Number of torpedoes or fighters.
        IntegerProperty_t     numLaunchers;                               ///< Number of torpedo launchers.
        IntegerProperty_t     mission;                                    ///< Mission.
        IntegerProperty_t     primaryEnemy;                               ///< Primary enemy.
        IntegerProperty_t     missionTowParameter;                        ///< Mission: tow Id.
        IntegerProperty_t     damage;                                     ///< Damage.
        IntegerProperty_t     crew;                                       ///< Current crew.
        IntegerProperty_t     colonists;                                  ///< Colonists in cargo room.
        StringProperty_t      name;                                       ///< Ship name.
        IntegerProperty_t     neutronium;
        IntegerProperty_t     tritanium;
        IntegerProperty_t     duranium;
        IntegerProperty_t     molybdenum;                                 ///< Ore in cargo room.
        IntegerProperty_t     supplies;                                   ///< Supplies in cargo room.
        Transfer              unload;                              ///< Unload transporter. For jettison / transfer to planet.
        Transfer              transfer;                            ///< Transfer transporters. For enemy-ship transfer.
        IntegerProperty_t     missionInterceptParameter;                  ///< Mission: intercept Id.
        IntegerProperty_t     money;                                      ///< Money in cargo room.

        ShipData(int = 0);
    };

    /** Compute ship mass from ship data record.
        \param ship [in] ship data record. Must be completely filled in (i.e.
        not a history record)
        \return mass */
    IntegerProperty_t getShipMass(const ShipData& data, const game::spec::ShipList& shipList);

    /** Check validity of a ship transporter.
        \param tr Transporter
        \return true if this is an active (nonempty) transport */
    bool isTransferActive(const ShipData::Transfer& tr);

} }

#endif
