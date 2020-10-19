/**
  *  \file game/proxy/referencelistproxy.cpp
  *  \brief Class game::proxy::ReferenceListProxy
  */

#include "game/proxy/referencelistproxy.hpp"

class game::proxy::ReferenceListProxy::Updater : public util::Request<ReferenceListProxy> {
 public:
    Updater(const game::ref::UserList& list)
        : m_list(list)
        { }
    virtual void handle(ReferenceListProxy& conn)
        { conn.onListChange(m_list); }
 private:
    game::ref::UserList m_list;
};

class game::proxy::ReferenceListProxy::Confirmer : public util::Request<ReferenceListProxy> {
 public:
    virtual void handle(ReferenceListProxy& conn)
        { conn.confirmRequest(); }
};

class game::proxy::ReferenceListProxy::Observer : public util::SlaveObject<Session> {
 public:
    Observer(util::RequestSender<ReferenceListProxy> reply)
        : m_reply(reply)
        { }
    virtual void init(Session& session)
        {
            m_observer.setSession(session);
            conn_listChange = m_observer.sig_listChange.add(this, &Observer::onListChange);
        }

    virtual void done(Session& /*session*/)
        { conn_listChange.disconnect(); }

    void onListChange()
        { m_reply.postNewRequest(new Updater(m_observer.getList())); }

    void updateContent(Session& session, Initializer_t& init)
        { init.call(session, m_observer); }

    void setConfig(const game::ref::Configuration& config)
        { m_observer.setConfig(config); }

    game::ref::Configuration getConfig() const
        { return m_observer.getConfig(); }

    void setConfigurationSelection(const game::ref::ConfigurationSelection& sel)
        { m_observer.setConfigurationSelection(sel); }

    void confirmRequest()
        { m_reply.postNewRequest(new Confirmer()); }

 private:
    util::RequestSender<ReferenceListProxy> m_reply;
    game::ref::ListObserver m_observer;
    afl::base::SignalConnection conn_listChange;
};

game::proxy::ReferenceListProxy::ReferenceListProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& disp)
    : sig_listChange(),
      sig_finish(),
      m_gameSender(gameSender),
      m_receiver(disp, *this),
      m_observerSender(gameSender, new Observer(m_receiver.getSender())),
      m_pendingRequests(0)
{ }

void
game::proxy::ReferenceListProxy::setConfigurationSelection(const game::ref::ConfigurationSelection& sel)
{
    class Request : public util::SlaveRequest<game::Session, Observer> {
     public:
        Request(const game::ref::ConfigurationSelection& sel)
            : m_sel(sel)
            { }
        virtual void handle(Session& /*session*/, Observer& obs)
            {
                obs.setConfigurationSelection(m_sel);
                obs.confirmRequest();
            }
     private:
        const game::ref::ConfigurationSelection& m_sel;
    };
    ++m_pendingRequests;
    m_observerSender.postNewRequest(new Request(sel));
}

void
game::proxy::ReferenceListProxy::setContentNew(std::auto_ptr<Initializer_t> pInit)
{
    class Request : public util::SlaveRequest<Session, Observer> {
     public:
        Request(std::auto_ptr<Initializer_t> pInit)
            : m_pInit(pInit)
            { }
        virtual void handle(Session& session, Observer& obs)
            {
                obs.updateContent(session, *m_pInit);
                obs.confirmRequest();
            }
     private:
        std::auto_ptr<Initializer_t> m_pInit;
    };
    if (pInit.get() != 0) {
        ++m_pendingRequests;
        m_observerSender.postNewRequest(new Request(pInit));
    }
}

bool
game::proxy::ReferenceListProxy::isIdle() const
{
    return m_pendingRequests == 0;
}

void
game::proxy::ReferenceListProxy::waitIdle(WaitIndicator& link)
{
    class Request : public util::SlaveRequest<Session, Observer> {
     public:
        virtual void handle(Session&, Observer&)
            { }
    };
    Request r;
    link.call(m_observerSender, r);
}


game::ref::Configuration
game::proxy::ReferenceListProxy::getConfig(WaitIndicator& link)
{
    class Request : public util::SlaveRequest<Session, Observer> {
     public:
        virtual void handle(Session& /*session*/, Observer& obs)
            { m_config = obs.getConfig(); }
        game::ref::Configuration m_config;
    };

    Request req;
    link.call(m_observerSender, req);
    return req.m_config;
}

void
game::proxy::ReferenceListProxy::setConfig(const game::ref::Configuration& config)
{
    class Request : public util::SlaveRequest<Session, Observer> {
     public:
        Request(game::ref::Configuration config)
            : m_config(config)
            { }
        virtual void handle(Session& /*session*/, Observer& obs)
            {
                obs.setConfig(m_config);
                obs.confirmRequest();
            }
     private:
        game::ref::Configuration m_config;
    };
    ++m_pendingRequests;
    m_observerSender.postNewRequest(new Request(config));
}

void
game::proxy::ReferenceListProxy::onListChange(const game::ref::UserList& list)
{
    sig_listChange.raise(list);
}

void
game::proxy::ReferenceListProxy::confirmRequest()
{
    if (--m_pendingRequests == 0) {
        sig_finish.raise();
    }
}
