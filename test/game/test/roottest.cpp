/**
  *  \file test/game/test/roottest.cpp
  *  \brief Test for game::test::Root
  */

#include "game/test/root.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.test.Root", a)
{
    afl::base::Ref<game::Root> t = game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,2,0)), game::RegistrationKey::Registered, 8);

    a.checkEqual("01. hostVersion",         t->hostVersion().getVersion(), MKVERSION(4,2,0));
    a.checkEqual("02. key getStatus",       t->registrationKey().getStatus(), game::RegistrationKey::Registered);
    a.checkEqual("03. key getMaxTechLevel", t->registrationKey().getMaxTechLevel(game::BeamTech), 8);
}
