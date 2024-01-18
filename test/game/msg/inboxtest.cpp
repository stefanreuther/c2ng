/**
  *  \file test/game/msg/inboxtest.cpp
  *  \brief Test for game::msg::Inbox
  */

#include "game/msg/inbox.hpp"

#include <set>
#include "afl/charset/utf8charset.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/parser/informationconsumer.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/playerlist.hpp"

using game::msg::Mailbox;

namespace {
    class Consumer : public game::parser::InformationConsumer {
     public:
        virtual void addMessageInformation(const game::parser::MessageInformation& info)
            { m_markers.insert(100*info.getObjectId() + int(info.getObjectType())); }

        bool hasInfo(game::parser::MessageInformation::Type type, int id) const
            { return m_markers.find(100*id + int(type)) != m_markers.end(); }

     private:
        std::set<int32_t> m_markers;
    };

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
AFL_TEST("game.msg.Inbox:basics", a)
{
    afl::string::NullTranslator tx;
    game::PlayerList list;

    game::msg::Inbox testee;
    a.checkEqual("01. addMessage", testee.addMessage("a", 10), 0U);
    a.checkEqual("02. addMessage", testee.addMessage("b", 20), 1U);
    a.checkEqual("03. addMessage", testee.addMessage("c", 15), 2U);
    a.checkEqual("04. getNumMessages", testee.getNumMessages(), 3U);
    a.checkEqual("05. getMessageText", testee.getMessageText(0, tx, list), "a");
    a.checkEqual("06. getMessageText", testee.getMessageText(1, tx, list), "b");
    a.checkEqual("07. getMessageText", testee.getMessageText(2, tx, list), "c");
    a.checkEqual("08. getMessageMetadata", testee.getMessageMetadata(0, tx, list).turnNumber, 10);
    a.checkEqual("09. getMessageMetadata", testee.getMessageMetadata(1, tx, list).turnNumber, 20);
    a.checkEqual("10. getMessageMetadata", testee.getMessageMetadata(2, tx, list).turnNumber, 15);

    a.checkEqual("11. getMessageForwardText", testee.getMessageForwardText(0, tx, list),
                     "--- Forwarded Message ---\n"
                     "a\n"
                     "--- End Forwarded Message ---");
    a.checkEqual("12. getMessageReplyText", testee.getMessageReplyText(0, tx, list),
                     "> a\n");

    // Out-of-range
    a.checkEqual("21. getMessageText", testee.getMessageText(3, tx, list), "");
    a.checkEqual("22. getMessageMetadata", testee.getMessageMetadata(3, tx, list).turnNumber, 0);
}

/** Test getMessageHeading().
    Verify that getMessageHeading() returns the expected values for a number of real and constructed messages. */
AFL_TEST("game.msg.Inbox:getMessageHeading", a)
{
    // Too short
    a.checkEqual("01", getMessageHeading("hi."), "(_) Unknown");
    a.checkEqual("02", getMessageHeading(""), "(_) Unknown");

    // pre-3.2
    a.checkEqual("11", getMessageHeading(
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
    a.checkEqual("21", getMessageHeading(
                     "(-r0000)<<< Sub Space Message >>>\n"
                     "FROM: ? <0>\n"
                     "TO  : The Crystal Confederation\n"
                     "\n"
                     "i think fed and cyborg will win the\n"
                     "game.\n"),
                 "(r) Anonymous Message");

    // Player-to-player
    a.checkEqual("31", getMessageHeading(
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
    a.checkEqual("41", getMessageHeading(
                     "(-r9000)<< Sub Space Message >>\n"
                     "FROM: The Robotic Imperium\n"
                     "TO: Freihaendler von Bosycs Stern\n"
                     "\n"
                     "> schreib' lieber drei oder mehr!\n"
                     "\n"
                     "drei oder mehr!\n"),
                 "(r) Sub Space Message");

    // Starbase message (c2nu)
    a.checkEqual("51", getMessageHeading(
                     "(-d0292)<<< Space Dock Message >>>\n"
                     "\n"
                     "From: Galibor's World ID#292\n"
                     "\n"
                     "A new starbase has been constructed at Galibor's World ID#292\n"
                     "\n"
                     "Location: (2644, 1964)\n"),
                 "(d) Starbase Message");

    // Starbase message (v3)
    a.checkEqual("61", getMessageHeading(
                     "(-d0279)<<< Space Dock Message >>>\n"
                     "\n"
                     "A new VICTORIOUS CLASS BATTLESHIP\n"
                     "Has been constructed\n"
                     "at Pedmont\n"
                     "space dock.\n"),
                 "(d) Starbase Message");

    // Minefield laid (PHost, German)
    a.checkEqual("71", getMessageHeading(
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
    a.checkEqual("81", getMessageHeading(
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
    a.checkEqual("91", getMessageHeading(
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
    a.checkEqual("101", getMessageHeading(
                     "(-g0000)FF / ff Allies:\n"
                     "Race:  2 YES / NO\n"
                     "Race:  4 YES / yes\n"),
                 "(g) HConfig");

    // HConfig
    a.checkEqual("111", getMessageHeading(
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
    a.checkEqual("121", getMessageHeading(
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
    a.checkEqual("131", getMessageHeading(
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
    a.checkEqual("141", getMessageHeading(
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
    a.checkEqual("151", getMessageHeading(
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
    a.checkEqual("161", getMessageHeading(
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
    a.checkEqual("171", getMessageHeading(
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
    a.checkEqual("181", getMessageHeading(
                     "(-a0125)<<< STARBASE+ >>>\n"
                     "\n"
                     "You have a total of  0  special\n"
                     "transports in your fleet.\n"),
                 "(a) Starbase+");

    // Distress
    a.checkEqual("191", getMessageHeading(
                     "(-e0466)<<< DISTRESS CALL! >>>\n"
                     "\n"
                     "KCCB KING CONDOR\n"
                     "ID # 466\n"
                     "Has struck a mine!\n"
                     "AT: ( 2456 , 2861 )\n"
                     "Damage is at  22%\n"),
                 "(e) Distress Call!");

    // Numbered
    a.checkEqual("201", getMessageHeading(
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
AFL_TEST("game.msg.Inbox:sort", a)
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

    a.checkEqual("01. getMessageText", testee.getMessageText(0, tx, list), TEXT[0]);
    a.checkEqual("02. getMessageText", testee.getMessageText(1, tx, list), TEXT[3]);
    a.checkEqual("03. getMessageText", testee.getMessageText(2, tx, list), TEXT[1]);
    a.checkEqual("04. getMessageText", testee.getMessageText(3, tx, list), TEXT[2]);
    a.checkEqual("05. getMessageText", testee.getMessageText(4, tx, list), TEXT[4]);
}

/** Test data reception. */
AFL_TEST("game.msg.Inbox:receiveMessageData", a)
{
    // Create
    afl::string::NullTranslator tx;
    game::PlayerList list;
    afl::charset::Utf8Charset cs;
    game::TeamSettings teamSettings;
    game::msg::Inbox testee;
    testee.addMessage("(-r3000)<<< Sub Space Message >>>\n"
                      "<<< VPA Data Transmission >>>\n"
                      "\n"
                      "OBJECT: Mine field 61\n"
                      "DATA: 2094989326\n"
                      "ocaalekakbhadaaaijmcaaaaaaaa\n", 3);
    a.checkEqual("01. getMessageMetadata", testee.getMessageMetadata(0, tx, list).dataStatus, game::msg::Mailbox::NoData);

    // Initial scan
    Consumer c1;
    testee.receiveMessageData(0, c1, teamSettings, false, cs);
    a.check("11. hasInfo", !c1.hasInfo(game::parser::MessageInformation::Minefield, 61));
    a.checkEqual("12. getMessageMetadata", testee.getMessageMetadata(0, tx, list).dataStatus, game::msg::Mailbox::DataReceivable);

    // Force reception
    Consumer c2;
    testee.receiveMessageData(0, c2, teamSettings, true, cs);
    a.check("21. hasInfo", c2.hasInfo(game::parser::MessageInformation::Minefield, 61));
    a.checkEqual("22. getMessageMetadata", testee.getMessageMetadata(0, tx, list).dataStatus, game::msg::Mailbox::DataReceived);
}

/** Test automatic reception. */
AFL_TEST("game.msg.Inbox:receiveMessageData:auto", a)
{
    // Create
    afl::string::NullTranslator tx;
    game::PlayerList list;
    game::TeamSettings teamSettings;
    teamSettings.setViewpointPlayer(10);
    teamSettings.setReceiveConfiguration(3, game::TeamSettings::MessageTypes_t(game::TeamSettings::MinefieldInformation));
    teamSettings.setReceiveConfiguration(4, game::TeamSettings::MessageTypes_t(game::TeamSettings::DrawingInformation));

    afl::charset::Utf8Charset cs;
    game::msg::Inbox testee;

    // Minefield from 3 (auto-receive)
    testee.addMessage("(-r3000)<<< Sub Space Message >>>\n"
                      "<<< VPA Data Transmission >>>\n"
                      "\n"
                      "OBJECT: Mine field 61\n"
                      "DATA: 2094989326\n"
                      "ocaalekakbhadaaaijmcaaaaaaaa\n", 3);

    // Drawing from 3 (not auto-receive)
    testee.addMessage("(-r3000)<<< Sub Space Message >>>\n"
                      "<<< VPA Data Transmission >>>\n"
                      "\n"
                      "OBJECT: Marker\n"
                      "DATA: -1680801779\n"
                      "cafaokjapjiaaaaaaaaaljdkaa\n", 3);

    // Same drawing from 4 (auto-receive)
    testee.addMessage("(-r4000)<<< Sub Space Message >>>\n"
                      "<<< VPA Data Transmission >>>\n"
                      "\n"
                      "OBJECT: Marker\n"
                      "DATA: -1680801779\n"
                      "cafaokjapjiaaaaaaaaaljdkaa\n", 3);
    a.checkEqual("01. getMessageMetadata", testee.getMessageMetadata(0, tx, list).dataStatus, game::msg::Mailbox::NoData);
    a.checkEqual("02. getMessageMetadata", testee.getMessageMetadata(1, tx, list).dataStatus, game::msg::Mailbox::NoData);
    a.checkEqual("03. getMessageMetadata", testee.getMessageMetadata(2, tx, list).dataStatus, game::msg::Mailbox::NoData);

    // Scan first
    Consumer c1;
    testee.receiveMessageData(0, c1, teamSettings, false, cs);
    a.check("11. hasInfo", c1.hasInfo(game::parser::MessageInformation::Minefield, 61));
    a.checkEqual("12. getMessageMetadata", testee.getMessageMetadata(0, tx, list).dataStatus, game::msg::Mailbox::DataReceived);

    // Scan second
    Consumer c2;
    testee.receiveMessageData(1, c2, teamSettings, false, cs);
    a.check("21. hasInfo", !c2.hasInfo(game::parser::MessageInformation::MarkerDrawing, 0));
    a.checkEqual("22. getMessageMetadata", testee.getMessageMetadata(1, tx, list).dataStatus, game::msg::Mailbox::DataReceivable);

    // Scan third
    Consumer c3;
    testee.receiveMessageData(2, c3, teamSettings, false, cs);
    a.check("31. hasInfo", c3.hasInfo(game::parser::MessageInformation::MarkerDrawing, 0));
    a.checkEqual("32. getMessageMetadata", testee.getMessageMetadata(2, tx, list).dataStatus, game::msg::Mailbox::DataReceived);
}

/** Test reception errors. */
AFL_TEST("game.msg.Inbox:receiveMessageData:error", a)
{
    // Create
    afl::string::NullTranslator tx;
    game::PlayerList list;
    afl::charset::Utf8Charset cs;
    game::TeamSettings teamSettings;
    game::msg::Inbox testee;

    // - message 0: failure (minefield body, planet header)
    testee.addMessage("(-r3000)<<< Sub Space Message >>>\n"
                      "<<< VPA Data Transmission >>>\n"
                      "\n"
                      "OBJECT: Planet 50\n"
                      "DATA: 2094989326\n"
                      "ocaalekakbhadaaaijmcaaaaaaaa\n", 3);
    // - message 1: not a data transfer
    testee.addMessage("(-r3000)<<< Sub Space Message >>>\n"
                      "Just some text\n", 3);
    // - message 2: checksum error
    testee.addMessage("(-r3000)<<< Sub Space Message >>>\n"
                      "<<< VPA Data Transmission >>>\n"
                      "\n"
                      "OBJECT: Mine field 61\n"
                      "DATA: 99999\n"
                      "ocaalekakbhadaaaijmcaaaaaaaa\n", 3);
    a.checkEqual("01. getMessageMetadata", testee.getMessageMetadata(0, tx, list).dataStatus, game::msg::Mailbox::NoData);

    // Initial scan
    Consumer c;
    testee.receiveMessageData(0, c, teamSettings, false, cs);
    a.checkEqual("11. getMessageMetadata", testee.getMessageMetadata(0, tx, list).dataStatus, game::msg::Mailbox::DataFailed);

    testee.receiveMessageData(1, c, teamSettings, false, cs);
    a.checkEqual("21. getMessageMetadata", testee.getMessageMetadata(1, tx, list).dataStatus, game::msg::Mailbox::NoData);

    testee.receiveMessageData(2, c, teamSettings, false, cs);
    a.checkEqual("31. getMessageMetadata", testee.getMessageMetadata(2, tx, list).dataStatus, game::msg::Mailbox::DataWrongChecksum);
}

/** Test primary link handling. */
AFL_TEST("game.msg.Inbox:setMessagePrimaryLink", a)
{
    // Create
    afl::string::NullTranslator tx;
    game::PlayerList list;
    game::msg::Inbox testee;

    testee.addMessage("(-i0006)<<< ION Advisory >>>\n"
                      "ION Disturbance\n"
                      "ID Number:  6\n"
                      "Centered At: (  1959, 1110)\n"
                      "North of Fred\n"
                      "Planet ID Number  268\n"
                      " 26 LY from planet\n", 3);

    // Check default settings
    a.checkEqual("01. getMessageMetadata", testee.getMessageMetadata(0, tx, list).primaryLink, game::Reference(game::Reference::IonStorm, 6));
    a.checkEqual("02. getMessageMetadata", testee.getMessageMetadata(0, tx, list).secondaryLink, game::Reference(game::map::Point(1959, 1110)));

    // Override association
    testee.setMessagePrimaryLink(0, game::Reference(game::Reference::Planet, 268));
    a.checkEqual("11. getMessageMetadata", testee.getMessageMetadata(0, tx, list).primaryLink, game::Reference(game::Reference::Planet, 268));
    a.checkEqual("12. getMessageMetadata", testee.getMessageMetadata(0, tx, list).secondaryLink, game::Reference(game::map::Point(1959, 1110)));
}
