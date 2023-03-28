/**
  *  \file u/t_game_interface_objectcommand.cpp
  *  \brief Test for game::interface::ObjectCommand
  */

#include "game/interface/objectcommand.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/process.hpp"
#include "interpreter/test/valueverifier.hpp"

/** Test ObjectCommand class. */
void
TestGameInterfaceObjectCommand::testIt()
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(new game::Game());
    game::map::Object& obj = *session.getGame()->currentTurn().universe().ships().create(5);

    // Test object
    game::interface::ObjectCommand testee(session, obj, game::interface::IFObjMark);
    interpreter::test::ValueVerifier verif(testee, "testIt");
    verif.verifyBasics();
    verif.verifyNotSerializable();

    // Call it
    interpreter::Process proc(session.world(), "dummy", 1);
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    testee.call(proc, args);

    TS_ASSERT(obj.isMarked());
    TS_ASSERT(obj.isDirty());
}

/** Test IFObjMark/4. */
void
TestGameInterfaceObjectCommand::testMark4()
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
        TS_ASSERT(obj.isMarked());
    }

    // Call with parameter "0"
    {
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 1);
        game::interface::IFObjMark(session, obj, proc, args);
        TS_ASSERT(!obj.isMarked());
    }

    // Call with parameter "1"
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, 1);
        game::interface::IFObjMark(session, obj, proc, args);
        TS_ASSERT(obj.isMarked());
    }
}

/** Test IFObjMark/2. */
void
TestGameInterfaceObjectCommand::testMark2()
{
    game::map::Ship obj(77);

    // Call with no parameter
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        game::interface::IFObjMark(obj, args);
        TS_ASSERT(obj.isMarked());
    }

    // Call with parameter "0"
    {
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 1);
        game::interface::IFObjMark(obj, args);
        TS_ASSERT(!obj.isMarked());
    }

    // Call with parameter "1"
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, 1);
        game::interface::IFObjMark(obj, args);
        TS_ASSERT(obj.isMarked());
    }

    // Error: too many parameters
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFObjMark(obj, args), interpreter::Error);
    }
}

/** Test IFObjUnmark/4. */
void
TestGameInterfaceObjectCommand::testUnmark4()
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
        TS_ASSERT(!obj.isMarked());
    }
}

/** Test IFObjUnmark/2. */
void
TestGameInterfaceObjectCommand::testUnmark2()
{
    game::map::Ship obj(77);
    obj.setIsMarked(true);

    // Call with no parameter
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        game::interface::IFObjUnmark(obj, args);
        TS_ASSERT(!obj.isMarked());
    }

    // Error: too many parameters
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFObjUnmark(obj, args), interpreter::Error);
    }
}

