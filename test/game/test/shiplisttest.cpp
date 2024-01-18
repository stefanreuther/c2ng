/**
  *  \file test/game/test/shiplisttest.cpp
  *  \brief Test for game::test::ShipList
  */

#include "game/test/shiplist.hpp"
#include "afl/test/testrunner.hpp"

using game::spec::Cost;

/** Test initStandardBeams().
    Verify that produced values are sane. */
AFL_TEST("game.test.ShipList:initStandardBeams", a)
{
    game::spec::ShipList sl;
    game::test::initStandardBeams(sl);

    const game::spec::Beam* p = sl.beams().get(1);
    a.check("01. beam 1", p);
    a.checkEqual("02. getName",        p->getName(sl.componentNamer()), "Laser");
    a.checkEqual("03. getKillPower",   p->getKillPower(), 10);
    a.checkEqual("04. getDamagePower", p->getDamagePower(), 3);
    a.checkEqual("05. Money",          p->cost().get(Cost::Money), 1);

    p = sl.beams().get(10);
    a.check("11. beam 10", p);
    a.checkEqual("12. getName",        p->getName(sl.componentNamer()), "Heavy Phaser");
    a.checkEqual("13. getKillPower",   p->getKillPower(), 35);
    a.checkEqual("14. getDamagePower", p->getDamagePower(), 45);
    a.checkEqual("15. Money",          p->cost().get(Cost::Money), 54);
}

/** Test initStandardTorpedoes().
    Verify that produced values are sane. */
AFL_TEST("game.test.ShipList:initStandardTorpedoes", a)
{
    game::spec::ShipList sl;
    game::test::initStandardTorpedoes(sl);

    const game::spec::TorpedoLauncher* p = sl.launchers().get(1);
    a.check("01. torp 1", p);
    a.checkEqual("02. getName",        p->getName(sl.componentNamer()), "Mark 1 Photon");
    a.checkEqual("03. getKillPower",   p->getKillPower(), 4);
    a.checkEqual("04. getDamagePower", p->getDamagePower(), 5);
    a.checkEqual("05. Money",          p->cost().get(Cost::Money), 1);

    p = sl.launchers().get(10);
    a.check("11. torp 10", p);
    a.checkEqual("12. getName",        p->getName(sl.componentNamer()), "Mark 8 Photon");
    a.checkEqual("13. getKillPower",   p->getKillPower(), 35);
    a.checkEqual("14. getDamagePower", p->getDamagePower(), 55);
    a.checkEqual("15. Money",          p->cost().get(Cost::Money), 190);
}

/** Test initPListBeams().
    Verify that produced values are sane. */
AFL_TEST("game.test.ShipList:initPListBeams", a)
{
    game::spec::ShipList sl;
    game::test::initPListBeams(sl);

    const game::spec::Beam* p = sl.beams().get(1);
    a.check("01. beam 1", p);
    a.checkEqual("02. getName",        p->getName(sl.componentNamer()), "Laser Cannon");
    a.checkEqual("03. getKillPower",   p->getKillPower(), 1);
    a.checkEqual("04. getDamagePower", p->getDamagePower(), 2);
    a.checkEqual("05. Money",          p->cost().get(Cost::Money), 1);

    p = sl.beams().get(10);
    a.check("11. beam 10", p);
    a.checkEqual("12. getName",        p->getName(sl.componentNamer()), "Multitraf Spiral");
    a.checkEqual("13. getKillPower",   p->getKillPower(), 40);
    a.checkEqual("14. getDamagePower", p->getDamagePower(), 80);
    a.checkEqual("15. Money",          p->cost().get(Cost::Money), 130);
}

/** Test initPListTorpedoes().
    Verify that produced values are sane. */
AFL_TEST("game.test.ShipList:initPListTorpedoes", a)
{
    game::spec::ShipList sl;
    game::test::initPListTorpedoes(sl);

    const game::spec::TorpedoLauncher* p = sl.launchers().get(1);
    a.check("01. torp 1", p);
    a.checkEqual("02. getName",        p->getName(sl.componentNamer()), "Space Rocket");
    a.checkEqual("03. getKillPower",   p->getKillPower(), 3);
    a.checkEqual("04. getDamagePower", p->getDamagePower(), 5);
    a.checkEqual("05. Money",          p->cost().get(Cost::Money), 5);

    p = sl.launchers().get(10);
    a.check("11. torp 10", p);
    a.checkEqual("12. getName",        p->getName(sl.componentNamer()), "Selphyr-Fataro-Dev.");
    a.checkEqual("13. getKillPower",   p->getKillPower(), 40);
    a.checkEqual("14. getDamagePower", p->getDamagePower(), 99);
    a.checkEqual("15. Money",          p->cost().get(Cost::Money), 150);
}
