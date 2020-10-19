/**
  *  \file u/t_game_spec_torpedolauncher.cpp
  *  \brief Test for game::spec::TorpedoLauncher
  */

#include "game/spec/torpedolauncher.hpp"

#include "t_game_spec.hpp"

/** Simple test. */
void
TestGameSpecTorpedoLauncher::testIt()
{
    class TestComponentNameProvider : public game::spec::ComponentNameProvider {
     public:
        virtual String_t getName(Type type, int /*index*/, const String_t& name) const
            {
                TS_ASSERT_EQUALS(type, Torpedo);
                return name;
            }
        virtual String_t getShortName(Type type, int /*index*/, const String_t& /*name*/, const String_t& shortName) const
            {
                TS_ASSERT_EQUALS(type, Torpedo);
                return shortName;
            }
    };

    // Check Id
    game::spec::TorpedoLauncher testee(4);
    TS_ASSERT_EQUALS(testee.getId(), 4);

    // Check type using the ComponentNameProvider
    testee.setName("torpedo name");
    testee.setShortName("trpd nm");

    TestComponentNameProvider cnp;
    TS_ASSERT_EQUALS(testee.getName(cnp), "torpedo name");
    TS_ASSERT_EQUALS(testee.getShortName(cnp), "trpd nm");

    // Check cost
    testee.cost().set(game::spec::Cost::Tritanium, 3);
    TS_ASSERT_EQUALS(testee.cost().get(game::spec::Cost::Tritanium), 3);
    TS_ASSERT_EQUALS(const_cast<const game::spec::TorpedoLauncher&>(testee).cost().get(game::spec::Cost::Tritanium), 3);
}

void
TestGameSpecTorpedoLauncher::testDerivedInformation()
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
    TS_ASSERT_EQUALS(t.getMinefieldCost(1, 1000, false, config, c), true);
    TS_ASSERT_EQUALS(c.toPHostString(), "T15 D15 M15 $546");

    TS_ASSERT_EQUALS(t.getMinefieldCost(9, 1000, false, config, c), true);
    TS_ASSERT_EQUALS(c.toPHostString(), "T3 D3 M3 $136");

    // Host
    {
        game::HostVersion h(game::HostVersion::Host, MKVERSION(3, 22, 40));
        TS_ASSERT_EQUALS(t.getRechargeTime(1, h, config), 32);
        TS_ASSERT_EQUALS(t.getHitOdds(1, h, config), 66);
    }

    // PHost
    {
        game::HostVersion h(game::HostVersion::PHost, MKVERSION(4, 0, 5));
        TS_ASSERT_EQUALS(t.getRechargeTime(1, h, config), 44);
        TS_ASSERT_EQUALS(t.getHitOdds(1, h, config), 65);
    }

}

