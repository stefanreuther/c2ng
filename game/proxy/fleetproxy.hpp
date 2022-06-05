/**
  *  \file game/proxy/fleetproxy.hpp
  *  \brief Class game::proxy::FleetProxy
  */
#ifndef C2NG_GAME_PROXY_FLEETPROXY_HPP
#define C2NG_GAME_PROXY_FLEETPROXY_HPP

#include "afl/base/signal.hpp"
#include "game/ref/fleetmemberlist.hpp"
#include "game/session.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** Fleet proxy.
        Asynchronous, bidirectional proxy to access the current fleet.
        While this proxy is alive, it makes sure that whenever a fleet is selected (game::map::Cursors::currentFleet()),
        a member of the fleat is selected as current ship (game::map::Cursors::currentShip()).
        Information about this fleet is published using the FleetProxy.

        Bidirectional, asynchronous:
        - select a fleet member
        - change notification

        Data is stored inside the FleetProxy and can be retrieved synchronously, without wait, at any time. */
    class FleetProxy {
     public:
        /** Constructor.
            @param gameSender Game sender
            @param receiver   RequestDispatcher to receive updates in this thread */
        FleetProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply);

        /** Destructor. */
        ~FleetProxy();

        /** Select a fleet member.
            This will select the given ship as current ship.
            getSelectedFleetMember() will update when acknowledged by the game side.
            If that ship is not part of the current fleet, the current fleet is changed as well.
            @param shipId New ship */
        void selectFleetMember(Id_t shipId);

        /** Get fleet member list.
            @return last reported FleetMemberList */
        const game::ref::FleetMemberList& getFleetMemberList() const;

        /** Get selected fleet member.
            After initialisation (=after first sig_change callback), this value is only zero if no more fleets exist.
            @return last reported fleet member (current ship) */
        Id_t getSelectedFleetMember() const;

        /** Signal: content change.
            Reported whenever getFleetMemberList() or getSelectedFleetMember() changes */
        afl::base::Signal<void()> sig_change;

     private:
        class Trampoline;
        class TrampolineFromSession;

        util::RequestReceiver<FleetProxy> m_reply;
        util::RequestSender<Trampoline> m_request;

        game::ref::FleetMemberList m_fleetMemberList;
        Id_t m_selectedFleetMember;

        void onFleetChange(std::auto_ptr<game::ref::FleetMemberList> memList, Id_t memId);
        void onFleetMemberSelected(Id_t memId);
    };

} }

#endif
