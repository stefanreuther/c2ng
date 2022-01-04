/**
  *  \file game/actions/buildstarbase.hpp
  *  \brief Class game::actions::BuildStarbase
  */
#ifndef C2NG_GAME_ACTIONS_BUILDSTARBASE_HPP
#define C2NG_GAME_ACTIONS_BUILDSTARBASE_HPP

#include "game/actions/cargocostaction.hpp"
#include "game/cargocontainer.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/map/planet.hpp"

namespace game { namespace actions {

    /** Building starbases.

        This class allows you to build a starbase on a planet (or cancel a build order).
        The cost can be billed to any container, but typically it will be the PlanetStorage.

        At the time of constructing the object, the order must be meaningful
        (i.e. not a build order when the planet is already building),
        but can become meaningless during its lifetime. */
    class BuildStarbase {
     public:
        /** Constructor.
            \param planet Target planet (that receives the starbase)
            \param container Cost are billed to this container
            \param wantBase true to build a base, false to cancel it
            \param config Host configuration (for starbase cost). */
        BuildStarbase(game::map::Planet& planet,
                      CargoContainer& container,
                      bool wantBase,
                      game::config::HostConfiguration& config);

        /** Destructor. */
        ~BuildStarbase();

        /** Commit.
            Must be called at most once.
            \throw Exception if order is invalid (not enough resources) */
        void commit();

        /** Check validity.
            \return true if order is valid (sufficient resources) */
        bool isValid() const;

        /** Access underlying CargoCostAction.
            \return CargoCostAction */
        const CargoCostAction& costAction() const;

     private:
        game::map::Planet& m_planet;
        bool m_wantBase;
        game::config::HostConfiguration& m_hostConfiguration;
        CargoCostAction m_costAction;

        afl::base::SignalConnection m_planetChangeConnection;
        afl::base::SignalConnection m_actionChangeConnection;
        afl::base::SignalConnection m_configChangeConnection;

        void update();
    };

} }

#endif
