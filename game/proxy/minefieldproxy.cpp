/**
  *  \file game/proxy/minefieldproxy.cpp
  *  \brief Class game::proxy::MinefieldProxy
  */

#include "game/proxy/minefieldproxy.hpp"
#include "afl/string/format.hpp"
#include "game/game.hpp"
#include "game/map/objectobserver.hpp"
#include "game/playerlist.hpp"
#include "game/proxy/objectlistener.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "util/math.hpp"
#include "util/string.hpp"

using afl::string::Format;
using game::config::HostConfiguration;
using game::map::Minefield;

namespace {
    int getEffectiveFighterSweepRate(const HostConfiguration& config, const Minefield& mf, int player)
    {
        if (mf.isWeb() && (config.getPlayerRaceNumber(player) != 11 || config[HostConfiguration::AllowColoniesSweepWebs]() == 0)) {
            return 0;
        } else {
            return config[HostConfiguration::FighterSweepRate](player);
        }
    }
}


class game::proxy::MinefieldProxy::Trampoline {
 public:
    Trampoline(Session& session, const util::RequestSender<MinefieldProxy>& reply);

    void addNewListener(ObjectListener* pl);
    void setPassageDistance(int distance);
    void buildSweepInfo(SweepInfo& out) const;
    void buildMinefieldInfo(MinefieldInfo& out) const;
    void buildPassageInfo(PassageInfo& out) const;
    void browse(game::map::ObjectCursor::Mode mode, bool marked);
    void erase(Id_t id);

 private:
    void onObjectChange();

    Minefield* getMinefield() const;
    void sendMinefieldInfo();
    void sendPassageInfo();

    Session& m_session;
    util::RequestSender<MinefieldProxy> m_reply;
    std::auto_ptr<game::map::ObjectObserver> m_observer;
    afl::container::PtrVector<ObjectListener> m_listeners;
    afl::base::SignalConnection conn_objectChange;

    int m_passageDistance;
    void* m_lastObject;
};

/*
 *  Trampoline
 */

game::proxy::MinefieldProxy::Trampoline::Trampoline(Session& session, const util::RequestSender<MinefieldProxy>& reply)
    : m_session(session),
      m_reply(reply),
      m_observer(),
      m_listeners(),
      conn_objectChange(),
      m_passageDistance(0),
      m_lastObject(0)
{
    if (Game* g = session.getGame().get()) {
        m_observer.reset(new game::map::ObjectObserver(g->cursors().currentMinefield()));
        conn_objectChange = m_observer->sig_objectChange.add(this, &Trampoline::onObjectChange);
        onObjectChange();
    }
}

inline void
game::proxy::MinefieldProxy::Trampoline::addNewListener(ObjectListener* pl)
{
    m_listeners.pushBackNew(pl);
    pl->handle(m_session, getMinefield());
}

inline void
game::proxy::MinefieldProxy::Trampoline::setPassageDistance(int distance)
{
    if (m_passageDistance != distance) {
        m_passageDistance = distance;
        sendPassageInfo();
    }
}

inline void
game::proxy::MinefieldProxy::Trampoline::buildSweepInfo(SweepInfo& out) const
{
    // ex showSweepInfo (part)
    const Minefield* p = getMinefield();
    const Root* r = m_session.getRoot().get();
    const Game* g = m_session.getGame().get();
    if (r != 0 && g != 0 && p != 0 && p->isValid()) {
        // Environment
        const int viewpointPlayer = g->getViewpointPlayer();

        // Figure out fighter sweep rate
        int frace = viewpointPlayer;
        int frate = getEffectiveFighterSweepRate(r->hostConfiguration(), *p, frace);
        if (frate == 0) {
            // We cannot sweep with fighters. Find someone who can.
            for (int i = 1; i <= MAX_PLAYERS; ++i) {
                frate = getEffectiveFighterSweepRate(r->hostConfiguration(), *p, i);
                if (frate != 0) {
                    frace = i;
                    break;
                }
            }
        }

        // Initial output
        const int32_t units = p->getUnitsForLaying(r->hostVersion(), r->hostConfiguration());
        out.units = units;
        out.isWeb = p->isWeb();

        // Beam weapons
        const game::spec::ShipList* sl = m_session.getShipList().get();
        if (sl != 0) {
            // FIXME: mark current ship's type
            const game::spec::BeamVector_t& vec = sl->beams();
            for (const game::spec::Beam* b = vec.findNext(0); b != 0; b = vec.findNext(b->getId())) {
                int rate = b->getNumMinesSwept(viewpointPlayer, p->isWeb(), r->hostConfiguration());
                if (rate > 0) {
                    out.weapons.push_back(SweepItem(util::divideAndRoundUp(units, rate), 0, b->getName(sl->componentNamer())));
                }
            }
        }

        // Fighters
        if (frate > 0) {
            afl::string::Translator& tx = m_session.translator();
            out.weapons.push_back(SweepItem(util::divideAndRoundUp(units, frate), 0,
                                            Format(tx("%s fighter"), r->playerList().getPlayerName(frace, Player::AdjectiveName, tx))));
        }
    }
}

inline void
game::proxy::MinefieldProxy::Trampoline::buildMinefieldInfo(MinefieldInfo& out) const
{
    // ex WMinefieldInfoTile::drawData (part)
    const Minefield* p = getMinefield();
    const Root* r = m_session.getRoot().get();
    if (r != 0 && p != 0 && p->isValid()) {
        // Environment
        const Game* g = m_session.getGame().get();
        const Turn* t = g != 0 ? g->getViewpointTurn().get() : 0;
        util::NumberFormatter fmt = r->userConfiguration().getNumberFormatter();
        afl::string::Translator& tx = m_session.translator();

        // Main information
        out.minefieldId = p->getId();
        out.radius = p->getRadius().orElse(0);
        out.center = p->getPosition().orElse(game::map::Point());
        out.controllingPlanetId = (t != 0 ? t->universe().findControllingPlanetId(*p, g->mapConfiguration()) : 0);

        // Textual information
        // - Owner
        int owner;
        if (p->getOwner().get(owner)) {
            out.text[Owner] = r->playerList().getPlayerName(owner, Player::ShortName, tx);
        }

        // - Size
        out.text[Radius] = Format(tx("%d ly radius"), fmt.formatNumber(out.radius));
        out.text[Units]  = Format(tx("%d units"), fmt.formatNumber(p->getUnits()));

        // - After decay
        const int32_t afterDecay = p->getUnitsAfterDecay(p->getUnits(), r->hostVersion(), r->hostConfiguration());
        out.text[AfterDecay] = Format(tx("%d units (%d ly)"),
                                      fmt.formatNumber(afterDecay),
                                      fmt.formatNumber(Minefield::getRadiusFromUnits(afterDecay)));

        // - Last info
        out.text[LastInfo] = util::formatAge(g->currentTurn().getTurnNumber(), p->getTurnLastSeen(), tx);

        // - Controlling planet
        if (const game::map::Planet* pl = t->universe().planets().get(out.controllingPlanetId)) {
            out.text[ControlPlanet] = pl->getName(tx);

            int planetOwner;
            if (!pl->getOwner().get(planetOwner)) {
                out.text[ControlPlayer] = tx("a planet with unknown owner");
            } else if (planetOwner == 0) {
                out.text[ControlPlayer] = tx("unowned planet");
            } else if (planetOwner == g->getViewpointPlayer()) {
                out.text[ControlPlayer] = tx("our planet");
            } else {
                out.text[ControlPlayer] = r->playerList().getPlayerName(planetOwner, Player::ShortName, tx);
            }
        } else {
            out.text[ControlPlanet] = tx("unknown");
            out.text[ControlPlayer] = String_t();
        }
    }
}

inline void
game::proxy::MinefieldProxy::Trampoline::buildPassageInfo(PassageInfo& out) const
{
    // Distance (just echo back)
    out.distance = m_passageDistance;

    // Ratios
    const Minefield* p = getMinefield();
    const Root* r = m_session.getRoot().get();
    const Game* g = m_session.getGame().get();
    if (p != 0 && r != 0 && g != 0) {
        out.normalPassageRate  = p->getPassRate(m_passageDistance, false, g->getViewpointPlayer(), r->hostConfiguration());
        out.cloakedPassageRate = p->getPassRate(m_passageDistance, true,  g->getViewpointPlayer(), r->hostConfiguration());
    } else {
        out.normalPassageRate = 0.0;
        out.cloakedPassageRate = 0.0;
    }
}

void
game::proxy::MinefieldProxy::Trampoline::browse(game::map::ObjectCursor::Mode mode, bool marked)
{
    if (m_observer.get() != 0) {
        m_observer->cursor().browse(mode, marked);
    }
}

inline void
game::proxy::MinefieldProxy::Trampoline::erase(Id_t id)
{
    if (m_observer.get() != 0) {
        if (game::map::MinefieldType* ty = dynamic_cast<game::map::MinefieldType*>(m_observer->getObjectType())) {
            ty->erase(id);
        }
    }
}

void
game::proxy::MinefieldProxy::Trampoline::onObjectChange()
{
    if (m_observer.get() != 0) {
        // Check whether object pointer changed; reset PassageInfo in that case
        Minefield*const p = getMinefield();
        const bool change = (p != m_lastObject);
        m_lastObject = p;

        // MinefieldInfo
        sendMinefieldInfo();

        // If object changes, reset PassageInfo
        if (change) {
            m_passageDistance = (p != 0 ? p->getRadius().orElse(0) : 0);
            sendPassageInfo();
        }

        // Inform listeners
        for (size_t i = 0; i < m_listeners.size(); ++i) {
            m_listeners[i]->handle(m_session, p);
        }
    }
}

Minefield*
game::proxy::MinefieldProxy::Trampoline::getMinefield() const
{
    if (m_observer.get() != 0) {
        return dynamic_cast<Minefield*>(m_observer->getCurrentObject());
    } else {
        return 0;
    }
}

inline void
game::proxy::MinefieldProxy::Trampoline::sendMinefieldInfo()
{
    class Task : public util::Request<MinefieldProxy> {
     public:
        Task(const Trampoline& tpl)
            : m_info()
            { tpl.buildMinefieldInfo(m_info); }
        virtual void handle(MinefieldProxy& proxy)
            { proxy.sig_minefieldChange.raise(m_info); }
     private:
        MinefieldInfo m_info;
    };
    m_reply.postNewRequest(new Task(*this));
}

void
game::proxy::MinefieldProxy::Trampoline::sendPassageInfo()
{
    class Task : public util::Request<MinefieldProxy> {
     public:
        Task(Trampoline& tpl)
            : m_info()
            { tpl.buildPassageInfo(m_info); }
        virtual void handle(MinefieldProxy& proxy)
            { proxy.sig_passageChange.raise(m_info); }
     private:
        PassageInfo m_info;
    };
    m_reply.postNewRequest(new Task(*this));
}

/*
 *  TrampolineFromSession
 */

class game::proxy::MinefieldProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(util::RequestSender<MinefieldProxy> reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply); }
 private:
    util::RequestSender<MinefieldProxy> m_reply;
};


/*
 *  MinefieldProxy
 */

game::proxy::MinefieldProxy::MinefieldProxy(util::RequestDispatcher& reply, util::RequestSender<Session> gameSender)
    : m_reply(reply, *this),
      m_trampoline(gameSender.makeTemporary(new TrampolineFromSession(m_reply.getSender())))
{ }

game::proxy::MinefieldProxy::~MinefieldProxy()
{ }

void
game::proxy::MinefieldProxy::setPassageDistance(int distance)
{
    m_trampoline.postRequest(&Trampoline::setPassageDistance, distance);
}

void
game::proxy::MinefieldProxy::getSweepInfo(WaitIndicator& ind, SweepInfo& out)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(SweepInfo& out)
            : m_out(out)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.buildSweepInfo(m_out); }
     private:
        SweepInfo& m_out;
    };
    Task t(out);
    ind.call(m_trampoline, t);
}

void
game::proxy::MinefieldProxy::browse(game::map::ObjectCursor::Mode mode, bool marked)
{
    m_trampoline.postRequest(&Trampoline::browse, mode, marked);
}

void
game::proxy::MinefieldProxy::erase(Id_t id)
{
    m_trampoline.postRequest(&Trampoline::erase, id);
}

// ObjectObserver:
void
game::proxy::MinefieldProxy::addNewListener(ObjectListener* pl)
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
