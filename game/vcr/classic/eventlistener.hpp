/**
  *  \file game/vcr/classic/eventlistener.hpp
  *  \brief Interface game::vcr::classic::EventListener
  */
#ifndef C2NG_GAME_VCR_CLASSIC_EVENTLISTENER_HPP
#define C2NG_GAME_VCR_CLASSIC_EVENTLISTENER_HPP

#include "afl/base/deletable.hpp"
#include "game/teamsettings.hpp"
#include "game/vcr/classic/types.hpp"
#include "game/vcr/object.hpp"

namespace game { namespace vcr { namespace classic {

    /** Combat event listener.
        This is an extended version of the Visualizer interface.
        Unlike Visualizer, it provides all events with necessary data to visualisation,
        and does not require callbacks into the Algorithm.
        This makes the EventListener interface usable in both game and UI code.

        Callbacks (in particular those that relate to the use of weapons) are intended to be self-contained.
        For example, the "fireTorpedo()" callback comes with all required information,
        saying that this torpedo hit caused the given difference in a unit's torpedo count,
        and a shield/damage/crew difference in the opponent's status.
        This allows these requests to be processed independently.

        An exception are the requests
        - placeObject(): always first in a battle, to initialize the object.
        - updateObject(), updateAmmo(), updateFighter(): always after a discontinuity (FF/REW),
          never during normal playback.

        A regular battle tick consists of a number of event callbacks, followed by updateTime().

        FIXME: consider dropping the Visualizer interface altogether.
        Visualizer essentially dates back to PCC1. */
    class EventListener : public afl::base::Deletable {
     public:
        struct UnitInfo {
            Object object;
            int position;
            String_t ownerName;
            TeamSettings::Relation relation;
            String_t beamName;
            String_t launcherName;
            UnitInfo()
                : object(), position(0), ownerName(), relation(), beamName(), launcherName()
                { }
        };

        struct HitEffect {
            int damageDone;
            int crewKilled;
            int shieldLost;
            HitEffect()
                : damageDone(0), crewKilled(0), shieldLost(0)
                { }
        };

        /** Place an object.
            This starts the battle.
            \param side Side
            \param info Information about the unit. */
        virtual void placeObject(Side side, const UnitInfo& info) = 0;

        /** Update time.
            Each battle tick ends with updateTime().
            \param time Time
            \param distance Distance */
        virtual void updateTime(Time_t time, int32_t distance) = 0;

        /** Start a fighter.
            \param side Owning side
            \param track Fighter track (starting at 0)
            \param position Fighter position [FIXME: range]
            \param distance Distance to base unit
            \param fighterDiff Delta to owning side's fighter count (typically, -1) */
        virtual void startFighter(Side side, int track, int position, int distance, int fighterDiff) = 0;

        /** Land a fighter.
            \param side Owning side
            \param track Fighter track (starting at 0)
            \param fighterDiff Delta to owning side's fighter count (typically, +1) */
        virtual void landFighter(Side side, int track, int fighterDiff) = 0;

        /** Kill a fighter.
            \param side Owning side
            \param track Fighter track (starting at 0) */
        virtual void killFighter(Side side, int track) = 0;

        /** Fire a beam.
            This method implements all four cases of beam firings (unit/fighter at unit/fighter).

            \param side Firing side
            \param track Origin of the beam: >=0 for a fighter on the given track, <0 for the unit's beams ([-1,-N] for a unit with N beams)
            \param target Target of the beam: >=0 for a fighter on the given track, <0 for unit
            \param hit Nonnegative for hit, negative for miss
            \param damage Effective damage (Weapon::getDamagePower())
            \param kill Effective kill (Weapon::getKillPower())
            \param effect Effect of the weapon hit upon the enemy side (will be all-zero when hitting a fighter) */
        virtual void fireBeam(Side side, int track, int target, int hit, int damage, int kill, const HitEffect& effect) = 0;

        /** Fire a torpedo.
            \param side Firing side
            \param hit Nonnegative for hit, negative for miss. You can use this value to modify the visualisation in a deterministic way.
            \param launcher Originating launcher, starting at 0
            \param torpedoDiff Delta to owning side's torpedo count (typically, -1)
            \param effect Effect of the weapon hit upon the enemy side */
        virtual void fireTorpedo(Side side, int hit, int launcher, int torpedoDiff, const HitEffect& effect) = 0;

        /** Update a beam.
            \param side Side
            \param id Beam number, starting at 0
            \param value New status in range [0, 100] */
        virtual void updateBeam(Side side, int id, int value) = 0;

        /** Update a torpedo launcher.
            \param side Side
            \param id Launcher number, starting at 0
            \param value New status in range [0, 100] */
        virtual void updateLauncher(Side side, int id, int value) = 0;

        /** Move a unit.
            \param side Side
            \param position New position [FIXME: range] */
        virtual void moveObject(Side side, int position) = 0;

        /** Move a fighter.
            \param side Owning side
            \param track Fighter track
            \param position New position [FIXME: range]
            \param distance Distance to base unit
            \param status Fighter status */
        virtual void moveFighter(Side side, int track, int position, int distance, FighterStatus status) = 0;

        /** Kill unit.
            \param side Side */
        virtual void killObject(Side side) = 0;

        /** Update unit status after discontinuity.
            \param side Side
            \param damage Damage
            \param crew Crew
            \param shield Shield */
        virtual void updateObject(Side side, int damage, int crew, int shield) = 0;

        /** Update ammo after discontinuity.
            \param side Side
            \param numTorpedoes Number of torpedoes
            \param numFighters Number of fighters */
        virtual void updateAmmo(Side side, int numTorpedoes, int numFighters) = 0;

        /** Update fighter after discontinuity.
            \param side Side
            \param track Fighter track
            \param position New position [FIXME: range]
            \param distance Distance to base unit
            \param status Fighter status (can also be FighterIdle to mark this track unoccupied) */
        virtual void updateFighter(Side side, int track, int position, int distance, FighterStatus status) = 0;

        /** Set battle result.
            This is the final report of a battle (only followed by the last time report).
            \param result Result status */
        virtual void setResult(BattleResult_t result) = 0;
    };

} } }

#endif
