/**
  *  \file u/t_game_spec_beam.cpp
  *  \brief Test for game::spec::Beam
  */

#include "game/spec/beam.hpp"

#include "t_game_spec.hpp"

/** Simple test. */
void
TestGameSpecBeam::testIt()
{
    class TestComponentNameProvider : public game::spec::ComponentNameProvider {
     public:
        virtual String_t getName(Type type, int /*index*/, const String_t& name) const
            {
                TS_ASSERT_EQUALS(type, Beam);
                return name;
            }
        virtual String_t getShortName(Type type, int /*index*/, const String_t& /*name*/, const String_t& shortName) const
            {
                TS_ASSERT_EQUALS(type, Beam);
                return shortName;
            }
    };

    // Check Id
    game::spec::Beam testee(4);
    TS_ASSERT_EQUALS(testee.getId(), 4);

    // Check type using the ComponentNameProvider
    testee.setName("beam name");
    testee.setShortName("bm nm");

    TestComponentNameProvider cnp;
    TS_ASSERT_EQUALS(testee.getName(cnp), "beam name");
    TS_ASSERT_EQUALS(testee.getShortName(cnp), "bm nm");
}

/** Test derived information. */
void
TestGameSpecBeam::testDerivedInformation()
{
    // Heavy Phaser
    game::spec::Beam b(10);
    b.setKillPower(35);
    b.setDamagePower(45);

    // Host configuration using defaults
    game::config::HostConfiguration config;

    // Independant of host version
    TS_ASSERT_EQUALS(b.getNumMinesSwept(1, true, config), 300);
    TS_ASSERT_EQUALS(b.getNumMinesSwept(1, false, config), 400);

    // Host
    {
        game::HostVersion h(game::HostVersion::Host, MKVERSION(3, 22, 40));
        TS_ASSERT_EQUALS(b.getRechargeTime(1, h, config), 100);
        TS_ASSERT_EQUALS(b.getHitOdds(1, h, config), 100);
    }

    // PHost
    {
        game::HostVersion h(game::HostVersion::PHost, MKVERSION(4, 0, 5));
        TS_ASSERT_EQUALS(b.getRechargeTime(1, h, config), 150);
        TS_ASSERT_EQUALS(b.getHitOdds(1, h, config), 100);
    }
}

