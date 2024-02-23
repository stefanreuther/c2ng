/**
  *  \file game/proxy/viewpointstarbaseadaptor.hpp
  *  \brief Class game::proxy::ViewpointStarbaseAdaptor
  */
#ifndef C2NG_GAME_PROXY_VIEWPOINTSTARBASEADAPTOR_HPP
#define C2NG_GAME_PROXY_VIEWPOINTSTARBASEADAPTOR_HPP

#include "afl/base/closure.hpp"
#include "game/map/planet.hpp"
#include "game/proxy/starbaseadaptor.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"

namespace game { namespace proxy {

    /** Adaptor for starbase from viewpoint-turn.
        Implements StarbaseAdaptor with access to a real, existing starbase.
        The starbase is accessed when the ViewpointStarbaseAdaptor is created,
        and does not follow a turn change. */
    class ViewpointStarbaseAdaptor : public StarbaseAdaptor {
     public:
        /** Constructor.
            Throws if the given session does not have a shiplist or game, or the given planet does not exist.
            @param session  Session
            @param planetId Planet Id */
        ViewpointStarbaseAdaptor(Session& session, Id_t planetId);

        // StarbaseAdaptor:
        virtual game::map::Planet& planet();
        virtual Session& session();
        virtual bool findShipCloningHere(Id_t& id, String_t& name);
        virtual void cancelAllCloneOrders();
        virtual void notifyListeners();

     private:
        Session& m_session;
        afl::base::Ref<Turn> m_turn;
        afl::base::Ref<game::spec::ShipList> m_shipList;
        game::map::Planet& m_planet;
    };

    /** Constructor.
        Use with RequestSender::makeTemporary() to construct a ViewpointStarbaseAdaptor. */
    class ViewpointStarbaseAdaptorFromSession : public afl::base::Closure<StarbaseAdaptor*(Session&)> {
     public:
        /** Constructor.
            @param planetId Planet Id */
        ViewpointStarbaseAdaptorFromSession(Id_t planetId)
            : m_planetId(planetId)
            { }

        // Closure:
        virtual ViewpointStarbaseAdaptor* call(Session& session);

     private:
        Id_t m_planetId;
    };

} }

#endif
