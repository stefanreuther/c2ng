/**
  *  \file u/t_game_spec_componentvector.cpp
  *  \brief Test for game::spec::ComponentVector
  */

#include "game/spec/componentvector.hpp"

#include "t_game_spec.hpp"
#include "game/spec/component.hpp"

namespace {
    class MyComponent : public game::spec::Component {
     public:
        MyComponent(int id)
            : Component(game::spec::ComponentNameProvider::Torpedo, id)
            { }
    };

    class MyComponentNameProvider : public game::spec::ComponentNameProvider {
     public:
        virtual String_t getName(Type /*type*/, int /*index*/, const String_t& name) const
            { return name; }
        virtual String_t getShortName(Type /*type*/, int /*index*/, const String_t& name, const String_t& shortName) const
            { return shortName.empty() ? name : shortName; }
    };
}

/** Test creation and access. */
void
TestGameSpecComponentVector::testCreate()
{
    game::spec::ComponentVector<MyComponent> testee;

    // Starts out empty
    TS_ASSERT(testee.get(0) == 0);
    TS_ASSERT(testee.get(1) == 0);
    TS_ASSERT(testee.get(1000) == 0);

    // Creation behaviour
    // - elements <= 0 refuse to be created
    // - create elements 1,2,9 (=sparse), making size (=max index) 9
    TS_ASSERT(testee.create(-1) == 0);
    TS_ASSERT(testee.create(0) == 0);
    TS_ASSERT(testee.create(1) != 0);
    TS_ASSERT(testee.create(2) != 0);
    TS_ASSERT(testee.create(9) != 0);
    TS_ASSERT_EQUALS(testee.size(), 9);

    // Check elements
    TS_ASSERT(testee.get(0) == 0);
    TS_ASSERT(testee.get(1) != 0);
    TS_ASSERT(testee.get(2) != 0);
    TS_ASSERT(testee.get(3) == 0);
    TS_ASSERT(testee.get(8) == 0);
    TS_ASSERT(testee.get(9) != 0);
    TS_ASSERT(testee.get(10) == 0);
}

/** Test name access. */
void
TestGameSpecComponentVector::testName()
{
    game::spec::ComponentVector<MyComponent> testee;

    MyComponent* one = testee.create(1);
    MyComponent* two = testee.create(2);
    MyComponent* six = testee.create(6);
    one->setName("One");
    two->setName("Two");
    six->setName("Six");
    one->setShortName("1");
    two->setShortName("2");
    six->setShortName("6");

    // Individual access
    TS_ASSERT_EQUALS(testee.names(MyComponentNameProvider())(2), "Two");
    TS_ASSERT_EQUALS(testee.names(MyComponentNameProvider())(3), "");
    TS_ASSERT_EQUALS(testee.names(MyComponentNameProvider())(3000), "");
    TS_ASSERT_EQUALS(testee.names(MyComponentNameProvider())(0), "");
    TS_ASSERT_EQUALS(testee.names(MyComponentNameProvider())(-1), "");

    // Higher-order functions
    class Concat : public afl::functional::BinaryFunction<String_t,String_t,String_t> {
     public:
        String_t get(String_t a, String_t b) const
            { return a + "|" + b; }
    };
    TS_ASSERT_EQUALS(testee.names(MyComponentNameProvider()).fold(Concat(), String_t()), "|One|Two|Six");
    TS_ASSERT_EQUALS(testee.shortNames(MyComponentNameProvider()).fold(Concat(), String_t()), "|1|2|6");
}
