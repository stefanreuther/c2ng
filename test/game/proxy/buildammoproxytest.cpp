/**
  *  \file test/game/proxy/buildammoproxytest.cpp
  *  \brief Test for game::proxy::BuildAmmoProxy
  */

#include "game/proxy/buildammoproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/shiplist.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"

namespace {
    const int PLAYER_NR = 4;
    const int PLANET_ID = 77;
    const int X = 1000;
    const int Y = 2000;

    /* Prepare session with
       - root
       - specification
       - one planet */
    void prepare(game::test::SessionThread& t)
    {
        // Create ship list
        afl::base::Ptr<game::spec::ShipList> shipList = new game::spec::ShipList();
        game::test::initPListBeams(*shipList);
        game::test::initPListTorpedoes(*shipList);
        game::test::addTranswarp(*shipList);
        game::test::addAnnihilation(*shipList);
        t.session().setShipList(shipList);

        // Create root
        afl::base::Ptr<game::Root> r = game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(3,0,0)), game::RegistrationKey::Unregistered, 10).asPtr();
        t.session().setRoot(r);

        // Create game with universe
        afl::base::Ptr<game::Game> g = new game::Game();
        game::map::Planet* p = g->currentTurn().universe().planets().create(PLANET_ID);
        game::map::PlanetData pd;
        pd.owner = PLAYER_NR;
        pd.colonistClans = 100;
        pd.money = 10000;
        pd.supplies = 5000;
        pd.minedTritanium = 2000;
        pd.minedDuranium = 3000;
        pd.minedMolybdenum = 4000;
        p->addCurrentPlanetData(pd, game::PlayerSet_t(PLAYER_NR));

        game::map::BaseData bd;
        for (int i = 1; i < 10; ++i) {
            // Set base storage
            bd.torpedoStorage.set(i, 0);
        }
        for (int i = 0; i < 4; ++i) {
            bd.techLevels[i] = 3;
        }
        p->addCurrentBaseData(bd, game::PlayerSet_t(PLAYER_NR));
        p->setPosition(game::map::Point(X, Y));
        p->setName("P");
        t.session().setGame(g);
        t.session().postprocessTurn(g->currentTurn(), game::PlayerSet_t(PLAYER_NR), game::PlayerSet_t(PLAYER_NR), game::map::Object::Playable);
    }

    /* Add ship to given session */
    void addShip(game::test::SessionThread& t, int x, int y, game::Id_t id, String_t friendlyCode, String_t name)
    {
        game::map::Ship* sh = t.session().getGame()->currentTurn().universe().ships().create(id);
        game::map::ShipData sd;
        sd.owner = PLAYER_NR;
        sd.friendlyCode = friendlyCode;
        sd.name = name;
        sd.x = x;
        sd.y = y;
        sd.hullType = game::test::ANNIHILATION_HULL_ID;
        sd.engineType = 9;
        sd.beamType = 0;
        sd.numBeams = 0;
        sd.torpedoType = 2;
        sd.numLaunchers = 1;
        sd.crew = 10;
        sd.ammo = 20;
        sh->addCurrentShipData(sd, game::PlayerSet_t(PLAYER_NR));
        sh->internalCheck(game::PlayerSet_t(PLAYER_NR), 15);
        sh->setPlayability(game::map::Object::Playable);
    }

    /* Receive updates from a proxy */
    class UpdateReceiver {
     public:
        const game::proxy::BuildAmmoProxy::Status& getResult() const
            { return m_result; }

        void onUpdate(const game::proxy::BuildAmmoProxy::Status& status)
            { m_result = status; }
     private:
        game::proxy::BuildAmmoProxy::Status m_result;
    };
}

/** Test behaviour on empty session. */
AFL_TEST("game.proxy.BuildAmmoProxy:empty", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::BuildAmmoProxy testee(t.gameSender(), ind, 99);

    game::proxy::BuildAmmoProxy::Status st;
    testee.getStatus(ind, st);

    a.checkEqual("01. parts", st.parts.size(), 0U);
    a.checkEqual("02. cost", st.cost.isZero(), true);
    a.checkEqual("03. available", st.available.isZero(), true);
    a.checkEqual("04. remaining", st.remaining.isZero(), true);
    a.checkEqual("05. missing", st.missing.isZero(), true);
}

/** Test behaviour for planet/planet build. */
AFL_TEST("game.proxy.BuildAmmoProxy:planet", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    game::proxy::BuildAmmoProxy testee(t.gameSender(), ind, PLANET_ID);
    testee.setPlanet();

    game::proxy::BuildAmmoProxy::Status st;
    testee.getStatus(ind, st);

    a.checkEqual("01. parts", st.parts.size(), 11U);
    a.checkEqual("02. parts", st.parts[1].type, game::Element::fromTorpedoType(2));
    a.checkEqual("03. parts", st.parts[1].page, game::spec::info::TorpedoPage);
    a.checkEqual("04. parts", st.parts[1].id, 2);
    a.checkEqual("05. parts", st.parts[1].name, "Fusion Bomb");
    a.checkEqual("06. parts", st.parts[1].techStatus, game::AvailableTech);
    a.checkEqual("07. parts", st.parts[1].isAccessible, true);
    a.checkEqual("08. parts", st.parts[1].techLevel, 3);
    a.checkEqual("09. parts", st.parts[1].amount, 0);
    a.checkEqual("10. parts", st.parts[1].maxAmount, 10000);
    a.checkEqual("11. parts", st.parts[1].cost.toCargoSpecString(), "1TDM 8$");

    a.checkEqual("21. parts", st.parts[9].type, game::Element::fromTorpedoType(10));
    a.checkEqual("22. parts", st.parts[9].page, game::spec::info::TorpedoPage);
    a.checkEqual("23. parts", st.parts[9].id, 10);
    a.checkEqual("24. parts", st.parts[9].name, "Selphyr-Fataro-Dev.");
    a.checkEqual("25. parts", st.parts[9].techStatus, game::BuyableTech);
    a.checkEqual("26. parts", st.parts[9].isAccessible, true);
    a.checkEqual("27. parts", st.parts[9].techLevel, 10);
    a.checkEqual("28. parts", st.parts[9].amount, 0);
    a.checkEqual("29. parts", st.parts[9].maxAmount, 10000);
    a.checkEqual("30. parts", st.parts[9].cost.toCargoSpecString(), "1TDM 80$");

    a.checkEqual("31. parts", st.parts[10].type, game::Element::Fighters);
    a.checkEqual("32. parts", st.parts[10].page, game::spec::info::FighterPage);
    a.checkEqual("33. parts", st.parts[10].id, PLAYER_NR);
    a.check("34. parts", st.parts[10].name.find("ighter") != String_t::npos); // matches if race name is included or not
    a.checkEqual("35. parts", st.parts[10].techStatus, game::AvailableTech);
    a.checkEqual("36. parts", st.parts[10].isAccessible, true);
    a.checkEqual("37. parts", st.parts[10].techLevel, 1);
    a.checkEqual("38. parts", st.parts[10].amount, 0);
    a.checkEqual("39. parts", st.parts[10].maxAmount, 60);
    a.checkEqual("40. parts", st.parts[10].cost.toCargoSpecString(), "3T 2M 100$");

    a.checkEqual("41. cost", st.cost.isZero(), true);
    a.checkEqual("42. available", st.available.toCargoSpecString(), "2000T 3000D 4000M 5000S 10000$");
    a.checkEqual("43. remaining", st.remaining.toCargoSpecString(), "2000T 3000D 4000M 5000S 10000$");
    a.checkEqual("44. missing", st.missing.isZero(), true);
    a.checkEqual("45. targetName", st.targetName, "P");
    a.checkEqual("46. available", st.availableTech, 3);

    // Buy something and wait for update
    UpdateReceiver recv;
    testee.sig_update.add(&recv, &UpdateReceiver::onUpdate);
    testee.addLimitCash(game::Element::fromTorpedoType(2), 3);
    t.sync();
    ind.processQueue();

    const game::proxy::BuildAmmoProxy::Status& rst = recv.getResult();
    a.checkEqual("51. size", rst.parts.size(), 11U);
    a.checkEqual("52. amount", rst.parts[1].amount, 3);
    a.checkEqual("53, maxAmount", rst.parts[1].maxAmount, 10000);
    a.checkEqual("54. cost", rst.cost.toCargoSpecString(), "3TDM 24$");
    a.checkEqual("55. remaining", rst.remaining.toCargoSpecString(), "1997T 2997D 3997M 5000S 9976$");
    a.checkEqual("56. targetName", rst.targetName, "P");

    // Commit
    testee.commit();
    t.sync();
    ind.processQueue();

    // Verify
    a.checkEqual("61. Torpedoes", t.session().getGame()->currentTurn().universe().planets().get(PLANET_ID)->getCargo(game::Element::fromTorpedoType(2)).orElse(0), 3);
}

/** Test behaviour for planet/ship build. */
AFL_TEST("game.proxy.BuildAmmoProxy:ship", a)
{
    const int SHIP_ID = 456;
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    addShip(t, X, Y, SHIP_ID, "hi", "USS Nerf");
    game::proxy::BuildAmmoProxy testee(t.gameSender(), ind, PLANET_ID);
    testee.setShip(SHIP_ID);

    game::proxy::BuildAmmoProxy::Status st;
    testee.getStatus(ind, st);
    a.checkEqual("01. parts", st.parts.size(), 11U);
    a.checkEqual("02. parts", st.parts[0].isAccessible, false);
    a.checkEqual("03. parts", st.parts[1].isAccessible, true);
    a.checkEqual("04. parts", st.parts[2].isAccessible, false);
    a.checkEqual("05. parts", st.parts[10].isAccessible, false);
    a.checkEqual("06. targetName", st.targetName, "USS Nerf");
    a.checkEqual("07. available", st.availableTech, 3);

    // Buy something and wait for update
    UpdateReceiver recv;
    testee.sig_update.add(&recv, &UpdateReceiver::onUpdate);
    testee.addLimitCash(game::Element::fromTorpedoType(2), 3);
    t.sync();
    ind.processQueue();

    const game::proxy::BuildAmmoProxy::Status& rst = recv.getResult();
    a.checkEqual("11. size", rst.parts.size(), 11U);
    a.checkEqual("12. amount", rst.parts[1].amount, 23);
    a.checkEqual("13. maxAmount", rst.parts[1].maxAmount, 320);
    a.checkEqual("14. cost", rst.cost.toCargoSpecString(), "3TDM 24$");
    a.checkEqual("15. remaining", rst.remaining.toCargoSpecString(), "1997T 2997D 3997M 5000S 9976$");
    a.checkEqual("16. targetName", rst.targetName, "USS Nerf");

    // Commit
    testee.commit();
    t.sync();
    ind.processQueue();

    // Verify
    a.checkEqual("21. Torpedoes", t.session().getGame()->currentTurn().universe().ships().get(SHIP_ID)->getCargo(game::Element::fromTorpedoType(2)).orElse(0), 23);
}

/** Test behaviour for planet/ship build, wrong ship. */
AFL_TEST("game.proxy.BuildAmmoProxy:far-ship", a)
{
    const int SHIP_ID = 456;
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    addShip(t, X+10, Y, SHIP_ID, "hi", "USS Nerf");      // note differing position
    game::proxy::BuildAmmoProxy testee(t.gameSender(), ind, PLANET_ID);
    testee.setShip(SHIP_ID);

    game::proxy::BuildAmmoProxy::Status st;
    testee.getStatus(ind, st);
    a.checkEqual("01. parts", st.parts.size(), 11U);
    a.checkEqual("02. parts", st.parts[0].isAccessible, false);
    a.checkEqual("03. parts", st.parts[1].isAccessible, false);
    a.checkEqual("04. parts", st.parts[2].isAccessible, false);
    a.checkEqual("05. parts", st.parts[10].isAccessible, false);
}
