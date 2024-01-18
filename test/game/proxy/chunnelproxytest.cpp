/**
  *  \file test/game/proxy/chunnelproxytest.cpp
  *  \brief Test for game::proxy::ChunnelProxy
  */

#include "game/proxy/chunnelproxy.hpp"

#include "afl/test/testrunner.hpp"
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
AFL_TEST("game.proxy.ChunnelProxy:postCandidateRequest", a)
{
    // Environment
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

    a.check("01. hasSpecialFunction", init.hasSpecialFunction(game::spec::BasicHullFunction::FirecloudChunnel,
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
        a.check("11. wait", disp.wait(100));
    }

    // Verify
    a.checkEqual("21. minDistance", recv.list.minDistance, 100);
    a.checkEqual("22. size",        recv.list.candidates.size(), 3U);
    a.checkEqual("23. pos",         recv.list.candidates[0].pos, game::map::Point(1200, 1000));
    a.checkEqual("24. hasOwn",      recv.list.candidates[0].hasOwn, true);
    a.checkEqual("25. hasAllied",   recv.list.candidates[0].hasAllied, false);
    a.checkEqual("26. pos",         recv.list.candidates[1].pos, game::map::Point(1300, 1000));
    a.checkEqual("27. hasOwn",      recv.list.candidates[1].hasOwn, true);
    a.checkEqual("28. hasAllied",   recv.list.candidates[1].hasAllied, false);
    a.checkEqual("29. pos",         recv.list.candidates[2].pos, game::map::Point(1700, 1000));
    a.checkEqual("30. hasOwn",      recv.list.candidates[2].hasOwn, true);
    a.checkEqual("31. hasAllied",   recv.list.candidates[2].hasAllied, true);
}

/** Test getCandidates.
    A: set up a universe. Call getCandidates.
    E: must return correct candidate list. */
AFL_TEST("game.proxy.ChunnelProxy:getCandidates", a)
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
    a.checkEqual("01. size",      list.size(), 2U);
    a.checkEqual("02. type",      list.get(0)->type,      UserList::ReferenceItem);
    a.checkEqual("03. reference", list.get(0)->reference, game::Reference(game::Reference::Ship, 11));
    a.checkEqual("04. marked",    list.get(0)->marked,    false);
    a.checkEqual("05. type",      list.get(1)->type,      UserList::ReferenceItem);
    a.checkEqual("06. reference", list.get(1)->reference, game::Reference(game::Reference::Ship, 12));
    a.checkEqual("07. marked",    list.get(1)->marked,    true);

    a.check("11. name", list.get(0)->name.find("#11") != String_t::npos);
    a.check("12. name", list.get(1)->name.find("#12") != String_t::npos);
}

/** Test setupChunnel.
    A: set up a universe. Call setupChunnel.
    E: must return correct result, must update universe. */
AFL_TEST("game.proxy.ChunnelProxy:setupChunnel", a)
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
    a.check("01. no errors", result.empty());

    // Verify universe
    a.checkEqual("11. getFriendlyCode", s5.getFriendlyCode().orElse(""), "011");
}

/** Test setupChunnel, error case.
    A: set up a universe where mate has too little fuel. Call setupChunnel.
    E: must return error message, but update universe. */
AFL_TEST("game.proxy.ChunnelProxy:setupChunnel:error", a)
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
    a.checkEqual("01. size", result.size(), 1U);
    a.check("02. result contains error", result[0].find("fuel") != String_t::npos);

    // Verify universe
    a.checkEqual("11. getFriendlyCode", s5.getFriendlyCode().orElse(""), "012");
}
