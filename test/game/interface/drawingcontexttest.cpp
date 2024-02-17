/**
  *  \file test/game/interface/drawingcontexttest.cpp
  *  \brief Test for game::interface::DrawingContext
  */

#include "game/interface/drawingcontext.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/drawing.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/process.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/world.hpp"

/** Test basics: general behaviour, specific properties. */
AFL_TEST("game.interface.DrawingContext:basics", a)
{
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::Turn> t = *new game::Turn();
    t->universe().drawings().addNew(new game::map::Drawing(game::map::Point(1100, 1200), game::map::Drawing::MarkerDrawing));
    t->universe().drawings().addNew(new game::map::Drawing(game::map::Point(1400, 1500), game::map::Drawing::MarkerDrawing));

    // Instance
    game::interface::DrawingContext testee(t, r, t->universe().drawings().begin());
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();
    verif.verifyTypes();
    a.checkNull("01. getObject", testee.getObject());

    // Specific properties
    verif.verifyInteger("LOC.X", 1100);
    verif.verifyString("COMMENT", "");

    // Iteration
    a.check("11. next", testee.next());
    verif.verifyInteger("LOC.X", 1400);
    a.check("12. next", !testee.next());
}

/** Test changing properties. */
AFL_TEST("game.interface.DrawingContext:set", a)
{
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::Turn> t = *new game::Turn();
    game::map::DrawingContainer::Iterator_t it = t->universe().drawings().addNew(new game::map::Drawing(game::map::Point(1100, 1200), game::map::Drawing::MarkerDrawing));
    (*it)->setColor(9);

    // Instance
    game::interface::DrawingContext testee(t, r, t->universe().drawings().begin());
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyInteger("COLOR", 9);

    // Try to modify
    AFL_CHECK_SUCCEEDS(a("01. setIntegerValue COLOR"), verif.setIntegerValue("COLOR", 11));
    verif.verifyInteger("COLOR", 11);
    a.checkEqual("02. getColor", (*it)->getColor(), 11);

    // Try to modify via method call
    std::auto_ptr<afl::data::Value> meth(verif.getValue("SETCOLOR"));
    interpreter::CallableValue* cv = dynamic_cast<interpreter::CallableValue*>(meth.get());
    a.checkNonNull("11. CallableValue", cv);
    {
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        afl::data::Segment seg;
        seg.pushBackInteger(13);
        interpreter::World world(log, tx, fs);
        interpreter::Process proc(world, "dummy", 1);
        AFL_CHECK_SUCCEEDS(a("12. call"), cv->call(proc, seg, false));
    }
    a.checkEqual("13. getColor", (*it)->getColor(), 13);

    // Cannot modify methods
    AFL_CHECK_THROWS(a("21. setIntegerValue SETCOLOR"), verif.setIntegerValue("SETCOLOR", 1), interpreter::Error);
}

/** Test changing properties on deleted object. */
AFL_TEST("game.interface.DrawingContext:set:deleted", a)
{
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::Turn> t = *new game::Turn();
    game::map::DrawingContainer::Iterator_t it = t->universe().drawings().addNew(new game::map::Drawing(game::map::Point(1100, 1200), game::map::Drawing::MarkerDrawing));

    // Instance
    game::interface::DrawingContext testee(t, r, t->universe().drawings().begin());
    interpreter::test::ContextVerifier verif(testee, a);

    // Parallel delete. Properties now report as null.
    t->universe().drawings().erase(it);
    verif.verifyNull("COLOR");

    // Try to modify, fails
    AFL_CHECK_THROWS(a, verif.setIntegerValue("COLOR", 11), interpreter::Error);
}

/** Test creating through factory function. */
AFL_TEST("game.interface.DrawingContext:create", a)
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());
    session.getGame()->currentTurn().universe().drawings().addNew(new game::map::Drawing(game::map::Point(1100, 1200), game::map::Drawing::MarkerDrawing));

    // Create
    std::auto_ptr<game::interface::DrawingContext> ctx(game::interface::DrawingContext::create(session, session.getGame()->currentTurn()));
    a.checkNonNull("01. create", ctx.get());

    interpreter::test::ContextVerifier verif(*ctx, a);
    verif.verifyInteger("LOC.X", 1100);
}

/*
 *  Test creating through factory function, given empty session
 */

// Only game
AFL_TEST("game.interface.DrawingContext:create:only-game", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(new game::Game());
    a.checkNull("", game::interface::DrawingContext::create(session, session.getGame()->currentTurn()));
}

// No Drawing
AFL_TEST("game.interface.DrawingContext:create:no-drawing", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());
    a.checkNull("", game::interface::DrawingContext::create(session, session.getGame()->currentTurn()));
}
