/**
  *  \file u/t_game_test_root.cpp
  *  \brief Test for game::test::Root
  */

#include "game/test/root.hpp"

#include "t_game_test.hpp"

void
TestGameTestRoot::testIt()
{
    afl::base::Ref<game::Root> t = game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,2,0)), game::RegistrationKey::Registered, 8);

    TS_ASSERT_EQUALS(t->hostVersion().getVersion(), MKVERSION(4,2,0));
    TS_ASSERT_EQUALS(t->registrationKey().getStatus(), game::RegistrationKey::Registered);
    TS_ASSERT_EQUALS(t->registrationKey().getMaxTechLevel(game::BeamTech), 8);
}

