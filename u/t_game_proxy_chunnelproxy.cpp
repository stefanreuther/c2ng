/**
  *  \file u/t_game_proxy_chunnelproxy.cpp
  *  \brief Test for game::proxy::ChunnelProxy
  */

#include "game/proxy/chunnelproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/element.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"
#include "util/simplerequestdispatcher.hpp"

namespace {
    using afl::base::Ptr;
    using game::Element;
    using game::Game;
    using game::HostVersion;
    using game::Root;
    using game::Turn;
    using game::map::Ship;
    using game::proxy::ChunnelProxy;
    using game::ref::UserList;
    using game::spec::ShipList;
    using game::test::SessionThread;
    using game::test::WaitIndicator;

    const int FIRECLOUD_ID = 55;
    const int NORMAL_ID = 44;
    const int OWNER = 9;

    void addShipList(SessionThread& h)
    {
        Ptr<ShipList> shipList = new ShipList();

        // A normal hull
        game::spec::Hull* pNormal = shipList->hulls().create(NORMAL_ID);
        pNormal->setMass(1);
        pNormal->setMaxCargo(100);
        pNormal->setMaxFuel(100);

        // A chunnel-able ship
        game::spec::Hull* pFirecloud = shipList->hulls().create(FIRECLOUD_ID);
        pFirecloud->setMass(1);
        pFirecloud->setMaxCargo(100);
        pFirecloud->setMaxFuel(100);
        pFirecloud->changeHullFunction(shipList->modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::FirecloudChunnel),
                                       game::PlayerSet_t::allUpTo(game::MAX_PLAYERS),
                                       game::PlayerSet_t(),
                                       true);

        h.session().setShipList(shipList);
    }

    void addRoot(SessionThread& h)
    {
        Ptr<Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,2,0))).asPtr();
        root->hostConfiguration()[game::config::HostConfiguration::AllowBeamUpMultiple].set(1);
        h.session().setRoot(root);
    }

    void addGame(SessionThread& h)
    {
        Ptr<Game> g = new Game();
        h.session().setGame(g);
    }

    void prepare(SessionThread& h)
    {
        addRoot(h);
        addShipList(h);
        addGame(h);
    }

    Ship& addShip(SessionThread& h, int id, int x, int y, int hull)
    {
        Ptr<Game> g = h.session().getGame();
        TS_ASSERT(g.get() != 0);

        Ship& sh = *g->currentTurn().universe().ships().create(id);
        game::map::ShipData data;
        data.owner                     = OWNER;
        data.x                         = x;
        data.y                         = y;
        data.engineType                = 1;
        data.hullType                  = hull;
        data.beamType                  = 0;
        data.torpedoType               = 0;
        data.warpFactor                = 3;
        data.neutronium                = 60;

        sh.addCurrentShipData(data, game::PlayerSet_t(OWNER));
        sh.internalCheck(game::PlayerSet_t(OWNER), 15);
        sh.setPlayability(game::map::Object::Playable);

        return sh;
    }

    struct CandidateReceiver {
        ChunnelProxy::CandidateList list;

        void onCandidateListUpdate(const ChunnelProxy::CandidateList& a)
            { list = a; }
    };
}

/** Test postCandidateRequest.
    A: set up a universe. Call postCandidateRequest.
    E: a callback is generated returning a list of candidates. */
void
TestGameProxyChunnelProxy::testCandidates()
{
    // Environment
    CxxTest::setAbortTestOnFail(true);
    SessionThread h;
    prepare(h);
    h.session().getRoot()->hostConfiguration()[game::config::HostConfiguration::AllowAlliedChunneling].set(1);
    h.session().getGame()->teamSettings().setPlayerTeam(OWNER+1, OWNER);

    Ship& init = addShip(h, 5, 1000, 1000, FIRECLOUD_ID);   // initiator
    addShip(h,  10,  1050, 1000, FIRECLOUD_ID);   // too close
    addShip(h,  11,  1200, 1000, FIRECLOUD_ID);   // acceptable distance
    addShip(h,  12,  1200, 1000, FIRECLOUD_ID);   // -"-
    addShip(h,  13,  1300, 1000, FIRECLOUD_ID);   // -"-
    addShip(h,  14,  1700, 1000, FIRECLOUD_ID);   // -"-
    addShip(h,  15,  1700, 1000, FIRECLOUD_ID).setOwner(OWNER+1);

    TS_ASSERT(init.hasSpecialFunction(game::spec::BasicHullFunction::FirecloudChunnel,
                                      h.session().getGame()->shipScores(),
                                      *h.session().getShipList(),
                                      h.session().getRoot()->hostConfiguration()));

    // Testee
    util::SimpleRequestDispatcher disp;
    ChunnelProxy testee(h.gameSender(), disp);

    CandidateReceiver recv;
    testee.sig_candidateListUpdate.add(&recv, &CandidateReceiver::onCandidateListUpdate);

    // Request candidates
    testee.postCandidateRequest(5);
    while (recv.list.candidates.empty()) {
        TS_ASSERT(disp.wait(100));
    }

    // Verify
    TS_ASSERT_EQUALS(recv.list.minDistance, 100);
    TS_ASSERT_EQUALS(recv.list.candidates.size(), 3U);
    TS_ASSERT_EQUALS(recv.list.candidates[0].pos, game::map::Point(1200, 1000));
    TS_ASSERT_EQUALS(recv.list.candidates[0].hasOwn, true);
    TS_ASSERT_EQUALS(recv.list.candidates[0].hasAllied, false);
    TS_ASSERT_EQUALS(recv.list.candidates[1].pos, game::map::Point(1300, 1000));
    TS_ASSERT_EQUALS(recv.list.candidates[1].hasOwn, true);
    TS_ASSERT_EQUALS(recv.list.candidates[1].hasAllied, false);
    TS_ASSERT_EQUALS(recv.list.candidates[2].pos, game::map::Point(1700, 1000));
    TS_ASSERT_EQUALS(recv.list.candidates[2].hasOwn, true);
    TS_ASSERT_EQUALS(recv.list.candidates[2].hasAllied, true);
}

/** Test postCandidateRequest.
    A: set up a universe. Call getCandidates.
    E: must return correct candidate list. */
void
TestGameProxyChunnelProxy::testGetCandidates()
{
    // Environment
    SessionThread h;
    prepare(h);
    /*Ship& s5  =*/ addShip(h,   5,  1000, 1000, FIRECLOUD_ID);   // initiator
    /*Ship& s11 =*/ addShip(h,  11,  1200, 1000, FIRECLOUD_ID);   // acceptable distance
    Ship& s12   =   addShip(h,  12,  1200, 1000, FIRECLOUD_ID);   // -"-
    /*Ship& s13 =*/ addShip(h,  13,  1200, 1000, NORMAL_ID);      // wrong type
    s12.setIsMarked(true);

    // Testee
    WaitIndicator ind;
    ChunnelProxy testee(h.gameSender(), ind);

    // Get candidates
    UserList list;
    testee.getCandidates(ind, 5, game::map::Point(1200, 1000), list);

    // Verify
    TS_ASSERT_EQUALS(list.size(), 2U);
    TS_ASSERT_EQUALS(list.get(0)->type,      UserList::ReferenceItem);
    TS_ASSERT_EQUALS(list.get(0)->reference, game::Reference(game::Reference::Ship, 11));
    TS_ASSERT_EQUALS(list.get(0)->marked,    false);
    TS_ASSERT_EQUALS(list.get(1)->type,      UserList::ReferenceItem);
    TS_ASSERT_EQUALS(list.get(1)->reference, game::Reference(game::Reference::Ship, 12));
    TS_ASSERT_EQUALS(list.get(1)->marked,    true);

    TS_ASSERT(list.get(0)->name.find("#11") != String_t::npos);
    TS_ASSERT(list.get(1)->name.find("#12") != String_t::npos);
}

/** Test setupChunnel.
    A: set up a universe. Call setupChunnel.
    E: must return correct result, must update universe. */
void
TestGameProxyChunnelProxy::testSetupChunnel()
{
    // Environment
    SessionThread h;
    prepare(h);
    Ship& s5    =   addShip(h,   5,  1000, 1000, FIRECLOUD_ID);   // initiator
    /*Ship& s11 =*/ addShip(h,  11,  1200, 1000, FIRECLOUD_ID);   // acceptable distance
    Ship& s12   =   addShip(h,  12,  1200, 1000, FIRECLOUD_ID);   // -"-
    /*Ship& s13 =*/ addShip(h,  13,  1200, 1000, NORMAL_ID);      // wrong type
    s12.setIsMarked(true);

    // Testee
    WaitIndicator ind;
    ChunnelProxy testee(h.gameSender(), ind);

    // Setup chunnel
    afl::data::StringList_t result = testee.setupChunnel(ind, 5, 11);
    TS_ASSERT(result.empty());

    // Verify universe
    TS_ASSERT_EQUALS(s5.getFriendlyCode().orElse(""), "011");
}

/** Test setupChunnel, error case.
    A: set up a universe where mate has too little fuel. Call setupChunnel.
    E: must return error message, but update universe. */
void
TestGameProxyChunnelProxy::testSetupChunnelError()
{
    // Environment
    SessionThread h;
    prepare(h);
    Ship& s5  = addShip(h,   5,  1000, 1000, FIRECLOUD_ID);   // initiator
    Ship& s12 = addShip(h,  12,  1200, 1000, FIRECLOUD_ID);   // -"-
    s12.setCargo(Element::Neutronium, 0);

    // Testee
    WaitIndicator ind;
    ChunnelProxy testee(h.gameSender(), ind);

    // Setup chunnel
    afl::data::StringList_t result = testee.setupChunnel(ind, 5, 12);
    TS_ASSERT_EQUALS(result.size(), 1U);
    TS_ASSERT(result[0].find("fuel") != String_t::npos);

    // Verify universe
    TS_ASSERT_EQUALS(s5.getFriendlyCode().orElse(""), "012");
}

