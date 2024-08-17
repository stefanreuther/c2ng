/**
  *  \file test/game/actions/cargotransfertest.cpp
  *  \brief Test for game::actions::CargoTransfer
  */

#include "game/actions/cargotransfer.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/cargospec.hpp"
#include "game/exception.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/shiplist.hpp"

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
AFL_TEST("game.actions.CargoTransfer:empty", a)
{
    game::actions::CargoTransfer testee;
    a.checkEqual("01. getNumContainers", testee.getNumContainers(), 0U);
    a.checkNull("02. get", testee.get(0));
    a.checkNull("03. get", testee.get(9999));
    a.check("04. isValid", testee.isValid());
    a.check("05. isSupplySaleAllowed", !testee.isSupplySaleAllowed());
    a.check("06.  isUnloadAllowed", !testee.isUnloadAllowed());
    a.checkEqual("07. move", testee.move(Element::Money, 100, 1, 2, false, false), 0);
    AFL_CHECK_SUCCEEDS(a("08. commit"), testee.commit());
}

/** Test normal operation. */
AFL_TEST("game.actions.CargoTransfer:normal", a)
{
    CargoSpec csa("100TDM 50S 50$", false);
    CargoSpec csb("30NTDM", false);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(csa));
    testee.addNew(new TestContainer(csb, CargoContainer::Flags_t(CargoContainer::SupplySale)));

    // Verify self-description
    a.checkEqual("01. getNumContainers", testee.getNumContainers(), 2U);
    a.checkNonNull("02. get", testee.get(0));
    a.checkNonNull("03. get", testee.get(1));
    a.checkNull("04. get", testee.get(2));
    a.check("05. isSupplySaleAllowed", testee.isSupplySaleAllowed());
    a.check("06. isUnloadAllowed", !testee.isUnloadAllowed());
    a.check("07. isOverload", !testee.get(0)->isOverload());
    a.check("08. isOverload", !testee.get(1)->isOverload());
    a.check("09. isOverload", !testee.isOverload());

    // Move stuff around
    // - Fail to move 100N
    a.checkEqual("11. move", testee.move(Element::Neutronium, 100, 1, 0, false, false),   0);
    // - Succeed to move 100N when allowing partially
    a.checkEqual("12. move", testee.move(Element::Neutronium, 100, 1, 0, true,  false),  30);
    // - Move some tritanium
    a.checkEqual("13. move", testee.move(Element::Tritanium,  -10, 1, 0, false, false), -10);
    a.checkEqual("14. move", testee.move(Element::Tritanium,   10, 0, 1, false, false),  10);
    // - Move supplies and sell inbetween
    a.checkEqual("15. move", testee.move(Element::Supplies,    10, 0, 1, true,  true),   10);

    // Underlying objects not yet changed
    a.checkEqual("21. cargo a", csa.toCargoSpecString(), "100TDM 50S 50$");
    a.checkEqual("22. cargo b", csb.toCargoSpecString(), "30TDM 30N");

    // Commit
    AFL_CHECK_SUCCEEDS(a("31. commit"), testee.commit());

    // Verify
    a.checkEqual("41. cargo a", csa.toCargoSpecString(), "30N 80T 100D 100M 40S 50$");
    a.checkEqual("42. cargo b", csb.toCargoSpecString(), "50T 30D 30M 10$");
}

/** Test unloading when there is no unload source.
    Unload must fail. */
AFL_TEST("game.actions.CargoTransfer:unload:no-source", a)
{
    CargoSpec p("", true);
    CargoSpec s("100T", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(p, CargoContainer::Flags_t(CargoContainer::UnloadTarget)));
    testee.addNew(new TestContainer(s));

    a.check("01. isUnloadAllowed", !testee.isUnloadAllowed());
    a.check("02. unload", !testee.unload(false));
    AFL_CHECK_SUCCEEDS(a("03. commit"), testee.commit());
    a.checkEqual("04. cargo p", p.toCargoSpecString(), "");
    a.checkEqual("05. cargo s", s.toCargoSpecString(), "100T");
}

/** Test unloading when there is no unload target.
    Unload must fail. */
AFL_TEST("game.actions.CargoTransfer:unload:no-target", a)
{
    CargoSpec p("", true);
    CargoSpec s("100T", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(p));
    testee.addNew(new TestContainer(s, CargoContainer::Flags_t(CargoContainer::UnloadSource)));

    a.check("01. isUnloadAllowed", !testee.isUnloadAllowed());
    a.check("02. unload", !testee.unload(false));
    AFL_CHECK_SUCCEEDS(a("03. commit"), testee.commit());
    a.checkEqual("04. cargo p", p.toCargoSpecString(), "");
    a.checkEqual("05. cargo s", s.toCargoSpecString(), "100T");
}

/** Test unloading when there are multiple unload targets.
    Unload must fail. */
AFL_TEST("game.actions.CargoTransfer:unload:multiple-targets", a)
{
    CargoSpec p1("1D", true);
    CargoSpec p2("1M", true);
    CargoSpec s("100T", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(p1, CargoContainer::Flags_t(CargoContainer::UnloadTarget)));
    testee.addNew(new TestContainer(p2, CargoContainer::Flags_t(CargoContainer::UnloadTarget)));
    testee.addNew(new TestContainer(s,  CargoContainer::Flags_t(CargoContainer::UnloadSource)));

    a.check("01. isUnloadAllowed", !testee.isUnloadAllowed());
    a.check("02. unload", !testee.unload(false));
    AFL_CHECK_SUCCEEDS(a("03. commit"), testee.commit());
    a.checkEqual("04. cargo p1", p1.toCargoSpecString(), "1D");
    a.checkEqual("05. cargo p2", p2.toCargoSpecString(), "1M");
    a.checkEqual("06. cargo s", s.toCargoSpecString(), "100T");
}

/** Test unloading, normal case.
    Unload must succeed. */
AFL_TEST("game.actions.CargoTransfer:unload:normal", a)
{
    CargoSpec p("1D", true);
    CargoSpec s1("100T 10M 20N", true);
    CargoSpec s2("100S 10M 50N", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(s1, CargoContainer::Flags_t(CargoContainer::UnloadSource)));
    testee.addNew(new TestContainer(p,  CargoContainer::Flags_t(CargoContainer::UnloadTarget)));
    testee.addNew(new TestContainer(s2, CargoContainer::Flags_t(CargoContainer::UnloadSource)));

    a.check("01. isUnloadAllowed", testee.isUnloadAllowed());
    a.check("02. unload", testee.unload(false));
    AFL_CHECK_SUCCEEDS(a("03. commit"), testee.commit());
    a.checkEqual("04. cargo p", p.toCargoSpecString(), "100T 1D 20M 100S");
    a.checkEqual("05. cargo s1", s1.toCargoSpecString(), "20N");
    a.checkEqual("06. cargo s2", s2.toCargoSpecString(), "50N");
}

/** Test unloading, with supply sale.
    Unload must succeed. */
AFL_TEST("game.actions.CargoTransfer:unload:supply-sale", a)
{
    CargoSpec p("1D", true);
    CargoSpec s("100T 50S 30$ 20N", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(s, CargoContainer::Flags_t(CargoContainer::UnloadSource)));
    testee.addNew(new TestContainer(p, CargoContainer::Flags_t(CargoContainer::UnloadTarget) + CargoContainer::SupplySale));

    a.check("01. isUnloadAllowed", testee.isUnloadAllowed());
    a.check("02. unload", testee.unload(true));
    AFL_CHECK_SUCCEEDS(a("03. commit"), testee.commit());
    a.checkEqual("04. cargo p", p.toCargoSpecString(), "100T 1D 80$");
    a.checkEqual("05. cargo s", s.toCargoSpecString(), "20N");
}

/** Test moving torpedoes. */
AFL_TEST("game.actions.CargoTransfer:cargospec-torps", a)
{
    class TorpContainer : public game::CargoContainer {
     public:
        virtual String_t getName(afl::string::Translator& /*tx*/) const
            { return "<Test>"; }
        virtual String_t getInfo1(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual String_t getInfo2(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual Flags_t getFlags() const
            { return Flags_t(); }
        virtual bool canHaveElement(Element::Type type) const
            { return type == Element::fromTorpedoType(10); }
        virtual int32_t getMaxAmount(Element::Type /*type*/) const
            { return 1000; }
        virtual int32_t getMinAmount(Element::Type /*type*/) const
            { return 0; }
        virtual int32_t getAmount(Element::Type /*type*/) const
            { return 100; }
        virtual void commit()
            { }
    };

    game::actions::CargoTransfer testee;
    testee.addNew(new TorpContainer());
    testee.addNew(new TorpContainer());

    game::spec::ShipList shipList;
    game::test::initStandardTorpedoes(shipList);

    CargoSpec cs("10W", false);
    testee.move(cs, shipList, 0, 1, false);
    a.check("01. empty", cs.isZero());
}

/** Test with limited room.
    Transfer must honor limited room. */
AFL_TEST("game.actions.CargoTransfer:limit:room", a)
{
    CargoSpec csa("100T", true);
    CargoSpec csb("50T", true);

    game::actions::CargoTransfer testee;
    TestContainer* ac = new TestContainer(csa);
    ac->setMax(110);
    testee.addNew(ac);
    testee.addNew(new TestContainer(csb));

    // Complete move fails
    a.checkEqual("01. move", testee.move(Element::Tritanium, 50, 1, 0, false, false), 0);

    // Partial move succeeds
    a.checkEqual("11. move", testee.move(Element::Tritanium, 50, 1, 0, true, false), 10);

    // Verify content of ac
    a.checkEqual("21. getChange", ac->getChange(Element::Tritanium), 10);

    // Finish
    testee.commit();
    a.checkEqual("31. cargo a", csa.get(CargoSpec::Tritanium), 110);
    a.checkEqual("32. cargo b", csb.get(CargoSpec::Tritanium), 40);
}

/** Test with limited types.
    Transfer must not move into prohibited types. */
AFL_TEST("game.actions.CargoTransfer:limit:types", a)
{
    CargoSpec csa("100T", true);
    CargoSpec csb("50TDM", true);

    game::actions::CargoTransfer testee;
    TestContainer* ac = new TestContainer(csa);
    ac->setElements(game::ElementTypes_t() + Element::Tritanium);
    testee.addNew(ac);
    testee.addNew(new TestContainer(csb));

    // Moving tritanium succeeds
    a.checkEqual("01. move", testee.move(Element::Tritanium,  10, 1, 0, true, false), 10);

    // Moving moly fails, because a cannot hold moly
    a.checkEqual("11. move", testee.move(Element::Molybdenum, 10, 1, 0, true, false),  0);

    // Check result
    AFL_CHECK_SUCCEEDS(a("21. commit"), testee.commit());
    a.checkEqual("22. cargo a", csa.toCargoSpecString(), "110T");
    a.checkEqual("23. cargo b", csb.toCargoSpecString(), "40T 50D 50M");
}

/** Test supply sale.
    Supply sale must only happen for "forward" transfers, but needs not involve the SupplySale unit. */
AFL_TEST("game.actions.CargoTransfer:supply-sale", a)
{
    CargoSpec csa1("50S", true);
    CargoSpec csa2("50S", true);
    CargoSpec csb("100T", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(csa1));
    testee.addNew(new TestContainer(csa2));
    testee.addNew(new TestContainer(csb, CargoContainer::Flags_t(CargoContainer::SupplySale)));  // only to enable supply sale

    a.check("01. isSupplySaleAllowed", testee.isSupplySaleAllowed());
    a.checkEqual("02. move", testee.move(Element::Supplies,  10, 0, 1, true, true),  10); // this one sells supplies
    a.checkEqual("03. move", testee.move(Element::Supplies, -10, 1, 0, true, true), -10); // this one doesn't

    AFL_CHECK_SUCCEEDS(a("11. commit"), testee.commit());
    a.checkEqual("12. cargo a1", csa1.toCargoSpecString(), "30S");
    a.checkEqual("13. cargo a2", csa2.toCargoSpecString(), "60S 10$");
}

/** Test overload configuration, empty transfer.
    The empty CargoTransfer must be able to store the "isOverload" bit. */
AFL_TEST("game.actions.CargoTransfer:overload:empty", a)
{
    game::actions::CargoTransfer testee;

    // Initial state
    a.checkEqual("01. isOverload", testee.isOverload(), false);

    // Configure
    testee.setOverload(true);
    a.checkEqual("11. isOverload", testee.isOverload(), true);
}

/** Test overload configuration, configuration before add.
    Containers added afterwards must receive the correct value. */
AFL_TEST("game.actions.CargoTransfer:overload:before-add", a)
{
    game::actions::CargoTransfer testee;
    testee.setOverload(true);

    // Add one
    CargoSpec csa("100T", true);
    testee.addNew(new TestContainer(csa));

    // Check
    a.checkNonNull("01. get", testee.get(0));
    a.check("02. isOverload", testee.get(0)->isOverload());
}

/** Test overload configuration, configuration after add.
    Containers must receive the correct value. */
AFL_TEST("game.actions.CargoTransfer:overload:after-add", a)
{
    game::actions::CargoTransfer testee;

    // Add one
    CargoSpec csa("100T", true);
    testee.addNew(new TestContainer(csa));
    a.checkNonNull("01. get", testee.get(0));
    a.check("02. isOverload", !testee.get(0)->isOverload());

    // Configure
    testee.setOverload(true);
    a.check("11. isOverload", testee.get(0)->isOverload());

    testee.setOverload(false);
    a.check("21. isOverload", !testee.get(0)->isOverload());
}

/** Test behaviour on temporary container.
    Temporary container can block commit. */
AFL_TEST("game.actions.CargoTransfer:temp", a)
{
    CargoSpec csa("100T", true);
    CargoSpec csb("100T", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(csa, CargoContainer::Flags_t(CargoContainer::Temporary)));
    testee.addNew(new TestContainer(csb));

    // Initial state: valid
    a.check("01. isValid", testee.isValid());

    // Move stuff into a, making it invalid
    a.checkEqual("11. move", testee.move(Element::Tritanium, 50, 1, 0, false, false), 50);
    a.check("12. isValid", !testee.isValid());
    AFL_CHECK_THROWS(a("13. commit"), testee.commit(), game::Exception);
}

/** Test move(CargoSpec).
    Function must behave as expected. */
AFL_TEST("game.actions.CargoTransfer:move-cargospec", a)
{
    CargoSpec csa("100TDM 10$", true);
    CargoSpec csb("100T", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(csa, CargoContainer::Flags_t(CargoContainer::SupplySale)));
    testee.addNew(new TestContainer(csb, CargoContainer::Flags_t(CargoContainer::SupplySale)));

    // Move
    CargoSpec toMove("40TDM$", true);
    game::spec::ShipList shipList;
    testee.move(toMove, shipList, 0, 1, false);
    a.checkEqual("01. result", toMove.toCargoSpecString(), "30$");

    testee.commit();
    a.checkEqual("11. cargo a", csa.toCargoSpecString(), "60TDM");
    a.checkEqual("12. cargo b", csb.toCargoSpecString(), "140T 40D 40M 10$");
}

/** Test move(CargoSpec) with supply sale.
    Function must behave as expected. */
AFL_TEST("game.actions.CargoTransfer:move-cargospec:supply-sale", a)
{
    CargoSpec csa("100TDM 50S 50$", true);
    CargoSpec csb("100T", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(csa, CargoContainer::Flags_t(CargoContainer::SupplySale)));
    testee.addNew(new TestContainer(csb, CargoContainer::Flags_t(CargoContainer::SupplySale)));

    // Move
    CargoSpec toMove("20S 30$", true);
    game::spec::ShipList shipList;
    testee.move(toMove, shipList, 0, 1, true);
    a.checkEqual("01. result", toMove.toCargoSpecString(), "");

    testee.commit();
    a.checkEqual("11. cargo a", csa.toCargoSpecString(), "100TDM 30S 20$");
    a.checkEqual("12. cargo b", csb.toCargoSpecString(), "100T 50$");
}

/** Test addHoldSpace(). */
AFL_TEST("game.actions.CargoTransfer:addHoldSpace", a)
{
    afl::string::NullTranslator tx;
    CargoSpec csa("100TDM 50S 50$", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(csa, CargoContainer::Flags_t()));
    testee.addHoldSpace("Ho ho ho");

    // Examine
    a.checkEqual("01. name 0", testee.get(0)->getName(tx), "<Test>");
    a.checkEqual("02. ele 0", testee.get(0)->canHaveElement(game::Element::Fighters), false);
    a.checkEqual("03. name 1", testee.get(1)->getName(tx), "Ho ho ho");
    a.checkEqual("04. ele 1", testee.get(1)->canHaveElement(game::Element::Fighters), true);
    a.checkEqual("05. isUnloadAllowed", testee.isUnloadAllowed(), false);
    a.checkEqual("06. isSupplySaleAllowed", testee.isSupplySaleAllowed(), false);
    a.checkEqual("07. isValid", testee.isValid(), true);

    // Move stuff into hold space. This makes the transaction invalid.
    a.checkEqual("11. move", testee.move(game::Element::Tritanium, 50, 0, 1, false, false), 50);
    a.checkEqual("12. isValid", testee.isValid(), false);

    // Move stuff back
    a.checkEqual("21. move", testee.move(game::Element::Tritanium, 10000, 1, 0, true, false), 50);
    a.checkEqual("22. isValid", testee.isValid(), true);

    // Commit
    AFL_CHECK_SUCCEEDS(a("31. commit"), testee.commit());
    a.checkEqual("32. cargo a", csa.toCargoSpecString(), "100TDM 50S 50$");
}

/** Test moveExt(). */
AFL_TEST("game.actions.CargoTransfer:moveExt", a)
{
    CargoSpec csa("100T", true);
    CargoSpec csb("100T", true);
    CargoSpec csc("100T", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(csa));
    testee.addNew(new TestContainer(csb));
    testee.addNew(new TestContainer(csc));

    // Move a->b
    testee.moveExt(Element::Tritanium, 555, 0, 1, 2, false);
    a.checkEqual("01. item 0", testee.get(0)->getEffectiveAmount(Element::Tritanium), 0);
    a.checkEqual("02. item 1", testee.get(1)->getEffectiveAmount(Element::Tritanium), 200);
    a.checkEqual("03. item 2", testee.get(2)->getEffectiveAmount(Element::Tritanium), 100);

    // Move a->b again, but now a is empty, so it takes from c
    testee.moveExt(Element::Tritanium, 555, 0, 1, 2, false);
    a.checkEqual("11. item 0", testee.get(0)->getEffectiveAmount(Element::Tritanium), 0);
    a.checkEqual("12. item 1", testee.get(1)->getEffectiveAmount(Element::Tritanium), 300);
    a.checkEqual("13. item 2", testee.get(2)->getEffectiveAmount(Element::Tritanium), 0);
}

/** Test moveExt(), reverse (negative) move. */
AFL_TEST("game.actions.CargoTransfer:moveExt:reverse", a)
{
    CargoSpec csa("100T", true);
    CargoSpec csb("100T", true);
    CargoSpec csc("100T", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(csa));
    testee.addNew(new TestContainer(csb));
    testee.addNew(new TestContainer(csc));

    // Move a->b reversed
    testee.moveExt(Element::Tritanium, -555, 0, 1, 2, false);
    a.checkEqual("01. item 0", testee.get(0)->getEffectiveAmount(Element::Tritanium), 200);
    a.checkEqual("02. item 1", testee.get(1)->getEffectiveAmount(Element::Tritanium), 0);
    a.checkEqual("03. item 2", testee.get(2)->getEffectiveAmount(Element::Tritanium), 100);

    // Move a->b reversed again; c is not touched because reverse move.
    testee.moveExt(Element::Tritanium, -555, 0, 1, 2, false);
    a.checkEqual("11. item 0", testee.get(0)->getEffectiveAmount(Element::Tritanium), 200);
    a.checkEqual("12. item 1", testee.get(1)->getEffectiveAmount(Element::Tritanium), 0);
    a.checkEqual("13. item 2", testee.get(2)->getEffectiveAmount(Element::Tritanium), 100);
}

/** Test distribute(DistributeEqually). */
AFL_TEST("game.actions.CargoTransfer:DistributeEqually", a)
{
    CargoSpec csa("100T", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(csa));
    testee.addNew(new TestContainer(csa));    // from
    testee.addNew(new TestContainer(csa, CargoContainer::Flags_t(CargoContainer::Temporary)));  // implicitly except
    testee.addNew(new TestContainer(csa));    // explicitly excepted
    testee.addNew(new TestContainer(csa));

    testee.distribute(Element::Tritanium, 1, 3, game::actions::CargoTransfer::DistributeEqually);

    a.checkEqual("01. item 0", testee.get(0)->getEffectiveAmount(Element::Tritanium), 150);
    a.checkEqual("02. item 1", testee.get(1)->getEffectiveAmount(Element::Tritanium), 0);
    a.checkEqual("03. item 2", testee.get(2)->getEffectiveAmount(Element::Tritanium), 100);
    a.checkEqual("04. item 3", testee.get(3)->getEffectiveAmount(Element::Tritanium), 100);
    a.checkEqual("05. item 4", testee.get(4)->getEffectiveAmount(Element::Tritanium), 150);
}

/** Test distribute(DistributeFreeSpace). */
AFL_TEST("game.actions.CargoTransfer:DistributeFreeSpace", a)
{
    CargoSpec csa("100T", true);

    game::actions::CargoTransfer testee;

    TestContainer* c1 = new TestContainer(csa);   // 100 free
    c1->setMax(200);
    testee.addNew(c1);

    TestContainer* c2 = new TestContainer(csa);   // from
    c2->setMax(200);
    testee.addNew(c2);

    TestContainer* c3 = new TestContainer(csa, CargoContainer::Flags_t(CargoContainer::Temporary));
    c3->setMax(200);
    testee.addNew(c3);

    TestContainer* c4 = new TestContainer(csa);   // implicitly excepted
    c4->setMax(200);
    testee.addNew(c4);

    TestContainer* c5 = new TestContainer(csa);   // 160 free
    c5->setMax(260);
    testee.addNew(c5);

    testee.distribute(Element::Tritanium, 1, 3, game::actions::CargoTransfer::DistributeFreeSpace);

    a.checkEqual("01. item 0", testee.get(0)->getEffectiveAmount(Element::Tritanium), 120);
    a.checkEqual("02. item 1", testee.get(1)->getEffectiveAmount(Element::Tritanium), 0);
    a.checkEqual("03. item 2", testee.get(2)->getEffectiveAmount(Element::Tritanium), 100);
    a.checkEqual("04. item 3", testee.get(3)->getEffectiveAmount(Element::Tritanium), 100);
    a.checkEqual("05. item 4", testee.get(4)->getEffectiveAmount(Element::Tritanium), 180);
}

/** Test distribute(DistributeProportionally). */
AFL_TEST("game.actions.CargoTransfer:DistributeProportionally", a)
{
    CargoSpec csa("100T", true);

    game::actions::CargoTransfer testee;

    TestContainer* c1 = new TestContainer(csa);   // 180/400 cargo room, should get 135/300 cargo
    c1->setMax(180);
    testee.addNew(c1);

    TestContainer* c2 = new TestContainer(csa);   // from
    c2->setMax(200);
    testee.addNew(c2);

    TestContainer* c3 = new TestContainer(csa, CargoContainer::Flags_t(CargoContainer::Temporary));
    c3->setMax(200);
    testee.addNew(c3);

    TestContainer* c4 = new TestContainer(csa);   // implicitly excepted
    c4->setMax(200);
    testee.addNew(c4);

    TestContainer* c5 = new TestContainer(csa);   // 220/400 cargo room, should get 165/300 cargo
    c5->setMax(220);
    testee.addNew(c5);

    testee.distribute(Element::Tritanium, 1, 3, game::actions::CargoTransfer::DistributeProportionally);

    a.checkEqual("01. item 0", testee.get(0)->getEffectiveAmount(Element::Tritanium), 135);
    a.checkEqual("02. item 1", testee.get(1)->getEffectiveAmount(Element::Tritanium), 0);
    a.checkEqual("03. item 2", testee.get(2)->getEffectiveAmount(Element::Tritanium), 100);
    a.checkEqual("04. item 3", testee.get(3)->getEffectiveAmount(Element::Tritanium), 100);
    a.checkEqual("05. item 4", testee.get(4)->getEffectiveAmount(Element::Tritanium), 165);
}

/** Test moveAll(). */
AFL_TEST("game.actions.CargoTransfer:moveAll", a)
{
    CargoSpec csa("100T", true);

    game::actions::CargoTransfer testee;
    testee.addNew(new TestContainer(csa));
    testee.addNew(new TestContainer(csa));    // to
    testee.addNew(new TestContainer(csa));
    testee.addNew(new TestContainer(csa));    // explicitly excepted
    testee.addNew(new TestContainer(csa));

    testee.moveAll(Element::Tritanium, 1, 3, false);

    a.checkEqual("01. item 0", testee.get(0)->getEffectiveAmount(Element::Tritanium), 0);
    a.checkEqual("02. item 1", testee.get(1)->getEffectiveAmount(Element::Tritanium), 400);
    a.checkEqual("03. item 2", testee.get(2)->getEffectiveAmount(Element::Tritanium), 0);
    a.checkEqual("04. item 3", testee.get(3)->getEffectiveAmount(Element::Tritanium), 100);
    a.checkEqual("05. item 4", testee.get(4)->getEffectiveAmount(Element::Tritanium), 0);
}
