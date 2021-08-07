/**
  *  \file u/t_game_proxy_hullspecificationproxy.cpp
  *  \brief Test for game::proxy::HullSpecificationProxy
  */

#include "game/proxy/hullspecificationproxy.hpp"

#include "t_game_proxy.hpp"
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
        Ptr<Root> r = new game::test::Root(HostVersion(HostVersion::PHost, MKVERSION(4,0,0)));
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
void
TestGameProxyHullSpecificationProxy::testIt()
{
    // Environment
    CxxTest::setAbortTestOnFail(true);

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
    TS_ASSERT(!recv.result.name.empty());

    // Verify
    TS_ASSERT_EQUALS(recv.result.name, "FIRST CLASS STARSHIP");
    TS_ASSERT_EQUALS(recv.result.image, "ship.13.57");
    TS_ASSERT_EQUALS(recv.result.hullId, HULL_NR);
    TS_ASSERT_EQUALS(recv.result.mass, 150);
    TS_ASSERT_EQUALS(recv.result.numEngines, 4);
    TS_ASSERT_EQUALS(recv.result.techLevel, 5);
    TS_ASSERT_EQUALS(recv.result.maxCrew, 100);
    TS_ASSERT_EQUALS(recv.result.maxCargo, 600);
    TS_ASSERT_EQUALS(recv.result.maxFuel, 250);
    TS_ASSERT_EQUALS(recv.result.maxBeams, 8);
    TS_ASSERT_EQUALS(recv.result.maxLaunchers, 3);
    TS_ASSERT_EQUALS(recv.result.numBays, 0);
    TS_ASSERT_EQUALS(recv.result.mineHitDamage, 66);
    TS_ASSERT_EQUALS(recv.result.fuelBurnPerTurn, 0);
    TS_ASSERT_EQUALS(recv.result.fuelBurnPerFight, 0);

    TS_ASSERT_EQUALS(recv.result.cost.toCargoSpecString(), "40T 50D 60M 128$");

    TS_ASSERT_EQUALS(recv.result.pointsToBuild, 400);
    TS_ASSERT_EQUALS(recv.result.pointsForKilling, 180);
    TS_ASSERT_EQUALS(recv.result.pointsForScrapping, 60);

    TS_ASSERT_EQUALS(recv.result.players, game::PlayerSet_t() + 1 + 4);

    // Weapon effects
    game::spec::info::WeaponEffects eff;
    testee.describeWeaponEffects(ind, eff);
    TS_ASSERT_EQUALS(eff.mass, 150);
    TS_ASSERT_EQUALS(eff.fighterEffects.size(), 1U);

    // Hull function details
    game::spec::info::AbilityDetails_t ab;
    testee.describeHullFunctionDetails(ind, ab);
    TS_ASSERT(ab.size() >= 1U);
}

