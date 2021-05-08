/**
  *  \file u/t_game_msg_inbox.cpp
  *  \brief Test for game::msg::Inbox
  */

#include "game/msg/inbox.hpp"

#include "t_game_msg.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/playerlist.hpp"

namespace {
    String_t getMessageHeading(String_t text)
    {
        afl::string::NullTranslator tx;
        game::PlayerList list;
        list.create(1)->setName(game::Player::LongName, "The Federation");
        list.create(2)->setName(game::Player::LongName, "The Birds");

        game::msg::Inbox inbox;
        inbox.addMessage(text, 1);
        return inbox.getMessageHeading(0, tx, list);
    }
}

/** Test basic operations.
    Verify correct values returned on interface methods. */
void
TestGameMsgInbox::testBasics()
{
    afl::string::NullTranslator tx;
    game::PlayerList list;
    
    game::msg::Inbox testee;
    testee.addMessage("a", 10);
    testee.addMessage("b", 20);
    testee.addMessage("c", 15);
    TS_ASSERT_EQUALS(testee.getNumMessages(), 3U);
    TS_ASSERT_EQUALS(testee.getMessageText(0, tx, list), "a");
    TS_ASSERT_EQUALS(testee.getMessageText(1, tx, list), "b");
    TS_ASSERT_EQUALS(testee.getMessageText(2, tx, list), "c");
    TS_ASSERT_EQUALS(testee.getMessageTurnNumber(0), 10);
    TS_ASSERT_EQUALS(testee.getMessageTurnNumber(1), 20);
    TS_ASSERT_EQUALS(testee.getMessageTurnNumber(2), 15);

    // Out-of-range
    TS_ASSERT_EQUALS(testee.getMessageText(3, tx, list), "");
    TS_ASSERT_EQUALS(testee.getMessageTurnNumber(3), 0);
}

/** Test getMessageHeading().
    Verify that getMessageHeading() returns the expected values for a number of real and constructed messages. */
void
TestGameMsgInbox::testHeaders()
{
    // Too short
    TS_ASSERT_EQUALS(getMessageHeading("hi."), "(_) Unknown");
    TS_ASSERT_EQUALS(getMessageHeading(""), "(_) Unknown");

    // pre-3.2
    TS_ASSERT_EQUALS(getMessageHeading(
                         "<<< Sub Space Message >>>\n"
                         "\n"
                         "From: 0                   \n"
                         "We have converted our\n"
                         "torpedoes into deep space mines\n"
                         "and laid them in a field centered\n"
                         "at ( 1000 ,  1000 )\n"
                         " 2940 mines were laid\n"
                         "Mine field ID#  1 now contains\n"
                         " 2940 mine units and is\n"
                         " 54 light years in radius\n"),
                     "(_) Sub Space Message");

    // Anonymous (PHost)
    TS_ASSERT_EQUALS(getMessageHeading(
                         "(-r0000)<<< Sub Space Message >>>\n"
                         "FROM: ? <0>\n"
                         "TO  : The Crystal Confederation\n"
                         "\n"
                         "i think fed and cyborg will win the\n"
                         "game.\n"),
                     "(r) Anonymous Message");

    // Player-to-player
    TS_ASSERT_EQUALS(getMessageHeading(
                         "(-r1000)<< Sub Space Message >>\n"
                         "FROM: Southern United Planets\n"
                         "TO: Northern United Planets\n"
                         "\n"
                         "Allied Planets\n"
                         "538 539 554 561 579 95 96 611 621 624\n"
                         "132 150 668 171 679 204 226 230 734 736\n"
                         "239 770 790 307 818 321 837 870 881 382\n"
                         "887 401 908 411 412 914 415 967 471\n"),
                     "(r) The Federation");

    // Player-to-unknown-player
    TS_ASSERT_EQUALS(getMessageHeading(
                         "(-r9000)<< Sub Space Message >>\n"
                         "FROM: The Robotic Imperium\n"
                         "TO: Freihaendler von Bosycs Stern\n"
                         "\n"
                         "> schreib' lieber drei oder mehr!\n"
                         "\n"
                         "drei oder mehr!\n"),
                     "(r) Sub Space Message");

    // Starbase message (c2nu)
    TS_ASSERT_EQUALS(getMessageHeading(
                         "(-d0292)<<< Space Dock Message >>>\n"
                         "\n"
                         "From: Galibor's World ID#292\n"
                         "\n"
                         "A new starbase has been constructed at Galibor's World ID#292\n"
                         "\n"
                         "Location: (2644, 1964)\n"),
                     "(d) Starbase Message");

    // Starbase message (v3)
    TS_ASSERT_EQUALS(getMessageHeading(
                         "(-d0279)<<< Space Dock Message >>>\n"
                         "\n"
                         "A new VICTORIOUS CLASS BATTLESHIP\n"
                         "Has been constructed\n"
                         "at Pedmont\n"
                         "space dock.\n"),
                     "(d) Starbase Message");

    // Minefield laid (PHost, German)
    TS_ASSERT_EQUALS(getMessageHeading(
                         "(-l0198)<<< Minenbericht >>>\n"
                         "\n"
                         "Von: Schiff Zorg\n"
                         "ID:  #674\n"
                         "\n"
                         "Wir haben 2000 neue Minen gelegt.\n"
                         "\n"
                         "Unser Minenfeld (ID #198) um\n"
                         "(2461, 2573) besteht nun aus\n"
                         "2000 Minen und hat einen Radius\n"
                         "von 44 Lichtjahren.\n"),
                     "(l) Minefield Laid");

    // Minefield laid (normal)
    TS_ASSERT_EQUALS(getMessageHeading(
                         "(-l0043)<<< Sub Space Message >>>\n"
                         "\n"
                         "From: Laestrygones\n"
                         "We have converted our\n"
                         "torpedoes into web mines\n"
                         "and laid them in a field centered\n"
                         "at ( 1856 ,  1995 )\n"
                         " 648 mines were laid\n"
                         "Mine field ID#  43 now contains\n"
                         " 648 mine units and is\n"
                         " 25 light years in radius\n"),
                     "(l) Minefield Laid");

    // Ion storm
    TS_ASSERT_EQUALS(getMessageHeading(
                         "(-i0039)<<< ION Advisory >>>\n"
                         "Ion Disturbance\n"
                         "ID Number:  39\n"
                         "Centered At: (  2297, 1650)\n"
                         "South of Organia 3\n"
                         "Planet ID Number  136\n"
                         " 51 LY from planet\n"
                         "Voltage : 119\n"
                         "Heading : 102\n"
                         "Speed   :  Warp 6\n"
                         "Radius  : 30\n"
                         "Class :  Level 3\n"
                         "  Strong\n"
                         "System is growing\n"),
                     "(i) Ion Storm");

    // FF allies (should this be 'HConfig'?)
    TS_ASSERT_EQUALS(getMessageHeading(
                         "(-g0000)FF / ff Allies:\n"
                         "Race:  2 YES / NO\n"
                         "Race:  4 YES / yes\n"),
                     "(g) HConfig");

    // HConfig
    TS_ASSERT_EQUALS(getMessageHeading(
                         "(-g0000)< Message from your Host >\n"
                         "\n"
                         "One engine ships tow  NO\n"
                         "Hyper drive ships     YES\n"
                         "Climate Death Rate    10 %\n"
                         "Gravity wells         YES\n"
                         "Crystal desert advant YES\n"
                         "Mines destroy webs    NO\n"
                         "Climate limits pop    YES\n"),
                     "(g) HConfig");

    // Mine sweep (PHost, English)
    TS_ASSERT_EQUALS(getMessageHeading(
                         "(-m0231)<<< Sub Space Message >>>\n"
                         "\n"
                         "From: DIAMOND FLAME CLASS\n"
                         "We are scanning for mines\n"
                         "Enemy Mine field detected!\n"
                         "AT ( 2467 , 1880 )\n"
                         "They are Gorn style mines.\n"
                         "We are INSIDE the mine field!\n"
                         "There are  22832 mine units.\n"
                         "Mine field ID Number :  231\n"
                         "The field is  302 light years across.\n"
                         "Ship is firing beam weapons at\n"
                         "random, wide setting to clear mines.\n"
                         " 4800 mines have been destroyed!\n"
                         " 18032 mines remain.\n"),
                     "(m) Mine Sweep");

    // Mine sweep (PHost, NewEnglish)
    TS_ASSERT_EQUALS(getMessageHeading(
                         "(-m0092)<<< Sub Space Message >>>\n"
                         "\n"
                         "From: BCB-182\n"
                         "  (ship #182)\n"
                         "\n"
                         "Enemy Mine field detected\n"
                         "at (2354, 2923)!\n"
                         "They are Romulan style mines.\n"
                         "\n"
                         "There are 273 mine units.\n"
                         "Mine field ID Number: #92\n"
                         "The field is 32 light years across.\n"
                         "\n"
                         "We are 0 light years from the\n"
                         "outside edge of the field.\n"
                         "Ship is using beam weapons to\n"
                         "destroy 273 mines.\n"
                         "\n"
                         "0 mines remain.\n"),
                     "(m) Mine Sweep");

    // Mine scan (PHost, NewEnglish)
    TS_ASSERT_EQUALS(getMessageHeading(
                         "(-m0055)<<< Sub Space Message >>>\n"
                         "\n"
                         "We are scanning our mines\n"
                         "at (2384, 1093).\n"
                         "Mine field #55 contains\n"
                         "2342 mines.\n"
                         "\n"
                         "FCode Planet: #156\n"),
                     "(m) Mine Scan");

    // Mine scan (c2nu)
    TS_ASSERT_EQUALS(getMessageHeading(
                         "(-m0321)<<< Mine Scan >>>\n"
                         "\n"
                         "From: Schwerin ID#492\n"
                         "\n"
                         "We are scanning our mines at (2425, 2045)\n"
                         "Mine field contains 1541 mines.\n"
                         "Mine field ID Number: 321\n"
                         "Local Friendly Code Planet: Smith's World ID#406\n"),
                     "(m) Mine Scan");  // taken from heading!

    // Mine scan (THost, old)
    TS_ASSERT_EQUALS(getMessageHeading(
                         "(om0001)<<< Sub Space Message >>>\n"
                         "\n"
                         "From: tester              \n"
                         "We are scanning our mines\n"
                         "at ( 500 ,  500 )\n"
                         "Mine field contains  4284 mines.\n"
                         "Mine field ID Number :  1\n"
                         "We are in the mine field.\n"
                         "Local Fcode Planet: ID#  57\n"),
                     "(m) Mine Scan");

    // Mine scan, not English
    TS_ASSERT_EQUALS(getMessageHeading(
                         "(-m0002)<<< Subruimte Bericht >>>\n"
                         "\n"
                         "VAN: <Alle Schepen>\n"
                         "We detecteren one mijnen\n"
                         "op ( 1456 ,  2027 )\n"
                         "Het mijnenveld bevat 2026 mijnen.\n"
                         "Nummer van dit mijnenveld is 2\n"
                         "We zijn  999 LY van de buitenrand\n"
                         "Fcode Planeet: 281\n"),
                     "(m) Subruimte Bericht");

    // Starbase PLus
    TS_ASSERT_EQUALS(getMessageHeading(
                         "(-a0125)<<< STARBASE+ >>>\n"
                         "\n"
                         "You have a total of  0  special\n"
                         "transports in your fleet.\n"),
                     "(a) Starbase+");

    // Distress
    TS_ASSERT_EQUALS(getMessageHeading(
                         "(-e0466)<<< DISTRESS CALL! >>>\n"
                         "\n"
                         "KCCB KING CONDOR\n"
                         "ID # 466\n"
                         "Has struck a mine!\n"
                         "AT: ( 2456 , 2861 )\n"
                         "Damage is at  22%\n"),
                     "(e) Distress Call!");

    // Numbered
    TS_ASSERT_EQUALS(getMessageHeading(
                         "(-h0000)<<< Game Settings (2) >>>\n"
                         "\n"
                         "Build Queue Planet: 0\n"
                         "Turn 90\n"
                         "Victory Countdown: 0\n"
                         "\n"
                         "Host started: 4/12/2012 9:00:12 PM\n"
                         "Host completed: 4/12/2012 9:04:45 PM\n"),
                     "(h) Game Settings");
}

/** Test sort().
    Perform an exemplary sort, verify result. */
void
TestGameMsgInbox::testSort()
{
    afl::string::NullTranslator tx;
    game::PlayerList list;

    const size_t N = 5;
    static const char*const TEXT[N] = {
        "(-a001) <<< H >>>\n",
        "(-b002) <<< H >>>\n",
        "(-c003) <<< H >>>\n",
        "(-a004) <<< H >>>\n",
        "(-c005) <<< H >>>\n",
    };

    game::msg::Inbox testee;
    for (size_t i = 0; i < N; ++i) {
        testee.addMessage(TEXT[i], 10);
    }
    testee.sort(tx, list);

    TS_ASSERT_EQUALS(testee.getMessageText(0, tx, list), TEXT[0]);
    TS_ASSERT_EQUALS(testee.getMessageText(1, tx, list), TEXT[3]);
    TS_ASSERT_EQUALS(testee.getMessageText(2, tx, list), TEXT[1]);
    TS_ASSERT_EQUALS(testee.getMessageText(3, tx, list), TEXT[2]);
    TS_ASSERT_EQUALS(testee.getMessageText(4, tx, list), TEXT[4]);
}

