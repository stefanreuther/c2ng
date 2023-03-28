/**
  *  \file u/t_game_interface_ufocontext.cpp
  *  \brief Test for game::interface::UfoContext
  */

#include "game/interface/ufocontext.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/map/configuration.hpp"
#include "game/session.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/error.hpp"
#include "interpreter/process.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "interpreter/callablevalue.hpp"

/** Verify types. */
void
TestGameInterfaceUfoContext::testTypes()
{
    // Create a session with a turn
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(new game::Game());
    game::Turn& turn = session.getGame()->currentTurn();

    // Add an Ufo
    game::map::Ufo* ufo = turn.universe().ufos().addUfo(51, 1, 2);
    TS_ASSERT(ufo != 0);
    ufo->setWarpFactor(2);
    ufo->setHeading(135);
    ufo->setPlanetRange(200);
    ufo->setShipRange(150);
    ufo->setInfo1("USS Rosswell");
    ufo->setInfo2("New Mexico");
    ufo->postprocess(42, session.getGame()->mapConfiguration());

    TS_ASSERT_EQUALS(turn.universe().ufos().getObjectByIndex(1), ufo);

    // Create a context
    game::interface::UfoContext testee(1, turn, session);
    interpreter::test::ContextVerifier v(testee, "testTypes");
    v.verifyTypes();
    v.verifyBasics();
    v.verifyNotSerializable();
    TS_ASSERT_EQUALS(testee.getObject(), ufo);

    // Verify some values
    v.verifyInteger("ID", 51);
    v.verifyInteger("HEADING$", 135);
    v.verifyString("HEADING", "SE");
    v.verifyString("INFO1", "USS Rosswell");
    v.verifyInteger("COLOR.EGA", 2);
    v.verifyInteger("COLOR", 12);

    // Verify set
    TS_ASSERT(!ufo->isStoredInHistory());
    v.setIntegerValue("KEEP", 1);
    TS_ASSERT(ufo->isStoredInHistory());

    // Verify inability to set
    TS_ASSERT_THROWS(v.setIntegerValue("MARK", 1), interpreter::Error);
}

/** Test iteration. */
void
TestGameInterfaceUfoContext::testIteration()
{
    // Create a session with a turn
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(new game::Game());
    game::Turn& turn = session.getGame()->currentTurn();

    // Add some Ufos
    game::map::Ufo* ufo = turn.universe().ufos().addUfo(51, 1, 2);
    ufo->setColorCode(10);
    TS_ASSERT(ufo->isValid());

    game::map::Ufo* ufo2 = turn.universe().ufos().addUfo(77, 1, 2);
    ufo2->setColorCode(10);
    TS_ASSERT(ufo2->isValid());

    TS_ASSERT_EQUALS(turn.universe().ufos().getObjectByIndex(1), ufo);
    TS_ASSERT_EQUALS(turn.universe().ufos().getObjectByIndex(2), ufo2);

    // Verify
    game::interface::UfoContext testee(1, turn, session);
    interpreter::test::ContextVerifier v(testee, "testIteration");
    v.verifyInteger("ID", 51);
    TS_ASSERT_EQUALS(testee.getObject(), ufo);

    TS_ASSERT(testee.next());
    v.verifyInteger("ID", 77);
    TS_ASSERT_EQUALS(testee.getObject(), ufo2);

    TS_ASSERT(!testee.next());
}

/** Test handling of empty (invalid) Ufo. */
void
TestGameInterfaceUfoContext::testEmpty()
{
    // Create a session with no Ufo
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(new game::Game());

    // Create an Ufo context
    game::interface::UfoContext testee(1, session.getGame()->currentTurn(), session);
    interpreter::test::ContextVerifier v(testee, "testEmpty");

    // Values are empty
    v.verifyNull("ID");
    v.verifyNull("MARK");

    // No object
    TS_ASSERT(testee.getObject() == 0);

    // Not assignable
    TS_ASSERT_THROWS(v.setIntegerValue("KEEP", 1), interpreter::Error);

    // No next
    TS_ASSERT(!testee.next());
}

/** Test command execution. */
void
TestGameInterfaceUfoContext::testCommands()
{
    // Create a session with a turn
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(new game::Game());
    game::Turn& turn = session.getGame()->currentTurn();

    // Add an Ufo
    game::map::Ufo* ufo = turn.universe().ufos().addUfo(51, 1, 2);
    TS_ASSERT(ufo != 0);

    // Create a context
    game::interface::UfoContext testee(1, turn, session);
    std::auto_ptr<afl::data::Value> meth(interpreter::test::ContextVerifier(testee, "testCommands").getValue("MARK"));

    // Invoke as command
    interpreter::CallableValue* cv = dynamic_cast<interpreter::CallableValue*>(meth.get());
    TS_ASSERT(cv != 0);
    interpreter::test::ValueVerifier(*cv, "testCommands").verifyBasics();
    {
        afl::data::Segment seg;
        interpreter::Process proc(session.world(), "dummy", 1);
        TS_ASSERT_THROWS_NOTHING(cv->call(proc, seg, false));
    }

    // Verify that command was executed
    TS_ASSERT(ufo->isMarked());
}

