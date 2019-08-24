/**
  *  \file client/proxy/taxationproxy.hpp
  *  \brief Class client::proxy::TaxationProxy
  */
#ifndef C2NG_CLIENT_PROXY_TAXATIONPROXY_HPP
#define C2NG_CLIENT_PROXY_TAXATIONPROXY_HPP

#include "afl/base/signal.hpp"
#include "client/downlink.hpp"
#include "game/actions/taxationaction.hpp"
#include "game/session.hpp"
#include "util/requestsender.hpp"
#include "util/slaverequestsender.hpp"

namespace client { namespace proxy {

    /** Taxation proxy.
        This proxies a game::actions::TaxationAction.

        Bidirectional synchronous: getStatus().

        Bidirectional asynchronous: changing taxes and receiving results

        - construct a TaxationProxy
        - configure it (setNumBuildings())
        - use getStatus() to obtain initial status including available settings
        - attach a listener to sig_change to receive asynchronous updates
        - use other functions to change settings; commit() to write them to the game

        Some information is given out in textual form to simplify the interface. */
    class TaxationProxy {
     public:
        typedef game::actions::TaxationAction::Direction Direction_t;
        typedef game::actions::TaxationAction::Area Area_t;
        typedef game::actions::TaxationAction::Areas_t Areas_t;

        /** Status for one taxation area. */
        struct AreaStatus {
            bool available;           ///< This area is available.
            int tax;                  ///< Current tax rate.
            int change;               ///< Current happiness change.
            String_t changeLabel;     ///< Textual representation of happiness change. \see game::tables::HappinessChangeName
            String_t description;     ///< Textual representation of tax rate/income. \see game::actions::TaxationAction::describe().
            String_t title;           ///< Title of this area (colony, natives).
        };

        /** Status of entire action. */
        struct Status {
            AreaStatus colonists;     ///< Colonist status.
            AreaStatus natives;       ///< Native status.
            bool valid;               ///< Validity flag. false if action could not be created (planet does not exist).
        };

        /** Constructor.

            You can construct a TaxationProxy for any planet Id.
            If the planet Id is out of range, Status::valid will report false.
            If the planet is in range, but not playable, the resulting action can be used
            to examine taxes, but not commit them.

            FIXME: inability to commit is not currently reported

            \param reply      RequestDispatcher to receive replies on
            \param gameSender Game sender.
            \param planetId   Planet Id. */
        TaxationProxy(util::RequestDispatcher& reply,
                      util::RequestSender<game::Session> gameSender,
                      game::Id_t planetId);

        /** Destructor. */
        ~TaxationProxy();

        // Synchronous query

        /** Get status.
            \param link Downlink
            \param out [out] Result */
        void getStatus(Downlink& link, Status& out);

        // Asynchronous modifiers

        /** Set number of buildings (mines + factories).
            Resulting changes are reported via sig_change.
            \param n Buildings
            \see game::actions::TaxationAction::setNumBuildings() */
        void setNumBuildings(int n);

        /** Set tax rate, limit to valid range.
            Resulting changes are reported via sig_change.
            \param a area
            \param value new tax rate
            \see game::actions::TaxationAction::setTaxLimited() */
        void setTaxLimited(Area_t a, int value);

        /** Change tax rate for better/worse revenue.
            Resulting changes are reported via sig_change.
            \param a area
            \param d direction
            \see game::actions::TaxationAction::changeRevenue() */
        void changeRevenue(Area_t a, Direction_t d);

        /** Change tax rate.
            Resulting changes are reported via sig_change.
            \param a area
            \param delta difference
            \see game::actions::TaxationAction::changeTax() */
        void changeTax(Area_t a, int delta);

        /** Set safe-tax for areas.
            Resulting changes are reported via sig_change.
            \param as areas
            \see game::actions::TaxationAction::setSafeTax() */
        void setSafeTax(Areas_t as);

        /** Revert tax rates.
            Resulting changes are reported via sig_change.
            \param as areas
            \see game::actions::TaxationAction::revert() */
        void revert(Areas_t as);

        /** Commit transaction.
            \see game::actions::TaxationAction::commit() */
        void commit();

        // Asynchronous responses

        /** Signal: change.
            Reports a new transaction status.
            Changes can originate within this transaction, or in a parallel change. */
        afl::base::Signal<void(const Status&)> sig_change;

     private:
        class Trampoline;
        util::RequestReceiver<TaxationProxy> m_reply;
        util::SlaveRequestSender<game::Session, Trampoline> m_trampoline;
    };

} }

#endif
