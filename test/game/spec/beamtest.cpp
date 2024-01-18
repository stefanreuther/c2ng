/**
  *  \file test/game/spec/beamtest.cpp
  *  \brief Test for game::spec::Beam
  */

#include "game/spec/beam.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("game.spec.Beam:basics", a)
{
    class TestComponentNameProvider : public game::spec::ComponentNameProvider {
     public:
        TestComponentNameProvider(afl::test::Assert a)
            : m_assert(a)
            { }
        virtual String_t getName(Type type, int /*index*/, const String_t& name) const
            {
                m_assert.checkEqual("getName", type, Beam);
                return name;
            }
        virtual String_t getShortName(Type type, int /*index*/, const String_t& /*name*/, const String_t& shortName) const
            {
                m_assert.checkEqual("getShortName", type, Beam);
                return shortName;
            }
     private:
        afl::test::Assert m_assert;
    };

    // Check Id
    game::spec::Beam testee(4);
    a.checkEqual("11. getId", testee.getId(), 4);

    // Check type using the ComponentNameProvider
    testee.setName("beam name");
    testee.setShortName("bm nm");

    TestComponentNameProvider cnp(a);
    a.checkEqual("21. getName", testee.getName(cnp), "beam name");
    a.checkEqual("22. getShortName", testee.getShortName(cnp), "bm nm");
}

/** Test derived information. */
AFL_TEST("game.spec.Beam:derived-information", a)
{
    // Heavy Phaser
    game::spec::Beam b(10);
    b.setKillPower(35);
    b.setDamagePower(45);

    // Host configuration using defaults
    game::config::HostConfiguration config;

    // Independant of host version
    a.checkEqual("01. getNumMinesSwept", b.getNumMinesSwept(1, true, config), 300);
    a.checkEqual("02. getNumMinesSwept", b.getNumMinesSwept(1, false, config), 400);

    // Host
    {
        game::HostVersion h(game::HostVersion::Host, MKVERSION(3, 22, 40));
        a.checkEqual("11. getRechargeTime", b.getRechargeTime(1, h, config), 100);
        a.checkEqual("12. getHitOdds", b.getHitOdds(1, h, config), 100);
    }

    // PHost
    {
        game::HostVersion h(game::HostVersion::PHost, MKVERSION(4, 0, 5));
        a.checkEqual("21. getRechargeTime", b.getRechargeTime(1, h, config), 150);
        a.checkEqual("22. getHitOdds", b.getHitOdds(1, h, config), 100);
    }
}
