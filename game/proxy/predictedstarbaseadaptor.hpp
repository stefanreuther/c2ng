/**
  *  \file game/proxy/predictedstarbaseadaptor.hpp
  *  \brief Class game::proxy::PredictedStarbaseAdaptor
  */
#ifndef C2NG_GAME_PROXY_PREDICTEDSTARBASEADAPTOR_HPP
#define C2NG_GAME_PROXY_PREDICTEDSTARBASEADAPTOR_HPP

#include "game/proxy/starbaseadaptor.hpp"
#include "game/game.hpp"
#include "game/interface/basetaskpredictor.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/types.hpp"

namespace game { namespace proxy {

    /** StarbaseAdaptor for predicted starbase.
        Uses the starbase's auto-task to predict the situation until the auto-task cursor using a game::interface::BaseTaskPredictor.
        In particular, parts are consumed by preceding commands.

        If the task ends with a BuildShip or EnqueueShip command, the predicted situation will show that build order still active.
        Pass waitClear=true to execute it and start with no active order.

        The prediction is NOT updated when the underlying planet changes. */
    class PredictedStarbaseAdaptor : public StarbaseAdaptor {
     public:
        /** Constructor.
            @param session   Session to work on
            @param planetId  Planet Id
            @param waitClear true: wait until build order is clear; false: start with last-submitted order */
        PredictedStarbaseAdaptor(Session& session, Id_t planetId, bool waitClear);
        ~PredictedStarbaseAdaptor();

        // StarbaseAdaptor:
        virtual game::map::Planet& planet();
        virtual Session& session();
        virtual bool findShipCloningHere(Id_t& id, String_t& name);
        virtual void cancelAllCloneOrders();
        virtual void notifyListeners();

     private:
        Session& m_session;

        // Smart pointers to keep these objects alive:
        afl::base::Ref<Turn> m_turn;
        afl::base::Ref<game::spec::ShipList> m_shipList;
        afl::base::Ref<Root> m_root;

        // Predictor (containing the subject planet)
        game::interface::BaseTaskPredictor m_pred;
    };

    /** Constructor.
        Use with RequestSender::makeTemporary() to construct a PredictedStarbaseAdaptor. */
    class PredictedStarbaseAdaptorFromSession : public afl::base::Closure<StarbaseAdaptor*(Session&)> {
     public:
        /** Constructor.
            @param planetId  Planet Id
            @param waitClear true: wait until build order is clear; false: start with last-submitted order */
        PredictedStarbaseAdaptorFromSession(Id_t planetId, bool waitClear)
            : m_planetId(planetId), m_waitClear(waitClear)
            { }

        virtual PredictedStarbaseAdaptor* call(Session& session);

     private:
        Id_t m_planetId;
        bool m_waitClear;
    };

} }

#endif
