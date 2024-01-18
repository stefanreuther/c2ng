/**
  *  \file test/game/spec/componenttest.cpp
  *  \brief Test for game::spec::Component
  */

#include "game/spec/component.hpp"

#include "afl/string/format.hpp"
#include "afl/test/testrunner.hpp"
#include "game/spec/componentnameprovider.hpp"

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
AFL_TEST("game.spec.Component:basics", a)
{
    game::spec::Component testee(game::spec::ComponentNameProvider::Torpedo, 3);
    const game::spec::Component& alias = testee;
    a.checkEqual("01. getId", testee.getId(), 3);

    // Mass
    a.checkEqual("11. getMass", testee.getMass(), 1);
    testee.setMass(24);
    a.checkEqual("12. getMass", testee.getMass(), 24);

    // Tech
    a.checkEqual("21. getTechLevel", testee.getTechLevel(), 1);
    testee.setTechLevel(9);
    a.checkEqual("22. getTechLevel", testee.getTechLevel(), 9);

    // Cost
    a.check("31. cost", testee.cost().isZero());
    testee.cost().set(game::spec::Cost::Tritanium, 30);
    a.check("32. cost", !testee.cost().isZero());
    a.check("33. cost", !alias.cost().isZero());

    // Copying
    game::spec::Component copy(alias);
    a.checkEqual("41. getMass", copy.getMass(), testee.getMass());
    a.checkEqual("42. getTechLevel", copy.getTechLevel(), testee.getTechLevel());
    a.checkEqual("43. cost", copy.cost(), testee.cost());
}

/** Test name access. */
AFL_TEST("game.spec.Component:name", a)
{
    game::spec::Component testee(game::spec::ComponentNameProvider::Torpedo, 7);
    a.checkEqual("01. getName", testee.getName(MyComponentNameProvider()), "<N7>");

    testee.setName("Seven");
    a.checkEqual("11. getName", testee.getName(MyComponentNameProvider()), "<N7>Seven");
    a.checkEqual("12. getShortName", testee.getShortName(MyComponentNameProvider()), "<N7>Seven");

    testee.setShortName("Se");
    a.checkEqual("21. getName", testee.getName(MyComponentNameProvider()), "<N7>Seven");
    a.checkEqual("22. getShortName", testee.getShortName(MyComponentNameProvider()), "<S7>Se");
}
