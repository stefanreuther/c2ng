/**
  *  \file test/server/talk/talkfoldertest.cpp
  *  \brief Test for server::talk::TalkFolder
  */

#include "server/talk/talkfolder.hpp"

#include "afl/data/access.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/test/testrunner.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"
#include "server/talk/talkpm.hpp"
#include "server/talk/user.hpp"
#include "server/talk/userfolder.hpp"
#include "server/talk/userpm.hpp"

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
AFL_TEST("server.talk.TalkFolder:basics", a)
{
    using server::talk::TalkFolder;

    // Infrastructure
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, server::talk::Configuration());
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
        a.checkEqual("01. create", id, 100);
    }

    // Get folders
    {
        afl::data::IntegerList_t result;
        testee.getFolders(result);
        a.checkEqual("11. size", result.size(), 3U);
        a.check("12. folder", std::find(result.begin(), result.end(), 1) != result.end());
        a.check("13. folder", std::find(result.begin(), result.end(), 2) != result.end());
        a.check("14. folder", std::find(result.begin(), result.end(), 100) != result.end());
    }

    // Configure
    {
        const String_t args[] = {"name", "New Mail", "description", "Incoming"};
        AFL_CHECK_SUCCEEDS(a("21. configure"), testee.configure(1, args));
    }

    // Get info
    {
        TalkFolder::Info i = testee.getInfo(1);
        a.checkEqual("31. name",          i.name, "New Mail");
        a.checkEqual("32. description",   i.description, "Incoming");
        a.checkEqual("33. numMessages",   i.numMessages, 0);
        a.checkEqual("34. isFixedFolder", i.isFixedFolder, true);
    }
    {
        TalkFolder::Info i = testee.getInfo(100);
        a.checkEqual("35. name",          i.name, "mine");
        a.checkEqual("36. description",   i.description, "My stuff");
        a.checkEqual("37. numMessages",   i.numMessages, 0);
        a.checkEqual("38. isFixedFolder", i.isFixedFolder, false);
    }
    {
        AFL_CHECK_THROWS(a("39. getInfo"), testee.getInfo(200), std::exception);
    }
    {
        static const int32_t ufids[] = {1,100,200,2};
        afl::container::PtrVector<TalkFolder::Info> result;

        AFL_CHECK_SUCCEEDS(a("40. getInfo"), testee.getInfo(ufids, result));
        a.checkEqual  ("41. size",   result.size(), 4U);
        a.checkNonNull("42. result", result[0]);
        a.checkNonNull("43. result", result[1]);
        a.checkNull   ("44. result", result[2]);
        a.checkNonNull("45. result", result[3]);
        a.checkEqual  ("46. name",   result[0]->name, "New Mail");
        a.checkEqual  ("47. name",   result[1]->name, "mine");
        a.checkEqual  ("48. name",   result[3]->name, "Outbox");
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
        a.checkEqual("51. getPMs", afl::data::Access(p).getArraySize(), 1U);
        a.checkEqual("52. getPMs", afl::data::Access(p)[0].toInteger(), 42);
    }
    {
        AFL_CHECK_THROWS(a("53. getPMs"), testee.getPMs(200, TalkFolder::ListParameters(), TalkFolder::FilterParameters()), std::exception);
    }

    // Remove
    a.checkEqual("61. remove", testee.remove(100), true);
    a.checkEqual("62. remove", testee.remove(100), false);
    a.checkEqual("63. remove", testee.remove(1),   false);
    a.checkEqual("64. refCount", server::talk::UserPM(root, 42).referenceCounter().get(), 1);

    // Error cases [must be at end because they might be partially executed]
    {
        const String_t args[] = {"description"};
        AFL_CHECK_THROWS(a("71. create"), testee.create("more", args), std::exception);
    }
    {
        const String_t args[] = {"description"};
        AFL_CHECK_THROWS(a("72. configure"), testee.configure(1, args), std::exception);
    }
}

/** Test commands as root. Must all fail because we need a user context. */
AFL_TEST("server.talk.TalkFolder:admin", a)
{
    using server::talk::TalkFolder;

    // Infrastructure
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, server::talk::Configuration());
    server::talk::Session session;

    // Make a system folders (not required, commands hopefully fail before looking here)
    root.defaultFolderRoot().subtree("1").hashKey("header").stringField("name").set("Inbox");
    root.defaultFolderRoot().intSetKey("all").add(1);

    // Testee
    TalkFolder testee(session, root);

    {
        afl::data::IntegerList_t result;
        AFL_CHECK_THROWS(a("01. getFolders"), testee.getFolders(result), std::exception);
    }
    {
        AFL_CHECK_THROWS(a("02. getInfo"), testee.getInfo(1), std::exception);
    }
    {
        static const int32_t ufids[] = {1};
        afl::container::PtrVector<TalkFolder::Info> result;
        AFL_CHECK_THROWS(a("03. getInfo"), testee.getInfo(ufids, result), std::exception);
    }
    {
        AFL_CHECK_THROWS(a("04. create"),    testee.create("foo", afl::base::Nothing), std::exception);
        AFL_CHECK_THROWS(a("05. remove"),    testee.remove(100), std::exception);
        AFL_CHECK_THROWS(a("06. configure"), testee.configure(1, afl::base::Nothing), std::exception);
        AFL_CHECK_THROWS(a("07. getPMs"),    testee.getPMs(1, TalkFolder::ListParameters(), TalkFolder::FilterParameters()), std::exception);
    }
}

/** Test message flags. */
AFL_TEST("server.talk.TalkFolder:message-flags", a)
{
    using server::talk::TalkFolder;
    using server::talk::TalkPM;
    using server::talk::Session;
    using afl::data::Access;
    using afl::data::Value;

    // Infrastructure
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, server::talk::Configuration());
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
        Access ap(res.get());
        a.checkEqual("01. getArraySize", ap.getArraySize(), 4U);
        a.checkEqual("02. result", ap[0].toInteger(), m1);
        a.checkEqual("03. result", ap[1].toInteger(), m2);
        a.checkEqual("04. result", ap[2].toInteger(), m3);
        a.checkEqual("05. result", ap[3].toInteger(), m4);
    }

    // FOLDERLSPM 1 SIZE
    {
        TalkFolder::ListParameters p;
        p.mode = TalkFolder::ListParameters::WantSize;
        TalkFolder::FilterParameters f;
        std::auto_ptr<Value> res(impl.getPMs(1, p, f));
        Access ap(res.get());
        a.checkEqual("11. folder size", ap.toInteger(), 4);
    }

    // FOLDERLSPM 1 CONTAINS 3
    {
        TalkFolder::ListParameters p;
        p.mode = TalkFolder::ListParameters::WantMemberCheck;
        p.item = m3;
        TalkFolder::FilterParameters f;
        std::auto_ptr<Value> res(impl.getPMs(1, p, f));
        Access ap(res.get());
        a.checkEqual("21. folder contains", ap.toInteger(), 1);
    }

    // FOLDERLSPM 1 LIMIT 1 2
    {
        TalkFolder::ListParameters p;
        p.mode = TalkFolder::ListParameters::WantRange;
        p.start = 1;
        p.count = 2;
        TalkFolder::FilterParameters f;
        std::auto_ptr<Value> res(impl.getPMs(1, p, f));
        Access ap(res.get());
        a.checkEqual("31. getArraySize", ap.getArraySize(), 2U);
        a.checkEqual("32. result", ap[0].toInteger(), m2);
        a.checkEqual("33. result", ap[1].toInteger(), m3);
    }

    // FOLDERLSPM 1 FLAGS 1 0
    {
        TalkFolder::ListParameters p;
        TalkFolder::FilterParameters f;
        f.flagMask = 1;
        f.flagCheck = 0;
        std::auto_ptr<Value> res(impl.getPMs(1, p, f));
        Access ap(res.get());
        a.checkEqual("41. getArraySize", ap.getArraySize(), 3U);
        a.checkEqual("42. result", ap[0].toInteger(), m2);
        a.checkEqual("43. result", ap[1].toInteger(), m3);
        a.checkEqual("44. result", ap[2].toInteger(), m4);
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
        Access ap(res.get());
        a.checkEqual("51. folder contains", ap.toInteger(), 1);
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
        Access ap(res.get());
        a.checkEqual("61. folder contains", ap.toInteger(), 0);
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
        Access ap(res.get());
        a.checkEqual("71. folder contains", ap.toInteger(), 0);
    }

    // FOLDERLSPM 1 FLAGS 1 0 SIZE
    {
        TalkFolder::ListParameters p;
        p.mode = TalkFolder::ListParameters::WantSize;
        TalkFolder::FilterParameters f;
        f.flagMask = 1;
        f.flagCheck = 0;
        std::auto_ptr<Value> res(impl.getPMs(1, p, f));
        Access ap(res.get());
        a.checkEqual("81. folder size", ap.toInteger(), 3);
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
        Access ap(res.get());
        a.checkEqual("91. getArraySize", ap.getArraySize(), 2U);
        a.checkEqual("92. result", ap[0].toInteger(), m2);
        a.checkEqual("93. result", ap[1].toInteger(), m3);
    }

    // FOLDERLSPM 1 FLAGS 1 0 SORT subject
    {
        TalkFolder::ListParameters p;
        p.sortKey = String_t("SUBJECT");
        TalkFolder::FilterParameters f;
        f.flagMask = 1;
        f.flagCheck = 0;
        std::auto_ptr<Value> res(impl.getPMs(1, p, f));
        Access ap(res.get());
        a.checkEqual("101. getArraySize", ap.getArraySize(), 3U);
        a.checkEqual("102. result", ap[0].toInteger(), m2);  // other
        a.checkEqual("103. result", ap[1].toInteger(), m4);  // re: re: subj
        a.checkEqual("104. result", ap[2].toInteger(), m3);  // re: subj
    }

    // FOLDERLSPM 1 SORT subject
    {
        TalkFolder::ListParameters p;
        p.sortKey = String_t("SUBJECT");
        TalkFolder::FilterParameters f;
        std::auto_ptr<Value> res(impl.getPMs(1, p, f));
        Access ap(res.get());
        a.checkEqual("111. getArraySize", ap.getArraySize(), 4U);
        a.checkEqual("112. result", ap[0].toInteger(), m2);  // other
        a.checkEqual("113. result", ap[1].toInteger(), m4);  // re: re: subj
        a.checkEqual("114. result", ap[2].toInteger(), m3);  // re: subj
        a.checkEqual("115. result", ap[3].toInteger(), m1);  // subj
    }
}
