/**
  *  \file test/game/cargocontainertest.cpp
  *  \brief Test for game::CargoContainer
  */

#include "game/cargocontainer.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.CargoContainer:interface")
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
AFL_TEST("game.CargoContainer:isValid:impossible", a)
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
    a.check("01. isValid", t.isValid());
    a.check("02. isEmpty", t.isEmpty());
    a.checkEqual("03. getChange", t.getChange(game::Element::Tritanium), 0);
    a.checkEqual("04. getEffectiveAmount", t.getEffectiveAmount(game::Element::Tritanium), 200);

    // Remove 50 Tritanium. This makes the container invalid because the effective amount is out of range.
    t.change(game::Element::Tritanium, -50);
    a.check("11. isEmpty", !t.isEmpty());
    a.check("12. isValid", !t.isValid());
    a.checkEqual("13. getChange", t.getChange(game::Element::Tritanium), -50);
    a.checkEqual("14. getEffectiveAmount", t.getEffectiveAmount(game::Element::Tritanium), 150);

    // Remove another 50 Tritanium. This makes the container valid because T is now valid, everything else unchanged.
    t.change(game::Element::Tritanium, -50);
    a.check("21. isValid", t.isValid());
    a.checkEqual("22. getChange", t.getChange(game::Element::Tritanium), -100);
    a.checkEqual("23. getEffectiveAmount", t.getEffectiveAmount(game::Element::Tritanium), 100);

    // Remove 50 Supplies. This again makes the container invalid.
    t.change(game::Element::Supplies, -50);
    a.check("31. isValid", !t.isValid());
    a.checkEqual("32. getChange", t.getChange(game::Element::Supplies), -50);

    // Add 50 supplies. This makes the contained valid (reverts the change).
    t.change(game::Element::Supplies, 50);
    a.check("41. isValid", t.isValid());
    a.check("42. isEmpty", !t.isEmpty());
    a.checkEqual("43. getChange", t.getChange(game::Element::Supplies), 0);

    // Clear.
    t.clear();
    a.check("51. isEmpty", t.isEmpty());
    a.check("52. isValid", t.isValid());
    a.checkEqual("53. getChange", t.getChange(game::Element::Supplies), 0);
    a.checkEqual("54. getChange", t.getChange(game::Element::Tritanium), 0);
    a.checkEqual("55. getEffectiveAmount", t.getEffectiveAmount(game::Element::Tritanium), 200);
}

/** Test initial state. */
AFL_TEST("game.CargoContainer:initial", a)
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

    a.check("01. isOverload", !t.isOverload());
    a.checkEqual("02. getChange", t.getChange(game::Element::Neutronium), 0);
    a.checkEqual("03. getChange", t.getChange(game::Element::Money), 0);
    a.checkEqual("04. getChange", t.getChange(game::Element::Supplies), 0);
    a.checkEqual("05. getEffectiveAmount", t.getEffectiveAmount(game::Element::Tritanium), 50);
    a.check("06. isValid", t.isValid());
    a.check("07. isEmpty", t.isEmpty());
}

/** Test overload. */
AFL_TEST("game.CargoContainer:overload", a)
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
    a.check("01. isOverload", !t.isOverload());
    a.checkEqual("02. getMaxAmount", t.getMaxAmount(game::Element::Neutronium), 10);

    t.setOverload(true);
    a.check("11. isOverload", t.isOverload());
    a.checkEqual("12. getMaxAmount", t.getMaxAmount(game::Element::Neutronium), 1000);

    t.setOverload(false);
    a.check("21. isOverload", !t.isOverload());
    a.checkEqual("22. getMaxAmount", t.getMaxAmount(game::Element::Neutronium), 10);
}
