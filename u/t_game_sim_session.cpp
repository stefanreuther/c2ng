/**
  *  \file u/t_game_sim_session.cpp
  *  \brief Test for game::sim::Session
  */

#include "game/sim/session.hpp"

#include "t_game_sim.hpp"

/** Simple coverage test. */
void
TestGameSimSession::testIt()
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

    TS_ASSERT_EQUALS(&testee.setup(), &ct.setup());
    TS_ASSERT_EQUALS(&testee.configuration(), &ct.configuration());

    TS_ASSERT(testee.getGameInterface() == 0);

    testee.setNewGameInterface(new Tester());
    TS_ASSERT(testee.getGameInterface() != 0);
    TS_ASSERT_EQUALS(testee.getGameInterface()->getMaxShipId(), 777);

    // Default is player relations enabled
    TS_ASSERT(testee.isUsePlayerRelations());

    // Explicitly retrieve relations
    {
        game::PlayerBitMatrix a, e;
        testee.getPlayerRelations(a, e);
        TS_ASSERT(a.get(1, 1));
        TS_ASSERT(e.get(2, 2));
    }

    // Implicitly use relations
    testee.usePlayerRelations();
    TS_ASSERT(testee.configuration().allianceSettings().get(1, 1));
    TS_ASSERT(testee.configuration().enemySettings().get(2, 2));

    // Turn off use of player relations; request to use it does not modify alliances
    testee.configuration().allianceSettings().set(1, 1, false);
    testee.setUsePlayerRelations(false);
    testee.usePlayerRelations();
    TS_ASSERT(!testee.configuration().allianceSettings().get(1, 1));
    TS_ASSERT(testee.configuration().enemySettings().get(2, 2));
}

