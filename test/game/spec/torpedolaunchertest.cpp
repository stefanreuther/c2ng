/**
  *  \file test/game/spec/torpedolaunchertest.cpp
  *  \brief Test for game::spec::TorpedoLauncher
  */

#include "game/spec/torpedolauncher.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("game.spec.TorpedoLauncher:basics", a)
{
    class TestComponentNameProvider : public game::spec::ComponentNameProvider {
     public:
        TestComponentNameProvider(afl::test::Assert a)
            : m_assert(a)
            { }
        virtual String_t getName(Type type, int /*index*/, const String_t& name) const
            {
                m_assert.checkEqual("getName", type, Torpedo);
                return name;
            }
        virtual String_t getShortName(Type type, int /*index*/, const String_t& /*name*/, const String_t& shortName) const
            {
                m_assert.checkEqual("getShortName", type, Torpedo);
                return shortName;
            }
     private:
        afl::test::Assert m_assert;
    };

    // Check Id
    game::spec::TorpedoLauncher testee(4);
    a.checkEqual("11. getId", testee.getId(), 4);
    a.checkEqual("12. getFiringRangeBonus", testee.getFiringRangeBonus(), 0);

    // Check type using the ComponentNameProvider
    testee.setName("torpedo name");
    testee.setShortName("trpd nm");
    testee.setFiringRangeBonus(50);

    TestComponentNameProvider cnp(a);
    a.checkEqual("21. getName", testee.getName(cnp), "torpedo name");
    a.checkEqual("22. getShortName", testee.getShortName(cnp), "trpd nm");
    a.checkEqual("23. getFiringRangeBonus", testee.getFiringRangeBonus(), 50);

    // Check cost
    testee.cost().set(game::spec::Cost::Tritanium, 3);
    a.checkEqual("31. cost", testee.cost().get(game::spec::Cost::Tritanium), 3);
    a.checkEqual("32. cost", const_cast<const game::spec::TorpedoLauncher&>(testee).cost().get(game::spec::Cost::Tritanium), 3);
}

AFL_TEST("game.spec.TorpedoLauncher:derived-information", a)
{
    // Mark 6 Photon
    game::spec::TorpedoLauncher t(8);
    t.setKillPower(46);
    t.setDamagePower(80);
    t.torpedoCost() = game::spec::Cost::fromString("35$ 1TDM");

    // Host configuration using defaults
    game::config::HostConfiguration config;

    // Independant of host version
    game::spec::Cost c;
    a.checkEqual("01. getMinefieldCost", t.getMinefieldCost(1, 1000, false, config, c), true);
    a.checkEqual("02. cost", c.toPHostString(), "T15 D15 M15 $546");

    a.checkEqual("11. getMinefieldCost", t.getMinefieldCost(9, 1000, false, config, c), true);
    a.checkEqual("12. cost", c.toPHostString(), "T3 D3 M3 $136");

    // Host
    {
        game::HostVersion h(game::HostVersion::Host, MKVERSION(3, 22, 40));
        a.checkEqual("21. getRechargeTime", t.getRechargeTime(1, h, config), 32);
        a.checkEqual("22. getHitOdds", t.getHitOdds(1, h, config), 66);
    }

    // PHost
    {
        game::HostVersion h(game::HostVersion::PHost, MKVERSION(4, 0, 5));
        a.checkEqual("31. getRechargeTime", t.getRechargeTime(1, h, config), 44);
        a.checkEqual("32. getHitOdds", t.getHitOdds(1, h, config), 65);
    }

    // SRace
    {
        game::HostVersion h(game::HostVersion::SRace, MKVERSION(3, 22, 40));
        a.checkEqual("41. getRechargeTime", t.getRechargeTime(1, h, config), 32);
        a.checkEqual("42. getHitOdds", t.getHitOdds(1, h, config), 66);
    }
}
