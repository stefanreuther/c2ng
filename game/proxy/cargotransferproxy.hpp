/**
  *  \file game/proxy/cargotransferproxy.hpp
  *  \brief Class game::proxy::CargoTransferProxy
  */
#ifndef C2NG_GAME_PROXY_CARGOTRANSFERPROXY_HPP
#define C2NG_GAME_PROXY_CARGOTRANSFERPROXY_HPP

#include "afl/base/signal.hpp"
#include "game/actions/cargotransfersetup.hpp"
#include "game/cargocontainer.hpp"
#include "game/element.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestsender.hpp"
#include "util/vector.hpp"

namespace game { namespace proxy {

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
        typedef util::Vector<int32_t, Element::Type> CargoVector_t;

        /** Names of cargo types. */
        typedef util::Vector<String_t, Element::Type> CargoNameVector_t;

        /** Information about one unit's cargo amounts. */
        struct Cargo {
            CargoVector_t amount;             /**< Current amount. \see CargoContainer::getEffectiveAmount() */
            CargoVector_t remaining;          /**< Remaining room. \see CargoContainer::getMaxAmount() */
        };

        /** Information about one participant. */
        struct Participant {
            String_t name;                    /**< Unit name. \see CargoContainer::getName() */
            // FIXME -> String_t info1;     // "Outrider, 3xHPh, 2xMk8"
            // FIXME -> String_t info2;     // "FCode: "xyz", 10% damage"
            Cargo cargo;                      /**< Cargo content. */
            bool isUnloadTarget;              /**< true if this unit is a possible "Unload" target. \see General::allowUnload */
            Participant()
                : name(), cargo(), isUnloadTarget(false)
                { }
        };

        /** Information about the general setup. */
        struct General {
            ElementTypes_t validTypes;        /**< Valid cargo types. */
            CargoNameVector_t typeNames;      /**< Names of all cargo types. \see Element::getName() */
            CargoNameVector_t typeUnits;      /**< Units of all cargo types. \see Element::getUnit() */
            bool allowUnload;                 /**< true if this setup allows the "Unload" action. */
            bool allowSupplySale;             /**< true if this setup allows selling supplies */
            General()
                : validTypes(), typeNames(), typeUnits(), allowUnload(false), allowSupplySale(false)
                { }
        };


        /*
         *  Operations
         */

        /** Constructor.
            \param gameSender Game sender
            \param reply      RequestDispatcher for replies back into UI thread. */
        CargoTransferProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply);

        /** Initialize for two-unit setup.
            \param setup Setup */
        void init(const game::actions::CargoTransferSetup& setup);

        /** Get general information.
            \param link       WaitIndicator object for UI synchronisation
            \param info [out] Result */
        void getGeneralInformation(WaitIndicator& link, General& info);

        /** Get information about participant.
            \param link       WaitIndicator object for UI synchronisation
            \param side       Side to query (starting at 0)
            \param info [out] Result */
        void getParticipantInformation(WaitIndicator& link, size_t side, Participant& info);

        /** Move cargo.
            \param type   Element type to move
            \param amount Amount to move
            \param from   Index of source unit
            \param to     Index of target unit
            \param sellSupplies true to enable supply sale

            Partial moves are always accepted.
            Resulting changes are reported via sig_change.

            \see game::actions::CargoTransfer::move() */
        void move(Element::Type type, int32_t amount, size_t from, size_t to, bool sellSupplies);

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

        util::RequestSender<Session> m_gameSender;
        util::RequestReceiver<CargoTransferProxy> m_reply;
        util::SlaveRequestSender<Session, Observer> m_observerSender;

        static void getCargo(Cargo& out, const CargoContainer& cont, Element::Type limit);
    };

} }

#endif
