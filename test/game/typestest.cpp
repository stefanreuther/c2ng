/**
  *  \file test/game/typestest.cpp
  *  \brief Test for game::Types
  */

#include "game/types.hpp"
#include "afl/test/testrunner.hpp"

/** Test native races.
    Enum values must match v3 race numbers. */
AFL_TEST("game.Types:NativeRace", a)
{
    a.checkEqual("01", int(game::NoNatives),         0);
    a.checkEqual("02", int(game::HumanoidNatives),   1);
    a.checkEqual("03", int(game::BovinoidNatives),   2);
    a.checkEqual("04", int(game::ReptilianNatives),  3);
    a.checkEqual("05", int(game::AvianNatives),      4);
    a.checkEqual("06", int(game::AmorphousNatives),  5);
    a.checkEqual("07", int(game::InsectoidNatives),  6);
    a.checkEqual("08", int(game::AmphibianNatives),  7);
    a.checkEqual("09", int(game::GhipsoldalNatives), 8);
    a.checkEqual("10", int(game::SiliconoidNatives), 9);
}

/** Test shipyard actions.
    Enum values must match v3 values. */
AFL_TEST("game.Types:ShipyardAction", a)
{
    a.checkEqual("01", int(game::NoShipyardAction),      0);
    a.checkEqual("02", int(game::FixShipyardAction),     1);
    a.checkEqual("03", int(game::RecycleShipyardAction), 2);
}

/** Test planetary buildings.
    This sequence appears in various file formats. */
AFL_TEST("game.Types:PlanetaryBuilding", a)
{
    a.checkEqual("01", int(game::MineBuilding),        0);
    a.checkEqual("02", int(game::FactoryBuilding),     1);
    a.checkEqual("03", int(game::DefenseBuilding),     2);
    a.checkEqual("04", int(game::BaseDefenseBuilding), 3);
}

/** Test IntegerProperty_t. */
AFL_TEST("game.Types:IntegerProperty_t", a)
{
    // Integer
    int iv;
    game::IntegerProperty_t ip;
    a.check("01. isValid", !ip.isValid());

    ip = 99;
    a.check("11. isValid", ip.isValid());
    a.check("12. get", ip.get(iv));
    a.checkEqual("13. value", iv, 99);

    ip = 0;
    a.check("21. isValid", ip.isValid());
    a.check("22. get", ip.get(iv));
    a.checkEqual("23. value", iv, 0);

    ip = 10000;
    a.check("31. isValid", ip.isValid());
    a.check("32. get", ip.get(iv));
    a.checkEqual("33. value", iv, 10000);
}

/** Test LongProperty_t. */
AFL_TEST("game.Types:LongProperty_t", a)
{
    int32_t lv;
    game::LongProperty_t lp;
    a.check("01. isValid", !lp.isValid());

    lp = 99;
    a.check("11. isValid", lp.isValid());
    a.check("12. get", lp.get(lv));
    a.checkEqual("13. value", lv, 99);

    lp = 0;
    a.check("21. isValid", lp.isValid());
    a.check("22. get", lp.get(lv));
    a.checkEqual("23. value", lv, 0);

    lp = 100000000;
    a.check("31. isValid", lp.isValid());
    a.check("32. get", lp.get(lv));
    a.checkEqual("33. value", lv, 100000000);
}

/** Test NegativeProperty_t. */
AFL_TEST("game.Types:NegativeProperty_t", a)
{
    int nv;
    game::NegativeProperty_t np;
    a.check("01. isValid", !np.isValid());

    np = 99;
    a.check("11. isValid", np.isValid());
    a.check("12. get", np.get(nv));
    a.checkEqual("13. value", nv, 99);

    np = -1;
    a.check("21. isValid", np.isValid());
    a.check("22. get", np.get(nv));
    a.checkEqual("23. value", nv, -1);

    np = -10000;
    a.check("31. isValid", np.isValid());
    a.check("32. get", np.get(nv));
    a.checkEqual("33. value", nv, -10000);

    np = 0;
    a.check("41. isValid", np.isValid());
    a.check("42. get", np.get(nv));
    a.checkEqual("43. value", nv, 0);

    np = 10000;
    a.check("51. isValid", np.isValid());
    a.check("52. get", np.get(nv));
    a.checkEqual("53. value", nv, 10000);
}

/** Test StringProperty_t. */
AFL_TEST("game.Types:StringProperty_t", a)
{
    String_t sv;
    game::StringProperty_t sp;
    a.check("01. isValid", !sp.isValid());

    sp = "";
    a.check("11. isValid", sp.isValid());
    a.check("12. get", sp.get(sv));
    a.checkEqual("13. value", sv, "");

    sp = "TestinG!";
    a.check("21. isValid", sp.isValid());
    a.check("22. get", sp.get(sv));
    a.checkEqual("23. value", sv, "TestinG!");
}
