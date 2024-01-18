/**
  *  \file test/game/spec/basecomponentvectortest.cpp
  *  \brief Test for game::spec::BaseComponentVector
  */

#include "game/spec/basecomponentvector.hpp"

#include "afl/test/testrunner.hpp"
#include "game/spec/component.hpp"
#include "game/spec/nullcomponentnameprovider.hpp"
#include <memory>

namespace {
    // setNew is protected; publish it for testing
    class PublicComponentVector : public game::spec::BaseComponentVector {
     public:
        using BaseComponentVector::setNew;
    };
}

/** Simple test. */
AFL_TEST("game.spec.BaseComponentVector:basics", a)
{
    PublicComponentVector testee;
    game::spec::NullComponentNameProvider cnp;

    // Initial state
    a.checkEqual("01. size", testee.size(), 0);
    a.checkNull("02. findNext", testee.findNext(0));
    a.checkNull("03. findNext", testee.findNext(1000));

    int n;
    a.check("11. shortNames", !testee.shortNames(cnp).getFirstKey(n));
    a.check("12. names", !testee.names(cnp).getFirstKey(n));

    // Create some elements
    {
        std::auto_ptr<game::spec::Component> a(new game::spec::Component(cnp.Hull, 2));
        a->setName("a");
        testee.setNew(2, a.release());
    }
    {
        std::auto_ptr<game::spec::Component> b(new game::spec::Component(cnp.Hull, 8));
        b->setName("b");
        testee.setNew(8, b.release());
    }

    // Verify
    a.checkEqual("21. size", testee.size(), 8);
    {
        game::spec::Component* p = testee.findNext(0);
        a.checkNonNull("22. findNext", p);
        a.checkEqual("23. getId", p->getId(), 2);

        p = testee.findNext(2);
        a.checkNonNull("31. findNext", p);
        a.checkEqual("32. getId", p->getId(), 8);

        p = testee.findNext(8);
        a.checkNull("41. findNext", p);
    }

    a.checkNull("51. get", testee.get(0));
    a.checkNull("52. get", testee.get(1));
    a.checkNonNull("53. get", testee.get(2));
    a.checkNull("54. get", testee.get(3));

    a.checkNull("61. get", testee.get(7));
    a.checkNonNull("62. get", testee.get(8));
    a.checkNull("63. get", testee.get(9));

    a.checkEqual("71. names", testee.names(cnp)(-1), "");
    a.checkEqual("72. names", testee.names(cnp)(0), "");
    a.checkEqual("73. names", testee.names(cnp)(2), "a");
    a.checkEqual("74. names", testee.names(cnp)(5), "");
    a.checkEqual("75. names", testee.names(cnp)(8), "b");
    a.checkEqual("76. names", testee.names(cnp)(999), "");

    n = 0;
    a.check("81. shortNames.getFirstKey", testee.shortNames(cnp).getFirstKey(n));
    a.checkEqual("82. result", n, 2);
    a.check("83. shortNames.getNextKey", testee.shortNames(cnp).getNextKey(n));
    a.checkEqual("84. result", n, 8);
    a.check("85. shortNames.getNextKey", !testee.shortNames(cnp).getNextKey(n));

    n = 0;
    a.check("91. names.getFirstKey", testee.names(cnp).getFirstKey(n));
    a.checkEqual("92. result", n, 2);

    // Clear restores initial state
    testee.clear();
    a.checkEqual("101. size", testee.size(), 0);
    a.checkNull("102. findNext", testee.findNext(0));
    a.checkNull("103. findNext", testee.findNext(1000));

    a.check("111. shortNames", !testee.shortNames(cnp).getFirstKey(n));
    a.check("112. names", !testee.names(cnp).getFirstKey(n));

    a.checkEqual("121. names", testee.names(cnp)(8), "");
}

/** Test out-of-range behaviour. */
AFL_TEST("game.spec.BaseComponentVector:out-of-range", a)
{
    PublicComponentVector testee;

    testee.setNew(0, new game::spec::Component(game::spec::ComponentNameProvider::Beam, 0));
    testee.setNew(-1, new game::spec::Component(game::spec::ComponentNameProvider::Beam, 0));
    testee.setNew(-99, new game::spec::Component(game::spec::ComponentNameProvider::Beam, 0));

    a.checkEqual("01. size", testee.size(), 0);
    a.checkNull("02. findNext", testee.findNext(0));
    a.checkNull("03. findNext", testee.findNext(1000));
    a.checkNull("04. get", testee.get(-99));
}
