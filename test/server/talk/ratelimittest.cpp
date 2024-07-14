/**
  *  \file test/server/talk/ratelimittest.cpp
  *  \brief Test for server::talk::RateLimit
  */

#include "server/talk/ratelimit.hpp"

#include "afl/net/redis/internaldatabase.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "server/talk/configuration.hpp"
#include "server/talk/root.hpp"
#include "server/talk/user.hpp"

using afl::net::redis::InternalDatabase;
using afl::sys::Log;
using server::talk::Configuration;
using server::talk::Root;
using server::talk::User;

namespace {
    struct Environment {
        Configuration& config;
        Log log;
        InternalDatabase db;
        Root root;
        User user;

        Environment(Configuration& config)
            : config(config), log(), db(), root(db, config), user(root, "1001")
            { }
    };
}

/** Check that defaults are sane. */
AFL_TEST("server.talk.RateLimit:default", a)
{
    Configuration config;
    Environment env(config);

    a.check("01", checkRateLimit(10, 1000, config, env.user, env.log));

    a.checkEqual("11", env.user.rateTime().get(), 1000);
}

/** Check cooldown handling. */
AFL_TEST("server.talk.RateLimit:cooldown", a)
{
    Configuration config;
    config.rateMaximum = 100;
    config.rateCooldown = 50;
    config.rateInterval = 50;

    Environment env(config);
    env.user.rateTime().set(800);
    env.user.rateScore().set(100);

    a.check("01", checkRateLimit(10, 870, config, env.user, env.log));

    a.checkEqual("11", env.user.rateTime().get(), 870);
    a.checkEqual("12", env.user.rateScore().get(), 40);
}

/** Check cooldown handling: insufficient time elapsed. */
AFL_TEST("server.talk.RateLimit:cooldown:insufficient", a)
{
    Configuration config;
    config.rateMaximum = 100;
    config.rateCooldown = 50;
    config.rateInterval = 50;

    Environment env(config);
    env.user.rateTime().set(800);
    env.user.rateScore().set(100);

    a.check("01", !checkRateLimit(10, 805, config, env.user, env.log));

    a.checkEqual("11", env.user.rateTime().get(), 805);
    a.checkEqual("12", env.user.rateScore().get(), 100);
}

/** Check cooldown handling. */
AFL_TEST("server.talk.RateLimit:cooldown:long", a)
{
    Configuration config;
    config.rateMaximum = 100;
    config.rateMinimum = -200;
    config.rateCooldown = 50;
    config.rateInterval = 50;

    Environment env(config);
    env.user.rateTime().set(800);
    env.user.rateScore().set(100);

    a.check("01", checkRateLimit(10, 5000, config, env.user, env.log));

    a.checkEqual("11", env.user.rateTime().get(), 5000);
    a.checkEqual("12", env.user.rateScore().get(), -190);
}
