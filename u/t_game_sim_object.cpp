/**
  *  \file u/t_game_sim_object.cpp
  *  \brief Test for game::sim::Object
  */

#include "game/sim/object.hpp"

#include "t_game_sim.hpp"

namespace {
    class Tester : public game::sim::Object {
     public:
        virtual bool hasImpliedAbility(game::sim::Ability /*which*/, const game::spec::ShipList& /*shipList*/, const game::config::HostConfiguration& /*config*/) const
            { return false; }
    };
}

/** Interface and setter/getter test. */
void
TestGameSimObject::testIt()
{
    Tester t;

    // Initial state (this also catches uninitialized members in valgrind)
    TS_ASSERT_EQUALS(t.getId(), 1);
    TS_ASSERT_EQUALS(t.getName(), "?");
    TS_ASSERT_EQUALS(t.getFriendlyCode(), "?""?""?");
    TS_ASSERT_EQUALS(t.getDamage(), 0);
    TS_ASSERT_EQUALS(t.getShield(), 100);
    TS_ASSERT_EQUALS(t.getOwner(), 12);
    TS_ASSERT_EQUALS(t.getExperienceLevel(), 0);
    TS_ASSERT_EQUALS(t.getFlags(), 0);
    TS_ASSERT_EQUALS(t.getFlakRatingOverride(), 0);
    TS_ASSERT_EQUALS(t.getFlakCompensationOverride(), 0);

    verifyObject(t);
}

/** Common part to verify an object. */
void
TestGameSimObject::verifyObject(game::sim::Object& t)
{
    // Get/Set
    t.markClean();
    t.setId(99);
    TS_ASSERT_EQUALS(t.getId(), 99);
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setName("Wow!");
    TS_ASSERT_EQUALS(t.getName(), "Wow!");
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setFriendlyCode("abc");
    TS_ASSERT_EQUALS(t.getFriendlyCode(), "abc");
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setDamage(142);
    TS_ASSERT_EQUALS(t.getDamage(), 142);
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setShield(20);
    TS_ASSERT_EQUALS(t.getShield(), 20);
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setOwner(30);
    TS_ASSERT_EQUALS(t.getOwner(), 30);
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setExperienceLevel(10);
    TS_ASSERT_EQUALS(t.getExperienceLevel(), 10);
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setFlags(game::sim::Object::fl_Commander);
    TS_ASSERT_EQUALS(t.getFlags(), game::sim::Object::fl_Commander);
    TS_ASSERT(!t.hasAnyNonstandardAbility());     // Commander bit alone is not effective
    TS_ASSERT(t.isDirty());
    t.setFlags(game::sim::Object::fl_Commander | game::sim::Object::fl_CommanderSet);
    TS_ASSERT(t.hasAnyNonstandardAbility());
    const game::config::HostConfiguration hostConfig;
    const game::spec::ShipList shipList;
    TS_ASSERT(t.hasAbility(game::sim::CommanderAbility, shipList, hostConfig));

    t.markClean();
    t.setFlakRatingOverride(1342);
    TS_ASSERT_EQUALS(t.getFlakRatingOverride(), 1342);
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setFlakCompensationOverride(9999);
    TS_ASSERT_EQUALS(t.getFlakCompensationOverride(), 9999);
    TS_ASSERT(t.isDirty());
}

/** Test setRandomFriendlyCodeFlags(). */
void
TestGameSimObject::testSetRandom()
{
    Tester t;
    t.setFriendlyCode("abc");
    t.setRandomFriendlyCodeFlags();
    TS_ASSERT_EQUALS(t.getFlags(), 0);

    t.setFriendlyCode("#bc");
    t.setRandomFriendlyCodeFlags();
    TS_ASSERT_EQUALS(t.getFlags(), game::sim::Object::fl_RandomFC + game::sim::Object::fl_RandomFC1);

    t.setFriendlyCode("a#c");
    t.setRandomFriendlyCodeFlags();
    TS_ASSERT_EQUALS(t.getFlags(), game::sim::Object::fl_RandomFC + game::sim::Object::fl_RandomFC2);

    t.setFriendlyCode("ab#");
    t.setRandomFriendlyCodeFlags();
    TS_ASSERT_EQUALS(t.getFlags(), game::sim::Object::fl_RandomFC + game::sim::Object::fl_RandomFC3);

    t.setFriendlyCode("#b#");
    t.setRandomFriendlyCodeFlags();
    TS_ASSERT_EQUALS(t.getFlags(), game::sim::Object::fl_RandomFC + game::sim::Object::fl_RandomFC1 + game::sim::Object::fl_RandomFC3);

    t.setFriendlyCode("xyz");
    t.setRandomFriendlyCodeFlags();
    TS_ASSERT_EQUALS(t.getFlags(), 0);
}

/** Test setRandomFriendlyCode(). */
void
TestGameSimObject::testRandom()
{
    Tester t;

    // Initial state: random disabled
    TS_ASSERT_EQUALS(t.getFlags(), 0);
    t.setFriendlyCode("aaa");
    t.setRandomFriendlyCode();
    TS_ASSERT_EQUALS(t.getFriendlyCode(), "aaa");

    // Enable randomness but don't specify digits
    t.setFlags(game::sim::Object::fl_RandomFC);
    for (int i = 0; i < 1000; ++i) {
        t.setRandomFriendlyCode();
        String_t s = t.getFriendlyCode();
        TS_ASSERT_EQUALS(s.size(), 3U);
        TS_ASSERT_LESS_THAN_EQUALS('0', s[0]);
        TS_ASSERT_LESS_THAN_EQUALS(s[0], '9');
        TS_ASSERT_LESS_THAN_EQUALS('0', s[1]);
        TS_ASSERT_LESS_THAN_EQUALS(s[1], '9');
        TS_ASSERT_LESS_THAN_EQUALS('0', s[2]);
        TS_ASSERT_LESS_THAN_EQUALS(s[2], '9');
    }

    // Enable randomness with digits
    t.setFlags(game::sim::Object::fl_RandomFC + game::sim::Object::fl_RandomFC2);
    t.setFriendlyCode("axc");
    for (int i = 0; i < 1000; ++i) {
        t.setRandomFriendlyCode();
        String_t s = t.getFriendlyCode();
        TS_ASSERT_EQUALS(s.size(), 3U);
        TS_ASSERT_EQUALS(s[0], 'a');
        TS_ASSERT_LESS_THAN_EQUALS('0', s[1]);
        TS_ASSERT_LESS_THAN_EQUALS(s[1], '9');
        TS_ASSERT_EQUALS(s[2], 'c');
    }

    // Same thing, but start with shorter code
    t.setFlags(game::sim::Object::fl_RandomFC + game::sim::Object::fl_RandomFC2);
    t.setFriendlyCode("a");
    for (int i = 0; i < 1000; ++i) {
        t.setRandomFriendlyCode();
        String_t s = t.getFriendlyCode();
        TS_ASSERT_EQUALS(s.size(), 3U);
        TS_ASSERT_EQUALS(s[0], 'a');
        TS_ASSERT_LESS_THAN_EQUALS('0', s[1]);
        TS_ASSERT_LESS_THAN_EQUALS(s[1], '9');
        TS_ASSERT_EQUALS(s[2], ' ');
    }
}

