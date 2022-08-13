/**
  *  \file u/t_server_user_usermanagement.cpp
  *  \brief Test for server::user::UserManagement
  */

#include "server/user/usermanagement.hpp"

#include "t_server_user.hpp"
#include "afl/data/access.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "server/common/numericalidgenerator.hpp"
#include "server/user/classicencrypter.hpp"
#include "server/user/multipasswordencrypter.hpp"
#include "server/user/root.hpp"

using afl::data::Access;
using afl::base::Nothing;

/** Test creation of a user. */
void
TestServerUserUserManagement::testCreation()
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
    TS_ASSERT_THROWS_NOTHING(id = testee.add("joe", "secret", config));
    TS_ASSERT_DIFFERS(id, "");

    // - Creating same user again fails
    TS_ASSERT_THROWS(testee.add("joe", "other", config), std::exception);

    // - Creating a different user works
    TS_ASSERT_DIFFERS(testee.add("joe2", "other", config), id);

    // - Cross-check
    TS_ASSERT_EQUALS(testee.getUserIdByName("joe"), id);
    TS_ASSERT_EQUALS(testee.getNameByUserId(id), "joe");
    TS_ASSERT_EQUALS(testee.login("joe", "secret"), id);
    TS_ASSERT_THROWS(testee.login("joe", "other"), std::exception);

    std::auto_ptr<server::Value_t> p;
    TS_ASSERT_THROWS_NOTHING(p.reset(testee.getProfileRaw(id, "screenname")));
    TS_ASSERT_EQUALS(Access(p).toString(), "joe");
    TS_ASSERT_THROWS_NOTHING(p.reset(testee.getProfileRaw(id, "createua")));
    TS_ASSERT_EQUALS(Access(p).toString(), "wget/1.16");
    TS_ASSERT_THROWS_NOTHING(p.reset(testee.getProfileRaw(id, "fancy")));
    TS_ASSERT(p.get() == 0);

    // Look up multiple
    const String_t ids[] = {id};
    afl::data::StringList_t names;
    TS_ASSERT_THROWS_NOTHING(testee.getNamesByUserId(ids, names));
    TS_ASSERT_EQUALS(names.size(), 1U);
    TS_ASSERT_EQUALS(names[0], "joe");
}

/** Test user name handling. */
void
TestServerUserUserManagement::testName()
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

    TS_ASSERT_THROWS_NOTHING(id = testee.add("joe random", "foo", Nothing));
    TS_ASSERT_THROWS_NOTHING(p.reset(testee.getProfileRaw(id, "screenname")));
    TS_ASSERT_EQUALS(Access(p).toString(), "joe random");
    TS_ASSERT_EQUALS(testee.getNameByUserId(id), "joe_random");

    TS_ASSERT_THROWS_NOTHING(id = testee.add("-=fancy=-", "foo", Nothing));
    TS_ASSERT_THROWS_NOTHING(p.reset(testee.getProfileRaw(id, "screenname")));
    TS_ASSERT_EQUALS(Access(p).toString(), "-=fancy=-");
    TS_ASSERT_EQUALS(testee.getNameByUserId(id), "fancy");

    TS_ASSERT_THROWS_NOTHING(id = testee.add("H4XoR", "foo", Nothing));
    TS_ASSERT_THROWS_NOTHING(p.reset(testee.getProfileRaw(id, "screenname")));
    TS_ASSERT_EQUALS(Access(p).toString(), "H4XoR");
    TS_ASSERT_EQUALS(testee.getNameByUserId(id), "h4xor");

    TS_ASSERT_THROWS_NOTHING(id = testee.add("  hi  ", "foo", Nothing));
    TS_ASSERT_THROWS_NOTHING(p.reset(testee.getProfileRaw(id, "screenname")));
    TS_ASSERT_EQUALS(Access(p).toString(), "  hi  ");
    TS_ASSERT_EQUALS(testee.getNameByUserId(id), "hi");

    TS_ASSERT_THROWS(testee.add("-=#=-", "foo", Nothing), std::exception);
    TS_ASSERT_THROWS(testee.add("", "foo", Nothing), std::exception);
}

/** Test handling blocked names. */
void
TestServerUserUserManagement::testBlockedName()
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
    TS_ASSERT_THROWS(testee.add("root", "foo", Nothing), std::exception);

    // Logging in fails
    TS_ASSERT_THROWS(testee.login("root", "foo"), std::exception);

    // Looking it up fails
    TS_ASSERT_THROWS(testee.getUserIdByName("root"), std::exception);
}

/** Test profile handling. */
void
TestServerUserUserManagement::testProfile()
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
    TS_ASSERT_EQUALS(Access(p).toString(), "Ottilie");

    // - default1 explicitly mentioned in config
    p.reset(testee.getProfileRaw(id, "default1"));
    TS_ASSERT_EQUALS(Access(p).toString(), "7");

    // - default2 taken from changed default
    p.reset(testee.getProfileRaw(id, "default2"));
    TS_ASSERT_EQUALS(Access(p).toString(), "12");

    // - copy1 taken from default:profilecopy at time of account creation
    p.reset(testee.getProfileRaw(id, "copy1"));
    TS_ASSERT_EQUALS(Access(p).toString(), "1");

    // - copy2 explicitly mentioned in config
    p.reset(testee.getProfileRaw(id, "copy2"));
    TS_ASSERT_EQUALS(Access(p).toString(), "9");

    // Verify multiple at once
    const String_t keys[] = {"default1", "copy1"};
    p.reset(testee.getProfileRaw(id, keys));
    TS_ASSERT_EQUALS(Access(p)[0].toString(), "7");
    TS_ASSERT_EQUALS(Access(p)[1].toString(), "1");
}

/** Test login(). */
void
TestServerUserUserManagement::testLogin()
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
        TS_ASSERT_EQUALS(testee.login("a_b", "z"), "1009");
        TS_ASSERT_EQUALS(testee.login("A_B", "z"), "1009");
        TS_ASSERT_EQUALS(testee.login("A->B", "z"), "1009");

        // Error cases
        TS_ASSERT_THROWS(testee.login("root", ""), std::exception);
        TS_ASSERT_THROWS(testee.login("a_b", ""), std::exception);
        TS_ASSERT_THROWS(testee.login("a_b", "zzz"), std::exception);
        TS_ASSERT_THROWS(testee.login("a_b", "Z"), std::exception);
        TS_ASSERT_THROWS(testee.login("", "Z"), std::exception);
        TS_ASSERT_THROWS(testee.login("/", "Z"), std::exception);
    }

    // Test it with different user key. This must make the test fail
    {
        server::user::ClassicEncrypter enc("abc");
        server::user::Root root(db, gen, enc, server::user::Configuration());
        server::user::UserManagement testee(root);

        TS_ASSERT_THROWS(testee.login("a_b", "z"), std::exception);
        TS_ASSERT_THROWS(testee.login("root", ""), std::exception);
    }
}

/** Test profile limitations. */
void
TestServerUserUserManagement::testProfileLimit()
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
    TS_ASSERT_THROWS_NOTHING(id = testee.add("joe_luser", "secret", config));
    TS_ASSERT_DIFFERS(id, "");

    // Verify created profile
    std::auto_ptr<afl::data::Value> p;
    TS_ASSERT_THROWS_NOTHING(p.reset(testee.getProfileRaw(id, "realname")));
    TS_ASSERT_EQUALS(Access(p).toString(), "John");
    TS_ASSERT_THROWS_NOTHING(p.reset(testee.getProfileRaw(id, "createua")));
    TS_ASSERT_EQUALS(Access(p).toString(), "wget/");  // truncated
    TS_ASSERT_THROWS_NOTHING(p.reset(testee.getProfileRaw(id, "screenname")));
    TS_ASSERT_EQUALS(Access(p).toString(), "joe_l");  // truncated

    // Update profile
    String_t update[] = { "infotown", "York",
                          "infooccupation", "Whatever" };
    TS_ASSERT_THROWS_NOTHING(testee.setProfile(id, update));
    TS_ASSERT_THROWS_NOTHING(p.reset(testee.getProfileRaw(id, "infotown")));
    TS_ASSERT_EQUALS(Access(p).toString(), "York");
    TS_ASSERT_THROWS_NOTHING(p.reset(testee.getProfileRaw(id, "infooccupation")));
    TS_ASSERT_EQUALS(Access(p).toString(), "Whate");  // truncated
}

/** Test profile limit turned off.
    Setting the limit to 0 means no limit. */
void
TestServerUserUserManagement::testProfileNoLimit()
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
    TS_ASSERT_THROWS_NOTHING(id = testee.add("joe_luser", "secret", config));
    TS_ASSERT_DIFFERS(id, "");

    // Verify created profile
    std::auto_ptr<afl::data::Value> p;
    TS_ASSERT_THROWS_NOTHING(p.reset(testee.getProfileRaw(id, "createua")));
    TS_ASSERT_EQUALS(Access(p).toString(), "wget/1.16");  // not truncated
}

/** Test profile limit at defaults. */
void
TestServerUserUserManagement::testProfileDefaultLimit()
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
    TS_ASSERT_THROWS_NOTHING(id = testee.add("joe_luser", "secret", config));
    TS_ASSERT_DIFFERS(id, "");

    // Verify created profile
    std::auto_ptr<afl::data::Value> p;
    TS_ASSERT_THROWS_NOTHING(p.reset(testee.getProfileRaw(id, "infotown")));
    TS_ASSERT_EQUALS(Access(p).toString().substr(0, 1000), String_t(1000, 'X'));  // preserve sensible start
}

void
TestServerUserUserManagement::testRemove()
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
    TS_ASSERT_THROWS_NOTHING(id = testee.add("joe", "secret", config));
    TS_ASSERT_DIFFERS(id, "");

    // Verify profile content
    TS_ASSERT_EQUALS(testee.getUserIdByName("joe"), id);
    TS_ASSERT_EQUALS(testee.getNameByUserId(id), "joe");
    TS_ASSERT_EQUALS(testee.login("joe", "secret"), id);
    std::auto_ptr<afl::data::Value> p;
    TS_ASSERT_THROWS_NOTHING(p.reset(testee.getProfileRaw(id, "screenname")));
    TS_ASSERT_EQUALS(Access(p).toString(), "Jonathan");

    // Remove the user
    TS_ASSERT_THROWS_NOTHING(testee.remove(id));
    TS_ASSERT_THROWS(testee.getUserIdByName("joe"), std::exception);
    TS_ASSERT_EQUALS(testee.getNameByUserId(id), "");
    TS_ASSERT_THROWS(testee.login("joe", "secret"), std::exception);
    TS_ASSERT_THROWS_NOTHING(p.reset(testee.getProfileRaw(id, "screenname")));
    TS_ASSERT_EQUALS(Access(p).toString(), "(joe)");
    TS_ASSERT_THROWS_NOTHING(p.reset(testee.getProfileRaw(id, "infotown")));
    TS_ASSERT_EQUALS(Access(p).toString(), "");

    // Create another joe. Must succeed and create a different Id.
    String_t id2;
    String_t config2[] = { "infotown", "Corrino", "screenname", "Joseph" };
    TS_ASSERT_THROWS_NOTHING(id2 = testee.add("joe", "secret", config2));
    TS_ASSERT_DIFFERS(id2, "");
    TS_ASSERT_DIFFERS(id2, id);
    TS_ASSERT_EQUALS(testee.getUserIdByName("joe"), id2);
    TS_ASSERT_EQUALS(testee.getNameByUserId(id2), "joe");
}

/** Test logging in when no password has been set. */
void
TestServerUserUserManagement::testLoginNoPassword()
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
    TS_ASSERT_THROWS(testee.login("a_b", "z"), std::runtime_error);
}

/** Test logging in with password upgrade. */
void
TestServerUserUserManagement::testLoginUpgrade()
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
    TS_ASSERT_EQUALS(testee.login("a_b", "z"), "1009");

    // Password has been upgraded (re-hashed with new key)
    TS_ASSERT_EQUALS(Subtree(db, "user:").subtree("1009").stringKey("password").get(), "1,2zwKRpT/uUBsg4skmgRPaQ");

    // Logging in succeeds again
    TS_ASSERT_EQUALS(testee.login("a_b", "z"), "1009");
}

