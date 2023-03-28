/**
  *  \file u/t_game_interface_drawingcontext.cpp
  *  \brief Test for game::interface::DrawingContext
  */

#include "game/interface/drawingcontext.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/map/drawing.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/process.hpp"
#include "interpreter/world.hpp"
#include "interpreter/arguments.hpp"

/** Test basics: general behaviour, specific properties. */
void
TestGameInterfaceDrawingContext::testBasics()
{
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::Turn> t = *new game::Turn();
    t->universe().drawings().addNew(new game::map::Drawing(game::map::Point(1100, 1200), game::map::Drawing::MarkerDrawing));
    t->universe().drawings().addNew(new game::map::Drawing(game::map::Point(1400, 1500), game::map::Drawing::MarkerDrawing));

    // Instance
    game::interface::DrawingContext testee(t, r, t->universe().drawings().begin());
    interpreter::test::ContextVerifier verif(testee, "testBasics");
    verif.verifyBasics();
    verif.verifyNotSerializable();
    verif.verifyTypes();
    TS_ASSERT(testee.getObject() == 0);

    // Specific properties
    verif.verifyInteger("LOC.X", 1100);
    verif.verifyString("COMMENT", "");

    // Iteration
    TS_ASSERT(testee.next());
    verif.verifyInteger("LOC.X", 1400);
    TS_ASSERT(!testee.next());
}

/** Test changing properties. */
void
TestGameInterfaceDrawingContext::testSet()
{
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::Turn> t = *new game::Turn();
    game::map::DrawingContainer::Iterator_t it = t->universe().drawings().addNew(new game::map::Drawing(game::map::Point(1100, 1200), game::map::Drawing::MarkerDrawing));
    (*it)->setColor(9);

    // Instance
    game::interface::DrawingContext testee(t, r, t->universe().drawings().begin());
    interpreter::test::ContextVerifier verif(testee, "testSet");
    verif.verifyInteger("COLOR", 9);

    // Try to modify
    TS_ASSERT_THROWS_NOTHING(verif.setIntegerValue("COLOR", 11));
    verif.verifyInteger("COLOR", 11);
    TS_ASSERT_EQUALS((*it)->getColor(), 11);

    // Try to modify via method call
    std::auto_ptr<afl::data::Value> meth(verif.getValue("SETCOLOR"));
    interpreter::CallableValue* cv = dynamic_cast<interpreter::CallableValue*>(meth.get());
    TS_ASSERT(cv != 0);
    {
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        afl::data::Segment seg;
        seg.pushBackInteger(13);
        interpreter::World world(log, tx, fs);
        interpreter::Process proc(world, "dummy", 1);
        TS_ASSERT_THROWS_NOTHING(cv->call(proc, seg, false));
    }
    TS_ASSERT_EQUALS((*it)->getColor(), 13);

    // Cannot modify methods
    TS_ASSERT_THROWS(verif.setIntegerValue("SETCOLOR", 1), interpreter::Error);
}

/** Test changing properties on deleted object. */
void
TestGameInterfaceDrawingContext::testSetDeleted()
{
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::Turn> t = *new game::Turn();
    game::map::DrawingContainer::Iterator_t it = t->universe().drawings().addNew(new game::map::Drawing(game::map::Point(1100, 1200), game::map::Drawing::MarkerDrawing));

    // Instance
    game::interface::DrawingContext testee(t, r, t->universe().drawings().begin());
    interpreter::test::ContextVerifier verif(testee, "testSetDeleted");

    // Parallel delete. Properties now report as null.
    t->universe().drawings().erase(it);
    verif.verifyNull("COLOR");

    // Try to modify, fails
    TS_ASSERT_THROWS(verif.setIntegerValue("COLOR", 11), interpreter::Error);
}

/** Test creating through factory function. */
void
TestGameInterfaceDrawingContext::testCreate()
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());
    session.getGame()->currentTurn().universe().drawings().addNew(new game::map::Drawing(game::map::Point(1100, 1200), game::map::Drawing::MarkerDrawing));

    // Create
    std::auto_ptr<game::interface::DrawingContext> ctx(game::interface::DrawingContext::create(session));
    TS_ASSERT(ctx.get() != 0);

    interpreter::test::ContextVerifier verif(*ctx, "testCreate");
    verif.verifyInteger("LOC.X", 1100);
}

/** Test creating through factory function, given empty session. */
void
TestGameInterfaceDrawingContext::testCreateEmpty()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;

    // Entirely empty session
    {
        game::Session session(tx, fs);
        TS_ASSERT(game::interface::DrawingContext::create(session) == 0);
    }

    // Only root
    {
        game::Session session(tx, fs);
        session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        TS_ASSERT(game::interface::DrawingContext::create(session) == 0);
    }

    // Only game
    {
        game::Session session(tx, fs);
        session.setGame(new game::Game());
        TS_ASSERT(game::interface::DrawingContext::create(session) == 0);
    }

    // No Drawing
    {
        game::Session session(tx, fs);
        session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        session.setGame(new game::Game());
        TS_ASSERT(game::interface::DrawingContext::create(session) == 0);
    }
}

