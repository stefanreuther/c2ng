/**
  *  \file u/t_game_test_shiplist.cpp
  *  \brief Test for game::test::ShipList
  */

#include "game/test/shiplist.hpp"

#include "t_game_test.hpp"

using game::spec::Cost;

/** Test initStandardBeams().
    Verify that produced values are sane. */
void
TestGameTestShipList::testStandardBeams()
{
    game::spec::ShipList sl;
    game::test::initStandardBeams(sl);

    const game::spec::Beam* p = sl.beams().get(1);
    TS_ASSERT(p);
    TS_ASSERT_EQUALS(p->getName(sl.componentNamer()), "Laser");
    TS_ASSERT_EQUALS(p->getKillPower(), 10);
    TS_ASSERT_EQUALS(p->getDamagePower(), 3);
    TS_ASSERT_EQUALS(p->cost().get(Cost::Money), 1);

    p = sl.beams().get(10);
    TS_ASSERT(p);
    TS_ASSERT_EQUALS(p->getName(sl.componentNamer()), "Heavy Phaser");
    TS_ASSERT_EQUALS(p->getKillPower(), 35);
    TS_ASSERT_EQUALS(p->getDamagePower(), 45);
    TS_ASSERT_EQUALS(p->cost().get(Cost::Money), 54);
}

/** Test initStandardTorpedoes().
    Verify that produced values are sane. */
void
TestGameTestShipList::testStandardTorpedoes()
{
    game::spec::ShipList sl;
    game::test::initStandardTorpedoes(sl);

    const game::spec::TorpedoLauncher* p = sl.launchers().get(1);
    TS_ASSERT(p);
    TS_ASSERT_EQUALS(p->getName(sl.componentNamer()), "Mark 1 Photon");
    TS_ASSERT_EQUALS(p->getKillPower(), 4);
    TS_ASSERT_EQUALS(p->getDamagePower(), 5);
    TS_ASSERT_EQUALS(p->cost().get(Cost::Money), 1);

    p = sl.launchers().get(10);
    TS_ASSERT(p);
    TS_ASSERT_EQUALS(p->getName(sl.componentNamer()), "Mark 8 Photon");
    TS_ASSERT_EQUALS(p->getKillPower(), 35);
    TS_ASSERT_EQUALS(p->getDamagePower(), 55);
    TS_ASSERT_EQUALS(p->cost().get(Cost::Money), 190);
}

/** Test initPListBeams().
    Verify that produced values are sane. */
void
TestGameTestShipList::testPListBeams()
{
    game::spec::ShipList sl;
    game::test::initPListBeams(sl);

    const game::spec::Beam* p = sl.beams().get(1);
    TS_ASSERT(p);
    TS_ASSERT_EQUALS(p->getName(sl.componentNamer()), "Laser Cannon");
    TS_ASSERT_EQUALS(p->getKillPower(), 1);
    TS_ASSERT_EQUALS(p->getDamagePower(), 2);
    TS_ASSERT_EQUALS(p->cost().get(Cost::Money), 1);

    p = sl.beams().get(10);
    TS_ASSERT(p);
    TS_ASSERT_EQUALS(p->getName(sl.componentNamer()), "Multitraf Spiral");
    TS_ASSERT_EQUALS(p->getKillPower(), 40);
    TS_ASSERT_EQUALS(p->getDamagePower(), 80);
    TS_ASSERT_EQUALS(p->cost().get(Cost::Money), 130);
}

/** Test initPListTorpedoes().
    Verify that produced values are sane. */
void
TestGameTestShipList::testPListTorpedoes()
{
    game::spec::ShipList sl;
    game::test::initPListTorpedoes(sl);

    const game::spec::TorpedoLauncher* p = sl.launchers().get(1);
    TS_ASSERT(p);
    TS_ASSERT_EQUALS(p->getName(sl.componentNamer()), "Space Rocket");
    TS_ASSERT_EQUALS(p->getKillPower(), 3);
    TS_ASSERT_EQUALS(p->getDamagePower(), 5);
    TS_ASSERT_EQUALS(p->cost().get(Cost::Money), 5);

    p = sl.launchers().get(10);
    TS_ASSERT(p);
    TS_ASSERT_EQUALS(p->getName(sl.componentNamer()), "Selphyr-Fataro-Dev.");
    TS_ASSERT_EQUALS(p->getKillPower(), 40);
    TS_ASSERT_EQUALS(p->getDamagePower(), 99);
    TS_ASSERT_EQUALS(p->cost().get(Cost::Money), 150);
}

