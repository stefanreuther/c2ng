/**
  *  \file game/proxy/buildqueueproxy.cpp
  *  \brief Class game::proxy::BuildQueueProxy
  *
  *  FIXME: we use slot numbers to identify items, but slot numbers change between calls.
  *  We should use a more stable identifier so that 2x increasePriority() can apply to the same item.
  */

#include "game/proxy/buildqueueproxy.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "game/score/compoundscore.hpp"
#include "game/turn.hpp"

class game::proxy::BuildQueueProxy::Trampoline {
 public:
    typedef game::actions::ChangeBuildQueue Action_t;

    Trampoline(Session& session, util::RequestSender<BuildQueueProxy> reply)
        : m_session(session),
          m_reply(reply),
          m_action()
        {
            Game* g = session.getGame().get();
            Root* r = session.getRoot().get();
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

    Action_t* get() const
        { return m_action.get(); }

    void sendUpdate()
        {
            class Task : public util::Request<BuildQueueProxy> {
             public:
                Task(Action_t& a, Session& session)
                    { a.describe(m_data, session.translator()); }

                virtual void handle(BuildQueueProxy& proxy)
                    { proxy.sig_update.raise(m_data); }
             private:
                Action_t::Infos_t m_data;
            };

            if (m_action.get() != 0) {
                m_reply.postNewRequest(new Task(*m_action, m_session));
            }
        }

    Session& session()
        { return m_session; }

 private:
    Session& m_session;
    util::RequestSender<BuildQueueProxy> m_reply;
    std::auto_ptr<Action_t> m_action;
};


class game::proxy::BuildQueueProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(const util::RequestSender<BuildQueueProxy>& reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply); }
 private:
    util::RequestSender<BuildQueueProxy> m_reply;
};



game::proxy::BuildQueueProxy::BuildQueueProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply)
    : m_reply(reply, *this),
      m_request(gameSender.makeTemporary(new TrampolineFromSession(m_reply.getSender())))
{ }

void
game::proxy::BuildQueueProxy::getStatus(WaitIndicator& link, Infos_t& data)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(Infos_t& data)
            : m_data(data)
            { }
        virtual void handle(Trampoline& tpl)
            {
                if (Trampoline::Action_t* p = tpl.get()) {
                    p->describe(m_data, tpl.session().translator());
                }
            }
     private:
        Infos_t& m_data;
    };
    Task t(data);
    link.call(m_request, t);
}

void
game::proxy::BuildQueueProxy::setPriority(size_t slot, int pri)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(size_t slot, int pri)
            : m_slot(slot), m_pri(pri)
            { }
        virtual void handle(Trampoline& tpl)
            {
                if (Trampoline::Action_t* p = tpl.get()) {
                    p->setPriority(m_slot, m_pri);
                    tpl.sendUpdate();
                }
            }
     private:
        size_t m_slot;
        int m_pri;
    };
    m_request.postNewRequest(new Task(slot, pri));
}

void
game::proxy::BuildQueueProxy::increasePriority(size_t slot)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(size_t slot)
            : m_slot(slot)
            { }
        virtual void handle(Trampoline& tpl)
            {
                if (Trampoline::Action_t* p = tpl.get()) {
                    p->increasePriority(m_slot);
                    tpl.sendUpdate();
                }
            }
     private:
        size_t m_slot;
    };
    m_request.postNewRequest(new Task(slot));
}

void
game::proxy::BuildQueueProxy::decreasePriority(size_t slot)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(size_t slot)
            : m_slot(slot)
            { }
        virtual void handle(Trampoline& tpl)
            {
                if (Trampoline::Action_t* p = tpl.get()) {
                    p->decreasePriority(m_slot);
                    tpl.sendUpdate();
                }
            }
     private:
        size_t m_slot;
    };
    m_request.postNewRequest(new Task(slot));
}

void
game::proxy::BuildQueueProxy::commit()
{
    class Task : public util::Request<Trampoline> {
     public:
        virtual void handle(Trampoline& tpl)
            {
                if (Trampoline::Action_t* p = tpl.get()) {
                    p->commit();
                }
            }
    };
    m_request.postNewRequest(new Task());
}
