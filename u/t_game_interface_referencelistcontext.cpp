/**
  *  \file u/t_game_interface_referencelistcontext.cpp
  *  \brief Test for game::interface::ReferenceListContext
  */

#include "game/interface/referencelistcontext.hpp"

#include "t_game_interface.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
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

    void addDefaultUniverse(Environment& env)
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
            TS_ASSERT(!sh4.isReliablyVisible(PLAYER));
        }
        env.session.setGame(g);
    }
}

/** Test creation function.
    Exercise creation of ReferenceListContext using "ReferenceList()" script function. */
void
TestGameInterfaceReferenceListContext::testCreate()
{
    // Success case
    {
        Environment env;
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        NewContextVerifier v("testCreate", game::interface::IFReferenceList(env.session, args));
        v.verif.verifyBasics();
        v.verif.verifyNotSerializable();
        v.verif.verifyTypes();
        TS_ASSERT_THROWS(v.verif.setIntegerValue("OBJECTS", 0), interpreter::Error);

        ReferenceListContext* ctx = dynamic_cast<ReferenceListContext*>(v.value.get());
        TS_ASSERT(ctx != 0);
        TS_ASSERT(ctx->getObject() == 0);
    }

    // Error case: arity error
    {
        Environment env;
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFReferenceList(env.session, args), interpreter::Error);
    }
}

/** Test "ReferenceList().Add" command. */
void
TestGameInterfaceReferenceListContext::testAdd()
{
    // Standard case: 'Call ReferenceList()->Add ...'
    {
        Environment env;
        ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
        NewCallable cv("add ok", ctx, "ADD");

        afl::data::Segment seg;
        seg.pushBackNew(new ReferenceContext(Reference(Reference::Planet, 33), env.session));
        seg.pushBackNew(0);
        seg.pushBackNew(new ReferenceContext(Reference(Reference::Ship, 77), env.session));
        cv.callable.call(env.proc, seg, false);

        TS_ASSERT_EQUALS(ctx.getList().size(), 2U);
        TS_ASSERT_EQUALS(ctx.getList()[0], Reference(Reference::Planet, 33));
        TS_ASSERT_EQUALS(ctx.getList()[1], Reference(Reference::Ship, 77));
    }

    // Type error: 'Add' with wrong type
    {
        Environment env;
        ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
        NewCallable cv("add type error", ctx, "ADD");

        afl::data::Segment seg;
        seg.pushBackNew(new ReferenceContext(Reference(Reference::Planet, 33), env.session));
        seg.pushBackInteger(16);
        TS_ASSERT_THROWS(cv.callable.call(env.proc, seg, false), interpreter::Error);
    }

    // Arity error: 'Add' with no args
    {
        Environment env;
        ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
        NewCallable cv("add arity error", ctx, "ADD");

        afl::data::Segment seg;
        TS_ASSERT_THROWS(cv.callable.call(env.proc, seg, false), interpreter::Error);
    }
}

/** Test "ReferenceList().AddObjects" command. */
void
TestGameInterfaceReferenceListContext::testAddObjects()
{
    // Standard case: 'Call ReferenceList()->AddObjects "ship", ...'
    {
        Environment env;
        ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
        NewCallable cv("addobjects ok", ctx, "ADDOBJECTS");

        afl::data::Segment seg;
        seg.pushBackString("ship");
        seg.pushBackInteger(10);
        seg.pushBackNew(0);
        seg.pushBackInteger(30);
        seg.pushBackInteger(20);
        cv.callable.call(env.proc, seg, false);

        TS_ASSERT_EQUALS(ctx.getList().size(), 3U);
        TS_ASSERT_EQUALS(ctx.getList()[0], Reference(Reference::Ship, 10));
        TS_ASSERT_EQUALS(ctx.getList()[1], Reference(Reference::Ship, 30));
        TS_ASSERT_EQUALS(ctx.getList()[2], Reference(Reference::Ship, 20));
    }

    // Type error: Id with wrong type
    {
        Environment env;
        ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
        NewCallable cv("addobjects type error", ctx, "ADDOBJECTS");

        afl::data::Segment seg;
        seg.pushBackString("ship");
        seg.pushBackInteger(10);
        seg.pushBackString("10");
        TS_ASSERT_THROWS(cv.callable.call(env.proc, seg, false), interpreter::Error);
    }

    // Arity error: at least one Id required
    {
        Environment env;
        ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
        NewCallable cv("addobjects arity error", ctx, "ADDOBJECTS");

        afl::data::Segment seg;
        seg.pushBackString("ship");
        TS_ASSERT_THROWS(cv.callable.call(env.proc, seg, false), interpreter::Error);
    }

    // Bad type string
    {
        Environment env;
        ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
        NewCallable cv("addobjects type-string error", ctx, "ADDOBJECTS");

        afl::data::Segment seg;
        seg.pushBackString("xyzzy");
        seg.pushBackInteger(10);
        seg.pushBackInteger(20);
        TS_ASSERT_THROWS(cv.callable.call(env.proc, seg, false), interpreter::Error);
    }
}

void
TestGameInterfaceReferenceListContext::testAddObjectsAt()
{
    // Default case: 'Call ReferenceList()->AddObjectsAt X,Y' -> ships 1+2 (played)
    {
        Environment env;
        addDefaultUniverse(env);
        ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
        NewCallable cv("addobjects default", ctx, "ADDOBJECTSAT");

        afl::data::Segment seg;
        seg.pushBackInteger(DEFAULT_X);
        seg.pushBackInteger(DEFAULT_Y);
        cv.callable.call(env.proc, seg, false);

        TS_ASSERT_EQUALS(ctx.getList().size(), 2U);
        TS_ASSERT_EQUALS(ctx.getList()[0], Reference(Reference::Ship, 1));
        TS_ASSERT_EQUALS(ctx.getList()[1], Reference(Reference::Ship, 2));
    }

    // With foreign ships: 'Call ReferenceList()->AddObjectsAt X,Y,"f"' -> ships 1+2+3+4
    {
        Environment env;
        addDefaultUniverse(env);
        ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
        NewCallable cv("addobjects f", ctx, "ADDOBJECTSAT");

        afl::data::Segment seg;
        seg.pushBackInteger(DEFAULT_X);
        seg.pushBackInteger(DEFAULT_Y);
        seg.pushBackString("F");
        cv.callable.call(env.proc, seg, false);

        TS_ASSERT_EQUALS(ctx.getList().size(), 4U);
        TS_ASSERT_EQUALS(ctx.getList()[0], Reference(Reference::Ship, 1));
        TS_ASSERT_EQUALS(ctx.getList()[1], Reference(Reference::Ship, 2));
        TS_ASSERT_EQUALS(ctx.getList()[2], Reference(Reference::Ship, 3));
        TS_ASSERT_EQUALS(ctx.getList()[3], Reference(Reference::Ship, 4));
    }

    // With foreign ships, reliable only: 'Call ReferenceList()->AddObjectsAt X,Y,"fs"' -> ships 1+2+3
    {
        Environment env;
        addDefaultUniverse(env);
        ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
        NewCallable cv("addobjects fs", ctx, "ADDOBJECTSAT");

        afl::data::Segment seg;
        seg.pushBackInteger(DEFAULT_X);
        seg.pushBackInteger(DEFAULT_Y);
        seg.pushBackString("fs");
        cv.callable.call(env.proc, seg, false);

        TS_ASSERT_EQUALS(ctx.getList().size(), 3U);
        TS_ASSERT_EQUALS(ctx.getList()[0], Reference(Reference::Ship, 1));
        TS_ASSERT_EQUALS(ctx.getList()[1], Reference(Reference::Ship, 2));
        TS_ASSERT_EQUALS(ctx.getList()[2], Reference(Reference::Ship, 3));
    }

    // With foreign ships and planet: 'Call ReferenceList()->AddObjectsAt X,Y,"fp"' -> planet 10 + ships 1+2+3+4
    {
        Environment env;
        addDefaultUniverse(env);
        ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
        NewCallable cv("addobjects fp", ctx, "ADDOBJECTSAT");

        afl::data::Segment seg;
        seg.pushBackInteger(DEFAULT_X);
        seg.pushBackInteger(DEFAULT_Y);
        seg.pushBackString("fp");
        cv.callable.call(env.proc, seg, false);

        TS_ASSERT_EQUALS(ctx.getList().size(), 5U);
        TS_ASSERT_EQUALS(ctx.getList()[0], Reference(Reference::Planet, 10));
        TS_ASSERT_EQUALS(ctx.getList()[1], Reference(Reference::Ship, 1));
        TS_ASSERT_EQUALS(ctx.getList()[2], Reference(Reference::Ship, 2));
        TS_ASSERT_EQUALS(ctx.getList()[3], Reference(Reference::Ship, 3));
        TS_ASSERT_EQUALS(ctx.getList()[4], Reference(Reference::Ship, 4));
    }

    // Exclude ship by number: 'Call ReferenceList()->AddObjectsAt X,Y,2' -> ship 1
    {
        Environment env;
        addDefaultUniverse(env);
        ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
        NewCallable cv("addobjects num", ctx, "ADDOBJECTSAT");

        afl::data::Segment seg;
        seg.pushBackInteger(DEFAULT_X);
        seg.pushBackInteger(DEFAULT_Y);
        seg.pushBackInteger(2);
        cv.callable.call(env.proc, seg, false);

        TS_ASSERT_EQUALS(ctx.getList().size(), 1U);
        TS_ASSERT_EQUALS(ctx.getList()[0], Reference(Reference::Ship, 1));
    }

    // Exclude ship, and options: 'Call ReferenceList()->AddObjectsAt X,Y,"fps1"' -> planet 10, ships 2+3
    {
        Environment env;
        addDefaultUniverse(env);
        ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
        NewCallable cv("addobjects fps1", ctx, "ADDOBJECTSAT");

        afl::data::Segment seg;
        seg.pushBackInteger(DEFAULT_X);
        seg.pushBackInteger(DEFAULT_Y);
        seg.pushBackString("fps1");
        cv.callable.call(env.proc, seg, false);

        TS_ASSERT_EQUALS(ctx.getList().size(), 3U);
        TS_ASSERT_EQUALS(ctx.getList()[0], Reference(Reference::Planet, 10));
        TS_ASSERT_EQUALS(ctx.getList()[1], Reference(Reference::Ship, 2));
        TS_ASSERT_EQUALS(ctx.getList()[2], Reference(Reference::Ship, 3));
    }

    // Null Y coordinate
    {
        Environment env;
        addDefaultUniverse(env);
        ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
        NewCallable cv("addobjects X,null", ctx, "ADDOBJECTSAT");

        afl::data::Segment seg;
        seg.pushBackInteger(DEFAULT_X);
        seg.pushBackNew(0);
        cv.callable.call(env.proc, seg, false);

        TS_ASSERT_EQUALS(ctx.getList().size(), 0U);
    }

    // Null X coordinate
    {
        Environment env;
        addDefaultUniverse(env);
        ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
        NewCallable cv("addobjects null,Y", ctx, "ADDOBJECTSAT");

        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackInteger(DEFAULT_Y);
        cv.callable.call(env.proc, seg, false);

        TS_ASSERT_EQUALS(ctx.getList().size(), 0U);
    }

    // Arity error
    {
        Environment env;
        addDefaultUniverse(env);
        ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
        NewCallable cv("addobjects arity error", ctx, "ADDOBJECTSAT");

        afl::data::Segment seg;
        seg.pushBackInteger(DEFAULT_X);
        TS_ASSERT_THROWS(cv.callable.call(env.proc, seg, false), interpreter::Error);
    }

    // Type error
    {
        Environment env;
        addDefaultUniverse(env);
        ReferenceListContext ctx(*new ReferenceListContext::Data(), env.session);
        NewCallable cv("addobjects type error", ctx, "ADDOBJECTSAT");

        afl::data::Segment seg;
        seg.pushBackString("X");
        TS_ASSERT_THROWS(cv.callable.call(env.proc, seg, false), interpreter::Error);
    }
}

/** Test accessing the Objects array. */
void
TestGameInterfaceReferenceListContext::testObjects()
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
    std::auto_ptr<afl::data::Value> obj(interpreter::test::ContextVerifier(ctx, "objects").getValue("OBJECTS"));
    interpreter::IndexableValue* ix = dynamic_cast<interpreter::IndexableValue*>(obj.get());
    TS_ASSERT(ix != 0);

    interpreter::test::ValueVerifier verif(*ix, "objects");
    verif.verifyBasics();
    verif.verifyNotSerializable();
    TS_ASSERT_EQUALS(ix->getDimension(0), 1);
    TS_ASSERT_EQUALS(ix->getDimension(1), 3);      // 3 elements, starting at 0

    // Test iteration
    {
        afl::test::Assert a("objects it");
        game::Reference ref;
        std::auto_ptr<interpreter::Context> it(ix->makeFirstContext());
        TS_ASSERT(it.get() != 0);
        interpreter::test::ContextVerifier itVerif(*it, a);
        itVerif.verifyBasics();
        itVerif.verifyNotSerializable();
        itVerif.verifyTypes();
        TS_ASSERT(it->getObject() == 0);
        itVerif.verifyInteger("ID", 10);
        itVerif.verifyString("KIND", "ship");
        TS_ASSERT(game::interface::checkReferenceArg(ref, it.get()));
        TS_ASSERT_EQUALS(ref, Reference(Reference::Ship, 10));
        TS_ASSERT(it->next());
        itVerif.verifyInteger("ID", 30);
        TS_ASSERT(game::interface::checkReferenceArg(ref, it.get()));
        TS_ASSERT_EQUALS(ref, Reference(Reference::Ship, 30));
        TS_ASSERT(it->next());
        itVerif.verifyInteger("ID", 20);
        TS_ASSERT(game::interface::checkReferenceArg(ref, it.get()));
        TS_ASSERT_EQUALS(ref, Reference(Reference::Ship, 20));
        TS_ASSERT(!it->next());
    }

    // Test element read access
    {
        afl::test::Assert a("objects get");
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
        TS_ASSERT(game::interface::checkReferenceArg(ref, p.get()));
        TS_ASSERT_EQUALS(ref, Reference(Reference::Ship, 20));
        TS_ASSERT(!mustBeContext(a, p.get()).next());
    }

    // Element read access, range error
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);

        TS_ASSERT_THROWS(ix->get(args), interpreter::Error);
    }

    // Element read access, null
    {
        afl::test::Assert a("objects get range");
        afl::data::Segment seg;
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 1);

        interpreter::test::verifyNewNull(a, ix->get(args));
    }

    // Element read access, type error
    {
        afl::data::Segment seg;
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 1);

        TS_ASSERT_THROWS(ix->get(args), interpreter::Error);
    }

    // Element read access, arity error
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);

        TS_ASSERT_THROWS(ix->get(args), interpreter::Error);
    }

    // Test element write access
    {
        afl::test::Assert a("objects set");
        afl::data::Segment seg;
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 1);
        game::interface::ReferenceContext nv(Reference(Reference::Planet, 77), env.session);

        ix->set(args, &nv);
        TS_ASSERT_EQUALS(ctx.getList()[2], Reference(Reference::Planet, 77));
    }

    // Test element write access, type error
    {
        afl::test::Assert a("objects set type");
        afl::data::Segment seg;
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 1);
        afl::data::StringValue nv("S");

        TS_ASSERT_THROWS(ix->set(args, &nv), interpreter::Error);
    }

    // Test element write access, index range error
    {
        afl::test::Assert a("objects set range");
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        game::interface::ReferenceContext nv(Reference(Reference::Planet, 77), env.session);

        TS_ASSERT_THROWS(ix->set(args, &nv), interpreter::Error);
    }

    // Test element write access, index null
    {
        afl::test::Assert a("objects set index null");
        afl::data::Segment seg;
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 1);
        game::interface::ReferenceContext nv(Reference(Reference::Planet, 77), env.session);

        TS_ASSERT_THROWS(ix->set(args, &nv), interpreter::Error);
    }

    // Test element write access, assigning null
    {
        afl::test::Assert a("objects set to null");
        afl::data::Segment seg;
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 1);

        TS_ASSERT_THROWS(ix->set(args, 0), interpreter::Error);
    }

    // Test element write access, arity error
    {
        afl::test::Assert a("objects set arity");
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        game::interface::ReferenceContext nv(Reference(Reference::Planet, 77), env.session);

        TS_ASSERT_THROWS(ix->set(args, &nv), interpreter::Error);
    }
}

