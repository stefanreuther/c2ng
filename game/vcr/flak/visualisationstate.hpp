/**
  *  \file game/vcr/flak/visualisationstate.hpp
  *  \brief Class game::vcr::flak::VisualisationState
  */
#ifndef C2NG_GAME_VCR_FLAK_VISUALISATIONSTATE_HPP
#define C2NG_GAME_VCR_FLAK_VISUALISATIONSTATE_HPP

#include <vector>
#include "afl/base/memory.hpp"
#include "game/vcr/flak/position.hpp"
#include "game/vcr/flak/visualizer.hpp"
#include "util/randomnumbergenerator.hpp"

namespace game { namespace vcr { namespace flak {

    /** FLAK Visualisation state.
        Stores the visualizer-side state of a FLAK simulation,
        by augmenting the Visualizer callbacks with additional state.

        A renderer can use it to display the fight, and needs not carry own state (except for rendering resources);
        the idea is to have enough state that a renderer can be switched mid-playback and have consistent appearance.

        A VisualisationState object contains things to render:
        - objects (fighters, torpedoes)
        - ships
        - fleets
        - active beams
        - active smoke particles

        Use the accessors for (read-only) access to current data;
        use the Visualizer functions to update it.

        Coordinates are given in meters, which matches the regular FLAK X/Y coordinate system.
        FLAK's Z coordinate has a different measure and is adjusted by VisualisationState.

        A VisualisationState object can be copied/stored for fast-forward/rewind operations. */
    class VisualisationState : public Visualizer {
     public:
        /** Type of an object. */
        enum ObjectType {
            NoObject,                     ///< Unused slot.
            TorpedoObject,                ///< Torpedo.
            FighterObject                 ///< Fighter.
        };

        /** Object (fighter/torpedo). */
        struct Object {
            ObjectType type;              ///< Type of the object.
            Position pos;                 ///< Position in 3D space.
            int player;                   ///< Owner.
            float heading;                ///< Fighter: heading (towards enemy or base).
            int xRotation, yRotation;     ///< Torpedo: random rotation to rotate the torpedo model if desired.
            Object()
                : type(), pos(), player(), heading(), xRotation(), yRotation()
                { }
        };

        /** Ship (or planet). */
        struct Ship : public Visualizer::ShipInfo {
            bool isAlive;                 ///< true if this ship is alive.
            float heading;                ///< Heading (auto-turns towards enemy).
            Position pos;                 ///< Position in 3D space.
            size_t enemy;                 ///< Ship index of enemy.
            Ship()
                : ShipInfo(), isAlive(), heading(), pos(), enemy()
                { }
        };

        /** Fleet. */
        struct Fleet {
            int player;                   ///< Owner.
            size_t firstShip;             ///< Ship index of first ship.
            size_t numShips;              ///< Number of ships.
            bool isAlive;                 ///< true if this fleet is alive (=any of its ships are alive).
            int32_t x, y;                 ///< Position in 2D space.
            size_t enemy;                 ///< Ship index of enemy.
            Fleet()
                : player(), firstShip(), numShips(), isAlive(), x(), y(), enemy()
                { }
        };

        /** Smoke particle. */
        struct Smoke {
            Position pos;                 ///< Position in 3D space.
            int dx, dy, dz;               ///< Movement vector.
            int age;                      ///< Age [0,setMaxSmokeAge())
            Smoke(const Position& pos, int dx, int dy, int dz, int age)
                : pos(pos), dx(dx), dy(dy), dz(dz), age(age)
                { }
        };

        /** Active beam. */
        struct Beam {
            Position from;                ///< Origin in 3D space.
            Position to;                  ///< Target in 3D space.
            int age;                      ///< Age [0,setMaxBeamAge())
            Beam(const Position& from, const Position& to, int age)
                : from(from), to(to), age(age)
                { }
        };

        VisualisationState();
        ~VisualisationState();

        afl::base::Memory<const Object> objects() const;
        afl::base::Memory<const Ship> ships() const;
        afl::base::Memory<const Fleet> fleets() const;
        afl::base::Memory<const Smoke> smoke() const;
        afl::base::Memory<const Beam> beams() const;

        /** Get time.
            \return time */
        int32_t getTime() const;

        /** Perform an animation step.
            Call once per tick.
            This will perform implicit animation actions:
            - ships turning towards their enemies
            - smoke particles and beams advancing time

            \retval true   Active animations remain
            \retval false  No more animations to play */
        bool animate();

        /** Get current size of arena.
            The arena size changes as the battle progresses.
            \return Radius of the arena size (display this many meters in every direction from (0,0,0)) */
        float getArenaSize() const;

        /** Get grid size.
            This is the size of the battle grid, which remains fixed during the fight.
            \return Grid size (display grid of this size around (0,0,0)) */
        int32_t getGridSize() const;

        /** Set beam age.
            Each beam effect will live this many ticks.
            \param n New age */
        void setMaxBeamAge(int n);

        /** Set smoke age.
            Each smoke particle will live this many ticks.
            \param n New age */
        void setMaxSmokeAge(int n);

        // Visualizer:
        virtual void updateTime(int32_t time);
        virtual void fireBeamFighterFighter(Object_t from, Object_t to, bool hits);
        virtual void fireBeamFighterShip(Object_t from, Ship_t to, bool hits);
        virtual void fireBeamShipFighter(Ship_t from, int beamNr, Object_t to, bool hits);
        virtual void fireBeamShipShip(Ship_t from, int beamNr, Ship_t to, bool hits);
        virtual void createFighter(Object_t id, const Position& pos, int player, Ship_t enemy);
        virtual void killFighter(Object_t id);
        virtual void landFighter(Object_t id);
        virtual void moveFighter(Object_t id, const Position& pos, Ship_t to);
        virtual void createFleet(Fleet_t fleetNr, int32_t x, int32_t y, int player, Ship_t firstShip, size_t numShips);
        virtual void setEnemy(Fleet_t fleetNr, Ship_t enemy);
        virtual void killFleet(Fleet_t fleetNr);
        virtual void moveFleet(Fleet_t fleetNr, int32_t x, int32_t y);
        virtual void createShip(Ship_t shipNr, const Position& pos, const ShipInfo& info);
        virtual void killShip(Ship_t shipNr);
        virtual void moveShip(Ship_t shipNr, const Position& pos);
        virtual void createTorpedo(Object_t id, const Position& pos, int player, Ship_t enemy);
        virtual void hitTorpedo(Object_t id, Ship_t shipNr);
        virtual void missTorpedo(Object_t id);
        virtual void moveTorpedo(Object_t id, const Position& pos);

     private:
        std::vector<Object> m_objects;
        std::vector<Ship> m_ships;
        std::vector<Fleet> m_fleets;
        std::vector<Smoke> m_smoke;
        std::vector<Beam> m_beams;

        util::RandomNumberGenerator m_rng;

        int m_maxBeamAge;
        int m_maxSmokeAge;
        int32_t m_gridSize;
        int32_t m_time;

        void addSmoke(const Position& pos, int n, int age);
        void addBeam(const Position& from, const Position& to);
    };

} } }

#endif
