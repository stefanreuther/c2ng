/**
  *  \file game/interface/planetmethod.hpp
  *  \brief Enum game::interface::PlanetMethod
  */
#ifndef C2NG_GAME_INTERFACE_PLANETMETHOD_HPP
#define C2NG_GAME_INTERFACE_PLANETMETHOD_HPP

#include "afl/base/optional.hpp"
#include "game/map/configuration.hpp"
#include "game/map/planet.hpp"
#include "game/session.hpp"
#include "game/shipbuildorder.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/genericvalue.hpp"
#include "interpreter/process.hpp"

namespace game { namespace interface {

    /** Planet method identifier. */
    enum PlanetMethod {
        ipmMark,                    // 0
        ipmUnmark,                  // 1
        ipmSetComment,              // 2
        ipmFixShip,                 // 3
        ipmRecycleShip,             // 4
        ipmBuildBase,               // 5
        ipmAutoBuild,               // 6
        ipmBuildDefense,            // 7
        ipmBuildFactories,          // 8
        ipmBuildMines,              // 9
        ipmSetColonistTax,          // 10
        ipmSetNativeTax,            // 11
        ipmSetFCode,                // 12
        ipmSetMission,              // 13
        ipmBuildBaseDefense,        // 14
        ipmSetTech,                 // 15
        ipmBuildFighters,           // 16
        ipmBuildEngines,            // 17
        ipmBuildHulls,              // 18
        ipmBuildLaunchers,          // 19
        ipmBuildBeams,              // 20
        ipmBuildTorps,              // 21
        ipmSellSupplies,            // 22
        ipmBuildShip,               // 23
        ipmCargoTransfer,           // 24
        ipmAutoTaxColonists,        // 25
        ipmAutoTaxNatives,          // 26
        ipmApplyBuildGoals          // 27
    };

    /** Parameter type for ipmApplyBuildGoals command (auto-build settings pack). */
    typedef interpreter::GenericValue<game::map::Planet::AutobuildSettings> AutobuildSettingsValue_t;

    /** Call planet method.
        @param pl        Planet
        @param ipm       Method identifier
        @param args      Parameters
        @param process   Process
        @param session   Session (for ship list, planet properties)
        @param mapConfig Map configuration (required indirectly through cargo transfer > mission update)
        @param turn      Turn (for universe)
        @param root      Root (for host version/configuration); mutable to attach listeners */
    void callPlanetMethod(game::map::Planet& pl,
                          PlanetMethod ipm,
                          interpreter::Arguments& args,
                          interpreter::Process& process,
                          Session& session,
                          const game::map::Configuration& mapConfig,
                          Turn& turn,
                          Root& root);

    /** Parse ship building command.
        @param [in,out] args      Parameters
        @param [in]     shipList  Ship list (for verifying indexes/limits)
        @return If order to build a ship was given, a ShipBuildOrder with nonzero getHullIndex().
                If order to cancel a ship build was given, a ShipBuildOrder with zero getHullIndex().
                If mandatory parameter is empty, Nothing.
        @throw interpreter::Error if parameters are invalid */
    afl::base::Optional<ShipBuildOrder> parseBuildShipCommand(interpreter::Arguments& args, const game::spec::ShipList& shipList);

} }

#endif
