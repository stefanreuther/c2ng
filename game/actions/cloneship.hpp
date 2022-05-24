/**
  *  \file game/actions/cloneship.hpp
  *  \brief Class game::actions::CloneShip
  */
#ifndef C2NG_GAME_ACTIONS_CLONESHIP_HPP
#define C2NG_GAME_ACTIONS_CLONESHIP_HPP

#include "afl/string/translator.hpp"
#include "game/actions/cargocostaction.hpp"
#include "game/actions/techupgrade.hpp"
#include "game/interpreterinterface.hpp"
#include "game/map/configuration.hpp"
#include "game/map/planet.hpp"
#include "game/map/planetstorage.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/root.hpp"
#include "game/shipbuildorder.hpp"
#include "game/unitscoredefinitionlist.hpp"
#include "util/randomnumbergenerator.hpp"

namespace game { namespace actions {

    /** Cloning a ship.
        Wraps a few precondition checks around the "cln" friendly code.

        For a successful clone, tech levels need to be set appropriately,
        and enough resources need to be available at time of the clone.
        This action will upgrade tech levels (and fail if that cannot be done),
        but will allow submitting a clone order without sufficient resources.

        For informational purposes, this class publishes:
        - getOrderStatus(): classify the clone request, in particular, whether it can be built normally;
        - getPaymentStatus(): determine whether request can be paid; this can prevent a commit;
        - getCloneAction(): a CargoCostAction reporting the total cost of the clone;
        - getTechUpgradeAction(): a CargoCostAction reporting the cost of the required tech upgrades.

        To use this action,
        - construct;
        - check status;
        - for CanClone, call commit();
        - for CanBuild, make a BuildShip action and give it the ship build order (getBuildOrder()).

        For now, this is a one-shot action that does not provide any events. */
    class CloneShip {
     public:
        /** Overall order status.
            Determines the further action flow. */
        enum OrderStatus {
            /** Success: regular clone.
                Verify PaymentStatus, then call commit(). */
            CanClone,

            /** Success: can build regularily (but cannot clone).
                Make a BuildShip action and give it the ship build order (getBuildOrder()), which checks further preconditions. */
            CanBuild,

            /** Failure: cloning is forbidden, period (THost rule or not registered). */
            PlayerCannotClone,

            /** Failure: ship is unclonable (hull function). */
            ShipIsUnclonable,

            /** Failure: remote owner cannot clone (but can build normally). */
            RemoteOwnerCanBuild,

            /** Failure: tech limit exceeded. */
            TechLimitExceeded
        };

        /** Payment status.
            For CanClone, determines whether the order can be paid (and committed). */
        enum PaymentStatus {
            /** Success: can pay the entire order.
                commit() will succeed. */
            CanPay,

            /** Partial: can pay tech, but not components.
                commit() will succeed. */
            CannotPayComponents,

            /** Failure: cannot even pay tech.
                commit() will fail. */
            CannotPayTech
        };

        /** Conflict status. */
        enum ConflictStatus {
            /** No conflict found. */
            NoConflict,

            /** Starbase is already building a ship. */
            IsBuilding,

            /** Another ship is already trying to clone here. */
            IsCloning
        };
        struct Conflict {
            int id;              ///< Id of ship trying to clone, or hull number of ship being built.
            String_t name;       ///< Name of ship trying to clone, or hull name of ship being built.
            Conflict()
                : id(), name()
                { }
        };


        /** Constructor.
            @param planet        Planet to work on; must be a played planet with starbase
            @param ship          Ship to clone; must be a played ship
            @param univ          Universe (for conflicting clone orders)
            @param shipScores    Ship score definitions (for hull functions)
            @param shipList      Ship list (for components, friendly codes, hull functions)
            @param root          Root (for host version, host configuration)
            @throw Exception preconditions not fulfilled (note that location and ownership are not verified!) */
        CloneShip(game::map::Planet& planet,
                  game::map::Ship& ship,
                  game::map::Universe& univ,
                  const UnitScoreDefinitionList& shipScores,
                  game::spec::ShipList& shipList,
                  Root& root);

        /** Commit.
            This will set the "cln" friendly code.
            - getPaymentStatus() should be CanPay or CannotPayComponents (checked)
            - getOrderStatus() should be CanClone (not checked)
            @param mapConfig Map configuration (for clearing waypoints)
            @param rng random number generator (for invalidating other clone orders)
            @throw Exception wrong payment status (tech cannot be paid) */
        void commit(const game::map::Configuration& mapConfig, util::RandomNumberGenerator& rng);

        /** Get build order.
            This build order can be used to build a clone of the ship.
            @return build order (always valid) */
        ShipBuildOrder getBuildOrder() const;

        /** Get order status.
            Order status determines how to proceed (commit(), getBuildOrder()) but is not interlocked.
            @return order status */
        OrderStatus getOrderStatus() const;

        /** Get payment status.
            Payment status determines whether the action can be committed.
            @return payment status */
        PaymentStatus getPaymentStatus() const;

        /** Get clone action.
            @return CargoCostAction reporting the total cost of the clone (tech plus components) */
        const CargoCostAction& getCloneAction() const;

        /** Get tech upgrade action.
            @return CargoCostAction reporting the cost of the required tech upgrades.
                    Note that if order status is TechLimitExceeded, this may not include all upgrades. */
        const CargoCostAction& getTechUpgradeAction() const;

        /** Check for conflicting orders.
            @param [out] result If non-null, information about the conflict is placed here
            @param [in]  tx     Translator (for generating resulting name)
            @param [in]  iface  Interface (for generating resulting name) */
        ConflictStatus findConflict(Conflict* result, afl::string::Translator& tx, InterpreterInterface& iface) const;

        /** Check whether ship has the CloneOnce ability.
            This means the copy will be Unclonable.
            @return ship has CloneOnce ability */
        bool isCloneOnce() const;

        /** Access this action's ship.
            @return ship */
        const game::map::Ship& ship() const;

        /** Access this action's planet.
            @return planet */
        const game::map::Planet& planet() const;

     private:
        void update();

        game::map::Ship& m_ship;
        game::map::Planet& m_planet;
        game::map::Universe& m_universe;
        const Root& m_root;
        const game::spec::ShipList& m_shipList;
        const UnitScoreDefinitionList& m_shipScores;

        /* We will keep two transactions open, so we need two PlanetStorage instances. */
        game::map::PlanetStorage m_costPlanet;
        CargoCostAction m_costAction;

        game::map::PlanetStorage m_techPlanet;
        TechUpgrade m_techUpgrade;

        game::spec::Cost m_shipCost;

        bool m_techFailure;
    };

} }

#endif
