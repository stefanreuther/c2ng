/**
  *  \file test/game/interface/notificationstoretest.cpp
  *  \brief Test for game::interface::NotificationStore
  */

#include "game/interface/notificationstore.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/interface/processlisteditor.hpp"
#include "game/parser/informationconsumer.hpp"
#include "game/playerlist.hpp"
#include "game/teamsettings.hpp"
#include "interpreter/process.hpp"
#include "interpreter/world.hpp"

using game::Reference;

namespace {
    class NullInformationConsumer : public game::parser::InformationConsumer {
     public:
        virtual void addMessageInformation(const game::parser::MessageInformation& /*info*/)
            { }
    };
}


/** Simple sequence test.
    This test is mostly taken from PCC2 that had more complex interaction with processes.

    A: Create two messages; one not associated with a process.
    E: Messages can be correctly retrieved, removeOrphanedMessages() works correctly. */
AFL_TEST("game.interface.NotificationStore:sequence", a)
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
    a.checkEqual("01. getNumMessages", store.getNumMessages(), 0U);

    // Out-of-bounds access correctly rejected
    a.checkNull("11. getMessageByIndex", store.getMessageByIndex(0));
    a.checkEqual("12. getMessageHeaderText", store.getMessageHeaderText(0, tx, list), "");
    a.checkEqual("13. getMessageBodyText", store.getMessageBodyText(0, tx, list), "");
    a.checkEqual("14. getMessageDisplayText", store.getMessageDisplayText(0, tx, list).getText(), "");
    {
        game::TeamSettings teams;
        afl::charset::Utf8Charset cs;
        NullInformationConsumer consumer;
        AFL_CHECK_SUCCEEDS(a("15. receiveMessageData"), store.receiveMessageData(0, consumer, teams, true, cs));
    }

    // Add a message
    game::interface::NotificationStore::Message* msg = store.addMessage(77777, "foo\n", "bar", Reference(Reference::Ship, 77));
    a.checkNonNull("21. msg", msg);
    a.checkEqual("22. getNumMessages",        store.getNumMessages(), 1U);
    a.checkEqual("23. getMessageByIndex",     store.getMessageByIndex(0), msg);
    a.checkEqual("24. getMessageHeading",     store.getMessageHeading(0, tx, list), "foo");
    a.checkEqual("25. getMessageText",        store.getMessageText(0, tx, list), "foo\nbar");
    a.checkEqual("26. getMessageBody",        store.getMessageBody(msg), "bar");
    a.checkEqual("27. getMessageBody",        store.getMessageBody(0), "");             // 0 is actually NULL
    a.checkEqual("28. primaryLink",           store.getMessageMetadata(0, tx, list).primaryLink, Reference(Reference::Ship, 77));
    a.checkEqual("29. getMessageReplyText",   store.getMessageReplyText(0, tx, list), "> foo\n> bar\n");
    a.checkEqual("30. getMessageForwardText", store.getMessageForwardText(0, tx, list), "--- Forwarded Message ---\nfoo\nbar\n--- End Forwarded Message ---");
    a.checkEqual("31. getMessageDisplayText", store.getMessageDisplayText(0, tx, list).getText(), "foo\nbar");

    // Add another message, associate that with a process
    interpreter::Process& proc = procList.create(world, "name");
    game::interface::NotificationStore::Message* msg2 = store.addMessage(proc.getProcessId(), "foo2\n", "bar2", Reference());
    a.checkNonNull("41. msg2", msg2);
    a.checkDifferent("42. msg", msg2, msg);
    a.checkEqual("43. getNumMessages",        store.getNumMessages(), 2U);
    a.checkEqual("44. getMessageByIndex",     store.getMessageByIndex(1), msg2);
    a.checkEqual("45. getMessageHeading",     store.getMessageHeading(1, tx, list), "foo2");
    a.checkEqual("46. getMessageText",        store.getMessageText(1, tx, list), "foo2\nbar2");
    a.checkEqual("47. getMessageBody",        store.getMessageBody(msg2), "bar2");
    a.checkEqual("48. getMessageDisplayText", store.getMessageDisplayText(1, tx, list).getText().substr(0, 9), "foo2\nbar2");
    a.check("49. getMessageDisplayText",      store.getMessageDisplayText(1, tx, list).getText().find("has been stopped") != String_t::npos);

    a.checkEqual("51. findMessageByProcessId", store.findMessageByProcessId(proc.getProcessId()), msg2);
    a.check("52. findMessageByProcessId", !store.findMessageByProcessId(88888));

    a.checkEqual("61. findMessageByProcessId", store.findIndexByProcessId(proc.getProcessId()).orElse(9999), 1U);
    a.check("62. findIndexByProcessId", !store.findIndexByProcessId(8888).isValid());

    // Delete first message; it has no associated process
    store.removeOrphanedMessages();
    a.checkEqual("71. getNumMessages",    store.getNumMessages(), 1U);
    a.checkEqual("72. getMessageByIndex", store.getMessageByIndex(0), msg2);
}

/** Test message header handling.
    A: Create a message that has a header in typical format.
    E: Check that header is correctly simplified */
AFL_TEST("game.interface.NotificationStore:header", a)
{
    // environment
    afl::string::NullTranslator tx;
    game::PlayerList list;

    // create empty store
    interpreter::ProcessList procList;
    game::interface::NotificationStore store(procList);
    a.checkEqual("01. getNumMessages", store.getNumMessages(), 0U);

    // add a message
    game::interface::NotificationStore::Message* msg = store.addMessage(77777, "(-s0123)<<< Ship Message >>>\nFROM: USS Kelvin\n\n", "Hi mom.", Reference(Reference::Ship, 123));
    a.checkNonNull("11. msg", msg);
    a.checkEqual("12. getNumMessages", store.getNumMessages(), 1U);
    a.checkEqual("13. getMessageByIndex", store.getMessageByIndex(0), msg);
    a.checkEqual("14. getMessageHeading", store.getMessageHeading(0, tx, list), "(-s) Ship Message");
    a.checkEqual("15. primaryLink", store.getMessageMetadata(0, tx, list).primaryLink, Reference(Reference::Ship, 123));
}

/** Test resumeConfirmedProcesses().
    A: Create two processes with a message each. Resume one message.
    E: One process resumed, one unchanged */
AFL_TEST("game.interface.NotificationStore:resume", a)
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
    a.checkEqual("01. getState", p1.getState(), interpreter::Process::Suspended);
    a.checkEqual("02. getState", p2.getState(), interpreter::Process::Suspended);

    // Messages for each
    store.addMessage(p1.getProcessId(), "m1", "b", Reference());
    store.addMessage(p2.getProcessId(), "m2", "b", Reference());
    a.checkEqual("11. getNumMessages", store.getNumMessages(), 2U);

    // Confirm m2
    game::interface::NotificationStore::Message* msg = store.findMessageByProcessId(p2.getProcessId());
    store.confirmMessage(msg, true);
    a.checkEqual("21. isMessageConfirmed", store.isMessageConfirmed(msg), true);

    // Resume
    game::interface::ProcessListEditor editor(procList);
    store.resumeConfirmedProcesses(editor);
    editor.commit(procList.allocateProcessGroup());

    // Verify
    a.checkEqual("31. getState", p1.getState(), interpreter::Process::Suspended);
    a.checkEqual("32. getState", p2.getState(), interpreter::Process::Runnable);
}

/** Test resumeConfirmedProcesses(), use general API.
    A: Create two processes with a message each. Resume one message.
    E: One process resumed, one unchanged */
AFL_TEST("game.interface.NotificationStore:resume:performMessageAction", a)
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
    a.checkEqual("01. getState", p1.getState(), interpreter::Process::Suspended);
    a.checkEqual("02. getState", p2.getState(), interpreter::Process::Suspended);

    // Messages for each
    store.addMessage(p1.getProcessId(), "m1", "b", Reference());
    store.addMessage(p2.getProcessId(), "m2", "b", Reference());
    a.checkEqual("11. getNumMessages", store.getNumMessages(), 2U);

    a.check("21. flags", !store.getMessageMetadata(1, tx, list).flags.contains(game::msg::Mailbox::Confirmed));
    a.check("22. actions", store.getMessageActions(1).contains(game::msg::Mailbox::ToggleConfirmed));

    // Confirm m2 using general API
    size_t index = store.findIndexByProcessId(p2.getProcessId()).orElse(9999);
    a.checkEqual("31. index", index, 1U);
    store.performMessageAction(index, game::msg::Mailbox::ToggleConfirmed);
    a.check("32. flags", store.getMessageMetadata(1, tx, list).flags.contains(game::msg::Mailbox::Confirmed));
    a.check("33. actions", !store.getMessageActions(1).contains(game::msg::Mailbox::ToggleConfirmed));

    // Resume
    game::interface::ProcessListEditor editor(procList);
    store.resumeConfirmedProcesses(editor);
    editor.commit(procList.allocateProcessGroup());

    // Verify
    a.checkEqual("41. getState", p1.getState(), interpreter::Process::Suspended);
    a.checkEqual("42. getState", p2.getState(), interpreter::Process::Runnable);
}

/** Test message replacement.

    A: Create two messages with same process Id.
    E: Only one message survives. */
AFL_TEST("game.interface.NotificationStore:replace", a)
{
    // Environment
    afl::string::NullTranslator tx;
    game::PlayerList list;

    // Create empty store
    interpreter::ProcessList procList;
    game::interface::NotificationStore store(procList);
    a.checkEqual("01. getNumMessages", store.getNumMessages(), 0U);

    // Add a message
    store.addMessage(77777, "h1", "b1", Reference());
    a.checkEqual("11. getNumMessages", store.getNumMessages(), 1U);
    a.checkEqual("12. getMessageHeading", store.getMessageHeading(0, tx, list), "h1");

    // Add another message with the same Id
    store.addMessage(77777, "h2", "b2", Reference());
    a.checkEqual("21. getNumMessages", store.getNumMessages(), 1U);
    a.checkEqual("22. getMessageHeading", store.getMessageHeading(0, tx, list), "h2");
}
