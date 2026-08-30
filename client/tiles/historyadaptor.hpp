/**
  *  \file client/tiles/historyadaptor.hpp
  *  \brief Class client::tiles::HistoryAdaptor
  */
#ifndef C2NG_CLIENT_TILES_HISTORYADAPTOR_HPP
#define C2NG_CLIENT_TILES_HISTORYADAPTOR_HPP

#include "afl/base/optional.hpp"
#include "afl/base/signal.hpp"
#include "afl/base/signalconnection.hpp"
#include "game/proxy/configurationobserverproxy.hpp"
#include "game/proxy/historyshipproxy.hpp"
#include "util/numberformatter.hpp"

namespace client { namespace tiles {

    /** UI-side state management for ship history viewing.
        In addition to the usual game-side state (currently-selected ship),
        the history screen manages a currently-selected history turn
        to communicate between tiles.

        HistoryAdaptor contains a HistoryShipProxy,
        and manages information being passed back and forth.

        For convenience, HistoryAdaptor also embeds a ConfigurationObserverProxy,
        and uses it to provide a NumberFormatter.

        To use, observe the desired event and inquire data as needed. */
    class HistoryAdaptor {
     public:
        /** Constructor.
            @param gameSender Game sender (used to construct HistoryShipProxy)
            @param reply      User-interface RequestDispatcher (used to construct HistoryShipProxy) */
        HistoryAdaptor(util::RequestSender<game::Session> gameSender, util::RequestDispatcher& reply);

        /** Destructor. */
        ~HistoryAdaptor();

        /** Access HistoryShipProxy.
            @return HistoryShipProxy */
        game::proxy::HistoryShipProxy& proxy();

        /** Get ship Id.
            @return last ship Id reported by HistoryShipProxy */
        game::Id_t getShipId() const;

        /** Get position list.
            @return last position list reported by HistoryShipProxy */
        const game::map::ShipLocationInfos_t& getPositionList() const;

        /** Get number formatter.
            @return NumberFormatter instance */
        util::NumberFormatter getNumberFormatter() const;

        /** Get turn number.
            @return last selected turn number */
        int getTurnNumber() const;

        /** Set turn number.
            On change, will emit sig_turnChange.
            @param turnNumber New turn number */
        void setTurnNumber(int turnNumber);

        /** Get current turn information.
            @return ShipLocationInfo for the currently-selected turn; null if none */
        const game::map::ShipLocationInfo* getCurrentTurnInformation() const;

        /** Signal: list change.
            Called when game side reports a new list, e.g. for a new ship,
            or when the NumberFormatter instance changes.
            Listener should call getPositionList(), getTurnNumber(). */
        afl::base::Signal<void()> sig_listChange;

        /** Signal: turn change.
            Called when the turn number changed (setTurnNumber), or game side provides appropriate change,
            or when the NumberFormatter instance changes.
            Listener should call getTurnNumber(), getCurrentTurnInformation(). */
        afl::base::Signal<void()> sig_turnChange;

     private:
        enum { IdThousands, IdClans };

        game::Id_t m_shipId;
        game::map::ShipLocationInfos_t m_locations;
        int m_turnNumber;
        bool m_useThousandsSeparator;
        bool m_useClans;

        game::proxy::HistoryShipProxy m_proxy;
        game::proxy::ConfigurationObserverProxy m_configProxy;
        afl::base::SignalConnection conn_proxyChange;
        afl::base::SignalConnection conn_configChange;

        void onChange(const game::proxy::HistoryShipProxy::Status& st);
        void onConfigChange(int id, int32_t value);
    };

    /** Find a turn number in a ShipLocationInfos_t.
        @param infos      Data to search in
        @param turnNumber Turn number
        @return Position such that infos[pos].turnNumber==turnNumber, if any */
    afl::base::Optional<size_t> findTurnNumber(const game::map::ShipLocationInfos_t& infos, int turnNumber);

} }

#endif
