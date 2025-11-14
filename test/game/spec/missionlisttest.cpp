/**
  *  \file test/game/spec/missionlisttest.cpp
  *  \brief Test for game::spec::MissionList
  */

#include "game/spec/missionlist.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/string/string.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/hostversion.hpp"
#include "game/limits.hpp"

using afl::base::Ref;
using game::HostVersion;
using game::config::HostConfiguration;
using game::spec::Mission;
using game::spec::MissionList;

namespace {
    void checkEntry(afl::test::Assert a, const util::StringList& list, size_t index, int32_t expectKey, String_t expectText)
    {
        String_t s;
        int32_t i;
        a.check("get", list.get(index, i, s));
        a.checkEqual("str", s, expectText);
        a.checkEqual("key", i, expectKey);
    }
}

/** Test mission.ini parsing. */
AFL_TEST("game.spec.MissionList:loadFromIniFile", a)
{
    // ex GameMissionTestSuite::testMissionIni
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
    Ref<MissionList> list = MissionList::create();
    list->loadFromIniFile(ms, cp);

    // Must have seven missions
    a.checkEqual("01. size", list->size(), 7U);
    a.checkEqual("02. getNumber", list->at(0)->getNumber(), 10);
    a.checkEqual("03. getNumber", list->at(1)->getNumber(), 11);
    a.checkEqual("04. getNumber", list->at(2)->getNumber(), 12);
    a.checkEqual("05. getNumber", list->at(3)->getNumber(), 13);
    a.checkEqual("06. getNumber", list->at(4)->getNumber(), 14);
    a.checkEqual("07. getNumber", list->at(5)->getNumber(), 15);
    a.checkEqual("08. getNumber", list->at(6)->getNumber(), 777);

    // Check names
    a.checkEqual("11. getName", list->at(0)->getName(), "one");
    a.checkEqual("12. getName", list->at(1)->getName(), "two");
    a.checkEqual("13. getName", list->at(2)->getName(), "three");
    a.checkEqual("14. getName", list->at(3)->getName(), "~four");     // tilde is not evaluated in mission.ini!
    a.checkEqual("15. getName", list->at(4)->getName(), "five");
    a.checkEqual("16. getName", list->at(5)->getName(), "six");
    a.checkEqual("17. getName", list->at(6)->getName(), "seven");

    // Check presence of Tow parameters
    a.checkEqual("21. getParameterType", list->at(0)->getParameterType(game::TowParameter), Mission::NoParameter);
    a.checkEqual("22. getParameterType", list->at(1)->getParameterType(game::TowParameter), Mission::NoParameter);
    a.checkEqual("23. getParameterType", list->at(2)->getParameterType(game::TowParameter), Mission::IntegerParameter);
    a.checkEqual("24. getParameterType", list->at(3)->getParameterType(game::TowParameter), Mission::IntegerParameter);
    a.checkEqual("25. getParameterType", list->at(4)->getParameterType(game::TowParameter), Mission::IntegerParameter);
    a.checkEqual("26. getParameterType", list->at(5)->getParameterType(game::TowParameter), Mission::IntegerParameter);
    a.checkEqual("27. getParameterType", list->at(6)->getParameterType(game::TowParameter), Mission::IntegerParameter);

    // Check presence of Intercept parameters
    a.checkEqual("31. getParameterType", list->at(0)->getParameterType(game::InterceptParameter), Mission::NoParameter);
    a.checkEqual("32. getParameterType", list->at(1)->getParameterType(game::InterceptParameter), Mission::IntegerParameter);
    a.checkEqual("33. getParameterType", list->at(2)->getParameterType(game::InterceptParameter), Mission::NoParameter);
    a.checkEqual("34. getParameterType", list->at(3)->getParameterType(game::InterceptParameter), Mission::IntegerParameter);
    a.checkEqual("35. getParameterType", list->at(4)->getParameterType(game::InterceptParameter), Mission::IntegerParameter);
    a.checkEqual("36. getParameterType", list->at(5)->getParameterType(game::InterceptParameter), Mission::IntegerParameter);
    a.checkEqual("37. getParameterType", list->at(6)->getParameterType(game::InterceptParameter), Mission::IntegerParameter);

    // Check names of Tow parameters
    a.checkEqual("41. getParameterName", list->at(2)->getParameterName(game::TowParameter), "TA");
    a.checkEqual("42. getParameterName", list->at(3)->getParameterName(game::TowParameter), "TA");
    a.checkEqual("43. getParameterName", list->at(4)->getParameterName(game::TowParameter), "TA");
    a.checkEqual("44. getParameterName", list->at(5)->getParameterName(game::TowParameter), "TA");
    a.checkEqual("45. getParameterName", list->at(6)->getParameterName(game::TowParameter), "Tow");

    // Check names of Intercept parameters
    a.checkEqual("51. getParameterName", list->at(1)->getParameterName(game::InterceptParameter), "IA");
    a.checkEqual("52. getParameterName", list->at(3)->getParameterName(game::InterceptParameter), "IA");
    a.checkEqual("53. getParameterName", list->at(4)->getParameterName(game::InterceptParameter), "IA");
    a.checkEqual("54. getParameterName", list->at(5)->getParameterName(game::InterceptParameter), "IA");
    a.checkEqual("55. getParameterName", list->at(6)->getParameterName(game::InterceptParameter), "Intercept");

    // Check hotkeys. Hotkeys are assigned in sequential order.
    a.checkEqual("61. getHotkey", list->at(0)->getHotkey(), 'a');
    a.checkEqual("62. getHotkey", list->at(1)->getHotkey(), 'b');
    a.checkEqual("63. getHotkey", list->at(2)->getHotkey(), 'c');
    a.checkEqual("64. getHotkey", list->at(3)->getHotkey(), 'd');
    a.checkEqual("65. getHotkey", list->at(4)->getHotkey(), 'e');
    a.checkEqual("66. getHotkey", list->at(5)->getHotkey(), 'f');
    a.checkEqual("67. getHotkey", list->at(6)->getHotkey(), 'g');
}

/** Test mission.ini parsing, race handling. */
AFL_TEST("game.spec.MissionList:loadFromIniFile:races", a)
{
    // ex GameMissionTestSuite::testMissionIniRaces
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
    Ref<MissionList> list = MissionList::create();
    list->loadFromIniFile(ms, cp);

    // Must have seven missions
    a.checkEqual("01. size", list->size(), 7U);
    for (int i = 0; i < 7; ++i) {
        a.checkEqual("02. getNumber", list->at(i)->getNumber(), 10+i);
    }

    // Check names
    a.checkEqual("11. getName", list->at(0)->getName(), "one");
    a.checkEqual("12. getName", list->at(1)->getName(), "two");
    a.checkEqual("13. getName", list->at(2)->getName(), "three");
    a.checkEqual("14. getName", list->at(3)->getName(), "four");
    a.checkEqual("15. getName", list->at(4)->getName(), "fi/ve");
    a.checkEqual("16. getName", list->at(5)->getName(), "six");
    a.checkEqual("17. getName", list->at(6)->getName(), "seven");

    // Check presence of Tow parameters
    a.checkEqual("21. getParameterType", list->at(0)->getParameterType(game::TowParameter), Mission::NoParameter);
    a.checkEqual("22. getParameterType", list->at(1)->getParameterType(game::TowParameter), Mission::NoParameter);
    a.checkEqual("23. getParameterType", list->at(2)->getParameterType(game::TowParameter), Mission::IntegerParameter);
    a.checkEqual("24. getParameterType", list->at(3)->getParameterType(game::TowParameter), Mission::IntegerParameter);
    a.checkEqual("25. getParameterType", list->at(4)->getParameterType(game::TowParameter), Mission::IntegerParameter);
    a.checkEqual("26. getParameterType", list->at(5)->getParameterType(game::TowParameter), Mission::IntegerParameter);
    a.checkEqual("27. getParameterType", list->at(6)->getParameterType(game::TowParameter), Mission::IntegerParameter);

    // Check presence of Intercept parameters
    a.checkEqual("31. getParameterType", list->at(0)->getParameterType(game::InterceptParameter), Mission::NoParameter);
    a.checkEqual("32. getParameterType", list->at(1)->getParameterType(game::InterceptParameter), Mission::IntegerParameter);
    a.checkEqual("33. getParameterType", list->at(2)->getParameterType(game::InterceptParameter), Mission::NoParameter);
    a.checkEqual("34. getParameterType", list->at(3)->getParameterType(game::InterceptParameter), Mission::IntegerParameter);
    a.checkEqual("35. getParameterType", list->at(4)->getParameterType(game::InterceptParameter), Mission::IntegerParameter);
    a.checkEqual("36. getParameterType", list->at(5)->getParameterType(game::InterceptParameter), Mission::IntegerParameter);
    a.checkEqual("37. getParameterType", list->at(6)->getParameterType(game::InterceptParameter), Mission::IntegerParameter);

    // Check names of Tow parameters
    a.checkEqual("41. getParameterName", list->at(2)->getParameterName(game::TowParameter), "TA");
    a.checkEqual("42. getParameterName", list->at(3)->getParameterName(game::TowParameter), "TA");
    a.checkEqual("43. getParameterName", list->at(4)->getParameterName(game::TowParameter), "TA2");
    a.checkEqual("44. getParameterName", list->at(5)->getParameterName(game::TowParameter), "TA");
    a.checkEqual("45. getParameterName", list->at(6)->getParameterName(game::TowParameter), "Tow");

    // Check names of Intercept parameters
    a.checkEqual("51. getParameterName", list->at(1)->getParameterName(game::InterceptParameter), "IA");
    a.checkEqual("52. getParameterName", list->at(3)->getParameterName(game::InterceptParameter), "IA");
    a.checkEqual("53. getParameterName", list->at(4)->getParameterName(game::InterceptParameter), "IA1");
    a.checkEqual("54. getParameterName", list->at(5)->getParameterName(game::InterceptParameter), "IA");
    a.checkEqual("55. getParameterName", list->at(6)->getParameterName(game::InterceptParameter), "Intercept");

    // Check race masks
    a.checkEqual("61. getRaceMask", (list->at(0)->getRaceMask().toInteger() & 0xFFE), 0x002U);
    a.checkEqual("62. getRaceMask", (list->at(1)->getRaceMask().toInteger() & 0xFFE), 0x004U);
    a.checkEqual("63. getRaceMask", (list->at(2)->getRaceMask().toInteger() & 0xFFE), 0x008U);
    a.checkEqual("64. getRaceMask", (list->at(3)->getRaceMask().toInteger() & 0xFFE), 0x010U);
    a.checkEqual("65. getRaceMask", (list->at(4)->getRaceMask().toInteger() & 0xFFE), 0xFFEU);
    a.checkEqual("66. getRaceMask", (list->at(5)->getRaceMask().toInteger() & 0xFFE), 0x402U);
    a.checkEqual("67. getRaceMask", (list->at(6)->getRaceMask().toInteger() & 0xFFE), 0x804U);
}

/** Test mission.ini parsing, parentheses/parameter special cases. */
AFL_TEST("game.spec.MissionList:loadFromIniFile:paren", a)
{
    // Generate a pseudo file
    static const char data[] =
        "10 one\n"
        "11 two)\n"
        "12 three (T\n";
    afl::io::ConstMemoryStream ms(afl::string::toBytes(data));
    afl::charset::CodepageCharset cp(afl::charset::g_codepageLatin1);

    // Read it
    Ref<MissionList> list = MissionList::create();
    list->loadFromIniFile(ms, cp);

    // Check
    a.checkEqual("01. size", list->size(), 3U);
    for (int i = 0; i < 3; ++i) {
        a.checkEqual("02. getNumber", list->at(i)->getNumber(), 10+i);
    }

    a.checkEqual("11. getName", list->at(0)->getName(), "one");
    a.checkEqual("12. getName", list->at(1)->getName(), "two)");
    a.checkEqual("13. getName", list->at(2)->getName(), "three (T");
}

/** Test loading from mission.cc. */
AFL_TEST("game.spec.MissionList:loadFromFile", a)
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
        "g = special\n"
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
        "Group=more\n"
        " = Bad3\n";
    afl::io::ConstMemoryStream ms(afl::string::toBytes(file));
    afl::sys::Log log;
    afl::string::NullTranslator tx;

    // Load
    Ref<MissionList> testee = MissionList::create();
    testee->loadFromFile(ms, log, tx);

    // Verify
    a.checkEqual("01. size", testee->size(), 3U);
    a.checkNonNull("02. at", testee->at(0));
    a.checkNonNull("03. at", testee->at(1));
    a.checkNonNull("04. at", testee->at(2));

    // Mission 1: Minimal, defaults
    a.checkEqual("11. getNumber",              testee->at(0)->getNumber(), 1);
    a.checkEqual("12. getName",                testee->at(0)->getName(), "Minimal");
    a.checkEqual("13. getRaceMask",            testee->at(0)->getRaceMask(), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS));
    a.checkEqual("14. getParameterName",       testee->at(0)->getParameterName(game::InterceptParameter), "Intercept");
    a.checkEqual("15. getParameterName",       testee->at(0)->getParameterName(game::TowParameter), "Tow");
    a.checkEqual("16. getConditionExpression", testee->at(0)->getConditionExpression(), "");
    a.checkEqual("17. getWarningExpression",   testee->at(0)->getWarningExpression(), "");
    a.checkEqual("18. getLabelExpression",     testee->at(0)->getLabelExpression(), "");
    a.checkEqual("19. getSetCommand",          testee->at(0)->getSetCommand(), "");
    a.checkEqual("1A. getGroup",               testee->at(0)->getGroup(), "");

    // Mission 2: Short, everything assigned using one-letter names
    a.checkEqual("21. getNumber",              testee->at(1)->getNumber(), 2);
    a.checkEqual("22. getName",                testee->at(1)->getName(), "Short");
    a.checkEqual("23. getRaceMask",            testee->at(1)->getRaceMask(), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS));
    a.checkEqual("24. getParameterName",       testee->at(1)->getParameterName(game::InterceptParameter), "Intercept2");
    a.checkEqual("25. getParameterName",       testee->at(1)->getParameterName(game::TowParameter), "Tow2");
    a.checkEqual("26. getConditionExpression", testee->at(1)->getConditionExpression(), "Cond2");
    a.checkEqual("27. getWarningExpression",   testee->at(1)->getWarningExpression(), "Work2");
    a.checkEqual("28. getLabelExpression",     testee->at(1)->getLabelExpression(), "Text2");
    a.checkEqual("29. getSetCommand",          testee->at(1)->getSetCommand(), "Set2");
    a.checkEqual("2A. getGroup",               testee->at(1)->getGroup(), "special");

    // Mission 3: Full, everything assigned using full names
    a.checkEqual("31. getNumber",              testee->at(2)->getNumber(), 3);
    a.checkEqual("32. getName",                testee->at(2)->getName(), "Full");
    a.checkEqual("33. getRaceMask",            testee->at(2)->getRaceMask(), game::PlayerSet_t(5));
    a.checkEqual("34. getParameterName",       testee->at(2)->getParameterName(game::InterceptParameter), "Intercept3");
    a.checkEqual("35. getParameterName",       testee->at(2)->getParameterName(game::TowParameter), "Tow3");
    a.checkEqual("36. getConditionExpression", testee->at(2)->getConditionExpression(), "Cond3");
    a.checkEqual("37. getWarningExpression",   testee->at(2)->getWarningExpression(), "Work3");
    a.checkEqual("38. getLabelExpression",     testee->at(2)->getLabelExpression(), "Text3");
    a.checkEqual("39. getSetCommand",          testee->at(2)->getSetCommand(), "Set3");
    a.checkEqual("3A. getGroup",               testee->at(2)->getGroup(), "more");
}

/** Test loading from mission.cc, error case. */
AFL_TEST("game.spec.MissionList:loadFromFile:error:no-delim", a)
{
    // File
    static const char* file = "no delim";
    afl::io::ConstMemoryStream ms(afl::string::toBytes(file));
    afl::sys::Log log;
    afl::string::NullTranslator tx;

    // Load
    Ref<MissionList> testee = MissionList::create();
    testee->loadFromFile(ms, log, tx);

    // Verify
    a.checkEqual("01. size", testee->size(), 0U);
}

/** Test loading from mission.cc, error case: bad number. */
AFL_TEST("game.spec.MissionList:loadFromFile:error:bad-num", a)
{
    // File
    static const char* file = "9999999,,Name";
    afl::io::ConstMemoryStream ms(afl::string::toBytes(file));
    afl::sys::Log log;
    afl::string::NullTranslator tx;

    // Load
    Ref<MissionList> testee = MissionList::create();
    testee->loadFromFile(ms, log, tx);

    // Verify
    a.checkEqual("01. size", testee->size(), 0U);
}

/** Test addMission(), merge missions, and, implicitly, sort(). */
AFL_TEST("game.spec.MissionList:addMission:merge", a)
{
    Ref<MissionList> testee = MissionList::create();

    // Add some "mission.cc" missions
    testee->addMission(Mission(1, ",Explore"));
    testee->addMission(Mission(9, "+1,Special 1"));
    testee->addMission(Mission(9, "+2,Special 2"));
    testee->addMission(Mission(9, "+3,Special 3"));

    // Add some "mission.ini" missions
    testee->addMission(Mission(1, ",Other Explore"));
    testee->addMission(Mission(4, ",Kill"));
    testee->addMission(Mission(9, ",Special"));

    // Sort
    testee->sort();

    a.checkEqual("01. size",        testee->size(), 5U);
    a.checkEqual("02. getNumber",   testee->at(0)->getNumber(), 1);
    a.checkEqual("03. getName",     testee->at(0)->getName(), "Explore");
    a.checkEqual("04. getNumber",   testee->at(1)->getNumber(), 4);
    a.checkEqual("05. getName",     testee->at(1)->getName(), "Kill");
    a.checkEqual("06. getNumber",   testee->at(2)->getNumber(), 9);
    a.checkEqual("07. getName",     testee->at(2)->getName(), "Special 1");
    a.checkEqual("08. getRaceMask", testee->at(2)->getRaceMask(), game::PlayerSet_t(1));
    a.checkEqual("09. getNumber",   testee->at(3)->getNumber(), 9);
    a.checkEqual("10. getName",     testee->at(3)->getName(), "Special 2");
    a.checkEqual("11. getRaceMask", testee->at(3)->getRaceMask(), game::PlayerSet_t(2));
    a.checkEqual("12. getNumber",   testee->at(4)->getNumber(), 9);
    a.checkEqual("13. getName",     testee->at(4)->getName(), "Special 3");
    a.checkEqual("14. getRaceMask", testee->at(4)->getRaceMask(), game::PlayerSet_t(3));

    // Test lookup
    const Mission* p;
    p = testee->findMissionByNumber(1, game::PlayerSet_t(1));
    a.checkNonNull("21. findMissionByNumber", p);
    a.checkEqual("22. getName", p->getName(), "Explore");

    p = testee->findMissionByNumber(9, game::PlayerSet_t(1));
    a.checkNonNull("31. findMissionByNumber", p);
    a.checkEqual("32. getName", p->getName(), "Special 1");

    p = testee->findMissionByNumber(9, game::PlayerSet_t(4));
    a.checkNull("41. findMissionByNumber", p);

    // Test position lookup
    size_t index = 9999;
    a.check("51. findIndexByNumber", testee->findIndexByNumber(1, game::PlayerSet_t(3)).get(index));
    a.checkEqual("52. index", index, 0U);

    a.check("61. findIndexByNumber", testee->findIndexByNumber(9, game::PlayerSet_t(2)).get(index));
    a.checkEqual("62. index", index, 3U);

    a.check("71. findIndexByNumber", !testee->findIndexByNumber(9, game::PlayerSet_t(4)).isValid());
}

/** Test addMission(), letter assignment. */
AFL_TEST("game.spec.MissionList:getHotkey", a)
{
    // Assign many missions
    Ref<MissionList> testee = MissionList::create();
    for (int i = 0; i < 30; ++i) {
        testee->addMission(Mission(20+i, ",egal"));
    }
    a.checkEqual("01. size", testee->size(), 30U);
    a.checkEqual("02", testee->at(0)->getHotkey(), 'a');
    a.checkEqual("03", testee->at(1)->getHotkey(), 'b');
    a.checkEqual("04", testee->at(25)->getHotkey(), 'z');
    a.checkEqual("05", testee->at(26)->getHotkey(), 'a');

    // Clear and add anew
    testee->clear();
    for (int i = 0; i < 5; ++i) {
        testee->addMission(Mission(20+i, ",egal"));
    }
    a.checkEqual("11. size", testee->size(), 5U);
    a.checkEqual("12", testee->at(0)->getHotkey(), 'a');
    a.checkEqual("13", testee->at(1)->getHotkey(), 'b');
    a.checkEqual("14", testee->at(4)->getHotkey(), 'e');
}

/** Test addMission(), letter assignment. */
AFL_TEST("game.spec.MissionList:getHotkey:2", a)
{
    // Preload, then assign many missions
    Ref<MissionList> testee = MissionList::create();
    testee->addMission(Mission(98, ",~kill"));
    testee->addMission(Mission(99, ",~jump"));
    for (int i = 0; i < 40; ++i) {
        testee->addMission(Mission(i, ",egal"));
    }
    a.checkEqual("01. size", testee->size(), 42U);

    // Manually assigned:
    a.checkEqual("11", testee->at(0)->getHotkey(), 'k');
    a.checkEqual("12", testee->at(1)->getHotkey(), 'j');

    // Auto-assigned:
    a.checkEqual("21", testee->at(2)->getHotkey(), '0');
    a.checkEqual("22", testee->at(3)->getHotkey(), '1');
    a.checkEqual("23", testee->at(11)->getHotkey(), '9');

    a.checkEqual("31", testee->at(12)->getHotkey(), 'a');
    a.checkEqual("32", testee->at(13)->getHotkey(), 'b');

    a.checkEqual("41", testee->at(20)->getHotkey(), 'i');
    a.checkEqual("42", testee->at(21)->getHotkey(), 'l');

    a.checkEqual("51", testee->at(35)->getHotkey(), 'z');
    a.checkEqual("52", testee->at(36)->getHotkey(), 'a');
}

/** Test getGroupedMissions(), base case. */
AFL_TEST("game.spec.MissionList:getGroupedMissions:base", a)
{
    Ref<MissionList> testee = MissionList::create();
    testee->addMission(Mission(1, ",one"));
    testee->addMission(Mission(2, ",two"));
    testee->addMission(Mission(3, ",three"));

    afl::string::NullTranslator tx;
    MissionList::Grouped g;
    testee->getGroupedMissions(g, tx);
    a.checkEqual("01", g.allName, "All");
    a.checkEqual("02", g.groups.size(), 1U);
    a.checkEqual("03", g.groups["All"].size(), 3U);

    checkEntry(a("All.0"), g.groups["All"], 0, 1, "1 - one");
    checkEntry(a("All.1"), g.groups["All"], 1, 2, "2 - two");
    checkEntry(a("All.2"), g.groups["All"], 2, 3, "3 - three");
}

/** Test getGroupedMissions(), complex case. */
AFL_TEST("game.spec.MissionList:getGroupedMissions:complex", a)
{
    // Missions
    Mission m1(10, ",one");
    m1.setGroup("g1");

    Mission m2(20, ",two");
    m2.setGroup("g2");

    Mission m3(30, ",three");
    m3.setGroup("g1,g2,All");

    Mission m4(40, ",four");
    m4.setGroup("All,g2");

    // MissionList
    Ref<MissionList> testee = MissionList::create();
    testee->addMission(m1);
    testee->addMission(m2);
    testee->addMission(m3);
    testee->addMission(m4);

    // Test
    afl::string::NullTranslator tx;
    MissionList::Grouped g;
    testee->getGroupedMissions(g, tx);
    a.checkEqual("01", g.allName, "All");
    a.checkEqual("02", g.groups.size(), 3U);
    a.checkEqual("03", g.groups["All"].size(), 4U);
    a.checkEqual("04", g.groups["g1"].size(), 2U);
    a.checkEqual("05", g.groups["g2"].size(), 3U);

    checkEntry(a("All.0"), g.groups["All"], 0, 10, "a - one");
    checkEntry(a("All.1"), g.groups["All"], 1, 20, "b - two");
    checkEntry(a("All.2"), g.groups["All"], 2, 30, "c - three");
    checkEntry(a("All.3"), g.groups["All"], 3, 40, "d - four");

    checkEntry(a("g1.0"), g.groups["g1"], 0, 10, "a - one");
    checkEntry(a("g1.1"), g.groups["g1"], 1, 30, "c - three");

    checkEntry(a("g2.0"), g.groups["g2"], 0, 20, "b - two");
    checkEntry(a("g2.1"), g.groups["g2"], 1, 30, "c - three");
    checkEntry(a("g2.2"), g.groups["g2"], 2, 40, "d - four");
}

/*
 *  isMissionCloaking
 */

AFL_TEST("game.spec.MissionList:isMissionCloaking", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config.setDefaultValues();
    HostVersion(HostVersion::PHost, MKVERSION(3,4,0)).setImpliedHostConfiguration(config);
    Ref<MissionList> testee = MissionList::create();

    // Normal
    a.check("1  / 3", !testee->isMissionCloaking( 1, 3, config));
    a.check("1  / 5", !testee->isMissionCloaking( 1, 5, config));

    // Special
    a.check("9  / 3",  testee->isMissionCloaking( 9, 3, config));
    a.check("9  / 5", !testee->isMissionCloaking( 9, 5, config));

    // Cloak
    a.check("10 / 3",  testee->isMissionCloaking(10, 3, config));
    a.check("10 / 5",  testee->isMissionCloaking(10, 5, config));

    // Extended spy
    a.check("29 / 3",  testee->isMissionCloaking(29, 3, config));
    a.check("29 / 5", !testee->isMissionCloaking(29, 5, config));

    // Extended cloak
    a.check("30 / 3",  testee->isMissionCloaking(30, 3, config));
    a.check("30 / 5",  testee->isMissionCloaking(30, 5, config));

    // Extended special
    a.check("31 / 3",  testee->isMissionCloaking(31, 3, config));
    a.check("31 / 5", !testee->isMissionCloaking(31, 5, config));
}

/*
 *  isExtendedMission
 */

AFL_TEST("game.spec.MissionList:isExtendedMission:phost:default", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config.setDefaultValues();
    HostVersion(HostVersion::PHost, MKVERSION(3,4,0)).setImpliedHostConfiguration(config);
    Ref<MissionList> testee = MissionList::create();

    a.check("01",  testee->isExtendedMission(20, Mission::pmsn_BuildTorpsFromCargo, config));
    a.check("02", !testee->isExtendedMission(10, Mission::pmsn_BuildTorpsFromCargo, config));
    a.check("03", !testee->isExtendedMission(50, Mission::pmsn_BuildTorpsFromCargo, config));
}

AFL_TEST("game.spec.MissionList:isExtendedMission:phost:off", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config.setDefaultValues();
    config[HostConfiguration::AllowExtendedMissions].set(0);
    HostVersion(HostVersion::PHost, MKVERSION(3,4,0)).setImpliedHostConfiguration(config);
    Ref<MissionList> testee = MissionList::create();

    a.check("01", !testee->isExtendedMission(20, Mission::pmsn_BuildTorpsFromCargo, config));
    a.check("02", !testee->isExtendedMission(10, Mission::pmsn_BuildTorpsFromCargo, config));
    a.check("03", !testee->isExtendedMission(50, Mission::pmsn_BuildTorpsFromCargo, config));
}

AFL_TEST("game.spec.MissionList:isExtendedMission:phost:moved", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config.setDefaultValues();
    config[HostConfiguration::ExtMissionsStartAt].set(50);
    HostVersion(HostVersion::PHost, MKVERSION(3,4,0)).setImpliedHostConfiguration(config);
    Ref<MissionList> testee = MissionList::create();

    a.check("01", !testee->isExtendedMission(20, Mission::pmsn_BuildTorpsFromCargo, config));
    a.check("02", !testee->isExtendedMission(10, Mission::pmsn_BuildTorpsFromCargo, config));
    a.check("03",  testee->isExtendedMission(50, Mission::pmsn_BuildTorpsFromCargo, config));
}

AFL_TEST("game.spec.MissionList:isExtendedMission:host", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config.setDefaultValues();
    config[HostConfiguration::AllowExtendedMissions].set(0);
    HostVersion(HostVersion::Host, MKVERSION(3,22,0)).setImpliedHostConfiguration(config);
    Ref<MissionList> testee = MissionList::create();

    a.check("01", !testee->isExtendedMission(20, Mission::pmsn_BuildTorpsFromCargo, config));
    a.check("02", !testee->isExtendedMission(10, Mission::pmsn_BuildTorpsFromCargo, config));
    a.check("03", !testee->isExtendedMission(50, Mission::pmsn_BuildTorpsFromCargo, config));
}

/*
 *  isSpecialMission
 */

AFL_TEST("game.spec.MissionList:isSpecialMission:phost:default", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config.setDefaultValues();
    HostVersion(HostVersion::PHost, MKVERSION(3,4,0)).setImpliedHostConfiguration(config);
    Ref<MissionList> testee = MissionList::create();

    a.check("01", !testee->isSpecialMission( 5, config));
    a.check("02",  testee->isSpecialMission( 9, config));
    a.check("03",  testee->isSpecialMission(31, config));
    a.check("04", !testee->isSpecialMission(61, config));
}

AFL_TEST("game.spec.MissionList:isSpecialMission:phost:off", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config.setDefaultValues();
    config[HostConfiguration::AllowExtendedMissions].set(0);
    HostVersion(HostVersion::PHost, MKVERSION(3,4,0)).setImpliedHostConfiguration(config);
    Ref<MissionList> testee = MissionList::create();

    a.check("01", !testee->isSpecialMission( 5, config));
    a.check("02",  testee->isSpecialMission( 9, config));
    a.check("03", !testee->isSpecialMission(31, config));
    a.check("04", !testee->isSpecialMission(61, config));
}

AFL_TEST("game.spec.MissionList:isSpecialMission:phost:moved", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config.setDefaultValues();
    config[HostConfiguration::ExtMissionsStartAt].set(50);
    HostVersion(HostVersion::PHost, MKVERSION(3,4,0)).setImpliedHostConfiguration(config);
    Ref<MissionList> testee = MissionList::create();

    a.check("01", !testee->isSpecialMission( 5, config));
    a.check("02",  testee->isSpecialMission( 9, config));
    a.check("03", !testee->isSpecialMission(31, config));
    a.check("04",  testee->isSpecialMission(61, config));
}

AFL_TEST("game.spec.MissionList:isSpecialMission:host", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config.setDefaultValues();
    config[HostConfiguration::AllowExtendedMissions].set(0);
    HostVersion(HostVersion::Host, MKVERSION(3,22,0)).setImpliedHostConfiguration(config);
    Ref<MissionList> testee = MissionList::create();

    a.check("01", !testee->isSpecialMission( 5, config));
    a.check("02",  testee->isSpecialMission( 9, config));
    a.check("03", !testee->isSpecialMission(31, config));
    a.check("04", !testee->isSpecialMission(61, config));
}
