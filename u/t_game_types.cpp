/**
  *  \file u/t_game_types.cpp
  *  \brief Test for game::Types
  */

#include "game/types.hpp"

#include "t_game.hpp"

/** Test native races.
    Enum values must match v3 race numbers. */
void
TestGameTypes::testNativeRace()
{
    TS_ASSERT_EQUALS(int(game::NoNatives),         0);
    TS_ASSERT_EQUALS(int(game::HumanoidNatives),   1);
    TS_ASSERT_EQUALS(int(game::BovinoidNatives),   2);
    TS_ASSERT_EQUALS(int(game::ReptilianNatives),  3);
    TS_ASSERT_EQUALS(int(game::AvianNatives),      4);
    TS_ASSERT_EQUALS(int(game::AmorphousNatives),  5);
    TS_ASSERT_EQUALS(int(game::InsectoidNatives),  6);
    TS_ASSERT_EQUALS(int(game::AmphibianNatives),  7);
    TS_ASSERT_EQUALS(int(game::GhipsoldalNatives), 8);
    TS_ASSERT_EQUALS(int(game::SiliconoidNatives), 9);
}

/** Test shipyard actions.
    Enum values must match v3 values. */
void
TestGameTypes::testShipyardAction()
{
    TS_ASSERT_EQUALS(int(game::NoShipyardAction),      0);
    TS_ASSERT_EQUALS(int(game::FixShipyardAction),     1);
    TS_ASSERT_EQUALS(int(game::RecycleShipyardAction), 2);
}

/** Test planetary buildings.
    This sequence appears in various file formats. */
void
TestGameTypes::testPlanetaryBuilding()
{
    TS_ASSERT_EQUALS(int(game::MineBuilding),        0);
    TS_ASSERT_EQUALS(int(game::FactoryBuilding),     1);
    TS_ASSERT_EQUALS(int(game::DefenseBuilding),     2);
    TS_ASSERT_EQUALS(int(game::BaseDefenseBuilding), 3);
}

/** Test IntegerProperty_t. */
void
TestGameTypes::testIntegerProperty()
{
    // Integer
    int iv;
    game::IntegerProperty_t ip;
    TS_ASSERT(!ip.isValid());

    ip = 99;
    TS_ASSERT(ip.isValid());
    TS_ASSERT(ip.get(iv));
    TS_ASSERT_EQUALS(iv, 99);

    ip = 0;
    TS_ASSERT(ip.isValid());
    TS_ASSERT(ip.get(iv));
    TS_ASSERT_EQUALS(iv, 0);

    ip = 10000;
    TS_ASSERT(ip.isValid());
    TS_ASSERT(ip.get(iv));
    TS_ASSERT_EQUALS(iv, 10000);
}

/** Test LongProperty_t. */
void
TestGameTypes::testLongProperty()
{
    int32_t lv;
    game::LongProperty_t lp;
    TS_ASSERT(!lp.isValid());

    lp = 99;
    TS_ASSERT(lp.isValid());
    TS_ASSERT(lp.get(lv));
    TS_ASSERT_EQUALS(lv, 99);

    lp = 0;
    TS_ASSERT(lp.isValid());
    TS_ASSERT(lp.get(lv));
    TS_ASSERT_EQUALS(lv, 0);

    lp = 100000000;
    TS_ASSERT(lp.isValid());
    TS_ASSERT(lp.get(lv));
    TS_ASSERT_EQUALS(lv, 100000000);
}

/** Test NegativeProperty_t. */
void
TestGameTypes::testNegativeProperty()
{
    int nv;
    game::NegativeProperty_t np;
    TS_ASSERT(!np.isValid());

    np = 99;
    TS_ASSERT(np.isValid());
    TS_ASSERT(np.get(nv));
    TS_ASSERT_EQUALS(nv, 99);

    np = -1;
    TS_ASSERT(np.isValid());
    TS_ASSERT(np.get(nv));
    TS_ASSERT_EQUALS(nv, -1);

    np = -10000;
    TS_ASSERT(np.isValid());
    TS_ASSERT(np.get(nv));
    TS_ASSERT_EQUALS(nv, -10000);

    np = 0;
    TS_ASSERT(np.isValid());
    TS_ASSERT(np.get(nv));
    TS_ASSERT_EQUALS(nv, 0);

    np = 10000;
    TS_ASSERT(np.isValid());
    TS_ASSERT(np.get(nv));
    TS_ASSERT_EQUALS(nv, 10000);
}

/** Test StringProperty_t. */
void
TestGameTypes::testStringProperty()
{
    String_t sv;
    game::StringProperty_t sp;
    TS_ASSERT(!sp.isValid());

    sp = "";
    TS_ASSERT(sp.isValid());
    TS_ASSERT(sp.get(sv));
    TS_ASSERT_EQUALS(sv, "");

    sp = "TestinG!";
    TS_ASSERT(sp.isValid());
    TS_ASSERT(sp.get(sv));
    TS_ASSERT_EQUALS(sv, "TestinG!");
}
