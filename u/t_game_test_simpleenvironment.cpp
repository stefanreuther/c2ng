/**
  *  \file u/t_game_test_simpleenvironment.cpp
  *  \brief Test for game::test::SimpleEnvironment
  */

#include <stdexcept>
#include "game/test/simpleenvironment.hpp"

#include "t_game_test.hpp"

void
TestGameTestSimpleEnvironment::testIt()
{
    game::test::SimpleEnvironment testee;

    // Empty command line
    String_t s;
    TS_ASSERT_EQUALS(testee.getCommandLine()->getNextElement(s), false);

    // Dummy invocation name and directories
    TS_ASSERT_DIFFERS(testee.getInvocationName(), "");
    TS_ASSERT_DIFFERS(testee.getSettingsDirectoryName("app"), "");
    TS_ASSERT_DIFFERS(testee.getInstallationDirectoryName(), "");

    // Empty environment
    TS_ASSERT_EQUALS(testee.getEnvironmentVariable("PATH"), "");

    // Pseudo language
    TS_ASSERT_EQUALS(testee.getUserLanguage().isValid(), true);

    // No channels
    TS_ASSERT_THROWS(testee.attachTextWriter(afl::sys::Environment::Output), std::exception);
    TS_ASSERT_THROWS(testee.attachTextReader(afl::sys::Environment::Input), std::exception);
    TS_ASSERT_THROWS(testee.attachStream(afl::sys::Environment::Output), std::exception);
}

