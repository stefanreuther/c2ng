/**
  *  \file u/t_game_spec_torpedo.cpp
  *  \brief Test for game::spec::Torpedo
  */

#include "game/spec/torpedo.hpp"

#include "t_game_spec.hpp"
#include "game/spec/torpedolauncher.hpp"

/** Simple test. */
void
TestGameSpecTorpedo::testIt()
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

    // Make a torpedo launcher
    game::spec::TorpedoLauncher launcher(4);
    launcher.cost().set(game::spec::Cost::Molybdenum, 10);
    launcher.torpedoCost().set(game::spec::Cost::Molybdenum, 3);
    launcher.setMass(15);
    launcher.setName("torpedo name");
    launcher.setShortName("trpd nm");

    // Build the torpedo
    game::spec::Torpedo testee(launcher);
    TS_ASSERT_EQUALS(testee.getId(), 4);
    TS_ASSERT_EQUALS(testee.getMass(), 1);
    TS_ASSERT_EQUALS(testee.cost().get(game::spec::Cost::Molybdenum), 3);

    // Check type using the ComponentNameProvider
    TestComponentNameProvider cnp;
    TS_ASSERT_EQUALS(testee.getName(cnp), "torpedo name");
    TS_ASSERT_EQUALS(testee.getShortName(cnp), "trpd nm");
}
