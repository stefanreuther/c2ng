/**
  *  \file u/t_game_spec_missionlist.cpp
  *  \brief Test for game::spec::MissionList
  */

#include "game/spec/missionlist.hpp"

#include "t_game_spec.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/string.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/sys/log.hpp"
#include "game/limits.hpp"
#include "afl/string/nulltranslator.hpp"

/** Test mission.ini parsing. */
void
TestGameSpecMissionList::testMissionIni()
{
    // ex GameMissionTestSuite::testMissionIni
    using game::spec::Mission;

    // Generate a pseudo file
    static const char data[] = ";22 comment\n"
        "10 one\n"
        "11 two (I:IA)*\n"
        "12 three (T:TA)#\n"
        "13 ~four (I:IA, T:TA)*#\n"
        "14 five(T:TA, I:IA) *#\n"
        "15 six (T:TA I:IA) *#\n"
        "777 seven (whatever) *#\n";
    afl::io::ConstMemoryStream ms(afl::string::toBytes(data));
    afl::charset::CodepageCharset cp(afl::charset::g_codepageLatin1);

    // Read it
    game::spec::MissionList list;
    list.loadFromIniFile(ms, cp);

    // Must have seven missions
    TS_ASSERT_EQUALS(list.size(), 7U);
    TS_ASSERT_EQUALS(list.at(0)->getNumber(), 10);
    TS_ASSERT_EQUALS(list.at(1)->getNumber(), 11);
    TS_ASSERT_EQUALS(list.at(2)->getNumber(), 12);
    TS_ASSERT_EQUALS(list.at(3)->getNumber(), 13);
    TS_ASSERT_EQUALS(list.at(4)->getNumber(), 14);
    TS_ASSERT_EQUALS(list.at(5)->getNumber(), 15);
    TS_ASSERT_EQUALS(list.at(6)->getNumber(), 777);

    // Check names
    TS_ASSERT_EQUALS(list.at(0)->getName(), "one");
    TS_ASSERT_EQUALS(list.at(1)->getName(), "two");
    TS_ASSERT_EQUALS(list.at(2)->getName(), "three");
    TS_ASSERT_EQUALS(list.at(3)->getName(), "~four");     // tilde is not evaluated in mission.ini!
    TS_ASSERT_EQUALS(list.at(4)->getName(), "five");
    TS_ASSERT_EQUALS(list.at(5)->getName(), "six");
    TS_ASSERT_EQUALS(list.at(6)->getName(), "seven");

    // Check presence of Tow parameters
    TS_ASSERT(list.at(0)->getParameterType(game::TowParameter) == Mission::NoParameter);
    TS_ASSERT(list.at(1)->getParameterType(game::TowParameter) == Mission::NoParameter);
    TS_ASSERT(list.at(2)->getParameterType(game::TowParameter) == Mission::IntegerParameter);
    TS_ASSERT(list.at(3)->getParameterType(game::TowParameter) == Mission::IntegerParameter);
    TS_ASSERT(list.at(4)->getParameterType(game::TowParameter) == Mission::IntegerParameter);
    TS_ASSERT(list.at(5)->getParameterType(game::TowParameter) == Mission::IntegerParameter);
    TS_ASSERT(list.at(6)->getParameterType(game::TowParameter) == Mission::IntegerParameter);

    // Check presence of Intercept parameters
    TS_ASSERT(list.at(0)->getParameterType(game::InterceptParameter) == Mission::NoParameter);
    TS_ASSERT(list.at(1)->getParameterType(game::InterceptParameter) == Mission::IntegerParameter);
    TS_ASSERT(list.at(2)->getParameterType(game::InterceptParameter) == Mission::NoParameter);
    TS_ASSERT(list.at(3)->getParameterType(game::InterceptParameter) == Mission::IntegerParameter);
    TS_ASSERT(list.at(4)->getParameterType(game::InterceptParameter) == Mission::IntegerParameter);
    TS_ASSERT(list.at(5)->getParameterType(game::InterceptParameter) == Mission::IntegerParameter);
    TS_ASSERT(list.at(6)->getParameterType(game::InterceptParameter) == Mission::IntegerParameter);

    // Check names of Tow parameters
    TS_ASSERT_EQUALS(list.at(2)->getParameterName(game::TowParameter), "TA");
    TS_ASSERT_EQUALS(list.at(3)->getParameterName(game::TowParameter), "TA");
    TS_ASSERT_EQUALS(list.at(4)->getParameterName(game::TowParameter), "TA");
    TS_ASSERT_EQUALS(list.at(5)->getParameterName(game::TowParameter), "TA");
    TS_ASSERT_EQUALS(list.at(6)->getParameterName(game::TowParameter), "Tow");

    // Check names of Intercept parameters
    TS_ASSERT_EQUALS(list.at(1)->getParameterName(game::InterceptParameter), "IA");
    TS_ASSERT_EQUALS(list.at(3)->getParameterName(game::InterceptParameter), "IA");
    TS_ASSERT_EQUALS(list.at(4)->getParameterName(game::InterceptParameter), "IA");
    TS_ASSERT_EQUALS(list.at(5)->getParameterName(game::InterceptParameter), "IA");
    TS_ASSERT_EQUALS(list.at(6)->getParameterName(game::InterceptParameter), "Intercept");

    // Check hotkeys. Hotkeys are assigned in sequential order.
    TS_ASSERT_EQUALS(list.at(0)->getHotkey(), 'a');
    TS_ASSERT_EQUALS(list.at(1)->getHotkey(), 'b');
    TS_ASSERT_EQUALS(list.at(2)->getHotkey(), 'c');
    TS_ASSERT_EQUALS(list.at(3)->getHotkey(), 'd');
    TS_ASSERT_EQUALS(list.at(4)->getHotkey(), 'e');
    TS_ASSERT_EQUALS(list.at(5)->getHotkey(), 'f');
    TS_ASSERT_EQUALS(list.at(6)->getHotkey(), 'g');
}

/** Test mission.ini parsing, race handling. */
void
TestGameSpecMissionList::testMissionIniRaces()
{
    // ex GameMissionTestSuite::testMissionIniRaces
    using game::spec::Mission;

    // Generate a pseudo file
    static const char data[] =
        "10 one/1\n"
        "11 two/2 (I:IA)*\n"
        "12 three (T:TA)#/3\n"
        "13 four (I:IA, T:TA)  *# /4\n"
        "14 fi/ve(T:TA2, I:IA1) *#\n"
        "15 six/1A (T:TA I:IA) *#\n"
        "16 seven (whatever) *# /2B\n";
    afl::io::ConstMemoryStream ms(afl::string::toBytes(data));
    afl::charset::CodepageCharset cp(afl::charset::g_codepageLatin1);

    // Read it
    game::spec::MissionList list;
    list.loadFromIniFile(ms, cp);

    // Must have seven missions
    TS_ASSERT_EQUALS(list.size(), 7U);
    for (int i = 0; i < 7; ++i) {
        TS_ASSERT_EQUALS(list.at(i)->getNumber(), 10+i);
    }

    // Check names
    TS_ASSERT_EQUALS(list.at(0)->getName(), "one");
    TS_ASSERT_EQUALS(list.at(1)->getName(), "two");
    TS_ASSERT_EQUALS(list.at(2)->getName(), "three");
    TS_ASSERT_EQUALS(list.at(3)->getName(), "four");
    TS_ASSERT_EQUALS(list.at(4)->getName(), "fi/ve");
    TS_ASSERT_EQUALS(list.at(5)->getName(), "six");
    TS_ASSERT_EQUALS(list.at(6)->getName(), "seven");

    // Check presence of Tow parameters
    TS_ASSERT(list.at(0)->getParameterType(game::TowParameter) == Mission::NoParameter);
    TS_ASSERT(list.at(1)->getParameterType(game::TowParameter) == Mission::NoParameter);
    TS_ASSERT(list.at(2)->getParameterType(game::TowParameter) == Mission::IntegerParameter);
    TS_ASSERT(list.at(3)->getParameterType(game::TowParameter) == Mission::IntegerParameter);
    TS_ASSERT(list.at(4)->getParameterType(game::TowParameter) == Mission::IntegerParameter);
    TS_ASSERT(list.at(5)->getParameterType(game::TowParameter) == Mission::IntegerParameter);
    TS_ASSERT(list.at(6)->getParameterType(game::TowParameter) == Mission::IntegerParameter);

    // Check presence of Intercept parameters
    TS_ASSERT(list.at(0)->getParameterType(game::InterceptParameter) == Mission::NoParameter);
    TS_ASSERT(list.at(1)->getParameterType(game::InterceptParameter) == Mission::IntegerParameter);
    TS_ASSERT(list.at(2)->getParameterType(game::InterceptParameter) == Mission::NoParameter);
    TS_ASSERT(list.at(3)->getParameterType(game::InterceptParameter) == Mission::IntegerParameter);
    TS_ASSERT(list.at(4)->getParameterType(game::InterceptParameter) == Mission::IntegerParameter);
    TS_ASSERT(list.at(5)->getParameterType(game::InterceptParameter) == Mission::IntegerParameter);
    TS_ASSERT(list.at(6)->getParameterType(game::InterceptParameter) == Mission::IntegerParameter);

    // Check names of Tow parameters
    TS_ASSERT_EQUALS(list.at(2)->getParameterName(game::TowParameter), "TA");
    TS_ASSERT_EQUALS(list.at(3)->getParameterName(game::TowParameter), "TA");
    TS_ASSERT_EQUALS(list.at(4)->getParameterName(game::TowParameter), "TA2");
    TS_ASSERT_EQUALS(list.at(5)->getParameterName(game::TowParameter), "TA");
    TS_ASSERT_EQUALS(list.at(6)->getParameterName(game::TowParameter), "Tow");

    // Check names of Intercept parameters
    TS_ASSERT_EQUALS(list.at(1)->getParameterName(game::InterceptParameter), "IA");
    TS_ASSERT_EQUALS(list.at(3)->getParameterName(game::InterceptParameter), "IA");
    TS_ASSERT_EQUALS(list.at(4)->getParameterName(game::InterceptParameter), "IA1");
    TS_ASSERT_EQUALS(list.at(5)->getParameterName(game::InterceptParameter), "IA");
    TS_ASSERT_EQUALS(list.at(6)->getParameterName(game::InterceptParameter), "Intercept");

    // Check race masks
    TS_ASSERT_EQUALS((list.at(0)->getRaceMask().toInteger() & 0xFFE), 0x002U);
    TS_ASSERT_EQUALS((list.at(1)->getRaceMask().toInteger() & 0xFFE), 0x004U);
    TS_ASSERT_EQUALS((list.at(2)->getRaceMask().toInteger() & 0xFFE), 0x008U);
    TS_ASSERT_EQUALS((list.at(3)->getRaceMask().toInteger() & 0xFFE), 0x010U);
    TS_ASSERT_EQUALS((list.at(4)->getRaceMask().toInteger() & 0xFFE), 0xFFEU);
    TS_ASSERT_EQUALS((list.at(5)->getRaceMask().toInteger() & 0xFFE), 0x402U);
    TS_ASSERT_EQUALS((list.at(6)->getRaceMask().toInteger() & 0xFFE), 0x804U);
}

/** Test loading from mission.ini. */
void
TestGameSpecMissionList::testLoad()
{
    // File
    static const char* file =
        "; mission.cc\n"
        "s=what?\n"           // ignored assignment
        "1,,Minimal\n"
        "\n"
        "2,,Short\n"
        "i=Intercept2\n"
        "j=Tow2\n"
        "s=Short2\n"
        "C=Cond2\n"
        "t=Text2\n"
        "w=Work2\n"
        "o=Set2\n"
        "y=Ignore2\n"
        "3,+5,Full\n"
        "I=Intercept3\n"
        " J = Tow3\n"
        "shortName = Short3\n"
        "Condition = Cond3\n"
        "Text=Text3\n"
        "WILLWORK=Work3\n"
        "OnSet=Set3\n"
        "; Some ignored assignments:\n"
        "Textignore=Bad3\n"
        "Tet=Bad3\n"
        " = Bad3\n";
    afl::io::ConstMemoryStream ms(afl::string::toBytes(file));
    afl::sys::Log log;
    afl::string::NullTranslator tx;

    // Load
    game::spec::MissionList testee;
    testee.loadFromFile(ms, log, tx);

    // Verify
    TS_ASSERT_EQUALS(testee.size(), 3U);
    TS_ASSERT(testee.at(0) != 0);
    TS_ASSERT(testee.at(1) != 0);
    TS_ASSERT(testee.at(2) != 0);

    // Mission 1: Minimal, defaults
    TS_ASSERT_EQUALS(testee.at(0)->getNumber(), 1);
    TS_ASSERT_EQUALS(testee.at(0)->getName(), "Minimal");
    TS_ASSERT_EQUALS(testee.at(0)->getRaceMask(), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS));
    TS_ASSERT_EQUALS(testee.at(0)->getParameterName(game::InterceptParameter), "Intercept");
    TS_ASSERT_EQUALS(testee.at(0)->getParameterName(game::TowParameter), "Tow");
    TS_ASSERT_EQUALS(testee.at(0)->getConditionExpression(), "");
    TS_ASSERT_EQUALS(testee.at(0)->getWarningExpression(), "");
    TS_ASSERT_EQUALS(testee.at(0)->getLabelExpression(), "");
    TS_ASSERT_EQUALS(testee.at(0)->getSetCommand(), "");

    // Mission 2: Short, everything assigned using one-letter names
    TS_ASSERT_EQUALS(testee.at(1)->getNumber(), 2);
    TS_ASSERT_EQUALS(testee.at(1)->getName(), "Short");
    TS_ASSERT_EQUALS(testee.at(1)->getRaceMask(), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS));
    TS_ASSERT_EQUALS(testee.at(1)->getParameterName(game::InterceptParameter), "Intercept2");
    TS_ASSERT_EQUALS(testee.at(1)->getParameterName(game::TowParameter), "Tow2");
    TS_ASSERT_EQUALS(testee.at(1)->getConditionExpression(), "Cond2");
    TS_ASSERT_EQUALS(testee.at(1)->getWarningExpression(), "Work2");
    TS_ASSERT_EQUALS(testee.at(1)->getLabelExpression(), "Text2");
    TS_ASSERT_EQUALS(testee.at(1)->getSetCommand(), "Set2");

    // Mission 3: Full, everything assigned using full names
    TS_ASSERT_EQUALS(testee.at(2)->getNumber(), 3);
    TS_ASSERT_EQUALS(testee.at(2)->getName(), "Full");
    TS_ASSERT_EQUALS(testee.at(2)->getRaceMask(), game::PlayerSet_t(5));
    TS_ASSERT_EQUALS(testee.at(2)->getParameterName(game::InterceptParameter), "Intercept3");
    TS_ASSERT_EQUALS(testee.at(2)->getParameterName(game::TowParameter), "Tow3");
    TS_ASSERT_EQUALS(testee.at(2)->getConditionExpression(), "Cond3");
    TS_ASSERT_EQUALS(testee.at(2)->getWarningExpression(), "Work3");
    TS_ASSERT_EQUALS(testee.at(2)->getLabelExpression(), "Text3");
    TS_ASSERT_EQUALS(testee.at(2)->getSetCommand(), "Set3");
}

/** Test addMission(), merge missions, and, implicitly, sort(). */
void
TestGameSpecMissionList::testAddMerge()
{
    using game::spec::Mission;

    game::spec::MissionList testee;

    // Add some "mission.cc" missions
    testee.addMission(Mission(1, ",Explore"));
    testee.addMission(Mission(9, "+1,Special 1"));
    testee.addMission(Mission(9, "+2,Special 2"));
    testee.addMission(Mission(9, "+3,Special 3"));

    // Add some "mission.ini" missions
    testee.addMission(Mission(1, ",Other Explore"));
    testee.addMission(Mission(4, ",Kill"));
    testee.addMission(Mission(9, ",Special"));

    // Sort
    testee.sort();

    TS_ASSERT_EQUALS(testee.size(), 5U);
    TS_ASSERT_EQUALS(testee.at(0)->getNumber(), 1);
    TS_ASSERT_EQUALS(testee.at(0)->getName(), "Explore");
    TS_ASSERT_EQUALS(testee.at(1)->getNumber(), 4);
    TS_ASSERT_EQUALS(testee.at(1)->getName(), "Kill");
    TS_ASSERT_EQUALS(testee.at(2)->getNumber(), 9);
    TS_ASSERT_EQUALS(testee.at(2)->getName(), "Special 1");
    TS_ASSERT_EQUALS(testee.at(2)->getRaceMask(), game::PlayerSet_t(1));
    TS_ASSERT_EQUALS(testee.at(3)->getNumber(), 9);
    TS_ASSERT_EQUALS(testee.at(3)->getName(), "Special 2");
    TS_ASSERT_EQUALS(testee.at(3)->getRaceMask(), game::PlayerSet_t(2));
    TS_ASSERT_EQUALS(testee.at(4)->getNumber(), 9);
    TS_ASSERT_EQUALS(testee.at(4)->getName(), "Special 3");
    TS_ASSERT_EQUALS(testee.at(4)->getRaceMask(), game::PlayerSet_t(3));

    // Test lookup
    const Mission* p;
    p = testee.findMissionByNumber(1, game::PlayerSet_t(1));
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->getName(), "Explore");

    p = testee.findMissionByNumber(9, game::PlayerSet_t(1));
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->getName(), "Special 1");

    p = testee.findMissionByNumber(9, game::PlayerSet_t(4));
    TS_ASSERT(p == 0);

    // Test position lookup
    size_t index = 9999;
    TS_ASSERT(testee.findIndexByNumber(1, game::PlayerSet_t(3)).get(index));
    TS_ASSERT_EQUALS(index, 0U);

    TS_ASSERT(testee.findIndexByNumber(9, game::PlayerSet_t(2)).get(index));
    TS_ASSERT_EQUALS(index, 3U);

    TS_ASSERT(!testee.findIndexByNumber(9, game::PlayerSet_t(4)).isValid());
}

/** Test addMission(), letter assignment. */
void
TestGameSpecMissionList::testAddMergeLetters()
{
    using game::spec::Mission;

    // Assign many missions
    game::spec::MissionList testee;
    for (int i = 0; i < 30; ++i) {
        testee.addMission(Mission(20+i, ",egal"));
    }
    TS_ASSERT_EQUALS(testee.size(), 30U);
    TS_ASSERT_EQUALS(testee.at(0)->getHotkey(), 'a');
    TS_ASSERT_EQUALS(testee.at(1)->getHotkey(), 'b');
    TS_ASSERT_EQUALS(testee.at(25)->getHotkey(), 'z');
    TS_ASSERT_EQUALS(testee.at(26)->getHotkey(), 'a');

    // Clear and add anew
    testee.clear();
    for (int i = 0; i < 5; ++i) {
        testee.addMission(Mission(20+i, ",egal"));
    }
    TS_ASSERT_EQUALS(testee.size(), 5U);
    TS_ASSERT_EQUALS(testee.at(0)->getHotkey(), 'a');
    TS_ASSERT_EQUALS(testee.at(1)->getHotkey(), 'b');
    TS_ASSERT_EQUALS(testee.at(4)->getHotkey(), 'e');
}

/** Test addMission(), letter assignment. */
void
TestGameSpecMissionList::testAddMergeLetters2()
{
    using game::spec::Mission;

    // Preload, then assign many missions
    game::spec::MissionList testee;
    testee.addMission(Mission(98, ",~kill"));
    testee.addMission(Mission(99, ",~jump"));
    for (int i = 0; i < 40; ++i) {
        testee.addMission(Mission(i, ",egal"));
    }
    TS_ASSERT_EQUALS(testee.size(), 42U);

    // Manually assigned:
    TS_ASSERT_EQUALS(testee.at(0)->getHotkey(), 'k');
    TS_ASSERT_EQUALS(testee.at(1)->getHotkey(), 'j');

    // Auto-assigned:
    TS_ASSERT_EQUALS(testee.at(2)->getHotkey(), '0');
    TS_ASSERT_EQUALS(testee.at(3)->getHotkey(), '1');
    TS_ASSERT_EQUALS(testee.at(11)->getHotkey(), '9');

    TS_ASSERT_EQUALS(testee.at(12)->getHotkey(), 'a');
    TS_ASSERT_EQUALS(testee.at(13)->getHotkey(), 'b');

    TS_ASSERT_EQUALS(testee.at(20)->getHotkey(), 'i');
    TS_ASSERT_EQUALS(testee.at(21)->getHotkey(), 'l');

    TS_ASSERT_EQUALS(testee.at(35)->getHotkey(), 'z');
    TS_ASSERT_EQUALS(testee.at(36)->getHotkey(), 'a');
}

