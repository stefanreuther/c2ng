/**
  *  \file test/game/playerlisttest.cpp
  *  \brief Test for game::PlayerList
  */

#include "game/playerlist.hpp"

#include "afl/charset/utf8reader.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/string/translator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/player.hpp"
#include "game/test/counter.hpp"

/** Test setup and expandNames(). */
AFL_TEST("game.PlayerList:expandNames", a)
{
    // ex GameRaceNameTestSuite::testRaceNameList
    using game::Player;

    afl::string::NullTranslator tx;
    game::PlayerList testee;

    // Construct a race name list
    game::Player* pl = testee.create(1);
    a.check("01. create", pl);
    pl->setName(Player::ShortName,     "The Feds");
    pl->setName(Player::LongName,      "The Solar Federation");
    pl->setName(Player::AdjectiveName, "Fed");

    pl->setName(Player::OriginalShortName,     "The Old Feds");
    pl->setName(Player::OriginalLongName,      "The Old Solar Federation");
    pl->setName(Player::OriginalAdjectiveName, "Old Fed");

    pl = testee.create(2);
    a.check("11. create", pl);
    pl->setName(Player::ShortName,     "The Lizards");
    pl->setName(Player::LongName,      "The Lizard Alliance");
    pl->setName(Player::AdjectiveName, "Lizard");

    pl = testee.create(5);
    a.check("21. create", pl);
    pl->setName(Player::ShortName,     "The Privateers");
    pl->setName(Player::LongName,      "The Privateer Bands");
    pl->setName(Player::AdjectiveName, "Privateer");

    pl = testee.create(6);
    a.check("31. create", pl);
    // No names for Cyborg

    pl = testee.create(10);
    a.check("41. create", pl);
    pl->setName(Player::ShortName,     "The Rebels");
    pl->setName(Player::LongName,      "The Rebel Confederation");
    pl->setName(Player::AdjectiveName, "Rebel");

    pl = testee.create(11);
    a.check("51. create", pl);
    pl->setName(Player::ShortName,     "The Colonies");
    pl->setName(Player::LongName,      "The Missing Colonies of Man");
    pl->setName(Player::AdjectiveName, "Colonial");

    pl = testee.create(12);
    a.check("61. create", pl);
    pl->initAlien();

    // Check it
    pl = testee.get(1);
    a.check("71. get", pl);
    a.checkEqual("72", pl->getName(Player::ShortName, tx), "The Feds");
    a.checkEqual("73", pl->getName(Player::OriginalShortName, tx), "The Old Feds");
    a.check("74. isReal", pl->isReal());

    pl = testee.get(11);
    a.check("81. get", pl);
    a.checkEqual("82", pl->getName(Player::ShortName, tx), "The Colonies");
    a.check("83. isReal", pl->isReal());

    pl = testee.get(0);
    a.check("91. get", pl);
    a.checkEqual("92", pl->getName(Player::ShortName, tx), "Nobody");
    a.check("93. isReal", !pl->isReal());

    pl = testee.get(12);
    a.check("101. get", pl);
    a.checkEqual("102", pl->getName(Player::ShortName, tx), "Alien Marauders");
    a.check("103. isReal", !pl->isReal());

    // We never set these
    a.checkNull("111", testee.get(3));
    a.checkNull("112", testee.get(13));
    a.checkNull("113", testee.get(23));

    a.checkNull("121", testee.get(-1));
    a.checkNull("122", testee.get(10000));

    // Not creatible
    a.checkNull("131", testee.create(-1));

    // Expansions
    a.checkEqual("141. expandNames", testee.expandNames("a %-5 ship", false, tx), "a Privateer ship");
    a.checkEqual("142. expandNames", testee.expandNames("attack %5!", false, tx), "attack The Privateers!");
    a.checkEqual("143. expandNames", testee.expandNames("%1...", false, tx), "The Feds...");
    a.checkEqual("144. expandNames", testee.expandNames("%1.", false, tx), "The Feds.");
    a.checkEqual("145. expandNames", testee.expandNames("%1.", true, tx), "The Old Feds.");
    a.checkEqual("146. expandNames", testee.expandNames("%-1 ship", true, tx), "Old Fed ship");
    a.checkEqual("147. expandNames", testee.expandNames("%1", false, tx), "The Feds");
    a.checkEqual("148. expandNames", testee.expandNames("%a...", false, tx), "The Rebels...");
    a.checkEqual("149. expandNames", testee.expandNames("...%b", false, tx), "...The Colonies");
    a.checkEqual("150. expandNames", testee.expandNames("%A...", false, tx), "The Rebels...");
    a.checkEqual("151. expandNames", testee.expandNames("...%B", false, tx), "...The Colonies");
    a.checkEqual("152. expandNames", testee.expandNames("%-A...", false, tx), "Rebel...");
    a.checkEqual("153. expandNames", testee.expandNames("...%-B", false, tx), "...Colonial");
    a.checkEqual("154. expandNames", testee.expandNames("%1%2", false, tx), "The FedsThe Lizards");
    a.checkEqual("155. expandNames", testee.expandNames("a%%b", false, tx), "a%b");
    a.checkEqual("156. expandNames", testee.expandNames("%%", false, tx), "%");
    a.checkEqual("157. expandNames", testee.expandNames("%%1", false, tx), "%1");

    // %6 expands to default name because object exists.
    // Object for %7 does not exist so it expands to 7 (same as %d below).
    a.checkEqual("161. expandNames", testee.expandNames("%6.", false, tx), "Player 6.");
    a.checkEqual("162. expandNames", testee.expandNames("%7.", false, tx), "7.");

    // Those are out-of-spec. As of 20110102, '%' quotes, i.e. keeps the
    // offending character (this is to avoid eating partial UTF-8 runes).
    // \change Unlike PCC2, c2ng expands %0 and %c because we have corresponding slots in our table.
    a.checkEqual("171. expandNames", testee.expandNames("%0", false, tx), "Nobody");
    a.checkEqual("172. expandNames", testee.expandNames("%01", false, tx), "Nobody1");
    a.checkEqual("173. expandNames", testee.expandNames("%c", false, tx), "Alien Marauders");
    a.checkEqual("174. expandNames", testee.expandNames("%d", false, tx), "d");
    a.checkEqual("175. expandNames", testee.expandNames("%x", false, tx), "x");
    a.checkEqual("176. expandNames", testee.expandNames("%.", false, tx), ".");
    a.checkEqual("177. expandNames", testee.expandNames("%-", false, tx), "");
    a.checkEqual("178. expandNames", testee.expandNames("%-.", false, tx), ".");
    a.checkEqual("179. expandNames", testee.expandNames("%", false, tx), "");

    // Make sure this doesn't produce error characters by breaking UTF-8
    String_t sa = testee.expandNames("a%\xc3\x80", false, tx);
    afl::charset::Utf8Reader rdr(afl::string::toBytes(sa), 0);
    while (rdr.hasMore()) {
        afl::charset::Unichar_t ch = rdr.eat();
        a.check("181. valid character", !afl::charset::isErrorCharacter(ch));
    }
}

/** Test iteration. */
AFL_TEST("game.PlayerList:iteration", a)
{
    game::PlayerList testee;
    testee.create(10);
    testee.create(1);
    testee.create(12);

    // First: 1
    game::Player* pl = testee.getFirstPlayer();
    a.checkNonNull("01. getFirstPlayer", pl);
    a.checkEqual("02. getId", pl->getId(), 1);
    a.checkEqual("03. get", pl, testee.get(1));

    // Second: 10
    pl = testee.getNextPlayer(pl);
    a.checkNonNull("11. getNextPlayer", pl);
    a.checkEqual("12. getId", pl->getId(), 10);
    a.checkEqual("13. get", pl, testee.get(10));

    // Last: 12
    pl = testee.getNextPlayer(pl);
    a.checkNonNull("21. getNextPlayer", pl);
    a.checkEqual("22. getId", pl->getId(), 12);
    a.checkEqual("23. get", pl, testee.get(12));

    // Final
    pl = testee.getNextPlayer(pl);
    a.checkNull("31. getNextPlayer", pl);
    a.checkNull("32. getNextPlayer", testee.getNextPlayer(pl));

    // Iteration from number
    a.checkEqual("41. getNextPlayer", testee.getNextPlayer(int(0)), testee.get(1));
    a.checkEqual("42. getNextPlayer", testee.getNextPlayer(int(1)), testee.get(10));
    a.checkEqual("43. getNextPlayer", testee.getNextPlayer(int(5)), testee.get(10));
    a.checkNull ("44. getNextPlayer", testee.getNextPlayer(int(12)));
    a.checkNull ("45. getNextPlayer", testee.getNextPlayer(int(120)));
    a.checkNull ("46. getNextPlayer", testee.getNextPlayer(int(100000000)));
    a.checkEqual("47. getNextPlayer", testee.getNextPlayer(int(-1)), testee.get(1));
    a.checkEqual("48. getNextPlayer", testee.getNextPlayer(int(-100000000)), testee.get(1));
}

/** Test setup and inquiry. */
AFL_TEST("game.PlayerList:setup", a)
{
    // Verify initial state
    game::PlayerList testee;
    a.checkEqual("01. size", testee.size(), 1);
    a.checkEqual("02. getAllPlayers", testee.getAllPlayers().toInteger(), 0U);
    a.checkNull("03. getNextPlayer", testee.getNextPlayer(0));

    // Add some players
    testee.create(1);
    testee.create(2);
    testee.create(5);
    a.checkEqual("11. size", testee.size(), 6);
    a.checkEqual("12. getAllPlayers", testee.getAllPlayers().toInteger(), 0x26U);

    // We didn't add 3, so this remains 0. Others exist.
    a.checkNonNull("21. get", testee.get(0));
    a.checkNonNull("22. get", testee.get(1));
    a.checkNonNull("23. get", testee.get(2));
    a.checkNull   ("24. get", testee.get(3));
    a.checkNonNull("25. get", testee.get(5));

    // Turn player 5 into a non-player
    game::Player* p = testee.get(5);
    a.check("31. get", p);
    p->setIsReal(false);
    a.checkEqual("32. getAllPlayers", testee.getAllPlayers().toInteger(), 6U);
}

/** Test character conversion. */
AFL_TEST("game.PlayerList:char", a)
{
    game::PlayerList testee;
    testee.create(1);
    testee.create(2);
    testee.create(5);
    testee.create(9);
    testee.create(10);
    testee.create(20);

    // Successful queries
    a.checkEqual("01. getPlayerFromCharacter", testee.getPlayerFromCharacter('0'), testee.get(0));
    a.checkEqual("02. getPlayerFromCharacter", testee.getPlayerFromCharacter('1'), testee.get(1));
    a.checkEqual("03. getPlayerFromCharacter", testee.getPlayerFromCharacter('2'), testee.get(2));
    a.checkEqual("04. getPlayerFromCharacter", testee.getPlayerFromCharacter('5'), testee.get(5));
    a.checkEqual("05. getPlayerFromCharacter", testee.getPlayerFromCharacter('9'), testee.get(9));
    a.checkEqual("06. getPlayerFromCharacter", testee.getPlayerFromCharacter('a'), testee.get(10));
    a.checkEqual("07. getPlayerFromCharacter", testee.getPlayerFromCharacter('K'), testee.get(20));
    a.checkEqual("08. getPlayerFromCharacter", testee.getPlayerFromCharacter('k'), testee.get(20));

    // Unsuccessful queries
    a.checkNull("11. getPlayerFromCharacter", testee.getPlayerFromCharacter('4'));
    a.checkNull("12. getPlayerFromCharacter", testee.getPlayerFromCharacter('L'));
    a.checkNull("13. getPlayerFromCharacter", testee.getPlayerFromCharacter('B'));
    a.checkNull("14. getPlayerFromCharacter", testee.getPlayerFromCharacter('Z'));
    a.checkNull("15. getPlayerFromCharacter", testee.getPlayerFromCharacter('@'));
    a.checkNull("16. getPlayerFromCharacter", testee.getPlayerFromCharacter(0));

    // Other direction
    a.checkEqual("21. getCharacterFromPlayer", testee.getCharacterFromPlayer(0), '0');
    a.checkEqual("22. getCharacterFromPlayer", testee.getCharacterFromPlayer(1), '1');
    a.checkEqual("23. getCharacterFromPlayer", testee.getCharacterFromPlayer(9), '9');
    a.checkEqual("24. getCharacterFromPlayer", testee.getCharacterFromPlayer(10), 'A');
    a.checkEqual("25. getCharacterFromPlayer", testee.getCharacterFromPlayer(20), 'K');
    a.checkEqual("26. getCharacterFromPlayer", testee.getCharacterFromPlayer(30), 'U');
    a.checkEqual("27. getCharacterFromPlayer", testee.getCharacterFromPlayer(-1), '\0');
    a.checkEqual("28. getCharacterFromPlayer", testee.getCharacterFromPlayer(40), '\0');
    a.checkEqual("29. getCharacterFromPlayer", testee.getCharacterFromPlayer(260), '\0');
    a.checkEqual("30. getCharacterFromPlayer", testee.getCharacterFromPlayer(100000000), '\0');
}

/** Test notifyListeners(). */
AFL_TEST("game.PlayerList:notify", a)
{
    // Create a PlayerList with a listener
    game::test::Counter c;
    game::PlayerList testee;
    testee.sig_change.add(&c, &game::test::Counter::increment);
    a.checkEqual("01. count", c.get(), 0);

    // Adding players registers as a change
    testee.create(2);
    testee.create(5);
    testee.notifyListeners();
    a.checkEqual("11. count", c.get(), 1);

    // Modify a player
    testee.get(2)->setName(game::Player::LongName, "Long");
    testee.notifyListeners();
    a.checkEqual("21. count", c.get(), 2);

    // Notify again does not longer call the listener because it has reset the status
    testee.notifyListeners();
    a.checkEqual("31. count", c.get(), 2);

    // Same thing again, now modify both
    testee.get(2)->setName(game::Player::LongName, "2");
    testee.get(5)->setName(game::Player::LongName, "2");
    testee.notifyListeners();
    a.checkEqual("41. count", c.get(), 3);

    // Notify again does not longer call the listener because it has reset the status
    testee.notifyListeners();
    a.checkEqual("51. count", c.get(), 3);

    // Re-adding a player no longer counts as a change...
    testee.create(2);
    testee.notifyListeners();
    a.checkEqual("61. count", c.get(), 3);

    // ...but adding a new one does
    testee.create(9);
    testee.notifyListeners();
    a.checkEqual("71. count", c.get(), 4);
}

/** Test getPlayerName(), getPlayerNames(). */
AFL_TEST("game.PlayerList:getPlayerName", a)
{
    afl::string::NullTranslator tx;
    game::PlayerList testee;
    game::Player* p = testee.create(3);
    p->setName(game::Player::LongName, "Long");
    p->setName(game::Player::EmailAddress, "e@mai.l");

    a.checkEqual("01", testee.getPlayerName(3, game::Player::LongName, tx), "Long");
    a.checkEqual("02", testee.getPlayerName(3, game::Player::EmailAddress, tx), "e@mai.l");
    a.checkEqual("03", testee.getPlayerName(3, game::Player::AdjectiveName, tx), "Player 3");

    a.checkEqual("11", testee.getPlayerName(1, game::Player::LongName, tx), "Player 1");
    a.checkEqual("12", testee.getPlayerName(1, game::Player::EmailAddress, tx), "");
    a.checkEqual("13", testee.getPlayerName(1, game::Player::AdjectiveName, tx), "Player 1");

    game::PlayerArray<String_t> names = testee.getPlayerNames(game::Player::LongName, tx);
    a.checkEqual("21", names.get(3), "Long");
    a.checkEqual("22", names.get(1), "");          // No fallback names for empty slots

    game::PlayerArray<String_t> adj = testee.getPlayerNames(game::Player::AdjectiveName, tx);
    a.checkEqual("31", adj.get(3), "Player 3");    // Fallback name for unset name
    a.checkEqual("32", adj.get(1), "");            // No fallback names for empty slots
}
