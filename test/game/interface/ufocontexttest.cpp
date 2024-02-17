/**
  *  \file test/game/interface/ufocontexttest.cpp
  *  \brief Test for game::interface::UfoContext
  */

#include "game/interface/ufocontext.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/process.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

/** Verify types. */
AFL_TEST("game.interface.UfoContext:basics", a)
{
    // Create a turn
    afl::string::NullTranslator tx;
    game::map::Configuration mapConfig;
    afl::base::Ref<game::Turn> turn(*new game::Turn());

    // Add an Ufo
    game::map::Ufo* ufo = turn->universe().ufos().addUfo(51, 1, 2);
    a.checkNonNull("01. ufo", ufo);
    ufo->setWarpFactor(2);
    ufo->setHeading(135);
    ufo->setPlanetRange(200);
    ufo->setShipRange(150);
    ufo->setInfo1("USS Rosswell");
    ufo->setInfo2("New Mexico");
    ufo->postprocess(42, mapConfig);

    a.checkEqual("11. getObjectByIndex", turn->universe().ufos().getObjectByIndex(1), ufo);

    // Create a context
    game::interface::UfoContext testee(1, turn, tx);
    interpreter::test::ContextVerifier v(testee, a);
    v.verifyTypes();
    v.verifyBasics();
    v.verifyNotSerializable();
    a.checkEqual("21. getObject", testee.getObject(), ufo);

    // Verify some values
    v.verifyInteger("ID", 51);
    v.verifyInteger("HEADING$", 135);
    v.verifyString("HEADING", "SE");
    v.verifyString("INFO1", "USS Rosswell");
    v.verifyInteger("COLOR.EGA", 2);
    v.verifyInteger("COLOR", 12);

    // Verify set
    a.check("31. isStoredInHistory", !ufo->isStoredInHistory());
    v.setIntegerValue("KEEP", 1);
    a.check("32. isStoredInHistory", ufo->isStoredInHistory());

    // Verify inability to set
    AFL_CHECK_THROWS(a("41. set MARK"), v.setIntegerValue("MARK", 1), interpreter::Error);
}

/** Test iteration. */
AFL_TEST("game.interface.UfoContext:iteration", a)
{
    // Create a turn
    afl::string::NullTranslator tx;
    afl::base::Ref<game::Turn> turn(*new game::Turn());

    // Add some Ufos
    game::map::Ufo* ufo = turn->universe().ufos().addUfo(51, 1, 2);
    ufo->setColorCode(10);
    a.check("01. isValid", ufo->isValid());

    game::map::Ufo* ufo2 = turn->universe().ufos().addUfo(77, 1, 2);
    ufo2->setColorCode(10);
    a.check("11. isValid", ufo2->isValid());

    a.checkEqual("21. getObjectByIndex", turn->universe().ufos().getObjectByIndex(1), ufo);
    a.checkEqual("22. getObjectByIndex", turn->universe().ufos().getObjectByIndex(2), ufo2);

    // Verify
    game::interface::UfoContext testee(1, turn, tx);
    interpreter::test::ContextVerifier v(testee, a);
    v.verifyInteger("ID", 51);
    a.checkEqual("31. getObject", testee.getObject(), ufo);

    a.check("41. next", testee.next());
    v.verifyInteger("ID", 77);
    a.checkEqual("42. getObject", testee.getObject(), ufo2);

    a.check("51. next", !testee.next());
}

/** Test handling of empty (invalid) Ufo. */
AFL_TEST("game.interface.UfoContext:empty", a)
{
    // Create a turn with no Ufo
    afl::string::NullTranslator tx;
    afl::base::Ref<game::Turn> turn(*new game::Turn());

    // Create an Ufo context
    game::interface::UfoContext testee(1, turn, tx);
    interpreter::test::ContextVerifier v(testee, a);

    // Values are empty
    v.verifyNull("ID");
    v.verifyNull("MARK");

    // No object
    a.checkNull("01. getObject", testee.getObject());

    // Not assignable
    AFL_CHECK_THROWS(a("11. set KEEP"), v.setIntegerValue("KEEP", 1), interpreter::Error);

    // No next
    a.check("21. next", !testee.next());
}

/** Test command execution. */
AFL_TEST("game.interface.UfoContext:commands", a)
{
    // Create a turn
    afl::string::NullTranslator tx;
    afl::base::Ref<game::Turn> turn(*new game::Turn());

    // Add an Ufo
    game::map::Ufo* ufo = turn->universe().ufos().addUfo(51, 1, 2);
    a.checkNonNull("01. ufo", ufo);

    // Create a context
    game::interface::UfoContext testee(1, turn, tx);
    std::auto_ptr<afl::data::Value> meth(interpreter::test::ContextVerifier(testee, a).getValue("MARK"));

    // Invoke as command
    interpreter::CallableValue* cv = dynamic_cast<interpreter::CallableValue*>(meth.get());
    a.checkNonNull("11. CallableValue", cv);
    interpreter::test::ValueVerifier(*cv, a("MARK")).verifyBasics();
    {
        afl::io::NullFileSystem fs;
        game::Session session(tx, fs);
        afl::data::Segment seg;
        interpreter::Process proc(session.world(), "dummy", 1);
        AFL_CHECK_SUCCEEDS(a("12. run"), cv->call(proc, seg, false));
    }

    // Verify that command was executed
    a.check("21. isMarked", ufo->isMarked());
}
