/**
  *  \file u/t_game_cargocontainer.cpp
  *  \brief Test for game::CargoContainer
  */

#include "game/cargocontainer.hpp"

#include "t_game.hpp"

/** Interface test. */
void
TestGameCargoContainer::testInterface()
{
    class Tester : public game::CargoContainer {
     public:
        virtual String_t getName(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual String_t getInfo1(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual String_t getInfo2(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual Flags_t getFlags() const
            { return Flags_t(); }
        virtual bool canHaveElement(game::Element::Type /*type*/) const
            { return false; }
        virtual int32_t getMaxAmount(game::Element::Type /*type*/) const
            { return 0; }
        virtual int32_t getMinAmount(game::Element::Type /*type*/) const
            { return 0; }
        virtual int32_t getAmount(game::Element::Type /*type*/) const
            { return 0; }
        virtual void commit()
            { }
    };
    Tester t;
}

/** Test isValid() on impossible transaction. */
void
TestGameCargoContainer::testValidImpossible()
{
    // An invalid container. Can contain everything, but all amounts are out of range.
    class Tester : public game::CargoContainer {
     public:
        virtual String_t getName(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual String_t getInfo1(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual String_t getInfo2(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual Flags_t getFlags() const
            { return Flags_t(); }
        virtual bool canHaveElement(game::Element::Type /*type*/) const
            { return true; }
        virtual int32_t getMaxAmount(game::Element::Type /*type*/) const
            { return 100; }
        virtual int32_t getMinAmount(game::Element::Type /*type*/) const
            { return 0; }
        virtual int32_t getAmount(game::Element::Type /*type*/) const
            { return 200; }
        virtual void commit()
            { }
    };
    Tester t;

    // Since there are no changes yet, the container is valid.
    TS_ASSERT(t.isValid());
    TS_ASSERT(t.isEmpty());
    TS_ASSERT_EQUALS(t.getChange(game::Element::Tritanium), 0);
    TS_ASSERT_EQUALS(t.getEffectiveAmount(game::Element::Tritanium), 200);

    // Remove 50 Tritanium. This makes the container invalid because the effective amount is out of range.
    t.change(game::Element::Tritanium, -50);
    TS_ASSERT(!t.isEmpty());
    TS_ASSERT(!t.isValid());
    TS_ASSERT_EQUALS(t.getChange(game::Element::Tritanium), -50);
    TS_ASSERT_EQUALS(t.getEffectiveAmount(game::Element::Tritanium), 150);

    // Remove another 50 Tritanium. This makes the container valid because T is now valid, everything else unchanged.
    t.change(game::Element::Tritanium, -50);
    TS_ASSERT(t.isValid());
    TS_ASSERT_EQUALS(t.getChange(game::Element::Tritanium), -100);
    TS_ASSERT_EQUALS(t.getEffectiveAmount(game::Element::Tritanium), 100);

    // Remove 50 Supplies. This again makes the container invalid.
    t.change(game::Element::Supplies, -50);
    TS_ASSERT(!t.isValid());
    TS_ASSERT_EQUALS(t.getChange(game::Element::Supplies), -50);

    // Add 50 supplies. This makes the contained valid (reverts the change).
    t.change(game::Element::Supplies, 50);
    TS_ASSERT(t.isValid());
    TS_ASSERT(!t.isEmpty());
    TS_ASSERT_EQUALS(t.getChange(game::Element::Supplies), 0);

    // Clear.
    t.clear();
    TS_ASSERT(t.isEmpty());
    TS_ASSERT(t.isValid());
    TS_ASSERT_EQUALS(t.getChange(game::Element::Supplies), 0);
    TS_ASSERT_EQUALS(t.getChange(game::Element::Tritanium), 0);
    TS_ASSERT_EQUALS(t.getEffectiveAmount(game::Element::Tritanium), 200);
}

/** Test initial state. */
void
TestGameCargoContainer::testInitial()
{
    class Tester : public game::CargoContainer {
     public:
        virtual String_t getName(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual String_t getInfo1(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual String_t getInfo2(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual Flags_t getFlags() const
            { return Flags_t(); }
        virtual bool canHaveElement(game::Element::Type /*type*/) const
            { return true; }
        virtual int32_t getMaxAmount(game::Element::Type /*type*/) const
            { return 100; }
        virtual int32_t getMinAmount(game::Element::Type /*type*/) const
            { return 0; }
        virtual int32_t getAmount(game::Element::Type /*type*/) const
            { return 50; }
        virtual void commit()
            { }
    };
    const Tester t;

    TS_ASSERT(!t.isOverload());
    TS_ASSERT_EQUALS(t.getChange(game::Element::Neutronium), 0);
    TS_ASSERT_EQUALS(t.getChange(game::Element::Money), 0);
    TS_ASSERT_EQUALS(t.getChange(game::Element::Supplies), 0);
    TS_ASSERT_EQUALS(t.getEffectiveAmount(game::Element::Tritanium), 50);
    TS_ASSERT(t.isValid());
    TS_ASSERT(t.isEmpty());
}

/** Test overload. */
void
TestGameCargoContainer::testOverload()
{
    class Tester : public game::CargoContainer {
     public:
        virtual String_t getName(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual String_t getInfo1(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual String_t getInfo2(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual Flags_t getFlags() const
            { return Flags_t(); }
        virtual bool canHaveElement(game::Element::Type /*type*/) const
            { return false; }
        virtual int32_t getMaxAmount(game::Element::Type /*type*/) const
            { return isOverload() ? 1000 : 10; }
        virtual int32_t getMinAmount(game::Element::Type /*type*/) const
            { return 0; }
        virtual int32_t getAmount(game::Element::Type /*type*/) const
            { return 0; }
        virtual void commit()
            { }
    };
    Tester t;

    // Overload can be toggled at will.
    // The result of setOverload() must be accessible in getMaxAmount().
    TS_ASSERT(!t.isOverload());
    TS_ASSERT_EQUALS(t.getMaxAmount(game::Element::Neutronium), 10);

    t.setOverload(true);
    TS_ASSERT(t.isOverload());
    TS_ASSERT_EQUALS(t.getMaxAmount(game::Element::Neutronium), 1000);

    t.setOverload(false);
    TS_ASSERT(!t.isOverload());
    TS_ASSERT_EQUALS(t.getMaxAmount(game::Element::Neutronium), 10);
}

