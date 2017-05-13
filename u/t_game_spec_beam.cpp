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

