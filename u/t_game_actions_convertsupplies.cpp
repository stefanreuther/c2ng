/**
  *  \file u/t_game_actions_convertsupplies.cpp
  *  \brief Test for game::actions::ConvertSupplies
  */

#include "game/actions/convertsupplies.hpp"

#include "t_game_actions.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/map/planet.hpp"
#include "game/map/reverter.hpp"

namespace {
    using game::map::Planet;
    using game::Element;

    class TestReverter : public game::map::Reverter {
     public:
        virtual afl::base::Optional<int> getMinBuildings(int /*planetId*/, game::PlanetaryBuilding /*building*/) const
            { return 0; }
        virtual int getSuppliesAllowedToBuy(int /*planetId*/) const
            { return 3000; }
        virtual afl::base::Optional<int> getMinTechLevel(int /*planetId*/, game::TechLevel /*techLevel*/) const
            { return 0; }
        virtual afl::base::Optional<int> getMinBaseStorage(int /*planetId*/, game::TechLevel /*area*/, int /*slot*/) const
            { return 0; }
        virtual int getNumTorpedoesAllowedToSell(int /*planetId*/, int /*slot*/) const
            { return 0; }
        virtual int getNumFightersAllowedToSell(int /*planetId*/) const
            { return 0; }
        virtual afl::base::Optional<String_t> getPreviousShipFriendlyCode(game::Id_t /*shipId*/) const
            { return afl::base::Nothing; }
        virtual afl::base::Optional<String_t> getPreviousPlanetFriendlyCode(game::Id_t /*planetId*/) const
            { return afl::base::Nothing; }
        virtual bool getPreviousShipMission(int /*shipId*/, int& /*m*/, int& /*i*/, int& /*t*/) const
            { return false; }
        virtual bool getPreviousShipBuildOrder(int /*planetId*/, game::ShipBuildOrder& /*result*/) const
            { return false; }
        virtual game::map::LocationReverter* createLocationReverter(game::map::Point /*pt*/) const
            { return 0; }
    };

    void prepare(Planet& p)
    {
        game::map::PlanetData pd;
        pd.owner = 3;
        pd.colonistClans = 100;
        pd.supplies = 1000;
        pd.money = 500;
        p.addCurrentPlanetData(pd, game::PlayerSet_t(3));

        afl::string::NullTranslator tx;
        afl::sys::Log log;
        p.internalCheck(game::map::Configuration(), tx, log);
        p.setPlayability(Planet::Playable);
    }
}

/** Test normal behaviour.
    A: prepare normal planet. Sell supplies.
    E: supply sale must work until supplies run out */
void
TestGameActionsConvertSupplies::testNormal()
{
    // Environment
    Planet p(77);
    prepare(p);

    // Testee
    game::actions::ConvertSupplies testee(p);
    TS_ASSERT_EQUALS(testee.getMaxSuppliesToSell(), 1000);
    TS_ASSERT_EQUALS(testee.getMaxSuppliesToBuy(), 0);

    // Cannot sell 2000 supplies
    TS_ASSERT_EQUALS(testee.sellSupplies(2000, false), 0);

    // Can sell 100 supplies normally
    TS_ASSERT_EQUALS(testee.sellSupplies(100, false), 100);
    TS_ASSERT_EQUALS(p.getCargo(Element::Supplies).orElse(0), 900);

    // Can sell 900 as part of 2000
    TS_ASSERT_EQUALS(testee.sellSupplies(2000, true), 900);
    TS_ASSERT_EQUALS(p.getCargo(Element::Supplies).orElse(0), 0);

    // Cannot buy supplies because we have no reverter
    TS_ASSERT_EQUALS(testee.buySupplies(100, true), 0);
}

/** Test behaviour with reserved supplies.
    A: prepare normal planet. Reserve some supplies. Sell supplies.
    E: reserved supplies must not be sold */
void
TestGameActionsConvertSupplies::testReserved()
{
    // Environment
    Planet p(77);
    prepare(p);

    // Testee
    game::actions::ConvertSupplies testee(p);
    testee.setReservedSupplies(300);
    TS_ASSERT_EQUALS(testee.getMaxSuppliesToSell(), 700);
    TS_ASSERT_EQUALS(testee.getMaxSuppliesToBuy(), 0);

    // Will not sell the reserved supplies
    TS_ASSERT_EQUALS(testee.sellSupplies(2000, true), 700);
    TS_ASSERT_EQUALS(p.getCargo(Element::Supplies).orElse(0), 300);
}

/** Test behaviour with undo.
    A: prepare normal planet and a reverter. Buy supplies.
    E: expected number of supplies can be bought */
void
TestGameActionsConvertSupplies::testBuy()
{
    // Environment
    Planet p(77);
    prepare(p);

    game::map::Universe univ;
    univ.setNewReverter(new TestReverter());

    // Testee
    game::actions::ConvertSupplies testee(p);
    testee.setUndoInformation(univ);
    testee.setReservedMoney(100);
    TS_ASSERT_EQUALS(testee.getMaxSuppliesToSell(), 1000);
    TS_ASSERT_EQUALS(testee.getMaxSuppliesToBuy(), 400);

    // Buy some supplies
    TS_ASSERT_EQUALS(testee.buySupplies(50, true), 50);
    TS_ASSERT_EQUALS(p.getCargo(Element::Supplies).orElse(0), 1050);
}

