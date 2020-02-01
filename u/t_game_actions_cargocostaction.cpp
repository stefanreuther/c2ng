/**
  *  \file u/t_game_actions_cargocostaction.cpp
  *  \brief Test for game::actions::CargoCostAction
  */

#include "game/actions/cargocostaction.hpp"

#include "t_game_actions.hpp"

using game::spec::Cost;
using game::Element;

namespace {
    class TestContainer : public game::CargoContainer {
     public:
        virtual String_t getName(afl::string::Translator& /*tx*/) const
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
void
TestGameActionsCargoCostAction::testNormal()
{
    TestContainer tc;
    game::actions::CargoCostAction testee(tc);

    testee.setCost(Cost::fromString("11T 12D 13M 14S 15$"));
    TS_ASSERT_EQUALS(testee.isValid(), true);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Tritanium),  11);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Duranium),   12);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Molybdenum), 13);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Supplies),   14);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Money),      15);

    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Tritanium),  39);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Duranium),   38);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Molybdenum), 37);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Supplies),   36);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Money),      35);
    TS_ASSERT_EQUALS(testee.getRemainingAmountAsCost().toCargoSpecString(), "39T 38D 37M 36S 35$");

    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Tritanium),  0);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Duranium),   0);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Molybdenum), 0);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Supplies),   0);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Money),      0);
    TS_ASSERT_EQUALS(testee.getMissingAmountAsCost().toCargoSpecString(), "");

    TS_ASSERT_EQUALS(testee.getAvailableAmountAsCost().toCargoSpecString(), "50TDM 50S 50$");
}

/** Test a missing mineral. */
void
TestGameActionsCargoCostAction::testMissingMineral()
{
    TestContainer tc;
    game::actions::CargoCostAction testee(tc);

    // We have 50, minimum 10, so 45 will overflow
    testee.setCost(Cost::fromString("45T"));
    TS_ASSERT_EQUALS(testee.isValid(), false);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Tritanium),  45);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Duranium),   0);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Molybdenum), 0);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Supplies),   0);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Money),      0);

    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Tritanium),   5);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Duranium),   50);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Molybdenum), 50);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Supplies),   50);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Money),      50);
    TS_ASSERT_EQUALS(testee.getRemainingAmountAsCost().toCargoSpecString(), "5T 50D 50M 50S 50$");

    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Tritanium),  5);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Duranium),   0);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Molybdenum), 0);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Supplies),   0);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Money),      0);
    TS_ASSERT_EQUALS(testee.getMissingAmountAsCost().toCargoSpecString(), "5T");
}

/** Test missing money. Will be compensated by selling supplies. */
void
TestGameActionsCargoCostAction::testMissingMoney()
{
    TestContainer tc;
    game::actions::CargoCostAction testee(tc);

    // We have 50, minimum 10, so 45 will overflow
    testee.setCost(Cost::fromString("45$"));
    TS_ASSERT_EQUALS(testee.isValid(), true);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Tritanium),  0);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Duranium),   0);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Molybdenum), 0);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Supplies),   0);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Money),      45);

    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Tritanium),  50);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Duranium),   50);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Molybdenum), 50);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Supplies),   45);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Money),      10);

    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Tritanium),  0);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Duranium),   0);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Molybdenum), 0);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Supplies),   0);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Money),      0);
}

/** Test missing supplies. */
void
TestGameActionsCargoCostAction::testMissingSupplies()
{
    TestContainer tc;
    game::actions::CargoCostAction testee(tc);

    // We have 50, minimum 10, so 45 will overflow
    testee.setCost(Cost::fromString("45S"));
    TS_ASSERT_EQUALS(testee.isValid(), false);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Tritanium),  0);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Duranium),   0);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Molybdenum), 0);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Supplies),   45);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Money),      0);

    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Tritanium),  50);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Duranium),   50);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Molybdenum), 50);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Supplies),   5);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Money),      50);

    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Tritanium),  0);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Duranium),   0);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Molybdenum), 0);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Supplies),   5);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Money),      0);
}

/** Test missing money, more than can be compensated. */
void
TestGameActionsCargoCostAction::testMissingLotsOfMoney()
{
    TestContainer tc;
    game::actions::CargoCostAction testee(tc);

    // Cost is 300$; we can spend 40$+40S = 80$.
    testee.setCost(Cost::fromString("300$"));
    TS_ASSERT_EQUALS(testee.isValid(), false);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Tritanium),  0);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Duranium),   0);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Molybdenum), 0);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Supplies),   0);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Money),      300);

    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Tritanium),  50);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Duranium),   50);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Molybdenum), 50);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Supplies),   -210);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Money),      10);
    TS_ASSERT_EQUALS(testee.getRemainingAmountAsCost().toCargoSpecString(), "50TDM -210S 10$");

    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Tritanium),  0);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Duranium),   0);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Molybdenum), 0);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Supplies),   220);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Money),      0);
    TS_ASSERT_EQUALS(testee.getMissingAmountAsCost().toCargoSpecString(), "220S");
}

/** Test multiple modifications.
    Since we're updating the cost incrementally, this might uncover problems. */
void
TestGameActionsCargoCostAction::testMultiModification()
{
    TestContainer tc;
    game::actions::CargoCostAction testee(tc);

    testee.setCost(Cost::fromString("200T"));
    testee.setCost(Cost::fromString("200D"));
    testee.setCost(Cost::fromString("200M"));
    testee.setCost(Cost::fromString("200$"));
    testee.setCost(Cost::fromString("200S"));
    testee.setCost(Cost::fromString("11T 12D 13M 14S 15$"));
    TS_ASSERT_EQUALS(testee.isValid(), true);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Tritanium),  11);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Duranium),   12);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Molybdenum), 13);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Supplies),   14);
    TS_ASSERT_EQUALS(testee.getCost().get(Cost::Money),      15);

    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Tritanium),  39);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Duranium),   38);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Molybdenum), 37);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Supplies),   36);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Money),      35);

    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Tritanium),  0);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Duranium),   0);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Molybdenum), 0);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Supplies),   0);
    TS_ASSERT_EQUALS(testee.getMissingAmount(Element::Money),      0);
}

/** Test change of underlying data. */
void
TestGameActionsCargoCostAction::testUnderlayingChange()
{
    class ChangingContainer : public game::CargoContainer {
     public:
        ChangingContainer()
            : m_amount(50)
            { }
        virtual String_t getName(afl::string::Translator& /*tx*/) const
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
    TS_ASSERT_EQUALS(testee.isValid(), true);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Tritanium),  39);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Duranium),   38);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Molybdenum), 37);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Supplies),   36);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Money),      35);

    // Change amount in container
    cc.set(23);
    TS_ASSERT_EQUALS(testee.isValid(), false);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Tritanium),  12);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Duranium),   11);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Molybdenum), 10);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Supplies),   9);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Money),      8);

    // Make valid again
    cc.set(150);
    TS_ASSERT_EQUALS(testee.isValid(), true);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Tritanium),  139);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Duranium),   138);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Molybdenum), 137);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Supplies),   136);
    TS_ASSERT_EQUALS(testee.getRemainingAmount(Element::Money),      135);
}

