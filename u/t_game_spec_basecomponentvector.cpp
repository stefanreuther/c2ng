/**
  *  \file u/t_game_spec_basecomponentvector.cpp
  *  \brief Test for game::spec::BaseComponentVector
  */

#include <memory>
#include "game/spec/basecomponentvector.hpp"

#include "t_game_spec.hpp"
#include "game/spec/nullcomponentnameprovider.hpp"

namespace {
    // setNew is protected; publish it for testing
    class PublicComponentVector : public game::spec::BaseComponentVector {
     public:
        using BaseComponentVector::setNew;
    };
}

/** Simple test. */
void
TestGameSpecBaseComponentVector::testIt()
{
    PublicComponentVector testee;
    game::spec::NullComponentNameProvider cnp;

    // Initial state
    TS_ASSERT_EQUALS(testee.size(), 0);
    TS_ASSERT(testee.findNext(0) == 0);
    TS_ASSERT(testee.findNext(1000) == 0);

    int n;
    TS_ASSERT(!testee.shortNames(cnp).getFirstKey(n));
    TS_ASSERT(!testee.names(cnp).getFirstKey(n));

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
    TS_ASSERT_EQUALS(testee.size(), 8);
    {
        game::spec::Component* p = testee.findNext(0);
        TS_ASSERT(p != 0);
        TS_ASSERT_EQUALS(p->getId(), 2);

        p = testee.findNext(2);
        TS_ASSERT(p != 0);
        TS_ASSERT_EQUALS(p->getId(), 8);

        p = testee.findNext(8);
        TS_ASSERT(p == 0);
    }

    TS_ASSERT(testee.get(0) == 0);
    TS_ASSERT(testee.get(1) == 0);
    TS_ASSERT(testee.get(2) != 0);
    TS_ASSERT(testee.get(3) == 0);

    TS_ASSERT(testee.get(7) == 0);
    TS_ASSERT(testee.get(8) != 0);
    TS_ASSERT(testee.get(9) == 0);

    TS_ASSERT_EQUALS(testee.names(cnp)(-1), "");
    TS_ASSERT_EQUALS(testee.names(cnp)(0), "");
    TS_ASSERT_EQUALS(testee.names(cnp)(2), "a");
    TS_ASSERT_EQUALS(testee.names(cnp)(5), "");
    TS_ASSERT_EQUALS(testee.names(cnp)(8), "b");
    TS_ASSERT_EQUALS(testee.names(cnp)(999), "");

    n = 0;
    TS_ASSERT(testee.shortNames(cnp).getFirstKey(n));
    TS_ASSERT_EQUALS(n, 2);
    TS_ASSERT(testee.shortNames(cnp).getNextKey(n));
    TS_ASSERT_EQUALS(n, 8);
    TS_ASSERT(!testee.shortNames(cnp).getNextKey(n));

    n = 0;
    TS_ASSERT(testee.names(cnp).getFirstKey(n));
    TS_ASSERT_EQUALS(n, 2);

    // Clear restores initial state
    testee.clear();
    TS_ASSERT_EQUALS(testee.size(), 0);
    TS_ASSERT(testee.findNext(0) == 0);
    TS_ASSERT(testee.findNext(1000) == 0);

    TS_ASSERT(!testee.shortNames(cnp).getFirstKey(n));
    TS_ASSERT(!testee.names(cnp).getFirstKey(n));

    TS_ASSERT_EQUALS(testee.names(cnp)(8), "");
}

/** Test out-of-range behaviour. */
void
TestGameSpecBaseComponentVector::testOutOfRange()
{
    PublicComponentVector testee;

    testee.setNew(0, new game::spec::Component(game::spec::ComponentNameProvider::Beam, 0));
    testee.setNew(-1, new game::spec::Component(game::spec::ComponentNameProvider::Beam, 0));
    testee.setNew(-99, new game::spec::Component(game::spec::ComponentNameProvider::Beam, 0));

    TS_ASSERT_EQUALS(testee.size(), 0);
    TS_ASSERT(testee.findNext(0) == 0);
    TS_ASSERT(testee.findNext(1000) == 0);
    TS_ASSERT(testee.get(-99) == 0);
}

