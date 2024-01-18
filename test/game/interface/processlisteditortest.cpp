/**
  *  \file test/game/interface/processlisteditortest.cpp
  *  \brief Test for game::interface::ProcessListEditor
  */

#include "game/interface/processlisteditor.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "afl/test/translator.hpp"
#include "game/interface/notificationstore.hpp"
#include "interpreter/mutexlist.hpp"
#include "interpreter/process.hpp"
#include "interpreter/processlist.hpp"
#include "interpreter/world.hpp"

using interpreter::Process;
using game::interface::ProcessListEditor;

namespace {
    struct TestHarness {
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        interpreter::World world;
        interpreter::ProcessList list;
        Process& p1;
        Process& p2;
        game::interface::NotificationStore notif;

        TestHarness()
            : log(), tx(), fs(),
              world(log, tx, fs),
              list(),
              p1(list.create(world, "p1")),
              p2(list.create(world, "p2")),
              notif(list)
            { }
    };
}

/** Test initialisation and inquiry.
    A: Use describe().
    E: Correct information delivered */
AFL_TEST("game.interface.ProcessListEditor:init", a)
{
    TestHarness h;
    ProcessListEditor t(h.list);
    afl::test::Translator tx("<", ">");

    a.checkEqual("01. getNumProcesses", t.getNumProcesses(), 2U);

    // First process
    ProcessListEditor::Info info;
    a.checkEqual("11. describe",           t.describe(0, info, h.notif, tx), true);
    a.checkEqual("12. processId",          info.processId, h.p1.getProcessId());
    a.checkEqual("13. priority",           info.priority,  h.p1.getPriority());
    a.checkEqual("14. name",               info.name,      "p1");
    a.checkEqual("15. status",             info.status,    "<Suspended>");
    a.checkEqual("16. invokingObject",     info.invokingObject.isSet(), false);
    a.checkEqual("17. isChanged",          info.isChanged, false);
    a.checkEqual("18. notificationStatus", info.notificationStatus, ProcessListEditor::NoMessage);

    // Second process
    a.checkEqual("21. describe",  t.describe(1, info, h.notif, tx), true);
    a.checkEqual("22. processId", info.processId, h.p2.getProcessId());
    a.checkEqual("23. priority",  info.priority,  h.p2.getPriority());
    a.checkEqual("24. name",      info.name,      "p2");

    // Out of range
    a.checkEqual("31. describe", t.describe(2, info, h.notif, tx), false);
}

/** Test setting process to Terminated.
    A: setProcessState(Terminated)
    E: Correct state change reported in describe(), process not yet affected */
AFL_TEST("game.interface.ProcessListEditor:setProcessState:Terminated", a)
{
    TestHarness h;
    ProcessListEditor t(h.list);
    afl::test::Translator tx("<", ">");

    t.setProcessState(h.p1.getProcessId(), ProcessListEditor::Terminated);

    ProcessListEditor::Info info;
    a.checkEqual("01. describe",  t.describe(0, info, h.notif, tx), true);
    a.checkEqual("02. status",    info.status,    "<Terminated>");
    a.checkEqual("03. isChanged", info.isChanged, true);
    a.checkEqual("04. getState",  h.p1.getState(), Process::Suspended);
}

/** Test setting process to Suspended.
    A: setProcessState(Terminated), then setProcessState(Suspended).
    E: Correct state change reported in describe(), process not yet affected */
AFL_TEST("game.interface.ProcessListEditor:setProcessState:Suspended", a)
{
    TestHarness h;
    ProcessListEditor t(h.list);
    afl::test::Translator tx("<", ">");

    t.setProcessState(h.p2.getProcessId(), ProcessListEditor::Terminated);
    t.setProcessState(h.p2.getProcessId(), ProcessListEditor::Suspended);

    ProcessListEditor::Info info;
    a.checkEqual("01. describe",  t.describe(1, info, h.notif, tx), true);
    a.checkEqual("02. status",    info.status,    "<Suspended>");
    a.checkEqual("03. isChanged", info.isChanged, false);
    a.checkEqual("04. getState",  h.p2.getState(), Process::Suspended);
}

/** Test bulk-setting to Runnable.
    A: setAllProcessState(Runnable).
    E: Correct state change reported in describe(), processes not yet affected */
AFL_TEST("game.interface.ProcessListEditor:setAllProcessState:Runnable", a)
{
    TestHarness h;
    ProcessListEditor t(h.list);
    afl::test::Translator tx("<", ">");

    t.setAllProcessState(ProcessListEditor::Runnable);

    ProcessListEditor::Info info;
    a.checkEqual("01. describe",  t.describe(0, info, h.notif, tx), true);
    a.checkEqual("02. status",    info.status,    "<Runnable>");
    a.checkEqual("03. isChanged", info.isChanged, true);
    a.checkEqual("04. getState",  h.p1.getState(), Process::Suspended);

    a.checkEqual("11. describe",  t.describe(1, info, h.notif, tx), true);
    a.checkEqual("12. status",    info.status,    "<Runnable>");
    a.checkEqual("13. isChanged", info.isChanged, true);
    a.checkEqual("14. getState",  h.p2.getState(), Process::Suspended);
}

/** Test bulk-setting to Suspended.
    A: setAllProcessState(Runnable), then setAllProcessState(Suspended).
    E: Correct state change reported in describe(), processes not yet affected */
AFL_TEST("game.interface.ProcessListEditor:setAllProcessState:Suspended", a)
{
    TestHarness h;
    ProcessListEditor t(h.list);
    afl::test::Translator tx("<", ">");

    t.setAllProcessState(ProcessListEditor::Runnable);
    t.setAllProcessState(ProcessListEditor::Suspended);

    ProcessListEditor::Info info;
    a.checkEqual("01. describe",  t.describe(0, info, h.notif, tx), true);
    a.checkEqual("02. status",    info.status,    "<Suspended>");
    a.checkEqual("03. isChanged", info.isChanged, false);
    a.checkEqual("04. getState",  h.p1.getState(), Process::Suspended);

    a.checkEqual("11. describe",  t.describe(1, info, h.notif, tx), true);
    a.checkEqual("12. status",    info.status,    "<Suspended>");
    a.checkEqual("13. isChanged", info.isChanged, false);
    a.checkEqual("14. getState",  h.p2.getState(), Process::Suspended);
}

/** Test commit.
    A: change process state, call commit().
    E: State visible on processes */
AFL_TEST("game.interface.ProcessListEditor:commit", a)
{
    TestHarness h;
    ProcessListEditor t(h.list);

    t.setProcessState(h.p1.getProcessId(), ProcessListEditor::Runnable);
    t.setProcessState(h.p2.getProcessId(), ProcessListEditor::Terminated);

    uint32_t pgid = h.list.allocateProcessGroup();
    t.commit(pgid);

    a.checkEqual("01. getState", h.p1.getState(), Process::Runnable);
    a.checkEqual("02. getState", h.p2.getState(), Process::Terminated);

    a.checkEqual("11. getProcessGroupId", h.p1.getProcessGroupId(), pgid);
}

/** Test setProcessPriority.
    A: set second process state, and set its priority to lower value.
    E: State immediately visible, list immediately re-sorted; state change still correctly applied. */
AFL_TEST("game.interface.ProcessListEditor:setPriority", a)
{
    TestHarness h;
    ProcessListEditor t(h.list);
    afl::test::Translator tx("<", ">");

    t.setProcessState(h.p2.getProcessId(), ProcessListEditor::Runnable);
    t.setProcessPriority(h.p2.getProcessId(), 10);

    // First process
    a.checkEqual("01. getPriority", h.p2.getPriority(), 10);

    ProcessListEditor::Info info;
    a.checkEqual("11. describe", t.describe(0, info, h.notif, tx), true);
    a.checkEqual("12. processId", info.processId, h.p2.getProcessId());
    a.checkEqual("13. priority",  info.priority,  10);
    a.checkEqual("14. name",      info.name,      "p2");
    a.checkEqual("15. status",    info.status,    "<Runnable>");
    a.checkEqual("16. isChanged", info.isChanged, true);

    // Second process
    a.checkEqual("21. describe", t.describe(1, info, h.notif, tx), true);
    a.checkEqual("22. processId", info.processId, h.p1.getProcessId());
    a.checkEqual("23. priority",  info.priority,  h.p1.getPriority());
    a.checkEqual("24. name",      info.name,      "p1");
    a.checkEqual("25. status",    info.status,    "<Suspended>");
    a.checkEqual("26. isChanged", info.isChanged, false);
}

/** Test notification message handling.
    A: add a notification.
    E: correct state reported. */
AFL_TEST("game.interface.ProcessListEditor:notification:unread", a)
{
    TestHarness h;
    ProcessListEditor t(h.list);
    afl::test::Translator tx("<", ">");

    h.notif.addMessage(h.p1.getProcessId(), "header", "body", game::Reference());

    ProcessListEditor::Info info;
    a.checkEqual("01. describe", t.describe(0, info, h.notif, tx), true);
    a.checkEqual("02. notificationStatus", info.notificationStatus, ProcessListEditor::UnreadMessage);
}

/** Test notification message handling, confirmed (read) notification.
    A: add a notification and confirm it.
    E: correct state reported. */
AFL_TEST("game.interface.ProcessListEditor:notification:confirmed", a)
{
    TestHarness h;
    ProcessListEditor t(h.list);
    afl::test::Translator tx("<", ">");

    h.notif.confirmMessage(h.notif.addMessage(h.p1.getProcessId(), "header", "body", game::Reference()), true);

    ProcessListEditor::Info info;
    a.checkEqual("01. describe", t.describe(0, info, h.notif, tx), true);
    a.checkEqual("02. notificationStatus", info.notificationStatus, ProcessListEditor::ConfirmedMessage);
}
