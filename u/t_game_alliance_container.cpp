/**
  *  \file u/t_game_alliance_container.cpp
  *  \brief Test for game::alliance::Container
  */

#include "game/alliance/container.hpp"

#include "t_game_alliance.hpp"
#include "afl/string/nulltranslator.hpp"

using game::alliance::Container;
using game::alliance::Level;
using game::alliance::Offer;

/** Test initialisation.
    A: create empty container.
    E: verify initial attributes */
void
TestGameAllianceContainer::testEmpty()
{
    Container testee;
    TS_ASSERT(testee.getLevels().empty());
    TS_ASSERT(testee.getOffers().empty());
    TS_ASSERT_EQUALS(testee.find("x"), Container::nil);
    TS_ASSERT(testee.getLevel(0) == 0);
    TS_ASSERT(testee.getOffer(0) == 0);
    TS_ASSERT(testee.getMutableOffer(0) == 0);
}

/** Test basic operation.
    A: create a container. Add some levels and work with them.
    E: verify correct results */
void
TestGameAllianceContainer::testIt()
{
    // Create container with two levels
    Container testee;
    testee.addLevel(Level("First Level", "one",  Level::Flags_t(Level::IsOffer)));
    testee.addLevel(Level("Second Level", "two", Level::Flags_t(Level::NeedsOffer)));

    // Both levels need to be present
    TS_ASSERT_EQUALS(testee.getLevels().size(), 2U);
    TS_ASSERT_EQUALS(testee.getOffers().size(), 2U);

    // Indexes need be resolvable
    Container::Index_t x1 = testee.find("one");
    Container::Index_t x2 = testee.find("two");
    TS_ASSERT(x1 != Container::nil);
    TS_ASSERT(x2 != Container::nil);
    TS_ASSERT(testee.getLevel(x1) != 0);
    TS_ASSERT(testee.getOffer(x1) != 0);
    TS_ASSERT(testee.getLevel(x2) != 0);
    TS_ASSERT(testee.getOffer(x2) != 0);
    TS_ASSERT_EQUALS(testee.getOffer(x1), testee.getMutableOffer(x1));
    TS_ASSERT_DIFFERS(x1, x2);

    // No offers must be present
    TS_ASSERT(!testee.isAny(4, Level::IsOffer,    false));
    TS_ASSERT(!testee.isAny(4, Level::NeedsOffer, false));
    TS_ASSERT(!testee.isAny(4, Level::IsEnemy,    false));
    TS_ASSERT(!testee.isAny(4, Level::IsOffer,    true));
    TS_ASSERT(!testee.isAny(4, Level::NeedsOffer, true));
    TS_ASSERT(!testee.isAny(4, Level::IsEnemy,    true));

    // Set an offer
    // - for now, container does NOT implement cascading
    testee.set(x1, 4, Offer::Yes);
    TS_ASSERT(!testee.isAny(4, Level::IsOffer,    false));
    TS_ASSERT( testee.isAny(4, Level::IsOffer,    true));

    // Clear all
    testee.setAll(4, Level::IsOffer, false);
    TS_ASSERT(!testee.isAny(4, Level::IsOffer,    false));
    TS_ASSERT(!testee.isAny(4, Level::IsOffer,    true));
}

/** Test Container copy operations.
    A: create a container. Create copies in different ways.
    E: copies have same structure */
void
TestGameAllianceContainer::testCopy()
{
    Container orig;
    orig.addLevel(Level("First Level", "one",  Level::Flags_t()));
    orig.addLevel(Level("Second Level", "two", Level::Flags_t()));
    Container::Index_t x1 = orig.find("one");
    Container::Index_t x2 = orig.find("two");

    Container copy1(orig);
    TS_ASSERT_EQUALS(copy1.find("one"), x1);
    TS_ASSERT_EQUALS(copy1.find("two"), x2);

    Container copy2;
    copy2 = orig;
    TS_ASSERT_EQUALS(copy2.find("one"), x1);
    TS_ASSERT_EQUALS(copy2.find("two"), x2);
}

/** Test Container::copyFrom().
    A: create two containers of different structure. Call copyFrom().
    E: new offer is copied but structure of merge target is unchanged */
void
TestGameAllianceContainer::testMerge()
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
    TS_ASSERT_EQUALS(left.getLevels().size(), 2U);
    TS_ASSERT_EQUALS(left.find("one"), x1);
    TS_ASSERT_EQUALS(left.getLevel(x1)->getName(), "First Level");
    TS_ASSERT_EQUALS(left.getOffer(x1)->newOffer.get(6), Offer::Yes);
}

/** Test listener handling.
    A: create a container and attach a listener.
    E: verify correct methods of listener are called. */
void
TestGameAllianceContainer::testListener()
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
    TS_ASSERT_EQUALS(c.numInit, 1);
    TS_ASSERT_EQUALS(c.numPostprocess, 0);
    TS_ASSERT_EQUALS(c.numHandleChanges, 0);

    // Test postprocess()
    testee.postprocess();
    TS_ASSERT_EQUALS(c.numInit, 1);
    TS_ASSERT_EQUALS(c.numPostprocess, 1);
    TS_ASSERT_EQUALS(c.numHandleChanges, 0);

    // Test set()
    Container::Index_t x = testee.find("me");
    testee.set(x, 4, Offer::Yes);
    TS_ASSERT_EQUALS(c.numHandleChanges, 1);
    testee.set(x, 4, Offer::No);
    TS_ASSERT_EQUALS(c.numHandleChanges, 2);

    // - no call if not actually a change
    testee.set(x, 4, Offer::No);
    TS_ASSERT_EQUALS(c.numHandleChanges, 2);

    // - no call if out of range
    testee.set(999, 4, Offer::No);
    TS_ASSERT_EQUALS(c.numHandleChanges, 2);
}

