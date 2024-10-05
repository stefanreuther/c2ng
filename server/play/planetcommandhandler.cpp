/**
  *  \file server/play/planetcommandhandler.cpp
  *  \brief Class server::play::PlanetCommandHandler
  */

#include <stdexcept>
#include "server/play/planetcommandhandler.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/interface/baseproperty.hpp"
#include "game/interface/planetmethod.hpp"
#include "game/interface/planetproperty.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/turn.hpp"
#include "interpreter/process.hpp"
#include "server/errors.hpp"
#include "server/play/packerlist.hpp"
#include "server/play/planetpacker.hpp"
#include "server/play/shippacker.hpp"

namespace gi = game::interface;

namespace {
    void handleSetBuildGoals(game::map::Planet& pl, interpreter::Arguments& args)
    {
        args.checkArgumentCount(game::NUM_PLANETARY_BUILDING_TYPES * 2);
        for (size_t i = 0; i < game::NUM_PLANETARY_BUILDING_TYPES; ++i) {
            game::PlanetaryBuilding type = game::PlanetaryBuilding(i);
            int32_t tmp;
            if (interpreter::checkIntegerArg(tmp, args.getNext(), 0, game::MAX_AUTOBUILD_GOAL)) {
                pl.setAutobuildGoal(type, tmp);
            }
            if (interpreter::checkIntegerArg(tmp, args.getNext(), 0, game::MAX_AUTOBUILD_SPEED)) {
                pl.setAutobuildSpeed(type, tmp);
            }
        }
    }
}

server::play::PlanetCommandHandler::PlanetCommandHandler(game::Session& session, game::Id_t id)
    : m_session(session),
      m_id(id)
{ }

void
server::play::PlanetCommandHandler::processCommand(const String_t& cmd, interpreter::Arguments& args, PackerList& objs)
{
    // ex ServerPlanetCommandHandler::processCommand
    game::Game& g = game::actions::mustHaveGame(m_session);
    game::Root& root = game::actions::mustHaveRoot(m_session);
    game::Turn& turn = g.currentTurn();

    game::map::Planet* pPlanet = turn.universe().planets().get(m_id);
    if (pPlanet == 0) {
        throw std::runtime_error(ITEM_NOT_FOUND);
    }

    // Temporary process
    interpreter::Process process(m_session.world(), "PlanetCommandHandler", 0);

    if (cmd == "setcomment") {
        gi::callPlanetMethod(*pPlanet, gi::ipmSetComment, args, process, m_session, g.mapConfiguration(), turn, root);
        objs.addNew(new PlanetPacker(m_session, m_id));
    } else if (cmd == "setfcode") {
        args.checkArgumentCount(1);
        gi::setPlanetProperty(*pPlanet, gi::ippFCode, args.getNext(), root);
        objs.addNew(new PlanetPacker(m_session, m_id));
    } else if (cmd == "fixship") {
        gi::callPlanetMethod(*pPlanet, gi::ipmFixShip, args, process, m_session, g.mapConfiguration(), turn, root);
        objs.addNew(new PlanetPacker(m_session, m_id));
    } else if (cmd == "recycleship") {
        gi::callPlanetMethod(*pPlanet, gi::ipmRecycleShip, args, process, m_session, g.mapConfiguration(), turn, root);
        objs.addNew(new PlanetPacker(m_session, m_id));
    } else if (cmd == "buildbase") {
        gi::callPlanetMethod(*pPlanet, gi::ipmBuildBase, args, process, m_session, g.mapConfiguration(), turn, root);
        objs.addNew(new PlanetPacker(m_session, m_id));
    } else if (cmd == "autobuild") {
        gi::callPlanetMethod(*pPlanet, gi::ipmAutoBuild, args, process, m_session, g.mapConfiguration(), turn, root);
        objs.addNew(new PlanetPacker(m_session, m_id));
    } else if (cmd == "builddefense") {
        gi::callPlanetMethod(*pPlanet, gi::ipmBuildDefense, args, process, m_session, g.mapConfiguration(), turn, root);
        objs.addNew(new PlanetPacker(m_session, m_id));
    } else if (cmd == "buildfactories") {
        gi::callPlanetMethod(*pPlanet, gi::ipmBuildFactories, args, process, m_session, g.mapConfiguration(), turn, root);
        objs.addNew(new PlanetPacker(m_session, m_id));
    } else if (cmd == "buildmines") {
        gi::callPlanetMethod(*pPlanet, gi::ipmBuildMines, args, process, m_session, g.mapConfiguration(), turn, root);
        objs.addNew(new PlanetPacker(m_session, m_id));
    } else if (cmd == "buildbasedefense") {
        gi::callPlanetMethod(*pPlanet, gi::ipmBuildBaseDefense, args, process, m_session, g.mapConfiguration(), turn, root);
        objs.addNew(new PlanetPacker(m_session, m_id));
    } else if (cmd == "setcolonisttax") {
        args.checkArgumentCount(1);
        gi::setPlanetProperty(*pPlanet, gi::ippColonistTax, args.getNext(), root);
        objs.addNew(new PlanetPacker(m_session, m_id));
    } else if (cmd == "setnativetax") {
        args.checkArgumentCount(1);
        gi::setPlanetProperty(*pPlanet, gi::ippNativeTax, args.getNext(), root);
        objs.addNew(new PlanetPacker(m_session, m_id));
    } else if (cmd == "setmission") {
        args.checkArgumentCount(1);
        gi::setBaseProperty(*pPlanet, gi::ibpMission, args.getNext());
        objs.addNew(new PlanetPacker(m_session, m_id));
    } else if (cmd == "settech") {
        gi::callPlanetMethod(*pPlanet, gi::ipmSetTech, args, process, m_session, g.mapConfiguration(), turn, root);
        objs.addNew(new PlanetPacker(m_session, m_id));
    } else if (cmd == "buildfighters") {
        gi::callPlanetMethod(*pPlanet, gi::ipmBuildFighters, args, process, m_session, g.mapConfiguration(), turn, root);
        objs.addNew(new PlanetPacker(m_session, m_id));
        // FIXME: command could modify a ship
    } else if (cmd == "buildengines") {
        gi::callPlanetMethod(*pPlanet, gi::ipmBuildEngines, args, process, m_session, g.mapConfiguration(), turn, root);
        objs.addNew(new PlanetPacker(m_session, m_id));
    } else if (cmd == "buildtorps") {
        gi::callPlanetMethod(*pPlanet, gi::ipmBuildTorps, args, process, m_session, g.mapConfiguration(), turn, root);
        objs.addNew(new PlanetPacker(m_session, m_id));
        // FIXME: command could modify a ship
    } else if (cmd == "buildhulls") {
        gi::callPlanetMethod(*pPlanet, gi::ipmBuildHulls, args, process, m_session, g.mapConfiguration(), turn, root);
        objs.addNew(new PlanetPacker(m_session, m_id));
    } else if (cmd == "buildlaunchers") {
        gi::callPlanetMethod(*pPlanet, gi::ipmBuildLaunchers, args, process, m_session, g.mapConfiguration(), turn, root);
        objs.addNew(new PlanetPacker(m_session, m_id));
    } else if (cmd == "buildbeams") {
        gi::callPlanetMethod(*pPlanet, gi::ipmBuildBeams, args, process, m_session, g.mapConfiguration(), turn, root);
        objs.addNew(new PlanetPacker(m_session, m_id));
    } else if (cmd == "sellsupplies") {
        gi::callPlanetMethod(*pPlanet, gi::ipmSellSupplies, args, process, m_session, g.mapConfiguration(), turn, root);
        objs.addNew(new PlanetPacker(m_session, m_id));
    } else if (cmd == "buildship") {
        gi::callPlanetMethod(*pPlanet, gi::ipmBuildShip, args, process, m_session, g.mapConfiguration(), turn, root);
        objs.addNew(new PlanetPacker(m_session, m_id));
    } else if (cmd == "cargotransfer") {
        interpreter::Arguments save(args);
        gi::callPlanetMethod(*pPlanet, gi::ipmCargoTransfer, args, process, m_session, g.mapConfiguration(), turn, root);
        objs.addNew(new PlanetPacker(m_session, m_id));

        // Add the ship
        int32_t shipId = 0;
        save.getNext(); // skip cargospec
        if (interpreter::checkIntegerArg(shipId, save.getNext())) {
            if (turn.universe().ships().get(shipId) != 0) {
                objs.addNew(new ShipPacker(m_session, shipId));
            }
        }
    } else if (cmd == "setbuildgoals") {
        handleSetBuildGoals(*pPlanet, args);
        objs.addNew(new PlanetPacker(m_session, m_id));
    } else {
        throw std::runtime_error(UNKNOWN_COMMAND);
    }
}
