/**
  *  \file u/t_server_talk_talkfolder.cpp
  *  \brief Test for server::talk::TalkFolder
  */

#include "server/talk/talkfolder.hpp"

#include "t_server_talk.hpp"
#include "afl/data/access.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"
#include "server/talk/user.hpp"
#include "server/talk/userfolder.hpp"
#include "server/talk/userpm.hpp"

/** Test folder commands. */
void
TestServerTalkTalkFolder::testIt()
{
    using server::talk::TalkFolder;

    // Infrastructure
    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler mq;
    server::talk::Root root(db, mq, server::talk::Configuration());
    server::talk::Session session;
    session.setUser("a");

    // Make two system folders
    root.defaultFolderRoot().subtree("1").hashKey("header").stringField("name").set("Inbox");
    root.defaultFolderRoot().subtree("1").hashKey("header").stringField("description").set("Incoming messages");
    root.defaultFolderRoot().subtree("2").hashKey("header").stringField("name").set("Outbox");
    root.defaultFolderRoot().subtree("2").hashKey("header").stringField("description").set("Sent messages");
    root.defaultFolderRoot().intSetKey("all").add(1);
    root.defaultFolderRoot().intSetKey("all").add(2);

    // Testee
    TalkFolder testee(session, root);

    // Create a user folder
    {
        const String_t args[] = {"description", "My stuff"};
        int32_t id = testee.create("mine", args);
        TS_ASSERT_EQUALS(id, 100);
    }

    // Get folders
    {
        afl::data::IntegerList_t result;
        testee.getFolders(result);
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT(std::find(result.begin(), result.end(), 1) != result.end());
        TS_ASSERT(std::find(result.begin(), result.end(), 2) != result.end());
        TS_ASSERT(std::find(result.begin(), result.end(), 100) != result.end());
    }

    // Configure
    {
        const String_t args[] = {"name", "New Mail", "description", "Incoming"};
        TS_ASSERT_THROWS_NOTHING(testee.configure(1, args));
    }

    // Get info
    {
        TalkFolder::Info i = testee.getInfo(1);
        TS_ASSERT_EQUALS(i.name, "New Mail");
        TS_ASSERT_EQUALS(i.description, "Incoming");
        TS_ASSERT_EQUALS(i.numMessages, 0);
        TS_ASSERT_EQUALS(i.isFixedFolder, true);
    }
    {
        TalkFolder::Info i = testee.getInfo(100);
        TS_ASSERT_EQUALS(i.name, "mine");
        TS_ASSERT_EQUALS(i.description, "My stuff");
        TS_ASSERT_EQUALS(i.numMessages, 0);
        TS_ASSERT_EQUALS(i.isFixedFolder, false);
    }
    {
        TS_ASSERT_THROWS(testee.getInfo(200), std::exception);
    }
    {
        static const int32_t ufids[] = {1,100,200,2};
        afl::container::PtrVector<TalkFolder::Info> result;
        TS_ASSERT_THROWS_NOTHING(testee.getInfo(ufids, result));
        TS_ASSERT_EQUALS(result.size(), 4U);
        TS_ASSERT(result[0] != 0);
        TS_ASSERT(result[1] != 0);
        TS_ASSERT(result[2] == 0);
        TS_ASSERT(result[3] != 0);
        TS_ASSERT_EQUALS(result[0]->name, "New Mail");
        TS_ASSERT_EQUALS(result[1]->name, "mine");
        TS_ASSERT_EQUALS(result[3]->name, "Outbox");
    }

    // Link some PMs for further use
    {
        server::talk::User u(root, "a");
        server::talk::UserFolder(u, 2).messages().add(42);
        server::talk::UserFolder(u, 100).messages().add(42);
        server::talk::UserPM(root, 42).referenceCounter().set(2);
    }

    // Get PMs
    {
        std::auto_ptr<afl::data::Value> p(testee.getPMs(2, TalkFolder::ListParameters()));
        TS_ASSERT_EQUALS(afl::data::Access(p).getArraySize(), 1U);
        TS_ASSERT_EQUALS(afl::data::Access(p)[0].toInteger(), 42);
    }
    {
        TS_ASSERT_THROWS(testee.getPMs(200, TalkFolder::ListParameters()), std::exception);
    }

    // Remove
    TS_ASSERT_EQUALS(testee.remove(100), true);
    TS_ASSERT_EQUALS(testee.remove(100), false);
    TS_ASSERT_EQUALS(testee.remove(1),   false);
    TS_ASSERT_EQUALS(server::talk::UserPM(root, 42).referenceCounter().get(), 1);

    // Error cases [must be at end because they might be partially executed]
    {
        const String_t args[] = {"description"};
        TS_ASSERT_THROWS(testee.create("more", args), std::exception);
    }
    {
        const String_t args[] = {"description"};
        TS_ASSERT_THROWS(testee.configure(1, args), std::exception);
    }
}

/** Test commands as root. Must all fail because we need a user context. */
void
TestServerTalkTalkFolder::testRoot()
{
    using server::talk::TalkFolder;

    // Infrastructure
    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler mq;
    server::talk::Root root(db, mq, server::talk::Configuration());
    server::talk::Session session;

    // Make a system folders (not required, commands hopefully fail before looking here)
    root.defaultFolderRoot().subtree("1").hashKey("header").stringField("name").set("Inbox");
    root.defaultFolderRoot().intSetKey("all").add(1);

    // Testee
    TalkFolder testee(session, root);

    {
        afl::data::IntegerList_t result;
        TS_ASSERT_THROWS(testee.getFolders(result), std::exception);
    }
    {
        TS_ASSERT_THROWS(testee.getInfo(1), std::exception);
    }
    {
        static const int32_t ufids[] = {1};
        afl::container::PtrVector<TalkFolder::Info> result;
        TS_ASSERT_THROWS(testee.getInfo(ufids, result), std::exception);
    }
    {
        TS_ASSERT_THROWS(testee.create("foo", afl::base::Nothing), std::exception);
        TS_ASSERT_THROWS(testee.remove(100), std::exception);
        TS_ASSERT_THROWS(testee.configure(1, afl::base::Nothing), std::exception);
        TS_ASSERT_THROWS(testee.getPMs(1, TalkFolder::ListParameters()), std::exception);
    }
}

