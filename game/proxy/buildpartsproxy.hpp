/**
  *  \file game/proxy/buildpartsproxy.hpp
  *  \brief Class game::proxy::BuildPartsProxy
  */
#ifndef C2NG_GAME_PROXY_BUILDPARTSPROXY_HPP
#define C2NG_GAME_PROXY_BUILDPARTSPROXY_HPP

#include "afl/base/signal.hpp"
#include "afl/string/string.hpp"
#include "game/actions/buildparts.hpp"
#include "game/session.hpp"
#include "game/spec/cost.hpp"
#include "game/types.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Bidirectional proxy for building starship parts.
        Proxies a game::actions::BuildParts.

        Bidirectional, synchronous:
        - getStatus

        Bidirectional, asynchronous:
        - select a part
        - build/scrap parts
        - commit the action
        - status update (action/universe changed) */
    class BuildPartsProxy {
     public:
        /** Action status. */
        struct Status {
            // Status
            game::actions::BuildParts::Status status;     ///< Status of transaction. @see game::actions::BuildParts::getStatus()

            // Parts
            String_t name;                ///< Name of selected part.
            int numExistingParts;         ///< Number of existing parts. @see game::actions::BuildParts::getNumExistingParts()
            int numParts;                 ///< Number of parts ordered. @see game::actions::BuildParts::getNumParts()

            // Cost
            game::spec::Cost cost;        ///< Total cost of all parts. @see game::spec::CargoCostAction::getCost()
            game::spec::Cost available;   ///< Available amounts. @see game::actions::CargoCostAction::getAvailableAmountAsCost()
            game::spec::Cost remaining;   ///< Remaining amounts. @see game::actions::CargoCostAction::getRemainingAmount()
            game::spec::Cost missing;     ///< Missing amounts. @see game::actions::CargoCostAction::getMissingAmount()

            Status()
                : status(game::actions::BuildParts::DisallowedTech), name(), numExistingParts(0), numParts(0), cost(), available(), remaining(), missing()
                { }
        };

        /** Constructor.
            \param gameSender Game sender
            \param receiver   RequestDispatcher to receive updates in this thread
            \param planetId   Planet Id */
        BuildPartsProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& receiver, Id_t planetId);

        /** Destructor. */
        ~BuildPartsProxy();

        /** Get status, synchronously.
            \param [in/out] ind    WaitIndicator for UI synchronisation
            \param [out]    result Result */
        void getStatus(WaitIndicator& ind, Status& st);

        /** Select part for building.
            Will respond with an update containing the new part costs and count.
            \param area Area
            \param id   Part Id (for hulls, a hull Id, NOT a slot number) */
        void selectPart(TechLevel area, int id);

        /** Buy or scrap some parts.
            Will respond with an update containing the new count and total costs.
            When trying to buy/sell more than the base can hold, the amount is limited;
            exceeding the available resources will report a failing transaction.
            \param amount Amount to buy, negative to sell
            \see game::actions::BuildParts::add */
        void add(int amount);

        /** Commit transaction.
            \see game::actions::BuildParts::commit */
        void commit();

        /** Signal: action updates.
            \param status New action status summary */
        afl::base::Signal<void(const Status&)> sig_change;

     private:
        struct Trampoline;
        class TrampolineFromSession;
        util::RequestReceiver<BuildPartsProxy> m_receiver;
        util::RequestSender<Trampoline> m_sender;
    };

} }

#endif
