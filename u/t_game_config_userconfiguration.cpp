/**
  *  \file u/t_game_config_userconfiguration.cpp
  *  \brief Test for game::config::UserConfiguration
  */

#include "game/config/userconfiguration.hpp"

#include "t_game_config.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/sys/log.hpp"
#include "game/config/markeroption.hpp"
#include "game/types.hpp"

namespace {
    afl::io::FileSystem& prepareFS(afl::io::InternalFileSystem& fs)
    {
        fs.createDirectory("/home");
        return fs;
    }

    afl::sys::Environment& prepareEnv(afl::sys::InternalEnvironment& env)
    {
        env.setSettingsDirectoryName("/home/*");
        return env;
    }

    struct ProfileEnvironment {
        afl::string::NullTranslator tx;
        afl::sys::Log log;
        afl::io::InternalFileSystem fs;
        afl::sys::InternalEnvironment env;
        util::ProfileDirectory profile;

        ProfileEnvironment()
            : tx(), log(), fs(), env(),
              profile(prepareEnv(env), prepareFS(fs), tx, log)
            { }
    };
}

/** Test defaults.
    This tests initialisation. */
void
TestGameConfigUserConfiguration::testDefaults()
{
    game::config::UserConfiguration testee;
    TS_ASSERT_EQUALS(testee[testee.Display_ThousandsSep](), 1);
    TS_ASSERT_EQUALS(testee[testee.Display_Clans](), 0);
}

/** Test getGameType(). */
void
TestGameConfigUserConfiguration::testGameType()
{
    // Uninitialized. Game type must be empty.
    {
        game::config::UserConfiguration testee;
        TS_ASSERT_EQUALS(testee.getGameType(), "");
        TS_ASSERT(testee.getOptionByName("game.type") == 0);
    }

    // Name has been set
    {
        game::config::UserConfiguration testee;
        testee.setOption("game.type", "foo", game::config::ConfigurationOption::User);
        TS_ASSERT_EQUALS(testee.getGameType(), "foo");
        TS_ASSERT(testee.getOptionByName("game.type") != 0);
    }
}

void
TestGameConfigUserConfiguration::testFormat()
{
    // Defaults: thousands separators, but no clans
    {
        game::config::UserConfiguration testee;
        TS_ASSERT_EQUALS(testee.formatNumber(1), "1");
        TS_ASSERT_EQUALS(testee.formatNumber(1000), "1,000");
        TS_ASSERT_EQUALS(testee.formatNumber(-1000), "-1,000");
        TS_ASSERT_EQUALS(testee.formatNumber(1000000), "1,000,000");
        TS_ASSERT_EQUALS(testee.formatNumber(-100000), "-100,000");
        TS_ASSERT_EQUALS(testee.formatPopulation(33), "3,300");
        TS_ASSERT_EQUALS(testee.formatPopulation(334455), "33,445,500");
        TS_ASSERT_EQUALS(testee.formatNumber(game::IntegerProperty_t(2000)), "2,000");
        TS_ASSERT_EQUALS(testee.formatNumber(game::IntegerProperty_t()), "");
        TS_ASSERT_EQUALS(testee.formatPopulation(game::IntegerProperty_t(2000)), "200,000");
        TS_ASSERT_EQUALS(testee.formatPopulation(game::IntegerProperty_t()), "");
    }

    // No thousands separators
    {
        game::config::UserConfiguration testee;
        testee[testee.Display_ThousandsSep].set(0);
        TS_ASSERT_EQUALS(testee.formatNumber(1), "1");
        TS_ASSERT_EQUALS(testee.formatNumber(1000), "1000");
        TS_ASSERT_EQUALS(testee.formatNumber(-1000), "-1000");
        TS_ASSERT_EQUALS(testee.formatNumber(1000000), "1000000");
        TS_ASSERT_EQUALS(testee.formatNumber(-100000), "-100000");
        TS_ASSERT_EQUALS(testee.formatPopulation(33), "3300");
        TS_ASSERT_EQUALS(testee.formatPopulation(334455), "33445500");
        TS_ASSERT_EQUALS(testee.formatNumber(game::IntegerProperty_t(2000)), "2000");
        TS_ASSERT_EQUALS(testee.formatNumber(game::IntegerProperty_t()), "");
        TS_ASSERT_EQUALS(testee.formatPopulation(game::IntegerProperty_t(2000)), "200000");
        TS_ASSERT_EQUALS(testee.formatPopulation(game::IntegerProperty_t()), "");
    }

    // Clans
    {
        game::config::UserConfiguration testee;
        testee[testee.Display_Clans].set(1);
        TS_ASSERT_EQUALS(testee.formatPopulation(33), "33c");
        TS_ASSERT_EQUALS(testee.formatPopulation(334455), "334,455c");
        TS_ASSERT_EQUALS(testee.formatPopulation(game::IntegerProperty_t(2000)), "2,000c");
        TS_ASSERT_EQUALS(testee.formatPopulation(game::IntegerProperty_t()), "");
    }
}

/** Test getCannedMarker().
    A: create UserConfiguration. Call getCannedMarker() with valid and invalid index.
    E: invalid index returns null; valid index returns expected value */
void
TestGameConfigUserConfiguration::testCannedMarker()
{
    game::config::UserConfiguration testee;
    TS_ASSERT(testee.getCannedMarker(-1) == 0);
    TS_ASSERT(testee.getCannedMarker(1000) == 0);

    const game::config::MarkerOptionDescriptor* opt = testee.getCannedMarker(2);
    TS_ASSERT(opt != 0);
    TS_ASSERT_EQUALS(testee[*opt]().color, 9);
    TS_ASSERT_EQUALS(testee[*opt]().markerKind, 1);
    TS_ASSERT_EQUALS(testee[*opt]().note, "");
}

/** Test saveUserConfiguration(), saveGameConfiguration() with empty configuration.
    A: create empty UserConfiguration. Save it.
    E: configurations should be empty. */
void
TestGameConfigUserConfiguration::testSaveEmpty()
{
    game::config::UserConfiguration testee;
    ProfileEnvironment env;

    // Save empty to directory
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("game");
    testee.saveGameConfiguration(*dir, env.log, env.tx);
    testee.saveUserConfiguration(env.profile, env.log, env.tx);

    // pcc2.ini in game directory must be empty
    afl::base::Ptr<afl::io::Stream> in = dir->openFileNT("pcc2.ini", afl::io::FileSystem::OpenRead);
    TS_ASSERT(in.get() != 0);
    TS_ASSERT_EQUALS(in->getSize(), 0U);

    // pcc2.ini in user directory must be empty
    in = env.fs.openFileNT("/home/PCC2/pcc2.ini", afl::io::FileSystem::OpenRead);
    TS_ASSERT(in.get() != 0);
    TS_ASSERT_EQUALS(in->getSize(), 0U);
}

/** Test saveUserConfiguration(), saveGameConfiguration() with loaded configuration.
    A: load UserConfiguration from empty directories. Save it.
    E: game configuration should be empty, user configuration should be populated. */
void
TestGameConfigUserConfiguration::testLoadSaveEmpty()
{
    game::config::UserConfiguration testee;
    ProfileEnvironment env;

    // Load, then save
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("game");
    testee.loadUserConfiguration(env.profile, env.log, env.tx);
    testee.loadGameConfiguration(*dir, env.log, env.tx);
    testee.saveGameConfiguration(*dir, env.log, env.tx);
    testee.saveUserConfiguration(env.profile, env.log, env.tx);

    // pcc2.ini in game directory must be empty
    afl::base::Ptr<afl::io::Stream> in = dir->openFileNT("pcc2.ini", afl::io::FileSystem::OpenRead);
    TS_ASSERT(in.get() != 0);
    TS_ASSERT_EQUALS(in->getSize(), 0U);

    // pcc2.ini in user directory must NOT be empty
    in = env.fs.openFileNT("/home/PCC2/pcc2.ini", afl::io::FileSystem::OpenRead);
    TS_ASSERT(in.get() != 0);
    TS_ASSERT_LESS_THAN(500U, in->getSize());
}

/** Test I/O with nonempty files.
    A: load UserConfiguration from nonempty directories.
    E: known items are converted, unknown items are preserved. Origin preserved for everything. */
void
TestGameConfigUserConfiguration::testNonEmpty()
{
    game::config::UserConfiguration testee;
    ProfileEnvironment env;

    // Set up
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("game");
    dir->openFile("pcc2.ini", afl::io::FileSystem::Create)
        ->fullWrite(afl::string::toBytes("Lock.Left = planet\n"
                                         "TestGameOption = gameValue\n"));
    env.fs.createDirectory("/home/PCC2");
    env.fs.openFile("/home/PCC2/pcc2.ini", afl::io::FileSystem::Create)
        ->fullWrite(afl::string::toBytes("unpack.racenames = accept\n"
                                         "TestUserOption = userValue\n"));

    // Load
    testee.loadUserConfiguration(env.profile, env.log, env.tx);
    testee.loadGameConfiguration(*dir, env.log, env.tx);

    // Verify
    // - options from game file
    {
        const game::config::IntegerOption* io = dynamic_cast<const game::config::IntegerOption*>(testee.getOptionByName("Lock.Left"));
        TS_ASSERT(io != 0);
        TS_ASSERT_EQUALS((*io)(), 1);
        TS_ASSERT_EQUALS(io->getSource(), game::config::ConfigurationOption::Game);
    }
    {
        const game::config::StringOption* so = dynamic_cast<const game::config::StringOption*>(testee.getOptionByName("TestGameOption"));
        TS_ASSERT(so != 0);
        TS_ASSERT_EQUALS((*so)(), "gameValue");
        TS_ASSERT_EQUALS(so->getSource(), game::config::ConfigurationOption::Game);
    }

    // - options from user file
    {
        const game::config::IntegerOption* io = dynamic_cast<const game::config::IntegerOption*>(testee.getOptionByName("Unpack.RaceNames"));
        TS_ASSERT(io != 0);
        TS_ASSERT_EQUALS((*io)(), 1);
        TS_ASSERT_EQUALS(io->getSource(), game::config::ConfigurationOption::User);
    }
    {
        const game::config::StringOption* so = dynamic_cast<const game::config::StringOption*>(testee.getOptionByName("TestUserOption"));
        TS_ASSERT(so != 0);
        TS_ASSERT_EQUALS((*so)(), "userValue");
        TS_ASSERT_EQUALS(so->getSource(), game::config::ConfigurationOption::User);
    }

    // - unset options are set to User to have them appear in user file upon save!
    {
        const game::config::IntegerOption* io = dynamic_cast<const game::config::IntegerOption*>(testee.getOptionByName("Lock.Right"));
        TS_ASSERT(io != 0);
        TS_ASSERT_DIFFERS((*io)(), 0);         // default value, but we don't care which one
        TS_ASSERT_EQUALS(io->getSource(), game::config::ConfigurationOption::User);
    }


    // Save
    testee.saveGameConfiguration(*dir, env.log, env.tx);
    testee.saveUserConfiguration(env.profile, env.log, env.tx);

    // Verify pcc2.ini in game directory
    {
        afl::base::Ptr<afl::io::Stream> in = dir->openFileNT("pcc2.ini", afl::io::FileSystem::OpenRead);
        TS_ASSERT(in.get() != 0);
        String_t content = afl::string::fromBytes(in->createVirtualMapping()->get());
        TS_ASSERT(content.find("Lock.Left = planet") != String_t::npos);
        TS_ASSERT(content.find("TestGameOption = gameValue") != String_t::npos);
    }

    // Verify pcc2.ini in user directory
    {
        afl::base::Ptr<afl::io::Stream> in = env.fs.openFileNT("/home/PCC2/pcc2.ini", afl::io::FileSystem::OpenRead);
        TS_ASSERT(in.get() != 0);
        String_t content = afl::string::fromBytes(in->createVirtualMapping()->get());
        TS_ASSERT(content.find("Unpack.RaceNames = accept") != String_t::npos);
        TS_ASSERT(content.find("TestUserOption = userValue") != String_t::npos);
    }
}

