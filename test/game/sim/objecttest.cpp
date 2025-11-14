/**
  *  \file test/game/sim/objecttest.cpp
  *  \brief Test for game::sim::Object
  */

#include "game/sim/object.hpp"

#include "afl/test/testrunner.hpp"
#include "game/sim/configuration.hpp"

#include "objecttest.hpp"

namespace {
    class Tester : public game::sim::Object {
     public:
        virtual bool hasImpliedAbility(game::sim::Ability /*which*/, const game::sim::Configuration& /*opts*/, const game::spec::ShipList& /*shipList*/, const game::config::HostConfiguration& /*config*/) const
            { return false; }
    };
}


// Common part to verify an object. */
void
game::sim::verifyObject(afl::test::Assert a, game::sim::Object& t)
{
    // Get/Set
    t.markClean();
    t.setId(99);
    a.checkEqual("11. getId", t.getId(), 99);
    a.check("12. isDirty", t.isDirty());

    t.markClean();
    t.setName("Wow!");
    a.checkEqual("21. getName", t.getName(), "Wow!");
    a.check("22. isDirty", t.isDirty());

    t.markClean();
    t.setFriendlyCode("abc");
    a.checkEqual("31. getFriendlyCode", t.getFriendlyCode(), "abc");
    a.check("32. isDirty", t.isDirty());

    t.markClean();
    t.setDamage(142);
    a.checkEqual("41. getDamage", t.getDamage(), 142);
    a.check("42. isDirty", t.isDirty());

    t.markClean();
    t.setShield(20);
    a.checkEqual("51. getShield", t.getShield(), 20);
    a.check("52. isDirty", t.isDirty());

    t.markClean();
    t.setOwner(30);
    a.checkEqual("61. getOwner", t.getOwner(), 30);
    a.check("62. isDirty", t.isDirty());

    t.markClean();
    t.setExperienceLevel(10);
    a.checkEqual("71. getExperienceLevel", t.getExperienceLevel(), 10);
    a.check("72. isDirty", t.isDirty());

    afl::base::Ref<const game::config::HostConfiguration> hostConfig = game::config::HostConfiguration::create();
    const game::spec::ShipList shipList;
    const game::sim::Configuration opts;

    t.markClean();
    t.setFlags(game::sim::Object::fl_Commander);
    a.checkEqual("81. getFlags", t.getFlags(), game::sim::Object::fl_Commander);
    a.check("82. hasAnyNonstandardAbility", !t.hasAnyNonstandardAbility());     // Commander bit alone is not effective
    a.check("83. getAbilities", t.getAbilities(opts, shipList, *hostConfig).empty());
    a.check("84. isDirty", t.isDirty());
    t.setFlags(game::sim::Object::fl_Commander | game::sim::Object::fl_CommanderSet);
    a.check("85. hasAnyNonstandardAbility", t.hasAnyNonstandardAbility());
    a.check("86. getAbilities", t.hasAbility(game::sim::CommanderAbility, opts, shipList, *hostConfig));
    a.checkEqual("87. getAbilities", t.getAbilities(opts, shipList, *hostConfig), game::sim::Abilities_t(game::sim::CommanderAbility));

    t.markClean();
    t.setFlakRatingOverride(1342);
    a.checkEqual("91. getFlakRatingOverride", t.getFlakRatingOverride(), 1342);
    a.check("92. isDirty", t.isDirty());

    t.markClean();
    t.setFlakCompensationOverride(9999);
    a.checkEqual("101. getFlakCompensationOverride", t.getFlakCompensationOverride(), 9999);
    a.check("102. isDirty", t.isDirty());
}

/*
 *  Tests
 */

/** Interface and setter/getter test. */
AFL_TEST("game.sim.Object:basics", a)
{
    Tester t;

    // Initial state (this also catches uninitialized members in valgrind)
    a.checkEqual("01. getId",                       t.getId(), 1);
    a.checkEqual("02. getName",                     t.getName(), "?");
    a.checkEqual("03. getFriendlyCode",             t.getFriendlyCode(), "?""?""?");
    a.checkEqual("04. getDamage",                   t.getDamage(), 0);
    a.checkEqual("05. getShield",                   t.getShield(), 100);
    a.checkEqual("06. getOwner",                    t.getOwner(), 12);
    a.checkEqual("07. getExperienceLevel",          t.getExperienceLevel(), 0);
    a.checkEqual("08. getFlags",                    t.getFlags(), 0);
    a.checkEqual("09. getFlakRatingOverride",       t.getFlakRatingOverride(), 0);
    a.checkEqual("10. getFlakCompensationOverride", t.getFlakCompensationOverride(), 0);

    verifyObject(a, t);
}

/** Test setRandomFriendlyCodeFlags(). */
AFL_TEST("game.sim.Object:setRandomFriendlyCodeFlags", a)
{
    Tester t;
    t.setFriendlyCode("abc");
    t.setRandomFriendlyCodeFlags();
    a.checkEqual("01. getFlags", t.getFlags(), 0);

    t.setFriendlyCode("#bc");
    t.setRandomFriendlyCodeFlags();
    a.checkEqual("11. getFlags", t.getFlags(), game::sim::Object::fl_RandomFC + game::sim::Object::fl_RandomFC1);

    t.setFriendlyCode("a#c");
    t.setRandomFriendlyCodeFlags();
    a.checkEqual("21. getFlags", t.getFlags(), game::sim::Object::fl_RandomFC + game::sim::Object::fl_RandomFC2);

    t.setFriendlyCode("ab#");
    t.setRandomFriendlyCodeFlags();
    a.checkEqual("31. getFlags", t.getFlags(), game::sim::Object::fl_RandomFC + game::sim::Object::fl_RandomFC3);

    t.setFriendlyCode("#b#");
    t.setRandomFriendlyCodeFlags();
    a.checkEqual("41. getFlags", t.getFlags(), game::sim::Object::fl_RandomFC + game::sim::Object::fl_RandomFC1 + game::sim::Object::fl_RandomFC3);

    t.setFriendlyCode("xyz");
    t.setRandomFriendlyCodeFlags();
    a.checkEqual("51. getFlags", t.getFlags(), 0);

    t.setFriendlyCode("a#");             // string shorter than usual
    t.setRandomFriendlyCodeFlags();
    a.checkEqual("61. getFlags", t.getFlags(), game::sim::Object::fl_RandomFC + game::sim::Object::fl_RandomFC2);
}

/** Test setRandomFriendlyCode(). */
AFL_TEST("game.sim.Object:setRandomFriendlyCode", a)
{
    Tester t;
    util::RandomNumberGenerator rng(666);

    // Initial state: random disabled
    a.checkEqual("01. getFlags", t.getFlags(), 0);
    t.setFriendlyCode("aaa");
    t.setRandomFriendlyCode(rng);
    a.checkEqual("02. getFriendlyCode", t.getFriendlyCode(), "aaa");

    // Enable randomness but don't specify digits
    t.setFlags(game::sim::Object::fl_RandomFC);
    for (int i = 0; i < 1000; ++i) {
        t.setRandomFriendlyCode(rng);
        String_t s = t.getFriendlyCode();
        a.checkEqual("11. size", s.size(), 3U);
        a.check("12. s[0]", '0' <= s[0]);
        a.check("13. s[0]", s[0] <= '9');
        a.check("14. s[1]", '0' <= s[1]);
        a.check("15. s[1]", s[1] <= '9');
        a.check("16. s[2]", '0' <= s[2]);
        a.check("17. s[2]", s[2] <= '9');
    }

    // Enable randomness with digits
    t.setFlags(game::sim::Object::fl_RandomFC + game::sim::Object::fl_RandomFC2);
    t.setFriendlyCode("axc");
    for (int i = 0; i < 1000; ++i) {
        t.setRandomFriendlyCode(rng);
        String_t s = t.getFriendlyCode();
        a.checkEqual("21. size", s.size(), 3U);
        a.checkEqual("22. s[0]", s[0], 'a');
        a.checkLessEqual("23. s[1]", '0', s[1]);
        a.checkLessEqual("24. s[2]", s[1], '9');
        a.checkEqual("25. s[2]", s[2], 'c');
    }

    // Same thing, but start with shorter code
    t.setFlags(game::sim::Object::fl_RandomFC + game::sim::Object::fl_RandomFC2);
    t.setFriendlyCode("a");
    for (int i = 0; i < 1000; ++i) {
        t.setRandomFriendlyCode(rng);
        String_t s = t.getFriendlyCode();
        a.checkEqual("31. size", s.size(), 3U);
        a.checkEqual("32. s[0]", s[0], 'a');
        a.checkLessEqual("33. s[1]", '0', s[1]);
        a.checkLessEqual("34. s[1]", s[1], '9');
        a.checkEqual("35. s[2]", s[2], ' ');
    }
}

/** Test copying. */
AFL_TEST("game.sim.Object:copy", a)
{
    Tester ta;
    ta.setId(100);
    ta.markClean();

    Tester tb;
    tb.setId(200);
    tb.markClean();

    // Assignment makes object dirty
    tb = ta;
    a.check("01. isDirty", tb.isDirty());

    // Copy of a dirty object is not dirty
    Tester tc(tb);
    a.check("11. isDirty", !tc.isDirty());

    // Self-assignment is not dirty
    tc = tc;
    a.check("21. isDirty", !tc.isDirty());
}
