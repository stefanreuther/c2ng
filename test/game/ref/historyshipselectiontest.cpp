/**
  *  \file test/game/ref/historyshipselectiontest.cpp
  *  \brief Test for game::ref::HistoryShipSelection
  */

#include "game/ref/historyshipselection.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/configuration.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/ref/historyshiplist.hpp"
#include "game/test/root.hpp"

using game::TeamSettings;
using game::map::Configuration;
using game::map::Point;
using game::map::Universe;
using game::ref::HistoryShipSelection;

namespace {
    const int TURN_NR = 32;

    game::map::Ship& addShip(Universe& u, game::Id_t id, Point pos, int owner)
    {
        // Let source be different from owner, to make this "true" scans.
        // With source=owner, Ship::internalCheck() would discard the ships as bogons,
        // because they should have got a proper full record (addCurrentShipData).
        game::PlayerSet_t source(owner+1);

        game::map::Ship& sh = *u.ships().create(id);
        sh.addShipXYData(pos, owner, 100, source);
        sh.internalCheck(source, TURN_NR);
        sh.setPlayability(game::map::Object::NotPlayable);

        return sh;
    }

    void addShipTrack(game::map::Ship& ship, int age, Point pos)
    {
        game::parser::MessageInformation mi(game::parser::MessageInformation::Ship, ship.getId(), TURN_NR - age);
        mi.addValue(game::parser::mi_X, pos.getX());
        mi.addValue(game::parser::mi_Y, pos.getY());
        mi.addValue(game::parser::mi_Mass, 100);
        ship.addMessageInformation(mi, game::PlayerSet_t());
    }

    void addShipNonTrack(game::map::Ship& ship, int age)
    {
        game::parser::MessageInformation mi(game::parser::MessageInformation::Ship, ship.getId(), TURN_NR - age);
        mi.addValue(game::parser::mi_Mass, 100);
        ship.addMessageInformation(mi, game::PlayerSet_t());
    }

    String_t toString(HistoryShipSelection::Modes_t m)
    {
        String_t result;
        if (m.contains(HistoryShipSelection::AllShips))     { result += 'a'; }
        if (m.contains(HistoryShipSelection::LocalShips))   { result += 'l'; }
        if (m.contains(HistoryShipSelection::ExactShips))   { result += 'x'; }
        if (m.contains(HistoryShipSelection::ForeignShips)) { result += 'f'; }
        if (m.contains(HistoryShipSelection::TeamShips))    { result += 't'; }
        if (m.contains(HistoryShipSelection::EnemyShips))   { result += 'e'; }
        if (m.contains(HistoryShipSelection::OwnShips))     { result += 'o'; }
        return result;
    }
}


/** Test basic operations. */
AFL_TEST("game.ref.HistoryShipSelection:basics", a)
{
    afl::string::NullTranslator tx;
    HistoryShipSelection t;

    a.checkEqual("01. getMode", t.getMode(), HistoryShipSelection::AllShips);
    a.checkEqual("02. getSortOrder", t.getSortOrder(), HistoryShipSelection::ById);

    t.setMode(HistoryShipSelection::OwnShips);
    t.setSortOrder(HistoryShipSelection::ByHull);
    t.setPosition(game::map::Point(1000, 2000));
    a.checkEqual("11. getMode", t.getMode(), HistoryShipSelection::OwnShips);
    a.checkEqual("12. getSortOrder", t.getSortOrder(), HistoryShipSelection::ByHull);

    a.checkEqual("21. getModeName", t.getModeName(tx), t.getModeName(HistoryShipSelection::OwnShips, tx));
    a.checkEqual("22. getSortOrderName", t.getSortOrderName(tx), t.getSortOrderName(HistoryShipSelection::ByHull, tx));

    a.checkEqual("31. getModeName", t.getModeName(HistoryShipSelection::LocalShips, tx), "Ships near (1000,2000)");
    a.checkEqual("32. getSortOrderName", t.getSortOrderName(HistoryShipSelection::ByName, tx), "Sort by Name");

    // All modes need to be printable
    for (size_t i = 0; i <= HistoryShipSelection::ModeMax; ++i) {
        a.checkDifferent("41. getModeName", t.getModeName(HistoryShipSelection::Mode(i), tx), "");
    }

    // All sort orders need to be printable
    for (size_t i = 0; i <= HistoryShipSelection::SortMax; ++i) {
        a.checkDifferent("51. getSortOrderName", t.getSortOrderName(HistoryShipSelection::SortOrder(i), tx), "");
    }
}

/** Test operations on mode sets. */
AFL_TEST("game.ref.HistoryShipSelection:mode-set", a)
{
    const Configuration mapConfig;

    // Team settings with no teams
    game::TeamSettings noTeams;
    noTeams.setViewpointPlayer(3);

    // Team settings with teams; 4+3 in one team
    game::TeamSettings hasTeams;
    hasTeams.setViewpointPlayer(3);
    hasTeams.setPlayerTeam(4, 3);

    // Universe with just player 3 ships
    Universe u3;
    addShip(u3, 1, Point(1000, 1000), 3);
    addShip(u3, 2, Point(1000, 1000), 3);

    // Universe with just player 4 ships
    Universe u4;
    addShip(u4, 1, Point(1000, 1000), 4);
    addShip(u4, 2, Point(1000, 1000), 4);

    // Universe with just player 5 ships
    Universe u5;
    addShip(u5, 1, Point(1000, 1000), 5);
    addShip(u5, 2, Point(1000, 1000), 5);

    // Universe with player 3+5 ships
    Universe u35;
    addShip(u35, 1, Point(1000, 1000), 5);
    addShip(u35, 2, Point(1000, 1000), 3);

    // Verify all combinations against HistoryShipSelection with no position
    {
        HistoryShipSelection t;

        // No teams
        a.checkEqual("01. getAvailableModes", toString(t.getAvailableModes(u3, mapConfig, noTeams)), "ao");
        a.checkEqual("02. getInitialMode", t.getInitialMode(u3, mapConfig, noTeams), HistoryShipSelection::AllShips);

        a.checkEqual("11. getAvailableModes", toString(t.getAvailableModes(u4, mapConfig, noTeams)), "af");
        a.checkEqual("12. getInitialMode", t.getInitialMode(u4, mapConfig, noTeams), HistoryShipSelection::AllShips);

        a.checkEqual("21. getAvailableModes", toString(t.getAvailableModes(u5, mapConfig, noTeams)), "af");
        a.checkEqual("22. getInitialMode", t.getInitialMode(u5, mapConfig, noTeams), HistoryShipSelection::AllShips);

        a.checkEqual("31. getAvailableModes", toString(t.getAvailableModes(u35, mapConfig, noTeams)), "afo");
        a.checkEqual("32. getInitialMode", t.getInitialMode(u35, mapConfig, noTeams), HistoryShipSelection::AllShips);

        // With teams
        a.checkEqual("41. getAvailableModes", toString(t.getAvailableModes(u3, mapConfig, hasTeams)), "ato");
        a.checkEqual("42. getInitialMode", t.getInitialMode(u3, mapConfig, hasTeams), HistoryShipSelection::AllShips);

        a.checkEqual("51. getAvailableModes", toString(t.getAvailableModes(u4, mapConfig, hasTeams)), "aft");
        a.checkEqual("52. getInitialMode", t.getInitialMode(u4, mapConfig, hasTeams), HistoryShipSelection::AllShips);

        a.checkEqual("61. getAvailableModes", toString(t.getAvailableModes(u5, mapConfig, hasTeams)), "afe");
        a.checkEqual("62. getInitialMode", t.getInitialMode(u5, mapConfig, hasTeams), HistoryShipSelection::AllShips);

        a.checkEqual("71. getAvailableModes", toString(t.getAvailableModes(u35, mapConfig, hasTeams)), "afteo");
        a.checkEqual("72. getInitialMode", t.getInitialMode(u35, mapConfig, hasTeams), HistoryShipSelection::AllShips);
    }

    // Verify all combinations against HistoryShipSelection with exact position
    {
        HistoryShipSelection t;
        t.setPosition(Point(1000, 1000));

        // No teams
        a.checkEqual("81. getAvailableModes", toString(t.getAvailableModes(u3, mapConfig, noTeams)), "alxo");
        a.checkEqual("82. getInitialMode", t.getInitialMode(u3, mapConfig, noTeams), HistoryShipSelection::LocalShips);

        a.checkEqual("91. getAvailableModes", toString(t.getAvailableModes(u4, mapConfig, noTeams)), "alxf");
        a.checkEqual("92. getInitialMode", t.getInitialMode(u4, mapConfig, noTeams), HistoryShipSelection::LocalShips);

        a.checkEqual("101. getAvailableModes", toString(t.getAvailableModes(u5, mapConfig, noTeams)), "alxf");
        a.checkEqual("102. getInitialMode", t.getInitialMode(u5, mapConfig, noTeams), HistoryShipSelection::LocalShips);

        a.checkEqual("111. getAvailableModes", toString(t.getAvailableModes(u35, mapConfig, noTeams)), "alxfo");
        a.checkEqual("112. getInitialMode", t.getInitialMode(u35, mapConfig, noTeams), HistoryShipSelection::LocalShips);

        // With teams
        a.checkEqual("121. getAvailableModes", toString(t.getAvailableModes(u3, mapConfig, hasTeams)), "alxto");
        a.checkEqual("122. getInitialMode", t.getInitialMode(u3, mapConfig, hasTeams), HistoryShipSelection::LocalShips);

        a.checkEqual("131. getAvailableModes", toString(t.getAvailableModes(u4, mapConfig, hasTeams)), "alxft");
        a.checkEqual("132. getInitialMode", t.getInitialMode(u4, mapConfig, hasTeams), HistoryShipSelection::LocalShips);

        a.checkEqual("141. getAvailableModes", toString(t.getAvailableModes(u5, mapConfig, hasTeams)), "alxfe");
        a.checkEqual("142. getInitialMode", t.getInitialMode(u5, mapConfig, hasTeams), HistoryShipSelection::LocalShips);

        a.checkEqual("151. getAvailableModes", toString(t.getAvailableModes(u35, mapConfig, hasTeams)), "alxfteo");
        a.checkEqual("152. getInitialMode", t.getInitialMode(u35, mapConfig, hasTeams), HistoryShipSelection::LocalShips);
    }

    // Verify all combinations against HistoryShipSelection with a close position
    {
        HistoryShipSelection t;
        t.setPosition(Point(1000, 1001));

        // No teams
        a.checkEqual("161. getAvailableModes", toString(t.getAvailableModes(u3, mapConfig, noTeams)), "alo");
        a.checkEqual("162. getInitialMode", t.getInitialMode(u3, mapConfig, noTeams), HistoryShipSelection::LocalShips);

        a.checkEqual("171. getAvailableModes", toString(t.getAvailableModes(u4, mapConfig, noTeams)), "alf");
        a.checkEqual("172. getInitialMode", t.getInitialMode(u4, mapConfig, noTeams), HistoryShipSelection::LocalShips);

        a.checkEqual("181. getAvailableModes", toString(t.getAvailableModes(u5, mapConfig, noTeams)), "alf");
        a.checkEqual("182. getInitialMode", t.getInitialMode(u5, mapConfig, noTeams), HistoryShipSelection::LocalShips);

        a.checkEqual("191. getAvailableModes", toString(t.getAvailableModes(u35, mapConfig, noTeams)), "alfo");
        a.checkEqual("192. getInitialMode", t.getInitialMode(u35, mapConfig, noTeams), HistoryShipSelection::LocalShips);

        // With teams
        a.checkEqual("201. getAvailableModes", toString(t.getAvailableModes(u3, mapConfig, hasTeams)), "alto");
        a.checkEqual("202. getInitialMode", t.getInitialMode(u3, mapConfig, hasTeams), HistoryShipSelection::LocalShips);

        a.checkEqual("211. getAvailableModes", toString(t.getAvailableModes(u4, mapConfig, hasTeams)), "alft");
        a.checkEqual("212. getInitialMode", t.getInitialMode(u4, mapConfig, hasTeams), HistoryShipSelection::LocalShips);

        a.checkEqual("221. getAvailableModes", toString(t.getAvailableModes(u5, mapConfig, hasTeams)), "alfe");
        a.checkEqual("222. getInitialMode", t.getInitialMode(u5, mapConfig, hasTeams), HistoryShipSelection::LocalShips);

        a.checkEqual("231. getAvailableModes", toString(t.getAvailableModes(u35, mapConfig, hasTeams)), "alfteo");
        a.checkEqual("232. getInitialMode", t.getInitialMode(u35, mapConfig, hasTeams), HistoryShipSelection::LocalShips);
    }
}

/** Test buildList(). */
AFL_TEST("game.ref.HistoryShipSelection:buildList", a)
{
    // Turn/universe
    game::Turn t;
    addShip(t.universe(), 1, Point(1000, 1000), 3).setName("i1");
    addShip(t.universe(), 2, Point(1000, 1000), 3).setName("i2");
    addShip(t.universe(), 3, Point(1000, 1000), 4).setName("i3");
    addShip(t.universe(), 4, Point(1000, 1000), 4).setName("i4");
    addShip(t.universe(), 5, Point(1000, 1000), 3).setName("i5");
    t.setTurnNumber(TURN_NR);

    // Session
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());

    // Build it
    HistoryShipSelection testee;
    testee.setMode(HistoryShipSelection::AllShips);
    testee.setSortOrder(HistoryShipSelection::ByOwner);

    game::ref::HistoryShipList list;
    testee.buildList(list, t, session);

    // Verify
    // Note: when naming things, we always go through Session->Game->ViewpointTurn (via Session::getReferenceName).
    // Since our turn is not connected to the rest, our names are ignored here.
    a.checkEqual("01. size", list.size(), 7U);
    a.checkEqual("02. name", list.get(0)->name, "Player 3");
    a.checkEqual("03. name", list.get(1)->name, "Ship #1");
    a.checkEqual("04. name", list.get(2)->name, "Ship #2");
    a.checkEqual("05. name", list.get(3)->name, "Ship #5");
    a.checkEqual("06. name", list.get(4)->name, "Player 4");
    a.checkEqual("07. name", list.get(5)->name, "Ship #3");
    a.checkEqual("08. name", list.get(6)->name, "Ship #4");
    a.checkEqual("09. name", list.get(6)->turnNumber, TURN_NR);
    a.checkEqual("10. getReferenceTurn", list.getReferenceTurn(), TURN_NR);
}

/** Test buildList(), with history. */
AFL_TEST("game.ref.HistoryShipSelection:buildList:history", a)
{
    const int ME = 3;
    const int ALLY = 4;
    const int ENEMY = 5;

    // Turn/universe
    game::Turn t;
    game::map::Ship& s1 = addShip(t.universe(), 1, Point(1000, 1000), ME);
    addShipTrack(s1, 1, Point(1000, 1020));
    addShipTrack(s1, 2, Point(1000, 1040));

    game::map::Ship& s2 = addShip(t.universe(), 2, Point(1000, 1000), ALLY);
    addShipTrack(s2, 1, Point(1000, 1040));

    /*game::map::Ship& s3 =*/ addShip(t.universe(), 3, Point(1000, 1000), ENEMY);
    t.setTurnNumber(TURN_NR);

    // Session
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());
    session.getGame()->teamSettings().setViewpointPlayer(ME);
    session.getGame()->teamSettings().setPlayerTeam(ALLY, ME);

    // Ships near (1000, 1035)
    game::ref::HistoryShipList list;
    HistoryShipSelection testee;
    testee.setMode(HistoryShipSelection::LocalShips);
    testee.setSortOrder(HistoryShipSelection::ByAge);
    testee.setPosition(Point(1000, 1035));
    testee.buildList(list, t, session);

    // Verify
    a.checkEqual("01. size", list.size(), 4U);
    a.checkEqual("02. name", list.get(0)->name, "previous turn");
    a.checkEqual("03. name", list.get(1)->name, "Ship #2");
    a.checkEqual("04. name", list.get(2)->name, "2 turns ago");
    a.checkEqual("05. name", list.get(3)->name, "Ship #1");

    // Own ships
    testee.setMode(HistoryShipSelection::OwnShips);
    testee.buildList(list, t, session);
    a.checkEqual("11. size", list.size(), 2U);
    a.checkEqual("12. name", list.get(0)->name, "current turn");
    a.checkEqual("13. name", list.get(1)->name, "Ship #1");

    // Team ships
    testee.setMode(HistoryShipSelection::TeamShips);
    testee.buildList(list, t, session);
    a.checkEqual("21. size", list.size(), 3U);
    a.checkEqual("22. name", list.get(0)->name, "current turn");
    a.checkEqual("23. name", list.get(1)->name, "Ship #1");
    a.checkEqual("24. name", list.get(2)->name, "Ship #2");

    // Enemy ships
    testee.setMode(HistoryShipSelection::EnemyShips);
    testee.buildList(list, t, session);
    a.checkEqual("31. size", list.size(), 2U);
    a.checkEqual("32. name", list.get(0)->name, "current turn");
    a.checkEqual("33. name", list.get(1)->name, "Ship #3");

    // Foreign ships
    testee.setMode(HistoryShipSelection::ForeignShips);
    testee.buildList(list, t, session);
    a.checkEqual("41. size", list.size(), 3U);
    a.checkEqual("42. name", list.get(0)->name, "current turn");
    a.checkEqual("43. name", list.get(1)->name, "Ship #2");
    a.checkEqual("44. name", list.get(2)->name, "Ship #3");

    // Exact location: fails!
    testee.setMode(HistoryShipSelection::ExactShips);
    testee.buildList(list, t, session);
    a.checkEqual("51. size", list.size(), 0U);
    a.check("52. getAvailableModes", !testee.getAvailableModes(t.universe(), session.getGame()->mapConfiguration(), session.getGame()->teamSettings()).contains(HistoryShipSelection::ExactShips));

    // Exact location: succeeds with different location
    testee.setPosition(Point(1000, 1000));
    a.check("61. getAvailableModes", testee.getAvailableModes(t.universe(), session.getGame()->mapConfiguration(), session.getGame()->teamSettings()).contains(HistoryShipSelection::ExactShips));
    testee.setSortOrder(HistoryShipSelection::ByOwner);
    testee.buildList(list, t, session);
    a.checkEqual("62. size", list.size(), 6U);
    a.checkEqual("63. name", list.get(0)->name, "Player 3");
    a.checkEqual("64. name", list.get(1)->name, "Ship #1");
    a.checkEqual("65. name", list.get(2)->name, "Player 4");
    a.checkEqual("66. name", list.get(3)->name, "Ship #2");
    a.checkEqual("67. name", list.get(4)->name, "Player 5");
    a.checkEqual("68. name", list.get(5)->name, "Ship #3");
}

/** Test buildList(), with ships that ONLY have history. */
AFL_TEST("game.ref.HistoryShipSelection:buildList:history-only", a)
{
    const int ME = 3;

    // Turn/universe with a ship that we saw last time 5 turns ago,
    // but also has a record from 4 turns ago.
    // (This exercises the loop in getShipLastTurn which is easy to get wrong because it goes backwards.)
    game::Turn t;
    game::map::Ship& s1 = *t.universe().ships().create(1);
    s1.setOwner(ME);
    s1.internalCheck(game::PlayerSet_t(), TURN_NR);
    addShipNonTrack(s1, 4);
    addShipTrack(s1, 5, Point(1000, 1020));
    t.setTurnNumber(TURN_NR);

    // Session
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());
    session.getGame()->teamSettings().setViewpointPlayer(ME);

    // All ships
    game::ref::HistoryShipList list;
    HistoryShipSelection testee;
    testee.setMode(HistoryShipSelection::AllShips);
    testee.setSortOrder(HistoryShipSelection::ByAge);
    testee.buildList(list, t, session);

    // Verify
    a.checkEqual("01. size", list.size(), 2U);
    a.checkEqual("02. name", list.get(0)->name, "5 turns ago");
    a.checkEqual("03. name", list.get(1)->name, "Ship #1");
}
