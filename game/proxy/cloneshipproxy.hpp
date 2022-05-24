/**
  *  \file game/proxy/cloneshipproxy.hpp
  *  \brief Class game::proxy::CloneShipProxy
  */
#ifndef C2NG_GAME_PROXY_CLONESHIPPROXY_HPP
#define C2NG_GAME_PROXY_CLONESHIPPROXY_HPP

#include "game/actions/cloneship.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Proxy for a CloneShip action.

        Bidirectional, synchronous:
        - retrieve status of action (getStatus())

        Asynchronous:
        - commit action

        To use,
        - construct
        - use getStatus() and perform UI
        - if user confirms, call commit() */
    class CloneShipProxy {
     public:
        /** Action status.
            Collects all status information required for performing UI. */
        struct Status {
            bool valid;                                                   ///< Validity flag. Following values are only valid if this is true.
            bool isInFleet;                                               ///< true if ship is part of a fleet (as leader or member).
            bool isCloneOnce;                                             ///< true if ship is clonable once only.

            Id_t planetId;                                                ///< Planet Id for starbase being used.
            game::ShipBuildOrder buildOrder;                              ///< Ship build order. @see game::actions::CloneShip::getBuildOrder().
            game::actions::CloneShip::OrderStatus orderStatus;            ///< Overall order status. @see game::actions::CloneShip::getOrderStatus().
            game::actions::CloneShip::PaymentStatus paymentStatus;        ///< Payment status. @see game::actions::CloneShip::getPaymentStatus().
            game::spec::Cost cost;                                        ///< Total cost of action. @see game::actions::CargoCostAction::getCost(), game::actions::CloneShip::getCloneAction().
            game::spec::Cost available;                                   ///< Available amounts. @see game::actions::CargoCostAction::getAvailableAmountAsCost(), game::actions::CloneShip::getCloneAction().
            game::spec::Cost remaining;                                   ///< Remaining amounts. @see game::actions::CargoCostAction::getRemainingAmountAsCost(), game::actions::CloneShip::getCloneAction().
            game::spec::Cost missing;                                     ///< Missing amounts. @see game::actions::CargoCostAction::getMissingAmountAsCost(), game::actions::CloneShip::getCloneAction().
            game::spec::Cost techCost;                                    ///< Tech cost. @see game::actions::CargoCostAction::getCost(), game::actions::CloneShip::getTechUpgradeAction().
            game::actions::CloneShip::ConflictStatus conflictStatus;      ///< Conflict type. @see game::actions::CloneShip::findConflict().
            game::actions::CloneShip::Conflict conflict;                  ///< Conflict details. @see game::actions::CloneShip::findConflict().

            Status()
                : valid(false), isInFleet(), isCloneOnce(), planetId(), buildOrder(), orderStatus(), paymentStatus(),
                  cost(), available(), remaining(), missing(), techCost(), conflictStatus(), conflict()
                { }
        };

        /** Constructor.
            Create a CloneShipProxy for a given ship.
            The ship must be played and orbiting a matching starbase.
            If preconditions are not fulfilled, the CloneShipProxy will remain passive.

            @param gameSender Game sender
            @param shipId     Ship Id */
        CloneShipProxy(util::RequestSender<Session> gameSender, Id_t shipId);

        /** Destructor. */
        ~CloneShipProxy();

        /** Retrieve status.
            @param [in,out] ind WaitIndicator for UI synchronisation
            @param [out]    st  Status */
        void getStatus(WaitIndicator& ind, Status& st);

        /** Commit action.
            @see game::actions::CloneShip::commit() */
        void commit();

     private:
        class Trampoline;
        class TrampolineFromSession;
        util::RequestSender<Trampoline> m_sender;
    };

} }

#endif
