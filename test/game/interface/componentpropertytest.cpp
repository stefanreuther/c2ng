/**
  *  \file test/game/interface/componentpropertytest.cpp
  *  \brief Test for game::interface::ComponentProperty
  */

#include "game/interface/componentproperty.hpp"

#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "interpreter/values.hpp"

using game::spec::Cost;
using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewString;

AFL_TEST("game.interface.ComponentProperty:getComponentProperty", a)
{
    game::spec::ShipList list;
    game::spec::Component comp(game::spec::ComponentNameProvider::Hull, 12);
    comp.setName("Twelve Long");
    comp.setShortName("Twelve Short");
    comp.setMass(140);
    comp.setTechLevel(7);
    comp.cost().set(Cost::Tritanium, 20);
    comp.cost().set(Cost::Duranium, 30);
    comp.cost().set(Cost::Molybdenum, 40);
    comp.cost().set(Cost::Money, 50);
    comp.cost().set(Cost::Supplies, 60);

    // Check
    verifyNewInteger(a("Mass"),  getComponentProperty(comp, game::interface::icpMass,      list), 140);
    verifyNewInteger(a("Tech"),  getComponentProperty(comp, game::interface::icpTech,      list), 7);
    verifyNewInteger(a("T"),     getComponentProperty(comp, game::interface::icpCostT,     list), 20);
    verifyNewInteger(a("D"),     getComponentProperty(comp, game::interface::icpCostD,     list), 30);
    verifyNewInteger(a("M"),     getComponentProperty(comp, game::interface::icpCostM,     list), 40);
    verifyNewInteger(a("MC"),    getComponentProperty(comp, game::interface::icpCostMC,    list), 50);
    verifyNewInteger(a("Sup"),   getComponentProperty(comp, game::interface::icpCostSup,   list), 60);
    verifyNewString (a("Str"),   getComponentProperty(comp, game::interface::icpCostStr,   list), "20T 30D 40M 60S 50$");
    verifyNewString (a("Name"),  getComponentProperty(comp, game::interface::icpName,      list), "Twelve Long");
    verifyNewString (a("Short"), getComponentProperty(comp, game::interface::icpNameShort, list), "Twelve Short");
    verifyNewInteger(a("Id"),    getComponentProperty(comp, game::interface::icpId,        list), 12);
}

AFL_TEST("game.interface.ComponentProperty:setComponentProperty", a)
{
    game::spec::ShipList list;
    game::spec::Component comp(game::spec::ComponentNameProvider::Hull, 12);
    comp.setName("a");
    comp.setShortName("b");

    std::auto_ptr<afl::data::Value> sv1(interpreter::makeStringValue("one"));
    std::auto_ptr<afl::data::Value> sv2(interpreter::makeStringValue("two"));

    // Successful set operations; verify
    AFL_CHECK_SUCCEEDS(a("icpName"),      setComponentProperty(comp, game::interface::icpName,      sv1.get(), list));
    AFL_CHECK_SUCCEEDS(a("icpNameShort"), setComponentProperty(comp, game::interface::icpNameShort, sv2.get(), list));
    a.checkEqual("getName",      comp.getName(list.componentNamer()), "one");
    a.checkEqual("getShortName", comp.getShortName(list.componentNamer()), "two");

    // Failing operation
    std::auto_ptr<afl::data::Value> iv(interpreter::makeIntegerValue(99));
    AFL_CHECK_THROWS(a("icpId"), setComponentProperty(comp, game::interface::icpId, iv.get(), list), interpreter::Error);
}
