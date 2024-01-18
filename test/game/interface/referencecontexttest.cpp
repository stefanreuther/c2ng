/**
  *  \file test/game/interface/referencecontexttest.cpp
  *  \brief Test for game::interface::ReferenceContext
  */

#include "game/interface/referencecontext.hpp"

#include "afl/data/integervalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
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
AFL_TEST("game.interface.ReferenceContext:getReferenceProperty", a)
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
        verifyNewNull   (a("ship irpLocX"),          getReferenceProperty(ref, game::interface::irpLocX,          session));
        verifyNewNull   (a("ship irpLocY"),          getReferenceProperty(ref, game::interface::irpLocY,          session));
        verifyNewInteger(a("ship irpId"),            getReferenceProperty(ref, game::interface::irpId,            session), 22);
        verifyNewString (a("ship irpReferenceName"), getReferenceProperty(ref, game::interface::irpReferenceName, session), "Ship #22");
        verifyNewString (a("ship irpPlainName"),     getReferenceProperty(ref, game::interface::irpPlainName,     session), "Twenty-Two");
        verifyNewString (a("ship irpDetailedName"),  getReferenceProperty(ref, game::interface::irpDetailedName,  session), "Ship #22: Twenty-Two");
        verifyNewString (a("ship irpKind"),          getReferenceProperty(ref, game::interface::irpKind,          session), "ship");

        NewContextVerifier v(a("ship object"), getReferenceProperty(ref, game::interface::irpObject, session));
        v.verif.verifyString("NAME", "Twenty-Two");
        v.verif.verifyInteger("LOC.X", 1000);
    }

    // Reference to a beam
    {
        Reference ref(Reference::Beam, 5);
        verifyNewNull   (a("beam irpLocX"),          getReferenceProperty(ref, game::interface::irpLocX,          session));
        verifyNewNull   (a("beam irpLocY"),          getReferenceProperty(ref, game::interface::irpLocY,          session));
        verifyNewInteger(a("beam irpId"),            getReferenceProperty(ref, game::interface::irpId,            session), 5);
        verifyNewString (a("beam irpReferenceName"), getReferenceProperty(ref, game::interface::irpReferenceName, session), "Beam Weapon #5");
        verifyNewString (a("beam irpPlainName"),     getReferenceProperty(ref, game::interface::irpPlainName,     session), "Positron Beam");
        verifyNewString (a("beam irpDetailedName"),  getReferenceProperty(ref, game::interface::irpDetailedName,  session), "Beam Weapon #5: Positron Beam");
        verifyNewString (a("beam irpKind"),          getReferenceProperty(ref, game::interface::irpKind,          session), "beam");

        NewContextVerifier v(a("beam object"), getReferenceProperty(ref, game::interface::irpObject, session));
        v.verif.verifyString("NAME", "Positron Beam");
        v.verif.verifyInteger("DAMAGE", 29);
    }

    // Reference to a location
    {
        Reference ref(game::map::Point(2500, 1300));
        verifyNewInteger(a("loc irpLocX"),          getReferenceProperty(ref, game::interface::irpLocX,          session), 2500);
        verifyNewInteger(a("loc irpLocY"),          getReferenceProperty(ref, game::interface::irpLocY,          session), 1300);
        verifyNewInteger(a("loc irpId"),            getReferenceProperty(ref, game::interface::irpId,            session), 2500 /*!*/);
        verifyNewString (a("loc irpReferenceName"), getReferenceProperty(ref, game::interface::irpReferenceName, session), "(2500,1300)");
        verifyNewString (a("loc irpPlainName"),     getReferenceProperty(ref, game::interface::irpPlainName,     session), "(2500,1300)");
        verifyNewString (a("loc irpDetailedName"),  getReferenceProperty(ref, game::interface::irpDetailedName,  session), "(2500,1300)");
        verifyNewString (a("loc irpKind"),          getReferenceProperty(ref, game::interface::irpKind,          session), "location");
        verifyNewNull   (a("loc irpObject"),        getReferenceProperty(ref, game::interface::irpObject,        session));
    }

    // Null reference to a location
    {
        Reference ref;
        verifyNewNull   (a("null irpLocX"),          getReferenceProperty(ref, game::interface::irpLocX,          session));
        verifyNewNull   (a("null irpLocY"),          getReferenceProperty(ref, game::interface::irpLocY,          session));
        verifyNewInteger(a("null irpId"),            getReferenceProperty(ref, game::interface::irpId,            session), 0  /*!*/);
        verifyNewString (a("null irpReferenceName"), getReferenceProperty(ref, game::interface::irpReferenceName, session), "" /*!*/);
        verifyNewNull   (a("null irpPlainName"),     getReferenceProperty(ref, game::interface::irpPlainName,     session));
        verifyNewNull   (a("null irpDetailedName"),  getReferenceProperty(ref, game::interface::irpDetailedName,  session));
        verifyNewNull   (a("null irpKind"),          getReferenceProperty(ref, game::interface::irpKind,          session));
        verifyNewNull   (a("null irpObject"),        getReferenceProperty(ref, game::interface::irpObject,        session));
    }
}

/** Test makeObjectValue(). */
AFL_TEST("game.interface.ReferenceContext:makeObjectValue", a)
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
    verifyNewNull(a("ref null"), game::interface::makeObjectValue(Reference(), session));

    // Special
    verifyNewNull(a("ref special"), game::interface::makeObjectValue(Reference(Reference::Special, 77), session));

    // Player
    {
        NewContextVerifier v(a("ref player"), game::interface::makeObjectValue(Reference(Reference::Player, 7), session));
        v.verif.verifyString("RACE.ADJ", "Tholian");
    }

    // Map Location
    verifyNewNull(a("ref map"), game::interface::makeObjectValue(Reference(game::map::Point(1000, 2000)), session));

    // Ship
    {
        NewContextVerifier v(a("ref ship"), game::interface::makeObjectValue(Reference(Reference::Ship, 22), session));
        v.verif.verifyString("NAME", "Twenty-Two");
        v.verif.verifyInteger("LOC.X", 1000);
    }

    // Planet
    {
        NewContextVerifier v(a("ref planet"), game::interface::makeObjectValue(Reference(Reference::Planet, 363), session));
        v.verif.verifyString("NAME", "Rambo 3");
        v.verif.verifyInteger("LOC.X", 2000);
    }

    // Starbase
    {
        NewContextVerifier v(a("ref base"), game::interface::makeObjectValue(Reference(Reference::Starbase, 363), session));
        v.verif.verifyString("NAME", "Rambo 3");
        v.verif.verifyInteger("LOC.X", 2000);
    }

    // Ion Storm
    {
        NewContextVerifier v(a("ref storm"), game::interface::makeObjectValue(Reference(Reference::IonStorm, 7), session));
        v.verif.verifyString("NAME", "Katrina");
        v.verif.verifyInteger("LOC.X", 1500);
    }

    // Minefield
    {
        NewContextVerifier v(a("ref mine"), game::interface::makeObjectValue(Reference(Reference::Minefield, 150), session));
        v.verif.verifyInteger("UNITS", 400);
        v.verif.verifyInteger("LOC.X", 2400);
    }

    // Ufo
    {
        NewContextVerifier v(a("ref mine"), game::interface::makeObjectValue(Reference(Reference::Ufo, 51), session));
        v.verif.verifyString("NAME", "Invader");
        v.verif.verifyInteger("LOC.X", 2300);
    }

    // Hull
    {
        NewContextVerifier v(a("ref hull"), game::interface::makeObjectValue(Reference(Reference::Hull, game::test::ANNIHILATION_HULL_ID), session));
        v.verif.verifyString("NAME", "ANNIHILATION CLASS BATTLESHIP");
        v.verif.verifyInteger("MASS", 960);
    }

    // Engine
    {
        NewContextVerifier v(a("ref engine"), game::interface::makeObjectValue(Reference(Reference::Engine, 9), session));
        v.verif.verifyString("NAME", "Transwarp Drive");
        v.verif.verifyInteger("COST.MC", 300);
    }

    // Beam
    {
        NewContextVerifier v(a("ref beam"), game::interface::makeObjectValue(Reference(Reference::Beam, 5), session));
        v.verif.verifyString("NAME", "Positron Beam");
        v.verif.verifyInteger("COST.MC", 12);
    }

    // Torpedo
    {
        NewContextVerifier v(a("ref torp"), game::interface::makeObjectValue(Reference(Reference::Torpedo, 7), session));
        v.verif.verifyString("NAME", "Mark 5 Photon");
        v.verif.verifyInteger("COST.MC", 57);
    }

    // Null
    verifyNewNull(a("ref player null"), game::interface::makeObjectValue(Reference(Reference::Player,    8), session));
    verifyNewNull(a("ref ship null"),   game::interface::makeObjectValue(Reference(Reference::Ship,      8), session));
    verifyNewNull(a("ref planet null"), game::interface::makeObjectValue(Reference(Reference::Planet,    8), session));
    verifyNewNull(a("ref base null"),   game::interface::makeObjectValue(Reference(Reference::Starbase,  8), session));
    verifyNewNull(a("ref storm null"),  game::interface::makeObjectValue(Reference(Reference::IonStorm,  8), session));
    verifyNewNull(a("ref mine null"),   game::interface::makeObjectValue(Reference(Reference::Minefield, 8), session));
    verifyNewNull(a("ref ufo null"),    game::interface::makeObjectValue(Reference(Reference::Ufo,       8), session));
    verifyNewNull(a("ref hull null"),   game::interface::makeObjectValue(Reference(Reference::Hull,      8), session));
    verifyNewNull(a("ref engine null"), game::interface::makeObjectValue(Reference(Reference::Engine,    8), session));
    verifyNewNull(a("ref beam null"),   game::interface::makeObjectValue(Reference(Reference::Beam,     11), session));
    verifyNewNull(a("ref torp null"),   game::interface::makeObjectValue(Reference(Reference::Torpedo,  11), session));
}

/** Test getReferenceTypeName().
    For the regular types, verify the backward mapping as well. */
AFL_TEST("game.interface.ReferenceContext:getReferenceTypeName", a)
{
    // Special cases
    a.checkNull("01", game::interface::getReferenceTypeName(Reference::Null));
    a.checkNonNull("02", game::interface::getReferenceTypeName(Reference::MapLocation));

    // Regular cases. Those must all map back and forth
    static const Reference::Type types[] = {
        Reference::Special,   Reference::Player,   Reference::Ship,
        Reference::Planet,    Reference::Starbase, Reference::IonStorm,
        Reference::Minefield, Reference::Ufo,      Reference::Hull,
        Reference::Engine,    Reference::Beam,     Reference::Torpedo,
    };
    for (size_t i = 0; i < sizeof(types)/sizeof(types[0]); ++i) {
        const char* ch = game::interface::getReferenceTypeName(types[i]);
        a.checkNonNull("11. getReferenceTypeName", ch);
        a.checkDifferent("12. result", String_t(ch), "");

        Reference::Type reverse;
        afl::test::Assert am(a(ch));
        am.check("parseReferenceTypeName", game::interface::parseReferenceTypeName(ch, reverse));
        am.check("strlen", std::strlen(ch) >= 2);
        am.checkEqual("reverse mapping", reverse, types[i]);
    }
}

/** Test parseReferenceTypeName().
    Long names are already tested above. Test the documented short names. */
AFL_TEST("game.interface.ReferenceContext:parseReferenceTypeName", a)
{
    Reference::Type t;
    a.check("01", game::interface::parseReferenceTypeName("b", t));
    a.checkEqual("02", t, Reference::Starbase);

    a.check("11", game::interface::parseReferenceTypeName("e", t));
    a.checkEqual("12", t, Reference::Engine);

    a.check("21", game::interface::parseReferenceTypeName("h", t));
    a.checkEqual("22", t, Reference::Hull);

    a.check("31", game::interface::parseReferenceTypeName("i", t));
    a.checkEqual("32", t, Reference::IonStorm);

    a.check("41", game::interface::parseReferenceTypeName("m", t));
    a.checkEqual("42", t, Reference::Minefield);

    a.check("51", game::interface::parseReferenceTypeName("p", t));
    a.checkEqual("52", t, Reference::Planet);

    a.check("61", game::interface::parseReferenceTypeName("s", t));
    a.checkEqual("62", t, Reference::Ship);

    a.check("71", game::interface::parseReferenceTypeName("t", t));
    a.checkEqual("72", t, Reference::Torpedo);

    a.check("81", game::interface::parseReferenceTypeName("w", t));
    a.checkEqual("82", t, Reference::Beam);

    a.check("91", game::interface::parseReferenceTypeName("y", t));
    a.checkEqual("92", t, Reference::Player);
}

/** Test ReferenceContext class. */
AFL_TEST("game.interface.ReferenceContext:basics", a)
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
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();
    verif.verifyTypes();
    verif.verifyString("KIND", "storm");
    verif.verifyInteger("ID", 7);
    a.checkNull("01. getObject", testee.getObject());
    a.checkEqual("02. getReference", testee.getReference(), Reference(Reference::IonStorm, 7));
    AFL_CHECK_THROWS(a("03. set ID"), verif.setIntegerValue("ID", 99), interpreter::Error);
}

/*
 *  IFLocationReference
 */

// Regular invocation
AFL_TEST("game.interface.ReferenceContext:IFLocationReference:normal", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    afl::data::Segment seg;
    seg.pushBackInteger(1400);
    seg.pushBackInteger(1800);
    interpreter::Arguments args(seg, 0, 2);
    NewContextVerifier v(a, game::interface::IFLocationReference(session, args));
    v.verif.verifyInteger("LOC.X", 1400);
    v.verif.verifyInteger("LOC.Y", 1800);
    v.verif.verifyString("KIND", "location");
}

// Null Y argument
AFL_TEST("game.interface.ReferenceContext:IFLocationReference:null-y", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    afl::data::Segment seg;
    seg.pushBackInteger(1400);
    seg.pushBackNew(0);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewNull(a, game::interface::IFLocationReference(session, args));
}

// Null X argument
AFL_TEST("game.interface.ReferenceContext:IFLocationReference:null-x", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    afl::data::Segment seg;
    seg.pushBackNew(0);
    seg.pushBackInteger(1400);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewNull(a, game::interface::IFLocationReference(session, args));
}

// Range error
AFL_TEST("game.interface.ReferenceContext:IFLocationReference:error:range", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    afl::data::Segment seg;
    seg.pushBackInteger(1400);
    seg.pushBackInteger(-1);
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFLocationReference(session, args), interpreter::Error);
}

// Type error
AFL_TEST("game.interface.ReferenceContext:IFLocationReference:error:type", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    afl::data::Segment seg;
    seg.pushBackInteger(1400);
    seg.pushBackString("X");
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFLocationReference(session, args), interpreter::Error);
}

// Arity error
AFL_TEST("game.interface.ReferenceContext:IFLocationReference:error:arity", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    afl::data::Segment seg;
    seg.pushBackInteger(1400);
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFLocationReference(session, args), interpreter::Error);
}

/*
 *  IFReference
 */


// Regular invocation
AFL_TEST("game.interface.ReferenceContext:IFReference:normal", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    afl::data::Segment seg;
    seg.pushBackString("base");
    seg.pushBackInteger(5);
    interpreter::Arguments args(seg, 0, 2);
    NewContextVerifier v(a, game::interface::IFReference(session, args));
    v.verif.verifyString("KIND", "base");
    v.verif.verifyInteger("ID", 5);
}

// Null Id argument
AFL_TEST("game.interface.ReferenceContext:IFReference:null-id", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    afl::data::Segment seg;
    seg.pushBackString("base");
    seg.pushBackNew(0);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewNull(a, game::interface::IFReference(session, args));
}

// Null type argument
AFL_TEST("game.interface.ReferenceContext:IFReference:null-type", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    afl::data::Segment seg;
    seg.pushBackNew(0);
    seg.pushBackInteger(5);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewNull(a, game::interface::IFReference(session, args));
}

// Range error
AFL_TEST("game.interface.ReferenceContext:IFReference:error:range", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    afl::data::Segment seg;
    seg.pushBackString("base");
    seg.pushBackInteger(-1);
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFReference(session, args), interpreter::Error);
}

// Type error
AFL_TEST("game.interface.ReferenceContext:IFReference:error:type", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    afl::data::Segment seg;
    seg.pushBackString("base");
    seg.pushBackString("X");
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFReference(session, args), interpreter::Error);
}

// Bad type string
AFL_TEST("game.interface.ReferenceContext:IFReference:error:bad-type", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    afl::data::Segment seg;
    seg.pushBackString("grill");
    seg.pushBackInteger(5);
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFReference(session, args), interpreter::Error);
}

// Arity error
AFL_TEST("game.interface.ReferenceContext:IFReference:error:arity", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    afl::data::Segment seg;
    seg.pushBackString("base");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFReference(session, args), interpreter::Error);
}

/*
 *  checkReferenceArg
 */

// Regular
AFL_TEST("game.interface.ReferenceContext:checkReferenceArg:normal", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    game::interface::ReferenceContext ctx(Reference(Reference::Planet, 77), session);
    Reference out;
    a.checkEqual("status", game::interface::checkReferenceArg(out, &ctx), true);
    a.checkEqual("result", out, Reference(Reference::Planet, 77));
}

// Null
AFL_TEST("game.interface.ReferenceContext:checkReferenceArg:null", a)
{
    Reference out;
    a.checkEqual("status", game::interface::checkReferenceArg(out, 0), false);
}

// Wrong type: integer
AFL_TEST("game.interface.ReferenceContext:checkReferenceArg:error:type", a)
{
    afl::data::IntegerValue iv(77);
    Reference out;
    AFL_CHECK_THROWS(a, game::interface::checkReferenceArg(out, &iv), interpreter::Error);
}

// Wrong type: other context
AFL_TEST("game.interface.ReferenceContext:checkReferenceArg:error:context", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    game::interface::GlobalContext ctx(session);
    Reference out;
    AFL_CHECK_THROWS(a, game::interface::checkReferenceArg(out, &ctx), interpreter::Error);
}
