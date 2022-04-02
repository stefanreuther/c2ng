/**
  *  \file u/t_game_proxy_basestorageproxy.cpp
  *  \brief Test for game::proxy::BaseStorageProxy
  */

#include "game/proxy/basestorageproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/test/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/map/universe.hpp"
#include "game/map/planet.hpp"
#include "game/game.hpp"
#include "game/turn.hpp"

namespace {
    const int PLAYER_NR = 4;
    const int PLANET_ID = 77;

    void prepare(game::test::SessionThread& t)
    {
        // Create ship list
        afl::base::Ptr<game::spec::ShipList> shipList = new game::spec::ShipList();
        game::test::initPListBeams(*shipList);
        game::test::initPListTorpedoes(*shipList);
        game::test::addTranswarp(*shipList);
        game::test::addOutrider(*shipList);
        shipList->hullAssignments().add(PLAYER_NR, 3, game::test::OUTRIDER_HULL_ID);
        t.session().setShipList(shipList);

        // Create root
        afl::base::Ptr<game::Root> r = new game::test::Root(game::HostVersion(game::HostVersion::PHost, MKVERSION(3,0,0)), game::RegistrationKey::Unregistered, 7);
        t.session().setRoot(r);

        // Create game with universe
        afl::base::Ptr<game::Game> g = new game::Game();
        game::map::Planet* p = g->currentTurn().universe().planets().create(PLANET_ID);
        game::map::PlanetData pd;
        pd.owner = PLAYER_NR;
        pd.colonistClans = 100;
        p->addCurrentPlanetData(pd, game::PlayerSet_t(PLAYER_NR));

        game::map::BaseData bd;
        for (int i = 1; i < 10; ++i) {
            // Set base storage with variable amounts derived from slot number
            bd.engineStorage.set(i, i&1);
            bd.hullStorage.set(i, i&2);
            bd.beamStorage.set(i, i&3);
            bd.launcherStorage.set(i, i&4);
        }
        for (int i = 0; i < 4; ++i) {
            bd.techLevels[i] = 3;
        }
        bd.owner = PLAYER_NR;
        p->addCurrentBaseData(bd, game::PlayerSet_t(PLAYER_NR));
        p->setPosition(game::map::Point(1000, 1000));
        p->setName("P");
        g->currentTurn().universe().postprocess(game::PlayerSet_t(PLAYER_NR), game::PlayerSet_t(PLAYER_NR), game::map::Object::ReadOnly,
                                                r->hostVersion(), r->hostConfiguration(), 12, *shipList, t.session().translator(), t.session().log());
        t.session().setGame(g);
    }

    class UpdateReceiver {
     public:
        UpdateReceiver(game::TechLevel area)
            : m_area(area)
            { }

        const game::proxy::BaseStorageProxy::Parts_t& getResult() const
            { return m_result; }

        void onUpdate(game::TechLevel area, const game::proxy::BaseStorageProxy::Parts_t& list)
            {
                if (area == m_area) {
                    m_result = list;
                }
            }
     private:
        const game::TechLevel m_area;
        game::proxy::BaseStorageProxy::Parts_t m_result;
    };
}

/** Test behaviour on empty session.
    A: create empty session. Call getParts().
    E: empty list returned */
void
TestGameProxyBaseStorageProxy::testEmpty()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::BaseStorageProxy testee(t.gameSender(), ind, 99);

    // Query list
    game::proxy::BaseStorageProxy::Parts_t list;
    testee.getParts(ind, game::BeamTech, list);

    // Verify
    TS_ASSERT_EQUALS(list.size(), 0U);
}

/** Test getParts().
    A: create session and populate with planet and ship list. Call getParts().
    E: verify returned lists */
void
TestGameProxyBaseStorageProxy::testGetParts()
{
    game::test::SessionThread t;
    prepare(t);
    game::test::WaitIndicator ind;
    game::proxy::BaseStorageProxy testee(t.gameSender(), ind, PLANET_ID);

    // Query hulls: expect 1
    {
        game::proxy::BaseStorageProxy::Parts_t list;
        testee.getParts(ind, game::HullTech, list);
        TS_ASSERT_EQUALS(list.size(), 1U);
        TS_ASSERT_EQUALS(list[0].id, game::test::OUTRIDER_HULL_ID);
        TS_ASSERT_EQUALS(list[0].numParts, 2);  /* Slot 3, masked '&2' */
        TS_ASSERT_EQUALS(list[0].techStatus, game::AvailableTech);
        TS_ASSERT_EQUALS(list[0].name, "OUTRIDER CLASS SCOUT");
    }

    // Query engines: expect 1
    {
        game::proxy::BaseStorageProxy::Parts_t list;
        testee.getParts(ind, game::EngineTech, list);
        TS_ASSERT_EQUALS(list.size(), 1U);
        TS_ASSERT_EQUALS(list[0].id, 9);
        TS_ASSERT_EQUALS(list[0].numParts, 1);   /* Slot 9, masked '&1' */
        TS_ASSERT_EQUALS(list[0].techStatus, game::LockedTech);
        TS_ASSERT_EQUALS(list[0].name, "Transwarp Drive");
    }

    // Query beams: expect 10
    {
        game::proxy::BaseStorageProxy::Parts_t list;
        testee.getParts(ind, game::BeamTech, list);
        TS_ASSERT_EQUALS(list.size(), 10U);
        TS_ASSERT_EQUALS(list[0].id, 1);
        TS_ASSERT_EQUALS(list[0].numParts, 1);   /* Slot 1, masked '&3' */
        TS_ASSERT_EQUALS(list[0].techStatus, game::AvailableTech);
        TS_ASSERT_EQUALS(list[0].name, "Laser Cannon");

        TS_ASSERT_EQUALS(list[5].id, 6);
        TS_ASSERT_EQUALS(list[5].numParts, 2);   /* Slot 6, masked '&3' */
        TS_ASSERT_EQUALS(list[5].techStatus, game::BuyableTech);
        TS_ASSERT_EQUALS(list[5].name, "Electron Ram");
    }

    // Query torpedoes: expect 10
    {
        game::proxy::BaseStorageProxy::Parts_t list;
        testee.getParts(ind, game::TorpedoTech, list);
        TS_ASSERT_EQUALS(list.size(), 10U);
        TS_ASSERT_EQUALS(list[0].id, 1);
        TS_ASSERT_EQUALS(list[0].numParts, 0);   /* Slot 1, masked '&4' */
        TS_ASSERT_EQUALS(list[0].techStatus, game::AvailableTech);
        TS_ASSERT_EQUALS(list[0].name, "Space Rocket");

        TS_ASSERT_EQUALS(list[6].id, 7);
        TS_ASSERT_EQUALS(list[6].numParts, 4);   /* Slot 7, masked '&4' */
        TS_ASSERT_EQUALS(list[6].techStatus, game::LockedTech);
        TS_ASSERT_EQUALS(list[6].name, "Arkon Bomb");
    }
}

/** Test update notification.
    A: create session and populate with planet and ship list. Register a listener. Perform a game-side modification.
    E: verify correct update returned */
void
TestGameProxyBaseStorageProxy::testUpdate()
{
    game::test::SessionThread t;
    prepare(t);
    game::test::WaitIndicator ind;
    game::proxy::BaseStorageProxy testee(t.gameSender(), ind, PLANET_ID);

    // Wait for possible initial notifications
    t.sync();
    ind.processQueue();

    // Set up a listener
    UpdateReceiver recv(game::TorpedoTech);
    testee.sig_update.add(&recv, &UpdateReceiver::onUpdate);

    // Modify
    class Task : public util::Request<game::Session> {
     public:
        virtual void handle(game::Session& s)
            {
                s.getShipList()->launchers().get(7)->setName("Seven");
                s.notifyListeners();
            }
    };
    t.gameSender().postNewRequest(new Task());
    t.sync();
    ind.processQueue();

    // Verify
    TS_ASSERT_EQUALS(recv.getResult().size(), 10U);
    TS_ASSERT_EQUALS(recv.getResult()[6].name, "Seven");
}

/** Test custom StarbaseAdaptor.
    A: create session. Create custom adaptor with custom planet.
    E: getParts() accesses expected values */
void
TestGameProxyBaseStorageProxy::testCustom()
{
    // Adaptor implementation for testing
    class Adaptor : public game::proxy::StarbaseAdaptor {
     public:
        Adaptor(game::Session& session)
            : m_session(session), m_planet(111)
            {
                // Prepare planet with bare minimum
                // - planet
                game::map::PlanetData pd;
                pd.owner = PLAYER_NR;
                m_planet.addCurrentPlanetData(pd, game::PlayerSet_t(PLAYER_NR));

                // - base
                game::map::BaseData bd;
                bd.owner = PLAYER_NR;
                bd.hullStorage.set(3, 333);
                m_planet.addCurrentBaseData(bd, game::PlayerSet_t(PLAYER_NR));

                // - internal metadata
                game::map::Configuration config;
                m_planet.internalCheck(config, session.translator(), session.log());
                m_planet.setPlayability(game::map::Object::Playable);
            }
        virtual game::map::Planet& planet()
            { return m_planet; }
        virtual game::Session& session()
            { return m_session; }
        virtual bool findShipCloningHere(game::Id_t& /*id*/, String_t& /*name*/)
            { return false; }
        virtual void cancelAllCloneOrders()
            { }
        virtual void notifyListeners()
            { }
     private:
        game::Session& m_session;
        game::map::Planet m_planet;
    };

    // Adaptor-adaptor
    class Maker : public afl::base::Closure<game::proxy::StarbaseAdaptor*(game::Session&)> {
     public:
        virtual Adaptor* call(game::Session& session)
            { return new Adaptor(session); }
    };

    // Setup
    game::test::SessionThread t;
    prepare(t);
    game::test::WaitIndicator ind;
    game::proxy::BaseStorageProxy testee(t.gameSender().makeTemporary(new Maker()), ind);

    // Query hulls. Must return prepared value.
    game::proxy::BaseStorageProxy::Parts_t list;
    testee.getParts(ind, game::HullTech, list);
    TS_ASSERT_EQUALS(list.size(), 1U);
    TS_ASSERT_EQUALS(list[0].id, game::test::OUTRIDER_HULL_ID);
    TS_ASSERT_EQUALS(list[0].numParts, 333);
    TS_ASSERT_EQUALS(list[0].name, "OUTRIDER CLASS SCOUT");
}

