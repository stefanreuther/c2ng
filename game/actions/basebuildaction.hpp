/**
  *  \file game/actions/basebuildaction.hpp
  *  \brief Class game::actions::BaseBuildAction
  */
#ifndef C2NG_GAME_ACTIONS_BASEBUILDACTION_HPP
#define C2NG_GAME_ACTIONS_BASEBUILDACTION_HPP

#include "afl/base/signal.hpp"
#include "game/actions/cargocostaction.hpp"
#include "game/cargocontainer.hpp"
#include "game/map/planet.hpp"
#include "game/root.hpp"
#include "game/spec/cost.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace actions {

    class BaseBuildExecutor;

    /** Generic starbase building action (build things with a starbase).
        This is the common part for everything that builds starship components and tech levels.
        Descendants will override the perform() methods to report what they are trying to do on a BaseBuildExecutor.

        BaseBuildAction will verify:
        - sufficient resources on planet
        - permitted tech level increase

        Descendant must verify:
        - new ranges are valid (e.g. hull exists; not selling more of a component than allowed)
        - tech constraints are satisfied

        Descendant must call update() whenever some input parameters change.
        In particular, it must call update() from the constructor if the constructor starts out with a nonzero action.
        This will cause the cost being recomputed.
        Cost parameters can be accessed using costAction(). */
    class BaseBuildAction : public afl::base::Deletable {
     public:
        enum Status {
            Success,
            MissingResources,
            DisallowedTech
        };

        /** Constructor.
            \param planet    Planet to work on. Must have a played starbase.
            \param container Container to bill the builds on. Usually a PlanetStorage for the same planet.
            \param shipList  Ship list. Needed to access component costs and hull slots.
            \param root      Game root. Needed to access host configuration and registration key. */
        BaseBuildAction(game::map::Planet& planet,
                        CargoContainer& container,
                        game::spec::ShipList& shipList,
                        Root& root);

        /** Destructor. */
        ~BaseBuildAction();

        /** Recompute.
            Call whenever some input parameters change. */
        void update();

        /** Get status.
            This function will call update() and thus perform().
            \return status */
        Status getStatus();

        /** Check validity.
            This function will call update() and thus perform().
            \return true if this action can be committed */
        bool isValid();

        /** Commit.
            \throw game::Exception if this action is not valid */
        void commit();

        /** Access underlying CargoCostAction.
            \return CargoCostAction */
        const CargoCostAction& costAction() const;

        /** Access underlying ship list.
            \return ship list */
        game::spec::ShipList& shipList() const;

        /** Access underlying host configuration.
            \return host configuration */
        game::config::HostConfiguration& hostConfiguration() const;

        /** Access underlying registration key.
            \return registration key */
        RegistrationKey& registrationKey() const;

        /** Access target planet.
            \return planet */
        game::map::Planet& planet() const;

        /** Perform configured action.
            This function must call BaseBuildExecutor's methods to describe the current action.
            It must not modify the underlying units.
            \param exec Target */
        virtual void perform(BaseBuildExecutor& exec) = 0;

        /** Change signal.
            Raised whenever anything changes. */
        afl::base::Signal<void()> sig_change;

     private:
        game::map::Planet& m_planet;
        game::spec::ShipList& m_shipList;
        Root& m_root;
        CargoCostAction m_costAction;
        bool m_needInaccessibleTech;

        afl::base::SignalConnection conn_planetChange;
        afl::base::SignalConnection conn_shipListChange;
        afl::base::SignalConnection conn_configChange;
    };

} }

inline const game::actions::CargoCostAction&
game::actions::BaseBuildAction::costAction() const
{
    return m_costAction;
}

inline game::spec::ShipList&
game::actions::BaseBuildAction::shipList() const
{
    return m_shipList;
}

inline game::config::HostConfiguration&
game::actions::BaseBuildAction::hostConfiguration() const
{
    return m_root.hostConfiguration();
}

inline game::RegistrationKey&
game::actions::BaseBuildAction::registrationKey() const
{
    return m_root.registrationKey();
}

inline game::map::Planet&
game::actions::BaseBuildAction::planet() const
{
    return m_planet;
}

#endif
