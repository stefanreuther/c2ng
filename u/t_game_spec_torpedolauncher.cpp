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

