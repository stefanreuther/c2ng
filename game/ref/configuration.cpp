/**
  *  \file game/ref/configuration.cpp
  */

#include "game/ref/configuration.hpp"
#include "game/config/integeroption.hpp"
#include "game/config/integervalueparser.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/game.hpp"
#include "game/ref/nullpredicate.hpp"
#include "game/ref/sortbybattleorder.hpp"
#include "game/ref/sortbydamage.hpp"
#include "game/ref/sortbyfleet.hpp"
#include "game/ref/sortbyhullmass.hpp"
#include "game/ref/sortbyhulltype.hpp"
#include "game/ref/sortbylocation.hpp"
#include "game/ref/sortbymass.hpp"
#include "game/ref/sortbyname.hpp"
#include "game/ref/sortbynewlocation.hpp"
#include "game/ref/sortbyowner.hpp"
#include "game/ref/sortbytowgroup.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "game/ref/sortbytransfertarget.hpp"

struct game::ref::ConfigurationSelection {
    game::config::IntegerOptionDescriptorWithDefault primary;
    game::config::IntegerOptionDescriptorWithDefault secondary;
};

const game::ref::ConfigurationSelection game::ref::REGULAR = {
    // ex opt_sort_ship
    // FIXME: getSortOrderConfig
    // FIXME: setSortOrderConfig
    { "Sort.Ship",           &game::config::IntegerValueParser::instance, game::ref::ConfigSortById },
    { "Sort.Ship.Secondary", &game::config::IntegerValueParser::instance, game::ref::ConfigSortById },
};

const game::ref::ConfigurationSelection game::ref::CARGO_TRANSFER = {
    { "Sort.Cargo",           &game::config::IntegerValueParser::instance, game::ref::ConfigSortByTransferTarget },
    { "Sort.Cargo.Secondary", &game::config::IntegerValueParser::instance, game::ref::ConfigSortById },
};

game::ref::SortPredicate&
game::ref::createSortPredicate(int config, Session& session, afl::base::Deleter& del)
{
    // ex GObjectList::postprocessUser (sort-of)
    Root* pRoot = session.getRoot().get();
    Game* pGame = session.getGame().get();
    game::spec::ShipList* pShipList = session.getShipList().get();
    Turn* pTurn = (pGame != 0 ? pGame->getViewpointTurn().get() : 0);

    switch (config) {
     case ConfigSortById:
        return del.addNew(new NullPredicate());

     case ConfigSortByOwner:
        if (pRoot != 0 && pTurn != 0) {
            return del.addNew(new SortByOwner(pTurn->universe(), pRoot->playerList()));
        }
        break;

     case ConfigSortByHull:
        if (pTurn != 0 && pShipList != 0) {
            return del.addNew(new SortByHullType(pTurn->universe(), *pShipList, session.translator()));
        }
        break;

     case ConfigSortByMass:
        if (pShipList != 0 && pTurn != 0) {
            return del.addNew(new SortByMass(pTurn->universe(), *pShipList));
        }
        break;

     case ConfigSortByFleet:
        if (pTurn != 0) {
            return del.addNew(new SortByFleet(pTurn->universe(), session.translator(), session.interface()));
        }
        break;

     case ConfigSortByTowGroup:
        if (pTurn != 0) {
            return del.addNew(new SortByTowGroup(pTurn->universe(), session.translator(), session.interface()));
        }
        break;

     case ConfigSortByBattleOrder:
        if (pTurn != 0 && pRoot != 0) {
            return del.addNew(new SortByBattleOrder(pTurn->universe(), pRoot->hostVersion(), session.translator()));
        }
        break;

     case ConfigSortByLocation:
        if (pTurn != 0) {
            return del.addNew(new SortByLocation(pTurn->universe(), session.translator()));
        }
        break;

     case ConfigSortByHullMass:
        if (pTurn != 0 && pShipList != 0) {
            return del.addNew(new SortByHullMass(pTurn->universe(), *pShipList));
        }
        break;

     case ConfigSortByDamage:
        if (pTurn != 0) {
            return del.addNew(new SortByDamage(pTurn->universe()));
        }
        break;

     case ConfigSortByName:
        return del.addNew(new SortByName(session));

     case ConfigSortByNewPosition:
        if (pTurn != 0 && pShipList != 0 && pRoot != 0 && pGame != 0) {
            return del.addNew(new SortByNewLocation(pTurn->universe(), *pGame, *pShipList, *pRoot, session.translator()));
        }
        break;

     case ConfigSortByTransferTarget:
        if (pTurn != 0 && pRoot != 0) {
            return del.addNew(new SortByTransferTarget(pTurn->universe(),
                                                       session.interface(),
                                                       game::map::Ship::TransferTransporter,
                                                       !pRoot->hostVersion().hasParallelShipTransfers(),
                                                       session.translator()));
        }
        break;
    }

    return del.addNew(new NullPredicate());
}



void
game::ref::fetchConfiguration(Session& session, const ConfigurationSelection& sel, Configuration& config)
{
    if (Root* pRoot = session.getRoot().get()) {
        config.order.first  = pRoot->userConfiguration()[sel.primary]();
        config.order.second = pRoot->userConfiguration()[sel.secondary]();
    }
}

void
game::ref::storeConfiguration(Session& session, const ConfigurationSelection& sel, const Configuration& config)
{
    if (Root* pRoot = session.getRoot().get()) {
        pRoot->userConfiguration()[sel.primary].set(config.order.first);
        pRoot->userConfiguration()[sel.secondary].set(config.order.second);
    }
}
