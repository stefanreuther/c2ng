/**
  *  \file game/proxy/historyshiplistproxy.cpp
  *  \brief Class game::proxy::HistoryShipListProxy
  */

#include "game/proxy/historyshiplistproxy.hpp"
#include "game/game.hpp"

using afl::base::Ptr;

class game::proxy::HistoryShipListProxy::Trampoline {
 public:
    Trampoline(Session& session, util::RequestSender<HistoryShipListProxy> reply);

    void setSelection(game::ref::HistoryShipSelection sel);
    void buildList();
    void sendList();
    void onUniverseChange();

 private:
    Session& m_session;
    util::RequestSender<HistoryShipListProxy> m_reply;

    game::ref::HistoryShipSelection m_selection;
    game::ref::HistoryShipList m_list;

    Ptr<Turn> m_turn;
    afl::base::SignalConnection conn_universeChange;
};


/*
 *  Trampoline
 */

game::proxy::HistoryShipListProxy::Trampoline::Trampoline(Session& session, util::RequestSender<HistoryShipListProxy> reply)
    : m_session(session),
      m_reply(reply),
      m_selection(),
      m_list(),
      m_turn(),
      conn_universeChange()
{ }

void
game::proxy::HistoryShipListProxy::Trampoline::setSelection(game::ref::HistoryShipSelection sel)
{
    m_selection = sel;
    buildList();
    sendList();
}

void
game::proxy::HistoryShipListProxy::Trampoline::buildList()
{
    // Clean up previous state
    conn_universeChange.disconnect();
    m_list.clear();

    // Obtain new turn
    Ptr<Game> g = m_session.getGame();
    if (g.get() != 0) {
        m_turn = &g->viewpointTurn();
    } else {
        m_turn = 0;
    }

    // If we got a turn, build the result and arrange for updates to arrive
    if (m_turn.get() != 0) {
        m_selection.buildList(m_list, *m_turn, m_session);
        conn_universeChange = m_turn->universe().sig_universeChange.add(this, &Trampoline::onUniverseChange);
    }
}

void
game::proxy::HistoryShipListProxy::Trampoline::sendList()
{
    m_reply.postRequest(&HistoryShipListProxy::updateList, m_list);
}

void
game::proxy::HistoryShipListProxy::Trampoline::onUniverseChange()
{
    if (m_turn.get() != 0) {
        game::ref::HistoryShipList tmpList;
        m_selection.buildList(tmpList, *m_turn, m_session);
        if (tmpList != m_list) {
            m_list = tmpList;
            sendList();
        }
    }
}


/*
 *  TrampolineFromSession
 */

class game::proxy::HistoryShipListProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(util::RequestSender<HistoryShipListProxy> reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply); }
 private:
    util::RequestSender<HistoryShipListProxy> m_reply;
};


/*
 *  HistoryShipListProxy
 */

game::proxy::HistoryShipListProxy::HistoryShipListProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply)
    : m_reply(reply, *this),
      m_request(gameSender.makeTemporary(new TrampolineFromSession(m_reply.getSender())))
{ }

game::proxy::HistoryShipListProxy::~HistoryShipListProxy()
{ }

void
game::proxy::HistoryShipListProxy::setSelection(const game::ref::HistoryShipSelection& sel)
{
    m_request.postRequest(&Trampoline::setSelection, sel);
}

void
game::proxy::HistoryShipListProxy::updateList(game::ref::HistoryShipList list)
{
    sig_listChange.raise(list);
}
