/**
  *  \file test/game/config/userconfigurationtest.cpp
  *  \brief Test for game::config::UserConfiguration
  */

#include "game/config/userconfiguration.hpp"

#include "afl/io/filemapping.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
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
              profile(prepareEnv(env), prepareFS(fs))
            { }
    };
}

/** Test defaults.
    This tests initialisation. */
AFL_TEST("game.config.UserConfiguration:defaults", a)
{
    game::config::UserConfiguration testee;
    a.checkEqual("Display_ThousandsSep", testee[testee.Display_ThousandsSep](), 1);
    a.checkEqual("Display_Clans",        testee[testee.Display_Clans](), 0);
}

/*
 *  getGameType
 */

// Uninitialized. Game type must be empty.
AFL_TEST("game.config.UserConfiguration:getGameType:uninit", a)
{
    game::config::UserConfiguration testee;
    a.checkEqual("getGameType", testee.getGameType(), "");
    a.checkNull("getOptionByName", testee.getOptionByName("game.type"));
}

// Name has been set
AFL_TEST("game.config.UserConfiguration:getGameType:init", a)
{
    game::config::UserConfiguration testee;
    testee.setOption("game.type", "foo", game::config::ConfigurationOption::User);
    a.checkEqual("getGameType", testee.getGameType(), "foo");
    a.checkNonNull("getOptionByName", testee.getOptionByName("game.type"));
}

/*
 *  Formatting
 */

// Defaults: thousands separators, but no clans
AFL_TEST("game.config.UserConfiguration:format:default", a)
{
    game::config::UserConfiguration testee;
    a.checkEqual("01", testee.formatNumber(1), "1");
    a.checkEqual("02", testee.formatNumber(1000), "1,000");
    a.checkEqual("03", testee.formatNumber(-1000), "-1,000");
    a.checkEqual("04", testee.formatNumber(1000000), "1,000,000");
    a.checkEqual("05", testee.formatNumber(-100000), "-100,000");
    a.checkEqual("06", testee.formatPopulation(33), "3,300");
    a.checkEqual("07", testee.formatPopulation(334455), "33,445,500");
    a.checkEqual("08", testee.formatNumber(game::IntegerProperty_t(2000)), "2,000");
    a.checkEqual("09", testee.formatNumber(game::IntegerProperty_t()), "");
    a.checkEqual("10", testee.formatPopulation(game::IntegerProperty_t(2000)), "200,000");
    a.checkEqual("11", testee.formatPopulation(game::IntegerProperty_t()), "");
}

// No thousands separators
AFL_TEST("game.config.UserConfiguration:format:no-thousands-separator", a)
{
    game::config::UserConfiguration testee;
    testee[testee.Display_ThousandsSep].set(0);
    a.checkEqual("21", testee.formatNumber(1), "1");
    a.checkEqual("22", testee.formatNumber(1000), "1000");
    a.checkEqual("23", testee.formatNumber(-1000), "-1000");
    a.checkEqual("24", testee.formatNumber(1000000), "1000000");
    a.checkEqual("25", testee.formatNumber(-100000), "-100000");
    a.checkEqual("26", testee.formatPopulation(33), "3300");
    a.checkEqual("27", testee.formatPopulation(334455), "33445500");
    a.checkEqual("28", testee.formatNumber(game::IntegerProperty_t(2000)), "2000");
    a.checkEqual("29", testee.formatNumber(game::IntegerProperty_t()), "");
    a.checkEqual("30", testee.formatPopulation(game::IntegerProperty_t(2000)), "200000");
    a.checkEqual("31", testee.formatPopulation(game::IntegerProperty_t()), "");
}

// Clans
AFL_TEST("game.config.UserConfiguration:format:clans", a)
{
    game::config::UserConfiguration testee;
    testee[testee.Display_Clans].set(1);
    a.checkEqual("41", testee.formatPopulation(33), "33c");
    a.checkEqual("42", testee.formatPopulation(334455), "334,455c");
    a.checkEqual("43", testee.formatPopulation(game::IntegerProperty_t(2000)), "2,000c");
    a.checkEqual("44", testee.formatPopulation(game::IntegerProperty_t()), "");
}

/** Test getCannedMarker().
    A: create UserConfiguration. Call getCannedMarker() with valid and invalid index.
    E: invalid index returns null; valid index returns expected value */
AFL_TEST("game.config.UserConfiguration:getCannedMarker", a)
{
    game::config::UserConfiguration testee;
    a.checkNull("01. wrong index", testee.getCannedMarker(-1));
    a.checkNull("02. wrong index", testee.getCannedMarker(1000));

    const game::config::MarkerOptionDescriptor* opt = testee.getCannedMarker(2);
    a.checkNonNull("11. getCannedMarker", opt);
    a.checkEqual("12. color", testee[*opt]().color, 9);
    a.checkEqual("13. markerKind", testee[*opt]().markerKind, 1);
    a.checkEqual("14. note", testee[*opt]().note, "");
}

/** Test saveUserConfiguration(), saveGameConfiguration() with empty configuration.
    A: create empty UserConfiguration. Save it.
    E: configurations should be empty. */
AFL_TEST("game.config.UserConfiguration:save:empty", a)
{
    game::config::UserConfiguration testee;
    ProfileEnvironment env;

    // Save empty to directory
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("game");
    testee.saveGameConfiguration(*dir, env.log, env.tx);
    testee.saveUserConfiguration(env.profile, env.log, env.tx);

    // pcc2.ini in game directory must be empty
    afl::base::Ptr<afl::io::Stream> in = dir->openFileNT("pcc2.ini", afl::io::FileSystem::OpenRead);
    a.checkNonNull("01. game file", in.get());
    a.checkEqual("02. game size", in->getSize(), 0U);

    // pcc2.ini in user directory must be empty
    in = env.fs.openFileNT("/home/PCC2/pcc2.ini", afl::io::FileSystem::OpenRead);
    a.checkNonNull("11. user file", in.get());
    a.checkEqual("12. user size", in->getSize(), 0U);
}

/** Test saveUserConfiguration(), saveGameConfiguration() with loaded configuration.
    A: load UserConfiguration from empty directories. Save it.
    E: game configuration should be empty, user configuration should be populated. */
AFL_TEST("game.config.UserConfiguration:save:previously-loaded", a)
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
    a.checkNonNull("01. game file", in.get());
    a.checkEqual("02. game size", in->getSize(), 0U);

    // pcc2.ini in user directory must NOT be empty
    in = env.fs.openFileNT("/home/PCC2/pcc2.ini", afl::io::FileSystem::OpenRead);
    a.checkNonNull("11. user file", in.get());
    a.check("12. user size", in->getSize() >= 500);
}

/** Test I/O with nonempty files.
    A: load UserConfiguration from nonempty directories.
    E: known items are converted, unknown items are preserved. Origin preserved for everything. */
AFL_TEST("game.config.UserConfiguration:load", a)
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
        a.checkNonNull("Lock.Left", io);
        a.checkEqual("Lock.Left value", (*io)(), 1);
        a.checkEqual("Lock.Left source", io->getSource(), game::config::ConfigurationOption::Game);
    }
    {
        const game::config::StringOption* so = dynamic_cast<const game::config::StringOption*>(testee.getOptionByName("TestGameOption"));
        a.checkNonNull("TestGameOption", so);
        a.checkEqual("TestGameOption value", (*so)(), "gameValue");
        a.checkEqual("TestGameOption source", so->getSource(), game::config::ConfigurationOption::Game);
    }

    // - options from user file
    {
        const game::config::IntegerOption* io = dynamic_cast<const game::config::IntegerOption*>(testee.getOptionByName("Unpack.RaceNames"));
        a.checkNonNull("Unpack.RaceNames", io);
        a.checkEqual("Unpack.RaceNames value", (*io)(), 1);
        a.checkEqual("Unpack.RaceNames source", io->getSource(), game::config::ConfigurationOption::User);
    }
    {
        const game::config::StringOption* so = dynamic_cast<const game::config::StringOption*>(testee.getOptionByName("TestUserOption"));
        a.checkNonNull("TestUserOption", so);
        a.checkEqual("TestUserOption value", (*so)(), "userValue");
        a.checkEqual("TestUserOption source", so->getSource(), game::config::ConfigurationOption::User);
    }

    // - unset options are set to User to have them appear in user file upon save!
    {
        const game::config::IntegerOption* io = dynamic_cast<const game::config::IntegerOption*>(testee.getOptionByName("Lock.Right"));
        a.checkNonNull("Lock.Right", io);
        a.checkDifferent("Lock.Right value", (*io)(), 0);         // default value, but we don't care which one
        a.checkEqual("Lock.Right source", io->getSource(), game::config::ConfigurationOption::User);
    }


    // Save
    testee.saveGameConfiguration(*dir, env.log, env.tx);
    testee.saveUserConfiguration(env.profile, env.log, env.tx);

    // Verify pcc2.ini in game directory
    {
        afl::base::Ptr<afl::io::Stream> in = dir->openFileNT("pcc2.ini", afl::io::FileSystem::OpenRead);
        a.checkNonNull("Game file", in.get());
        String_t content = afl::string::fromBytes(in->createVirtualMapping()->get());
        a.check("Game option: Lock.Left", content.find("Lock.Left = planet") != String_t::npos);
        a.check("Game option: TestGameOption", content.find("TestGameOption = gameValue") != String_t::npos);
    }

    // Verify pcc2.ini in user directory
    {
        afl::base::Ptr<afl::io::Stream> in = env.fs.openFileNT("/home/PCC2/pcc2.ini", afl::io::FileSystem::OpenRead);
        a.checkNonNull("User file", in.get());
        String_t content = afl::string::fromBytes(in->createVirtualMapping()->get());
        a.check("User option: Unpack.RaceNames", content.find("Unpack.RaceNames = accept") != String_t::npos);
        a.check("User option: TestUserOption", content.find("TestUserOption = userValue") != String_t::npos);
    }
}
