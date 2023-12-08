/**
  *  \file u/t_game_actions_cargotransfer.cpp
  *  \brief Test for game::actions::CargoTransfer
  */

#include "game/actions/cargotransfer.hpp"

#include "t_game_actions.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/cargospec.hpp"
#include "game/exception.hpp"
#include "game/spec/shiplist.hpp"

using game::Element;
using game::CargoSpec;
using game::CargoContainer;

namespace {
    class TestContainer : public game::CargoContainer {
     public:
        TestContainer(CargoSpec& storage, Flags_t flags = Flags_t())
            : m_storage(storage),
              m_flags(flags),
              m_max(100000),
              m_min(0),
              m_elements(game::ElementTypes_t() + Element::Neutronium + Element::Tritanium + Element::Duranium
                         + Element::Molybdenum + Element::Colonists + Element::Supplies + Element::Money)
            { }

        virtual String_t getName(afl::string::Translator& /*tx*/) const
            { return "<Test>"; }
        virtual String_t getInfo1(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual String_t getInfo2(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual Flags_t getFlags() const
            { return m_flags; }
        virtual bool canHaveElement(Element::Type type) const
            { return m_elements.contains(type); }
        virtual int32_t getMaxAmount(Element::Type /*type*/) const
            { return m_max; }
        virtual int32_t getMinAmount(Element::Type /*type*/) const
            { return m_min; }
        virtual int32_t getAmount(Element::Type type) const
            {
                switch (type) {
                 case Element::Neutronium: return m_storage.get(CargoSpec::Neutronium);
                 case Element::Tritanium:  return m_storage.get(CargoSpec::Tritanium);
                 case Element::Duranium:   return m_storage.get(CargoSpec::Duranium);
                 case Element::Molybdenum: return m_storage.get(CargoSpec::Molybdenum);
                 case Element::Colonists:  return m_storage.get(CargoSpec::Colonists);
                 case Element::Supplies:   return m_storage.get(CargoSpec::Supplies);
                 case Element::Money:      return m_storage.get(CargoSpec::Money);
                 default:                  return 0;
                }
            }
        virtual void commit()
            {
                m_storage.add(CargoSpec::Neutronium, getChange(Element::Neutronium));
                m_storage.add(CargoSpec::Tritanium,  getChange(Element::Tritanium));
                m_storage.add(CargoSpec::Duranium,   getChange(Element::Duranium));
                m_storage.add(CargoSpec::Molybdenum, getChange(Element::Molybdenum));
                m_storage.add(CargoSpec::Colonists,  getChange(Element::Colonists));
                m_storage.add(CargoSpec::Supplies,   getChange(Element::Supplies));
                m_storage.add(CargoSpec::Money,      getChange(Element::Money));
            }

        void setMin(int32_t min)
            { m_min = min; }
        void setMax(int32_t max)
            { m_max = max; }
        void setElements(game::ElementTypes_t types)
            { m_elements = types; }
     private:
        CargoSpec& m_storage;
        Flags_t m_flags;
        int32_t m_max;
        int32_t m_min;
        game::ElementTypes_t m_elements;
    };
}


/** Test empty cargo transfer.
    The empty cargo is a valid transaction. */
void
TestGameActionsCargoTransfer::testEmpty()
{
    game::actions::CargoTransfer testee;
    TS_ASSERT_EQUALS(testee.getNumContainers(), 0U);
    TS_ASSERT(testee.get(0) == 0);
    TS_ASSERT(testee.get(9999) == 0);
    TS_ASSERT(testee.isValid());
    TS_ASSERT(!testee.isSupplySaleAllowed());
    TS_ASSERT(!testee.isUnloadAllowed());
    TS_ASSERT_EQUALS(testee.move(Element::Money, 100, 1, 2, false, false), 0);
    TS_ASSERT_THROWS_NOTHING(testee.commit());
}

/** Test normal operation. */
void
TestGameActionsCargoTransfer::testNormal()
{
    CargoSpec a("100TDM 50S 50$", false);
    CargoSpec b("30NTDM", false);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(a));
    testee.addNew(new TestContainer(b, CargoContainer::Flags_t(CargoContainer::SupplySale)));

    // Verify self-description
    TS_ASSERT_EQUALS(testee.getNumContainers(), 2U);
    TS_ASSERT(testee.get(0) != 0);
    TS_ASSERT(testee.get(1) != 0);
    TS_ASSERT(testee.get(2) == 0);
    TS_ASSERT(testee.isSupplySaleAllowed());
    TS_ASSERT(!testee.isUnloadAllowed());
    TS_ASSERT(!testee.get(0)->isOverload());
    TS_ASSERT(!testee.get(1)->isOverload());
    TS_ASSERT(!testee.isOverload());

    // Move stuff around
    // - Fail to move 100N
    TS_ASSERT_EQUALS(testee.move(Element::Neutronium, 100, 1, 0, false, false),   0);
    // - Succeed to move 100N when allowing partially
    TS_ASSERT_EQUALS(testee.move(Element::Neutronium, 100, 1, 0, true,  false),  30);
    // - Move some tritanium
    TS_ASSERT_EQUALS(testee.move(Element::Tritanium,  -10, 1, 0, false, false), -10);
    TS_ASSERT_EQUALS(testee.move(Element::Tritanium,   10, 0, 1, false, false),  10);
    // - Move supplies and sell inbetween
    TS_ASSERT_EQUALS(testee.move(Element::Supplies,    10, 0, 1, true,  true),   10);

    // Underlying objects not yet changed
    TS_ASSERT_EQUALS(a.toCargoSpecString(), "100TDM 50S 50$");
    TS_ASSERT_EQUALS(b.toCargoSpecString(), "30TDM 30N");

    // Commit
    TS_ASSERT_THROWS_NOTHING(testee.commit());

    // Verify
    TS_ASSERT_EQUALS(a.toCargoSpecString(), "30N 80T 100D 100M 40S 50$");
    TS_ASSERT_EQUALS(b.toCargoSpecString(), "50T 30D 30M 10$");
}

/** Test unloading when there is no unload source.
    Unload must fail. */
void
TestGameActionsCargoTransfer::testUnloadNoSource()
{
    CargoSpec p("", true);
    CargoSpec s("100T", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(p, CargoContainer::Flags_t(CargoContainer::UnloadTarget)));
    testee.addNew(new TestContainer(s));

    TS_ASSERT(!testee.isUnloadAllowed());
    TS_ASSERT(!testee.unload(false));
    TS_ASSERT_THROWS_NOTHING(testee.commit());
    TS_ASSERT_EQUALS(p.toCargoSpecString(), "");
    TS_ASSERT_EQUALS(s.toCargoSpecString(), "100T");
}

/** Test unloading when there is no unload target.
    Unload must fail. */
void
TestGameActionsCargoTransfer::testUnloadNoTarget()
{
    CargoSpec p("", true);
    CargoSpec s("100T", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(p));
    testee.addNew(new TestContainer(s, CargoContainer::Flags_t(CargoContainer::UnloadSource)));

    TS_ASSERT(!testee.isUnloadAllowed());
    TS_ASSERT(!testee.unload(false));
    TS_ASSERT_THROWS_NOTHING(testee.commit());
    TS_ASSERT_EQUALS(p.toCargoSpecString(), "");
    TS_ASSERT_EQUALS(s.toCargoSpecString(), "100T");
}

/** Test unloading when there are multiple unload targets.
    Unload must fail. */
void
TestGameActionsCargoTransfer::testUnloadMultipleTarget()
{
    CargoSpec p1("1D", true);
    CargoSpec p2("1M", true);
    CargoSpec s("100T", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(p1, CargoContainer::Flags_t(CargoContainer::UnloadTarget)));
    testee.addNew(new TestContainer(p2, CargoContainer::Flags_t(CargoContainer::UnloadTarget)));
    testee.addNew(new TestContainer(s,  CargoContainer::Flags_t(CargoContainer::UnloadSource)));

    TS_ASSERT(!testee.isUnloadAllowed());
    TS_ASSERT(!testee.unload(false));
    TS_ASSERT_THROWS_NOTHING(testee.commit());
    TS_ASSERT_EQUALS(p1.toCargoSpecString(), "1D");
    TS_ASSERT_EQUALS(p2.toCargoSpecString(), "1M");
    TS_ASSERT_EQUALS(s.toCargoSpecString(), "100T");
}

/** Test unloading, normal case.
    Unload must succeed. */
void
TestGameActionsCargoTransfer::testUnloadNormal()
{
    CargoSpec p("1D", true);
    CargoSpec s1("100T 10M 20N", true);
    CargoSpec s2("100S 10M 50N", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(s1, CargoContainer::Flags_t(CargoContainer::UnloadSource)));
    testee.addNew(new TestContainer(p,  CargoContainer::Flags_t(CargoContainer::UnloadTarget)));
    testee.addNew(new TestContainer(s2, CargoContainer::Flags_t(CargoContainer::UnloadSource)));

    TS_ASSERT(testee.isUnloadAllowed());
    TS_ASSERT(testee.unload(false));
    TS_ASSERT_THROWS_NOTHING(testee.commit());
    TS_ASSERT_EQUALS(p.toCargoSpecString(), "100T 1D 20M 100S");
    TS_ASSERT_EQUALS(s1.toCargoSpecString(), "20N");
    TS_ASSERT_EQUALS(s2.toCargoSpecString(), "50N");
}

/** Test unloading, with supply sale.
    Unload must succeed. */
void
TestGameActionsCargoTransfer::testUnloadSell()
{
    CargoSpec p("1D", true);
    CargoSpec s("100T 50S 30$ 20N", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(s, CargoContainer::Flags_t(CargoContainer::UnloadSource)));
    testee.addNew(new TestContainer(p, CargoContainer::Flags_t(CargoContainer::UnloadTarget) + CargoContainer::SupplySale));

    TS_ASSERT(testee.isUnloadAllowed());
    TS_ASSERT(testee.unload(true));
    TS_ASSERT_THROWS_NOTHING(testee.commit());
    TS_ASSERT_EQUALS(p.toCargoSpecString(), "100T 1D 80$");
    TS_ASSERT_EQUALS(s.toCargoSpecString(), "20N");
}

/** Test with limited room.
    Transfer must honor limited room. */
void
TestGameActionsCargoTransfer::testLimitRoom()
{
    CargoSpec a("100T", true);
    CargoSpec b("50T", true);

    game::actions::CargoTransfer testee;
    TestContainer* ac = new TestContainer(a);
    ac->setMax(110);
    testee.addNew(ac);
    testee.addNew(new TestContainer(b));

    // Complete move fails
    TS_ASSERT_EQUALS(testee.move(Element::Tritanium, 50, 1, 0, false, false), 0);

    // Partial move succeeds
    TS_ASSERT_EQUALS(testee.move(Element::Tritanium, 50, 1, 0, true, false), 10);

    // Verify content of ac
    TS_ASSERT_EQUALS(ac->getChange(Element::Tritanium), 10);

    // Finish
    testee.commit();
    TS_ASSERT_EQUALS(a.get(CargoSpec::Tritanium), 110);
    TS_ASSERT_EQUALS(b.get(CargoSpec::Tritanium), 40);
}

/** Test with limited types.
    Transfer must not move into prohibited types. */
void
TestGameActionsCargoTransfer::testLimitTypes()
{
    CargoSpec a("100T", true);
    CargoSpec b("50TDM", true);

    game::actions::CargoTransfer testee;
    TestContainer* ac = new TestContainer(a);
    ac->setElements(game::ElementTypes_t() + Element::Tritanium);
    testee.addNew(ac);
    testee.addNew(new TestContainer(b));

    // Moving tritanium succeeds
    TS_ASSERT_EQUALS(testee.move(Element::Tritanium,  10, 1, 0, true, false), 10);

    // Moving moly fails, because a cannot hold moly
    TS_ASSERT_EQUALS(testee.move(Element::Molybdenum, 10, 1, 0, true, false),  0);

    // Check result
    TS_ASSERT_THROWS_NOTHING(testee.commit());
    TS_ASSERT_EQUALS(a.toCargoSpecString(), "110T");
    TS_ASSERT_EQUALS(b.toCargoSpecString(), "40T 50D 50M");
}

/** Test supply sale.
    Supply sale must only happen for "forward" transfers, but needs not involve the SupplySale unit. */
void
TestGameActionsCargoTransfer::testSupplySale()
{
    CargoSpec a1("50S", true);
    CargoSpec a2("50S", true);
    CargoSpec b("100T", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(a1));
    testee.addNew(new TestContainer(a2));
    testee.addNew(new TestContainer(b, CargoContainer::Flags_t(CargoContainer::SupplySale)));  // only to enable supply sale

    TS_ASSERT(testee.isSupplySaleAllowed());
    TS_ASSERT_EQUALS(testee.move(Element::Supplies,  10, 0, 1, true, true),  10); // this one sells supplies
    TS_ASSERT_EQUALS(testee.move(Element::Supplies, -10, 1, 0, true, true), -10); // this one doesn't

    TS_ASSERT_THROWS_NOTHING(testee.commit());
    TS_ASSERT_EQUALS(a1.toCargoSpecString(), "30S");
    TS_ASSERT_EQUALS(a2.toCargoSpecString(), "60S 10$");
}

/** Test overload configuration, empty transfer.
    The empty CargoTransfer must be able to store the "isOverload" bit. */
void
TestGameActionsCargoTransfer::testOverloadEmpty()
{
    game::actions::CargoTransfer testee;

    // Initial state
    TS_ASSERT_EQUALS(testee.isOverload(), false);

    // Configure
    testee.setOverload(true);
    TS_ASSERT_EQUALS(testee.isOverload(), true);
}

/** Test overload configuration, configuration before add.
    Containers added afterwards must receive the correct value. */
void
TestGameActionsCargoTransfer::testOverloadBefore()
{
    game::actions::CargoTransfer testee;
    testee.setOverload(true);

    // Add one
    CargoSpec a("100T", true);
    testee.addNew(new TestContainer(a));

    // Check
    TS_ASSERT(testee.get(0) != 0);
    TS_ASSERT(testee.get(0)->isOverload());
}

/** Test overload configuration, configuration after add.
    Containers must receive the correct value. */
void
TestGameActionsCargoTransfer::testOverloadAfter()
{
    game::actions::CargoTransfer testee;

    // Add one
    CargoSpec a("100T", true);
    testee.addNew(new TestContainer(a));
    TS_ASSERT(testee.get(0) != 0);
    TS_ASSERT(!testee.get(0)->isOverload());

    // Configure
    testee.setOverload(true);
    TS_ASSERT(testee.get(0)->isOverload());

    testee.setOverload(false);
    TS_ASSERT(!testee.get(0)->isOverload());
}

/** Test behaviour on temporary container.
    Temporary container can block commit. */
void
TestGameActionsCargoTransfer::testTemporary()
{
    CargoSpec a("100T", true);
    CargoSpec b("100T", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(a, CargoContainer::Flags_t(CargoContainer::Temporary)));
    testee.addNew(new TestContainer(b));

    // Initial state: valid
    TS_ASSERT(testee.isValid());

    // Move stuff into a, making it invalid
    TS_ASSERT_EQUALS(testee.move(Element::Tritanium, 50, 1, 0, false, false), 50);
    TS_ASSERT(!testee.isValid());
    TS_ASSERT_THROWS(testee.commit(), game::Exception);
}

/** Test move(CargoSpec).
    Function must behave as expected. */
void
TestGameActionsCargoTransfer::testCargoSpec()
{
    CargoSpec a("100TDM 10$", true);
    CargoSpec b("100T", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(a, CargoContainer::Flags_t(CargoContainer::SupplySale)));
    testee.addNew(new TestContainer(b, CargoContainer::Flags_t(CargoContainer::SupplySale)));

    // Move
    CargoSpec toMove("40TDM$", true);
    game::spec::ShipList shipList;
    testee.move(toMove, shipList, 0, 1, false);
    TS_ASSERT_EQUALS(toMove.toCargoSpecString(), "30$");

    testee.commit();
    TS_ASSERT_EQUALS(a.toCargoSpecString(), "60TDM");
    TS_ASSERT_EQUALS(b.toCargoSpecString(), "140T 40D 40M 10$");
}

/** Test move(CargoSpec) with supply sale.
    Function must behave as expected. */
void
TestGameActionsCargoTransfer::testCargoSpecSupplySale()
{
    CargoSpec a("100TDM 50S 50$", true);
    CargoSpec b("100T", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(a, CargoContainer::Flags_t(CargoContainer::SupplySale)));
    testee.addNew(new TestContainer(b, CargoContainer::Flags_t(CargoContainer::SupplySale)));

    // Move
    CargoSpec toMove("20S 30$", true);
    game::spec::ShipList shipList;
    testee.move(toMove, shipList, 0, 1, true);
    TS_ASSERT_EQUALS(toMove.toCargoSpecString(), "");

    testee.commit();
    TS_ASSERT_EQUALS(a.toCargoSpecString(), "100TDM 30S 20$");
    TS_ASSERT_EQUALS(b.toCargoSpecString(), "100T 50$");
}

/** Test addHoldSpace(). */
void
TestGameActionsCargoTransfer::testHoldSpace()
{
    afl::string::NullTranslator tx;
    CargoSpec a("100TDM 50S 50$", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(a, CargoContainer::Flags_t()));
    testee.addHoldSpace("Ho ho ho");

    // Examine
    TS_ASSERT_EQUALS(testee.get(0)->getName(tx), "<Test>");
    TS_ASSERT_EQUALS(testee.get(0)->canHaveElement(game::Element::Fighters), false);
    TS_ASSERT_EQUALS(testee.get(1)->getName(tx), "Ho ho ho");
    TS_ASSERT_EQUALS(testee.get(1)->canHaveElement(game::Element::Fighters), true);
    TS_ASSERT_EQUALS(testee.isUnloadAllowed(), false);
    TS_ASSERT_EQUALS(testee.isSupplySaleAllowed(), false);
    TS_ASSERT_EQUALS(testee.isValid(), true);

    // Move stuff into hold space. This makes the transaction invalid.
    TS_ASSERT_EQUALS(testee.move(game::Element::Tritanium, 50, 0, 1, false, false), 50);
    TS_ASSERT_EQUALS(testee.isValid(), false);

    // Move stuff back
    TS_ASSERT_EQUALS(testee.move(game::Element::Tritanium, 10000, 1, 0, true, false), 50);
    TS_ASSERT_EQUALS(testee.isValid(), true);

    // Commit
    TS_ASSERT_THROWS_NOTHING(testee.commit());
    TS_ASSERT_EQUALS(a.toCargoSpecString(), "100TDM 50S 50$");
}

/** Test moveExt(). */
void
TestGameActionsCargoTransfer::testMoveExt()
{
    CargoSpec a("100T", true);
    CargoSpec b("100T", true);
    CargoSpec c("100T", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(a));
    testee.addNew(new TestContainer(b));
    testee.addNew(new TestContainer(c));

    // Move a->b
    testee.moveExt(Element::Tritanium, 555, 0, 1, 2, false);
    TS_ASSERT_EQUALS(testee.get(0)->getEffectiveAmount(Element::Tritanium), 0);
    TS_ASSERT_EQUALS(testee.get(1)->getEffectiveAmount(Element::Tritanium), 200);
    TS_ASSERT_EQUALS(testee.get(2)->getEffectiveAmount(Element::Tritanium), 100);

    // Move a->b again, but now a is empty, so it takes from c
    testee.moveExt(Element::Tritanium, 555, 0, 1, 2, false);
    TS_ASSERT_EQUALS(testee.get(0)->getEffectiveAmount(Element::Tritanium), 0);
    TS_ASSERT_EQUALS(testee.get(1)->getEffectiveAmount(Element::Tritanium), 300);
    TS_ASSERT_EQUALS(testee.get(2)->getEffectiveAmount(Element::Tritanium), 0);
}

/** Test moveExt(), reverse (negative) move. */
void
TestGameActionsCargoTransfer::testMoveExtReverse()
{
    CargoSpec a("100T", true);
    CargoSpec b("100T", true);
    CargoSpec c("100T", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(a));
    testee.addNew(new TestContainer(b));
    testee.addNew(new TestContainer(c));

    // Move a->b reversed
    testee.moveExt(Element::Tritanium, -555, 0, 1, 2, false);
    TS_ASSERT_EQUALS(testee.get(0)->getEffectiveAmount(Element::Tritanium), 200);
    TS_ASSERT_EQUALS(testee.get(1)->getEffectiveAmount(Element::Tritanium), 0);
    TS_ASSERT_EQUALS(testee.get(2)->getEffectiveAmount(Element::Tritanium), 100);

    // Move a->b reversed again; c is not touched because reverse move.
    testee.moveExt(Element::Tritanium, -555, 0, 1, 2, false);
    TS_ASSERT_EQUALS(testee.get(0)->getEffectiveAmount(Element::Tritanium), 200);
    TS_ASSERT_EQUALS(testee.get(1)->getEffectiveAmount(Element::Tritanium), 0);
    TS_ASSERT_EQUALS(testee.get(2)->getEffectiveAmount(Element::Tritanium), 100);
}

/** Test distribute(DistributeEqually). */
void
TestGameActionsCargoTransfer::testDistributeEqually()
{
    CargoSpec a("100T", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(a));
    testee.addNew(new TestContainer(a));    // from
    testee.addNew(new TestContainer(a, CargoContainer::Flags_t(CargoContainer::Temporary)));  // implicitly except
    testee.addNew(new TestContainer(a));    // explicitly excepted
    testee.addNew(new TestContainer(a));

    testee.distribute(Element::Tritanium, 1, 3, game::actions::CargoTransfer::DistributeEqually);

    TS_ASSERT_EQUALS(testee.get(0)->getEffectiveAmount(Element::Tritanium), 150);
    TS_ASSERT_EQUALS(testee.get(1)->getEffectiveAmount(Element::Tritanium), 0);
    TS_ASSERT_EQUALS(testee.get(2)->getEffectiveAmount(Element::Tritanium), 100);
    TS_ASSERT_EQUALS(testee.get(3)->getEffectiveAmount(Element::Tritanium), 100);
    TS_ASSERT_EQUALS(testee.get(4)->getEffectiveAmount(Element::Tritanium), 150);
}

/** Test distribute(DistributeFreeSpace). */
void
TestGameActionsCargoTransfer::testDistributeFreeSpace()
{
    CargoSpec a("100T", true);

    game::actions::CargoTransfer testee;

    TestContainer* c1 = new TestContainer(a);   // 100 free
    c1->setMax(200);
    testee.addNew(c1);

    TestContainer* c2 = new TestContainer(a);   // from
    c2->setMax(200);
    testee.addNew(c2);

    TestContainer* c3 = new TestContainer(a, CargoContainer::Flags_t(CargoContainer::Temporary));
    c3->setMax(200);
    testee.addNew(c3);

    TestContainer* c4 = new TestContainer(a);   // implicitly excepted
    c4->setMax(200);
    testee.addNew(c4);

    TestContainer* c5 = new TestContainer(a);   // 160 free
    c5->setMax(260);
    testee.addNew(c5);

    testee.distribute(Element::Tritanium, 1, 3, game::actions::CargoTransfer::DistributeFreeSpace);

    TS_ASSERT_EQUALS(testee.get(0)->getEffectiveAmount(Element::Tritanium), 120);
    TS_ASSERT_EQUALS(testee.get(1)->getEffectiveAmount(Element::Tritanium), 0);
    TS_ASSERT_EQUALS(testee.get(2)->getEffectiveAmount(Element::Tritanium), 100);
    TS_ASSERT_EQUALS(testee.get(3)->getEffectiveAmount(Element::Tritanium), 100);
    TS_ASSERT_EQUALS(testee.get(4)->getEffectiveAmount(Element::Tritanium), 180);
}

/** Test distribute(DistributeProportionally). */
void
TestGameActionsCargoTransfer::testDistributeProportionally()
{
    CargoSpec a("100T", true);

    game::actions::CargoTransfer testee;

    TestContainer* c1 = new TestContainer(a);   // 180/400 cargo room, should get 135/300 cargo
    c1->setMax(180);
    testee.addNew(c1);

    TestContainer* c2 = new TestContainer(a);   // from
    c2->setMax(200);
    testee.addNew(c2);

    TestContainer* c3 = new TestContainer(a, CargoContainer::Flags_t(CargoContainer::Temporary));
    c3->setMax(200);
    testee.addNew(c3);

    TestContainer* c4 = new TestContainer(a);   // implicitly excepted
    c4->setMax(200);
    testee.addNew(c4);

    TestContainer* c5 = new TestContainer(a);   // 220/400 cargo room, should get 165/300 cargo
    c5->setMax(220);
    testee.addNew(c5);

    testee.distribute(Element::Tritanium, 1, 3, game::actions::CargoTransfer::DistributeProportionally);

    TS_ASSERT_EQUALS(testee.get(0)->getEffectiveAmount(Element::Tritanium), 135);
    TS_ASSERT_EQUALS(testee.get(1)->getEffectiveAmount(Element::Tritanium), 0);
    TS_ASSERT_EQUALS(testee.get(2)->getEffectiveAmount(Element::Tritanium), 100);
    TS_ASSERT_EQUALS(testee.get(3)->getEffectiveAmount(Element::Tritanium), 100);
    TS_ASSERT_EQUALS(testee.get(4)->getEffectiveAmount(Element::Tritanium), 165);
}

/** Test moveAll(). */
void
TestGameActionsCargoTransfer::testMoveAll()
{
    CargoSpec a("100T", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(a));
    testee.addNew(new TestContainer(a));    // to
    testee.addNew(new TestContainer(a));
    testee.addNew(new TestContainer(a));    // explicitly excepted
    testee.addNew(new TestContainer(a));

    testee.moveAll(Element::Tritanium, 1, 3, false);

    TS_ASSERT_EQUALS(testee.get(0)->getEffectiveAmount(Element::Tritanium), 0);
    TS_ASSERT_EQUALS(testee.get(1)->getEffectiveAmount(Element::Tritanium), 400);
    TS_ASSERT_EQUALS(testee.get(2)->getEffectiveAmount(Element::Tritanium), 0);
    TS_ASSERT_EQUALS(testee.get(3)->getEffectiveAmount(Element::Tritanium), 100);
    TS_ASSERT_EQUALS(testee.get(4)->getEffectiveAmount(Element::Tritanium), 0);
}

