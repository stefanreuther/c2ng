/**
  *  \file test/server/user/usermanagementtest.cpp
  *  \brief Test for server::user::UserManagement
  */

#include "server/user/usermanagement.hpp"

#include "afl/data/access.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/test/testrunner.hpp"
#include "server/common/numericalidgenerator.hpp"
#include "server/user/classicencrypter.hpp"
#include "server/user/multipasswordencrypter.hpp"
#include "server/user/root.hpp"

using afl::data::Access;
using afl::base::Nothing;

/** Test creation of a user. */
AFL_TEST("server.user.UserManagement:create", a)
{
    // Environment
    server::common::NumericalIdGenerator gen;
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Root root(db, gen, enc, server::user::Configuration());

    // Testee
    server::user::UserManagement testee(root);

    // Operate
    // - Create a user. Must succeed.
    String_t id;
    String_t config[] = { "realname", "John Doe",
                          "createua", "wget/1.16" };
    AFL_CHECK_SUCCEEDS(a("01. add"), id = testee.add("joe", "secret", config));
    a.checkDifferent("02. add", id, "");

    // - Creating same user again fails
    AFL_CHECK_THROWS(a("11. add"), testee.add("joe", "other", config), std::exception);

    // - Creating a different user works
    a.checkDifferent("21. add", testee.add("joe2", "other", config), id);

    // - Cross-check
    a.checkEqual("31. getUserIdByName", testee.getUserIdByName("joe"), id);
    a.checkEqual("32. getNameByUserId", testee.getNameByUserId(id), "joe");
    a.checkEqual("33. login", testee.login("joe", "secret"), id);
    AFL_CHECK_THROWS(a("34. login"), testee.login("joe", "other"), std::exception);

    std::auto_ptr<server::Value_t> p;
    AFL_CHECK_SUCCEEDS(a("41. screenname"), p.reset(testee.getProfileRaw(id, "screenname")));
    a.checkEqual("42. screenname", Access(p).toString(), "joe");
    AFL_CHECK_SUCCEEDS(a("43. createua"), p.reset(testee.getProfileRaw(id, "createua")));
    a.checkEqual("44. createua", Access(p).toString(), "wget/1.16");
    AFL_CHECK_SUCCEEDS(a("45. fancy"), p.reset(testee.getProfileRaw(id, "fancy")));
    a.checkNull("46. fancy", p.get());

    // Look up multiple
    const String_t ids[] = {id};
    afl::data::StringList_t names;
    AFL_CHECK_SUCCEEDS(a("51. getNamesByUserId"), testee.getNamesByUserId(ids, names));
    a.checkEqual("52. size", names.size(), 1U);
    a.checkEqual("53. result", names[0], "joe");
}

/** Test user name handling. */
AFL_TEST("server.user.UserManagement:name", a)
{
    // Environment
    server::common::NumericalIdGenerator gen;
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Root root(db, gen, enc, server::user::Configuration());

    // Testee
    server::user::UserManagement testee(root);

    String_t id;
    std::auto_ptr<server::Value_t> p;

    AFL_CHECK_SUCCEEDS(a("01. add"), id = testee.add("joe random", "foo", Nothing));
    AFL_CHECK_SUCCEEDS(a("02. screenname"), p.reset(testee.getProfileRaw(id, "screenname")));
    a.checkEqual("03. result", Access(p).toString(), "joe random");
    a.checkEqual("04. getNameByUserId", testee.getNameByUserId(id), "joe_random");

    AFL_CHECK_SUCCEEDS(a("11. add"), id = testee.add("-=fancy=-", "foo", Nothing));
    AFL_CHECK_SUCCEEDS(a("12. screenname"), p.reset(testee.getProfileRaw(id, "screenname")));
    a.checkEqual("13. result", Access(p).toString(), "-=fancy=-");
    a.checkEqual("14. getNameByUserId", testee.getNameByUserId(id), "fancy");

    AFL_CHECK_SUCCEEDS(a("21. add"), id = testee.add("H4XoR", "foo", Nothing));
    AFL_CHECK_SUCCEEDS(a("22. screenname"), p.reset(testee.getProfileRaw(id, "screenname")));
    a.checkEqual("23. result", Access(p).toString(), "H4XoR");
    a.checkEqual("24. getNameByUserId", testee.getNameByUserId(id), "h4xor");

    AFL_CHECK_SUCCEEDS(a("31. add"), id = testee.add("  hi  ", "foo", Nothing));
    AFL_CHECK_SUCCEEDS(a("32. screenname"), p.reset(testee.getProfileRaw(id, "screenname")));
    a.checkEqual("33. result", Access(p).toString(), "  hi  ");
    a.checkEqual("34. getNameByUserId", testee.getNameByUserId(id), "hi");

    AFL_CHECK_THROWS(a("41. empty name"), testee.add("-=#=-", "foo", Nothing), std::exception);
    AFL_CHECK_THROWS(a("42. empty name"), testee.add("", "foo", Nothing), std::exception);
}

/** Test handling blocked names. */
AFL_TEST("server.user.UserManagement:name:blocked", a)
{
    // Environment
    server::common::NumericalIdGenerator gen;
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Root root(db, gen, enc, server::user::Configuration());

    // Testee
    server::user::UserManagement testee(root);

    // Block a name
    afl::net::redis::StringKey(db, "uid:root").set("0");

    // Allocating this name fails
    AFL_CHECK_THROWS(a("01. add"), testee.add("root", "foo", Nothing), std::exception);

    // Logging in fails
    AFL_CHECK_THROWS(a("11. login"), testee.login("root", "foo"), std::exception);

    // Looking it up fails
    AFL_CHECK_THROWS(a("21. getUserIdByName"), testee.getUserIdByName("root"), std::exception);
}

/** Test profile handling. */
AFL_TEST("server.user.UserManagement:profile", a)
{
    // Environment
    server::common::NumericalIdGenerator gen;
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Root root(db, gen, enc, server::user::Configuration());

    // Testee
    server::user::UserManagement testee(root);

    // Default profile
    afl::net::redis::HashKey(db, "default:profile").intField("default1").set(1);
    afl::net::redis::HashKey(db, "default:profile").intField("default2").set(2);

    // Default profile
    afl::net::redis::HashKey(db, "default:profilecopy").intField("copy1").set(1);
    afl::net::redis::HashKey(db, "default:profilecopy").intField("copy2").set(2);

    // Create a user
    const String_t config[] = { "screenname", "Ottilie", "default1", "7", "copy2", "9" };
    String_t id = testee.add("otto", "w", config);

    // Update profiles
    afl::net::redis::HashKey(db, "default:profile").intField("default1").set(11);
    afl::net::redis::HashKey(db, "default:profile").intField("default2").set(12);
    afl::net::redis::HashKey(db, "default:profilecopy").intField("copy1").set(11);
    afl::net::redis::HashKey(db, "default:profilecopy").intField("copy2").set(12);

    // Verify individual items
    // - screenname normally set from parameter, overriden from config
    std::auto_ptr<server::Value_t> p;
    p.reset(testee.getProfileRaw(id, "screenname"));
    a.checkEqual("01. screenname", Access(p).toString(), "Ottilie");

    // - default1 explicitly mentioned in config
    p.reset(testee.getProfileRaw(id, "default1"));
    a.checkEqual("11. default1", Access(p).toString(), "7");

    // - default2 taken from changed default
    p.reset(testee.getProfileRaw(id, "default2"));
    a.checkEqual("21. default2", Access(p).toString(), "12");

    // - copy1 taken from default:profilecopy at time of account creation
    p.reset(testee.getProfileRaw(id, "copy1"));
    a.checkEqual("31. copy1", Access(p).toString(), "1");

    // - copy2 explicitly mentioned in config
    p.reset(testee.getProfileRaw(id, "copy2"));
    a.checkEqual("41. copy2", Access(p).toString(), "9");

    // Verify multiple at once
    const String_t keys[] = {"default1", "copy1"};
    p.reset(testee.getProfileRaw(id, keys));
    a.checkEqual("51. default1", Access(p)[0].toString(), "7");
    a.checkEqual("52. copy1", Access(p)[1].toString(), "1");
}

/** Test login(). */
AFL_TEST("server.user.UserManagement:login", a)
{
    // ex TestServerTalkTalkNNTP::testLogin
    using afl::net::redis::Subtree;

    // Infrastructure
    afl::net::redis::InternalDatabase db;
    server::common::NumericalIdGenerator gen;
    Subtree(db, "user:").subtree("1009").stringKey("password").set("1,52YluJAXWKqqhVThh22cNw");
    Subtree(db, "uid:").stringKey("a_b").set("1009");
    Subtree(db, "uid:").stringKey("root").set("0");

    // Test it
    {
        server::user::ClassicEncrypter enc("xyz");
        server::user::Root root(db, gen, enc, server::user::Configuration());
        server::user::UserManagement testee(root);

        // Success cases
        a.checkEqual("01. login", testee.login("a_b", "z"), "1009");
        a.checkEqual("02. login", testee.login("A_B", "z"), "1009");
        a.checkEqual("03. login", testee.login("A->B", "z"), "1009");

        // Error cases
        AFL_CHECK_THROWS(a("11. blocked name"), testee.login("root", ""), std::exception);
        AFL_CHECK_THROWS(a("12. bad password"), testee.login("a_b", ""), std::exception);
        AFL_CHECK_THROWS(a("13. bad password"), testee.login("a_b", "zzz"), std::exception);
        AFL_CHECK_THROWS(a("14. bad password"), testee.login("a_b", "Z"), std::exception);
        AFL_CHECK_THROWS(a("15. empty name"),   testee.login("", "Z"), std::exception);
        AFL_CHECK_THROWS(a("16. empty name"),   testee.login("/", "Z"), std::exception);
    }

    // Test it with different user key. This must make the test fail
    {
        server::user::ClassicEncrypter enc("abc");
        server::user::Root root(db, gen, enc, server::user::Configuration());
        server::user::UserManagement testee(root);

        AFL_CHECK_THROWS(a("21. wrong key"), testee.login("a_b", "z"), std::exception);
        AFL_CHECK_THROWS(a("22. wrong key"), testee.login("root", ""), std::exception);
    }
}

/** Test profile limitations. */
AFL_TEST("server.user.UserManagement:profile:limit", a)
{
    // Environment
    server::user::Configuration fig;
    fig.profileMaxValueSize = 5;
    server::common::NumericalIdGenerator gen;
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Root root(db, gen, enc, fig);

    // Testee
    server::user::UserManagement testee(root);

    // Create a user. Must succeed.
    String_t id;
    String_t config[] = { "realname", "John",
                          "createua", "wget/1.16" };
    AFL_CHECK_SUCCEEDS(a("01. add"), id = testee.add("joe_luser", "secret", config));
    a.checkDifferent("02. add", id, "");

    // Verify created profile
    std::auto_ptr<afl::data::Value> p;
    AFL_CHECK_SUCCEEDS(a("11. realname"), p.reset(testee.getProfileRaw(id, "realname")));
    a.checkEqual("12. realname", Access(p).toString(), "John");
    AFL_CHECK_SUCCEEDS(a("13. createua"), p.reset(testee.getProfileRaw(id, "createua")));
    a.checkEqual("14. createua", Access(p).toString(), "wget/");  // truncated
    AFL_CHECK_SUCCEEDS(a("15. screenname"), p.reset(testee.getProfileRaw(id, "screenname")));
    a.checkEqual("16. screenname", Access(p).toString(), "joe_l");  // truncated

    // Update profile
    String_t update[] = { "infotown", "York",
                          "infooccupation", "Whatever" };
    AFL_CHECK_SUCCEEDS(a("21. setProfile"), testee.setProfile(id, update));
    AFL_CHECK_SUCCEEDS(a("22. infotown"), p.reset(testee.getProfileRaw(id, "infotown")));
    a.checkEqual("23. infotown", Access(p).toString(), "York");
    AFL_CHECK_SUCCEEDS(a("24. infooccupation"), p.reset(testee.getProfileRaw(id, "infooccupation")));
    a.checkEqual("25. infooccupation", Access(p).toString(), "Whate");  // truncated
}

/** Test profile limit turned off.
    Setting the limit to 0 means no limit. */
AFL_TEST("server.user.UserManagement:profile:unlimited", a)
{
    // Environment
    server::user::Configuration fig;
    fig.profileMaxValueSize = 0;
    server::common::NumericalIdGenerator gen;
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Root root(db, gen, enc, fig);

    // Testee
    server::user::UserManagement testee(root);

    // Create a user. Must succeed.
    String_t id;
    String_t config[] = { "createua", "wget/1.16" };
    AFL_CHECK_SUCCEEDS(a("01. add"), id = testee.add("joe_luser", "secret", config));
    a.checkDifferent("02. add", id, "");

    // Verify created profile
    std::auto_ptr<afl::data::Value> p;
    AFL_CHECK_SUCCEEDS(a("11. createua"), p.reset(testee.getProfileRaw(id, "createua")));
    a.checkEqual("12. createua", Access(p).toString(), "wget/1.16");  // not truncated
}

/** Test profile limit at defaults. */
AFL_TEST("server.user.UserManagement:profile:default-limit", a)
{
    // Environment
    server::common::NumericalIdGenerator gen;
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Root root(db, gen, enc, server::user::Configuration());

    // Testee
    server::user::UserManagement testee(root);

    // Create a user. Must succeed.
    String_t id;
    String_t config[] = { "infotown", String_t(20000, 'X') };
    AFL_CHECK_SUCCEEDS(a("01. add"), id = testee.add("joe_luser", "secret", config));
    a.checkDifferent("02. add", id, "");

    // Verify created profile
    std::auto_ptr<afl::data::Value> p;
    AFL_CHECK_SUCCEEDS(a("11. infotown"), p.reset(testee.getProfileRaw(id, "infotown")));
    a.checkEqual("12. infotown", Access(p).toString().substr(0, 1000), String_t(1000, 'X'));  // preserve sensible start
}

AFL_TEST("server.user.UserManagement:remove", a)
{
    // Environment
    server::common::NumericalIdGenerator gen;
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Root root(db, gen, enc, server::user::Configuration());

    // Testee
    server::user::UserManagement testee(root);

    // Create a user. Must succeed.
    String_t id;
    String_t config[] = { "infotown", "Arrakis", "screenname", "Jonathan" };
    AFL_CHECK_SUCCEEDS(a("01. add"), id = testee.add("joe", "secret", config));
    a.checkDifferent("02. add", id, "");

    // Verify profile content
    a.checkEqual("11. getUserIdByName", testee.getUserIdByName("joe"), id);
    a.checkEqual("12. getNameByUserId", testee.getNameByUserId(id), "joe");
    a.checkEqual("13. login", testee.login("joe", "secret"), id);
    std::auto_ptr<afl::data::Value> p;
    AFL_CHECK_SUCCEEDS(a("14. screenname"), p.reset(testee.getProfileRaw(id, "screenname")));
    a.checkEqual("15. screenname", Access(p).toString(), "Jonathan");

    // Remove the user
    AFL_CHECK_SUCCEEDS(a("21. remove"), testee.remove(id));
    AFL_CHECK_THROWS(a("22. getUserIdByName"), testee.getUserIdByName("joe"), std::exception);
    a.checkEqual("23. getNameByUserId", testee.getNameByUserId(id), "");
    AFL_CHECK_THROWS(a("24. login"), testee.login("joe", "secret"), std::exception);
    AFL_CHECK_SUCCEEDS(a("25. screenname"), p.reset(testee.getProfileRaw(id, "screenname")));
    a.checkEqual("26. screenname", Access(p).toString(), "(joe)");
    AFL_CHECK_SUCCEEDS(a("27. infotown"), p.reset(testee.getProfileRaw(id, "infotown")));
    a.checkEqual("28. infotown", Access(p).toString(), "");

    // Create another joe. Must succeed and create a different Id.
    String_t id2;
    String_t config2[] = { "infotown", "Corrino", "screenname", "Joseph" };
    AFL_CHECK_SUCCEEDS(a("31. add"), id2 = testee.add("joe", "secret", config2));
    a.checkDifferent("32. id2", id2, "");
    a.checkDifferent("33. id2", id2, id);
    a.checkEqual("34. getUserIdByName", testee.getUserIdByName("joe"), id2);
    a.checkEqual("35. getNameByUserId", testee.getNameByUserId(id2), "joe");
}

/** Test logging in when no password has been set. */
AFL_TEST("server.user.UserManagement:login:no-password", a)
{
    // ex TestServerTalkTalkNNTP::testLogin
    using afl::net::redis::Subtree;

    // Infrastructure
    afl::net::redis::InternalDatabase db;
    server::common::NumericalIdGenerator gen;
    Subtree(db, "uid:").stringKey("a_b").set("1009");
    server::user::ClassicEncrypter enc("xyz");
    server::user::Root root(db, gen, enc, server::user::Configuration());
    server::user::UserManagement testee(root);

    // Login fails, no password set
    AFL_CHECK_THROWS(a, testee.login("a_b", "z"), std::runtime_error);
}

/** Test logging in with password upgrade. */
AFL_TEST("server.user.UserManagement:login:password-upgrade", a)
{
    using afl::net::redis::Subtree;

    // Infrastructure
    afl::net::redis::InternalDatabase db;
    server::common::NumericalIdGenerator gen;
    Subtree(db, "user:").subtree("1009").stringKey("password").set("1,52YluJAXWKqqhVThh22cNw");
    Subtree(db, "uid:").stringKey("a_b").set("1009");

    // Use two ClassicEncrypter's because these are deterministic
    server::user::ClassicEncrypter oldEnc("xyz");
    server::user::ClassicEncrypter newEnc("abc");
    server::user::MultiPasswordEncrypter enc(newEnc, oldEnc);
    server::user::Root root(db, gen, enc, server::user::Configuration());
    server::user::UserManagement testee(root);

    // Logging in succeeds
    a.checkEqual("01. login", testee.login("a_b", "z"), "1009");

    // Password has been upgraded (re-hashed with new key)
    a.checkEqual("11. password", Subtree(db, "user:").subtree("1009").stringKey("password").get(), "1,2zwKRpT/uUBsg4skmgRPaQ");

    // Logging in succeeds again
    a.checkEqual("21. login", testee.login("a_b", "z"), "1009");
}
