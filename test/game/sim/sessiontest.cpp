/**
  *  \file test/game/sim/sessiontest.cpp
  *  \brief Test for game::sim::Session
  */

#include "game/sim/session.hpp"
#include "afl/test/testrunner.hpp"

/** Simple coverage test. */
AFL_TEST("game.sim.Session", a)
{
    // GameInterface implementation for testing
    // Returns getMaxShipId()=777
    class Tester : public game::sim::GameInterface {
     public:
        virtual bool hasGame() const
            { return false; }
        virtual bool hasShip(game::Id_t /*shipId*/) const
            { return false; }
        virtual String_t getPlanetName(game::Id_t /*id*/) const
            { return String_t(); }
        virtual game::Id_t getMaxPlanetId() const
            { return 0; }
        virtual int getShipOwner(game::Id_t /*id*/) const
            { return 0; }
        virtual game::Id_t getMaxShipId() const
            { return 777; }
        virtual bool copyShipFromGame(game::sim::Ship& /*out*/) const
            { return false; }
        virtual bool copyShipToGame(const game::sim::Ship& /*in*/)
            { return false; }
        virtual Relation getShipRelation(const game::sim::Ship& /*in*/) const
            { return Unknown; }
        virtual afl::base::Optional<game::map::Point> getShipPosition(const game::sim::Ship& /*in*/) const
            { return afl::base::Nothing; }
        virtual bool copyPlanetFromGame(game::sim::Planet& /*out*/) const
            { return false; }
        virtual bool copyPlanetToGame(const game::sim::Planet& /*in*/)
            { return false; }
        virtual Relation getPlanetRelation(const game::sim::Planet& /*in*/) const
            { return Unknown; }
        virtual afl::base::Optional<game::map::Point> getPlanetPosition(const game::sim::Planet& /*in*/) const
            { return afl::base::Nothing; }
        virtual void getPlayerRelations(game::PlayerBitMatrix& alliances, game::PlayerBitMatrix& enemies) const
            { alliances.set(1, 1, true); enemies.set(2, 2, true); }
    };

    game::sim::Session testee;
    const game::sim::Session& ct = testee;

    a.checkEqual("01. setup", &testee.setup(), &ct.setup());
    a.checkEqual("02. configuration", &testee.configuration(), &ct.configuration());

    a.checkNull("11. getGameInterface", testee.getGameInterface());

    testee.setNewGameInterface(new Tester());
    a.checkNonNull("21. getGameInterface", testee.getGameInterface());
    a.checkEqual("22. getMaxShipId", testee.getGameInterface()->getMaxShipId(), 777);

    // Default is player relations enabled
    a.check("31. isUsePlayerRelations", testee.isUsePlayerRelations());

    // Explicitly retrieve relations
    {
        game::PlayerBitMatrix aa, ee;
        testee.getPlayerRelations(aa, ee);
        a.check("41. getPlayerRelations", aa.get(1, 1));
        a.check("42. getPlayerRelations", ee.get(2, 2));
    }

    // Implicitly use relations
    testee.usePlayerRelations();
    a.check("51. allianceSettings", testee.configuration().allianceSettings().get(1, 1));
    a.check("52. enemySettings", testee.configuration().enemySettings().get(2, 2));

    // Turn off use of player relations; request to use it does not modify alliances
    testee.configuration().allianceSettings().set(1, 1, false);
    testee.setUsePlayerRelations(false);
    testee.usePlayerRelations();
    a.check("61. allianceSettings", !testee.configuration().allianceSettings().get(1, 1));
    a.check("62. enemySettings", testee.configuration().enemySettings().get(2, 2));
}
