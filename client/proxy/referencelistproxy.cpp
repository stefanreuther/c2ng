/**
  *  \file client/proxy/referencelistproxy.cpp
  */

#include "client/proxy/referencelistproxy.hpp"
#include "client/dialogs/referencesortorder.hpp"
#include "client/downlink.hpp"

class client::proxy::ReferenceListProxy::Updater : public util::Request<ReferenceListProxy> {
 public:
    Updater(const game::ref::UserList& list)
        : m_list(list)
        { }
    virtual void handle(client::proxy::ReferenceListProxy& conn)
        { conn.onListChange(m_list); }
 private:
    game::ref::UserList m_list;
};

class client::proxy::ReferenceListProxy::Observer : public util::SlaveObject<game::Session> {
 public:
    Observer(util::RequestSender<ReferenceListProxy> reply)
        : m_reply(reply)
        { }
    virtual void init(game::Session& session)
        {
            m_observer.setSession(session);
            conn_listChange = m_observer.sig_listChange.add(this, &Observer::onListChange);
        }

    virtual void done(game::Session& /*session*/)
        {
            conn_listChange.disconnect();
        }

    void onListChange()
        {
            m_reply.postNewRequest(new Updater(m_observer.getList()));
        }

    void updateContent(game::Session& session, Initializer_t& init)
        {
            init.call(session, m_observer);
        }

    void setConfig(const game::ref::Configuration& config)
        {
            m_observer.setConfig(config);
        }

    game::ref::Configuration getConfig() const
        {
            return m_observer.getConfig();
        }

 private:
    util::RequestSender<ReferenceListProxy> m_reply;
    game::ref::ListObserver m_observer;
    afl::base::SignalConnection conn_listChange;
};

client::proxy::ReferenceListProxy::ReferenceListProxy(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx)
    : m_root(root),
      m_gameSender(gameSender),
      m_translator(tx),
      m_receiver(root.engine().dispatcher(), *this),
      m_observerSender(gameSender, new Observer(m_receiver.getSender()))
{ }

void
client::proxy::ReferenceListProxy::setContentNew(std::auto_ptr<Initializer_t> pInit)
{
    class Request : public util::SlaveRequest<game::Session, Observer> {
     public:
        Request(std::auto_ptr<Initializer_t> pInit)
            : m_pInit(pInit)
            { }
        virtual void handle(game::Session& session, Observer& obs)
            { obs.updateContent(session, *m_pInit); }
     private:
        std::auto_ptr<Initializer_t> m_pInit;
    };
    if (pInit.get() != 0) {
        m_observerSender.postNewRequest(new Request(pInit));
    }
}

void
client::proxy::ReferenceListProxy::onMenu(gfx::Point pt)
{
    game::ref::Configuration order = getConfig();
    if (client::dialogs::doReferenceSortOrderMenu(order, pt, m_root, m_translator)) {
        setConfig(order);
    }
}

void
client::proxy::ReferenceListProxy::onListChange(const game::ref::UserList& list)
{
    sig_listChange.raise(list);
}

game::ref::Configuration
client::proxy::ReferenceListProxy::getConfig()
{
    class Request : public util::SlaveRequest<game::Session, Observer> {
     public:
        virtual void handle(game::Session& /*session*/, Observer& obs)
            { m_config = obs.getConfig(); }
        game::ref::Configuration m_config;
    };

    Downlink link(m_root);
    Request req;
    link.call(m_observerSender, req);
    return req.m_config;
}

void
client::proxy::ReferenceListProxy::setConfig(const game::ref::Configuration& config)
{
    class Request : public util::SlaveRequest<game::Session, Observer> {
     public:
        Request(game::ref::Configuration config)
            : m_config(config)
            { }
        virtual void handle(game::Session& /*session*/, Observer& obs)
            { obs.setConfig(m_config); }
     private:
        game::ref::Configuration m_config;
    };
    m_observerSender.postNewRequest(new Request(config));
}
