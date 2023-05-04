/**
  *  \file game/sim/session.hpp
  *  \brief Class game::sim::Session
  */
#ifndef C2NG_GAME_SIM_SESSION_HPP
#define C2NG_GAME_SIM_SESSION_HPP

#include "afl/base/refcounted.hpp"
#include "game/playerbitmatrix.hpp"
#include "game/sim/configuration.hpp"
#include "game/sim/gameinterface.hpp"
#include "game/sim/setup.hpp"

namespace game { namespace sim {

    /** Simulator session.
        Contains all information to set up (but not run) a battle simulation.

        The simulator session is independant from the game.
        In particular, there is no automatic "push" of game-side changes into the simulator.
        Game changes are based on "pull" principle, that is,
        - copy planet/ship information using GameInterface methods
        - apply player relations using usePlayerRelations() */
    class Session : public afl::base::RefCounted {
     public:
        /** Constructor.
            Make an empty session. */
        Session();

        /** Destructor. */
        ~Session();

        /** Access setup (mutable).
            @return setup */
        Setup& setup();

        /** Access setup (const).
            @return setup */
        const Setup& setup() const;

        /** Access configuration (mutable).
            @return configuration */
        Configuration& configuration();

        /** Access configuration (const).
            @return configuration */
        const Configuration& configuration() const;

        /** Set GameInterface instance.
            The GameInterface provides information about the actual game.
            @param gi Newly-allocated instance. Session takes ownership. Can be null. */
        void setNewGameInterface(GameInterface* gi);

        /** Get GameInterface instance.
            @return instance, owned by Session; can be null */
        GameInterface* getGameInterface() const;

        /** Configure use of game's player relations.
            If enabled, usePlayerRelations() will use the actual game's relations;
            if disabled, configuration remains unchanged.
            This function call only configures the settings, but does not yet use them.
            @param flag Flag */
        void setUsePlayerRelations(bool flag);

        /** Check whether use of game's player relations is enabled.
            @return flag as last set using setUsePlayerRelations(). */
        bool isUsePlayerRelations() const;

        /** Get player relations.
            If a GameInterface is present, uses it to retrieve values.
            Otherwise, returns empty matrices.
            @param [out] alliances Alliance relations
            @param [out] enemies   Enemy relations */
        void getPlayerRelations(PlayerBitMatrix& alliances, PlayerBitMatrix& enemies) const;

        /** Use player relations.
            If use of player relations is enabled, updates the configuration accordingly.
            Should be called whenever the simulation editor is opened. */
        void usePlayerRelations();

     private:
        Setup m_setup;
        Configuration m_config;
        std::auto_ptr<GameInterface> m_gameInterface;
        bool m_usePlayerRelations; // ex WSimScreen::sim_default_relations
    };

} }

#endif
