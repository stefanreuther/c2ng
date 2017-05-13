/**
  *  \file u/t_game_playerlist.cpp
  *  \brief Test for game::PlayerList
  */

#include "game/playerlist.hpp"

#include "t_game.hpp"
#include "game/player.hpp"
#include "afl/charset/utf8reader.hpp"
#include "helper/counter.hpp"

/** Test setup and expandNames(). */
void
TestGamePlayerList::testExpand()
{
    // ex GameRaceNameTestSuite::testRaceNameList
    using game::Player;

    game::PlayerList testee;

    // Construct a race name list
    game::Player* pl = testee.create(1);
    TS_ASSERT(pl);
    pl->setName(Player::ShortName,     "The Feds");
    pl->setName(Player::LongName,      "The Solar Federation");
    pl->setName(Player::AdjectiveName, "Fed");

    pl = testee.create(2);
    TS_ASSERT(pl);
    pl->setName(Player::ShortName,     "The Lizards");
    pl->setName(Player::LongName,      "The Lizard Alliance");
    pl->setName(Player::AdjectiveName, "Lizard");

    pl = testee.create(5);
    TS_ASSERT(pl);
    pl->setName(Player::ShortName,     "The Privateers");
    pl->setName(Player::LongName,      "The Privateer Bands");
    pl->setName(Player::AdjectiveName, "Privateer");

    pl = testee.create(10);
    TS_ASSERT(pl);
    pl->setName(Player::ShortName,     "The Rebels");
    pl->setName(Player::LongName,      "The Rebel Confederation");
    pl->setName(Player::AdjectiveName, "Rebel");

    pl = testee.create(11);
    TS_ASSERT(pl);
    pl->setName(Player::ShortName,     "The Colonies");
    pl->setName(Player::LongName,      "The Missing Colonies of Man");
    pl->setName(Player::AdjectiveName, "Colonial");

    pl = testee.create(12);
    TS_ASSERT(pl);
    pl->initAlien();

    // Check it
    pl = testee.get(1);
    TS_ASSERT(pl);
    TS_ASSERT_EQUALS(pl->getName(Player::ShortName), "The Feds");
    TS_ASSERT(pl->isReal());

    pl = testee.get(11);
    TS_ASSERT(pl);
    TS_ASSERT_EQUALS(pl->getName(Player::ShortName), "The Colonies");
    TS_ASSERT(pl->isReal());

    pl = testee.get(0);
    TS_ASSERT(pl);
    TS_ASSERT_EQUALS(pl->getName(Player::ShortName), "Nobody");
    TS_ASSERT(!pl->isReal());

    pl = testee.get(12);
    TS_ASSERT(pl);
    TS_ASSERT_EQUALS(pl->getName(Player::ShortName), "Alien Marauders");
    TS_ASSERT(!pl->isReal());

    // We never set these
    TS_ASSERT(testee.get(3) == 0);
    TS_ASSERT(testee.get(13) == 0);
    TS_ASSERT(testee.get(23) == 0);

    TS_ASSERT(testee.get(-1) == 0);
    TS_ASSERT(testee.get(10000) == 0);

    // Not creatible
    TS_ASSERT(testee.create(-1) == 0);

    // Expansions
    TS_ASSERT_EQUALS(testee.expandNames("a %-5 ship"), "a Privateer ship");
    TS_ASSERT_EQUALS(testee.expandNames("attack %5!"), "attack The Privateers!");
    TS_ASSERT_EQUALS(testee.expandNames("%1..."), "The Feds...");
    TS_ASSERT_EQUALS(testee.expandNames("%1."), "The Feds.");
    TS_ASSERT_EQUALS(testee.expandNames("%1"), "The Feds");
    TS_ASSERT_EQUALS(testee.expandNames("%a..."), "The Rebels...");
    TS_ASSERT_EQUALS(testee.expandNames("...%b"), "...The Colonies");
    TS_ASSERT_EQUALS(testee.expandNames("%A..."), "The Rebels...");
    TS_ASSERT_EQUALS(testee.expandNames("...%B"), "...The Colonies");
    TS_ASSERT_EQUALS(testee.expandNames("%-A..."), "Rebel...");
    TS_ASSERT_EQUALS(testee.expandNames("...%-B"), "...Colonial");
    TS_ASSERT_EQUALS(testee.expandNames("%1%2"), "The FedsThe Lizards");
    TS_ASSERT_EQUALS(testee.expandNames("a%%b"), "a%b");
    TS_ASSERT_EQUALS(testee.expandNames("%%"), "%");
    TS_ASSERT_EQUALS(testee.expandNames("%%1"), "%1");

    // Those are out-of-spec. As of 20110102, '%' quotes, i.e. keeps the
    // offending character (this is to avoid eating partial UTF-8 runes).
    // \change Unlike PCC2, c2ng expands %0 and %c because we have corresponding slots in our table.
    TS_ASSERT_EQUALS(testee.expandNames("%0"), "Nobody");
    TS_ASSERT_EQUALS(testee.expandNames("%01"), "Nobody1");
    TS_ASSERT_EQUALS(testee.expandNames("%c"), "Alien Marauders");
    TS_ASSERT_EQUALS(testee.expandNames("%d"), "d");
    TS_ASSERT_EQUALS(testee.expandNames("%x"), "x");
    TS_ASSERT_EQUALS(testee.expandNames("%."), ".");
    TS_ASSERT_EQUALS(testee.expandNames("%-"), "");
    TS_ASSERT_EQUALS(testee.expandNames("%-."), ".");
    TS_ASSERT_EQUALS(testee.expandNames("%"), "");

    // Make sure this doesn't produce error characters by breaking UTF-8
    String_t a = testee.expandNames("a%\xc3\x80");
    afl::charset::Utf8Reader rdr(afl::string::toBytes(a), 0);
    while (rdr.hasMore()) {
        afl::charset::Unichar_t ch = rdr.eat();
        TS_ASSERT(!afl::charset::isErrorCharacter(ch));
    }
}

/** Test iteration. */
void
TestGamePlayerList::testIteration()
{
    game::PlayerList testee;
    testee.create(10);
    testee.create(1);
    testee.create(12);

    // First: 1
    game::Player* pl = testee.getFirstPlayer();
    TS_ASSERT(pl != 0);
    TS_ASSERT_EQUALS(pl->getId(), 1);
    TS_ASSERT_EQUALS(pl, testee.get(1));

    // Second: 10
    pl = testee.getNextPlayer(pl);
    TS_ASSERT(pl != 0);
    TS_ASSERT_EQUALS(pl->getId(), 10);
    TS_ASSERT_EQUALS(pl, testee.get(10));

    // Last: 12
    pl = testee.getNextPlayer(pl);
    TS_ASSERT(pl != 0);
    TS_ASSERT_EQUALS(pl->getId(), 12);
    TS_ASSERT_EQUALS(pl, testee.get(12));

    // Final
    pl = testee.getNextPlayer(pl);
    TS_ASSERT(pl == 0);
    TS_ASSERT(testee.getNextPlayer(pl) == 0);

    // Iteration from number
    TS_ASSERT_EQUALS(testee.getNextPlayer(int(0)), testee.get(1));
    TS_ASSERT_EQUALS(testee.getNextPlayer(int(1)), testee.get(10));
    TS_ASSERT_EQUALS(testee.getNextPlayer(int(5)), testee.get(10));
    TS_ASSERT(testee.getNextPlayer(int(12)) == 0);
    TS_ASSERT(testee.getNextPlayer(int(120)) == 0);
    TS_ASSERT(testee.getNextPlayer(int(100000000)) == 0);
    TS_ASSERT_EQUALS(testee.getNextPlayer(int(-1)), testee.get(1));
    TS_ASSERT_EQUALS(testee.getNextPlayer(int(-100000000)), testee.get(1));
}

/** Test setup and inquiry. */
void
TestGamePlayerList::testSetup()
{
    // Verify initial state
    game::PlayerList testee;
    TS_ASSERT_EQUALS(testee.size(), 1);
    TS_ASSERT_EQUALS(testee.getAllPlayers().toInteger(), 0U);
    TS_ASSERT(testee.getNextPlayer(0) == 0);

    // Add some players
    testee.create(1);
    testee.create(2);
    testee.create(5);
    TS_ASSERT_EQUALS(testee.size(), 6);
    TS_ASSERT_EQUALS(testee.getAllPlayers().toInteger(), 0x26U);

    // We didn't add 3, so this remains 0. Others exist.
    TS_ASSERT(testee.get(0) != 0);
    TS_ASSERT(testee.get(1) != 0);
    TS_ASSERT(testee.get(2) != 0);
    TS_ASSERT(testee.get(3) == 0);
    TS_ASSERT(testee.get(5) != 0);

    // Turn player 5 into a non-player
    game::Player* p = testee.get(5);
    TS_ASSERT(p);
    p->setIsReal(false);
    TS_ASSERT_EQUALS(testee.getAllPlayers().toInteger(), 6U);
}

/** Test character conversion. */
void
TestGamePlayerList::testChar()
{
    game::PlayerList testee;
    testee.create(1);
    testee.create(2);
    testee.create(5);
    testee.create(9);
    testee.create(10);
    testee.create(20);

    // Successful queries
    TS_ASSERT_EQUALS(testee.getPlayerFromCharacter('0'), testee.get(0));
    TS_ASSERT_EQUALS(testee.getPlayerFromCharacter('1'), testee.get(1));
    TS_ASSERT_EQUALS(testee.getPlayerFromCharacter('2'), testee.get(2));
    TS_ASSERT_EQUALS(testee.getPlayerFromCharacter('5'), testee.get(5));
    TS_ASSERT_EQUALS(testee.getPlayerFromCharacter('9'), testee.get(9));
    TS_ASSERT_EQUALS(testee.getPlayerFromCharacter('a'), testee.get(10));
    TS_ASSERT_EQUALS(testee.getPlayerFromCharacter('K'), testee.get(20));
    TS_ASSERT_EQUALS(testee.getPlayerFromCharacter('k'), testee.get(20));

    // Unsuccessful queries
    TS_ASSERT(testee.getPlayerFromCharacter('4') == 0);
    TS_ASSERT(testee.getPlayerFromCharacter('L') == 0);
    TS_ASSERT(testee.getPlayerFromCharacter('B') == 0);
    TS_ASSERT(testee.getPlayerFromCharacter('Z') == 0);
    TS_ASSERT(testee.getPlayerFromCharacter('@') == 0);
    TS_ASSERT(testee.getPlayerFromCharacter(0) == 0);

    // Other direction
    TS_ASSERT_EQUALS(testee.getCharacterFromPlayer(0), '0');
    TS_ASSERT_EQUALS(testee.getCharacterFromPlayer(1), '1');
    TS_ASSERT_EQUALS(testee.getCharacterFromPlayer(9), '9');
    TS_ASSERT_EQUALS(testee.getCharacterFromPlayer(10), 'A');
    TS_ASSERT_EQUALS(testee.getCharacterFromPlayer(20), 'K');
    TS_ASSERT_EQUALS(testee.getCharacterFromPlayer(30), 'U');
    TS_ASSERT_EQUALS(testee.getCharacterFromPlayer(-1), '\0');
    TS_ASSERT_EQUALS(testee.getCharacterFromPlayer(40), '\0');
    TS_ASSERT_EQUALS(testee.getCharacterFromPlayer(260), '\0');
    TS_ASSERT_EQUALS(testee.getCharacterFromPlayer(100000000), '\0');
}

/** Test notifyListeners(). */
void
TestGamePlayerList::testNotify()
{
    // Create a PlayerList with a listener
    Counter c;
    game::PlayerList testee;
    testee.sig_change.add(&c, &Counter::increment);
    TS_ASSERT_EQUALS(c.get(), 0);

    // Adding players registers as a change
    testee.create(2);
    testee.create(5);
    testee.notifyListeners();
    TS_ASSERT_EQUALS(c.get(), 1);

    // Modify a player
    testee.get(2)->setName(game::Player::LongName, "Long");
    testee.notifyListeners();
    TS_ASSERT_EQUALS(c.get(), 2);

    // Notify again does not longer call the listener because it has reset the status
    testee.notifyListeners();
    TS_ASSERT_EQUALS(c.get(), 2);

    // Same thing again, now modify both
    testee.get(2)->setName(game::Player::LongName, "2");
    testee.get(5)->setName(game::Player::LongName, "2");
    testee.notifyListeners();
    TS_ASSERT_EQUALS(c.get(), 3);

    // Notify again does not longer call the listener because it has reset the status
    testee.notifyListeners();
    TS_ASSERT_EQUALS(c.get(), 3);

    // Re-adding a player no longer counts as a change...
    testee.create(2);
    testee.notifyListeners();
    TS_ASSERT_EQUALS(c.get(), 3);

    // ...but adding a new one does
    testee.create(9);
    testee.notifyListeners();
    TS_ASSERT_EQUALS(c.get(), 4);
}

