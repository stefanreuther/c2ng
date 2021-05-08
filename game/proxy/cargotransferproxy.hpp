/**
  *  \file game/proxy/cargotransferproxy.hpp
  *  \brief Class game::proxy::CargoTransferProxy
  */
#ifndef C2NG_GAME_PROXY_CARGOTRANSFERPROXY_HPP
#define C2NG_GAME_PROXY_CARGOTRANSFERPROXY_HPP

#include "afl/base/signal.hpp"
#include "game/actions/cargotransfer.hpp"
#include "game/actions/cargotransfersetup.hpp"
#include "game/actions/multitransfersetup.hpp"
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

        Bidirectional synchronous: get information about the transaction, multi-unit setup

        Bidirectional asynchronous: 1:1 setup, cargo move and transaction commit

        To use for 1:1 transfer, obtain a CargoTransferSetup object (e.g. using CargoTransferSetupProxy).
        To use for multi-ship transfer, obtain a MultiTransferSetup (as of 20210405, no proxy provided).
        Initialize using init(), then call methods as needed, and finalize with commit(). */
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
            String_t info1;                   /**< First subtitle, "Outrider, 3xHPh, 2xMk8". \see CargoContainer::getInfo1() */
            String_t info2;                   /**< Second subtitle, "FCode: "xyz", 10% damage". \see CargoContainer::getInfo2() */
            Cargo cargo;                      /**< Cargo content. */
            bool isUnloadTarget;              /**< true if this unit is a possible "Unload" target. \see General::allowUnload */
            bool isTemporary;                 /**< true if this unit is temporary. \see CargoContainer::Temporary */
            Participant()
                : name(), info1(), info2(), cargo(), isUnloadTarget(false), isTemporary(false)
                { }
        };

        /** Information about the general setup. */
        struct General {
            ElementTypes_t validTypes;        /**< Valid cargo types. */
            CargoNameVector_t typeNames;      /**< Names of all cargo types. \see Element::getName() */
            CargoNameVector_t typeUnits;      /**< Units of all cargo types. \see Element::getUnit() */
            bool allowUnload;                 /**< true if this setup allows the "Unload" action. */
            bool allowSupplySale;             /**< true if this setup allows selling supplies */
            size_t numParticipants;           /**< Number of participants. */
            General()
                : validTypes(), typeNames(), typeUnits(), allowUnload(false), allowSupplySale(false), numParticipants(0)
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

            The CargoTransferSetup object can be prepared and validated using a CargoTransferSetupProxy, or by working directly on the game::Session.
            Using an invalid CargoTransferSetup object will cause the CargoTransferProxy to be not usable.
            Use getGeneralInformation(), General::numParticipants to check when in doubt.

            \param setup Setup

            \see game::actions::CargoTransferSetup::build() */
        void init(const game::actions::CargoTransferSetup& setup);

        /** Initialize for multi-unit setup.

            A multi-unit transfer is validated when it is being built.
            The return value will determine whether it succeeded.
            If the build failed, the CargoTransferProxy will not be usable.

            \param link  WaitIndicator object for UI synchronisation
            \param setup Setup
            \return result

            \see game::actions::MultiTransferSetup::build() */
        game::actions::MultiTransferSetup::Result init(WaitIndicator& link, const game::actions::MultiTransferSetup& setup);

        /** Add new hold space.
            \param name Name
            \see game::actions::CargoTransfer::addHoldSpace */
        void addHoldSpace(const String_t& name);

        /** Get general information.
            \param link       WaitIndicator object for UI synchronisation
            \param info [out] Result */
        void getGeneralInformation(WaitIndicator& link, General& info);

        /** Get information about participant.
            \param link       WaitIndicator object for UI synchronisation
            \param side       Side to query (starting at 0)
            \param info [out] Result */
        void getParticipantInformation(WaitIndicator& link, size_t side, Participant& info);

        /** Set overload permission.
            \param enable New value
            \see game::actions::CargoTransfer::setOverload() */
        void setOverload(bool enable);

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

        /** Move with extension.
            \param type   Element type to move
            \param amount Amount to move
            \param from   Index of source unit
            \param to     Index of target unit
            \param extension Index of extension unit
            \param sellSupplies true to enable supply sale

            Partial moves are always accepted.
            Resulting changes are reported via sig_change.

            \see game::actions::CargoTransfer::moveExt() */
        void moveExt(Element::Type type, int32_t amount, size_t from, size_t to, size_t extension, bool sellSupplies);

        /** Move all cargo to a given unit.
            Take cargo from all units and put it on the target unit.

            \param type       Element type to move
            \param to         Index of target unit
            \param except     Exception; this unit's cargo is not removed (use same as to to not except any unit)
            \param sellSupplies If enabled, convert supplies to mc

            Resulting changes are reported via sig_change.

            \see game::actions::CargoTransfer::moveAll() */
        void moveAll(Element::Type type, size_t to, size_t except, bool sellSupplies);

        /** Distribute cargo.
            \param type       Element type to move
            \param from       Index of source unit
            \param except     Index of exception unit (use same as from to not except any unit)
            \param mode       Distribution mode

            Resulting changes are reported via sig_change.

            \see game::actions::CargoTransfer::distribute() */
        void distribute(Element::Type type, size_t from, size_t except, game::actions::CargoTransfer::DistributeMode mode);

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
        class ObserverFromSession;

        util::RequestSender<Session> m_gameSender;
        util::RequestReceiver<CargoTransferProxy> m_reply;
        util::RequestSender<Observer> m_observerSender;

        static void getCargo(Cargo& out, const CargoContainer& cont, Element::Type limit);
    };

} }

#endif
