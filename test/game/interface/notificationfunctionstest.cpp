/**
  *  \file test/game/interface/notificationfunctionstest.cpp
  *  \brief Test for game::interface::NotificationFunctions
  */

#include "game/interface/notificationfunctions.hpp"

#include "afl/data/segment.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "interpreter/values.hpp"

/** Test NotifyConfirmedFunction. */
AFL_TEST("game.interface.NotificationFunctions:NotifyConfirmedFunction", a)
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    interpreter::Process proc(session.world(), "tester", 777);

    // Test object
    game::interface::NotifyConfirmedFunction testee(session);

    // Verify
    interpreter::test::ValueVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();
    a.checkEqual("01. isProcedureCall", testee.isProcedureCall(), false);
    a.checkEqual("02. getDimension", testee.getDimension(0), 0);
    AFL_CHECK_THROWS(a("03. makeFirstContext"), testee.makeFirstContext(), interpreter::Error);

    // Sample invocation
    afl::data::Segment seg;
    testee.call(proc, seg, true);
    a.checkEqual("11. call", interpreter::getBooleanValue(proc.getResult()), 0);
}

/** Test a scenario. */
AFL_TEST("game.interface.NotificationFunctions:scenario", a)
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
        a.checkEqual("01. getResult", interpreter::getBooleanValue(p1.getResult()), 0);
        p1.dropValue();

        game::interface::NotifyConfirmedFunction(session).call(p2, seg, true);
        a.checkEqual("11. getResult", interpreter::getBooleanValue(p2.getResult()), 0);
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
        a.checkEqual("21. getResult", interpreter::getBooleanValue(p1.getResult()), 0);
        p1.dropValue();

        game::interface::NotifyConfirmedFunction(session).call(p2, seg, true);
        a.checkEqual("31. getResult", interpreter::getBooleanValue(p2.getResult()), 0);
        p2.dropValue();
    }

    // Check count
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        interpreter::test::verifyNewInteger(a("CC$NumNotifications"), game::interface::IFCCNumNotifications(session, args), 2);
    }

    // Confirm
    session.notifications().confirmMessage(session.notifications().findMessageByProcessId(777), true);
    {
        // Check CCNotifyConfirmed() in both processes
        afl::data::Segment seg;
        game::interface::NotifyConfirmedFunction(session).call(p1, seg, true);
        a.checkEqual("41. getResult", interpreter::getBooleanValue(p1.getResult()), 1);
        p1.dropValue();

        game::interface::NotifyConfirmedFunction(session).call(p2, seg, true);
        a.checkEqual("51. getResult", interpreter::getBooleanValue(p2.getResult()), 0);
        p2.dropValue();
    }
}

/** Test some error cases. */
AFL_TEST("game.interface.NotificationFunctions:error-cases", a)
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
        AFL_CHECK_THROWS(a("01. NotifyConfirmedFunction arity error"), game::interface::NotifyConfirmedFunction(session).call(proc, seg, true), interpreter::Error);
    }

    // CC$NotifyFunction
    // - arity error
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        AFL_CHECK_THROWS(a("11. IFCCNotify arity error"), game::interface::IFCCNotify(session, proc, args), interpreter::Error);
    }
    // - null parameters
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, 2);
        AFL_CHECK_SUCCEEDS(a("12. IFCCNotify"), game::interface::IFCCNotify(session, proc, args));
        a.checkEqual("13. getNumMessages", session.notifications().getNumMessages(), 0U);
    }

    // CC$NumNotifications
    // - arity error
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("21. IFCCNumNotifications arity error"), game::interface::IFCCNumNotifications(session, args), interpreter::Error);
    }
}
