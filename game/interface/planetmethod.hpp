/**
  *  \file game/interface/planetmethod.hpp
  */
#ifndef C2NG_GAME_INTERFACE_PLANETMETHOD_HPP
#define C2NG_GAME_INTERFACE_PLANETMETHOD_HPP

#include "game/map/planet.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/process.hpp"
#include "game/shipbuildorder.hpp"

namespace game { namespace interface {

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
        ipmAutoTaxNatives           // 26
    };

    void callPlanetMethod(game::map::Planet& pl,
                          PlanetMethod ipm,
                          interpreter::Arguments& args,
                          interpreter::Process& process,
                          Session& session,
                          Turn& turn,
                          Root& root);

    bool parseBuildShipCommand(interpreter::Arguments& args, ShipBuildOrder& o, const game::spec::ShipList& shipList);
} }

#endif
