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
#include "server/talk/talkpm.hpp"

namespace {
    void makeSystemFolders(server::talk::Root& root)
    {
        root.defaultFolderRoot().subtree("1").hashKey("header").stringField("name").set("Inbox");
        root.defaultFolderRoot().subtree("1").hashKey("header").stringField("description").set("Incoming messages");
        root.defaultFolderRoot().subtree("2").hashKey("header").stringField("name").set("Outbox");
        root.defaultFolderRoot().subtree("2").hashKey("header").stringField("description").set("Sent messages");
        root.defaultFolderRoot().intSetKey("all").add(1);
        root.defaultFolderRoot().intSetKey("all").add(2);
    }
}

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
    makeSystemFolders(root);

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
        std::auto_ptr<afl::data::Value> p(testee.getPMs(2, TalkFolder::ListParameters(), TalkFolder::FilterParameters()));
        TS_ASSERT_EQUALS(afl::data::Access(p).getArraySize(), 1U);
        TS_ASSERT_EQUALS(afl::data::Access(p)[0].toInteger(), 42);
    }
    {
        TS_ASSERT_THROWS(testee.getPMs(200, TalkFolder::ListParameters(), TalkFolder::FilterParameters()), std::exception);
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
        TS_ASSERT_THROWS(testee.getPMs(1, TalkFolder::ListParameters(), TalkFolder::FilterParameters()), std::exception);
    }
}

/** Test message flags. */
void
TestServerTalkTalkFolder::testMessageFlags()
{
    using server::talk::TalkFolder;
    using server::talk::TalkPM;
    using server::talk::Session;
    using afl::data::Access;
    using afl::data::Value;

    // Infrastructure
    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler mq;
    server::talk::Root root(db, mq, server::talk::Configuration());
    makeSystemFolders(root);

    // Sessions
    Session aSession;
    Session bSession;
    aSession.setUser("a");
    bSession.setUser("b");

    // Send messages from A to B
    int32_t m1 = TalkPM(aSession, root).create("u:b", "subj", "text:text1", afl::base::Nothing);
    int32_t m2 = TalkPM(aSession, root).create("u:b", "other", "text:text2", afl::base::Nothing);
    int32_t m3 = TalkPM(aSession, root).create("u:b", "re: subj", "text:text3", m1);
    int32_t m4 = TalkPM(aSession, root).create("u:b", "re: re: subj", "text:text3", m3);

    // Mark 1 read
    {
        int32_t m1s[] = {m1};
        TalkPM(bSession, root).changeFlags(1, 0, 1, m1s);
    }

    // FOLDERLSPM 1
    TalkFolder impl(bSession, root);
    {
        TalkFolder::ListParameters p;
        TalkFolder::FilterParameters f;
        std::auto_ptr<Value> res(impl.getPMs(1, p, f));
        Access a(res.get());
        TS_ASSERT_EQUALS(a.getArraySize(), 4U);
        TS_ASSERT_EQUALS(a[0].toInteger(), m1);
        TS_ASSERT_EQUALS(a[1].toInteger(), m2);
        TS_ASSERT_EQUALS(a[2].toInteger(), m3);
        TS_ASSERT_EQUALS(a[3].toInteger(), m4);
    }

    // FOLDERLSPM 1 SIZE
    {
        TalkFolder::ListParameters p;
        p.mode = TalkFolder::ListParameters::WantSize;
        TalkFolder::FilterParameters f;
        std::auto_ptr<Value> res(impl.getPMs(1, p, f));
        Access a(res.get());
        TS_ASSERT_EQUALS(a.toInteger(), 4);
    }

    // FOLDERLSPM 1 CONTAINS 3
    {
        TalkFolder::ListParameters p;
        p.mode = TalkFolder::ListParameters::WantMemberCheck;
        p.item = m3;
        TalkFolder::FilterParameters f;
        std::auto_ptr<Value> res(impl.getPMs(1, p, f));
        Access a(res.get());
        TS_ASSERT_EQUALS(a.toInteger(), 1);
    }

    // FOLDERLSPM 1 LIMIT 1 2
    {
        TalkFolder::ListParameters p;
        p.mode = TalkFolder::ListParameters::WantRange;
        p.start = 1;
        p.count = 2;
        TalkFolder::FilterParameters f;
        std::auto_ptr<Value> res(impl.getPMs(1, p, f));
        Access a(res.get());
        TS_ASSERT_EQUALS(a.getArraySize(), 2U);
        TS_ASSERT_EQUALS(a[0].toInteger(), m2);
        TS_ASSERT_EQUALS(a[1].toInteger(), m3);
    }

    // FOLDERLSPM 1 FLAGS 1 0
    {
        TalkFolder::ListParameters p;
        TalkFolder::FilterParameters f;
        f.flagMask = 1;
        f.flagCheck = 0;
        std::auto_ptr<Value> res(impl.getPMs(1, p, f));
        Access a(res.get());
        TS_ASSERT_EQUALS(a.getArraySize(), 3U);
        TS_ASSERT_EQUALS(a[0].toInteger(), m2);
        TS_ASSERT_EQUALS(a[1].toInteger(), m3);
        TS_ASSERT_EQUALS(a[2].toInteger(), m4);
    }

    // FOLDERLSPM 1 FLAGS 1 0 CONTAINS 3
    {
        TalkFolder::ListParameters p;
        p.mode = TalkFolder::ListParameters::WantMemberCheck;
        p.item = m3;
        TalkFolder::FilterParameters f;
        f.flagMask = 1;
        f.flagCheck = 0;
        std::auto_ptr<Value> res(impl.getPMs(1, p, f));
        Access a(res.get());
        TS_ASSERT_EQUALS(a.toInteger(), 1);
    }

    // FOLDERLSPM 1 FLAGS 1 1 CONTAINS 3
    {
        TalkFolder::ListParameters p;
        p.mode = TalkFolder::ListParameters::WantMemberCheck;
        p.item = m3;
        TalkFolder::FilterParameters f;
        f.flagMask = 1;
        f.flagCheck = 1;
        std::auto_ptr<Value> res(impl.getPMs(1, p, f));
        Access a(res.get());
        TS_ASSERT_EQUALS(a.toInteger(), 0);
    }

    // FOLDERLSPM 1 FLAGS 1 0 CONTAINS 999
    {
        TalkFolder::ListParameters p;
        p.mode = TalkFolder::ListParameters::WantMemberCheck;
        p.item = 999;
        TalkFolder::FilterParameters f;
        f.flagMask = 1;
        f.flagCheck = 0;
        std::auto_ptr<Value> res(impl.getPMs(1, p, f));
        Access a(res.get());
        TS_ASSERT_EQUALS(a.toInteger(), 0);
    }

    // FOLDERLSPM 1 FLAGS 1 0 SIZE
    {
        TalkFolder::ListParameters p;
        p.mode = TalkFolder::ListParameters::WantSize;
        TalkFolder::FilterParameters f;
        f.flagMask = 1;
        f.flagCheck = 0;
        std::auto_ptr<Value> res(impl.getPMs(1, p, f));
        Access a(res.get());
        TS_ASSERT_EQUALS(a.toInteger(), 3);
    }

    // FOLDERLSPM 1 LIMIT 1 2 FLAGS 128 0
    {
        TalkFolder::ListParameters p;
        p.mode = TalkFolder::ListParameters::WantRange;
        p.start = 1;
        p.count = 2;
        TalkFolder::FilterParameters f;
        f.flagMask = 128;
        f.flagCheck = 0;
        std::auto_ptr<Value> res(impl.getPMs(1, p, f));
        Access a(res.get());
        TS_ASSERT_EQUALS(a.getArraySize(), 2U);
        TS_ASSERT_EQUALS(a[0].toInteger(), m2);
        TS_ASSERT_EQUALS(a[1].toInteger(), m3);
    }

    // FOLDERLSPM 1 FLAGS 1 0 SORT subject
    {
        TalkFolder::ListParameters p;
        p.sortKey = String_t("SUBJECT");
        TalkFolder::FilterParameters f;
        f.flagMask = 1;
        f.flagCheck = 0;
        std::auto_ptr<Value> res(impl.getPMs(1, p, f));
        Access a(res.get());
        TS_ASSERT_EQUALS(a.getArraySize(), 3U);
        TS_ASSERT_EQUALS(a[0].toInteger(), m2);  // other
        TS_ASSERT_EQUALS(a[1].toInteger(), m4);  // re: re: subj
        TS_ASSERT_EQUALS(a[2].toInteger(), m3);  // re: subj
    }

    // FOLDERLSPM 1 SORT subject
    {
        TalkFolder::ListParameters p;
        p.sortKey = String_t("SUBJECT");
        TalkFolder::FilterParameters f;
        std::auto_ptr<Value> res(impl.getPMs(1, p, f));
        Access a(res.get());
        TS_ASSERT_EQUALS(a.getArraySize(), 4U);
        TS_ASSERT_EQUALS(a[0].toInteger(), m2);  // other
        TS_ASSERT_EQUALS(a[1].toInteger(), m4);  // re: re: subj
        TS_ASSERT_EQUALS(a[2].toInteger(), m3);  // re: subj
        TS_ASSERT_EQUALS(a[3].toInteger(), m1);  // subj
    }
}

