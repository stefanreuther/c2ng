/**
  *  \file test/game/teamsettingstest.cpp
  *  \brief Test for game::TeamSettings
  */

#include "game/teamsettings.hpp"

#include "afl/base/ref.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/counter.hpp"

/** Test initialisation.
    Object must report virgin state. */
AFL_TEST("game.TeamSettings:init", a)
{
    game::TeamSettings                          testee;
    a.check     ("01. hasAnyTeams",            !testee.hasAnyTeams());
    a.check     ("02. isNamedTeam",            !testee.isNamedTeam(9));
    a.checkEqual("03. getTeamPlayers",          testee.getTeamPlayers(9), game::PlayerSet_t(9));
    a.check     ("04. getSendConfiguration",    testee.getSendConfiguration(9).empty());
    a.check     ("05. getReceiveConfiguration", testee.getReceiveConfiguration(9).empty());
    a.checkEqual("06. getPasscode",             testee.getPasscode(), 0);

    a.check("11. getAllSendConfigurations",    !testee.getAllSendConfigurations().empty());
    a.check("12. getAllReceiveConfigurations", !testee.getAllReceiveConfigurations().empty());
}

/** Test setters and getters. */
AFL_TEST("game.TeamSettings:basics", a)
{
    afl::string::NullTranslator tx;
    game::TeamSettings testee;

    // Set
    testee.setPlayerTeam(1, 2);
    testee.setPlayerTeam(9, 2);
    testee.setPlayerTeam(8, 4);
    testee.setPlayerTeam(999999999, 2);   // out-of-range, must not crash

    // Must preserve
    a.checkEqual("01. getPlayerTeam", testee.getPlayerTeam(1), 2);
    a.checkEqual("02. getPlayerTeam", testee.getPlayerTeam(9), 2);
    a.checkEqual("03. getPlayerTeam", testee.getPlayerTeam(8), 4);
    a.checkEqual("04. getPlayerTeam", testee.getPlayerTeam(999999999), 0);  // out-of-range

    // Accessors
    a.checkEqual("11. getNumTeamMembers", testee.getNumTeamMembers(2), 3);     // 1, 2, 9
    a.checkEqual("12. getNumTeamMembers", testee.getNumTeamMembers(1), 0);
    a.checkEqual("13. getNumTeamMembers", testee.getNumTeamMembers(4), 2);     // 4, 8
    a.checkEqual("14. getTeamPlayers", testee.getTeamPlayers(2), game::PlayerSet_t() + 1 + 2 + 9);
    a.checkEqual("15. getTeamPlayers", testee.getTeamPlayers(1), game::PlayerSet_t());
    a.checkEqual("16. getTeamPlayers", testee.getTeamPlayers(4), game::PlayerSet_t() + 4 + 8);

    // Some names
    testee.setTeamName(1, "One");
    a.checkEqual("21. getTeamName", testee.getTeamName(1, tx), "One");
    a.checkEqual("22. getTeamName", testee.getTeamName(2, tx), "Team 2");
    a.check("23. isNamedTeam", testee.isNamedTeam(1));
    a.check("24. isNamedTeam", !testee.isNamedTeam(2));
    a.check("25. isNamedTeam", !testee.isNamedTeam(0));
    a.check("26. isNamedTeam", !testee.isNamedTeam(999999999));

    // We now have teams
    a.check("31. hasAnyTeams", testee.hasAnyTeams());
}

/** Test other modifications. */
AFL_TEST("game.TeamSettings:modify", a)
{
    game::TeamSettings testee;
    testee.setPlayerTeam(1, 2);       // 1,2,9 in team 2
    testee.setPlayerTeam(9, 2);
    testee.setPlayerTeam(4, 3);       // 3,4 in team 3
    testee.setPlayerTeam(8, 4);       // 8 in team 4

    // Verify counts
    a.checkEqual("01. getNumTeamMembers", testee.getNumTeamMembers(1), 0);
    a.checkEqual("02. getNumTeamMembers", testee.getNumTeamMembers(2), 3);
    a.checkEqual("03. getNumTeamMembers", testee.getNumTeamMembers(3), 2);
    a.checkEqual("04. getNumTeamMembers", testee.getNumTeamMembers(4), 1);

    // Remove player 4. Because team 4 is not available they get 1 as the first free one
    testee.removePlayerTeam(4);
    a.checkEqual("11. getPlayerTeam",     testee.getPlayerTeam(4), 1);
    a.checkEqual("12. getNumTeamMembers", testee.getNumTeamMembers(1), 1);

    // Add 4 to 4.
    testee.setPlayerTeam(4, 4);
    a.checkEqual("21. getNumTeamMembers", testee.getNumTeamMembers(1), 0);

    // Remove 8 from 4. 1 is still free, but because 8 is also free, this one goes to 8.
    testee.removePlayerTeam(8);
    a.checkEqual("31. getPlayerTeam",     testee.getPlayerTeam(8), 8);
    a.checkEqual("32. getNumTeamMembers", testee.getNumTeamMembers(4), 1);
}

/** Test viewpoint functions. */
AFL_TEST("game.TeamSettings:viewpoint", a)
{
    game::TeamSettings testee;
    testee.setPlayerTeam(1, 2);       // 1,2,9 in team 2
    testee.setPlayerTeam(9, 2);
    testee.setPlayerTeam(4, 3);       // 3,4 in team 3
    testee.setPlayerTeam(8, 4);       // 8 in team 4

    // Start with no viewpoint
    a.checkEqual("01. getViewpointPlayer", testee.getViewpointPlayer(), 0);

    // Set viewpoint
    testee.setViewpointPlayer(9);
    a.checkEqual("11. getViewpointPlayer", testee.getViewpointPlayer(), 9);

    // Relations
    a.checkEqual("21. getPlayerRelation", testee.getPlayerRelation(1), game::TeamSettings::AlliedPlayer);
    a.checkEqual("22. getPlayerRelation", testee.getPlayerRelation(2), game::TeamSettings::AlliedPlayer);
    a.checkEqual("23. getPlayerRelation", testee.getPlayerRelation(3), game::TeamSettings::EnemyPlayer);
    a.checkEqual("24. getPlayerRelation", testee.getPlayerRelation(4), game::TeamSettings::EnemyPlayer);
    a.checkEqual("25. getPlayerRelation", testee.getPlayerRelation(8), game::TeamSettings::EnemyPlayer);
    a.checkEqual("26. getPlayerRelation", testee.getPlayerRelation(9), game::TeamSettings::ThisPlayer);
    a.checkEqual("27. getPlayerRelation", testee.getPlayerRelation(10), game::TeamSettings::EnemyPlayer);

    // Colors
    a.checkEqual("31. getPlayerColor", testee.getPlayerColor(1), util::SkinColor::Yellow);
    a.checkEqual("32. getPlayerColor", testee.getPlayerColor(2), util::SkinColor::Yellow);
    a.checkEqual("33. getPlayerColor", testee.getPlayerColor(3), util::SkinColor::Red);
    a.checkEqual("34. getPlayerColor", testee.getPlayerColor(4), util::SkinColor::Red);
    a.checkEqual("35. getPlayerColor", testee.getPlayerColor(8), util::SkinColor::Red);
    a.checkEqual("36. getPlayerColor", testee.getPlayerColor(9), util::SkinColor::Green);
    a.checkEqual("37. getPlayerColor", testee.getPlayerColor(10), util::SkinColor::Red);
}

/** Test Load/Save. */
AFL_TEST("game.TeamSettings:load+save", a)
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
    AFL_CHECK_SUCCEEDS(a("01. load"), testee.load(*dir, 9, cs, tx));
    a.checkEqual("02. getTeamName", testee.getTeamName(1, tx), "human");
    a.checkEqual("03. getTeamName", testee.getTeamName(9, tx), "icke");

    // Erase the file so it is guaranteed to be written back
    dir->erase("team9.cc");
    AFL_CHECK_THROWS(a("11. file deleted"), dir->openFile("team9.cc", afl::io::FileSystem::OpenRead), std::exception);

    // Write back
    testee.save(*dir, 9, cs);

    // Verify file has been recreated with identical content
    afl::base::Ref<afl::io::Stream> file = dir->openFile("team9.cc", afl::io::FileSystem::OpenRead);
    a.checkEqual("21. file content", file->createVirtualMapping()->get().equalContent(DATA), true);
}

/** Test copyFrom(). */
AFL_TEST("game.TeamSettings:copyFrom", a)
{
    afl::string::NullTranslator tx;
    game::test::Counter counter;
    game::TeamSettings ta;
    ta.sig_teamChange.add(&counter, &game::test::Counter::increment);

    game::TeamSettings tb;
    tb.copyFrom(ta);
    a.checkEqual("01. counter", counter.get(), 0);

    tb.setTeamName(3, "three");
    tb.setPlayerTeam(7, 3);
    a.checkEqual("11. counter", counter.get(), 0);

    ta.copyFrom(tb);
    a.checkEqual("21. counter", counter.get(), 1);
    a.checkEqual("22. getPlayerTeam", ta.getPlayerTeam(7), 3);
    a.checkEqual("23. getTeamName",   ta.getTeamName(3, tx), "three");
}

/** Test setting and retrieving transfer settings. */
AFL_TEST("game.TeamSettings:transfer-settings", a)
{
    game::TeamSettings testee;

    // Set
    testee.setSendConfiguration(1, game::TeamSettings::MessageTypes_t() + game::TeamSettings::ResultAccess);
    testee.setSendConfiguration(999999999, game::TeamSettings::MessageTypes_t() + game::TeamSettings::ResultAccess);  // out-of-range, must not crash

    testee.setReceiveConfiguration(1, game::TeamSettings::MessageTypes_t() + game::TeamSettings::PlanetList);
    testee.setReceiveConfiguration(999999999, game::TeamSettings::MessageTypes_t() + game::TeamSettings::PlanetList);  // out-of-range, must not crash

    // Retrieve
    a.checkEqual("01. getSendConfiguration", testee.getSendConfiguration(1), game::TeamSettings::MessageTypes_t() + game::TeamSettings::ResultAccess);
    a.checkEqual("02. getSendConfiguration", testee.getSendConfiguration(999999999), game::TeamSettings::MessageTypes_t());

    a.checkEqual("11. getReceiveConfiguration", testee.getReceiveConfiguration(1), game::TeamSettings::MessageTypes_t() + game::TeamSettings::PlanetList);
    a.checkEqual("12. getReceiveConfiguration", testee.getReceiveConfiguration(999999999), game::TeamSettings::MessageTypes_t());

    // Passcode
    testee.setPasscode(4711);
    a.checkEqual("21. getPasscode", testee.getPasscode(), 4711);
}

/** Test synchronizeDataTransferConfigurationFromTeams(). */
AFL_TEST("game.TeamSettings:synchronizeDataTransferConfigurationFromTeams", a)
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
    a.checkEqual("01. getSendConfiguration", testee.getSendConfiguration(3), T1);
    a.checkEqual("02. getReceiveConfiguration", testee.getReceiveConfiguration(3), T2);

    a.checkEqual("11. getSendConfiguration", testee.getSendConfiguration(4), T1+T2);
    a.checkEqual("12. getReceiveConfiguration", testee.getReceiveConfiguration(4), T2+T3);

    a.checkEqual("21. getSendConfiguration", testee.getSendConfiguration(5), T1+T2);
    a.checkEqual("22. getReceiveConfiguration", testee.getReceiveConfiguration(5), T1+T2+T3);

    a.checkEqual("31. getSendConfiguration", testee.getSendConfiguration(10), T2);
    a.checkEqual("32. getReceiveConfiguration", testee.getReceiveConfiguration(10), T3);

    a.checkEqual("41. getSendConfiguration", testee.getSendConfiguration(1), T2);
    a.checkEqual("42. getReceiveConfiguration", testee.getReceiveConfiguration(1), T1+T3);
}
