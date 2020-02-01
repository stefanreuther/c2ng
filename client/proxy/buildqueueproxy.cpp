/**
  *  \file client/proxy/buildqueueproxy.cpp
  *
  *  FIXME: we use slot numbers to identify items, but slot numbers change between calls.
  *  We should use a more stable identifier so that 2x increasePriority() can apply to the same item.
  */

#include "client/proxy/buildqueueproxy.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "game/score/compoundscore.hpp"
#include "game/turn.hpp"

class client::proxy::BuildQueueProxy::Trampoline : public util::SlaveObject<game::Session> {
 public:
    typedef game::actions::ChangeBuildQueue Action_t;

    Trampoline(util::RequestSender<BuildQueueProxy> reply)
        : m_reply(reply),
          m_action()
        { }

    virtual void init(game::Session& session)
        {
            game::Game* g = session.getGame().get();
            game::Root* r = session.getRoot().get();
            game::spec::ShipList* sl = session.getShipList().get();
            if (g != 0 && r != 0 && sl != 0) {
                m_action.reset(new Action_t(g->currentTurn().universe(),
                                            *sl,
                                            r->hostVersion(),
                                            r->hostConfiguration(),
                                            session.rng(),
                                            g->getViewpointPlayer()));
                m_action->setAvailableBuildPoints(game::score::CompoundScore(g->scores(), game::score::ScoreId_BuildPoints, 1).get(g->scores(), g->currentTurn().getTurnNumber(), g->getViewpointPlayer()));
            }
        }

    virtual void done(game::Session& /*session*/)
        { m_action.reset(); }

    Action_t* get() const
        { return m_action.get(); }

    void sendUpdate(game::Session& session)
        {
            class Task : public util::Request<BuildQueueProxy> {
             public:
                Task(Action_t& a, game::Session& session)
                    { a.describe(m_data, session.translator()); }

                virtual void handle(BuildQueueProxy& proxy)
                    { proxy.sig_update.raise(m_data); }
             private:
                Action_t::Infos_t m_data;
            };

            if (m_action.get() != 0) {
                m_reply.postNewRequest(new Task(*m_action, session));
            }
        }

 private:
    util::RequestSender<BuildQueueProxy> m_reply;
    std::auto_ptr<Action_t> m_action;
};


client::proxy::BuildQueueProxy::BuildQueueProxy(util::RequestSender<game::Session> gameSender, util::RequestDispatcher& reply)
    : m_reply(reply, *this),
      m_request(gameSender, new Trampoline(m_reply.getSender()))
{ }

void
client::proxy::BuildQueueProxy::init(client::Downlink& link, Infos_t& data)
{
    class Task : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        Task(Infos_t& data)
            : m_data(data)
            { }
        virtual void handle(game::Session& session, Trampoline& tpl)
            {
                if (Trampoline::Action_t* p = tpl.get()) {
                    p->describe(m_data, session.translator());
                }
            }
     private:
        Infos_t& m_data;
    };
    Task t(data);
    link.call(m_request, t);
}

void
client::proxy::BuildQueueProxy::setPriority(size_t slot, int pri)
{
    class Task : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        Task(size_t slot, int pri)
            : m_slot(slot), m_pri(pri)
            { }
        virtual void handle(game::Session& session, Trampoline& tpl)
            {
                if (Trampoline::Action_t* p = tpl.get()) {
                    p->setPriority(m_slot, m_pri);
                    tpl.sendUpdate(session);
                }
            }
     private:
        size_t m_slot;
        int m_pri;
    };
    m_request.postNewRequest(new Task(slot, pri));
}

void
client::proxy::BuildQueueProxy::increasePriority(size_t slot)
{
    class Task : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        Task(size_t slot)
            : m_slot(slot)
            { }
        virtual void handle(game::Session& session, Trampoline& tpl)
            {
                if (Trampoline::Action_t* p = tpl.get()) {
                    p->increasePriority(m_slot);
                    tpl.sendUpdate(session);
                }
            }
     private:
        size_t m_slot;
    };
    m_request.postNewRequest(new Task(slot));
}

void
client::proxy::BuildQueueProxy::decreasePriority(size_t slot)
{
    class Task : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        Task(size_t slot)
            : m_slot(slot)
            { }
        virtual void handle(game::Session& session, Trampoline& tpl)
            {
                if (Trampoline::Action_t* p = tpl.get()) {
                    p->decreasePriority(m_slot);
                    tpl.sendUpdate(session);
                }
            }
     private:
        size_t m_slot;
    };
    m_request.postNewRequest(new Task(slot));
}

void
client::proxy::BuildQueueProxy::commit()
{
    class Task : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        virtual void handle(game::Session& /*session*/, Trampoline& tpl)
            {
                if (Trampoline::Action_t* p = tpl.get()) {
                    p->commit();
                }
            }
    };
    m_request.postNewRequest(new Task());
}
