/**
  *  \file game/proxy/basestorageproxy.cpp
  *  \brief Class game::proxy::BaseStorageProxy
  */

#include "game/proxy/basestorageproxy.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/proxy/currentstarbaseadaptor.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/registrationkey.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"

using game::spec::ShipList;
using game::map::Planet;

namespace {
    struct ComparePartsByName {
        bool operator()(const game::proxy::BaseStorageProxy::Part& a,
                        const game::proxy::BaseStorageProxy::Part& b) const
            { return a.name < b.name; }
    };
}


/*
 *  Trampoline
 */

class game::proxy::BaseStorageProxy::Trampoline {
 public:
    Trampoline(StarbaseAdaptor& adaptor, util::RequestSender<BaseStorageProxy> reply, bool allHulls);

    void packParts(TechLevel area, Parts_t& result) const;

    void packComponents(Parts_t& result, TechLevel area, const game::spec::BaseComponentVector& vec, const Planet& planet, const ShipList& sl) const;
    void packHulls(Parts_t& result, const Planet& planet, const ShipList& sl, const Root& root) const;
    void packAllHulls(Parts_t& result, const Planet& planet, const ShipList& sl, const Root& root) const;
    Part packComponent(TechLevel area, const game::spec::Component& comp, int id, const Planet& planet, const ShipList& sl) const;
    static int getPlanetOwner(const Planet& planet);
    int getAllowedTech(TechLevel area) const;

    void onChange();

 private:
    StarbaseAdaptor& m_adaptor;
    Session& m_session;
    util::RequestSender<BaseStorageProxy> m_reply;
    bool m_allHulls;

    afl::base::Ptr<Game> m_game;
    afl::base::Ptr<Root> m_root;
    afl::base::Ptr<ShipList> m_shipList;

    afl::base::SignalConnection conn_shiplistChange;
    afl::base::SignalConnection conn_planetChange;
};


game::proxy::BaseStorageProxy::Trampoline::Trampoline(StarbaseAdaptor& adaptor, util::RequestSender<BaseStorageProxy> reply, bool allHulls)
    : m_adaptor(adaptor),
      m_session(m_adaptor.session()),
      m_reply(reply),
      m_allHulls(allHulls),
      m_game(m_session.getGame()),
      m_root(m_session.getRoot()),
      m_shipList(m_session.getShipList())
{
    if (m_shipList.get() != 0) {
        conn_shiplistChange = m_shipList->sig_change.add(this, &Trampoline::onChange);
    }
    conn_planetChange = m_adaptor.planet().sig_change.add(this, &Trampoline::onChange);
}

void
game::proxy::BaseStorageProxy::Trampoline::packParts(TechLevel area, Parts_t& result) const
{
    const Planet& pl = m_adaptor.planet();
    if (m_shipList.get() != 0 && m_root.get() != 0) {
        switch (area) {
         case HullTech:
            if (m_allHulls) {
                packAllHulls(result, pl, *m_shipList, *m_root);
            } else {
                packHulls(result, pl, *m_shipList, *m_root);
            }
            break;

         case EngineTech:
            packComponents(result, area, m_shipList->engines(), pl, *m_shipList);
            break;

         case BeamTech:
            packComponents(result, area, m_shipList->beams(), pl, *m_shipList);
            break;

         case TorpedoTech:
            packComponents(result, area, m_shipList->launchers(), pl, *m_shipList);
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

inline void
game::proxy::BaseStorageProxy::Trampoline::packAllHulls(Parts_t& result, const Planet& planet, const ShipList& sl, const Root& root) const
{
    const int playerNr = getPlanetOwner(planet);
    const game::spec::HullAssignmentList& hullAssignments = sl.hullAssignments();
    const game::spec::BaseComponentVector& vec = sl.hulls();
    for (game::spec::Component* p = vec.findNext(0); p != 0; p = vec.findNext(p->getId())) {
        result.push_back(packComponent(HullTech, *p, hullAssignments.getIndexFromHull(root.hostConfiguration(), playerNr, p->getId()), planet, sl));
    }
    std::sort(result.begin(), result.end(), ComparePartsByName());
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
    return planet.getOwner().orElse(0);
}

inline int
game::proxy::BaseStorageProxy::Trampoline::getAllowedTech(TechLevel area) const
{
    return m_root.get() != 0
        ? m_root->registrationKey().getMaxTechLevel(area)
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

class game::proxy::BaseStorageProxy::TrampolineFromAdaptor : public afl::base::Closure<Trampoline*(StarbaseAdaptor&)> {
 public:
    TrampolineFromAdaptor(const util::RequestSender<BaseStorageProxy>& reply, bool allHulls)
        : m_reply(reply), m_allHulls(allHulls)
        { }
    virtual Trampoline* call(StarbaseAdaptor& adaptor)
        { return new Trampoline(adaptor, m_reply, m_allHulls); }
 private:
    util::RequestSender<BaseStorageProxy> m_reply;
    bool m_allHulls;
};


/*
 *  BaseStorageProxy
 */

game::proxy::BaseStorageProxy::BaseStorageProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& receiver, Id_t planetId, bool allHulls)
    : m_receiver(receiver, *this),
      m_sender(gameSender.makeTemporary(new CurrentStarbaseAdaptorFromSession(planetId)).makeTemporary(new TrampolineFromAdaptor(m_receiver.getSender(), allHulls))),
      m_allHulls(allHulls)
{ }

game::proxy::BaseStorageProxy::BaseStorageProxy(util::RequestSender<StarbaseAdaptor> adaptorSender, util::RequestDispatcher& receiver, bool allHulls)
    : m_receiver(receiver, *this),
      m_sender(adaptorSender.makeTemporary(new TrampolineFromAdaptor(m_receiver.getSender(), allHulls))),
      m_allHulls(allHulls)
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

bool
game::proxy::BaseStorageProxy::hasAllHulls() const
{
    return m_allHulls;
}
