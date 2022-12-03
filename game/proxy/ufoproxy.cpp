/**
  *  \file game/proxy/ufoproxy.cpp
  *  \brief Class game::proxy::UfoProxy
  */

#include "game/proxy/ufoproxy.hpp"
#include "afl/string/format.hpp"
#include "game/game.hpp"
#include "game/map/objectobserver.hpp"
#include "game/map/ufo.hpp"
#include "game/proxy/objectlistener.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "util/string.hpp"

using afl::string::Format;
using game::map::Ufo;

class game::proxy::UfoProxy::Trampoline {
 public:
    Trampoline(Session& session, const util::RequestSender<UfoProxy>& reply);

    void addNewListener(ObjectListener* pl);
    void buildUfoInfo(UfoInfo& out) const;
    void browse(game::map::ObjectCursor::Mode mode, bool marked);
    void browseToOtherEnd();
    void toggleStoredInHistory();

 private:
    void onObjectChange();

    Ufo* getUfo() const;
    void sendUfoInfo();

    Session& m_session;
    util::RequestSender<UfoProxy> m_reply;
    std::auto_ptr<game::map::ObjectObserver> m_observer;
    afl::container::PtrVector<ObjectListener> m_listeners;
    afl::base::SignalConnection conn_objectChange;
};


game::proxy::UfoProxy::Trampoline::Trampoline(Session& session, const util::RequestSender<UfoProxy>& reply)
    : m_session(session),
      m_reply(reply),
      m_observer(),
      m_listeners(),
      conn_objectChange()
{
    if (Game* g = session.getGame().get()) {
        m_observer.reset(new game::map::ObjectObserver(g->cursors().currentUfo()));
        conn_objectChange = m_observer->sig_objectChange.add(this, &Trampoline::onObjectChange);
    }
    onObjectChange();
}

inline void
game::proxy::UfoProxy::Trampoline::addNewListener(ObjectListener* pl)
{
    m_listeners.pushBackNew(pl);
    pl->handle(m_session, getUfo());
}

void
game::proxy::UfoProxy::Trampoline::buildUfoInfo(UfoInfo& out) const
{
    // ex WUfoSettingsTile::drawData (part), WUfoInfoTile::drawData (part)
    const Root*const root = m_session.getRoot().get();
    const Game*const g = m_session.getGame().get();
    const Ufo*const p = getUfo();
    if (root != 0 && p != 0 && g != 0) {
        // Environment
        util::NumberFormatter fmt = root->userConfiguration().getNumberFormatter();
        afl::string::Translator& tx = m_session.translator();

        // Id
        out.ufoId = p->getId();

        // Center
        out.center = p->getPosition().orElse(game::map::Point());

        // Radius
        if (p->getRadius().get(out.radius)) {
            if (out.radius == 0) {
                out.text[Radius] = tx("(small)");
            } else {
                out.text[Radius] = Format(tx("%d ly"), fmt.formatNumber(out.radius));
            }
        } else {
            out.radius = 0;
            out.text[Radius] = tx("unknown");
        }

        // Info
        out.text[Info1] = p->getInfo1();
        out.text[Info2] = p->getInfo2();
        out.colorCode = p->getColorCode();

        // Speed
        int speed;
        if (p->getSpeed().get(speed)) {
            if (speed == 0) {
                out.text[Speed] = tx("not moving");
            } else {
                out.text[Speed] = Format(tx("warp %d"), speed);
            }
        } else {
            out.text[Speed] = tx("unknown");
        }

        // Heading
        int heading;
        if (p->getHeading().get(heading)) {
            out.text[Heading] = Format("%d\xC2\xB0", heading);
        } else {
            out.text[Heading] = tx("unknown");
        }

        game::map::Point vec = p->getMovementVector();
        if (vec != game::map::Point()) {
            out.text[Heading] += Format(" (%+d,%+d)", vec.getX(), vec.getY());
        }

        // Ranges
        int shipRange;
        if (p->getShipRange().get(shipRange)) {
            out.text[ShipRange] = Format(tx("%d ly"), fmt.formatNumber(shipRange));
        } else {
            out.text[ShipRange] = tx("unknown");
        }

        int planetRange;
        if (p->getPlanetRange().get(planetRange)) {
            out.text[PlanetRange] = Format(tx("%d ly"), fmt.formatNumber(planetRange));
        } else {
            out.text[PlanetRange] = tx("unknown");
        }

        // Last info
        out.text[LastInfo] = util::formatAge(g->getViewpointTurnNumber(), p->getLastTurn(), tx);

        // Other end
        if (const Ufo*const oe = p->getOtherEnd()) {
            out.text[OtherEndName] = oe->getName(LongName, tx, m_session.interface());
            out.hasOtherEnd = true;
        } else {
            out.text[OtherEndName] = tx("none/not known");
            out.hasOtherEnd = false;
        }

        // History
        out.isStoredInHistory = p->isStoredInHistory();
    }
}

void
game::proxy::UfoProxy::Trampoline::browse(game::map::ObjectCursor::Mode mode, bool marked)
{
    if (m_observer.get() != 0) {
        m_observer->cursor().browse(mode, marked);
    }
}

void
game::proxy::UfoProxy::Trampoline::browseToOtherEnd()
{
    // ex WUfoSettingsTile::onOtherEnd
    const Ufo* p = getUfo();
    if (m_observer.get() != 0 && p != 0) {
        if (game::map::ObjectType* ty = m_observer->getObjectType()) {
            if (Id_t id = ty->findIndexForObject(p->getOtherEnd())) {
                m_observer->cursor().setCurrentIndex(id);
            }
        }
    }
}

void
game::proxy::UfoProxy::Trampoline::toggleStoredInHistory()
{
    if (Ufo* p = getUfo()) {
        p->setIsStoredInHistory(!p->isStoredInHistory());
        m_session.notifyListeners();
    }
}

void
game::proxy::UfoProxy::Trampoline::onObjectChange()
{
    // Update
    Ufo*const p = getUfo();

    // UfoInfo
    sendUfoInfo();

    // Inform listeners
    for (size_t i = 0; i < m_listeners.size(); ++i) {
        m_listeners[i]->handle(m_session, p);
    }
}

Ufo*
game::proxy::UfoProxy::Trampoline::getUfo() const
{
    if (m_observer.get() != 0) {
        return dynamic_cast<Ufo*>(m_observer->getCurrentObject());
    } else {
        return 0;
    }
}

void
game::proxy::UfoProxy::Trampoline::sendUfoInfo()
{
    class Task : public util::Request<UfoProxy> {
     public:
        Task(const Trampoline& tpl)
            : m_info()
            { tpl.buildUfoInfo(m_info); }
        virtual void handle(UfoProxy& proxy)
            { proxy.sig_ufoChange.raise(m_info); }
     private:
        UfoInfo m_info;
    };
    m_reply.postNewRequest(new Task(*this));
}



/*
 *  TrampolineFromSession
 */

class game::proxy::UfoProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(util::RequestSender<UfoProxy> reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply); }
 private:
    util::RequestSender<UfoProxy> m_reply;
};


/*
 *  UfoProxy
 */


game::proxy::UfoProxy::UfoProxy(util::RequestDispatcher& reply, util::RequestSender<Session> gameSender)
    : m_reply(reply, *this),
      m_trampoline(gameSender.makeTemporary(new TrampolineFromSession(m_reply.getSender())))
{ }

// Destructor.
game::proxy::UfoProxy::~UfoProxy()
{ }

// Browse Ufos.
void
game::proxy::UfoProxy::browse(game::map::ObjectCursor::Mode mode, bool marked)
{
    m_trampoline.postRequest(&Trampoline::browse, mode, marked);
}

// Browse to other end.
void
game::proxy::UfoProxy::browseToOtherEnd()
{
    m_trampoline.postRequest(&Trampoline::browseToOtherEnd);
}

// Toggle "stored in history" flag.
void
game::proxy::UfoProxy::toggleStoredInHistory()
{
    m_trampoline.postRequest(&Trampoline::toggleStoredInHistory);
}

// ObjectObserver:
void
game::proxy::UfoProxy::addNewListener(ObjectListener* pl)
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
