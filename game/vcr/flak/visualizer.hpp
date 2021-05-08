/**
  *  \file game/vcr/flak/visualizer.hpp
  *
  *  As of 20210417, this is a rough and preliminary port.
  */
#ifndef C2NG_GAME_VCR_FLAK_VISUALIZER_HPP
#define C2NG_GAME_VCR_FLAK_VISUALIZER_HPP

#include "afl/base/deletable.hpp"
#include "game/vcr/flak/algorithm.hpp"

namespace game { namespace vcr { namespace flak {

    //! Visualisation of a fight.
    class Visualizer : public afl::base::Deletable {
     public:
        typedef size_t ship_t;      ///< Type containing a ship number.
        typedef size_t fleet_t;     ///< Type containing a fleet number.

        /** Create an object (torp/fighter).
            \param obj the object, fully initialized
            \param source who is creating the object */
        virtual void createObject(const Algorithm::Object& obj, ship_t source) = 0;
        /** Destroy an object.
            - if obj is a torpedo: if violent=true, the torpedo hits.
              Its enemy_ptr points to the ship being hit; the destroyObject()
              is immediately followed by a hitShip(). enemy_ptr may also be
              null if the torp would have hit but its target is already dead.
              If violent=false, the torpedo misses.
            - if obj is a fighter: if violent=true, the fighter explodes for
              one of these reasons: an enemy beam (fighter intercept, enemy
              ship; in both cases, the destroyObject() is immediately preceded
              by fireBeam()), or because it wants to return to its base but
              none still existed (no preceding call here!). If violent is
              false, the fighter returns to a baseship.
            \param obj the object being destroyed (still intact)
            \param violent true if object is destroyed violently (torp hits, fighter killed). */
        virtual void destroyObject(const Algorithm::Object& obj, bool violent) = 0;
        /** Destroy ship. Called at the end of a tick, when death is declared. */
        virtual void destroyShip(ship_t unit) = 0;
        /** Fire a beam between two fighters. The beam always hits, obj2
            dies. The respective destroyObject(obj2, true) will be called
            immediately after. */
        virtual void fireBeam(const Algorithm::Object& obj1, const Algorithm::Object& obj2) = 0;
        /** Fire a beam from fighter at ship. The beam always hits, the
            call is immediately followed by a hitShip() call. */
        virtual void fireBeam(const Algorithm::Object& obj1, ship_t unit2) = 0;
        /** Fire beam from ship at fighter. If the beam hits, obj2 dies, and
            destroyObject(obj2, true) will be called immediately after. */
        virtual void fireBeam(ship_t unit1, const Algorithm::Object& obj2, bool hits) = 0;
        /** Fire beam from ship at ship. If the beam hits, the call is
            immediately followed by a hitShip() call. */
        virtual void fireBeam(ship_t unit1, ship_t unit2, bool hits) = 0;
        /** Hit a ship. This is called immerdiately after a weapon hit on a
            ship (fireBeam() for a beam from ship or fighter, destroyObject()
            for a torpedo)
            \param unit ship that got hit
            \param expl,kill weapon parameters
            \param death_flag zero if weapon is death ray */
        virtual void hitShip(ship_t unit, int expl, int kill, int death_flag) = 0;
    };

} } }

#endif
