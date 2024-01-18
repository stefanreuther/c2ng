/**
  *  \file test/game/alliance/containertest.cpp
  *  \brief Test for game::alliance::Container
  */

#include "game/alliance/container.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

using game::alliance::Container;
using game::alliance::Level;
using game::alliance::Offer;

/** Test initialisation.
    A: create empty container.
    E: verify initial attributes */
AFL_TEST("game.alliance.Container:empty", a)
{
    Container testee;
    a.check("01. getLevels", testee.getLevels().empty());
    a.check("02. getOffers", testee.getOffers().empty());
    a.checkEqual("03. find", testee.find("x"), Container::nil);
    a.checkNull("04. getLevel", testee.getLevel(0));
    a.checkNull("05. getOffer", testee.getOffer(0));
    a.checkNull("06. getMutableOffer", testee.getMutableOffer(0));
}

/** Test basic operation.
    A: create a container. Add some levels and work with them.
    E: verify correct results */
AFL_TEST("game.alliance.Container:basic", a)
{
    // Create container with two levels
    Container testee;
    testee.addLevel(Level("First Level", "one",  Level::Flags_t(Level::IsOffer)));
    testee.addLevel(Level("Second Level", "two", Level::Flags_t(Level::NeedsOffer)));

    // Both levels need to be present
    a.checkEqual("01. getLevels. getLevel", testee.getLevels().size(), 2U);
    a.checkEqual("02. getOffers. getOffer", testee.getOffers().size(), 2U);

    // Indexes need be resolvable
    Container::Index_t x1 = testee.find("one");
    Container::Index_t x2 = testee.find("two");
    a.check("11. find", x1 != Container::nil);
    a.check("12. find", x2 != Container::nil);
    a.checkNonNull("13. getLevel", testee.getLevel(x1));
    a.checkNonNull("14. getOffer", testee.getOffer(x1));
    a.checkNonNull("15. getLevel", testee.getLevel(x2));
    a.checkNonNull("16. getOffer", testee.getOffer(x2));
    a.checkEqual("17. getOffer", testee.getOffer(x1), testee.getMutableOffer(x1));
    a.checkDifferent("18. different levels", x1, x2);

    // No offers must be present
    a.check("21. isAny", !testee.isAny(4, Level::IsOffer,    false));
    a.check("22. isAny", !testee.isAny(4, Level::NeedsOffer, false));
    a.check("23. isAny", !testee.isAny(4, Level::IsEnemy,    false));
    a.check("24. isAny", !testee.isAny(4, Level::IsOffer,    true));
    a.check("25. isAny", !testee.isAny(4, Level::NeedsOffer, true));
    a.check("26. isAny", !testee.isAny(4, Level::IsEnemy,    true));

    // Set an offer
    // - for now, container does NOT implement cascading
    testee.set(x1, 4, Offer::Yes);
    a.check("31. isAny", !testee.isAny(4, Level::IsOffer,    false));
    a.check("32. isAny",  testee.isAny(4, Level::IsOffer,    true));

    // Clear all
    testee.setAll(4, Level::IsOffer, false);
    a.check("41. isAny", !testee.isAny(4, Level::IsOffer,    false));
    a.check("42. isAny", !testee.isAny(4, Level::IsOffer,    true));
}

/** Test Container copy operations.
    A: create a container. Create copies in different ways.
    E: copies have same structure */
AFL_TEST("game.alliance.Container:copy", a)
{
    Container orig;
    orig.addLevel(Level("First Level", "one",  Level::Flags_t()));
    orig.addLevel(Level("Second Level", "two", Level::Flags_t()));
    Container::Index_t x1 = orig.find("one");
    Container::Index_t x2 = orig.find("two");

    Container copy1(orig);
    a.checkEqual("01. find", copy1.find("one"), x1);
    a.checkEqual("02. find", copy1.find("two"), x2);

    Container copy2;
    copy2 = orig;
    a.checkEqual("11. find", copy2.find("one"), x1);
    a.checkEqual("12. find", copy2.find("two"), x2);
}

/** Test Container::copyFrom().
    A: create two containers of different structure. Call copyFrom().
    E: new offer is copied but structure of merge target is unchanged */
AFL_TEST("game.alliance.Container:copyFrom", a)
{
    // Create left container
    Container left;
    left.addLevel(Level("First Level", "one",  Level::Flags_t()));
    left.addLevel(Level("Second Level", "two", Level::Flags_t()));
    Container::Index_t x1 = left.find("one");

    // Create right container with different structure
    Container right;
    right.addLevel(Level("Right 2", "two", Level::Flags_t()));
    right.addLevel(Level("Right 1", "one",  Level::Flags_t()));
    right.addLevel(Level("Right 3", "three",  Level::Flags_t()));
    right.set(right.find("one"), 6, Offer::Yes);

    // Merge
    left.copyFrom(right);

    // Verify that merge didn't change the structure but copied the offer
    a.checkEqual("01. getLevels", left.getLevels().size(), 2U);
    a.checkEqual("02. find", left.find("one"), x1);
    a.checkEqual("03. getLevel", left.getLevel(x1)->getName(), "First Level");
    a.checkEqual("04. getOffer", left.getOffer(x1)->newOffer.get(6), Offer::Yes);
}

/** Test listener handling.
    A: create a container and attach a listener.
    E: verify correct methods of listener are called. */
AFL_TEST("game.alliance.Container:addNewHandler", a)
{
    // Test listener
    struct Counters {
        int numInit;
        int numPostprocess;
        int numHandleChanges;
        Counters()
            : numInit(0), numPostprocess(0), numHandleChanges(0)
            { }
    };
    class Handler : public game::alliance::Handler {
     public:
        Handler(Counters& c)
            : m_counters(c)
            { }
        virtual void init(Container&, afl::string::Translator&)
            { ++m_counters.numInit; }
        virtual void postprocess(Container&)
            { ++m_counters.numPostprocess; }
        virtual void handleChanges(const Container&)
            { ++m_counters.numHandleChanges; }
     private:
        Counters& m_counters;
    };

    // Test init()
    afl::string::NullTranslator tx;
    Counters c;
    Container testee;
    testee.addLevel(Level("Level", "me", Level::Flags_t()));
    testee.addNewHandler(new Handler(c), tx);
    a.checkEqual("01. numInit",          c.numInit, 1);
    a.checkEqual("02. numPostprocess",   c.numPostprocess, 0);
    a.checkEqual("03. numHandleChanges", c.numHandleChanges, 0);

    // Test postprocess()
    testee.postprocess();
    a.checkEqual("11. numInit",          c.numInit, 1);
    a.checkEqual("12. numPostprocess",   c.numPostprocess, 1);
    a.checkEqual("13. numHandleChanges", c.numHandleChanges, 0);

    // Test set()
    Container::Index_t x = testee.find("me");
    testee.set(x, 4, Offer::Yes);
    a.checkEqual("21. numHandleChanges", c.numHandleChanges, 1);
    testee.set(x, 4, Offer::No);
    a.checkEqual("22. numHandleChanges", c.numHandleChanges, 2);

    // - no call if not actually a change
    testee.set(x, 4, Offer::No);
    a.checkEqual("31. numHandleChanges", c.numHandleChanges, 2);

    // - no call if out of range
    testee.set(999, 4, Offer::No);
    a.checkEqual("41. numHandleChanges", c.numHandleChanges, 2);
}
