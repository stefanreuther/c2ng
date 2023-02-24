/**
  *  \file game/proxy/ionstormproxy.cpp
  *  \class game::proxy::IonStormProxy
  */

#include "game/proxy/ionstormproxy.hpp"
#include "afl/string/format.hpp"
#include "game/game.hpp"
#include "game/map/ionstorm.hpp"
#include "game/map/objectobserver.hpp"
#include "game/proxy/objectlistener.hpp"
#include "game/root.hpp"
#include "game/tables/ionstormclassname.hpp"

using afl::string::Format;
using game::map::IonStorm;

class game::proxy::IonStormProxy::Trampoline {
 public:
    Trampoline(Session& session, const util::RequestSender<IonStormProxy>& reply);

    void addNewListener(ObjectListener* pl);
    void buildIonStormInfo(IonStormInfo& out) const;
    void browse(game::map::ObjectCursor::Mode mode, bool marked);

 private:
    void onObjectChange();

    IonStorm* getIonStorm() const;
    void sendIonStormInfo();

    Session& m_session;
    util::RequestSender<IonStormProxy> m_reply;
    std::auto_ptr<game::map::ObjectObserver> m_observer;
    afl::container::PtrVector<ObjectListener> m_listeners;
    afl::base::SignalConnection conn_objectChange;
};


game::proxy::IonStormProxy::Trampoline::Trampoline(Session& session, const util::RequestSender<IonStormProxy>& reply)
    : m_session(session),
      m_reply(reply),
      m_observer(),
      m_listeners(),
      conn_objectChange()
{
    if (Game* g = session.getGame().get()) {
        m_observer.reset(new game::map::ObjectObserver(g->cursors().currentIonStorm()));
        conn_objectChange = m_observer->sig_objectChange.add(this, &Trampoline::onObjectChange);
    }
    onObjectChange();
}

inline void
game::proxy::IonStormProxy::Trampoline::addNewListener(ObjectListener* pl)
{
    m_listeners.pushBackNew(pl);
    pl->handle(m_session, getIonStorm());
}

void
game::proxy::IonStormProxy::Trampoline::buildIonStormInfo(IonStormInfo& out) const
{
    // ex WIonStormInfoTile::drawData (part)
    const Root* root = m_session.getRoot().get();
    const IonStorm* st = getIonStorm();
    if (root != 0 && st != 0) {
        // Environment
        util::NumberFormatter fmt = root->userConfiguration().getNumberFormatter();
        afl::string::Translator& tx = m_session.translator();

        // Id
        out.stormId = st->getId();

        // Center
        out.center = st->getPosition().orElse(game::map::Point());

        // Radius
        if (st->getRadius().get(out.radius)) {
            out.text[Radius] = Format(tx("%d ly"), fmt.formatNumber(out.radius));
        } else {
            out.radius = 0;
            out.text[Radius] = tx("unknown");
        }

        // Heading
        int heading;
        if (st->getHeading().get(heading)) {
            out.text[Heading] = Format(tx("%d\xC2\xB0"), fmt.formatNumber(heading));
            out.heading = heading;
        } else {
            out.text[Heading] = tx("unknown");
            out.heading = 0;
        }

        // Speed
        int speed;
        if (st->getWarpFactor().get(speed)) {
            out.text[Speed] = Format(tx("warp %d"), fmt.formatNumber(speed));
            out.speed = speed;
        } else {
            out.text[Speed] = tx("unknown");
            out.heading = 0;
        }

        // Voltage, ClassName
        int voltage, classNr;
        if (st->getVoltage().get(voltage) && st->getClass().get(classNr)) {
            out.text[Voltage] = Format(tx("%d MeV"), fmt.formatNumber(voltage));
            out.text[ClassName] = Format(tx("Class %d (%s)"), fmt.formatNumber(classNr), game::tables::IonStormClassName(tx).get(voltage));
            out.voltage = voltage;
        } else {
            out.text[Voltage] = out.text[ClassName] = tx("unknown");
            out.voltage = 0;
        }

        // Status
        out.text[Status] = st->isGrowing() ? tx("growing") : tx("weakening");

        // Forecast
        st->getForecast(out.forecast);
    }
}

void
game::proxy::IonStormProxy::Trampoline::browse(game::map::ObjectCursor::Mode mode, bool marked)
{
    if (m_observer.get() != 0) {
        m_observer->cursor().browse(mode, marked);
    }
}

void
game::proxy::IonStormProxy::Trampoline::onObjectChange()
{
    // Update
    IonStorm*const p = getIonStorm();

    // IonStormInfo
    sendIonStormInfo();

    // Inform listeners
    for (size_t i = 0; i < m_listeners.size(); ++i) {
        m_listeners[i]->handle(m_session, p);
    }
}

IonStorm*
game::proxy::IonStormProxy::Trampoline::getIonStorm() const
{
    if (m_observer.get() != 0) {
        return dynamic_cast<IonStorm*>(m_observer->getCurrentObject());
    } else {
        return 0;
    }
}

void
game::proxy::IonStormProxy::Trampoline::sendIonStormInfo()
{
    class Task : public util::Request<IonStormProxy> {
     public:
        Task(const Trampoline& tpl)
            : m_info()
            { tpl.buildIonStormInfo(m_info); }
        virtual void handle(IonStormProxy& proxy)
            { proxy.sig_stormChange.raise(m_info); }
     private:
        IonStormInfo m_info;
    };
    m_reply.postNewRequest(new Task(*this));
}



/*
 *  TrampolineFromSession
 */

class game::proxy::IonStormProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(util::RequestSender<IonStormProxy> reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply); }
 private:
    util::RequestSender<IonStormProxy> m_reply;
};


/*
 *  IonStormProxy
 */

game::proxy::IonStormProxy::IonStormProxy(util::RequestDispatcher& reply, util::RequestSender<Session> gameSender)
    : m_reply(reply, *this),
      m_trampoline(gameSender.makeTemporary(new TrampolineFromSession(m_reply.getSender())))
{ }

game::proxy::IonStormProxy::~IonStormProxy()
{ }

void
game::proxy::IonStormProxy::browse(game::map::ObjectCursor::Mode mode, bool marked)
{
    m_trampoline.postRequest(&Trampoline::browse, mode, marked);
}

void
game::proxy::IonStormProxy::addNewListener(ObjectListener* pl)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(std::auto_ptr<ObjectListener> pl)
            : m_listener(pl)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.addNewListener(m_listener.release()); }
     private:
        std::auto_ptr<ObjectListener> m_listener;
    };
    m_trampoline.postNewRequest(new Task(std::auto_ptr<ObjectListener>(pl)));
}
