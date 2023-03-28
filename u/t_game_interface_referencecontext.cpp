/**
  *  \file u/t_game_interface_referencecontext.cpp
  *  \brief Test for game::interface::ReferenceContext
  */

#include "game/interface/referencecontext.hpp"

#include "t_game_interface.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/interface/globalcontext.hpp"
#include "game/map/ionstorm.hpp"
#include "game/map/minefield.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "game/turn.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

using game::Reference;
using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewNull;
using interpreter::test::verifyNewString;

namespace {
    /*
     *  A simplification for the test "this afl::data::Value actually needs to be a Context,
     *  and I want to verify its properties".
     */

    interpreter::Context& mustBeContext(afl::test::Assert a, afl::data::Value* v)
    {
        interpreter::Context* ctx = dynamic_cast<interpreter::Context*>(v);
        a.check("ctx != 0", ctx != 0);
        return *ctx;
    }

    struct NewContextVerifier {
        NewContextVerifier(afl::test::Assert a, afl::data::Value* v)
            : value(v),
              verif(mustBeContext(a, v), a)
            { }

        std::auto_ptr<afl::data::Value> value;
        interpreter::test::ContextVerifier verif;
    };
}

/** Test getReferenceProperty(). */
void
TestGameInterfaceReferenceContext::testGetReferenceProperty()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    afl::base::Ptr<game::Game> g = new game::Game();
    game::map::Ship& sh = *g->currentTurn().universe().ships().create(22);
    sh.addShipXYData(game::map::Point(1000, 1200), 7, 100, game::PlayerSet_t(1));
    sh.setName("Twenty-Two");
    sh.internalCheck(game::PlayerSet_t(1), 10);
    session.setGame(g);

    afl::base::Ptr<game::spec::ShipList> sl = new game::spec::ShipList();
    game::test::initStandardBeams(*sl);
    session.setShipList(sl);

    afl::base::Ptr<game::Root> r = game::test::makeRoot(game::HostVersion()).asPtr();
    session.setRoot(r);

    // Reference to a ship
    {
        Reference ref(Reference::Ship, 22);
        verifyNewNull   ("ship irpLocX",          getReferenceProperty(ref, game::interface::irpLocX,          session));
        verifyNewNull   ("ship irpLocY",          getReferenceProperty(ref, game::interface::irpLocY,          session));
        verifyNewInteger("ship irpId",            getReferenceProperty(ref, game::interface::irpId,            session), 22);
        verifyNewString ("ship irpReferenceName", getReferenceProperty(ref, game::interface::irpReferenceName, session), "Ship #22");
        verifyNewString ("ship irpPlainName",     getReferenceProperty(ref, game::interface::irpPlainName,     session), "Twenty-Two");
        verifyNewString ("ship irpDetailedName",  getReferenceProperty(ref, game::interface::irpDetailedName,  session), "Ship #22: Twenty-Two");
        verifyNewString ("ship irpKind",          getReferenceProperty(ref, game::interface::irpKind,          session), "ship");

        NewContextVerifier v("ship object", getReferenceProperty(ref, game::interface::irpObject, session));
        v.verif.verifyString("NAME", "Twenty-Two");
        v.verif.verifyInteger("LOC.X", 1000);
    }

    // Reference to a beam
    {
        Reference ref(Reference::Beam, 5);
        verifyNewNull   ("beam irpLocX",          getReferenceProperty(ref, game::interface::irpLocX,          session));
        verifyNewNull   ("beam irpLocY",          getReferenceProperty(ref, game::interface::irpLocY,          session));
        verifyNewInteger("beam irpId",            getReferenceProperty(ref, game::interface::irpId,            session), 5);
        verifyNewString ("beam irpReferenceName", getReferenceProperty(ref, game::interface::irpReferenceName, session), "Beam Weapon #5");
        verifyNewString ("beam irpPlainName",     getReferenceProperty(ref, game::interface::irpPlainName,     session), "Positron Beam");
        verifyNewString ("beam irpDetailedName",  getReferenceProperty(ref, game::interface::irpDetailedName,  session), "Beam Weapon #5: Positron Beam");
        verifyNewString ("beam irpKind",          getReferenceProperty(ref, game::interface::irpKind,          session), "beam");

        NewContextVerifier v("beam object", getReferenceProperty(ref, game::interface::irpObject, session));
        v.verif.verifyString("NAME", "Positron Beam");
        v.verif.verifyInteger("DAMAGE", 29);
    }

    // Reference to a location
    {
        Reference ref(game::map::Point(2500, 1300));
        verifyNewInteger("loc irpLocX",          getReferenceProperty(ref, game::interface::irpLocX,          session), 2500);
        verifyNewInteger("loc irpLocY",          getReferenceProperty(ref, game::interface::irpLocY,          session), 1300);
        verifyNewInteger("loc irpId",            getReferenceProperty(ref, game::interface::irpId,            session), 2500 /*!*/);
        verifyNewString ("loc irpReferenceName", getReferenceProperty(ref, game::interface::irpReferenceName, session), "(2500,1300)");
        verifyNewString ("loc irpPlainName",     getReferenceProperty(ref, game::interface::irpPlainName,     session), "(2500,1300)");
        verifyNewString ("loc irpDetailedName",  getReferenceProperty(ref, game::interface::irpDetailedName,  session), "(2500,1300)");
        verifyNewString ("loc irpKind",          getReferenceProperty(ref, game::interface::irpKind,          session), "location");
        verifyNewNull   ("loc irpObject",        getReferenceProperty(ref, game::interface::irpObject,        session));
    }

    // Null reference to a location
    {
        Reference ref;
        verifyNewNull   ("null irpLocX",          getReferenceProperty(ref, game::interface::irpLocX,          session));
        verifyNewNull   ("null irpLocY",          getReferenceProperty(ref, game::interface::irpLocY,          session));
        verifyNewInteger("null irpId",            getReferenceProperty(ref, game::interface::irpId,            session), 0  /*!*/);
        verifyNewString ("null irpReferenceName", getReferenceProperty(ref, game::interface::irpReferenceName, session), "" /*!*/);
        verifyNewNull   ("null irpPlainName",     getReferenceProperty(ref, game::interface::irpPlainName,     session));
        verifyNewNull   ("null irpDetailedName",  getReferenceProperty(ref, game::interface::irpDetailedName,  session));
        verifyNewNull   ("null irpKind",          getReferenceProperty(ref, game::interface::irpKind,          session));
        verifyNewNull   ("null irpObject",        getReferenceProperty(ref, game::interface::irpObject,        session));
    }
}

/** Test makeObjectValue(). */
void
TestGameInterfaceReferenceContext::testMakeObjectValue()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Root
    afl::base::Ptr<game::Root> r = game::test::makeRoot(game::HostVersion()).asPtr();
    session.setRoot(r);

    // - Player
    game::Player& p = *r->playerList().create(7);
    p.setName(game::Player::LongName, "The Tholian Empire");
    p.setName(game::Player::ShortName, "The Tholians");
    p.setName(game::Player::AdjectiveName, "Tholian");

    // Game
    afl::base::Ptr<game::Game> g = new game::Game();
    session.setGame(g);

    // - ship
    game::map::Ship& sh = *g->currentTurn().universe().ships().create(22);
    sh.addShipXYData(game::map::Point(1000, 1200), 7, 100, game::PlayerSet_t(1));
    sh.setName("Twenty-Two");
    sh.internalCheck(game::PlayerSet_t(1), 10);

    // - planet
    game::map::Planet& pl = *g->currentTurn().universe().planets().create(363);
    pl.setPosition(game::map::Point(2000, 1300));
    pl.setName("Rambo 3");
    pl.internalCheck(g->mapConfiguration(), game::PlayerSet_t(1), 10, tx, session.log());

    // - ion storm
    game::map::IonStorm& st = *g->currentTurn().universe().ionStorms().create(7);
    st.setName("Katrina");
    st.setPosition(game::map::Point(1500, 1200));
    st.setRadius(50);
    st.setVoltage(20);

    // - minefield
    game::map::Minefield& mf = *g->currentTurn().universe().minefields().create(150);
    mf.addReport(game::map::Point(2400, 2300), 7, game::map::Minefield::IsMine, game::map::Minefield::UnitsKnown, 400, 10, game::map::Minefield::MinefieldScanned);
    mf.internalCheck(10, r->hostVersion(), r->hostConfiguration());

    // - ufo
    game::map::Ufo& ufo = *g->currentTurn().universe().ufos().addUfo(51, 42, 1);
    ufo.setPosition(game::map::Point(2300, 1100));
    ufo.setRadius(20);
    ufo.setName("Invader");
    ufo.postprocess(10, g->mapConfiguration());

    // Ship List
    afl::base::Ptr<game::spec::ShipList> sl = new game::spec::ShipList();
    session.setShipList(sl);
    game::test::initStandardBeams(*sl);
    game::test::initStandardTorpedoes(*sl);
    game::test::addTranswarp(*sl);
    game::test::addAnnihilation(*sl);

    // --- Test ---

    // Null
    verifyNewNull("ref null", game::interface::makeObjectValue(Reference(), session));

    // Special
    verifyNewNull("ref special", game::interface::makeObjectValue(Reference(Reference::Special, 77), session));

    // Player
    {
        NewContextVerifier v("ref player", game::interface::makeObjectValue(Reference(Reference::Player, 7), session));
        v.verif.verifyString("RACE.ADJ", "Tholian");
    }

    // Map Location
    verifyNewNull("ref map", game::interface::makeObjectValue(Reference(game::map::Point(1000, 2000)), session));

    // Ship
    {
        NewContextVerifier v("ref ship", game::interface::makeObjectValue(Reference(Reference::Ship, 22), session));
        v.verif.verifyString("NAME", "Twenty-Two");
        v.verif.verifyInteger("LOC.X", 1000);
    }

    // Planet
    {
        NewContextVerifier v("ref planet", game::interface::makeObjectValue(Reference(Reference::Planet, 363), session));
        v.verif.verifyString("NAME", "Rambo 3");
        v.verif.verifyInteger("LOC.X", 2000);
    }

    // Starbase
    {
        NewContextVerifier v("ref base", game::interface::makeObjectValue(Reference(Reference::Starbase, 363), session));
        v.verif.verifyString("NAME", "Rambo 3");
        v.verif.verifyInteger("LOC.X", 2000);
    }

    // Ion Storm
    {
        NewContextVerifier v("ref storm", game::interface::makeObjectValue(Reference(Reference::IonStorm, 7), session));
        v.verif.verifyString("NAME", "Katrina");
        v.verif.verifyInteger("LOC.X", 1500);
    }

    // Minefield
    {
        NewContextVerifier v("ref mine", game::interface::makeObjectValue(Reference(Reference::Minefield, 150), session));
        v.verif.verifyInteger("UNITS", 400);
        v.verif.verifyInteger("LOC.X", 2400);
    }

    // Ufo
    {
        NewContextVerifier v("ref mine", game::interface::makeObjectValue(Reference(Reference::Ufo, 51), session));
        v.verif.verifyString("NAME", "Invader");
        v.verif.verifyInteger("LOC.X", 2300);
    }

    // Hull
    {
        NewContextVerifier v("ref hull", game::interface::makeObjectValue(Reference(Reference::Hull, game::test::ANNIHILATION_HULL_ID), session));
        v.verif.verifyString("NAME", "ANNIHILATION CLASS BATTLESHIP");
        v.verif.verifyInteger("MASS", 960);
    }

    // Engine
    {
        NewContextVerifier v("ref engine", game::interface::makeObjectValue(Reference(Reference::Engine, 9), session));
        v.verif.verifyString("NAME", "Transwarp Drive");
        v.verif.verifyInteger("COST.MC", 300);
    }

    // Beam
    {
        NewContextVerifier v("ref beam", game::interface::makeObjectValue(Reference(Reference::Beam, 5), session));
        v.verif.verifyString("NAME", "Positron Beam");
        v.verif.verifyInteger("COST.MC", 12);
    }

    // Torpedo
    {
        NewContextVerifier v("ref torp", game::interface::makeObjectValue(Reference(Reference::Torpedo, 7), session));
        v.verif.verifyString("NAME", "Mark 5 Photon");
        v.verif.verifyInteger("COST.MC", 57);
    }

    // Null
    verifyNewNull("ref player null", game::interface::makeObjectValue(Reference(Reference::Player,    8), session));
    verifyNewNull("ref ship null",   game::interface::makeObjectValue(Reference(Reference::Ship,      8), session));
    verifyNewNull("ref planet null", game::interface::makeObjectValue(Reference(Reference::Planet,    8), session));
    verifyNewNull("ref base null",   game::interface::makeObjectValue(Reference(Reference::Starbase,  8), session));
    verifyNewNull("ref storm null",  game::interface::makeObjectValue(Reference(Reference::IonStorm,  8), session));
    verifyNewNull("ref mine null",   game::interface::makeObjectValue(Reference(Reference::Minefield, 8), session));
    verifyNewNull("ref ufo null",    game::interface::makeObjectValue(Reference(Reference::Ufo,       8), session));
    verifyNewNull("ref hull null",   game::interface::makeObjectValue(Reference(Reference::Hull,      8), session));
    verifyNewNull("ref engine null", game::interface::makeObjectValue(Reference(Reference::Engine,    8), session));
    verifyNewNull("ref beam null",   game::interface::makeObjectValue(Reference(Reference::Beam,     11), session));
    verifyNewNull("ref torp null",   game::interface::makeObjectValue(Reference(Reference::Torpedo,  11), session));
}

/** Test getReferenceTypeName().
    For the regular types, verify the backward mapping as well. */
void
TestGameInterfaceReferenceContext::testGetReferenceTypeName()
{
    // Special cases
    TS_ASSERT(game::interface::getReferenceTypeName(Reference::Null) == 0);
    TS_ASSERT(game::interface::getReferenceTypeName(Reference::MapLocation) != 0);

    // Regular cases. Those must all map back and forth
    static const Reference::Type types[] = {
        Reference::Special,   Reference::Player,   Reference::Ship,
        Reference::Planet,    Reference::Starbase, Reference::IonStorm,
        Reference::Minefield, Reference::Ufo,      Reference::Hull,
        Reference::Engine,    Reference::Beam,     Reference::Torpedo,
    };
    for (size_t i = 0; i < sizeof(types)/sizeof(types[0]); ++i) {
        const char* ch = game::interface::getReferenceTypeName(types[i]);
        TS_ASSERT(ch != 0);
        TS_ASSERT_DIFFERS(String_t(ch), "");

        Reference::Type reverse;
        TSM_ASSERT(ch, game::interface::parseReferenceTypeName(ch, reverse));
        TSM_ASSERT_LESS_THAN_EQUALS(ch, 2U, std::strlen(ch));
        TSM_ASSERT_EQUALS(ch, reverse, types[i]);
    }
}

/** Test parseReferenceTypeName().
    Long names are already tested above. Test the documented short names. */
void
TestGameInterfaceReferenceContext::testParseReferenceTypeName()
{
    Reference::Type t;
    TS_ASSERT(game::interface::parseReferenceTypeName("b", t));
    TS_ASSERT_EQUALS(t, Reference::Starbase);

    TS_ASSERT(game::interface::parseReferenceTypeName("e", t));
    TS_ASSERT_EQUALS(t, Reference::Engine);

    TS_ASSERT(game::interface::parseReferenceTypeName("h", t));
    TS_ASSERT_EQUALS(t, Reference::Hull);

    TS_ASSERT(game::interface::parseReferenceTypeName("i", t));
    TS_ASSERT_EQUALS(t, Reference::IonStorm);

    TS_ASSERT(game::interface::parseReferenceTypeName("m", t));
    TS_ASSERT_EQUALS(t, Reference::Minefield);

    TS_ASSERT(game::interface::parseReferenceTypeName("p", t));
    TS_ASSERT_EQUALS(t, Reference::Planet);

    TS_ASSERT(game::interface::parseReferenceTypeName("s", t));
    TS_ASSERT_EQUALS(t, Reference::Ship);

    TS_ASSERT(game::interface::parseReferenceTypeName("t", t));
    TS_ASSERT_EQUALS(t, Reference::Torpedo);

    TS_ASSERT(game::interface::parseReferenceTypeName("w", t));
    TS_ASSERT_EQUALS(t, Reference::Beam);

    TS_ASSERT(game::interface::parseReferenceTypeName("y", t));
    TS_ASSERT_EQUALS(t, Reference::Player);
}

/** Test ReferenceContext class. */
void
TestGameInterfaceReferenceContext::testReferenceContext()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Root
    afl::base::Ptr<game::Root> r = game::test::makeRoot(game::HostVersion()).asPtr();
    session.setRoot(r);

    // Game
    afl::base::Ptr<game::Game> g = new game::Game();
    session.setGame(g);

    // - ion storm
    game::map::IonStorm& st = *g->currentTurn().universe().ionStorms().create(7);
    st.setName("Katrina");
    st.setPosition(game::map::Point(1500, 1200));
    st.setRadius(50);
    st.setVoltage(20);

    // Ship List
    afl::base::Ptr<game::spec::ShipList> sl = new game::spec::ShipList();
    session.setShipList(sl);

    // Test object
    game::interface::ReferenceContext testee(Reference(Reference::IonStorm, 7), session);
    interpreter::test::ContextVerifier verif(testee, "testReferenceContext");
    verif.verifyBasics();
    verif.verifyNotSerializable();
    verif.verifyTypes();
    verif.verifyString("KIND", "storm");
    verif.verifyInteger("ID", 7);
    TS_ASSERT(testee.getObject() == 0);
    TS_ASSERT_EQUALS(testee.getReference(), Reference(Reference::IonStorm, 7));
    TS_ASSERT_THROWS(verif.setIntegerValue("ID", 99), interpreter::Error);
}

/** Test IFLocationReference(). */
void
TestGameInterfaceReferenceContext::testIFLocationReference()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Regular invocation
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1400);
        seg.pushBackInteger(1800);
        interpreter::Arguments args(seg, 0, 2);
        NewContextVerifier v("X,Y", game::interface::IFLocationReference(session, args));
        v.verif.verifyInteger("LOC.X", 1400);
        v.verif.verifyInteger("LOC.Y", 1800);
        v.verif.verifyString("KIND", "location");
    }

    // Null Y argument
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1400);
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewNull("X,EMPTY", game::interface::IFLocationReference(session, args));
    }

    // Null X argument
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackInteger(1400);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewNull("EMPTY,Y", game::interface::IFLocationReference(session, args));
    }

    // Range error
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1400);
        seg.pushBackInteger(-1);
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFLocationReference(session, args), interpreter::Error);
    }

    // Type error
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1400);
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFLocationReference(session, args), interpreter::Error);
    }

    // Arity error
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1400);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFLocationReference(session, args), interpreter::Error);
    }
}

/** Test IFReference(). */
void
TestGameInterfaceReferenceContext::testIFReference()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Regular invocation
    {
        afl::data::Segment seg;
        seg.pushBackString("base");
        seg.pushBackInteger(5);
        interpreter::Arguments args(seg, 0, 2);
        NewContextVerifier v("base,5", game::interface::IFReference(session, args));
        v.verif.verifyString("KIND", "base");
        v.verif.verifyInteger("ID", 5);
    }

    // Null Id argument
    {
        afl::data::Segment seg;
        seg.pushBackString("base");
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewNull("base,EMPTY", game::interface::IFReference(session, args));
    }

    // Null type argument
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackInteger(5);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewNull("EMPTY,5", game::interface::IFReference(session, args));
    }

    // Range error
    {
        afl::data::Segment seg;
        seg.pushBackString("base");
        seg.pushBackInteger(-1);
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFReference(session, args), interpreter::Error);
    }

    // Type error
    {
        afl::data::Segment seg;
        seg.pushBackString("base");
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFReference(session, args), interpreter::Error);
    }

    // Bad type string
    {
        afl::data::Segment seg;
        seg.pushBackString("grill");
        seg.pushBackInteger(5);
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFReference(session, args), interpreter::Error);
    }

    // Arity error
    {
        afl::data::Segment seg;
        seg.pushBackString("base");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFReference(session, args), interpreter::Error);
    }
}

/** Test checkReferenceArg(). */
void
TestGameInterfaceReferenceContext::testCheckReferenceArg()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Regular
    {
        game::interface::ReferenceContext ctx(Reference(Reference::Planet, 77), session);
        Reference out;
        TS_ASSERT_EQUALS(game::interface::checkReferenceArg(out, &ctx), true);
        TS_ASSERT_EQUALS(out, Reference(Reference::Planet, 77));
    }

    // Null
    {
        Reference out;
        TS_ASSERT_EQUALS(game::interface::checkReferenceArg(out, 0), false);
    }

    // Wrong type: integer
    {
        afl::data::IntegerValue iv(77);
        Reference out;
        TS_ASSERT_THROWS(game::interface::checkReferenceArg(out, &iv), interpreter::Error);
    }

    // Wrong type: other context
    {
        game::interface::GlobalContext ctx(session);
        Reference out;
        TS_ASSERT_THROWS(game::interface::checkReferenceArg(out, &ctx), interpreter::Error);
    }
}

