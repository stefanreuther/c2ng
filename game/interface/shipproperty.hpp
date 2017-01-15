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

    /** Get ship property.
        \param sh        Ship to inquire.
        \param isp       Ship property to inquire.
        \param tx        Translator.
        \param iface     Interface to other properties.
        \param root      Root (needed for configuration)
        \param shipList  Ship list (needed for specifications)
        \param game      Game (needed for unit score definitions)
        \param turn      Turn (needed for related units to name locations; cannot be const because name accessors are not const)
        \return property value */
    afl::data::Value* getShipProperty(const game::map::Ship& sh, ShipProperty isp,
                                      afl::string::Translator& tx,
                                      InterpreterInterface& iface,
                                      afl::base::Ptr<const Root> root,
                                      afl::base::Ptr<const game::spec::ShipList> shipList,
                                      afl::base::Ptr<const Game> game,
                                      afl::base::Ptr<Turn> turn);
    void setShipProperty(game::map::Ship& sh, ShipProperty isp, afl::data::Value* value,
                         afl::base::Ref<Root> root,
                         afl::base::Ref<game::spec::ShipList> shipList,
                         afl::base::Ref<Turn> turn);

} }

#endif
