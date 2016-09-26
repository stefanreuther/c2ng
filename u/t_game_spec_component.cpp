/**
  *  \file u/t_game_spec_component.cpp
  *  \brief Test for game::spec::Component
  */

#include "game/spec/component.hpp"

#include "t_game_spec.hpp"
#include "game/spec/componentnameprovider.hpp"
#include "afl/string/format.hpp"

namespace {
    class MyComponentNameProvider : public game::spec::ComponentNameProvider {
     public:
        virtual String_t getName(Type /*type*/, int index, const String_t& name) const
            { return afl::string::Format("<N%d>%s", index, name); }
        virtual String_t getShortName(Type type, int index, const String_t& name, const String_t& shortName) const
            {
                if (shortName.empty()) {
                    return getName(type, index, name);
                } else {
                    return afl::string::Format("<S%d>%s", index, shortName);
                }
            }
    };
}

/** Test data setters/getters. */
void
TestGameSpecComponent::testData()
{
    game::spec::Component testee(game::spec::ComponentNameProvider::Torpedo, 3);
    const game::spec::Component& alias = testee;
    TS_ASSERT_EQUALS(testee.getId(), 3);

    // Mass
    TS_ASSERT_EQUALS(testee.getMass(), 1);
    testee.setMass(24);
    TS_ASSERT_EQUALS(testee.getMass(), 24);

    // Tech
    TS_ASSERT_EQUALS(testee.getTechLevel(), 1);
    testee.setTechLevel(9);
    TS_ASSERT_EQUALS(testee.getTechLevel(), 9);

    // Cost
    TS_ASSERT(testee.cost().isZero());
    testee.cost().set(game::spec::Cost::Tritanium, 30);
    TS_ASSERT(!testee.cost().isZero());
    TS_ASSERT(!alias.cost().isZero());

    // Copying
    game::spec::Component copy(alias);
    TS_ASSERT_EQUALS(copy.getMass(), testee.getMass());
    TS_ASSERT_EQUALS(copy.getTechLevel(), testee.getTechLevel());
    TS_ASSERT_EQUALS(copy.cost(), testee.cost());
}

/** Test name access. */
void
TestGameSpecComponent::testName()
{
    game::spec::Component testee(game::spec::ComponentNameProvider::Torpedo, 7);
    TS_ASSERT_EQUALS(testee.getName(MyComponentNameProvider()), "<N7>");

    testee.setName("Seven");
    TS_ASSERT_EQUALS(testee.getName(MyComponentNameProvider()), "<N7>Seven");
    TS_ASSERT_EQUALS(testee.getShortName(MyComponentNameProvider()), "<N7>Seven");

    testee.setShortName("Se");
    TS_ASSERT_EQUALS(testee.getName(MyComponentNameProvider()), "<N7>Seven");
    TS_ASSERT_EQUALS(testee.getShortName(MyComponentNameProvider()), "<S7>Se");
}
