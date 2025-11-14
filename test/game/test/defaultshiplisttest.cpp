/**
  *  \file test/game/test/defaultshiplisttest.cpp
  *  \brief Test for game::test::DefaultShipList
  */

#include "game/test/defaultshiplist.hpp"
#include "afl/test/testrunner.hpp"

/** Quick sanity test. */
AFL_TEST("game.test.DefaultShipList", a)
{
    game::spec::ShipList sl;
    game::test::initDefaultShipList(sl);
    afl::base::Ref<game::config::HostConfiguration> config = game::config::HostConfiguration::create();

    a.checkEqual("01. hull 105",        sl.hulls().get(105)->getName(sl.componentNamer()), "MERLIN CLASS ALCHEMY SHIP");
    a.checkEqual("02. engine 9",        sl.engines().get(9)->getName(sl.componentNamer()), "Transwarp Drive");
    a.checkEqual("03. beam 10",         sl.beams().get(10)->getName(sl.componentNamer()), "Heavy Phaser");
    a.checkEqual("04. launcher 10",     sl.launchers().get(10)->getName(sl.componentNamer()), "Mark 8 Photon");
    a.checkEqual("05. hull assignment", sl.hullAssignments().getHullFromIndex(*config, 1, 4), 16);
}
