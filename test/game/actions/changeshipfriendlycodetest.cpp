/**
  *  \file test/game/actions/changeshipfriendlycodetest.cpp
  *  \brief Test for game::actions::ChangeShipFriendlyCode
  */

#include "game/actions/changeshipfriendlycode.hpp"

#include "afl/test/testrunner.hpp"
#include "game/map/reverter.hpp"

namespace {
    game::map::Ship& addShip(game::map::Universe& univ, game::Id_t id, String_t fc)
    {
        game::map::Ship* p = univ.ships().create(id);
        p->setFriendlyCode(fc);
        return *p;
    }
}

/** Test normal behaviour.
    A: create universe with a single ship. Call setFriendlyCode. Call undo().
    E: friendly code must be changed/reverted */
AFL_TEST("game.actions.ChangeShipFriendlyCode:normal", a)
{
    game::spec::FriendlyCodeList fcl;
    util::RandomNumberGenerator rng(0);

    game::map::Universe univ;
    game::map::Ship& sh = addShip(univ, 77, "abc");

    game::actions::ChangeShipFriendlyCode t(univ);
    t.addShip(77, fcl, rng);

    t.setFriendlyCode("xyz");
    a.checkEqual("01. getFriendlyCode", sh.getFriendlyCode().orElse(""), "xyz");

    t.undo();
    a.checkEqual("11. getFriendlyCode", sh.getFriendlyCode().orElse(""), "abc");
}

/** Test unsetFriendlyCode(), avoid new code.
    A: Call setFriendlyCode(), then unsetFriendlyCode() with same friendly code.
    E: friendly code back at original value */
AFL_TEST("game.actions.ChangeShipFriendlyCode:unsetFriendlyCode:new", a)
{
    game::spec::FriendlyCodeList fcl;
    util::RandomNumberGenerator rng(0);

    game::map::Universe univ;
    game::map::Ship& sh = addShip(univ, 77, "abc");

    game::actions::ChangeShipFriendlyCode t(univ);
    t.addShip(77, fcl, rng);

    t.setFriendlyCode("xyz");
    t.unsetFriendlyCode("xyz");
    a.checkEqual("01. getFriendlyCode", sh.getFriendlyCode().orElse(""), "abc");
}

/** Test unsetFriendlyCode(), avoid old code.
    A: Call unsetFriendlyCode() with the ship's friendly code.
    E: friendly code set to random value */
AFL_TEST("game.actions.ChangeShipFriendlyCode:unsetFriendlyCode:old", a)
{
    game::spec::FriendlyCodeList fcl;
    util::RandomNumberGenerator rng(0);

    game::map::Universe univ;
    game::map::Ship& sh = addShip(univ, 77, "abc");

    game::actions::ChangeShipFriendlyCode t(univ);
    t.addShip(77, fcl, rng);

    t.unsetFriendlyCode("abc");
    a.checkDifferent("01. getFriendlyCode", sh.getFriendlyCode().orElse(""), "abc");
}

/** Test unsetFriendlyCode(), avoid old code, fallback to Reverter.
    A: Call unsetFriendlyCode() with the ship's friendly code.
    E: friendly code set to Reverter's value */
AFL_TEST("game.actions.ChangeShipFriendlyCode:unsetFriendlyCode:reverter", a)
{
    // Reverter mock
    class TestReverter : public game::map::Reverter {
     public:
        virtual afl::base::Optional<int> getMinBuildings(int /*planetId*/, game::PlanetaryBuilding /*building*/) const
            { return 0; }
        virtual int getSuppliesAllowedToBuy(int /*planetId*/) const
            { return 0; }
        virtual afl::base::Optional<int> getMinTechLevel(int /*planetId*/, game::TechLevel /*techLevel*/) const
            { return 1; }
        virtual afl::base::Optional<int> getMinBaseStorage(int /*planetId*/, game::TechLevel /*area*/, int /*slot*/) const
            { return 1; }
        virtual int getNumTorpedoesAllowedToSell(int /*planetId*/, int /*slot*/) const
            { return 0; }
        virtual int getNumFightersAllowedToSell(int /*planetId*/) const
            { return 0; }
        virtual afl::base::Optional<String_t> getPreviousShipFriendlyCode(game::Id_t /*shipId*/) const
            { return String_t("rev"); }
        virtual afl::base::Optional<String_t> getPreviousPlanetFriendlyCode(game::Id_t /*planetId*/) const
            { return String_t("x"); }
        virtual bool getPreviousShipMission(int /*shipId*/, int& /*m*/, int& /*i*/, int& /*t*/) const
            { return false; }
        virtual bool getPreviousShipBuildOrder(int /*planetId*/, game::ShipBuildOrder& /*result*/) const
            { return false; }
        virtual game::map::LocationReverter* createLocationReverter(game::map::Point /*pt*/) const
            { return 0; }
    };

    game::spec::FriendlyCodeList fcl;
    util::RandomNumberGenerator rng(0);

    game::map::Universe univ;
    univ.setNewReverter(new TestReverter());
    game::map::Ship& sh = addShip(univ, 77, "abc");

    game::actions::ChangeShipFriendlyCode t(univ);
    t.addShip(77, fcl, rng);

    t.unsetFriendlyCode("abc");
    a.checkEqual("01. getFriendlyCode", sh.getFriendlyCode().orElse(""), "rev");
}
