/**
  *  \file game/interface/shipproperty.hpp
  *  \brief Enum game::interface::ShipProperty
  */
#ifndef C2NG_GAME_INTERFACE_SHIPPROPERTY_HPP
#define C2NG_GAME_INTERFACE_SHIPPROPERTY_HPP

#include "afl/data/value.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/root.hpp"
#include "game/session.hpp"

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
        ispMessages,
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
        ispReference,
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
        \param session   Session (for names, auto-tasks)
        \param root      Root (needed for configuration)
        \param shipList  Ship list (needed for specifications)
        \param game      Game (needed for unit score definitions)
        \param turn      Turn (needed for related units to name locations)
        \return newly-allocated property value */
    afl::data::Value* getShipProperty(const game::map::Ship& sh, ShipProperty isp,
                                      Session& session,
                                      afl::base::Ref<const Root> root,
                                      afl::base::Ref<const game::spec::ShipList> shipList,
                                      afl::base::Ref<const Game> game,
                                      afl::base::Ref<const Turn> turn);

    /** Set ship property.
        \param sh        Ship.
        \param isp       Ship property to inquire.
        \param value     Value to set (owned by caller)
        \param root      Root (needed for configuration)
        \param shipList  Ship list (for mission definitions)
        \param mapConfig Map configuration
        \param turn      Turn (for fleets)
        \throw interpreter::Error if property is not assignable */
    void setShipProperty(game::map::Ship& sh, ShipProperty isp, const afl::data::Value* value,
                         const Root& root,
                         const game::spec::ShipList& shipList,
                         const game::map::Configuration& mapConfig,
                         Turn& turn);

} }

#endif
