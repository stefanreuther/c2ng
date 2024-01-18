/**
  *  \file test/game/spec/componentvectortest.cpp
  *  \brief Test for game::spec::ComponentVector
  */

#include "game/spec/componentvector.hpp"

#include "afl/test/testrunner.hpp"
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
AFL_TEST("game.spec.ComponentVector:create", a)
{
    game::spec::ComponentVector<MyComponent> testee;

    // Starts out empty
    a.checkNull("01. get", testee.get(0));
    a.checkNull("02. get", testee.get(1));
    a.checkNull("03. get", testee.get(1000));

    // Creation behaviour
    // - elements <= 0 refuse to be created
    // - create elements 1,2,9 (=sparse), making size (=max index) 9
    a.checkNull   ("11. create", testee.create(-1));
    a.checkNull   ("12. create", testee.create(0));
    a.checkNonNull("13. create", testee.create(1));
    a.checkNonNull("14. create", testee.create(2));
    a.checkNonNull("15. create", testee.create(9));
    a.checkEqual  ("16. size", testee.size(), 9);

    // Check elements
    a.checkNull   ("21. get", testee.get(0));
    a.checkNonNull("22. get", testee.get(1));
    a.checkNonNull("23. get", testee.get(2));
    a.checkNull   ("24. get", testee.get(3));
    a.checkNull   ("25. get", testee.get(8));
    a.checkNonNull("26. get", testee.get(9));
    a.checkNull   ("27. get", testee.get(10));

    // Test iteration
    MyComponent* p = testee.findNext(0);
    a.checkNonNull("31. findNext", p);
    a.checkEqual("32. getId", p->getId(), 1);

    p = testee.findNext(1);
    a.checkNonNull("41. findNext", p);
    a.checkEqual("42. getId", p->getId(), 2);

    p = testee.findNext(2);
    a.checkNonNull("51. findNext", p);
    a.checkEqual("52. getId", p->getId(), 9);

    p = testee.findNext(9);
    a.checkNull("61. findNext", p);
}

/** Test name access. */
AFL_TEST("game.spec.ComponentVector:name", a)
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
    a.checkEqual("01. names", testee.names(MyComponentNameProvider())(2), "Two");
    a.checkEqual("02. names", testee.names(MyComponentNameProvider())(3), "");
    a.checkEqual("03. names", testee.names(MyComponentNameProvider())(3000), "");
    a.checkEqual("04. names", testee.names(MyComponentNameProvider())(0), "");
    a.checkEqual("05. names", testee.names(MyComponentNameProvider())(-1), "");

    // Higher-order functions
    class Concat : public afl::functional::BinaryFunction<String_t,String_t,String_t> {
     public:
        String_t get(String_t a, String_t b) const
            { return a + "|" + b; }
    };
    a.checkEqual("11. names", testee.names(MyComponentNameProvider()).fold(Concat(), String_t()), "|One|Two|Six");
    a.checkEqual("12. shortNames", testee.shortNames(MyComponentNameProvider()).fold(Concat(), String_t()), "|1|2|6");
}
