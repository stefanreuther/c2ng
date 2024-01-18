/**
  *  \file test/game/actions/cargocostactiontest.cpp
  *  \brief Test for game::actions::CargoCostAction
  */

#include "game/actions/cargocostaction.hpp"
#include "afl/test/testrunner.hpp"

using game::spec::Cost;
using game::Element;

namespace {
    class TestContainer : public game::CargoContainer {
     public:
        virtual String_t getName(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual String_t getInfo1(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual String_t getInfo2(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual Flags_t getFlags() const
            { return Flags_t(SupplySale); }
        virtual bool canHaveElement(Element::Type /*type*/) const
            { return true; }
        virtual int32_t getMaxAmount(Element::Type /*type*/) const
            { return 1000; }
        virtual int32_t getMinAmount(Element::Type /*type*/) const
            { return 10; }
        virtual int32_t getAmount(Element::Type /*type*/) const
            { return 50; }
        virtual void commit()
            { }
    };
}

/** Test a normal case. */
AFL_TEST("game.actions.CargoCostAction:normal", a)
{
    TestContainer tc;
    game::actions::CargoCostAction testee(tc);

    testee.setCost(Cost::fromString("11T 12D 13M 14S 15$"));
    a.checkEqual("01. isValid", testee.isValid(), true);
    a.checkEqual("02. getCost", testee.getCost().get(Cost::Tritanium),  11);
    a.checkEqual("03. getCost", testee.getCost().get(Cost::Duranium),   12);
    a.checkEqual("04. getCost", testee.getCost().get(Cost::Molybdenum), 13);
    a.checkEqual("05. getCost", testee.getCost().get(Cost::Supplies),   14);
    a.checkEqual("06. getCost", testee.getCost().get(Cost::Money),      15);

    a.checkEqual("11. getRemainingAmount", testee.getRemainingAmount(Element::Tritanium),  39);
    a.checkEqual("12. getRemainingAmount", testee.getRemainingAmount(Element::Duranium),   38);
    a.checkEqual("13. getRemainingAmount", testee.getRemainingAmount(Element::Molybdenum), 37);
    a.checkEqual("14. getRemainingAmount", testee.getRemainingAmount(Element::Supplies),   36);
    a.checkEqual("15. getRemainingAmount", testee.getRemainingAmount(Element::Money),      35);
    a.checkEqual("16. getRemainingAmount", testee.getRemainingAmountAsCost().toCargoSpecString(), "39T 38D 37M 36S 35$");

    a.checkEqual("21. getMissingAmount", testee.getMissingAmount(Element::Tritanium),  0);
    a.checkEqual("22. getMissingAmount", testee.getMissingAmount(Element::Duranium),   0);
    a.checkEqual("23. getMissingAmount", testee.getMissingAmount(Element::Molybdenum), 0);
    a.checkEqual("24. getMissingAmount", testee.getMissingAmount(Element::Supplies),   0);
    a.checkEqual("25. getMissingAmount", testee.getMissingAmount(Element::Money),      0);
    a.checkEqual("26. getMissingAmount", testee.getMissingAmountAsCost().toCargoSpecString(), "");

    a.checkEqual("31. getMissingAmountAsCost", testee.getAvailableAmountAsCost().toCargoSpecString(), "50TDM 50S 50$");
}

/** Test a missing mineral. */
AFL_TEST("game.actions.CargoCostAction:missing-mineral", a)
{
    TestContainer tc;
    game::actions::CargoCostAction testee(tc);

    // We have 50, minimum 10, so 45 will overflow
    testee.setCost(Cost::fromString("45T"));
    a.checkEqual("01. isValid", testee.isValid(), false);
    a.checkEqual("02. getCost", testee.getCost().get(Cost::Tritanium),  45);
    a.checkEqual("03. getCost", testee.getCost().get(Cost::Duranium),   0);
    a.checkEqual("04. getCost", testee.getCost().get(Cost::Molybdenum), 0);
    a.checkEqual("05. getCost", testee.getCost().get(Cost::Supplies),   0);
    a.checkEqual("06. getCost", testee.getCost().get(Cost::Money),      0);

    a.checkEqual("11. getRemainingAmount", testee.getRemainingAmount(Element::Tritanium),   5);
    a.checkEqual("12. getRemainingAmount", testee.getRemainingAmount(Element::Duranium),   50);
    a.checkEqual("13. getRemainingAmount", testee.getRemainingAmount(Element::Molybdenum), 50);
    a.checkEqual("14. getRemainingAmount", testee.getRemainingAmount(Element::Supplies),   50);
    a.checkEqual("15. getRemainingAmount", testee.getRemainingAmount(Element::Money),      50);
    a.checkEqual("16. getRemainingAmount", testee.getRemainingAmountAsCost().toCargoSpecString(), "5T 50D 50M 50S 50$");

    a.checkEqual("21. getMissingAmount", testee.getMissingAmount(Element::Tritanium),  5);
    a.checkEqual("22. getMissingAmount", testee.getMissingAmount(Element::Duranium),   0);
    a.checkEqual("23. getMissingAmount", testee.getMissingAmount(Element::Molybdenum), 0);
    a.checkEqual("24. getMissingAmount", testee.getMissingAmount(Element::Supplies),   0);
    a.checkEqual("25. getMissingAmount", testee.getMissingAmount(Element::Money),      0);
    a.checkEqual("26. getMissingAmount", testee.getMissingAmountAsCost().toCargoSpecString(), "5T");
}

/** Test missing money. Will be compensated by selling supplies. */
AFL_TEST("game.actions.CargoCostAction:missing-money", a)
{
    TestContainer tc;
    game::actions::CargoCostAction testee(tc);

    // We have 50, minimum 10, so 45 will overflow
    testee.setCost(Cost::fromString("45$"));
    a.checkEqual("01. isValid", testee.isValid(), true);
    a.checkEqual("02. getCost", testee.getCost().get(Cost::Tritanium),  0);
    a.checkEqual("03. getCost", testee.getCost().get(Cost::Duranium),   0);
    a.checkEqual("04. getCost", testee.getCost().get(Cost::Molybdenum), 0);
    a.checkEqual("05. getCost", testee.getCost().get(Cost::Supplies),   0);
    a.checkEqual("06. getCost", testee.getCost().get(Cost::Money),      45);

    a.checkEqual("11. getRemainingAmount", testee.getRemainingAmount(Element::Tritanium),  50);
    a.checkEqual("12. getRemainingAmount", testee.getRemainingAmount(Element::Duranium),   50);
    a.checkEqual("13. getRemainingAmount", testee.getRemainingAmount(Element::Molybdenum), 50);
    a.checkEqual("14. getRemainingAmount", testee.getRemainingAmount(Element::Supplies),   45);
    a.checkEqual("15. getRemainingAmount", testee.getRemainingAmount(Element::Money),      10);

    a.checkEqual("21. getMissingAmount", testee.getMissingAmount(Element::Tritanium),  0);
    a.checkEqual("22. getMissingAmount", testee.getMissingAmount(Element::Duranium),   0);
    a.checkEqual("23. getMissingAmount", testee.getMissingAmount(Element::Molybdenum), 0);
    a.checkEqual("24. getMissingAmount", testee.getMissingAmount(Element::Supplies),   0);
    a.checkEqual("25. getMissingAmount", testee.getMissingAmount(Element::Money),      0);
}

/** Test missing supplies. */
AFL_TEST("game.actions.CargoCostAction:missing-supplies", a)
{
    TestContainer tc;
    game::actions::CargoCostAction testee(tc);

    // We have 50, minimum 10, so 45 will overflow
    testee.setCost(Cost::fromString("45S"));
    a.checkEqual("01. isValid", testee.isValid(), false);
    a.checkEqual("02. getCost", testee.getCost().get(Cost::Tritanium),  0);
    a.checkEqual("03. getCost", testee.getCost().get(Cost::Duranium),   0);
    a.checkEqual("04. getCost", testee.getCost().get(Cost::Molybdenum), 0);
    a.checkEqual("05. getCost", testee.getCost().get(Cost::Supplies),   45);
    a.checkEqual("06. getCost", testee.getCost().get(Cost::Money),      0);

    a.checkEqual("11. getRemainingAmount", testee.getRemainingAmount(Element::Tritanium),  50);
    a.checkEqual("12. getRemainingAmount", testee.getRemainingAmount(Element::Duranium),   50);
    a.checkEqual("13. getRemainingAmount", testee.getRemainingAmount(Element::Molybdenum), 50);
    a.checkEqual("14. getRemainingAmount", testee.getRemainingAmount(Element::Supplies),   5);
    a.checkEqual("15. getRemainingAmount", testee.getRemainingAmount(Element::Money),      50);

    a.checkEqual("21. getMissingAmount", testee.getMissingAmount(Element::Tritanium),  0);
    a.checkEqual("22. getMissingAmount", testee.getMissingAmount(Element::Duranium),   0);
    a.checkEqual("23. getMissingAmount", testee.getMissingAmount(Element::Molybdenum), 0);
    a.checkEqual("24. getMissingAmount", testee.getMissingAmount(Element::Supplies),   5);
    a.checkEqual("25. getMissingAmount", testee.getMissingAmount(Element::Money),      0);
}

/** Test missing money, more than can be compensated. */
AFL_TEST("game.actions.CargoCostAction:missing-money-and-supplies", a)
{
    TestContainer tc;
    game::actions::CargoCostAction testee(tc);

    // Cost is 300$; we can spend 40$+40S = 80$.
    testee.setCost(Cost::fromString("300$"));
    a.checkEqual("01. isValid", testee.isValid(), false);
    a.checkEqual("02. getCost", testee.getCost().get(Cost::Tritanium),  0);
    a.checkEqual("03. getCost", testee.getCost().get(Cost::Duranium),   0);
    a.checkEqual("04. getCost", testee.getCost().get(Cost::Molybdenum), 0);
    a.checkEqual("05. getCost", testee.getCost().get(Cost::Supplies),   0);
    a.checkEqual("06. getCost", testee.getCost().get(Cost::Money),      300);

    a.checkEqual("11. getRemainingAmount", testee.getRemainingAmount(Element::Tritanium),  50);
    a.checkEqual("12. getRemainingAmount", testee.getRemainingAmount(Element::Duranium),   50);
    a.checkEqual("13. getRemainingAmount", testee.getRemainingAmount(Element::Molybdenum), 50);
    a.checkEqual("14. getRemainingAmount", testee.getRemainingAmount(Element::Supplies),   -210);
    a.checkEqual("15. getRemainingAmount", testee.getRemainingAmount(Element::Money),      10);
    a.checkEqual("16. getRemainingAmount", testee.getRemainingAmountAsCost().toCargoSpecString(), "50TDM -210S 10$");

    a.checkEqual("21. getMissingAmount", testee.getMissingAmount(Element::Tritanium),  0);
    a.checkEqual("22. getMissingAmount", testee.getMissingAmount(Element::Duranium),   0);
    a.checkEqual("23. getMissingAmount", testee.getMissingAmount(Element::Molybdenum), 0);
    a.checkEqual("24. getMissingAmount", testee.getMissingAmount(Element::Supplies),   220);
    a.checkEqual("25. getMissingAmount", testee.getMissingAmount(Element::Money),      0);
    a.checkEqual("26. getMissingAmount", testee.getMissingAmountAsCost().toCargoSpecString(), "220S");
}

/** Test multiple modifications.
    Since we're updating the cost incrementally, this might uncover problems. */
AFL_TEST("game.actions.CargoCostAction:multiple-modifications", a)
{
    TestContainer tc;
    game::actions::CargoCostAction testee(tc);

    testee.setCost(Cost::fromString("200T"));
    testee.setCost(Cost::fromString("200D"));
    testee.setCost(Cost::fromString("200M"));
    testee.setCost(Cost::fromString("200$"));
    testee.setCost(Cost::fromString("200S"));
    testee.setCost(Cost::fromString("11T 12D 13M 14S 15$"));
    a.checkEqual("01. isValid", testee.isValid(), true);
    a.checkEqual("02. getCost", testee.getCost().get(Cost::Tritanium),  11);
    a.checkEqual("03. getCost", testee.getCost().get(Cost::Duranium),   12);
    a.checkEqual("04. getCost", testee.getCost().get(Cost::Molybdenum), 13);
    a.checkEqual("05. getCost", testee.getCost().get(Cost::Supplies),   14);
    a.checkEqual("06. getCost", testee.getCost().get(Cost::Money),      15);

    a.checkEqual("11. getRemainingAmount", testee.getRemainingAmount(Element::Tritanium),  39);
    a.checkEqual("12. getRemainingAmount", testee.getRemainingAmount(Element::Duranium),   38);
    a.checkEqual("13. getRemainingAmount", testee.getRemainingAmount(Element::Molybdenum), 37);
    a.checkEqual("14. getRemainingAmount", testee.getRemainingAmount(Element::Supplies),   36);
    a.checkEqual("15. getRemainingAmount", testee.getRemainingAmount(Element::Money),      35);

    a.checkEqual("21. getMissingAmount", testee.getMissingAmount(Element::Tritanium),  0);
    a.checkEqual("22. getMissingAmount", testee.getMissingAmount(Element::Duranium),   0);
    a.checkEqual("23. getMissingAmount", testee.getMissingAmount(Element::Molybdenum), 0);
    a.checkEqual("24. getMissingAmount", testee.getMissingAmount(Element::Supplies),   0);
    a.checkEqual("25. getMissingAmount", testee.getMissingAmount(Element::Money),      0);
}

/** Test change of underlying data. */
AFL_TEST("game.actions.CargoCostAction:parallel-modification", a)
{
    class ChangingContainer : public game::CargoContainer {
     public:
        ChangingContainer()
            : m_amount(50)
            { }
        virtual String_t getName(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual String_t getInfo1(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual String_t getInfo2(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual Flags_t getFlags() const
            { return Flags_t(); }
        virtual bool canHaveElement(Element::Type /*type*/) const
            { return true; }
        virtual int32_t getMaxAmount(Element::Type /*type*/) const
            { return 1000; }
        virtual int32_t getMinAmount(Element::Type /*type*/) const
            { return 10; }
        virtual int32_t getAmount(Element::Type /*type*/) const
            { return m_amount; }
        virtual void commit()
            { }
        void set(int n)
            { m_amount = n; sig_change.raise(); }
     private:
        int m_amount;
    };
    ChangingContainer cc;
    game::actions::CargoCostAction testee(cc);

    // Set the initial cost
    testee.setCost(Cost::fromString("11T 12D 13M 14S 15$"));
    a.checkEqual("01. isValid", testee.isValid(), true);
    a.checkEqual("02. getRemainingAmount", testee.getRemainingAmount(Element::Tritanium),  39);
    a.checkEqual("03. getRemainingAmount", testee.getRemainingAmount(Element::Duranium),   38);
    a.checkEqual("04. getRemainingAmount", testee.getRemainingAmount(Element::Molybdenum), 37);
    a.checkEqual("05. getRemainingAmount", testee.getRemainingAmount(Element::Supplies),   36);
    a.checkEqual("06. getRemainingAmount", testee.getRemainingAmount(Element::Money),      35);

    // Change amount in container
    cc.set(23);
    a.checkEqual("11. isValid", testee.isValid(), false);
    a.checkEqual("12. getRemainingAmount", testee.getRemainingAmount(Element::Tritanium),  12);
    a.checkEqual("13. getRemainingAmount", testee.getRemainingAmount(Element::Duranium),   11);
    a.checkEqual("14. getRemainingAmount", testee.getRemainingAmount(Element::Molybdenum), 10);
    a.checkEqual("15. getRemainingAmount", testee.getRemainingAmount(Element::Supplies),   9);
    a.checkEqual("16. getRemainingAmount", testee.getRemainingAmount(Element::Money),      8);

    // Make valid again
    cc.set(150);
    a.checkEqual("21. isValid", testee.isValid(), true);
    a.checkEqual("22. getRemainingAmount", testee.getRemainingAmount(Element::Tritanium),  139);
    a.checkEqual("23. getRemainingAmount", testee.getRemainingAmount(Element::Duranium),   138);
    a.checkEqual("24. getRemainingAmount", testee.getRemainingAmount(Element::Molybdenum), 137);
    a.checkEqual("25. getRemainingAmount", testee.getRemainingAmount(Element::Supplies),   136);
    a.checkEqual("26. getRemainingAmount", testee.getRemainingAmount(Element::Money),      135);
}

/** Test setReservedAmount(), basic case. */
AFL_TEST("game.actions.CargoCostAction:setReservedAmount", a)
{
    TestContainer tc;
    game::actions::CargoCostAction testee(tc);

    testee.setReservedAmount(Cost::fromString("7T 8D 9M 10S 11$"));
    testee.setCost(Cost::fromString("11T 12D 13M 14S 15$"));
    a.checkEqual("01. isValid", testee.isValid(), true);
    a.checkEqual("02. getCost", testee.getCost().get(Cost::Tritanium),  11);
    a.checkEqual("03. getCost", testee.getCost().get(Cost::Duranium),   12);
    a.checkEqual("04. getCost", testee.getCost().get(Cost::Molybdenum), 13);
    a.checkEqual("05. getCost", testee.getCost().get(Cost::Supplies),   14);
    a.checkEqual("06. getCost", testee.getCost().get(Cost::Money),      15);

    a.checkEqual("11. getRemainingAmount", testee.getRemainingAmount(Element::Tritanium),  32);
    a.checkEqual("12. getRemainingAmount", testee.getRemainingAmount(Element::Duranium),   30);
    a.checkEqual("13. getRemainingAmount", testee.getRemainingAmount(Element::Molybdenum), 28);
    a.checkEqual("14. getRemainingAmount", testee.getRemainingAmount(Element::Supplies),   26);
    a.checkEqual("15. getRemainingAmount", testee.getRemainingAmount(Element::Money),      24);
    a.checkEqual("16. getRemainingAmount", testee.getRemainingAmountAsCost().toCargoSpecString(), "32T 30D 28M 26S 24$");

    a.checkEqual("21. getMissingAmount", testee.getMissingAmount(Element::Tritanium),  0);
    a.checkEqual("22. getMissingAmount", testee.getMissingAmount(Element::Duranium),   0);
    a.checkEqual("23. getMissingAmount", testee.getMissingAmount(Element::Molybdenum), 0);
    a.checkEqual("24. getMissingAmount", testee.getMissingAmount(Element::Supplies),   0);
    a.checkEqual("25. getMissingAmount", testee.getMissingAmount(Element::Money),      0);
    a.checkEqual("26. getMissingAmount", testee.getMissingAmountAsCost().toCargoSpecString(), "");

    a.checkEqual("31. getMissingAmountAsCost", testee.getAvailableAmountAsCost().toCargoSpecString(), "43T 42D 41M 40S 39$");
}

/** Test setReservedAmount(), money overflows into supplies. */
AFL_TEST("game.actions.CargoCostAction:setReservedAmount:cost-overflow", a)
{
    TestContainer tc;
    game::actions::CargoCostAction testee(tc);

    // We have 50, minimum 10, so 20+25 = 45 will overflow
    testee.setReservedAmount(Cost::fromString("20$"));
    testee.setCost(Cost::fromString("25$"));
    a.checkEqual("01. isValid", testee.isValid(), true);
    a.checkEqual("02. getCost", testee.getCost().get(Cost::Tritanium),  0);
    a.checkEqual("03. getCost", testee.getCost().get(Cost::Duranium),   0);
    a.checkEqual("04. getCost", testee.getCost().get(Cost::Molybdenum), 0);
    a.checkEqual("05. getCost", testee.getCost().get(Cost::Supplies),   0);
    a.checkEqual("06. getCost", testee.getCost().get(Cost::Money),      25);

    a.checkEqual("11. getRemainingAmount", testee.getRemainingAmount(Element::Tritanium),  50);
    a.checkEqual("12. getRemainingAmount", testee.getRemainingAmount(Element::Duranium),   50);
    a.checkEqual("13. getRemainingAmount", testee.getRemainingAmount(Element::Molybdenum), 50);
    a.checkEqual("14. getRemainingAmount", testee.getRemainingAmount(Element::Supplies),   45);
    a.checkEqual("15. getRemainingAmount", testee.getRemainingAmount(Element::Money),      10);

    a.checkEqual("21. getMissingAmount", testee.getMissingAmount(Element::Tritanium),  0);
    a.checkEqual("22. getMissingAmount", testee.getMissingAmount(Element::Duranium),   0);
    a.checkEqual("23. getMissingAmount", testee.getMissingAmount(Element::Molybdenum), 0);
    a.checkEqual("24. getMissingAmount", testee.getMissingAmount(Element::Supplies),   0);
    a.checkEqual("25. getMissingAmount", testee.getMissingAmount(Element::Money),      0);
}

/** Test setReservedAmount(), reservation exceeds available money. */
AFL_TEST("game.actions.CargoCostAction:setReservedAmount:reserve-overflow", a)
{
    TestContainer tc;
    game::actions::CargoCostAction testee(tc);

    // We have 50, minimum 10, reserved 45, so that will already overflow. Spend another 10.
    testee.setReservedAmount(Cost::fromString("45$"));
    testee.setCost(Cost::fromString("10$"));
    a.checkEqual("01. isValid", testee.isValid(), true);
    a.checkEqual("02. getCost", testee.getCost().get(Cost::Tritanium),  0);
    a.checkEqual("03. getCost", testee.getCost().get(Cost::Duranium),   0);
    a.checkEqual("04. getCost", testee.getCost().get(Cost::Molybdenum), 0);
    a.checkEqual("05. getCost", testee.getCost().get(Cost::Supplies),   0);
    a.checkEqual("06. getCost", testee.getCost().get(Cost::Money),      10);

    a.checkEqual("11. getRemainingAmount", testee.getRemainingAmount(Element::Tritanium),  50);
    a.checkEqual("12. getRemainingAmount", testee.getRemainingAmount(Element::Duranium),   50);
    a.checkEqual("13. getRemainingAmount", testee.getRemainingAmount(Element::Molybdenum), 50);
    a.checkEqual("14. getRemainingAmount", testee.getRemainingAmount(Element::Supplies),   35);
    a.checkEqual("15. getRemainingAmount", testee.getRemainingAmount(Element::Money),      10);

    a.checkEqual("21. getMissingAmount", testee.getMissingAmount(Element::Tritanium),  0);
    a.checkEqual("22. getMissingAmount", testee.getMissingAmount(Element::Duranium),   0);
    a.checkEqual("23. getMissingAmount", testee.getMissingAmount(Element::Molybdenum), 0);
    a.checkEqual("24. getMissingAmount", testee.getMissingAmount(Element::Supplies),   0);
    a.checkEqual("25. getMissingAmount", testee.getMissingAmount(Element::Money),      0);
}
