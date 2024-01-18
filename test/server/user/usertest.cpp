/**
  *  \file test/server/user/usertest.cpp
  *  \brief Test for server::user::User
  */

#include "server/user/user.hpp"

#include "afl/data/access.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/test/testrunner.hpp"
#include "server/common/numericalidgenerator.hpp"
#include "server/user/classicencrypter.hpp"
#include "server/user/root.hpp"

AFL_TEST("server.user.User", a)
{
    using afl::data::Access;

    // Prepare database
    afl::net::redis::InternalDatabase db;
    server::user::ClassicEncrypter enc("key");
    server::common::NumericalIdGenerator gen;
    server::user::Root root(db, gen, enc, server::user::Configuration());

    const char* UID = "1009";
    afl::net::redis::Subtree userTree = root.userRoot().subtree(UID);
    userTree.subtree("tokens").stringSetKey("login").add("ttkk");
    userTree.stringKey("password").set("12345");     // That's the stupidest combination I've ever heard of in my life! That's the kinda thing an idiot would have on his luggage!
    userTree.hashKey("profile").stringField("userfield").set("uservalue");
    userTree.hashKey("profile").intField("userint").set(0);

    // Test accessors
    server::user::User testee(root, UID);
    a.check("01. tokensByType", testee.tokensByType("login").contains("ttkk"));
    a.checkEqual("02. passwordHash", testee.passwordHash().get(), "12345");  // That's amazing! I've got the same combination on my luggage!

    std::auto_ptr<afl::data::Value> p;
    p.reset(testee.getProfileRaw("userfield"));
    a.checkEqual("11. userfield", Access(p).toString(), "uservalue");
    p.reset(testee.getProfileRaw("userint"));
    a.checkEqual("12. userint", Access(p).toInteger(), 0);
}
