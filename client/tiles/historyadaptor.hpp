/**
  *  \file client/tiles/historyadaptor.hpp
  *  \brief Class client::tiles::HistoryAdaptor
  */
#ifndef C2NG_CLIENT_TILES_HISTORYADAPTOR_HPP
#define C2NG_CLIENT_TILES_HISTORYADAPTOR_HPP

#include "afl/base/signal.hpp"
#include "afl/base/signalconnection.hpp"
#include "game/proxy/historyshipproxy.hpp"

namespace client { namespace tiles {

    /** UI-side state management for ship history viewing.
        In addition to the usual game-side state (currently-selected ship),
        the history screen manages a currently-selected history turn
        to communicate between tiles.

        HistoryAdaptor contains a HistoryShipProxy,
        and manages information being passed back and forth.

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
            Called when game side reports a new list, e.g. for a new ship.
            Listener should call getPositionList(), getTurnNumber(). */
        afl::base::Signal<void()> sig_listChange;

        /** Signal: turn change.
            Called when the turn number changed (setTurnNumber), or game side provides appropriate change.
            Listener should call getTurnNumber(), getCurrentTurnInformation(). */
        afl::base::Signal<void()> sig_turnChange;

     private:
        game::Id_t m_shipId;
        game::map::ShipLocationInfos_t m_locations;
        int m_turnNumber;

        game::proxy::HistoryShipProxy m_proxy;
        afl::base::SignalConnection conn_proxyChange;

        void onChange(const game::proxy::HistoryShipProxy::Status& st);
    };

    /** Find a turn number in a ShipLocationInfos_t.
        @param [in]  infos      Data to search in
        @param [in]  turnNumber Turn number
        @param [out] pos        Position such that infos[pos].turnNumber==turnNumber
        @return true on success, false if not found (pos not updated) */
    bool findTurnNumber(const game::map::ShipLocationInfos_t& infos, int turnNumber, size_t& pos);

} }

#endif
