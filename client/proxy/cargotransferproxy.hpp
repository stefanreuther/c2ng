/**
  *  \file client/proxy/cargotransferproxy.hpp
  *  \brief Class client::proxy::CargoTransferProxy
  */
#ifndef C2NG_CLIENT_PROXY_CARGOTRANSFERPROXY_HPP
#define C2NG_CLIENT_PROXY_CARGOTRANSFERPROXY_HPP

#include "afl/base/signal.hpp"
#include "client/downlink.hpp"
#include "game/actions/cargotransfersetup.hpp"
#include "game/cargocontainer.hpp"
#include "game/element.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"
#include "util/vector.hpp"

namespace client { namespace proxy {

    /** Cargo Transfer proxy.
        This proxies a game::actions::CargoTransfer object.

        Bidirectional synchronous: get information about the transaction

        Bidirectional asynchronous: setup, cargo move and transaction commit */
    class CargoTransferProxy {
     public:
        /*
         *  Data Types
         */

        /** Cargo amounts. */
        typedef util::Vector<int32_t, game::Element::Type> CargoVector_t;

        /** Names of cargo types. */
        typedef util::Vector<String_t, game::Element::Type> CargoNameVector_t;

        /** Information about one unit's cargo amounts. */
        struct Cargo {
            CargoVector_t amount;             /**< Current amount. \see game::CargoContainer::getEffectiveAmount() */
            CargoVector_t remaining;          /**< Remaining room. \see game::CargoContainer::getMaxAmount() */
        };

        /** Information about one participant. */
        struct Participant {
            String_t name;                    /**< Unit name. \see game::CargoContainer::getName() */
            // FIXME -> String_t info1;     // "Outrider, 3xHPh, 2xMk8"
            // FIXME -> String_t info2;     // "FCode: "xyz", 10% damage"
            Cargo cargo;                      /**< Cargo content. */
            bool isUnloadTarget;              /**< true if this unit is a possible "Unload" target. \see General::allowUnload */
        };

        /** Information about the general setup. */
        struct General {
            game::ElementTypes_t validTypes;  /**< Valid cargo types. */
            CargoNameVector_t typeNames;      /**< Names of all cargo types. \see game::Element::getName() */
            CargoNameVector_t typeUnits;      /**< Units of all cargo types. \see game::Element::getUnit() */
            bool allowUnload;                 /**< true if this setup allows the "Unload" action. */
            bool allowSupplySale;             /**< true if this setup allows selling supplies */
        };


        /*
         *  Operations
         */

        /** Constructor.
            \param root       Root. Used for callbacks into UI thread.
            \param gameSender Game sender */
        CargoTransferProxy(ui::Root& root, util::RequestSender<game::Session> gameSender);

        /** Initialize for two-unit setup.
            \param setup Setup */
        void init(const game::actions::CargoTransferSetup& setup);

        /** Get general information.
            \param link       Downlink object for UI synchronisation
            \param info [out] Result */
        void getGeneralInformation(Downlink& link, General& info);

        /** Get information about participant.
            \param link       Downlink object for UI synchronisation
            \param side       Side to query (starting at 0)
            \param info [out] Result */
        void getParticipantInformation(Downlink& link, size_t side, Participant& info);

        /** Move cargo.
            \param type   Element type to move
            \param amount Amount to move
            \param from   Index of source unit
            \param to     Index of target unit
            \param sellSupplies true to enable supply sale

            Partial moves are always accepted.
            Resulting changes are reported via sig_change.

            \see game::actions::CargoTransfer::move() */
        void move(game::Element::Type type, int32_t amount, size_t from, size_t to, bool sellSupplies);

        /** Unload.
            \param sellSupplies true to enable supply sale

            Resulting changes are reported via sig_change.

            \see game::actions::CargoTransfer::unload() */
        void unload(bool sellSupplies);

        /** Commit the transaction.
            \see game::actions::CargoTransfer::commit() */
        void commit();

        /** Signal: content change.
            \param size_t Unit index
            \param Cargo  Unit's cargo */
        afl::base::Signal<void(size_t, const Cargo&)> sig_change;

     private:
        class Notifier;
        class Observer;

        util::RequestSender<game::Session> m_gameSender;
        util::RequestReceiver<CargoTransferProxy> m_reply;
        util::SlaveRequestSender<game::Session, Observer> m_observerSender;

        static void getCargo(Cargo& out, const game::CargoContainer& cont, game::Element::Type limit);
    };

} }

#endif
