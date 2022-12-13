/**
  *  \file game/proxy/maplocationproxy.hpp
  *  \brief Class game::proxy::MapLocationProxy
  */
#ifndef C2NG_GAME_PROXY_MAPLOCATIONPROXY_HPP
#define C2NG_GAME_PROXY_MAPLOCATIONPROXY_HPP

#include "afl/base/signal.hpp"
#include "game/map/configuration.hpp"
#include "game/map/location.hpp"
#include "game/reference.hpp"
#include "game/session.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Asynchronous, bidirectional proxy for map location.
        This accesses the Session > Game > game::map::Cursors > game::map::Location object.
        In addition, it provides a game::map::Configuration object and reports changes. */
    class MapLocationProxy {
     public:
        /** Constructor.
            \param gameSender RequestSender to send to the game
            \param reply RequestDispatcher to receive replies */
        MapLocationProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply);

        /** Destructor. */
        ~MapLocationProxy();

        /** Post a request to query the current location.
            This call is used for retrieving the initial position.
            Every postQueryLocation() call will eventually produce a sig_locationResult callback. */
        void postQueryLocation();

        /** Set location to point.
            Will eventually generate a sig_positionChange callback.
            \param pt Point
            \see game::map::Location::set() */
        void setPosition(game::map::Point pt);

        /** Set location to reference.
            Will eventually generate a sig_positionChange callback.
            \param ref Reference
            \see game::map::Location::set() */
        void setPosition(game::Reference ref);

        /** Browse from current object.
            Will respond with sig_browseResult in addition to sig_positionChange.
            \param flags Flags
            \see game::map::Location::browse() */
        void browse(game::map::Location::BrowseFlags_t flags);

        /** Get possible "other end" position.
            - if position is at ship, return its waypoint
            - if position is at ship waypoint, return its position
            - if position is in a wormhole, return exit position
            - if circular map is active, switch between map images
            \param [in]  ind      UI synchronisation
            \param [in]  shipId   Focus ship Id; can be 0
            \param [out] result   Result
            \return true on success; false if no alternate position found
            \see game::map::Location::getOtherPosition() */
        bool getOtherPosition(WaitIndicator& ind, game::Id_t shipId, game::map::Point& result);

        /** Location callback.
            Called in response to postQueryLocation().
            \param ref Reference result
            \param pt Point result */
        afl::base::Signal<void(Reference, game::map::Point, game::map::Configuration)> sig_locationResult;

        /** Browse result callback.
            Called in response to browse().
            \param ref Reference
            \param pt Location */
        afl::base::Signal<void(Reference, game::map::Point)> sig_browseResult;

        /** Position change callback.
            Called if the game-side location reports a sig_positionChange. */
        afl::base::Signal<void(game::map::Point)> sig_positionChange;

        /** Configuration change callback.
            Called if the map configuration changes on game side. */
        afl::base::Signal<void(game::map::Configuration)> sig_configChange;

     private:
        class Trampoline;
        class TrampolineFromSession;
        util::RequestReceiver<MapLocationProxy> m_reply;
        util::RequestSender<Trampoline> m_trampoline;

        /* If we send down multiple setPosition() requests, suppress responses.
           This is to avoid building up lag. */
        int m_numOutstandingRequests;

        void emitPositionChange(game::map::Point pt);
        void emitBrowseResult(Reference ref, game::map::Point pt);
    };

} }

#endif
