/**
  *  \file game/map/shipdata.hpp
  *  \brief Structure game::map::ShipData
  */
#ifndef C2NG_GAME_MAP_SHIPDATA_HPP
#define C2NG_GAME_MAP_SHIPDATA_HPP

#include "game/types.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace map {

    /** Ship data.
        Represents data for one ship.
        All fields are optional to be able to represent partial data from history.
        This structure is published separately to allow low-level algorithms to be made;
        normal code uses class Ship. */
    struct ShipData {
        /** Ship transporter. */
        struct Transfer {
            IntegerProperty_t neutronium;                          ///< Neutronium to transfer.
            IntegerProperty_t tritanium;                           ///< Tritanium to transfer.
            IntegerProperty_t duranium;                            ///< Duranium to transfer.
            IntegerProperty_t molybdenum;                          ///< Molybdenum to transfer.
            IntegerProperty_t colonists;                           ///< Clans to transfer.
            IntegerProperty_t supplies;                            ///< Supplies to transfer.
            IntegerProperty_t targetId;                            ///< Target ship/planet Id depending on type.
        };

        IntegerProperty_t     owner;                               ///< Ship owner.
        StringProperty_t      friendlyCode;                        ///< Friendly code.
        IntegerProperty_t     warpFactor;                          ///< Warp factor.
        NegativeProperty_t    waypointDX;                          ///< Waypoint displacement, X.
        NegativeProperty_t    waypointDY;                          ///< Waypoint displacement, Y.
        IntegerProperty_t     x;                                   ///< Position, X.
        IntegerProperty_t     y;                                   ///< Position, Y.
        IntegerProperty_t     engineType;                          ///< Engine type.
        IntegerProperty_t     hullType;                            ///< Hull type.
        IntegerProperty_t     beamType;                            ///< Beam type.
        IntegerProperty_t     numBeams;                            ///< Number of beams.
        IntegerProperty_t     numBays;                             ///< Number of fighter bays.
        IntegerProperty_t     launcherType;                        ///< Torpedo type.
        IntegerProperty_t     ammo;                                ///< Number of torpedoes or fighters.
        IntegerProperty_t     numLaunchers;                        ///< Number of torpedo launchers.
        IntegerProperty_t     mission;                             ///< Mission.
        IntegerProperty_t     primaryEnemy;                        ///< Primary enemy.
        IntegerProperty_t     missionTowParameter;                 ///< Mission: tow Id.
        IntegerProperty_t     damage;                              ///< Damage.
        IntegerProperty_t     crew;                                ///< Current crew.
        IntegerProperty_t     colonists;                           ///< Colonists in cargo room.
        StringProperty_t      name;                                ///< Ship name.
        IntegerProperty_t     neutronium;                          ///< Neutronium cargo.
        IntegerProperty_t     tritanium;                           ///< Tritanium cargo.
        IntegerProperty_t     duranium;                            ///< Duranium cargo.
        IntegerProperty_t     molybdenum;                          ///< Molybdenum cargo.
        IntegerProperty_t     supplies;                            ///< Supplies in cargo room.
        Transfer              unload;                              ///< Unload transporter. For jettison / transfer to planet.
        Transfer              transfer;                            ///< Transfer transporters. For enemy-ship transfer.
        IntegerProperty_t     missionInterceptParameter;           ///< Mission: intercept Id.
        IntegerProperty_t     money;                               ///< Money in cargo room.

        /** Constructor.
            \param id Ignored (provided to allow ObjectVector<ShipData> which instantiates using an Id). */
        ShipData(int = 0);
    };

    /** Compute ship mass from ship data record.
        \param [in] ship ship data record
        \param [in] shipList ship list (for beams, torpedoes)
        \return mass (unknown if any value is unknown) */
    IntegerProperty_t getShipMass(const ShipData& data, const game::spec::ShipList& shipList);

    /** Check validity of a ship transporter.
        \param tr Transporter
        \return true if this is an active (nonempty) transport */
    bool isTransferActive(const ShipData::Transfer& tr);

} }

#endif
