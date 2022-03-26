/**
  *  \file u/t_game_interface_notificationstore.cpp
  *  \brief Test for game::interface::NotificationStore
  */

#include "game/interface/notificationstore.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/interface/processlisteditor.hpp"
#include "game/playerlist.hpp"
#include "interpreter/process.hpp"
#include "interpreter/world.hpp"

/** Simple sequence test.
    This test is mostly taken from PCC2 that had more complex interaction with processes.

    A: Create two messages; one not associated with a process.
    E: Messages can be correctly retrieved, removeOrphanedMessages() works correctly. */
void
TestGameInterfaceNotificationStore::testIt()
{
    // ex IntNotifyTestSuite::testNotify
    // Environment
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    game::PlayerList list;
    interpreter::World world(log, tx, fs);

    // Create empty store
    interpreter::ProcessList procList;
    game::interface::NotificationStore store(procList);
    TS_ASSERT_EQUALS(store.getNumMessages(), 0U);

    // Add a message
    game::interface::NotificationStore::Message* msg = store.addMessage(77777, "foo\n", "bar");
    TS_ASSERT(msg != 0);
    TS_ASSERT_EQUALS(store.getNumMessages(), 1U);
    TS_ASSERT_EQUALS(store.getMessageByIndex(0), msg);
    TS_ASSERT_EQUALS(store.getMessageHeading(0, tx, list), "foo");
    TS_ASSERT_EQUALS(store.getMessageText(0, tx, list), "foo\nbar");
    TS_ASSERT_EQUALS(store.getMessageBody(msg), "bar");
    TS_ASSERT_EQUALS(store.getMessageBody(0), "");             // 0 is actually NULL

    // Add another message, associate that with a process
    interpreter::Process& proc = procList.create(world, "name");
    game::interface::NotificationStore::Message* msg2 = store.addMessage(proc.getProcessId(), "foo2\n", "bar2");
    TS_ASSERT(msg2 != 0);
    TS_ASSERT(msg2 != msg);
    TS_ASSERT_EQUALS(store.getNumMessages(), 2U);
    TS_ASSERT_EQUALS(store.getMessageByIndex(1), msg2);
    TS_ASSERT_EQUALS(store.getMessageHeading(1, tx, list), "foo2");
    TS_ASSERT_EQUALS(store.getMessageText(1, tx, list).substr(0, 10), "foo2\nbar2\n"); // text is followed by explanation of the process link
    TS_ASSERT_EQUALS(store.getMessageBody(msg2), "bar2");

    TS_ASSERT_EQUALS(store.findMessageByProcessId(proc.getProcessId()), msg2);
    TS_ASSERT(!store.findMessageByProcessId(88888));

    // Delete first message; it has no associated process
    store.removeOrphanedMessages();
    TS_ASSERT_EQUALS(store.getNumMessages(), 1U);
    TS_ASSERT_EQUALS(store.getMessageByIndex(0), msg2);
}

/** Test message header handling.

    A: Create a message that has a header in typical format.
    E: Check that header is correctly simplified */
void
TestGameInterfaceNotificationStore::testHeader()
{
    // environment
    afl::string::NullTranslator tx;
    game::PlayerList list;

    // create empty store
    interpreter::ProcessList procList;
    game::interface::NotificationStore store(procList);
    TS_ASSERT_EQUALS(store.getNumMessages(), 0U);

    // add a message
    game::interface::NotificationStore::Message* msg = store.addMessage(77777, "(-s0123)<<< Ship Message >>>\nFROM: USS Kelvin\n\n", "Hi mom.");
    TS_ASSERT(msg != 0);
    TS_ASSERT_EQUALS(store.getNumMessages(), 1U);
    TS_ASSERT_EQUALS(store.getMessageByIndex(0), msg);
    TS_ASSERT_EQUALS(store.getMessageHeading(0, tx, list), "(-s) Ship Message");
}

/** Test resumeConfirmedProcesses().

    A: Create two processes with a message each. Resume one message.
    E: One process resumed, one unchanged */
void
TestGameInterfaceNotificationStore::testResume()
{
    // Environment
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    game::PlayerList list;
    interpreter::World world(log, tx, fs);

    // Message store
    interpreter::ProcessList procList;
    game::interface::NotificationStore store(procList);

    // Two processes
    interpreter::Process& p1 = procList.create(world, "p1");
    interpreter::Process& p2 = procList.create(world, "p2");
    TS_ASSERT_EQUALS(p1.getState(), interpreter::Process::Suspended);
    TS_ASSERT_EQUALS(p2.getState(), interpreter::Process::Suspended);

    // Messages for each
    store.addMessage(p1.getProcessId(), "m1", "b");
    store.addMessage(p2.getProcessId(), "m2", "b");
    TS_ASSERT_EQUALS(store.getNumMessages(), 2U);

    // Confirm m2
    game::interface::NotificationStore::Message* msg = store.findMessageByProcessId(p2.getProcessId());
    store.confirmMessage(msg, true);
    TS_ASSERT_EQUALS(store.isMessageConfirmed(msg), true);

    // Resume
    game::interface::ProcessListEditor editor(procList);
    store.resumeConfirmedProcesses(editor);
    editor.commit(procList.allocateProcessGroup());

    // Verify
    TS_ASSERT_EQUALS(p1.getState(), interpreter::Process::Suspended);
    TS_ASSERT_EQUALS(p2.getState(), interpreter::Process::Runnable);
}

/** Test message replacement.

    A: Create two messages with same process Id.
    E: Only one message survives. */
void
TestGameInterfaceNotificationStore::testReplace()
{
    // Environment
    afl::string::NullTranslator tx;
    game::PlayerList list;

    // Create empty store
    interpreter::ProcessList procList;
    game::interface::NotificationStore store(procList);
    TS_ASSERT_EQUALS(store.getNumMessages(), 0U);

    // Add a message
    store.addMessage(77777, "h1", "b1");
    TS_ASSERT_EQUALS(store.getNumMessages(), 1U);
    TS_ASSERT_EQUALS(store.getMessageHeading(0, tx, list), "h1");

    // Add another message with the same Id
    store.addMessage(77777, "h2", "b2");
    TS_ASSERT_EQUALS(store.getNumMessages(), 1U);
    TS_ASSERT_EQUALS(store.getMessageHeading(0, tx, list), "h2");
}

