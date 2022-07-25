/**
  *  \file u/t_server_talk_userfolder.cpp
  *  \brief Test for server::talk::UserFolder
  */

#include "server/talk/userfolder.hpp"

#include "t_server_talk.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "server/talk/configuration.hpp"
#include "server/talk/root.hpp"
#include "server/talk/user.hpp"
#include "server/talk/userpm.hpp"

/** Simple tests. */
void
TestServerTalkUserFolder::testIt()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // User
    server::talk::User u(root, "1002");
    u.pmFolders().add(100);

    // Folder
    server::talk::UserFolder uf(u, 100);
    for (int i = 0; i < 12; ++i) {
        uf.messages().add(i+99);
    }

    // Verify property accessibility
    uf.unreadMessages().set(1);
    uf.header().stringField("name").set("nn");
    uf.header().stringField("description").set("dd");

    TS_ASSERT_EQUALS(uf.messages().size(), 12);
    TS_ASSERT_EQUALS(uf.unreadMessages().get(), 1);
    TS_ASSERT_EQUALS(uf.getHeader("name", root), "nn");
    TS_ASSERT_EQUALS(uf.getHeader("description", root), "dd");
    TS_ASSERT_EQUALS(uf.checkExistance(root), true);

    server::interface::TalkFolder::Info i = uf.describe(true, root);
    TS_ASSERT_EQUALS(i.name, "nn");
    TS_ASSERT_EQUALS(i.description, "dd");
    TS_ASSERT_EQUALS(i.numMessages, 12);
    TS_ASSERT_EQUALS(i.hasUnreadMessages, 1);
    TS_ASSERT_EQUALS(i.isFixedFolder, false);
}

/** Test allocateFolder(). */
void
TestServerTalkUserFolder::testAllocate()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // User
    server::talk::User u(root, "1002");

    // Allocate new folder. Database is empty, so this must be #100.
    int32_t id = server::talk::UserFolder::allocateFolder(u);
    TS_ASSERT_EQUALS(id, 100);
    u.pmFolders().add(id);
    server::talk::UserFolder(u, id).header().stringField("name").set("n100");

    // Another one. Must be 101.
    id = server::talk::UserFolder::allocateFolder(u);
    TS_ASSERT_EQUALS(id, 101);
    u.pmFolders().add(id);
    server::talk::UserFolder(u, id).header().stringField("name").set("n101");

    // Verify existence
    TS_ASSERT_EQUALS(server::talk::UserFolder(u, 100).checkExistance(root), true);
    TS_ASSERT_EQUALS(server::talk::UserFolder(u, 101).checkExistance(root), true);
    TS_ASSERT_EQUALS(server::talk::UserFolder(u, 100).getHeader("name", root), "n100");
    TS_ASSERT_EQUALS(server::talk::UserFolder(u, 101).getHeader("name", root), "n101");

    // Remove: this will NOT unlink the folder. It will only remove its header.
    server::talk::UserFolder(u, 100).remove();
    TS_ASSERT_EQUALS(server::talk::UserFolder(u, 100).checkExistance(root), true);
    TS_ASSERT_EQUALS(server::talk::UserFolder(u, 101).checkExistance(root), true);
    TS_ASSERT_EQUALS(server::talk::UserFolder(u, 100).getHeader("name", root), "");
    TS_ASSERT_EQUALS(server::talk::UserFolder(u, 101).getHeader("name", root), "n101");
}

/** Test mixed system/user properties. */
void
TestServerTalkUserFolder::testMixedProperties()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Make two system folders
    root.defaultFolderRoot().subtree("1").hashKey("header").stringField("name").set("Inbox");
    root.defaultFolderRoot().subtree("1").hashKey("header").stringField("description").set("Incoming messages");
    root.defaultFolderRoot().subtree("2").hashKey("header").stringField("name").set("Outbox");
    root.defaultFolderRoot().subtree("2").hashKey("header").stringField("description").set("Sent messages");
    root.defaultFolderRoot().intSetKey("all").add(1);
    root.defaultFolderRoot().intSetKey("all").add(2);

    // Create a user with a user folder.
    server::talk::User u(root, "1002");
    u.pmFolderData().subtree("100").hashKey("header").stringField("name").set("Saved");
    u.pmFolderData().subtree("100").hashKey("header").stringField("description").set("Saved messages");
    u.pmFolderData().subtree("2").hashKey("header").stringField("description").set("Outgoing messages");
    u.pmFolderData().intSetKey("all").add(100);

    // Check existance
    server::talk::UserFolder uf1(u, 1);
    server::talk::UserFolder uf2(u, 2);
    server::talk::UserFolder uf100(u, 100);
    server::talk::UserFolder uf101(u, 101);

    TS_ASSERT_EQUALS(uf1.checkExistance(root), false);
    TS_ASSERT_EQUALS(uf2.checkExistance(root), false);
    TS_ASSERT_EQUALS(uf100.checkExistance(root), true);
    TS_ASSERT_THROWS(uf101.checkExistance(root), std::exception);

    // Check headers
    TS_ASSERT_EQUALS(uf1.getHeader("name", root), "Inbox");
    TS_ASSERT_EQUALS(uf2.getHeader("name", root), "Outbox");
    TS_ASSERT_EQUALS(uf100.getHeader("name", root), "Saved");

    TS_ASSERT_EQUALS(uf1.getHeader("description", root), "Incoming messages");
    TS_ASSERT_EQUALS(uf2.getHeader("description", root), "Outgoing messages");  // overridden by user
    TS_ASSERT_EQUALS(uf100.getHeader("description", root), "Saved messages");
}

/** Test findFolder(). */
void
TestServerTalkUserFolder::testFindFolder()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Make two system folders
    const int SYSFOLDER1 = 1, SYSFOLDER2 = 2;
    root.defaultFolderRoot().intSetKey("all").add(SYSFOLDER1);
    root.defaultFolderRoot().intSetKey("all").add(SYSFOLDER2);

    // Create a user with a user folder.
    const int USERFOLDER = 100;
    server::talk::User u(root, "1002");
    u.pmFolderData().intSetKey("all").add(USERFOLDER);

    // Create a message
    const int PMID = 33;
    server::talk::UserPM pm(root, PMID);
    pm.text().set("whatever...");

    server::talk::UserFolder(u, SYSFOLDER2).messages().add(PMID);
    server::talk::UserFolder(u, USERFOLDER).messages().add(PMID);
    pm.addReference();
    pm.addReference();
    pm.addReference();

    // Verify
    // - no preference given: use system folder
    TS_ASSERT_EQUALS(server::talk::UserFolder::findFolder(u, root, PMID, 0), SYSFOLDER2);

    // - valid preference given: use it
    TS_ASSERT_EQUALS(server::talk::UserFolder::findFolder(u, root, PMID, USERFOLDER), USERFOLDER);

    // - invalid preference given: ignored
    TS_ASSERT_EQUALS(server::talk::UserFolder::findFolder(u, root, PMID, SYSFOLDER1), SYSFOLDER2);

    // - invalid message given: no result
    TS_ASSERT_EQUALS(server::talk::UserFolder::findFolder(u, root, PMID+1, 0), 0);
}

/** Test findSuggestedFolder(). */
void
TestServerTalkUserFolder::testFindSuggestedFolder()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Make two system folders
    const int SYSFOLDER1 = 1, SYSFOLDER2 = 2;
    root.defaultFolderRoot().intSetKey("all").add(SYSFOLDER1);
    root.defaultFolderRoot().intSetKey("all").add(SYSFOLDER2);

    // Create a user with a user folder.
    const int USERFOLDER1 = 100;
    const int USERFOLDER2 = 101;
    server::talk::User u(root, "1002");
    u.pmFolderData().intSetKey("all").add(USERFOLDER1);
    u.pmFolderData().intSetKey("all").add(USERFOLDER2);

    // Create messages
    // - message 1 in USERFOLDER2
    server::talk::UserPM pm1(root, 1);
    pm1.text().set("one");
    pm1.addReference();
    server::talk::UserFolder(u, USERFOLDER2).messages().add(1);

    // - message 2 out of reach
    server::talk::UserPM pm2(root, 2);
    pm2.text().set("two");
    pm2.parentMessageId().set(1);

    // - message 3 in SYSFOLDER1 and USERFOLDER1
    server::talk::UserPM pm3(root, 3);
    pm3.text().set("three");
    pm3.parentMessageId().set(2);
    pm3.addReference();
    pm3.addReference();
    server::talk::UserFolder(u, SYSFOLDER1).messages().add(3);
    server::talk::UserFolder(u, USERFOLDER1).messages().add(3);

    // - message 13 in USERFOLDER2 (but child of 2)
    server::talk::UserPM pm13(root, 13);
    pm13.text().set("thirteen");
    pm13.parentMessageId().set(2);
    pm13.addReference();
    server::talk::UserFolder(u, USERFOLDER2).messages().add(13);

    // Verify
    // - no suggestion for 1 (has no parent)
    TS_ASSERT_EQUALS(server::talk::UserFolder::findSuggestedFolder(u, root, 1, USERFOLDER2), 0);

    // - for 3, suggest USERFOLDER2, no matter where from
    TS_ASSERT_EQUALS(server::talk::UserFolder::findSuggestedFolder(u, root, 3, SYSFOLDER1), USERFOLDER2);
    TS_ASSERT_EQUALS(server::talk::UserFolder::findSuggestedFolder(u, root, 3, USERFOLDER1), USERFOLDER2);

    // - for 13, do not suggest anything when coming from USERFOLDER2 because that'd be our only suggestion
    TS_ASSERT_EQUALS(server::talk::UserFolder::findSuggestedFolder(u, root, 13, USERFOLDER2), 0);
}

