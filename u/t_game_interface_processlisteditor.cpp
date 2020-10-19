/**
  *  \file u/t_game_interface_processlisteditor.cpp
  *  \brief Test for game::interface::ProcessListEditor
  */

#include "game/interface/processlisteditor.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/sys/log.hpp"
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
        afl::io::NullFileSystem fs;
        interpreter::World world;
        interpreter::ProcessList list;
        Process& p1;
        Process& p2;
        game::interface::NotificationStore notif;

        TestHarness()
            : log(), fs(),
              world(log, fs),
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
void
TestGameInterfaceProcessListEditor::testInit()
{
    TestHarness h;
    ProcessListEditor t(h.list);
    afl::test::Translator tx("<", ">");

    TS_ASSERT_EQUALS(t.getNumProcesses(), 2U);

    // First process
    ProcessListEditor::Info info;
    TS_ASSERT_EQUALS(t.describe(0, info, h.notif, tx), true);
    TS_ASSERT_EQUALS(info.processId, h.p1.getProcessId());
    TS_ASSERT_EQUALS(info.priority,  h.p1.getPriority());
    TS_ASSERT_EQUALS(info.name,      "p1");
    TS_ASSERT_EQUALS(info.status,    "<Suspended>");
    TS_ASSERT_EQUALS(info.invokingObject.isSet(), false);
    TS_ASSERT_EQUALS(info.isChanged, false);
    TS_ASSERT_EQUALS(info.notificationStatus, ProcessListEditor::NoMessage);

    // Second process
    TS_ASSERT_EQUALS(t.describe(1, info, h.notif, tx), true);
    TS_ASSERT_EQUALS(info.processId, h.p2.getProcessId());
    TS_ASSERT_EQUALS(info.priority,  h.p2.getPriority());
    TS_ASSERT_EQUALS(info.name,      "p2");

    // Out of range
    TS_ASSERT_EQUALS(t.describe(2, info, h.notif, tx), false);
}

/** Test setting process to Terminated.
    A: setProcessState(Terminated)
    E: Correct state change reported in describe(), process not yet affected */
void
TestGameInterfaceProcessListEditor::testSetOneTerminated()
{
    TestHarness h;
    ProcessListEditor t(h.list);
    afl::test::Translator tx("<", ">");

    t.setProcessState(h.p1.getProcessId(), ProcessListEditor::Terminated);

    ProcessListEditor::Info info;
    TS_ASSERT_EQUALS(t.describe(0, info, h.notif, tx), true);
    TS_ASSERT_EQUALS(info.status,    "<Terminated>");
    TS_ASSERT_EQUALS(info.isChanged, true);
    TS_ASSERT_EQUALS(h.p1.getState(), Process::Suspended);
}

/** Test setting process to Suspended.
    A: setProcessState(Terminated), then setProcessState(Suspended).
    E: Correct state change reported in describe(), process not yet affected */
void
TestGameInterfaceProcessListEditor::testSetOneSuspended()
{
    TestHarness h;
    ProcessListEditor t(h.list);
    afl::test::Translator tx("<", ">");

    t.setProcessState(h.p2.getProcessId(), ProcessListEditor::Terminated);
    t.setProcessState(h.p2.getProcessId(), ProcessListEditor::Suspended);

    ProcessListEditor::Info info;
    TS_ASSERT_EQUALS(t.describe(1, info, h.notif, tx), true);
    TS_ASSERT_EQUALS(info.status,    "<Suspended>");
    TS_ASSERT_EQUALS(info.isChanged, false);
    TS_ASSERT_EQUALS(h.p2.getState(), Process::Suspended);
}

/** Test bulk-setting to Runnable.
    A: setAllProcessState(Runnable).
    E: Correct state change reported in describe(), processes not yet affected */
void
TestGameInterfaceProcessListEditor::testSetAllRunnable()
{
    TestHarness h;
    ProcessListEditor t(h.list);
    afl::test::Translator tx("<", ">");

    t.setAllProcessState(ProcessListEditor::Runnable);

    ProcessListEditor::Info info;
    TS_ASSERT_EQUALS(t.describe(0, info, h.notif, tx), true);
    TS_ASSERT_EQUALS(info.status,    "<Runnable>");
    TS_ASSERT_EQUALS(info.isChanged, true);
    TS_ASSERT_EQUALS(h.p1.getState(), Process::Suspended);

    TS_ASSERT_EQUALS(t.describe(1, info, h.notif, tx), true);
    TS_ASSERT_EQUALS(info.status,    "<Runnable>");
    TS_ASSERT_EQUALS(info.isChanged, true);
    TS_ASSERT_EQUALS(h.p2.getState(), Process::Suspended);
}

/** Test bulk-setting to Suspended.
    A: setAllProcessState(Runnable), then setAllProcessState(Suspended).
    E: Correct state change reported in describe(), processes not yet affected */
void
TestGameInterfaceProcessListEditor::testSetAllSuspended()
{
    TestHarness h;
    ProcessListEditor t(h.list);
    afl::test::Translator tx("<", ">");

    t.setAllProcessState(ProcessListEditor::Runnable);
    t.setAllProcessState(ProcessListEditor::Suspended);

    ProcessListEditor::Info info;
    TS_ASSERT_EQUALS(t.describe(0, info, h.notif, tx), true);
    TS_ASSERT_EQUALS(info.status,    "<Suspended>");
    TS_ASSERT_EQUALS(info.isChanged, false);
    TS_ASSERT_EQUALS(h.p1.getState(), Process::Suspended);

    TS_ASSERT_EQUALS(t.describe(1, info, h.notif, tx), true);
    TS_ASSERT_EQUALS(info.status,    "<Suspended>");
    TS_ASSERT_EQUALS(info.isChanged, false);
    TS_ASSERT_EQUALS(h.p2.getState(), Process::Suspended);
}

/** Test commit.
    A: change process state, call commit().
    E: State visible on processes */
void
TestGameInterfaceProcessListEditor::testCommit()
{
    TestHarness h;
    ProcessListEditor t(h.list);

    t.setProcessState(h.p1.getProcessId(), ProcessListEditor::Runnable);
    t.setProcessState(h.p2.getProcessId(), ProcessListEditor::Terminated);

    uint32_t pgid = h.list.allocateProcessGroup();
    t.commit(pgid);

    TS_ASSERT_EQUALS(h.p1.getState(), Process::Runnable);
    TS_ASSERT_EQUALS(h.p2.getState(), Process::Terminated);

    TS_ASSERT_EQUALS(h.p1.getProcessGroupId(), pgid);
}

/** Test setProcessPriority.
    A: set second process state, and set its priority to lower value.
    E: State immediately visible, list immediately re-sorted; state change still correctly applied. */
void
TestGameInterfaceProcessListEditor::testSetPriority()
{
    TestHarness h;
    ProcessListEditor t(h.list);
    afl::test::Translator tx("<", ">");

    t.setProcessState(h.p2.getProcessId(), ProcessListEditor::Runnable);
    t.setProcessPriority(h.p2.getProcessId(), 10);

    // First process
    TS_ASSERT_EQUALS(h.p2.getPriority(), 10);

    ProcessListEditor::Info info;
    TS_ASSERT_EQUALS(t.describe(0, info, h.notif, tx), true);
    TS_ASSERT_EQUALS(info.processId, h.p2.getProcessId());
    TS_ASSERT_EQUALS(info.priority,  10);
    TS_ASSERT_EQUALS(info.name,      "p2");
    TS_ASSERT_EQUALS(info.status,    "<Runnable>");
    TS_ASSERT_EQUALS(info.isChanged, true);

    // Second process
    TS_ASSERT_EQUALS(t.describe(1, info, h.notif, tx), true);
    TS_ASSERT_EQUALS(info.processId, h.p1.getProcessId());
    TS_ASSERT_EQUALS(info.priority,  h.p1.getPriority());
    TS_ASSERT_EQUALS(info.name,      "p1");
    TS_ASSERT_EQUALS(info.status,    "<Suspended>");
    TS_ASSERT_EQUALS(info.isChanged, false);
}

/** Test notification message handling.
    A: add a notification.
    E: correct state reported. */
void
TestGameInterfaceProcessListEditor::testNotification()
{
    TestHarness h;
    ProcessListEditor t(h.list);
    afl::test::Translator tx("<", ">");

    h.notif.addMessage(h.p1.getProcessId(), "header", "body");

    ProcessListEditor::Info info;
    TS_ASSERT_EQUALS(t.describe(0, info, h.notif, tx), true);
    TS_ASSERT_EQUALS(info.notificationStatus, ProcessListEditor::UnreadMessage);
}

/** Test notification message handling, confirmed (read) notification.
    A: add a notification and confirm it.
    E: correct state reported. */
void
TestGameInterfaceProcessListEditor::testReadNotification()
{
    TestHarness h;
    ProcessListEditor t(h.list);
    afl::test::Translator tx("<", ">");

    h.notif.confirmMessage(h.notif.addMessage(h.p1.getProcessId(), "header", "body"), true);

    ProcessListEditor::Info info;
    TS_ASSERT_EQUALS(t.describe(0, info, h.notif, tx), true);
    TS_ASSERT_EQUALS(info.notificationStatus, ProcessListEditor::ConfirmedMessage);
}

