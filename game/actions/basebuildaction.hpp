/**
  *  \file game/actions/basebuildaction.hpp
  *  \brief Class game::actions::BaseBuildAction
  */
#ifndef C2NG_GAME_ACTIONS_BASEBUILDACTION_HPP
#define C2NG_GAME_ACTIONS_BASEBUILDACTION_HPP

#include "afl/base/signal.hpp"
#include "afl/string/translator.hpp"
#include "game/actions/cargocostaction.hpp"
#include "game/cargocontainer.hpp"
#include "game/map/planet.hpp"
#include "game/root.hpp"
#include "game/spec/cost.hpp"
#include "game/spec/costsummary.hpp"
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
        /** Status of the action. */
        enum Status {
            Success,                 ///< Success (no impediments found).
            MissingResources,        ///< Not enough resources.
            DisallowedTech,          ///< Disallowed tech level (BaseBuildExecutor::setBaseTechLevel called with tech above allowed by registration).
            ForeignHull,             ///< Foreign hull required (BaseBuildExecutor::accountHull called).
            DisabledTech             ///< Tech upgrade required but disabled using setUseTechUpgrade().
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

        /** Check permission to use tech upgrades.
            \return permission
            \see setUseTechUpgrade */
        bool isUseTechUpgrade() const;

        /** Set permission to use tech upgrades.
            By default, this is enabled, and tech upgrades are implicitly performed.
            When this is disabled, and a tech upgrade is required, the request will fail and report status DisabledTech;
            tech upgrades will not be included in cost.
            \param b New status */
        void setUseTechUpgrade(bool b);

        /** Set reserved mineral amount.
            This amount will not be spent by this action.
            Use if the action is a nested transaction.
            \param cost Reserved amount
            \see CargoCostAction::setReservedAmount */
        void setReservedAmount(game::spec::Cost cost);

        /** Access underlying CargoCostAction.
            \return CargoCostAction */
        const CargoCostAction& costAction() const;

        /** Get cost summary.
            Adds all items for the currently-selected build order to the given CostSummary.
            \param [out] result Result
            \param [in]  tx Translator */
        void getCostSummary(game::spec::CostSummary& result, afl::string::Translator& tx);

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
        int m_impediments;
        bool m_useTechUpgrades;
        bool m_inUpdate;

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
