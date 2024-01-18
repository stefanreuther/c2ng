/**
  *  \file test/game/interface/objectcommandtest.cpp
  *  \brief Test for game::interface::ObjectCommand
  */

#include "game/interface/objectcommand.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/process.hpp"
#include "interpreter/test/valueverifier.hpp"

/** Test ObjectCommand class. */
AFL_TEST("game.interface.ObjectCommand:basics", a)
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(new game::Game());
    game::map::Object& obj = *session.getGame()->currentTurn().universe().ships().create(5);

    // Test object
    game::interface::ObjectCommand testee(session, obj, game::interface::IFObjMark);
    interpreter::test::ValueVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();

    // Call it
    interpreter::Process proc(session.world(), "dummy", 1);
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    testee.call(proc, args);

    a.check("01. isMarked", obj.isMarked());
    a.check("02. isDirty", obj.isDirty());
}

/** Test IFObjMark/4. */
AFL_TEST("game.interface.ObjectCommand:IFObjMark/4", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(new game::Game());
    game::map::Object& obj = *session.getGame()->currentTurn().universe().ships().create(5);
    interpreter::Process proc(session.world(), "dummy", 1);

    // Call with no parameter
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        game::interface::IFObjMark(session, obj, proc, args);
        a.check("01. isMarked", obj.isMarked());
    }

    // Call with parameter "0"
    {
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 1);
        game::interface::IFObjMark(session, obj, proc, args);
        a.check("11. isMarked", !obj.isMarked());
    }

    // Call with parameter "1"
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, 1);
        game::interface::IFObjMark(session, obj, proc, args);
        a.check("21. isMarked", obj.isMarked());
    }
}

/** Test IFObjMark/2. */
AFL_TEST("game.interface.ObjectCommand:IFObjMark/2", a)
{
    game::map::Ship obj(77);

    // Call with no parameter
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        game::interface::IFObjMark(obj, args);
        a.check("01. isMarked", obj.isMarked());
    }

    // Call with parameter "0"
    {
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 1);
        game::interface::IFObjMark(obj, args);
        a.check("11. isMarked", !obj.isMarked());
    }

    // Call with parameter "1"
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, 1);
        game::interface::IFObjMark(obj, args);
        a.check("21. isMarked", obj.isMarked());
    }

    // Error: too many parameters
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 2);
        AFL_CHECK_THROWS(a("31. arity error"), game::interface::IFObjMark(obj, args), interpreter::Error);
    }
}

/** Test IFObjUnmark/4. */
AFL_TEST("game.interface.ObjectCommand:IFObjUnmark/4", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(new game::Game());
    game::map::Object& obj = *session.getGame()->currentTurn().universe().ships().create(5);
    interpreter::Process proc(session.world(), "dummy", 1);
    obj.setIsMarked(true);

    // Call with no parameter
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        game::interface::IFObjUnmark(session, obj, proc, args);
        a.check("01. isMarked", !obj.isMarked());
    }
}

/** Test IFObjUnmark/2. */
AFL_TEST("game.interface.ObjectCommand:IFObjUnmark/2", a)
{
    game::map::Ship obj(77);
    obj.setIsMarked(true);

    // Call with no parameter
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        game::interface::IFObjUnmark(obj, args);
        a.check("01. isMarked", !obj.isMarked());
    }

    // Error: too many parameters
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("11. arity error"), game::interface::IFObjUnmark(obj, args), interpreter::Error);
    }
}
