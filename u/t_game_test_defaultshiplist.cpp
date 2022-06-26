/**
  *  \file u/t_game_test_defaultshiplist.cpp
  *  \brief Test for game::test::DefaultShipList
  */

#include "game/test/defaultshiplist.hpp"

#include "t_game_test.hpp"

/** Quick sanity test. */
void
TestGameTestDefaultShipList::testIt()
{
    game::spec::ShipList sl;
    game::test::initDefaultShipList(sl);

    TS_ASSERT_EQUALS(sl.hulls().get(105)->getName(sl.componentNamer()), "MERLIN CLASS ALCHEMY SHIP");
    TS_ASSERT_EQUALS(sl.engines().get(9)->getName(sl.componentNamer()), "Transwarp Drive");
    TS_ASSERT_EQUALS(sl.beams().get(10)->getName(sl.componentNamer()), "Heavy Phaser");
    TS_ASSERT_EQUALS(sl.launchers().get(10)->getName(sl.componentNamer()), "Mark 8 Photon");
    TS_ASSERT_EQUALS(sl.hullAssignments().getHullFromIndex(game::config::HostConfiguration(), 1, 4), 16);
}

