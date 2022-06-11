/**
  *  \file game/proxy/historyshipproxy.cpp
  *  \brief Class game::proxy::HistoryShipProxy
  */

#include "game/proxy/historyshipproxy.hpp"
#include "afl/base/ptr.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/map/cursors.hpp"
#include "game/map/objectobserver.hpp"
#include "game/turn.hpp"

using game::map::Point;

/*
 *  Trampoline
 */

class game::proxy::HistoryShipProxy::Trampoline {
 public:
    Trampoline(Session& session, util::RequestSender<HistoryShipProxy> reply)
        : m_session(session),
          m_reply(reply),
          m_game(session.getGame()),
          m_observer(game::actions::mustHaveGame(session).cursors().currentHistoryShip()),
          m_inhibitUpdate(false)
        {
            m_observer.sig_objectChange.add(this, &Trampoline::onObjectChanged);
            onObjectChanged();
        }

    void browseAt(Point pt, Mode mode, bool marked);
    void onObjectChanged();

 private:
    void sendUpdate(afl::base::Optional<int> turnNumber);

    Session& m_session;
    util::RequestSender<HistoryShipProxy> m_reply;
    afl::base::Ptr<Game> m_game;           // to keep it alive
    game::map::ObjectObserver m_observer;
    bool m_inhibitUpdate;
};

/* Implementation of HistoryShipProxy::browseAt */
void
game::proxy::HistoryShipProxy::Trampoline::browseAt(Point pt, Mode mode, bool marked)
{
    Id_t id = m_observer.cursor().getCurrentIndex();
    int turnNumber = 0;

    Game* g = m_session.getGame().get();
    Turn* t = (g != 0 ? g->getViewpointTurn().get() : 0);
    if (t != 0) {
        game::map::HistoryShipType& ty = t->universe().historyShips();
        switch (mode) {
         case Next:     id = ty.findNextShipAtWrap      (pt, id, marked, turnNumber); break;
         case Previous: id = ty.findPreviousShipAtWrap  (pt, id, marked, turnNumber); break;
         case First:    id = ty.findNextShipAtNoWrap    (pt, 0,  marked, turnNumber); break;
         case Last:     id = ty.findPreviousShipAtNoWrap(pt, 0,  marked, turnNumber); break;
        }
    }

    if (id != 0 && id != m_observer.cursor().getCurrentIndex()) {
        m_inhibitUpdate = true;
        m_observer.cursor().setCurrentIndex(id);
        m_inhibitUpdate = false;
        sendUpdate(turnNumber);
    }
}

/* Handler for ObjectObserver::sig_objectChange */
void
game::proxy::HistoryShipProxy::Trampoline::onObjectChanged()
{
    if (!m_inhibitUpdate) {
        sendUpdate(afl::base::Nothing);
    }
}

/* Send update with given turn number */
void
game::proxy::HistoryShipProxy::Trampoline::sendUpdate(afl::base::Optional<int> turnNumber)
{
    const game::map::Ship* sh = dynamic_cast<game::map::Ship*>(m_observer.getCurrentObject());
    const Game* g = m_session.getGame().get();
    const Turn* t = (g != 0 ? g->getViewpointTurn().get() : 0);
    const Root* r = m_session.getRoot().get();
    const game::spec::ShipList* sl = m_session.getShipList().get();

    // If we do not have all required objects, send an empty update
    std::auto_ptr<Status> st(new Status());
    if (sh != 0 && g != 0 && t != 0 && r != 0 && sl != 0) {
        st->shipId = sh->getId();
        packShipLocationInfo(st->locations, *sh, t->universe(), t->getTurnNumber(), g->mapConfiguration(),
                             r->hostConfiguration(), r->hostVersion(), *sl, m_session.translator());
        st->turnNumber = turnNumber;
    }
    m_reply.postRequest(&HistoryShipProxy::sendUpdate, st);
}


/*
 *  TrampolineFromSession
 */

class game::proxy::HistoryShipProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(util::RequestSender<HistoryShipProxy> reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply); }
 private:
    util::RequestSender<HistoryShipProxy> m_reply;
};


/*
 *  HistoryShipProxy
 */

game::proxy::HistoryShipProxy::HistoryShipProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply)
    : m_reply(reply, *this),
      m_request(gameSender.makeTemporary(new TrampolineFromSession(m_reply.getSender())))
{ }

game::proxy::HistoryShipProxy::~HistoryShipProxy()
{ }

void
game::proxy::HistoryShipProxy::browseAt(game::map::Point pt, Mode mode, bool marked)
{
    m_request.postRequest(&Trampoline::browseAt, pt, mode, marked);
}

void
game::proxy::HistoryShipProxy::sendUpdate(std::auto_ptr<Status> st)
{
    if (st.get() != 0) {
        sig_change.raise(*st);
    }
}
