/**
  *  \file u/t_game_interface_friendlycodefunction.cpp
  *  \brief Test for game::interface::FriendlyCodeFunction
  */

#include "game/interface/friendlycodefunction.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

/** Test general behaviour. */
void
TestGameInterfaceFriendlyCodeFunction::testIt()
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new game::spec::ShipList());
    session.getShipList()->friendlyCodes().addCode(game::spec::FriendlyCode("abc", ",one", session.translator()));
    session.getShipList()->friendlyCodes().addCode(game::spec::FriendlyCode("xyz", ",two", session.translator()));

    // Test basic properties
    game::interface::FriendlyCodeFunction testee(session);
    interpreter::test::ValueVerifier verif(testee, "testIt");
    verif.verifyBasics();
    verif.verifyNotSerializable();

    TS_ASSERT_EQUALS(testee.getDimension(0), 0);

    // Test successful invocation
    {
        afl::data::Segment seg;
        seg.pushBackString("xyz");
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        TS_ASSERT(result.get() != 0);
        interpreter::test::ContextVerifier(*result, "get xyz").verifyString("DESCRIPTION", "two");
    }

    // Invocation with null
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        TS_ASSERT(result.get() == 0);
    }

    // Invocation with unknown value
    {
        afl::data::Segment seg;
        seg.pushBackString("pqr");
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        TS_ASSERT(result.get() == 0);
    }

    // Test failing invocation: arity error
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(testee.get(args), interpreter::Error);
    }

    // Cannot assign 'FriendlyCode("xyz") := ...'
    {
        afl::data::Segment seg;
        seg.pushBackString("xyz");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(testee.set(args, 0), interpreter::Error);
    }

    // Iteration
    {
        std::auto_ptr<interpreter::Context> p(testee.makeFirstContext());
        TS_ASSERT(p.get() != 0);
        interpreter::test::ContextVerifier(*p, "get first").verifyString("DESCRIPTION", "one");
    }
}

/** Test behaviour on missing environment objects. */
void
TestGameInterfaceFriendlyCodeFunction::testEmpty()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;

    // No root
    {
        game::Session session(tx, fs);
        session.setShipList(new game::spec::ShipList());
        session.getShipList()->friendlyCodes().addCode(game::spec::FriendlyCode("abc", ",one", session.translator()));

        game::interface::FriendlyCodeFunction testee(session);

        // Invocation
        afl::data::Segment seg;
        seg.pushBackString("xyz");
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        TS_ASSERT(result.get() == 0);

        // Iteration
        result.reset(testee.makeFirstContext());
        TS_ASSERT(result.get() == 0);
    }

    // No ship list
    {
        game::Session session(tx, fs);
        session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());

        game::interface::FriendlyCodeFunction testee(session);

        // Invocation
        afl::data::Segment seg;
        seg.pushBackString("xyz");
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        TS_ASSERT(result.get() == 0);

        // Iteration
        result.reset(testee.makeFirstContext());
        TS_ASSERT(result.get() == 0);
    }
}

