/**
  *  \file game/vcr/classic/visualizer.hpp
  *  \brief Interface game::vcr::classic::Visualizer
  */
#ifndef C2NG_GAME_VCR_CLASSIC_VISUALIZER_HPP
#define C2NG_GAME_VCR_CLASSIC_VISUALIZER_HPP

#include "game/vcr/classic/types.hpp"
#include "afl/base/deletable.hpp"

namespace game { namespace vcr { namespace classic {

    class Algorithm;

    /** Interface to VCR visualisation.
        A classic VCR player calls this object to report actions.

        Sequences:
        - firing a torpedo: updateLauncher(), fireTorpedo()
        - firing a beam from unit at unit: updateBeam(), fireBeam()
        - firing a beam from unit at fighter: updateBeam(), fireBeam(), optionally killFighter()
        - firing a beam from fighter at unit: fireBeam()
        - firing a beam from fighter at fighter: fireBeam(), optionally killFighter()
        - spurious fighter kill (TimHost bug): killFighter()
        - launching a fighter: startFighter()
        - landing a fighter: landFighter()
        - recharging a beam: updateBeam()
        - recharging a torpedo launcher: updateLauncher()
        - unit or fighter movement: NO CALLBACK.
          Note that movement is at different places depending on host order:
          PHost moves everything last so queries from callbacks see the old state;
          THost moves units first, fighters almost last.
        - unit killed: killObject() */
    class Visualizer : public afl::base::Deletable {
     public:
        /** Start a fighter.
            When this function is called, the fighter's data is already accessible on the Algorithm's accessor interface.
            \param side Owning side
            \param track Fighter track (starting at 0) */
        virtual void startFighter(Algorithm& algo, Side side, int track) = 0;

        /** Land a fighter.
            When this function is called, the fighter's data is still accessible on the Algorithm's accessor interface.
            \param side Owning side
            \param track Fighter track (starting at 0) */
        virtual void landFighter(Algorithm& algo, Side side, int track) = 0;

        /** Kill a fighter.
            When this function is called, the fighter's data is still accessible on the Algorithm's accessor interface.
            \param side Owning side
            \param track Fighter track (starting at 0) */
        virtual void killFighter(Algorithm& algo, Side side, int track) = 0;

        /** Fire a beam.
            This method implements all four cases of beam firings (unit/fighter at unit/fighter).
            - If the beam originates from a unit, its charge state has already been updated on the Algorithm's accessor interface.
            - If this beam hits a unit, the unit's state has already been updated (e.g. new damage).
            - If this beam hits a fighter, the fighter is still presend and its status can still be queried.
            When this function is called, the beam's status has already been updated;
            the unit's status has also been updated and allows you to determine whether damage was done.

            \param side Firing side
            \param track Origin of the beam: >=0 for a fighter on the given track, <0 for the unit's beams ([-1,-N] for a unit with N beams)
            \param target Target of the beam: >=0 for a fighter on the given track, <0 for unit
            \param hit Nonnegative for hit, negative for miss (FIXME: as of 20180306, only +1 and -1 used)
            \param damage Effective damage (Weapon::getDamagePower())
            \param kill Effective kill (Weapon::getKillPower()) */
        virtual void fireBeam(Algorithm& algo, Side side, int track, int target, int hit, int damage, int kill) = 0;

        /** Fire a torpedo.
            When this function is called, the launcher's status has already been updated;
            the unit's status has also been updated and allows you to determine whether damage was done.
            \param side Firing side
            \param hit Nonnegative for hit, negative for miss. You can use this value to modify the visualisation in a deterministic way.
            \param launcher Originating launcher, starting at 0 */
        virtual void fireTorpedo(Algorithm& algo, Side side, int hit, int launcher) = 0;

        /** Update a beam.
            Called whenever a beam is charged or fired.
            When this function is called, the beam's status has already been updated on the Algorithm's accessor interface.
            \param side Side
            \param id Beam number, starting at 0 */
        virtual void updateBeam(Algorithm& algo, Side side, int id) = 0;

        /** Update a torpedo launcher.
            Called whenever a torpedo launcher is charged or fired.
            When this function is called, the launcher's status has already been updated on the Algorithm's accessor interface.
            \param side Side
            \param id Launcher number, starting at 0 */
        virtual void updateLauncher(Algorithm& algo, Side side, int id) = 0;

        /** Kill unit.
            Called at the end of the fight for the losing side(s).
            \param side Losing side */
        virtual void killObject(Algorithm& algo, Side side) = 0;
    };

} } }


#endif
