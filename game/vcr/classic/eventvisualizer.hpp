/**
  *  \file game/vcr/classic/eventvisualizer.hpp
  *  \brief Class game::vcr::classic::EventVisualizer
  */
#ifndef C2NG_GAME_VCR_CLASSIC_EVENTVISUALIZER_HPP
#define C2NG_GAME_VCR_CLASSIC_EVENTVISUALIZER_HPP

#include "game/config/hostconfiguration.hpp"
#include "game/playerlist.hpp"
#include "game/spec/shiplist.hpp"
#include "game/teamsettings.hpp"
#include "game/vcr/classic/eventlistener.hpp"
#include "game/vcr/classic/visualizer.hpp"
#include "game/vcr/object.hpp"

namespace game { namespace vcr { namespace classic {

    class Algorithm;
    class Battle;

    /** Event-based Visualizer.
        This implements the Visualizer interface to produce events on an EventListener.
        Therefore, it calls back into Algorithm to produce the desired events. */
    class EventVisualizer : public Visualizer {
     public:
        /** Constructor.
            \param listener Target */
        explicit EventVisualizer(EventListener& listener);

        /** Destructor. */
        ~EventVisualizer();

        /** Initialize.
            Call this first before any playback.
            This initializes playback (Algorithm::initBattle()) and generates the placeObject() callbacks.
            \param algo Battle algorithm
            \param battle Battle to play
            \param shipList Ship List
            \param players Player list
            \param teams Team settings
            \param config Host configuration */
        void init(Algorithm& algo,
                  const Battle& battle,
                  const game::spec::ShipList& shipList,
                  const PlayerList& players,
                  const TeamSettings* teams,
                  const game::config::HostConfiguration& config);

        /** Play single cycle.
            Calls Algorithm::playCycle() and generates all needed event callbacks.
            If the fight ended, calls Algorithm::doneBattle() and generates callbacks.
            \param algo Battle algorithm
            \retval true Produced data for a battle tick
            \retval false Battle has ended, final callbacks generated */
        bool playCycle(Algorithm& algo);

        /** Refresh after jump.
            Generates updateObject() etc. callbacks after an unspecified change to the battle (e.g. fast forward, rewind).
            \param algo Battle algorithm
            \param done true if the battle had ended (inverse of playCycle() return value) */
        void refresh(Algorithm& algo, bool done);

        // Visualizer:
        virtual void startFighter(Algorithm& algo, Side side, int track);
        virtual void landFighter(Algorithm& algo, Side side, int track);
        virtual void killFighter(Algorithm& algo, Side side, int track);
        virtual void fireBeam(Algorithm& algo, Side side, int track, int target, int hit, int damage, int kill);
        virtual void fireTorpedo(Algorithm& algo, Side side, int hit, int launcher);
        virtual void updateBeam(Algorithm& algo, Side side, int id);
        virtual void updateLauncher(Algorithm& algo, Side side, int id);
        virtual void killObject(Algorithm& algo, Side side);

     private:
        EventListener& m_listener;

        struct UnitState {
            int damage;
            int crew;
            int shield;
            int position;
            int numTorpedoes;
            int numFighters;
            int maxFighterTrack;
            int numBeams;
            int numLaunchers;
            UnitState()
                : damage(0), crew(0), shield(0), position(0), numTorpedoes(0), numFighters(0), maxFighterTrack(0),
                  numBeams(0), numLaunchers(0)
                { }
        };
        UnitState m_unitState[2];

        void initSide(Side side, Algorithm& algo,
                      const Object& obj,
                      const game::spec::ShipList& shipList,
                      const PlayerList& players,
                      const TeamSettings* teams,
                      const game::config::HostConfiguration& config);
        void updateSide(Side side, Algorithm& algo);
        void refreshSide(Side side, Algorithm& algo);
        EventListener::HitEffect getHitEffect(Algorithm& algo, Side side);
    };

} } }

#endif
