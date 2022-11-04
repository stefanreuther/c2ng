/**
  *  \file u/t_game_teamsettings.cpp
  *  \brief Test for game::TeamSettings
  */

#include "game/teamsettings.hpp"

#include "t_game.hpp"
#include "afl/base/ref.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/test/counter.hpp"

/** Test initialisation.
    Object must report virgin state. */
void
TestGameTeamSettings::testInit()
{
    game::TeamSettings testee;
    TS_ASSERT(!testee.hasAnyTeams());
    TS_ASSERT(!testee.isNamedTeam(9));
    TS_ASSERT_EQUALS(testee.getTeamPlayers(9), game::PlayerSet_t(9));
    TS_ASSERT(testee.getSendConfiguration(9).empty());
    TS_ASSERT(testee.getReceiveConfiguration(9).empty());
    TS_ASSERT_EQUALS(testee.getPasscode(), 0);

    TS_ASSERT(!testee.getAllSendConfigurations().empty());
    TS_ASSERT(!testee.getAllReceiveConfigurations().empty());
}

/** Test setters and getters. */
void
TestGameTeamSettings::testSet()
{
    afl::string::NullTranslator tx;
    game::TeamSettings testee;

    // Set
    testee.setPlayerTeam(1, 2);
    testee.setPlayerTeam(9, 2);
    testee.setPlayerTeam(8, 4);
    testee.setPlayerTeam(999999999, 2);   // out-of-range, must not crash

    // Must preserve
    TS_ASSERT_EQUALS(testee.getPlayerTeam(1), 2);
    TS_ASSERT_EQUALS(testee.getPlayerTeam(9), 2);
    TS_ASSERT_EQUALS(testee.getPlayerTeam(8), 4);
    TS_ASSERT_EQUALS(testee.getPlayerTeam(999999999), 0);  // out-of-range

    // Accessors
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(2), 3);     // 1, 2, 9
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(1), 0);
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(4), 2);     // 4, 8
    TS_ASSERT_EQUALS(testee.getTeamPlayers(2), game::PlayerSet_t() + 1 + 2 + 9);
    TS_ASSERT_EQUALS(testee.getTeamPlayers(1), game::PlayerSet_t());
    TS_ASSERT_EQUALS(testee.getTeamPlayers(4), game::PlayerSet_t() + 4 + 8);

    // Some names
    testee.setTeamName(1, "One");
    TS_ASSERT_EQUALS(testee.getTeamName(1, tx), "One");
    TS_ASSERT_EQUALS(testee.getTeamName(2, tx), "Team 2");
    TS_ASSERT(testee.isNamedTeam(1));
    TS_ASSERT(!testee.isNamedTeam(2));
    TS_ASSERT(!testee.isNamedTeam(0));
    TS_ASSERT(!testee.isNamedTeam(999999999));

    // We now have teams
    TS_ASSERT(testee.hasAnyTeams());
}

/** Test other modifications. */
void
TestGameTeamSettings::testModify()
{
    game::TeamSettings testee;
    testee.setPlayerTeam(1, 2);       // 1,2,9 in team 2
    testee.setPlayerTeam(9, 2);
    testee.setPlayerTeam(4, 3);       // 3,4 in team 3
    testee.setPlayerTeam(8, 4);       // 8 in team 4

    // Verify counts
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(1), 0);
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(2), 3);
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(3), 2);
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(4), 1);

    // Remove player 4. Because team 4 is not available they get 1 as the first free one
    testee.removePlayerTeam(4);
    TS_ASSERT_EQUALS(testee.getPlayerTeam(4), 1);
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(1), 1);

    // Add 4 to 4.
    testee.setPlayerTeam(4, 4);
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(1), 0);

    // Remove 8 from 4. 1 is still free, but because 8 is also free, this one goes to 8.
    testee.removePlayerTeam(8);
    TS_ASSERT_EQUALS(testee.getPlayerTeam(8), 8);
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(4), 1);
}

/** Test viewpoint functions. */
void
TestGameTeamSettings::testViewpoint()
{
    game::TeamSettings testee;
    testee.setPlayerTeam(1, 2);       // 1,2,9 in team 2
    testee.setPlayerTeam(9, 2);
    testee.setPlayerTeam(4, 3);       // 3,4 in team 3
    testee.setPlayerTeam(8, 4);       // 8 in team 4

    // Start with no viewpoint
    TS_ASSERT_EQUALS(testee.getViewpointPlayer(), 0);

    // Set viewpoint
    testee.setViewpointPlayer(9);
    TS_ASSERT_EQUALS(testee.getViewpointPlayer(), 9);

    // Relations
    TS_ASSERT_EQUALS(testee.getPlayerRelation(1), game::TeamSettings::AlliedPlayer);
    TS_ASSERT_EQUALS(testee.getPlayerRelation(2), game::TeamSettings::AlliedPlayer);
    TS_ASSERT_EQUALS(testee.getPlayerRelation(3), game::TeamSettings::EnemyPlayer);
    TS_ASSERT_EQUALS(testee.getPlayerRelation(4), game::TeamSettings::EnemyPlayer);
    TS_ASSERT_EQUALS(testee.getPlayerRelation(8), game::TeamSettings::EnemyPlayer);
    TS_ASSERT_EQUALS(testee.getPlayerRelation(9), game::TeamSettings::ThisPlayer);
    TS_ASSERT_EQUALS(testee.getPlayerRelation(10), game::TeamSettings::EnemyPlayer);

    // Colors
    TS_ASSERT_EQUALS(testee.getPlayerColor(1), util::SkinColor::Yellow);
    TS_ASSERT_EQUALS(testee.getPlayerColor(2), util::SkinColor::Yellow);
    TS_ASSERT_EQUALS(testee.getPlayerColor(3), util::SkinColor::Red);
    TS_ASSERT_EQUALS(testee.getPlayerColor(4), util::SkinColor::Red);
    TS_ASSERT_EQUALS(testee.getPlayerColor(8), util::SkinColor::Red);
    TS_ASSERT_EQUALS(testee.getPlayerColor(9), util::SkinColor::Green);
    TS_ASSERT_EQUALS(testee.getPlayerColor(10), util::SkinColor::Red);
}

/** Test Load/Save. */
void
TestGameTeamSettings::testLoadSave()
{
    // An actual team.cc file created by PCC1
    static const uint8_t DATA[] = {
        0x43, 0x43, 0x74, 0x65, 0x61, 0x6d, 0x30, 0x1a, 0x03, 0x00, 0x01, 0x02, 0x05, 0x09, 0x05, 0x02, 0x05, 0x02, 0x09, 0x02, 0x09, 0x0c, 0x04, 0x04,
        0x04, 0x03, 0x04, 0x04, 0x04, 0x04, 0x03, 0x04, 0x03, 0x04, 0x05, 0x68, 0x75, 0x6d, 0x61, 0x6e, 0x12, 0x64, 0x69, 0x65, 0x20, 0x77, 0x6f, 0x20,
        0x69, 0x63, 0x68, 0x20, 0x76, 0x65, 0x72, 0x68, 0x61, 0x75, 0x65, 0x00, 0x05, 0x68, 0x75, 0x6d, 0x61, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x04, 0x69,
        0x63, 0x6b, 0x65, 0x00, 0x07, 0x6b, 0x6c, 0x65, 0x6d, 0x65, 0x6e, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("gamedir");
    dir->addStream("team9.cc", *new afl::io::ConstMemoryStream(DATA));

    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    afl::string::NullTranslator tx;

    // Test
    game::TeamSettings testee;
    TS_ASSERT_THROWS_NOTHING(testee.load(*dir, 9, cs, tx));
    TS_ASSERT_EQUALS(testee.getTeamName(1, tx), "human");
    TS_ASSERT_EQUALS(testee.getTeamName(9, tx), "icke");

    // Erase the file so it is guaranteed to be written back
    dir->erase("team9.cc");
    TS_ASSERT_THROWS(dir->openFile("team9.cc", afl::io::FileSystem::OpenRead), std::exception);

    // Write back
    testee.save(*dir, 9, cs);

    // Verify file has been recreated with identical content
    afl::base::Ref<afl::io::Stream> file = dir->openFile("team9.cc", afl::io::FileSystem::OpenRead);
    TS_ASSERT_EQUALS(file->createVirtualMapping()->get().equalContent(DATA), true);
}

/** Test copyFrom(). */
void
TestGameTeamSettings::testCopyFrom()
{
    afl::string::NullTranslator tx;
    game::test::Counter counter;
    game::TeamSettings a;
    a.sig_teamChange.add(&counter, &game::test::Counter::increment);

    game::TeamSettings b;
    b.copyFrom(a);
    TS_ASSERT_EQUALS(counter.get(), 0);

    b.setTeamName(3, "three");
    b.setPlayerTeam(7, 3);
    TS_ASSERT_EQUALS(counter.get(), 0);

    a.copyFrom(b);
    TS_ASSERT_EQUALS(counter.get(), 1);
    TS_ASSERT_EQUALS(a.getPlayerTeam(7), 3);
    TS_ASSERT_EQUALS(a.getTeamName(3, tx), "three");
}

/** Test setting and retrieving transfer settings. */
void
TestGameTeamSettings::testTransferSettings()
{
    game::TeamSettings testee;

    // Set
    testee.setSendConfiguration(1, game::TeamSettings::MessageTypes_t() + game::TeamSettings::ResultAccess);
    testee.setSendConfiguration(999999999, game::TeamSettings::MessageTypes_t() + game::TeamSettings::ResultAccess);  // out-of-range, must not crash

    testee.setReceiveConfiguration(1, game::TeamSettings::MessageTypes_t() + game::TeamSettings::PlanetList);
    testee.setReceiveConfiguration(999999999, game::TeamSettings::MessageTypes_t() + game::TeamSettings::PlanetList);  // out-of-range, must not crash

    // Retrieve
    TS_ASSERT_EQUALS(testee.getSendConfiguration(1), game::TeamSettings::MessageTypes_t() + game::TeamSettings::ResultAccess);
    TS_ASSERT_EQUALS(testee.getSendConfiguration(999999999), game::TeamSettings::MessageTypes_t());

    TS_ASSERT_EQUALS(testee.getReceiveConfiguration(1), game::TeamSettings::MessageTypes_t() + game::TeamSettings::PlanetList);
    TS_ASSERT_EQUALS(testee.getReceiveConfiguration(999999999), game::TeamSettings::MessageTypes_t());

    // Passcode
    testee.setPasscode(4711);
    TS_ASSERT_EQUALS(testee.getPasscode(), 4711);
}

/** Test synchronizeDataTransferConfigurationFromTeams(). */
void
TestGameTeamSettings::testSyncTransferSettings()
{
    const game::TeamSettings::MessageTypes_t T1(game::TeamSettings::PlanetList);
    const game::TeamSettings::MessageTypes_t T2(game::TeamSettings::ResultAccess);
    const game::TeamSettings::MessageTypes_t T3(game::TeamSettings::PlanetInformation);

    game::TeamSettings testee;

    // I am bird
    testee.setViewpointPlayer(3);
    testee.setSendConfiguration(3, T1);
    testee.setReceiveConfiguration(3, T2);

    // Team member Klingon
    testee.setPlayerTeam(4, 3);
    testee.setSendConfiguration(4, T2);
    testee.setReceiveConfiguration(4, T3);

    // Team member Orion
    testee.setPlayerTeam(5, 3);
    testee.setSendConfiguration(5, T1+T2);
    testee.setReceiveConfiguration(5, T1+T2+T3);

    // Non-team-member Rebel [same config as Klingon]
    testee.setSendConfiguration(10, T2);
    testee.setReceiveConfiguration(10, T3);

    // Non-team-member Fed [same config as Orion]
    testee.setSendConfiguration(1, T1+T2);
    testee.setReceiveConfiguration(1, T1+T2+T3);

    // Sync
    testee.synchronizeDataTransferConfigurationFromTeams();

    // Verify
    TS_ASSERT_EQUALS(testee.getSendConfiguration(3), T1);
    TS_ASSERT_EQUALS(testee.getReceiveConfiguration(3), T2);

    TS_ASSERT_EQUALS(testee.getSendConfiguration(4), T1+T2);
    TS_ASSERT_EQUALS(testee.getReceiveConfiguration(4), T2+T3);

    TS_ASSERT_EQUALS(testee.getSendConfiguration(5), T1+T2);
    TS_ASSERT_EQUALS(testee.getReceiveConfiguration(5), T1+T2+T3);

    TS_ASSERT_EQUALS(testee.getSendConfiguration(10), T2);
    TS_ASSERT_EQUALS(testee.getReceiveConfiguration(10), T3);

    TS_ASSERT_EQUALS(testee.getSendConfiguration(1), T2);
    TS_ASSERT_EQUALS(testee.getReceiveConfiguration(1), T1+T3);
}

