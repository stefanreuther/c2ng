/**
  *  \file game/proxy/buildshipproxy.hpp
  *  \brief Class game::proxy::BuildShipProxy
  */
#ifndef C2NG_GAME_PROXY_BUILDSHIPPROXY_HPP
#define C2NG_GAME_PROXY_BUILDSHIPPROXY_HPP

#include <vector>
#include "afl/base/signal.hpp"
#include "afl/string/string.hpp"
#include "game/session.hpp"
#include "game/spec/cost.hpp"
#include "game/types.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"
#include "afl/data/stringlist.hpp"
#include "game/actions/buildship.hpp"
#include "game/spec/costsummary.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Bidirectional proxy for ship building.
        Proxies a game::actions::BuildShip and some related functions.

        In addition, it manages a "current part" which is used to report the current part cost
        (function selectPart, Status::partCost).

        Bidirectional synchronous:
        - getStatus
        - getCostSummary
        - findShipCloningHere

        Bidirectional asynchronous:
        - modify action
        - commit the action
        - cancel a build order */
    class BuildShipProxy {
     public:
        typedef game::actions::BuildShip::Weapon Weapon_t;

        /** Action status summary. */
        struct Status {
            // Status
            game::actions::BuildShip::Status status;  ///< Overall status of action. @see game::actions::BaseBuildAction::getStatus()

            // Cost
            game::spec::Cost totalCost;               ///< Total cost of ship build order. @see game::spec::CargoCostAction::getCost()
            game::spec::Cost partCost;                ///< Cost of selected part. @see game::spec::Component::cost()
            game::spec::Cost available;               ///< Available amounts. @see game::actions::CargoCostAction::getAvailableAmountAsCost()
            game::spec::Cost remaining;               ///< Remaining amounts. @see game::actions::CargoCostAction::getRemainingAmount()
            game::spec::Cost missing;                 ///< Missing amounts. @see game::actions::CargoCostAction::getMissingAmount()

            // Tech levels
            int partTech;                             ///< Tech level of selected part.
            int availableTech;                        ///< Corresponding tech level of starbase.

                                                      // Order
            ShipBuildOrder order;                     ///< Ship build order in raw form.
            afl::data::StringList_t description;      ///< Description of ship build order in textual form. @see game::ShipBuildOrder::describe()
            int numEngines;                           ///< Number of engines.
            int maxBeams;                             ///< Maximum number of beams. Actual number is in @c order.
            int maxLaunchers;                         ///< Maximum number of launchers. Actual number is in @c order.
            bool isNew;                               ///< true if this is a new order (base is not currently building).
            bool isUsePartsFromStorage;               ///< Status of use-parts-from-storage flag.
            bool isChange;                            ///< true if this a change to a pre-existing build order.

            Status()
                : status(game::actions::BuildShip::MissingResources), totalCost(), partCost(), available(), remaining(), missing(), partTech(), availableTech(), order(), description(),
                  numEngines(), maxBeams(), maxLaunchers(), isNew(false), isUsePartsFromStorage(false), isChange(false)
                { }
        };

        /** Constructor.
            \param gameSender Game sender
            \param receiver   RequestDispatcher to receive updates in this thread
            \param planetId   Planet Id */
        BuildShipProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& receiver, Id_t planetId);

        /** Destructor. */
        ~BuildShipProxy();

        /** Get status, synchronously.
            \param [in/out] ind    WaitIndicator for UI synchronisation
            \param [out]    result Result */
        void getStatus(WaitIndicator& ind, Status& result);

        /** Get cost summary, synchronously.
            \param [in/out] ind    WaitIndicator for UI synchronisation
            \param [out]    result Result, containing cost breakdown in human-readable form  */
        void getCostSummary(WaitIndicator& ind, game::spec::CostSummary& result);

        /** Find ship cloning at this planet.
            \param [in/out] ind    WaitIndicator for UI synchronisation
            \param [out]    id     Ship Id
            \param [out]    name   Ship name
            \retval true  Found a ship; id/name updated
            \retval false No cloning ship found
            \see game::map::Universe::findShipCloningAt */
        bool findShipCloningHere(WaitIndicator& ind, Id_t& id, String_t& name);

        /** Cancel all clone orders at this planet.
            The operation is processed asynchronously.
            \see game::map::cancelAllCloneOrders */
        void cancelAllCloneOrders();

        /** Select part.
            This part's cost is reported in the Status structure.
            \param area Part area
            \param id   Part Id (type) */
        void selectPart(TechLevel area, int id);

        /** Set part in build order.
            \param area Area to change.
            \param id   Part Id (type)
            \see game::actions::BuildShip::setPart */
        void setPart(TechLevel area, int id);

        /** Set number of weapons.
            \param area   Area to change
            \param amount New number. Out-of-range values will be forced into range.
            \see game::actions::BuildShip::setNumParts */
        void setNumParts(Weapon_t area, int amount);

        /** Change number of weapons.
            \param area   Area to change
            \param amount Amount to add. Out-of-range values will be forced into range.
            \see game::actions::BuildShip::addParts */
        void addParts(Weapon_t area, int delta);

        /** Choose whether parts from storage will be used.
            If enabled, ship building will use parts if possible.
            If disabled, everything will be built anew, even when there is already a matching part in storage.
            \param flag new setting
            \see game::actions::BuildShip::setUsePartsFromStorage */
        void setUsePartsFromStorage(bool flag);

        /** Commit the transaction.
            This will build the parts and set the build order.
            \see game::actions::BuildShip::commit */
        void commit();

        /** Cancel a pre-existing build order. */
        void cancel();

        /** Signal: action update. */
        afl::base::Signal<void(const Status&)> sig_change;

     private:
        struct Trampoline;
        class TrampolineFromSession;
        util::RequestReceiver<BuildShipProxy> m_receiver;
        util::RequestSender<Trampoline> m_sender;
    };

} }

#endif
