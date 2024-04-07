/**
  *  \file test/game/interface/loadcontexttest.cpp
  *  \brief Test for game::interface::LoadContext
  */

#include "game/interface/loadcontext.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/minefield.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "game/turn.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

using interpreter::TagNode;
using interpreter::test::ContextVerifier;
using interpreter::test::verifyNewNull;

/** Test loadContext() with a fully-populated session. */
AFL_TEST("game.interface.LoadContext:loadContext:full", a)
{
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session session(tx, fs);
    afl::io::ConstMemoryStream ms(afl::base::Nothing);

    // Root
    afl::base::Ptr<game::Root> root = game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0))).asPtr();
    game::Player& p = *root->playerList().create(4);
    p.setName(game::Player::ShortName, "Fourier");
    session.setRoot(root);

    // ShipList
    afl::base::Ptr<game::spec::ShipList> shipList = new game::spec::ShipList();
    game::test::addOutrider(*shipList);
    game::test::addTranswarp(*shipList);
    game::test::initStandardBeams(*shipList);
    game::test::initStandardTorpedoes(*shipList);
    session.setShipList(shipList);

    // Game
    afl::base::Ptr<game::Game> g = new game::Game();
    session.setGame(g);

    // - ship
    game::map::Ship& sh = *g->currentTurn().universe().ships().create(33);
    sh.addShipXYData(game::map::Point(1000, 1000), 5, 100, game::PlayerSet_t(1));
    sh.setName("USS Tester");
    sh.internalCheck(game::PlayerSet_t(1), 10);

    // - planet
    game::map::Planet& pl = *g->currentTurn().universe().planets().create(44);
    pl.setPosition(game::map::Point(2000, 2000));
    pl.setName("Pluto");
    pl.internalCheck(g->mapConfiguration(), game::PlayerSet_t(1), 10, tx, session.log());

    // - minefield
    game::map::Minefield& mf = *g->currentTurn().universe().minefields().create(22);
    mf.addReport(game::map::Point(1111, 1111), 7, game::map::Minefield::IsMine, game::map::Minefield::RadiusKnown, 30, 10, game::map::Minefield::MinefieldScanned);
    mf.internalCheck(10, root->hostVersion(), root->hostConfiguration());

    // - storm
    game::map::IonStorm& st = *g->currentTurn().universe().ionStorms().create(11);
    st.setName("Xaver");
    st.setPosition(game::map::Point(1200, 1200));
    st.setVoltage(50);
    st.setRadius(42);

    // Tests
    game::interface::LoadContext testee(session);

    {
        TagNode t = { TagNode::Tag_Ship, 33 };
        std::auto_ptr<interpreter::Context> ctx(testee.loadContext(t, ms));
        a.checkNonNull("Tag_Ship", ctx.get());
        ContextVerifier(*ctx, a("Tag_Ship")).verifyString("NAME", "USS Tester");
    }
    {
        TagNode t = { TagNode::Tag_Planet, 44 };
        std::auto_ptr<interpreter::Context> ctx(testee.loadContext(t, ms));
        a.checkNonNull("Tag_Planet", ctx.get());
        ContextVerifier(*ctx, a("Tag_Planet")).verifyString("NAME", "Pluto");
    }
    {
        TagNode t = { TagNode::Tag_Minefield, 22 };
        std::auto_ptr<interpreter::Context> ctx(testee.loadContext(t, ms));
        a.checkNonNull("Tag_Minefield", ctx.get());
        ContextVerifier(*ctx, a("Tag_Minefield")).verifyInteger("RADIUS", 30);
    }
    {
        TagNode t = { TagNode::Tag_Ion, 11 };
        std::auto_ptr<interpreter::Context> ctx(testee.loadContext(t, ms));
        a.checkNonNull("Tag_Ion", ctx.get());
        ContextVerifier(*ctx, a("Tag_Ion")).verifyString("NAME", "Xaver");
    }
    {
        TagNode t = { TagNode::Tag_Hull, game::test::OUTRIDER_HULL_ID };
        std::auto_ptr<interpreter::Context> ctx(testee.loadContext(t, ms));
        a.checkNonNull("Tag_Hull", ctx.get());
        ContextVerifier(*ctx, a("Tag_Hull")).verifyString("NAME", "OUTRIDER CLASS SCOUT");
    }
    {
        TagNode t = { TagNode::Tag_Engine, game::test::TRANSWARP_ENGINE_ID };
        std::auto_ptr<interpreter::Context> ctx(testee.loadContext(t, ms));
        a.checkNonNull("Tag_Engine", ctx.get());
        ContextVerifier(*ctx, a("Tag_Engine")).verifyString("NAME", "Transwarp Drive");
    }
    {
        TagNode t = { TagNode::Tag_Beam, 5 };
        std::auto_ptr<interpreter::Context> ctx(testee.loadContext(t, ms));
        a.checkNonNull("Tag_Beam", ctx.get());
        ContextVerifier(*ctx, a("Tag_Beam")).verifyString("NAME", "Positron Beam");
    }
    {
        TagNode t = { TagNode::Tag_Torpedo, 6 };
        std::auto_ptr<interpreter::Context> ctx(testee.loadContext(t, ms));
        a.checkNonNull("Tag_Torpedo", ctx.get());
        ContextVerifier(*ctx, a("Tag_Torpedo")).verifyString("NAME", "Mark 4 Photon");
        ContextVerifier(*ctx, a("Tag_Torpedo")).verifyInteger("COST.MC", 13);
    }
    {
        TagNode t = { TagNode::Tag_Launcher, 7 };
        std::auto_ptr<interpreter::Context> ctx(testee.loadContext(t, ms));
        a.checkNonNull("Tag_Launcher", ctx.get());
        ContextVerifier(*ctx, a("Tag_Launcher")).verifyString("NAME", "Mark 5 Photon");
        ContextVerifier(*ctx, a("Tag_Launcher")).verifyInteger("COST.MC", 57);
    }
    {
        TagNode t = { TagNode::Tag_Global, 0 };
        std::auto_ptr<interpreter::Context> ctx(testee.loadContext(t, ms));
        a.checkNonNull("Tag_Global", ctx.get());
        ContextVerifier(*ctx, a("Tag_Global")).verifyString("SYSTEM.PROGRAM", "PCC");
    }
    {
        TagNode t = { TagNode::Tag_Iterator, 22 };
        std::auto_ptr<interpreter::Context> ctx(testee.loadContext(t, ms));
        a.checkNonNull("Tag_Iterator", ctx.get());
        ContextVerifier(*ctx, a("Tag_Iterator")).verifyInteger("SCREEN", 22);
        ContextVerifier(*ctx, a("Tag_Iterator")).verifyInteger("COUNT", 1);
    }
    {
        TagNode t = { TagNode::Tag_Player, 4 };
        std::auto_ptr<interpreter::Context> ctx(testee.loadContext(t, ms));
        a.checkNonNull("Tag_Player", ctx.get());
        ContextVerifier(*ctx, a("Tag_Player")).verifyString("RACE.SHORT", "Fourier");
    }
    {
        TagNode t = { 0xFE98, 4 };
        verifyNewNull(a("invalid"), testee.loadContext(t, ms));
    }
}

/** Test loadContext() with an empty session. */
AFL_TEST("game.interface.LoadContext:loadContext:empty", a)
{
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session session(tx, fs);
    afl::io::ConstMemoryStream ms(afl::base::Nothing);

    game::interface::LoadContext testee(session);

    {
        TagNode t = { TagNode::Tag_Ship, 33 };
        verifyNewNull(a("Tag_Ship"), testee.loadContext(t, ms));
    }
    {
        TagNode t = { TagNode::Tag_Planet, 44 };
        verifyNewNull(a("Tag_Planet"), testee.loadContext(t, ms));
    }
    {
        TagNode t = { TagNode::Tag_Minefield, 22 };
        verifyNewNull(a("Tag_Minefield"), testee.loadContext(t, ms));
    }
    {
        TagNode t = { TagNode::Tag_Ion, 11 };
        verifyNewNull(a("Tag_Ion"), testee.loadContext(t, ms));
    }
    {
        TagNode t = { TagNode::Tag_Hull, game::test::OUTRIDER_HULL_ID };
        verifyNewNull(a("Tag_Hull"), testee.loadContext(t, ms));
    }
    {
        TagNode t = { TagNode::Tag_Engine, game::test::TRANSWARP_ENGINE_ID };
        verifyNewNull(a("Tag_Engine"), testee.loadContext(t, ms));
    }
    {
        TagNode t = { TagNode::Tag_Beam, 5 };
        verifyNewNull(a("Tag_Beam"), testee.loadContext(t, ms));
    }
    {
        TagNode t = { TagNode::Tag_Torpedo, 6 };
        verifyNewNull(a("Tag_Torpedo"), testee.loadContext(t, ms));
    }
    {
        TagNode t = { TagNode::Tag_Launcher, 7 };
        verifyNewNull(a("Tag_Launcher"), testee.loadContext(t, ms));
    }
    {
        // GlobalContext can always be created
        TagNode t = { TagNode::Tag_Global, 0 };
        std::auto_ptr<interpreter::Context> ctx(testee.loadContext(t, ms));
        a.checkNonNull("Tag_Global", ctx.get());
        ContextVerifier(*ctx, a("Tag_Global")).verifyString("SYSTEM.PROGRAM", "PCC");
    }
    {
        TagNode t = { TagNode::Tag_Iterator, 1 };
        verifyNewNull(a("Tag_Iterator"), testee.loadContext(t, ms));
    }
    {
        TagNode t = { TagNode::Tag_Player, 4 };
        verifyNewNull(a("Tag_Player"), testee.loadContext(t, ms));
    }
    {
        TagNode t = { 0xFE98, 4 };
        verifyNewNull(a("invalid"), testee.loadContext(t, ms));
    }
}

/** Test other functions.
    Basically, just for coverage. */
AFL_TEST("game.interface.LoadContext:others", a)
{
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session session(tx, fs);

    game::interface::LoadContext testee(session);
    verifyNewNull(a("loadBCO"),            testee.loadBCO(33));
    verifyNewNull(a("loadArray"),          testee.loadArray(44));
    verifyNewNull(a("loadHash"),           testee.loadHash(55));
    verifyNewNull(a("loadStructureValue"), testee.loadStructureValue(66));
    verifyNewNull(a("loadStructureType"),  testee.loadStructureType(77));

    std::auto_ptr<interpreter::Process> p(testee.createProcess());
    a.checkNull("createProcess", p.get());

    interpreter::Process proc(session.world(), "tester", 777);
    AFL_CHECK_SUCCEEDS(a("finishProcess"), testee.finishProcess(proc));
}
