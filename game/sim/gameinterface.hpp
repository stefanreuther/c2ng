/**
  *  \file game/sim/gameinterface.hpp
  *  \brief Interface game::sim::GameInterface
  */
#ifndef C2NG_GAME_SIM_GAMEINTERFACE_HPP
#define C2NG_GAME_SIM_GAMEINTERFACE_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/optional.hpp"
#include "game/map/point.hpp"
#include "game/playerbitmatrix.hpp"
#include "game/types.hpp"

namespace game { namespace sim {

    class Ship;
    class Planet;

    /** Interface between Simulator and Game.
        Provides methods for the simulator setup code to adapt to various environments.
        When we're running from a full client with game data available, we want to allow data transfers to and from the game.
        We don't want to depend on the full game data, though, to be able to run standalone. */
    class GameInterface : public afl::base::Deletable {
        // ex GSimulatorGameInterface
     public:
        /** Relation of a unit to the simulation. */
        enum Relation {
            Unknown,               ///< Unknown or different from unit in simulation. ex ShipIsUnknown.
            ReadOnly,              ///< Read-only, possibly foreign (copyFromGame valid). ex ShipIsForeign.
            Playable               ///< Playable (copyFromGame and copyToGame valid). ex ShipIsPlayed.
        };

        /** Check availability of game data.
            \return true on availability */
        virtual bool hasGame() const = 0;

        /** Check whether game has a ship with the given Id.
            Used to avoid generating ships that collide with ships in the game.
            \param shipId Ship Id
            \return true if ship exists (visible on map) */
        virtual bool hasShip(Id_t shipId) const = 0;

        /** Get name of a planet.
            Used to set the name of the planet in a simulation.
            \param id Planet Id
            \return name, can be empty */
        virtual String_t getPlanetName(Id_t id) const = 0;

        /** Get highest possible planet Id.
            \return Id */
        virtual Id_t getMaxPlanetId() const = 0;

        /** Get (real) owner of a ship.
            Used to set default aggressiveness of an intercepting ship.
            \param id Ship Id
            \return owner; 0 if Id is out of range or unknown */
        virtual int getShipOwner(Id_t id) const = 0;

        /** Get highest possible ship Id.
            \return Id */
        virtual game::Id_t getMaxShipId() const = 0;

        /** Update simulation ship from game.
            \param [in,out] out Ship (identified by its Id)
            \return true on success */
        virtual bool copyShipFromGame(Ship& out) const = 0;

        /** Update game data from simulation ship.
            \param [in] in Ship
            \return true on success */
        virtual bool copyShipToGame(const Ship& in) = 0;

        /** Get relation between simulation ship and its game equivalent.
            \param [in] in Ship
            \return relation */
        virtual Relation getShipRelation(const Ship& in) const = 0;

        /** Get position of a ship on the map.
            Can be called for ships with relation Unknown; if an unrelated ship exists, returns that.
            \param [in] in Ship
            \return relation */
        virtual afl::base::Optional<game::map::Point> getShipPosition(const Ship& in) const = 0;

        /** Update simulation planet from game.
            \param [in,out] out planet (identified by its Id)
            \return true on success */
        virtual bool copyPlanetFromGame(Planet& out) const = 0;

        /** Update game data from simulation planet.
            \param [in] in planet
            \return true on success */
        virtual bool copyPlanetToGame(const Planet& in) = 0;

        /** Get relation between simulation planet and its game equivalent.
            \param [in] in Planet
            \return relation */
        virtual Relation getPlanetRelation(const Planet& in) const = 0;

        /** Get position of a planet on the map.
            Can be called for planets with relation Unknown; if an unrelated planet exists, returns that.
            \param [in] in Planet
            \return relation */
        virtual afl::base::Optional<game::map::Point> getPlanetPosition(const Planet& in) const = 0;

        /** Get player relations.
            \param [out] alliances Alliance relations
            \param [out] enemies   Enemy relations */
        virtual void getPlayerRelations(PlayerBitMatrix& alliances, PlayerBitMatrix& enemies) const = 0;
    };

} }


#endif
