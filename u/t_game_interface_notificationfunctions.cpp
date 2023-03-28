/**
  *  \file u/t_game_interface_notificationfunctions.cpp
  *  \brief Test for game::interface::NotificationFunctions
  */

#include "game/interface/notificationfunctions.hpp"

#include "t_game_interface.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "game/session.hpp"
#include "interpreter/error.hpp"
#include "afl/data/segment.hpp"
#include "interpreter/values.hpp"
#include "interpreter/arguments.hpp"

/** Test NotifyConfirmedFunction. */
void
TestGameInterfaceNotificationFunctions::testNotifyConfirmed()
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    interpreter::Process proc(session.world(), "tester", 777);

    // Test object
    game::interface::NotifyConfirmedFunction testee(session);

    // Verify
    interpreter::test::ValueVerifier verif(testee, "testNotifyConfirmed");
    verif.verifyBasics();
    verif.verifyNotSerializable();
    TS_ASSERT_EQUALS(testee.isProcedureCall(), false);
    TS_ASSERT_EQUALS(testee.getDimension(0), 0);
    TS_ASSERT_THROWS(testee.makeFirstContext(), interpreter::Error);

    // Sample invocation
    afl::data::Segment seg;
    testee.call(proc, seg, true);
    TS_ASSERT_EQUALS(interpreter::getBooleanValue(proc.getResult()), 0);
}

/** Test a scenario. */
void
TestGameInterfaceNotificationFunctions::testScenario()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    interpreter::Process p1(session.world(), "p1", 777);
    interpreter::Process p2(session.world(), "p2", 778);

    // Create notifications
    // - not associated with process
    {
        // Add using CC$Notify
        afl::data::Segment seg;
        seg.pushBackString("msg");
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 2);
        game::interface::IFCCNotify(session, p1, args);
    }
    {
        // Check CCNotifyConfirmed() in both processes
        afl::data::Segment seg;
        game::interface::NotifyConfirmedFunction(session).call(p1, seg, true);
        TS_ASSERT_EQUALS(interpreter::getBooleanValue(p1.getResult()), 0);
        p1.dropValue();

        game::interface::NotifyConfirmedFunction(session).call(p2, seg, true);
        TS_ASSERT_EQUALS(interpreter::getBooleanValue(p2.getResult()), 0);
        p2.dropValue();
    }

    // - associated with process
    {
        // Add using CC$Notify
        afl::data::Segment seg;
        seg.pushBackString("msg2");
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, 2);
        game::interface::IFCCNotify(session, p1, args);
    }
    {
        // Check CCNotifyConfirmed() in both processes
        afl::data::Segment seg;
        game::interface::NotifyConfirmedFunction(session).call(p1, seg, true);
        TS_ASSERT_EQUALS(interpreter::getBooleanValue(p1.getResult()), 0);
        p1.dropValue();

        game::interface::NotifyConfirmedFunction(session).call(p2, seg, true);
        TS_ASSERT_EQUALS(interpreter::getBooleanValue(p2.getResult()), 0);
        p2.dropValue();
    }

    // Check count
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        interpreter::test::verifyNewInteger("CC$NumNotifications", game::interface::IFCCNumNotifications(session, args), 2);
    }

    // Confirm
    session.notifications().confirmMessage(session.notifications().findMessageByProcessId(777), true);
    {
        // Check CCNotifyConfirmed() in both processes
        afl::data::Segment seg;
        game::interface::NotifyConfirmedFunction(session).call(p1, seg, true);
        TS_ASSERT_EQUALS(interpreter::getBooleanValue(p1.getResult()), 1);
        p1.dropValue();

        game::interface::NotifyConfirmedFunction(session).call(p2, seg, true);
        TS_ASSERT_EQUALS(interpreter::getBooleanValue(p2.getResult()), 0);
        p2.dropValue();
    }

}

/** Test some error cases. */
void
TestGameInterfaceNotificationFunctions::testErrors()
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    interpreter::Process proc(session.world(), "tester", 777);

    // NotifyConfirmedFunction
    // - arity error
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        TS_ASSERT_THROWS(game::interface::NotifyConfirmedFunction(session).call(proc, seg, true), interpreter::Error);
    }

    // CC$NotifyFunction
    // - arity error
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFCCNotify(session, proc, args), interpreter::Error);
    }
    // - null parameters
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFCCNotify(session, proc, args));
        TS_ASSERT_EQUALS(session.notifications().getNumMessages(), 0U);
    }

    // CC$NumNotifications
    // - arity error
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFCCNumNotifications(session, args), interpreter::Error);
    }
}

