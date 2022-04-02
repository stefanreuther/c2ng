/**
  *  \file game/proxy/currentstarbaseadaptor.hpp
  *  \brief Class game::proxy::CurrentStarbaseAdaptor
  */
#ifndef C2NG_GAME_PROXY_CURRENTSTARBASEADAPTOR_HPP
#define C2NG_GAME_PROXY_CURRENTSTARBASEADAPTOR_HPP

#include "afl/base/closure.hpp"
#include "game/game.hpp"
#include "game/proxy/starbaseadaptor.hpp"

namespace game { namespace proxy {

    /** Adaptor for current starbase.
        Implements StarbaseAdaptor with access to a real, existing starbase. */
    class CurrentStarbaseAdaptor : public StarbaseAdaptor {
     public:
        /** Constructor.
            Throws if the given session does not have a shiplist or game, or the given planet does not exist.
            @param session  Session
            @param planetId Planet Id */
        CurrentStarbaseAdaptor(Session& session, Id_t planetId);

        // StarbaseAdaptor:
        virtual game::map::Planet& planet();
        virtual Session& session();
        virtual bool findShipCloningHere(Id_t& id, String_t& name);
        virtual void cancelAllCloneOrders();
        virtual void notifyListeners();

     private:
        Session& m_session;
        afl::base::Ptr<Game> m_pGame;
        afl::base::Ptr<game::spec::ShipList> m_pShipList;
        Game& m_game;
        game::spec::ShipList& m_shipList;
        game::map::Planet& m_planet;
    };

    /** Constructor.
        Use with RequestSender::makeTemporary() to construct a CurrentStarbaseAdaptor. */
    class CurrentStarbaseAdaptorFromSession : public afl::base::Closure<StarbaseAdaptor*(Session&)> {
     public:
        /** Constructor.
            @param planetId Planet Id */
        CurrentStarbaseAdaptorFromSession(Id_t planetId)
            : m_planetId(planetId)
            { }

        // Closure:
        virtual CurrentStarbaseAdaptor* call(Session& session);

     private:
        Id_t m_planetId;
    };

} }

#endif
