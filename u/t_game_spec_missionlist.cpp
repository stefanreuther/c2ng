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

void
TestGameSpecMissionList::testMissionIni()
{
    // ex GameMissionTestSuite::testMissionIni
    using game::spec::Mission;

    // Generate a pseudo file
    const char data[] = ";22 comment\n"
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

void
TestGameSpecMissionList::testMissionIniRaces()
{
    // ex GameMissionTestSuite::testMissionIniRaces
    using game::spec::Mission;

    // Generate a pseudo file
    const char data[] =
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
