/**
  *  \file test/game/proxy/hullspecificationproxytest.cpp
  *  \brief Test for game::proxy::HullSpecificationProxy
  */

#include "game/proxy/hullspecificationproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "client/picturenamer.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"

namespace {
    using afl::base::Ptr;
    using game::HostVersion;
    using game::Root;
    using game::map::Ship;
    using game::proxy::HullSpecificationProxy;
    using game::spec::Cost;
    using game::spec::Hull;
    using game::spec::ShipList;

    const int HULL_NR = 57;

    void addShipList(game::test::SessionThread& s)
    {
        Ptr<ShipList> shipList = new ShipList();

        // Add hull
        Hull& h = *shipList->hulls().create(HULL_NR);
        h.setMass(150);
        h.setTechLevel(5);
        h.setName("FIRST CLASS STARSHIP");
        h.cost().set(Cost::Tritanium, 40);
        h.cost().set(Cost::Duranium, 50);
        h.cost().set(Cost::Molybdenum, 60);
        h.cost().set(Cost::Money, 128);
        h.setExternalPictureNumber(12);
        h.setInternalPictureNumber(13);
        h.setMaxFuel(250);
        h.setMaxCrew(100);
        h.setNumEngines(4);
        h.setMaxCargo(600);
        h.setNumBays(0);
        h.setMaxLaunchers(3);
        h.setMaxBeams(8);
        h.changeHullFunction(shipList->modifiedHullFunctions().getFunctionIdFromHostId(99),
                             game::PlayerSet_t::allUpTo(game::MAX_PLAYERS),
                             game::PlayerSet_t(),
                             true);

        // Buildable by 1+4
        shipList->hullAssignments().add(1, 1, HULL_NR);
        shipList->hullAssignments().add(4, 7, HULL_NR);

        // HullFunction
        game::spec::BasicHullFunction* b = shipList->basicHullFunctions().addFunction(99, "Func");
        b->setDescription("Func Desc");

        s.session().setShipList(shipList);
    }

    void addGame(game::test::SessionThread& h)
    {
        h.session().setGame(new game::Game());
    }

    void addRoot(game::test::SessionThread& h)
    {
        Ptr<Root> r = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(4,0,0))).asPtr();
        r->hostConfiguration()[game::config::HostConfiguration::BuildQueue].set("PBP");
        h.session().setRoot(r);
    }

    void addShip(game::test::SessionThread& h, int id)
    {
        Ship& sh = *h.session().getGame()->currentTurn().universe().ships().create(id);
        sh.setHull(HULL_NR);
    }

    struct UpdateReceiver {
        HullSpecificationProxy::HullSpecification result;

        void onUpdate(const HullSpecificationProxy::HullSpecification& r)
            { result = r; }
    };
}

/** Simple test.
    A: create a ship and a hull. Request its specification.
    E: correct specification reported */
AFL_TEST("game.proxy.HullSpecificationProxy:normal", a)
{
    // Environment

    const int SHIP_ID = 42;
    game::test::SessionThread h;
    addShipList(h);
    addGame(h);
    addRoot(h);
    addShip(h, SHIP_ID);

    // Testee
    game::test::WaitIndicator ind;
    util::RequestDispatcher& disp = ind;
    game::proxy::HullSpecificationProxy testee(h.gameSender(), disp, std::auto_ptr<game::spec::info::PictureNamer>(new client::PictureNamer()));

    UpdateReceiver recv;
    testee.sig_update.add(&recv, &UpdateReceiver::onUpdate);

    // Request specification
    testee.setExistingShipId(SHIP_ID);
    h.sync();
    ind.processQueue();
    a.check("01. name", !recv.result.name.empty());

    // Verify
    a.checkEqual("11. name",               recv.result.name, "FIRST CLASS STARSHIP");
    a.checkEqual("12. image",              recv.result.image, "ship.13.57");
    a.checkEqual("13. hullId",             recv.result.hullId, HULL_NR);
    a.checkEqual("14. mass",               recv.result.mass, 150);
    a.checkEqual("15. numEngines",         recv.result.numEngines, 4);
    a.checkEqual("16. techLevel",          recv.result.techLevel, 5);
    a.checkEqual("17. maxCrew",            recv.result.maxCrew, 100);
    a.checkEqual("18. maxCargo",           recv.result.maxCargo, 600);
    a.checkEqual("19. maxFuel",            recv.result.maxFuel, 250);
    a.checkEqual("20. maxBeams",           recv.result.maxBeams, 8);
    a.checkEqual("21. maxLaunchers",       recv.result.maxLaunchers, 3);
    a.checkEqual("22. numBays",            recv.result.numBays, 0);
    a.checkEqual("23. mineHitDamage",      recv.result.mineHitDamage, 66);
    a.checkEqual("24. fuelBurnPerTurn",    recv.result.fuelBurnPerTurn, 0);
    a.checkEqual("25. fuelBurnPerFight",   recv.result.fuelBurnPerFight, 0);

    a.checkEqual("31. cost",               recv.result.cost.toCargoSpecString(), "40T 50D 60M 128$");

    a.checkEqual("41. pointsToBuild",      recv.result.pointsToBuild, 400);
    a.checkEqual("42. pointsForKilling",   recv.result.pointsForKilling, 180);
    a.checkEqual("43. pointsForScrapping", recv.result.pointsForScrapping, 60);

    a.checkEqual("51. players",            recv.result.players, game::PlayerSet_t() + 1 + 4);

    // Weapon effects
    game::spec::info::WeaponEffects eff;
    testee.describeWeaponEffects(ind, eff);
    a.checkEqual("61. mass", eff.mass, 150);
    a.checkEqual("62. fighterEffects", eff.fighterEffects.size(), 1U);

    // Hull function details
    game::spec::info::AbilityDetails_t ab;
    testee.describeHullFunctionDetails(ind, ab, false);
    a.check("71. abilities", ab.size() >= 1U);
}

/** Test setQuery().
    A: create a hull. Request its data using setQuery().
    E: correct specification reported */
AFL_TEST("game.proxy.HullSpecificationProxy:setQuery", a)
{
    // Environment

    game::test::SessionThread h;
    addShipList(h);
    addGame(h);
    addRoot(h);

    // Testee
    game::test::WaitIndicator ind;
    util::RequestDispatcher& disp = ind;
    game::proxy::HullSpecificationProxy testee(h.gameSender(), disp, std::auto_ptr<game::spec::info::PictureNamer>(new client::PictureNamer()));

    UpdateReceiver recv;
    testee.sig_update.add(&recv, &UpdateReceiver::onUpdate);

    // Request specification
    game::ShipQuery q;
    q.setHullType(57);
    testee.setQuery(q);
    h.sync();
    ind.processQueue();
    a.check("01. name", !recv.result.name.empty());

    // Verify
    a.checkEqual("11. name",   recv.result.name, "FIRST CLASS STARSHIP");
    a.checkEqual("12. hullId", recv.result.hullId, HULL_NR);
}
