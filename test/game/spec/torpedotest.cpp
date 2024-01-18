/**
  *  \file test/game/spec/torpedotest.cpp
  *  \brief Test for game::spec::Torpedo
  */

#include "game/spec/torpedo.hpp"

#include "afl/test/testrunner.hpp"
#include "game/spec/torpedolauncher.hpp"

/** Simple test. */
AFL_TEST("game.spec.Torpedo:basics", a)
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

    // Make a torpedo launcher
    game::spec::TorpedoLauncher launcher(4);
    launcher.cost().set(game::spec::Cost::Molybdenum, 10);
    launcher.torpedoCost().set(game::spec::Cost::Molybdenum, 3);
    launcher.setMass(15);
    launcher.setName("torpedo name");
    launcher.setShortName("trpd nm");

    // Build the torpedo
    game::spec::Torpedo testee(launcher);
    a.checkEqual("11. getId", testee.getId(), 4);
    a.checkEqual("12. getMass", testee.getMass(), 1);
    a.checkEqual("13. cost", testee.cost().get(game::spec::Cost::Molybdenum), 3);

    // Check type using the ComponentNameProvider
    TestComponentNameProvider cnp(a);
    a.checkEqual("21. getName", testee.getName(cnp), "torpedo name");
    a.checkEqual("22. getShortName", testee.getShortName(cnp), "trpd nm");
}
