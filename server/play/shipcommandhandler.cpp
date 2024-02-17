/**
  *  \file server/play/shipcommandhandler.cpp
  */

#include <stdexcept>
#include "server/play/shipcommandhandler.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/interface/shipmethod.hpp"
#include "game/interface/shipproperty.hpp"
#include "server/errors.hpp"
#include "server/play/packerlist.hpp"
#include "server/play/planetpacker.hpp"
#include "server/play/shippacker.hpp"
#include "server/play/shipxypacker.hpp"

namespace gi = game::interface;

namespace {
    void addPlanet(server::play::PackerList& objs,
                   game::Session& session,
                   const game::map::Ship& ship,
                   const game::map::Universe& univ)
    {
        game::map::Point pt;
        if (ship.getPosition().get(pt)) {
            if (game::Id_t id = univ.findPlanetAt(pt)) {
                objs.addNew(new server::play::PlanetPacker(session, id));
            }
        }
    }
}


server::play::ShipCommandHandler::ShipCommandHandler(game::Session& session, game::Id_t id)
    : m_session(session),
      m_id(id)
{ }

void
server::play::ShipCommandHandler::processCommand(const String_t& cmd, interpreter::Arguments& args, PackerList& objs)
{
    // ex ServerPlanetCommandHandler::processCommand
    // Preconditions
    game::Game& g = game::actions::mustHaveGame(m_session);
    game::Root& root = game::actions::mustHaveRoot(m_session);
    game::Turn& turn = g.currentTurn();
    game::spec::ShipList& sl = game::actions::mustHaveShipList(m_session);

    game::map::Ship* pShip = turn.universe().ships().get(m_id);
    if (pShip == 0) {
        throw std::runtime_error(ITEM_NOT_FOUND);
    }

    // Temporary process
    interpreter::Process process(m_session.world(), "ShipCommandHandler", 0);

    // Missing: mark, unmark, fixship, recycleship
    if (cmd == "setcomment") {
        gi::callShipMethod(*pShip, gi::ismSetComment, args, process, m_session, root, g.mapConfiguration(), sl, turn);
        objs.addNew(new ShipPacker(m_session, m_id));
    } else if (cmd == "setfcode") {
        args.checkArgumentCount(1);
        gi::setShipProperty(*pShip, gi::ispFCode, args.getNext(), root, sl, g.mapConfiguration(), turn.universe());
        objs.addNew(new ShipPacker(m_session, m_id));
    } else if (cmd == "setname") {
        args.checkArgumentCount(1);
        gi::setShipProperty(*pShip, gi::ispName, args.getNext(), root, sl, g.mapConfiguration(), turn.universe());
        objs.addNew(new ShipXYPacker(m_session));
    } else if (cmd == "setwaypoint") {
        gi::callShipMethod(*pShip, gi::ismSetWaypoint, args, process, m_session, root, g.mapConfiguration(), sl, turn);
        objs.addNew(new ShipPacker(m_session, m_id));
    } else if (cmd == "setenemy") {
        args.checkArgumentCount(1);
        gi::setShipProperty(*pShip, gi::ispEnemyId, args.getNext(), root, sl, g.mapConfiguration(), turn.universe());
        objs.addNew(new ShipPacker(m_session, m_id));
    } else if (cmd == "setspeed") {
        args.checkArgumentCount(1);
        gi::setShipProperty(*pShip, gi::ispSpeedId, args.getNext(), root, sl, g.mapConfiguration(), turn.universe());
        objs.addNew(new ShipPacker(m_session, m_id));
    } else if (cmd == "setmission") {
        gi::callShipMethod(*pShip, gi::ismSetMission, args, process, m_session, root, g.mapConfiguration(), sl, turn);
        objs.addNew(new ShipPacker(m_session, m_id));
    } else if (cmd == "cargotransfer") {
        interpreter::Arguments save(args);
        gi::callShipMethod(*pShip, gi::ismCargoTransfer, args, process, m_session, root, g.mapConfiguration(), sl, turn);
        objs.addNew(new ShipPacker(m_session, m_id));

        // Add the other ship. If there's anything wrong, the command above will have failed,
        // so we can take it easy regarding error checking here.
        int32_t otherShipId = 0;
        save.getNext(); // skip cargospec
        if (interpreter::checkIntegerArg(otherShipId, save.getNext())) {
            if (turn.universe().ships().get(otherShipId) != 0) {
                objs.addNew(new ShipPacker(m_session, otherShipId));
            }
        }
    } else if (cmd == "cargoupload") {
        gi::callShipMethod(*pShip, gi::ismCargoUpload, args, process, m_session, root, g.mapConfiguration(), sl, turn);
        objs.addNew(new ShipPacker(m_session, m_id));
        addPlanet(objs, m_session, *pShip, turn.universe());
    } else if (cmd == "cargounload") {
        gi::callShipMethod(*pShip, gi::ismCargoUnload, args, process, m_session, root, g.mapConfiguration(), sl, turn);
        objs.addNew(new ShipPacker(m_session, m_id));
        addPlanet(objs, m_session, *pShip, turn.universe());
    } else {
        throw std::runtime_error(UNKNOWN_COMMAND);
    }
}
