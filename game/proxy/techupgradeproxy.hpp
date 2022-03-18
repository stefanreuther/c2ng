/**
  *  \file game/proxy/techupgradeproxy.hpp
  *  \brief Class game::proxy::TechUpgradeProxy
  */
#ifndef C2NG_GAME_PROXY_TECHUPGRADEPROXY_HPP
#define C2NG_GAME_PROXY_TECHUPGRADEPROXY_HPP

#include "afl/base/signal.hpp"
#include "game/actions/techupgrade.hpp"
#include "game/session.hpp"
#include "game/spec/cost.hpp"
#include "game/types.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Bidirectional proxy for tech upgrades.
        Proxies a game::actions::TechUpgrade.

        Bidirectional, synchronous:
        - getStatus

        Bidirectional, asynchronous:
        - modify action
        - commit the action
        - status update (action/universe changed) */
    class TechUpgradeProxy {
     public:
        /** Status summary. */
        struct Status {
            game::spec::Cost cost;        ///< Total cost of action. @see game::spec::CargoCostAction::getCost()
            game::spec::Cost available;   ///< Available amounts. @see game::actions::CargoCostAction::getAvailableAmountAsCost()
            game::spec::Cost remaining;   ///< Remaining amounts. @see game::actions::CargoCostAction::getRemainingAmount()
            game::spec::Cost missing;     ///< Missing amounts. @see game::actions::CargoCostAction::getMissingAmount()
            int min[NUM_TECH_AREAS];      ///< Minimum valid tech levels. @see game::actions::TechUpgrade::getMinTechLevel()
            int max[NUM_TECH_AREAS];      ///< Maximum valid tech levels. @see game::actions::TechUpgrade::getMaxTechLevel()
            int current[NUM_TECH_AREAS];  ///< Current tech level. @see game::actions::TechUpgrade::getTechLevel()
            game::actions::TechUpgrade::Status status;  ///< Status of transaction. @see game::actions::TechUpgrade::getStatus()

            Status()
                : cost(), available(), remaining(), missing(), status(game::actions::BaseBuildAction::DisallowedTech)
                {
                    afl::base::Memory<int>(min).fill(0);
                    afl::base::Memory<int>(max).fill(0);
                    afl::base::Memory<int>(current).fill(0);
                }
        };

        /** Structure for setAll(). */
        struct Order {
            int values[NUM_TECH_AREAS];   ///< New tech levels.
        };

        /** Constructor.
            \param gameSender Game sender
            \param receiver   RequestDispatcher to receive updates in this thread
            \param planetId   Planet Id. Planet must satisfy a TechUpgrade action */
        TechUpgradeProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& receiver, Id_t planetId);
        ~TechUpgradeProxy();

        /** Get status, synchronously.
            \param [in,out] ind    WaitIndicator for UI synchronisation
            \param [out]    result Result */
        void getStatus(WaitIndicator& ind, Status& result);

        /** Set all levels at once.
            This is not guaranteed to be atomic, but still saves roundtrip times.
            \param order New levels */
        void setAll(const Order& order);

        /** Set new target tech level.
            Invalid requests are silently ignored.
            Otherwise, both upgrades and downgrades will be performed.
            \param area Area
            \param value New level
            \see game::actions::TechUpgrade::setTechLevel */
        void setTechLevel(TechLevel area, int value);

        /** Upgrade to new target tech level.
            Like setTechLevel(), but will never lower a tech level.
            \param area Area
            \param value New level
            \see game::actions::TechUpgrade::upgradeTechLevel */
        void upgradeTechLevel(TechLevel area, int value);

        /** Set reserved mineral amount.
            This amount will not be spent by this action.
            Use if the action is a nested transaction.
            \param cost Reserved amount
            \see BaseBuildAction::setReservedAmount */
        void setReservedAmount(game::spec::Cost cost);

        /** Commit transaction.
            Will perform all configured upgrades and downgrades. */
        void commit();

        /** Signal: action updates.
            \param status New action status summary */
        afl::base::Signal<void(const Status&)> sig_change;

     private:
        class Trampoline;
        class TrampolineFromSession;
        util::RequestReceiver<TechUpgradeProxy> m_receiver;
        util::RequestSender<Trampoline> m_sender;
    };

} }

#endif
