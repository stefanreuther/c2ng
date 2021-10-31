/**
  *  \file game/vcr/flak/visualizer.hpp
  *  \brief Interface game::vcr::flak::Visualizer
  */
#ifndef C2NG_GAME_VCR_FLAK_VISUALIZER_HPP
#define C2NG_GAME_VCR_FLAK_VISUALIZER_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"
#include "game/vcr/flak/position.hpp"

namespace game { namespace vcr { namespace flak {

    /** Visualisation of a FLAK fight.
        This interface allows receiving visualisation events from a FLAK Algorithm.

        Ships and fleets are identified by indexes, in the same way as in the Algorithm or Setup as 0-based indexes.

        Fighters and torpedoes are called "Object" in FLAK and are identified by a nonzero index.
        Indexes are re-used when an object gets destroyed, and can be used as array indexes.
        Functions explicitly state when they allocate an index anew, or release it. */
    class Visualizer : public afl::base::Deletable {
     public:
        typedef size_t Ship_t;      ///< Type containing a ship number.
        typedef size_t Fleet_t;     ///< Type containing a fleet number.
        typedef size_t Object_t;    ///< Type containing an object number.

        static const Ship_t NO_ENEMY = Ship_t(-1);


        /*
         *  General
         */

        /** Update time. Called once per battle tick.
            \param time Time */
        virtual void updateTime(int32_t time) = 0;


        /*
         *  Beams
         */

        /** Fire beam from fighter, at fighter (JS: beamff).
            \param from     Object Id of firing fighter
            \param to       Object Id of targeted fighter
            \param hits     true if beam hits. In this case, a killFighter() call follows. */
        virtual void fireBeamFighterFighter(Object_t from, Object_t to, bool hits) = 0;

        /** Fire beam from fighter, at ship (JS: beamfs).
            \param from     Object Id of firing fighter
            \param to       Ship Id of targeted ship
            \param hits     true if beam hits */
        virtual void fireBeamFighterShip(Object_t from, Ship_t to, bool hits) = 0;

        /** Fire beam from ship, at fighter (JS: beamsf).
            \param from     Ship Id of firing ship
            \param beamNr   Number of firing beam (0-based)
            \param to       Object Id of targeted fighter
            \param hits     true if beam hits. In this case, a killFighter() call follows. */
        virtual void fireBeamShipFighter(Ship_t from, int beamNr, Object_t to, bool hits) = 0;

        /** Fire beam from ship, at ship (JS: beamss).
            \param from     Ship Id of firing ship
            \param beamNr   Number of firing beam (0-based)
            \param to       Ship Id of targeted ship
            \param hits     true if beam hits */
        virtual void fireBeamShipShip(Ship_t from, int beamNr, Ship_t to, bool hits) = 0;


        /*
         *  Fighters
         */

        /** Create (launch) new fighter (JS: fnew).
            \param id     Object Id to associate with this fighter (must be unused, will become used)
            \param pos    Initial position
            \param player Owner
            \param enemy  Initial enemy */
        virtual void createFighter(Object_t id, const Position& pos, int player, Ship_t enemy) = 0;

        /** Kill a fighter (JS: fkill).
            Called when a fighter dies after being hit by a beam.
            \param id     Object Id (must be used, will become unused) */
        virtual void killFighter(Object_t id) = 0;

        /** Land a fighter (JS: fland).
            Called when a fighter returns to a base.
            \param id     Object Id (must be used, will become unused) */
        virtual void landFighter(Object_t id) = 0;

        /** Move fighter (JS: fmove).
            \param id     Object Id
            \param pos    Position
            \param to     Target (enemy for attacking fighter, base for retreating) */
        virtual void moveFighter(Object_t id, const Position& pos, Ship_t to) = 0;


        /*
         *  Fleets
         */

        /** Create fleet (JS: gnew).
            \param fleetNr   Fleet number
            \param x,y       Position
            \param player    Owner
            \param firstShip First ship Id
            \param numShips  Number of ships */
        virtual void createFleet(Fleet_t fleetNr, int32_t x, int32_t y, int player, Ship_t firstShip, size_t numShips) = 0;

        /** Change fleet enemy (JS: genemy).
            \param fleetNr   Fleet number
            \param enemy     Enemy */
        virtual void setEnemy(Fleet_t fleetNr, Ship_t enemy) = 0;

        /** Kill a fleet (JS: gkill).
            Called when all ships are dead.
            \param fleetNr   Fleet number */
        virtual void killFleet(Fleet_t fleetNr) = 0;

        /** Move fleet (JS: gmove).
            \param fleetNr   Fleet number
            \param x,y       Position */
        virtual void moveFleet(Fleet_t fleetNr, int32_t x, int32_t y) = 0;


        /*
         *  Ships
         */

        struct ShipInfo {
            String_t name;
            bool isPlanet;
            int player;
            int shield;
            int damage;
            int crew;
            int numBeams;
            int numLaunchers;
            int numTorpedoes;
            int numBays;
            int numFighters;
            int torpedoType;
            int beamType;
            int mass;
            int id;

            ShipInfo()
                : name(), isPlanet(), player(),
                  shield(), damage(), crew(),
                  numBeams(), numLaunchers(), numTorpedoes(), numBays(), numFighters(),
                  torpedoType(), beamType(),
                  mass(), id()
                { }
        };

        /** Create ship (JS: snew).
            \param shipNr   Ship number
            \param pos      Position
            \param info     Information about this ship */
        virtual void createShip(Ship_t shipNr, const Position& pos, const ShipInfo& info) = 0;

        /** Kill a ship (JS: skill).
            \param shipNr   Ship number */
        virtual void killShip(Ship_t shipNr) = 0;

        /** Move a ship (JS: smove).
            \param shipNr   Ship number
            \param pos      Position */
        virtual void moveShip(Ship_t shipNr, const Position& pos) = 0;

        /*
         *  Torpedoes
         */

        /** Create (launch) a torpedo (JS: tnew).
            \param id     Object Id to associate with this torpedo (must be unused, will become used)
            \param pos    Initial position
            \param player Owner
            \param enemy  Target */
        virtual void createTorpedo(Object_t id, const Position& pos, int player, Ship_t enemy) = 0;

        /** Torpedo hits target (JS: thit).
            \param id     Object Id (must be used, will become unused)
            \param shipNr Ship being hit (can differ from original target) */
        virtual void hitTorpedo(Object_t id, Ship_t shipNr) = 0;

        /** Torpedo misses (JS: tmiss).
            \param id     Object Id (must be used, will become unused) */
        virtual void missTorpedo(Object_t id) = 0;

        /** Move torpedo (JS: tmove).
            \param id     Object Id
            \param pos    Position */
        virtual void moveTorpedo(Object_t id, const Position& pos) = 0;
    };

} } }

#endif
