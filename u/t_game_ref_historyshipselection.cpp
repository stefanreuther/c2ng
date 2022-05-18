/**
  *  \file u/t_game_ref_historyshipselection.cpp
  *  \brief Test for game::ref::HistoryShipSelection
  */

#include "game/ref/historyshipselection.hpp"

#include "t_game_ref.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
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
        // With source=owner, combinedCheck1 would discard the ships as bogons,
        // because they should have got a proper full record (addCurrentShipData).
        game::PlayerSet_t source(owner+1);

        game::map::Ship& sh = *u.ships().create(id);
        sh.addShipXYData(pos, owner, 100, source);
        sh.internalCheck();
        sh.combinedCheck1(u, source, TURN_NR);
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
void
TestGameRefHistoryShipSelection::testBasic()
{
    afl::string::NullTranslator tx;
    HistoryShipSelection t;

    TS_ASSERT_EQUALS(t.getMode(), HistoryShipSelection::AllShips);
    TS_ASSERT_EQUALS(t.getSortOrder(), HistoryShipSelection::ById);

    t.setMode(HistoryShipSelection::OwnShips);
    t.setSortOrder(HistoryShipSelection::ByHull);
    t.setPosition(game::map::Point(1000, 2000));
    TS_ASSERT_EQUALS(t.getMode(), HistoryShipSelection::OwnShips);
    TS_ASSERT_EQUALS(t.getSortOrder(), HistoryShipSelection::ByHull);

    TS_ASSERT_EQUALS(t.getModeName(tx), t.getModeName(HistoryShipSelection::OwnShips, tx));
    TS_ASSERT_EQUALS(t.getSortOrderName(tx), t.getSortOrderName(HistoryShipSelection::ByHull, tx));

    TS_ASSERT_EQUALS(t.getModeName(HistoryShipSelection::LocalShips, tx), "Ships near (1000,2000)");
    TS_ASSERT_EQUALS(t.getSortOrderName(HistoryShipSelection::ByName, tx), "Sort by Name");

    // All modes need to be printable
    for (size_t i = 0; i <= HistoryShipSelection::ModeMax; ++i) {
        TS_ASSERT_DIFFERS(t.getModeName(HistoryShipSelection::Mode(i), tx), "");
    }

    // All sort orders need to be printable
    for (size_t i = 0; i <= HistoryShipSelection::SortMax; ++i) {
        TS_ASSERT_DIFFERS(t.getSortOrderName(HistoryShipSelection::SortOrder(i), tx), "");
    }
}

/** Test operations on mode sets. */
void
TestGameRefHistoryShipSelection::testModeSet()
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
        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u3, mapConfig, noTeams)), "ao");
        TS_ASSERT_EQUALS(t.getInitialMode(u3, mapConfig, noTeams), HistoryShipSelection::AllShips);

        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u4, mapConfig, noTeams)), "af");
        TS_ASSERT_EQUALS(t.getInitialMode(u4, mapConfig, noTeams), HistoryShipSelection::AllShips);

        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u5, mapConfig, noTeams)), "af");
        TS_ASSERT_EQUALS(t.getInitialMode(u5, mapConfig, noTeams), HistoryShipSelection::AllShips);

        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u35, mapConfig, noTeams)), "afo");
        TS_ASSERT_EQUALS(t.getInitialMode(u35, mapConfig, noTeams), HistoryShipSelection::AllShips);

        // With teams
        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u3, mapConfig, hasTeams)), "ato");
        TS_ASSERT_EQUALS(t.getInitialMode(u3, mapConfig, hasTeams), HistoryShipSelection::AllShips);

        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u4, mapConfig, hasTeams)), "aft");
        TS_ASSERT_EQUALS(t.getInitialMode(u4, mapConfig, hasTeams), HistoryShipSelection::AllShips);

        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u5, mapConfig, hasTeams)), "afe");
        TS_ASSERT_EQUALS(t.getInitialMode(u5, mapConfig, hasTeams), HistoryShipSelection::AllShips);

        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u35, mapConfig, hasTeams)), "afteo");
        TS_ASSERT_EQUALS(t.getInitialMode(u35, mapConfig, hasTeams), HistoryShipSelection::AllShips);
    }

    // Verify all combinations against HistoryShipSelection with exact position
    {
        HistoryShipSelection t;
        t.setPosition(Point(1000, 1000));

        // No teams
        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u3, mapConfig, noTeams)), "alxo");
        TS_ASSERT_EQUALS(t.getInitialMode(u3, mapConfig, noTeams), HistoryShipSelection::LocalShips);

        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u4, mapConfig, noTeams)), "alxf");
        TS_ASSERT_EQUALS(t.getInitialMode(u4, mapConfig, noTeams), HistoryShipSelection::LocalShips);

        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u5, mapConfig, noTeams)), "alxf");
        TS_ASSERT_EQUALS(t.getInitialMode(u5, mapConfig, noTeams), HistoryShipSelection::LocalShips);

        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u35, mapConfig, noTeams)), "alxfo");
        TS_ASSERT_EQUALS(t.getInitialMode(u35, mapConfig, noTeams), HistoryShipSelection::LocalShips);

        // With teams
        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u3, mapConfig, hasTeams)), "alxto");
        TS_ASSERT_EQUALS(t.getInitialMode(u3, mapConfig, hasTeams), HistoryShipSelection::LocalShips);

        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u4, mapConfig, hasTeams)), "alxft");
        TS_ASSERT_EQUALS(t.getInitialMode(u4, mapConfig, hasTeams), HistoryShipSelection::LocalShips);

        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u5, mapConfig, hasTeams)), "alxfe");
        TS_ASSERT_EQUALS(t.getInitialMode(u5, mapConfig, hasTeams), HistoryShipSelection::LocalShips);

        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u35, mapConfig, hasTeams)), "alxfteo");
        TS_ASSERT_EQUALS(t.getInitialMode(u35, mapConfig, hasTeams), HistoryShipSelection::LocalShips);
    }

    // Verify all combinations against HistoryShipSelection with a close position
    {
        HistoryShipSelection t;
        t.setPosition(Point(1000, 1001));

        // No teams
        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u3, mapConfig, noTeams)), "alo");
        TS_ASSERT_EQUALS(t.getInitialMode(u3, mapConfig, noTeams), HistoryShipSelection::LocalShips);

        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u4, mapConfig, noTeams)), "alf");
        TS_ASSERT_EQUALS(t.getInitialMode(u4, mapConfig, noTeams), HistoryShipSelection::LocalShips);

        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u5, mapConfig, noTeams)), "alf");
        TS_ASSERT_EQUALS(t.getInitialMode(u5, mapConfig, noTeams), HistoryShipSelection::LocalShips);

        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u35, mapConfig, noTeams)), "alfo");
        TS_ASSERT_EQUALS(t.getInitialMode(u35, mapConfig, noTeams), HistoryShipSelection::LocalShips);

        // With teams
        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u3, mapConfig, hasTeams)), "alto");
        TS_ASSERT_EQUALS(t.getInitialMode(u3, mapConfig, hasTeams), HistoryShipSelection::LocalShips);

        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u4, mapConfig, hasTeams)), "alft");
        TS_ASSERT_EQUALS(t.getInitialMode(u4, mapConfig, hasTeams), HistoryShipSelection::LocalShips);

        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u5, mapConfig, hasTeams)), "alfe");
        TS_ASSERT_EQUALS(t.getInitialMode(u5, mapConfig, hasTeams), HistoryShipSelection::LocalShips);

        TS_ASSERT_EQUALS(toString(t.getAvailableModes(u35, mapConfig, hasTeams)), "alfteo");
        TS_ASSERT_EQUALS(t.getInitialMode(u35, mapConfig, hasTeams), HistoryShipSelection::LocalShips);
    }
}

/** Test buildList(). */
void
TestGameRefHistoryShipSelection::testBuildList()
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
    session.setRoot(new game::test::Root(game::HostVersion()));
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
    TS_ASSERT_EQUALS(list.size(), 7U);
    TS_ASSERT_EQUALS(list.get(0)->name, "Player 3");
    TS_ASSERT_EQUALS(list.get(1)->name, "Ship #1");
    TS_ASSERT_EQUALS(list.get(2)->name, "Ship #2");
    TS_ASSERT_EQUALS(list.get(3)->name, "Ship #5");
    TS_ASSERT_EQUALS(list.get(4)->name, "Player 4");
    TS_ASSERT_EQUALS(list.get(5)->name, "Ship #3");
    TS_ASSERT_EQUALS(list.get(6)->name, "Ship #4");
    TS_ASSERT_EQUALS(list.get(6)->turnNumber, TURN_NR);
    TS_ASSERT_EQUALS(list.getReferenceTurn(), TURN_NR);
}

/** Test buildList(), with history. */
void
TestGameRefHistoryShipSelection::testBuildListHist()
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
    session.setRoot(new game::test::Root(game::HostVersion()));
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
    TS_ASSERT_EQUALS(list.size(), 4U);
    TS_ASSERT_EQUALS(list.get(0)->name, "previous turn");
    TS_ASSERT_EQUALS(list.get(1)->name, "Ship #2");
    TS_ASSERT_EQUALS(list.get(2)->name, "2 turns ago");
    TS_ASSERT_EQUALS(list.get(3)->name, "Ship #1");

    // Own ships
    testee.setMode(HistoryShipSelection::OwnShips);
    testee.buildList(list, t, session);
    TS_ASSERT_EQUALS(list.size(), 2U);
    TS_ASSERT_EQUALS(list.get(0)->name, "current turn");
    TS_ASSERT_EQUALS(list.get(1)->name, "Ship #1");

    // Team ships
    testee.setMode(HistoryShipSelection::TeamShips);
    testee.buildList(list, t, session);
    TS_ASSERT_EQUALS(list.size(), 3U);
    TS_ASSERT_EQUALS(list.get(0)->name, "current turn");
    TS_ASSERT_EQUALS(list.get(1)->name, "Ship #1");
    TS_ASSERT_EQUALS(list.get(2)->name, "Ship #2");

    // Enemy ships
    testee.setMode(HistoryShipSelection::EnemyShips);
    testee.buildList(list, t, session);
    TS_ASSERT_EQUALS(list.size(), 2U);
    TS_ASSERT_EQUALS(list.get(0)->name, "current turn");
    TS_ASSERT_EQUALS(list.get(1)->name, "Ship #3");

    // Foreign ships
    testee.setMode(HistoryShipSelection::ForeignShips);
    testee.buildList(list, t, session);
    TS_ASSERT_EQUALS(list.size(), 3U);
    TS_ASSERT_EQUALS(list.get(0)->name, "current turn");
    TS_ASSERT_EQUALS(list.get(1)->name, "Ship #2");
    TS_ASSERT_EQUALS(list.get(2)->name, "Ship #3");

    // Exact location: fails!
    testee.setMode(HistoryShipSelection::ExactShips);
    testee.buildList(list, t, session);
    TS_ASSERT_EQUALS(list.size(), 0U);
    TS_ASSERT(!testee.getAvailableModes(t.universe(), session.getGame()->mapConfiguration(), session.getGame()->teamSettings()).contains(HistoryShipSelection::ExactShips));

    // Exact location: succeeds with different location
    testee.setPosition(Point(1000, 1000));
    TS_ASSERT(testee.getAvailableModes(t.universe(), session.getGame()->mapConfiguration(), session.getGame()->teamSettings()).contains(HistoryShipSelection::ExactShips));
    testee.setSortOrder(HistoryShipSelection::ByOwner);
    testee.buildList(list, t, session);
    TS_ASSERT_EQUALS(list.size(), 6U);
    TS_ASSERT_EQUALS(list.get(0)->name, "Player 3");
    TS_ASSERT_EQUALS(list.get(1)->name, "Ship #1");
    TS_ASSERT_EQUALS(list.get(2)->name, "Player 4");
    TS_ASSERT_EQUALS(list.get(3)->name, "Ship #2");
    TS_ASSERT_EQUALS(list.get(4)->name, "Player 5");
    TS_ASSERT_EQUALS(list.get(5)->name, "Ship #3");
}

/** Test buildList(), with ships that ONLY have history. */
void
TestGameRefHistoryShipSelection::testBuildListHist2()
{
    const int ME = 3;

    // Turn/universe with a ship that we saw last time 5 turns ago,
    // but also has a record from 4 turns ago.
    // (This exercises the loop in getShipLastTurn which is easy to get wrong because it goes backwards.)
    game::Turn t;
    game::map::Ship& s1 = *t.universe().ships().create(1);
    s1.setOwner(ME);
    s1.internalCheck();
    s1.combinedCheck1(t.universe(), game::PlayerSet_t(), TURN_NR);
    addShipNonTrack(s1, 4);
    addShipTrack(s1, 5, Point(1000, 1020));
    t.setTurnNumber(TURN_NR);

    // Session
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(new game::test::Root(game::HostVersion()));
    session.setGame(new game::Game());
    session.getGame()->teamSettings().setViewpointPlayer(ME);

    // All ships
    game::ref::HistoryShipList list;
    HistoryShipSelection testee;
    testee.setMode(HistoryShipSelection::AllShips);
    testee.setSortOrder(HistoryShipSelection::ByAge);
    testee.buildList(list, t, session);

    // Verify
    TS_ASSERT_EQUALS(list.size(), 2U);
    TS_ASSERT_EQUALS(list.get(0)->name, "5 turns ago");
    TS_ASSERT_EQUALS(list.get(1)->name, "Ship #1");
}

