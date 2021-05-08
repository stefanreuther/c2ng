/**
  *  \file game/proxy/basestorageproxy.cpp
  *  \brief Class game::proxy::BaseStorageProxy
  */

#include "game/proxy/basestorageproxy.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/registrationkey.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"

using game::spec::ShipList;
using game::map::Planet;

/*
 *  Trampoline
 */

class game::proxy::BaseStorageProxy::Trampoline {
 public:
    Trampoline(Session& session, util::RequestSender<BaseStorageProxy> reply, Id_t id);

    void packParts(TechLevel area, Parts_t& result) const;

    void packComponents(Parts_t& result, TechLevel area, const game::spec::BaseComponentVector& vec, const Planet& planet, const ShipList& sl) const;
    void packHulls(Parts_t& result, const Planet& planet, const ShipList& sl, const Root& root) const;
    Part packComponent(TechLevel area, const game::spec::Component& comp, int id, const Planet& planet, const ShipList& sl) const;
    static int getPlanetOwner(const Planet& planet);
    int getAllowedTech(TechLevel area) const;
    Planet* getPlanet() const;

    void onChange();

 private:
    Session& m_session;
    util::RequestSender<BaseStorageProxy> m_reply;
    Id_t m_id;

    afl::base::Ptr<Game> m_game;
    afl::base::Ptr<Root> m_root;
    afl::base::Ptr<ShipList> m_shipList;

    afl::base::SignalConnection conn_shiplistChange;
    afl::base::SignalConnection conn_planetChange;
};


game::proxy::BaseStorageProxy::Trampoline::Trampoline(Session& session, util::RequestSender<BaseStorageProxy> reply, Id_t id)
    : m_session(session),
      m_reply(reply),
      m_id(id),
      m_game(session.getGame()),
      m_root(session.getRoot()),
      m_shipList(session.getShipList())
{
    if (m_shipList.get() != 0) {
        conn_shiplistChange = m_shipList->sig_change.add(this, &Trampoline::onChange);
    }
    if (Planet* p = getPlanet()) {
        conn_planetChange = p->sig_change.add(this, &Trampoline::onChange);
    }
}

void
game::proxy::BaseStorageProxy::Trampoline::packParts(TechLevel area, Parts_t& result) const
{
    const Planet* pl = getPlanet();
    if (m_shipList.get() != 0 && m_root.get() != 0) {
        switch (area) {
         case HullTech:
            packHulls(result, *pl, *m_shipList, *m_root);
            break;

         case EngineTech:
            packComponents(result, area, m_shipList->engines(), *pl, *m_shipList);
            break;

         case BeamTech:
            packComponents(result, area, m_shipList->beams(), *pl, *m_shipList);
            break;

         case TorpedoTech:
            packComponents(result, area, m_shipList->launchers(), *pl, *m_shipList);
            break;
        }
    }
}

void
game::proxy::BaseStorageProxy::Trampoline::packComponents(Parts_t& result, TechLevel area, const game::spec::BaseComponentVector& vec, const Planet& planet, const ShipList& sl) const
{
    for (game::spec::Component* p = vec.findNext(0); p != 0; p = vec.findNext(p->getId())) {
        result.push_back(packComponent(area, *p, p->getId(), planet, sl));
    }
}

inline void
game::proxy::BaseStorageProxy::Trampoline::packHulls(Parts_t& result, const Planet& planet, const ShipList& sl, const Root& root) const
{
    const int playerNr = getPlanetOwner(planet);
    const game::spec::HullAssignmentList& hullAssignments = sl.hullAssignments();
    for (int i = 1, n = hullAssignments.getMaxIndex(root.hostConfiguration(), playerNr); i <= n; ++i) {
        const int hullNr = hullAssignments.getHullFromIndex(root.hostConfiguration(), playerNr, i);
        if (const game::spec::Hull* h = sl.hulls().get(hullNr)) {
            result.push_back(packComponent(HullTech, *h, i, planet, sl));
        }
    }
}

game::proxy::BaseStorageProxy::Part
game::proxy::BaseStorageProxy::Trampoline::packComponent(TechLevel area, const game::spec::Component& comp, int id, const Planet& planet, const ShipList& sl) const
{
    const int haveTech = planet.getBaseTechLevel(area).orElse(0);
    const int allowedTech = getAllowedTech(area);
    const int needTech = comp.getTechLevel();
    const TechStatus techStatus = needTech <= haveTech ? AvailableTech : needTech <= allowedTech ? BuyableTech : LockedTech;

    return Part(comp.getId(),
                planet.getBaseStorage(area, id).orElse(0),
                techStatus,
                comp.getName(sl.componentNamer()));
}

inline int
game::proxy::BaseStorageProxy::Trampoline::getPlanetOwner(const Planet& planet)
{
    int playerNr = 0;
    planet.getOwner(playerNr);
    return playerNr;
}

inline int
game::proxy::BaseStorageProxy::Trampoline::getAllowedTech(TechLevel area) const
{
    return m_root.get() != 0
        ? m_root->registrationKey().getMaxTechLevel(area)
        : 0;
}

Planet*
game::proxy::BaseStorageProxy::Trampoline::getPlanet() const
{
    return m_game.get() != 0
        ? m_game->currentTurn().universe().planets().get(m_id)
        : 0;
}

void
game::proxy::BaseStorageProxy::Trampoline::onChange()
{
    // For now, don't try to be clever and always update everything.
    // Just updating the area we just changed will not handle outside changes,
    // such as from a parallel TechUpgrade action that makes our tech available.
    class Updater : public util::Request<BaseStorageProxy> {
     public:
        Updater(const Trampoline& tpl)
            {
                for (size_t i = 0; i < NUM_TECH_AREAS; ++i) {
                    tpl.packParts(TechLevel(i), m_partsUpdate[i]);
                }
            }
        virtual void handle(BaseStorageProxy& proxy)
            {
                for (size_t i = 0; i < NUM_TECH_AREAS; ++i) {
                    proxy.sig_update.raise(TechLevel(i), m_partsUpdate[i]);
                }
            }
     private:
        Parts_t m_partsUpdate[NUM_TECH_AREAS];
    };
    m_reply.postNewRequest(new Updater(*this));
}


/*
 *  TrampolineFromSession
 */

class game::proxy::BaseStorageProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(const util::RequestSender<BaseStorageProxy>& reply, Id_t id)
        : m_reply(reply), m_id(id)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply, m_id); }
 private:
    util::RequestSender<BaseStorageProxy> m_reply;
    Id_t m_id;
};


/*
 *  BaseStorageProxy
 */

game::proxy::BaseStorageProxy::BaseStorageProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& receiver, Id_t planetId)
    : m_receiver(receiver, *this),
      m_sender(gameSender.makeTemporary(new TrampolineFromSession(m_receiver.getSender(), planetId)))
{ }

game::proxy::BaseStorageProxy::~BaseStorageProxy()
{ }

void
game::proxy::BaseStorageProxy::getParts(WaitIndicator& ind, TechLevel level, Parts_t& result)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(TechLevel level, Parts_t& result)
            : m_level(level), m_result(result)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.packParts(m_level, m_result); }
     private:
        TechLevel m_level;
        Parts_t& m_result;
    };
    Task t(level, result);
    ind.call(m_sender, t);
}
