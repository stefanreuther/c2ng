/**
  *  \file u/t_server_user_user.cpp
  *  \brief Test for server::user::User
  */

#include "server/user/user.hpp"

#include "t_server_user.hpp"
#include "afl/data/access.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "server/common/numericalidgenerator.hpp"
#include "server/user/classicencrypter.hpp"
#include "server/user/root.hpp"

void
TestServerUserUser::testAccessors()
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
    TS_ASSERT(testee.tokensByType("login").contains("ttkk"));
    TS_ASSERT_EQUALS(testee.passwordHash().get(), "12345");  // That's amazing! I've got the same combination on my luggage!

    std::auto_ptr<afl::data::Value> p;
    p.reset(testee.getProfileRaw("userfield"));
    TS_ASSERT_EQUALS(Access(p).toString(), "uservalue");
    p.reset(testee.getProfileRaw("userint"));
    TS_ASSERT_EQUALS(Access(p).toInteger(), 0);
}

