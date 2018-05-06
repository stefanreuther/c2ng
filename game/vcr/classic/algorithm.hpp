/**
  *  \file game/vcr/classic/algorithm.hpp
  *  \brief Interface game::vcr::classic::Algorithm
  */
#ifndef C2NG_GAME_VCR_CLASSIC_ALGORITHM_HPP
#define C2NG_GAME_VCR_CLASSIC_ALGORITHM_HPP

#include "game/vcr/object.hpp"
#include "game/vcr/classic/types.hpp"
#include "afl/base/uncopyable.hpp"
#include "afl/base/deletable.hpp"
#include "game/vcr/statistic.hpp"

namespace game { namespace vcr { namespace classic {

    class Visualizer;
    class StatusToken;

    /** Classic VCR algorithm.
        This class is the base for all classic (1:1) VCR player algorithms.
        Its interface includes
        - playing with a visualizer
        - saving and restoring state
        - gather statistical information for simulation

        Playing interface:
        - initBattle() initializes the player for the given VCR.
        - optionally call playFastForward();
        - call playCycle() repeatedly.
          If playCycle() returns true, the battle continues.
          If playCycle() returns false, the battle has ended, and playCycle() did nothing.
          The first thing playCycle() does is incrementing the timer.
        - doneBattle() finishes playing and determines the result.
        This does final exploding and such.

        playBattle(vcr) does all but the last step at once.

        An algorithm must always have an assigned Visualizer,
        although you can change the Visualizer during runtime.
        The Visualizer must live at least as long as the player. */
    class Algorithm : public afl::base::Deletable, private afl::base::Uncopyable {
     public:
        /** Maximum coordinate.
            X coordinates shall be normalized to [0,MAX_COORDINATE). */
        static const int32_t MAX_COORDINATE = 640;

        /** Maximum number of fighter tracks. */
        static const int MAX_FIGHTER_TRACKS = 50;

        /** Constructor.
            \param vis Visualizer */
        explicit Algorithm(Visualizer& vis);

        /** Destructor. */
        virtual ~Algorithm();

        /** Set visualizer.
            \param vis Visualizer */
        void setVisualizer(Visualizer& vis);

        /** Get visualizer. */
        Visualizer& visualizer();

        /** Check battle.
            Limits values in \c left, \c right, \c seed to those this algorithm can handle.
            \param left [in/out] left unit
            \param right [in/out] right unit
            \param seed [in/out] seed
            \retval false everything ok
            \retval true either parameter was modified.
            If this is a host-generated fight, this means it is bogus and should not be played.

            FIXME: change polarity of return value? */
        virtual bool checkBattle(Object& left, Object& right, uint16_t& seed) = 0;

        /** Initialize player.
            \param left [in] left unit
            \param right [in] right unit
            \param seed [in] seed */
        virtual void initBattle(const Object& left, const Object& right, uint16_t seed) = 0;

        /** Finish up fight.
            Performs final explosions on the visualizer and copies status back to the provided objects.
            \param left [out] left unit
            \param right [out] right unit */
        virtual void doneBattle(Object& left, Object& right) = 0;

        /** Set capabilities.
            Returns false if capability set not supported.
            \param cap Capability set */
        virtual bool setCapabilities(uint16_t cap) = 0;

        /** Play one cycle.
            Either does nothing and returns false (last cycle),
            or advances time, does something and returns true. */
        virtual bool playCycle() = 0;

        /** Fast forward.
            If possible, jump forward in time as far as possible.
            Does not need to keep the display up-to-date. */
        virtual void playFastForward() = 0;

        /** Play battle.
            Shortcut for playing a whole battle at once.
            Does not necessarily keep display up-to-date.
            \param left [in] left unit
            \param right [in] right unit
            \param seed [in] seed */
        void playBattle(const Object& left, const Object& right, uint16_t seed);

        /** \name Accessor Interface */
        //@{

        /** Get beam status.
            \param side side
            \param id beam number, starting at 0
            \return value in range [0, 100] (uncharged .. fully charged) */
        virtual int getBeamStatus(Side side, int id) = 0;

        /** Get torpedo status.
            \param side side
            \param id launcher number, starting at 0
            \return value in range [0, 100] (uncharged .. fully charged) */
        virtual int getLauncherStatus(Side side, int id) = 0;

        /** Get torpedo count (number of torps on ship).
            \param side side
            \return number of torpedoes */
        virtual int getNumTorpedoes(Side side) = 0;

        /** Get fighter count (number of fighters on ship).
            \param side side
            \return number of fighters */
        virtual int getNumFighters(Side side) = 0;

        /** Get shield status.
            \param side side
            \return value in range [0, 100] (no shields .. full shields, as in VGAP rules docs) */
        virtual int getShield(Side side) = 0;

        /** Get damage level.
            \param side side
            \return damage level.
            Suggested range is [0, 9999].
            VGAP rules allow damage levels up to 99 or 150, but allowing 9999 makes a Cube-vs.-NFC fight look much more impressive. */
        virtual int getDamage(Side side) = 0;

        /** Get crew.
            \param side side
            \return number of crewmen */
        virtual int getCrew(Side side) = 0;

        /** Get Fighter Position.
            \param side side
            \param id track number
            \return position. Positions are [0, 640]. FIXME: range */
        virtual int getFighterX(Side side, int id) = 0;

        /** Get Fighter Status.
            \param side side
            \param id track number */
        virtual FighterStatus getFighterStatus(Side side, int id) = 0;

        /** Get Object Position.
            \return position. Positions are [0, 640]. FIXME: range */
        virtual int getObjectX(Side side) = 0;

        /** Get distance in meters. */
        virtual int32_t getDistance() = 0;

        /** Get a status token.
            The status token can later be passed back to restoreStatus() to restart playing at that place.
            \return newly-allocated status token */
        virtual StatusToken* createStatusToken() = 0;

        /** Set status.
            \param token status token as returned by createStatusToken(). */
        virtual void restoreStatus(const StatusToken& token) = 0;

        /** Get current time in battle ticks.
            PCC traditionally renders battle ticks as seconds. */
        virtual Time_t getTime() = 0;

        /** Get battle result. */
        virtual BattleResult_t getResult() = 0;

        /** Get battle statistic.
            \param side side */
        virtual Statistic getStatistic(Side side) = 0;

     private:
        Visualizer* m_pVisualizer;
    };

} } }

#endif
