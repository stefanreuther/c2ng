/**
  *  \file game/proxy/fictivestarbaseadaptor.hpp
  *  \brief Class game::proxy::FictiveStarbaseAdaptor
  */
#ifndef C2NG_GAME_PROXY_FICTIVESTARBASEADAPTOR_HPP
#define C2NG_GAME_PROXY_FICTIVESTARBASEADAPTOR_HPP

#include <memory>
#include "game/map/planet.hpp"
#include "game/proxy/starbaseadaptor.hpp"
#include "game/session.hpp"

namespace game { namespace proxy {

    /** StarbaseAdaptor for a fictive starbase.
        Uses the given planet from viewpoint turn, if existant, to provide a fictive starbase.
        Partial information is completed with defaults;
        if the given planet does not exist at all (e.g. due to Id 0 being specified), it is created from scratch.

        The starbase can be examined but changing it has no effect on the game. */
    class FictiveStarbaseAdaptor : public StarbaseAdaptor {
     public:
        /** Constructor.
            @param session  Session
            @param planetId Planet Id. Planet need not exist. */
        FictiveStarbaseAdaptor(Session& session, Id_t planetId);
        ~FictiveStarbaseAdaptor();

        // StarbaseAdaptor:
        virtual game::map::Planet& planet();
        virtual Session& session();
        virtual bool findShipCloningHere(Id_t& id, String_t& name);
        virtual void cancelAllCloneOrders();
        virtual void notifyListeners();

     private:
        Session& m_session;
        std::auto_ptr<game::map::Planet> m_planet;
    };

    /** Constructor.
        Use with RequestSender::makeTemporary() to construct a FictiveStarbaseAdaptor. */
    class FictiveStarbaseAdaptorFromSession : public afl::base::Closure<StarbaseAdaptor*(Session&)> {
     public:
        /** Constructor.
            @param planetId Planet Id */
        FictiveStarbaseAdaptorFromSession(Id_t planetId)
            : m_planetId(planetId)
            { }

        // Closure:
        virtual FictiveStarbaseAdaptor* call(Session& session);

     private:
        Id_t m_planetId;
    };

} }

#endif
