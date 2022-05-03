/**
  *  \file game/proxy/configurationobserverproxy.cpp
  *  \brief Class game::proxy::ConfigurationObserverProxy
  */

#include "game/proxy/configurationobserverproxy.hpp"
#include "afl/container/ptrvector.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/root.hpp"

/*
 *  BaseObserver - Base class for all possible observers
 */

class game::proxy::ConfigurationObserverProxy::BaseObserver {
 public:
    virtual ~BaseObserver()
        { }
    virtual void onChange(Trampoline& tpl, const game::config::UserConfiguration& config) = 0;
};


/*
 *  Trampoline
 */

class game::proxy::ConfigurationObserverProxy::Trampoline {
 public:
    Trampoline(Session& session, const util::RequestSender<ConfigurationObserverProxy>& reply);

    game::config::UserConfiguration* getConfiguration();

    void observeOption(int id, const game::config::IntegerOptionDescriptor* opt);
    void observeOption(int id, const game::config::StringOptionDescriptor* opt);

    void onConfigChange();

    util::RequestSender<ConfigurationObserverProxy>& reply()
        { return m_reply; }

 private:
    Session& m_session;
    util::RequestSender<ConfigurationObserverProxy> m_reply;
    afl::container::PtrVector<BaseObserver> m_observers;
    afl::base::SignalConnection conn_configChange;
};


/*
 *  ScalarObserver - Common implementation for observing a scalar configuration option
 */

template<typename Descriptor, typename Value>
class game::proxy::ConfigurationObserverProxy::ScalarObserver : public BaseObserver {
 public:
    ScalarObserver(int id, const Descriptor& desc, Trampoline& tpl)
        : m_id(id),
          m_descriptor(desc),
          m_value()
        {
            if (const game::config::UserConfiguration* uc = tpl.getConfiguration()) {
                m_value = (*uc)[m_descriptor]();
                sendStatus(tpl);
            }
        }

    void onChange(Trampoline& tpl, const game::config::UserConfiguration& config)
        {
            Value newValue = config[m_descriptor]();
            if (newValue != m_value) {
                m_value = newValue;
                sendStatus(tpl);
            }
        }

 private:
    void sendStatus(Trampoline& tpl);

    const int m_id;
    const Descriptor& m_descriptor;
    Value m_value;
};

template<typename Descriptor, typename Value>
void
game::proxy::ConfigurationObserverProxy::ScalarObserver<Descriptor, Value>::sendStatus(Trampoline& tpl)
{
    tpl.reply().postRequest(&ConfigurationObserverProxy::emitChange, m_id, m_value);
}


/*
 *  Trampoline Implementation
 */

game::proxy::ConfigurationObserverProxy::Trampoline::Trampoline(Session& session, const util::RequestSender<ConfigurationObserverProxy>& reply)
    : m_session(session), m_reply(reply), conn_configChange()
{
    if (game::config::UserConfiguration* uc = getConfiguration()) {
        conn_configChange = uc->sig_change.add(this, &Trampoline::onConfigChange);
    }
}

game::config::UserConfiguration*
game::proxy::ConfigurationObserverProxy::Trampoline::getConfiguration()
{
    if (Root* r = m_session.getRoot().get()) {
        return &r->userConfiguration();
    } else {
        return 0;
    }
}

void
game::proxy::ConfigurationObserverProxy::Trampoline::observeOption(int id, const game::config::IntegerOptionDescriptor* opt)
{
    // The "opt" parameter has pointer type to go through RequestSender::postRequest painlessly
    // (The generic template function would try to make a value copy of a `const T&`.)
    m_observers.pushBackNew(new ScalarObserver<game::config::IntegerOptionDescriptor, int32_t>(id, *opt, *this));
}

void
game::proxy::ConfigurationObserverProxy::Trampoline::observeOption(int id, const game::config::StringOptionDescriptor* opt)
{
    m_observers.pushBackNew(new ScalarObserver<game::config::StringOptionDescriptor, String_t>(id, *opt, *this));
}

void
game::proxy::ConfigurationObserverProxy::Trampoline::onConfigChange()
{
    if (game::config::UserConfiguration* uc = getConfiguration()) {
        for (size_t i = 0, n = m_observers.size(); i < n; ++i) {
            m_observers[i]->onChange(*this, *uc);
        }
    }
}


/*
 *  TrampolineFromSession
 */

class game::proxy::ConfigurationObserverProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(util::RequestSender<ConfigurationObserverProxy> reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply); }
 private:
    util::RequestSender<ConfigurationObserverProxy> m_reply;
};


/*
 *  ConfigurationObserverProxy
 */

game::proxy::ConfigurationObserverProxy::ConfigurationObserverProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply)
    : ConfigurationProxy(gameSender),
      m_receiver(reply, *this),
      m_sender(gameSender.makeTemporary(new TrampolineFromSession(m_receiver.getSender())))
{ }

void
game::proxy::ConfigurationObserverProxy::observeOption(int id, const game::config::IntegerOptionDescriptor& opt)
{
    m_sender.postRequest(&Trampoline::observeOption, id, &opt);
}

void
game::proxy::ConfigurationObserverProxy::observeOption(int id, const game::config::StringOptionDescriptor& opt)
{
    m_sender.postRequest(&Trampoline::observeOption, id, &opt);
}

void
game::proxy::ConfigurationObserverProxy::emitChange(int id, int32_t value)
{
    sig_intOptionChange.raise(id, value);
}

void
game::proxy::ConfigurationObserverProxy::emitChange(int id, String_t value)
{
    sig_stringOptionChange.raise(id, value);
}
