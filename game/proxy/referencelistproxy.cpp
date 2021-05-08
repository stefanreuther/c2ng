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

class game::proxy::ReferenceListProxy::Observer {
 public:
    Observer(Session& session, const util::RequestSender<ReferenceListProxy>& reply)
        : m_reply(reply),
          m_session(session)
        {
            m_observer.setSession(session);
            conn_listChange = m_observer.sig_listChange.add(this, &Observer::onListChange);
        }

    void onListChange()
        { m_reply.postNewRequest(new Updater(m_observer.getList())); }

    void updateContent(Initializer_t& init)
        { init.call(m_session, m_observer); }

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
    Session& m_session;
    game::ref::ListObserver m_observer;
    afl::base::SignalConnection conn_listChange;
};

class game::proxy::ReferenceListProxy::ObserverFromSession : public afl::base::Closure<Observer*(Session&)> {
 public:
    ObserverFromSession(const util::RequestSender<ReferenceListProxy>& reply)
        : m_reply(reply)
        { }
    virtual Observer* call(Session& session)
        { return new Observer(session, m_reply); }
 private:
    util::RequestSender<ReferenceListProxy> m_reply;
};

game::proxy::ReferenceListProxy::ReferenceListProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& disp)
    : sig_listChange(),
      sig_finish(),
      m_gameSender(gameSender),
      m_receiver(disp, *this),
      m_observerSender(gameSender.makeTemporary(new ObserverFromSession(m_receiver.getSender()))),
      m_pendingRequests(0)
{ }

void
game::proxy::ReferenceListProxy::setConfigurationSelection(const game::ref::ConfigurationSelection& sel)
{
    class Request : public util::Request<Observer> {
     public:
        Request(const game::ref::ConfigurationSelection& sel)
            : m_sel(sel)
            { }
        virtual void handle(Observer& obs)
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
    class Request : public util::Request<Observer> {
     public:
        Request(std::auto_ptr<Initializer_t> pInit)
            : m_pInit(pInit)
            { }
        virtual void handle(Observer& obs)
            {
                obs.updateContent(*m_pInit);
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
    class Request : public util::Request<Observer> {
     public:
        virtual void handle(Observer&)
            { }
    };
    Request r;
    link.call(m_observerSender, r);
}


game::ref::Configuration
game::proxy::ReferenceListProxy::getConfig(WaitIndicator& link)
{
    class Request : public util::Request<Observer> {
     public:
        virtual void handle(Observer& obs)
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
    class Request : public util::Request<Observer> {
     public:
        Request(game::ref::Configuration config)
            : m_config(config)
            { }
        virtual void handle(Observer& obs)
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
