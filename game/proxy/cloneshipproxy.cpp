/**
  *  \file game/proxy/cloneshipproxy.cpp
  *  \brief Class game::proxy::CloneShipProxy
  */

#include "game/proxy/cloneshipproxy.hpp"
#include "afl/base/closure.hpp"
#include "game/actions/preconditions.hpp"
#include "game/exception.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/turn.hpp"

using game::map::Planet;
using game::map::Point;
using game::map::Ship;
using game::map::Universe;

class game::proxy::CloneShipProxy::Trampoline {
 public:
    Trampoline(Session& session, Planet& planet, Ship& ship, Universe& univ, Game& g)
        : m_session(session),
          m_game(g),
          m_root(game::actions::mustHaveRoot(session)),
          m_shipList(game::actions::mustHaveShipList(session)),
          m_action(planet, ship, univ, g.shipScores(), *m_shipList, *m_root)
        { }

    void getStatus(Status& st)
        {
            st.planetId       = m_action.planet().getId();
            st.isInFleet      = m_action.ship().getFleetNumber() != 0;
            st.isCloneOnce    = m_action.isCloneOnce();
            st.buildOrder     = m_action.getBuildOrder();
            st.orderStatus    = m_action.getOrderStatus();
            st.paymentStatus  = m_action.getPaymentStatus();
            st.cost           = m_action.getCloneAction().getCost();
            st.available      = m_action.getCloneAction().getAvailableAmountAsCost();
            st.remaining      = m_action.getCloneAction().getRemainingAmountAsCost();
            st.missing        = m_action.getCloneAction().getMissingAmountAsCost();
            st.techCost       = m_action.getTechUpgradeAction().getCost();
            st.conflictStatus = m_action.findConflict(&st.conflict, m_session.translator(), m_session.interface());
            st.valid          = true;
        }

    void commit()
        {
            m_action.commit(m_game->mapConfiguration(), m_session.rng());
        }

 private:
    Session& m_session;
    afl::base::Ref<Game> m_game;
    afl::base::Ref<Root> m_root;
    afl::base::Ref<game::spec::ShipList> m_shipList;
    game::actions::CloneShip m_action;
};

class game::proxy::CloneShipProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(Id_t id)
        : m_id(id)
        { }
    virtual Trampoline* call(Session& session)
        {
            Game& g = game::actions::mustHaveGame(session);
            Universe& univ = g.viewpointTurn().universe();
            Ship& sh = game::actions::mustExist(univ.ships().get(m_id));
            Point pt;
            if (!sh.getPosition().get(pt)) {
                throw Exception(Exception::eNoBase);
            }
            Planet& pl = game::actions::mustExist(univ.planets().get(univ.findPlanetAt(pt)));

            return new Trampoline(session, pl, sh, univ, g);
        }
 private:
    const Id_t m_id;
};

game::proxy::CloneShipProxy::CloneShipProxy(util::RequestSender<Session> gameSender, Id_t shipId)
    : m_sender(gameSender.makeTemporary(new TrampolineFromSession(shipId)))
{ }

game::proxy::CloneShipProxy::~CloneShipProxy()
{ }

void
game::proxy::CloneShipProxy::getStatus(WaitIndicator& ind, Status& st)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(Status& st)
            : m_status(st)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.getStatus(m_status); }
     private:
        Status& m_status;
    };

    st.valid = false;
    Task t(st);
    ind.call(m_sender, t);
}

void
game::proxy::CloneShipProxy::commit()
{
    m_sender.postRequest(&Trampoline::commit);
}
