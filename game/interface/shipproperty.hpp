/**
  *  \file game/interface/shipproperty.hpp
  */
#ifndef C2NG_GAME_INTERFACE_SHIPPROPERTY_HPP
#define C2NG_GAME_INTERFACE_SHIPPROPERTY_HPP

#include "game/map/ship.hpp"
#include "afl/data/value.hpp"
#include "game/game.hpp"
#include "game/root.hpp"

namespace game { namespace interface {

    /** Definition of ship properties. */
    enum ShipProperty {
        ispAuxId,
        ispAuxAmmo,
        ispAuxCount,
        ispAuxShort,
        ispAuxName,
        ispBeamId,
        ispBeamCount,
        ispBeamShort,
        ispBeamName,
        ispCargoColonists,
        ispCargoD,
        ispCargoFree,
        ispCargoM,
        ispCargoMoney,
        ispCargoN,
        ispCargoStr,
        ispCargoSupplies,
        ispCargoT,
        ispCrew,
        ispDamage,
        ispEnemyId,
        ispEngineId,
        ispEngineName,
        ispFCode,
        ispFighterBays,
        ispFighterCount,
        ispFleetId,
        ispFleetName,
        ispFleetStatus,
        ispFleet,
        ispHasFunction,
        ispHeadingAngle,
        ispHeadingName,
        ispHullSpecial,
        ispId,
        ispLevel,
        ispLocX,
        ispLocY,
        ispLoc,
        ispMarked,
        ispMass,
        ispMissionId,
        ispMissionIntercept,
        ispMissionShort,
        ispMissionTow,
        ispMissionName,
        ispMoveETA,
        ispMoveFuel,
        ispName,
        ispOrbitId,
        ispOrbitName,
        ispPlayed,
        ispRealOwner,
        ispScore,
        ispSpeedId,
        ispSpeedName,
        ispTask,
        ispTorpId,
        ispTorpCount,
        ispTorpLCount,
        ispTorpShort,
        ispTorpName,
        ispTransferShipColonists,
        ispTransferShipD,
        ispTransferShipId,
        ispTransferShipM,
        ispTransferShipN,
        ispTransferShipName,
        ispTransferShipSupplies,
        ispTransferShipT,
        ispTransferShip,
        ispTransferUnloadColonists,
        ispTransferUnloadD,
        ispTransferUnloadId,
        ispTransferUnloadM,
        ispTransferUnloadN,
        ispTransferUnloadName,
        ispTransferUnloadSupplies,
        ispTransferUnloadT,
        ispTransferUnload,
        ispTypeChar,
        ispTypeStr,
        ispWaypointDistance,
        ispWaypointDX,
        ispWaypointDY,
        ispWaypointPlanetId,
        ispWaypointX,
        ispWaypointY,
        ispWaypointName
    };

    afl::data::Value* getShipProperty(const game::map::Ship& sh, ShipProperty isp,
                                      afl::string::Translator& tx,
                                      InterpreterInterface& iface,
                                      afl::base::Ptr<Root> root,
                                      afl::base::Ptr<game::spec::ShipList> shipList,
                                      afl::base::Ptr<Game> game);
    void setShipProperty(game::map::Ship& sh, ShipProperty isp, afl::data::Value* value,
                         afl::base::Ptr<Root> root);

} }

#endif
