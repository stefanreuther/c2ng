/**
  *  \file test/game/interface/referencelistcontexttest.cpp
  *  \brief Test for game::interface::ReferenceListContext
  */

#include "game/interface/referencelistcontext.hpp"

#include "afl/data/segment.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/interface/referencecontext.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/shipdata.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/process.hpp"
#include "interpreter/test/contextverifier.hpp"

using game::Reference;
using game::interface::ReferenceContext;
using game::interface::ReferenceListContext;
using interpreter::CallableValue;

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


    /*
     *  A simplification for "retrieve an attribute as Callable"
     *  (manage lifetime and ensure correct type)
     */

    interpreter::CallableValue& mustBeCallable(afl::test::Assert a, afl::data::Value* v)
    {
        CallableValue* cv = dynamic_cast<CallableValue*>(v);
        a.check("cv != 0", cv != 0);

        // Verify the callable, just in case
        interpreter::test::ValueVerifier pv(*cv, a("callable"));
        pv.verifyBasics();
        pv.verifyNotSerializable();

        return *cv;
    }

    struct NewCallable {
        NewCallable(afl::test::Assert a, interpreter::Context& ctx, const char* name)
            : value(interpreter::test::ContextVerifier(ctx, a("context")).getValue(name)),
              callable(mustBeCallable(a("callable"), value.get()))
            { }

        std::auto_ptr<afl::data::Value> value;
        interpreter::CallableValue& callable;
    };


    /*
     *  Environment
     */
    struct Environment {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;
        interpreter::Process proc;

        Environment()
            : tx(), fs(), session(tx, fs),
              proc(session.world(), "tester", 777)
            { }
    };

    const int DEFAULT_X = 1200;
    const int DEFAULT_Y = 1300;
    const int PLAYER = 1;

    void addDefaultUniverse(afl::test::Assert a, Environment& env)
    {
        afl::base::Ptr<game::Game> g = new game::Game();
        game::map::Universe& univ = g->currentTurn().universe();
        g->setViewpointPlayer(PLAYER);

        const game::PlayerSet_t SET(PLAYER);

        // A planet
        game::map::Planet& p = *univ.planets().create(10);
        p.setPosition(game::map::Point(DEFAULT_X, DEFAULT_Y));
        p.internalCheck(g->mapConfiguration(), SET, 10, env.tx, env.session.log());

        // Two played ships
        game::map::ShipData sd;
        sd.owner = PLAYER;
        sd.x = DEFAULT_X;
        sd.y = DEFAULT_Y;
        {
            game::map::Ship& sh1 = *univ.ships().create(1);
            sh1.addCurrentShipData(sd, SET);
            sh1.setPlayability(game::map::Object::Playable);
            sh1.internalCheck(SET, 10);
        }
        {
            game::map::Ship& sh2 = *univ.ships().create(2);
            sh2.addCurrentShipData(sd, SET);
            sh2.setPlayability(game::map::Object::Playable);
            sh2.internalCheck(SET, 10);
        }

        // A foreign ship
        {
            game::map::Ship& sh3 = *univ.ships().create(3);
            sh3.addShipXYData(game::map::Point(DEFAULT_X, DEFAULT_Y), 2, 100, SET);
            sh3.setPlayability(game::map::Object::NotPlayable);
            sh3.internalCheck(SET, 10);
        }

        // A guessed ship
        {
            game::map::Ship& sh4 = *univ.ships().create(4);
            game::parser::MessageInformation mi4(game::parser::MessageInformation::Ship, 4, 10);
            mi4.addValue(game::parser::mi_X, DEFAULT_X);
            mi4.addValue(game::parser::mi_Y, DEFAULT_Y);
            mi4.addValue(game::parser::mi_Owner, 3);
            mi4.addValue(game::parser::mi_Mass, 100);
            sh4.addMessageInformation(mi4, game::PlayerSet_t());
            sh4.setPlayability(game::map::Object::NotPlayable);
            sh4.internalCheck(SET, 10);
            a.check("sh4 !isReliablyVisible", !sh4.isReliablyVisible(PLAYER));
        }
        env.session.setGame(g);
    }
}

/** Test creation function.
    Exercise creation of ReferenceListContext using "ReferenceList()" script function. */

// Success case
AFL_TEST("game.interface.ReferenceListContext:create:normal", a)
{
    Environment env;
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    NewContextVerifier v(a, game::interface::IFReferenceList(env.session, args));
    v.verif.verifyBasics();
    v.verif.verifyNotSerializable();
    v.verif.verifyTypes();
    AFL_CHECK_THROWS(a("01. set OBJECTS"), v.verif.setIntegerValue("OBJECTS", 0), interpreter::Error);

    ReferenceListContext* ctx = dynamic_cast<ReferenceListContext*>(v.value.get());
    a.checkNonNull("11. ctx", ctx);
    a.checkNull("12. getObject", ctx->getObject());
}

// Error case: arity error
AFL_TEST("game.interface.ReferenceListContext:create:error:arity", a)
{
    Environment env;
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFReferenceList(env.session, args), interpreter::Error);
}

/*
 * Test "ReferenceList().Add" command
 */


// Standard case: 'Call ReferenceList()->Add ...'
AFL_TEST("game.interface.ReferenceListContext:Add:normal", a)
{
    Environment env;
    ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
    NewCallable cv(a, ctx, "ADD");

    afl::data::Segment seg;
    seg.pushBackNew(new ReferenceContext(Reference(Reference::Planet, 33), env.session));
    seg.pushBackNew(0);
    seg.pushBackNew(new ReferenceContext(Reference(Reference::Ship, 77), env.session));
    cv.callable.call(env.proc, seg, false);

    a.checkEqual("01. size", ctx.getList().size(), 2U);
    a.checkEqual("02. index 0", ctx.getList()[0], Reference(Reference::Planet, 33));
    a.checkEqual("03. index 1", ctx.getList()[1], Reference(Reference::Ship, 77));
}

// Type error: 'Add' with wrong type
AFL_TEST("game.interface.ReferenceListContext:Add:error:type", a)
{
    Environment env;
    ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
    ctx.getList().add(Reference(Reference::Ship, 22));
    NewCallable cv(a, ctx, "ADD");

    afl::data::Segment seg;
    seg.pushBackNew(new ReferenceContext(Reference(Reference::Planet, 33), env.session));
    seg.pushBackInteger(16);
    AFL_CHECK_THROWS(a, cv.callable.call(env.proc, seg, false), interpreter::Error);

    // List should not be modified
    a.checkEqual("01. size", ctx.getList().size(), 1U);
}

// Arity error: 'Add' with no args
AFL_TEST("game.interface.ReferenceListContext:Add:error:arity", a)
{
    Environment env;
    ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
    NewCallable cv(a, ctx, "ADD");

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, cv.callable.call(env.proc, seg, false), interpreter::Error);
}

/*
 *  Test "ReferenceList().AddObjects" command
 */

// Standard case: 'Call ReferenceList()->AddObjects "ship", ...'
AFL_TEST("game.interface.ReferenceListContext:AddObjects:normal", a)
{
    Environment env;
    ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
    NewCallable cv(a, ctx, "ADDOBJECTS");

    afl::data::Segment seg;
    seg.pushBackString("ship");
    seg.pushBackInteger(10);
    seg.pushBackNew(0);
    seg.pushBackInteger(30);
    seg.pushBackInteger(20);
    cv.callable.call(env.proc, seg, false);

    a.checkEqual("01. size", ctx.getList().size(), 3U);
    a.checkEqual("02. index 0", ctx.getList()[0], Reference(Reference::Ship, 10));
    a.checkEqual("03. index 1", ctx.getList()[1], Reference(Reference::Ship, 30));
    a.checkEqual("04. index 2", ctx.getList()[2], Reference(Reference::Ship, 20));
}

// Type error: Id with wrong type
AFL_TEST("game.interface.ReferenceListContext:AddObjects:error:type", a)
{
    Environment env;
    ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
    NewCallable cv(a, ctx, "ADDOBJECTS");

    afl::data::Segment seg;
    seg.pushBackString("ship");
    seg.pushBackInteger(10);
    seg.pushBackString("10");
    AFL_CHECK_THROWS(a, cv.callable.call(env.proc, seg, false), interpreter::Error);
}

// Arity error: at least one Id required
AFL_TEST("game.interface.ReferenceListContext:AddObjects:error:arity", a)
{
    Environment env;
    ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
    NewCallable cv(a, ctx, "ADDOBJECTS");

    afl::data::Segment seg;
    seg.pushBackString("ship");
    AFL_CHECK_THROWS(a, cv.callable.call(env.proc, seg, false), interpreter::Error);
}

// Bad type string
AFL_TEST("game.interface.ReferenceListContext:AddObjects:error:bad-type", a)
{
    Environment env;
    ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
    NewCallable cv(a, ctx, "ADDOBJECTS");

    afl::data::Segment seg;
    seg.pushBackString("xyzzy");
    seg.pushBackInteger(10);
    seg.pushBackInteger(20);
    AFL_CHECK_THROWS(a, cv.callable.call(env.proc, seg, false), interpreter::Error);
}

/*
 *  AddObjectsAt
 */


// Default case: 'Call ReferenceList()->AddObjectsAt X,Y' -> ships 1+2 (played)
AFL_TEST("game.interface.ReferenceListContext:AddObjectsAt:default", a)
{
    Environment env;
    addDefaultUniverse(a, env);
    ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
    NewCallable cv(a, ctx, "ADDOBJECTSAT");

    afl::data::Segment seg;
    seg.pushBackInteger(DEFAULT_X);
    seg.pushBackInteger(DEFAULT_Y);
    cv.callable.call(env.proc, seg, false);

    a.checkEqual("01. size", ctx.getList().size(), 2U);
    a.checkEqual("02. index 0", ctx.getList()[0], Reference(Reference::Ship, 1));
    a.checkEqual("03. index 1", ctx.getList()[1], Reference(Reference::Ship, 2));
}

// With foreign ships: 'Call ReferenceList()->AddObjectsAt X,Y,"f"' -> ships 1+2+3+4
AFL_TEST("game.interface.ReferenceListContext:AddObjectsAt:foreign", a)
{
    Environment env;
    addDefaultUniverse(a, env);
    ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
    NewCallable cv(a, ctx, "ADDOBJECTSAT");

    afl::data::Segment seg;
    seg.pushBackInteger(DEFAULT_X);
    seg.pushBackInteger(DEFAULT_Y);
    seg.pushBackString("F");
    cv.callable.call(env.proc, seg, false);

    a.checkEqual("01. size", ctx.getList().size(), 4U);
    a.checkEqual("02. index 0", ctx.getList()[0], Reference(Reference::Ship, 1));
    a.checkEqual("03. index 1", ctx.getList()[1], Reference(Reference::Ship, 2));
    a.checkEqual("04. index 2", ctx.getList()[2], Reference(Reference::Ship, 3));
    a.checkEqual("05. index 3", ctx.getList()[3], Reference(Reference::Ship, 4));
}

// With foreign ships, reliable only: 'Call ReferenceList()->AddObjectsAt X,Y,"fs"' -> ships 1+2+3
AFL_TEST("game.interface.ReferenceListContext:AddObjectsAt:foreign+safe", a)
{
    Environment env;
    addDefaultUniverse(a, env);
    ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
    NewCallable cv(a, ctx, "ADDOBJECTSAT");

    afl::data::Segment seg;
    seg.pushBackInteger(DEFAULT_X);
    seg.pushBackInteger(DEFAULT_Y);
    seg.pushBackString("fs");
    cv.callable.call(env.proc, seg, false);

    a.checkEqual("01. size", ctx.getList().size(), 3U);
    a.checkEqual("02. index 0", ctx.getList()[0], Reference(Reference::Ship, 1));
    a.checkEqual("03. index 1", ctx.getList()[1], Reference(Reference::Ship, 2));
    a.checkEqual("04. index 2", ctx.getList()[2], Reference(Reference::Ship, 3));
}

// With foreign ships and planet: 'Call ReferenceList()->AddObjectsAt X,Y,"fp"' -> planet 10 + ships 1+2+3+4
AFL_TEST("game.interface.ReferenceListContext:AddObjectsAt:foreign+planet", a)
{
    Environment env;
    addDefaultUniverse(a, env);
    ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
    NewCallable cv(a, ctx, "ADDOBJECTSAT");

    afl::data::Segment seg;
    seg.pushBackInteger(DEFAULT_X);
    seg.pushBackInteger(DEFAULT_Y);
    seg.pushBackString("fp");
    cv.callable.call(env.proc, seg, false);

    a.checkEqual("01. size", ctx.getList().size(), 5U);
    a.checkEqual("02. index 0", ctx.getList()[0], Reference(Reference::Planet, 10));
    a.checkEqual("03. index 1", ctx.getList()[1], Reference(Reference::Ship, 1));
    a.checkEqual("04. index 2", ctx.getList()[2], Reference(Reference::Ship, 2));
    a.checkEqual("05. index 3", ctx.getList()[3], Reference(Reference::Ship, 3));
    a.checkEqual("06. index 4", ctx.getList()[4], Reference(Reference::Ship, 4));
}

// Exclude ship by number: 'Call ReferenceList()->AddObjectsAt X,Y,2' -> ship 1
AFL_TEST("game.interface.ReferenceListContext:AddObjectsAt:exclude-ship", a)
{
    Environment env;
    addDefaultUniverse(a, env);
    ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
    NewCallable cv(a, ctx, "ADDOBJECTSAT");

    afl::data::Segment seg;
    seg.pushBackInteger(DEFAULT_X);
    seg.pushBackInteger(DEFAULT_Y);
    seg.pushBackInteger(2);
    cv.callable.call(env.proc, seg, false);

    a.checkEqual("01. size", ctx.getList().size(), 1U);
    a.checkEqual("02. index 0", ctx.getList()[0], Reference(Reference::Ship, 1));
}

// Exclude ship, and options: 'Call ReferenceList()->AddObjectsAt X,Y,"fps1"' -> planet 10, ships 2+3
AFL_TEST("game.interface.ReferenceListContext:AddObjectsAt:exclude-ship-options", a)
{
    Environment env;
    addDefaultUniverse(a, env);
    ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
    NewCallable cv(a, ctx, "ADDOBJECTSAT");

    afl::data::Segment seg;
    seg.pushBackInteger(DEFAULT_X);
    seg.pushBackInteger(DEFAULT_Y);
    seg.pushBackString("fps1");
    cv.callable.call(env.proc, seg, false);

    a.checkEqual("01. size", ctx.getList().size(), 3U);
    a.checkEqual("02. index 0", ctx.getList()[0], Reference(Reference::Planet, 10));
    a.checkEqual("03. index 1", ctx.getList()[1], Reference(Reference::Ship, 2));
    a.checkEqual("04. index 2", ctx.getList()[2], Reference(Reference::Ship, 3));
}

// Null Y coordinate
AFL_TEST("game.interface.ReferenceListContext:AddObjectsAt:null-y", a)
{
    Environment env;
    addDefaultUniverse(a, env);
    ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
    NewCallable cv(a, ctx, "ADDOBJECTSAT");

    afl::data::Segment seg;
    seg.pushBackInteger(DEFAULT_X);
    seg.pushBackNew(0);
    cv.callable.call(env.proc, seg, false);

    a.checkEqual("01. size", ctx.getList().size(), 0U);
}

// Null X coordinate
AFL_TEST("game.interface.ReferenceListContext:AddObjectsAt:null-x", a)
{
    Environment env;
    addDefaultUniverse(a, env);
    ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
    NewCallable cv(a, ctx, "ADDOBJECTSAT");

    afl::data::Segment seg;
    seg.pushBackNew(0);
    seg.pushBackInteger(DEFAULT_Y);
    cv.callable.call(env.proc, seg, false);

    a.checkEqual("01. size", ctx.getList().size(), 0U);
}

// Arity error
AFL_TEST("game.interface.ReferenceListContext:AddObjectsAt:error:arity", a)
{
    Environment env;
    addDefaultUniverse(a, env);
    ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
    NewCallable cv(a, ctx, "ADDOBJECTSAT");

    afl::data::Segment seg;
    seg.pushBackInteger(DEFAULT_X);
    AFL_CHECK_THROWS(a, cv.callable.call(env.proc, seg, false), interpreter::Error);
}

// Type error
AFL_TEST("game.interface.ReferenceListContext:AddObjectsAt:error:type", a)
{
    Environment env;
    addDefaultUniverse(a, env);
    ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
    NewCallable cv(a, ctx, "ADDOBJECTSAT");

    afl::data::Segment seg;
    seg.pushBackString("X");
    AFL_CHECK_THROWS(a, cv.callable.call(env.proc, seg, false), interpreter::Error);
}

/*
 *  Test accessing the Objects array
 */

AFL_TEST("game.interface.ReferenceListContext:Objects", a)
{
    Environment env;
    ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);

    // Use AddObjects to populate the array
    NewCallable cv("objects addobjects ok", ctx, "ADDOBJECTS");
    afl::data::Segment seg;
    seg.pushBackString("ship");
    seg.pushBackInteger(10);
    seg.pushBackInteger(30);
    seg.pushBackInteger(20);
    cv.callable.call(env.proc, seg, false);

    // Retrieve OBJECTS attribute
    std::auto_ptr<afl::data::Value> obj(interpreter::test::ContextVerifier(ctx, a("Objects")).getValue("OBJECTS"));
    interpreter::IndexableValue* ix = dynamic_cast<interpreter::IndexableValue*>(obj.get());
    a.checkNonNull("01. IndexableValue", ix);

    interpreter::test::ValueVerifier verif(*ix, a("Objects"));
    verif.verifyBasics();
    verif.verifyNotSerializable();
    a.checkEqual("11. getDimension 0", ix->getDimension(0), 1U);
    a.checkEqual("12. getDimension 1", ix->getDimension(1), 3U);      // 3 elements, starting at 0

    // Test iteration
    {
        game::Reference ref;
        std::auto_ptr<interpreter::Context> it(ix->makeFirstContext());
        a.checkNonNull("21. makeFirstContext", it.get());
        interpreter::test::ContextVerifier itVerif(*it, a("21. makeFirstContext"));
        itVerif.verifyBasics();
        itVerif.verifyNotSerializable();
        itVerif.verifyTypes();
        a.checkNull("22. getObject", it->getObject());
        itVerif.verifyInteger("ID", 10);
        itVerif.verifyString("KIND", "ship");
        a.check("23. checkReferenceArg", game::interface::checkReferenceArg(ref, it.get()));
        a.checkEqual("24. ref", ref, Reference(Reference::Ship, 10));
        a.check("25. next", it->next());
        itVerif.verifyInteger("ID", 30);
        a.check("26. checkReferenceArg", game::interface::checkReferenceArg(ref, it.get()));
        a.checkEqual("27. ref", ref, Reference(Reference::Ship, 30));
        a.check("28. next", it->next());
        itVerif.verifyInteger("ID", 20);
        a.check("29. checkReferenceArg", game::interface::checkReferenceArg(ref, it.get()));
        a.checkEqual("30. ref", ref, Reference(Reference::Ship, 20));
        a.check("31. next", !it->next());
    }

    // Test element read access
    {
        game::Reference ref;
        afl::data::Segment seg;
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 1);

        std::auto_ptr<afl::data::Value> p(ix->get(args));
        interpreter::test::ContextVerifier eleVerif(mustBeContext(a, p.get()), a);
        eleVerif.verifyBasics();
        eleVerif.verifyNotSerializable();
        eleVerif.verifyTypes();
        eleVerif.verifyInteger("ID", 20);
        a.check("41. checkReferenceArg", game::interface::checkReferenceArg(ref, p.get()));
        a.checkEqual("42. ref", ref, Reference(Reference::Ship, 20));
        a.check("43. mustBeContext", !mustBeContext(a, p.get()).next());
    }

    // Element read access, range error
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);

        AFL_CHECK_THROWS(a("51. range error"), ix->get(args), interpreter::Error);
    }

    // Element read access, null
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 1);

        interpreter::test::verifyNewNull(a("52. null"), ix->get(args));
    }

    // Element read access, type error
    {
        afl::data::Segment seg;
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 1);

        AFL_CHECK_THROWS(a("61. type error"), ix->get(args), interpreter::Error);
    }

    // Element read access, arity error
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);

        AFL_CHECK_THROWS(a("71. arity error"), ix->get(args), interpreter::Error);
    }

    // Test element write access
    {
        afl::data::Segment seg;
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 1);
        game::interface::ReferenceContext nv(Reference(Reference::Planet, 77), env.session);

        ix->set(args, &nv);
        a.checkEqual("81. index 2", ctx.getList()[2], Reference(Reference::Planet, 77));
    }

    // Test element write access, type error
    {
        afl::data::Segment seg;
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 1);
        afl::data::StringValue nv("S");

        AFL_CHECK_THROWS(a("91. write type error"), ix->set(args, &nv), interpreter::Error);
    }

    // Test element write access, index range error
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        game::interface::ReferenceContext nv(Reference(Reference::Planet, 77), env.session);

        AFL_CHECK_THROWS(a("101. write range error"), ix->set(args, &nv), interpreter::Error);
    }

    // Test element write access, index null
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 1);
        game::interface::ReferenceContext nv(Reference(Reference::Planet, 77), env.session);

        AFL_CHECK_THROWS(a("111. write null index"), ix->set(args, &nv), interpreter::Error);
    }

    // Test element write access, assigning null
    {
        afl::data::Segment seg;
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 1);

        AFL_CHECK_THROWS(a("121. write null value"), ix->set(args, 0), interpreter::Error);
    }

    // Test element write access, arity error
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        game::interface::ReferenceContext nv(Reference(Reference::Planet, 77), env.session);

        AFL_CHECK_THROWS(a("131. write arity"), ix->set(args, &nv), interpreter::Error);
    }
}

