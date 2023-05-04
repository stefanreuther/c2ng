/**
  *  \file u/t_game_sim_gameinterface.cpp
  *  \brief Test for game::sim::GameInterface
  */

#include "game/sim/gameinterface.hpp"

#include "t_game_sim.hpp"

/** Interface test. */
void
TestGameSimGameInterface::testInterface()
{
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
            { return 0; }
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
        virtual void getPlayerRelations(game::PlayerBitMatrix& /*alliances*/, game::PlayerBitMatrix& /*enemies*/) const
            { }
    };
    Tester t;
}

