/**
  *  \file game/proxy/buildshipproxy.cpp
  *  \brief Class game::proxy::BuildShipProxy
  */

#include "game/proxy/buildshipproxy.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/planetstorage.hpp"
#include "game/map/shiputils.hpp"
#include "game/map/universe.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/registrationkey.hpp"
#include "game/turn.hpp"

class game::proxy::BuildShipProxy::Trampoline {
 public:
    Trampoline(Session& session, util::RequestSender<BuildShipProxy> reply, Id_t id);

    void selectPart(TechLevel area, int id);
    void setPart(TechLevel area, int id);
    void setNumParts(Weapon_t area, int amount);
    void addParts(Weapon_t area, int delta);
    void setUsePartsFromStorage(bool flag);
    void commit();
    void cancel();

    void packStatus(Status& st);
    void getCostSummary(game::spec::CostSummary& result);
    void getQuery(ShipQuery& result);

    bool findShipCloningHere(Id_t& id, String_t& name);
    void cancelAllCloneOrders();
    void sendStatus();

 private:
    Session& m_session;
    util::RequestSender<BuildShipProxy> m_reply;
    Id_t m_id;

    // We'll be making dumb pointers to these objects, so make smart ones to keep them alive:
    afl::base::Ptr<Game> m_pGame;
    afl::base::Ptr<Root> m_pRoot;
    afl::base::Ptr<game::spec::ShipList> m_pShipList;

    // Working objects:
    Game& m_game;
    Root& m_root;
    game::spec::ShipList& m_shipList;
    game::map::Planet& m_planet;
    game::map::PlanetStorage m_container;
    game::actions::BuildShip m_action;

    // Focused part:
    TechLevel m_partArea;
    int m_partId;
};

game::proxy::BuildShipProxy::Trampoline::Trampoline(Session& session, util::RequestSender<BuildShipProxy> reply, Id_t id)
    : m_session(session),
      m_reply(reply),
      m_id(id),
      m_pGame(session.getGame()),
      m_pRoot(session.getRoot()),
      m_pShipList(session.getShipList()),
      m_game(game::actions::mustHaveGame(session)),
      m_root(game::actions::mustHaveRoot(session)),
      m_shipList(game::actions::mustHaveShipList(session)),
      m_planet(game::actions::mustExist(m_game.currentTurn().universe().planets().get(id))),
      m_container(m_planet, m_root.hostConfiguration()),
      m_action(m_planet, m_container, m_shipList, m_root),
      m_partArea(HullTech),
      m_partId(m_action.getBuildOrder().getHullIndex())
{
    m_action.sig_change.add(this, &Trampoline::sendStatus);
}

void
game::proxy::BuildShipProxy::Trampoline::selectPart(TechLevel area, int id)
{
    m_partArea = area;
    m_partId = id;
    sendStatus();
}

void
game::proxy::BuildShipProxy::Trampoline::setPart(TechLevel area, int id)
{
    m_action.setPart(area, id);
}

void
game::proxy::BuildShipProxy::Trampoline::setNumParts(Weapon_t area, int amount)
{
    m_action.setNumParts(area, amount);
}

void
game::proxy::BuildShipProxy::Trampoline::addParts(Weapon_t area, int delta)
{
    m_action.addParts(area, delta);
}

void
game::proxy::BuildShipProxy::Trampoline::setUsePartsFromStorage(bool flag)
{
    m_action.setUsePartsFromStorage(flag);
}

void
game::proxy::BuildShipProxy::Trampoline::commit()
{
    m_action.commit();
    m_session.notifyListeners();
}

void
game::proxy::BuildShipProxy::Trampoline::cancel()
{
    m_planet.setBaseBuildOrder(ShipBuildOrder());
    m_session.notifyListeners();
}

void
game::proxy::BuildShipProxy::Trampoline::packStatus(Status& st) /*const - cannot be used because m_action.getStatus() is not const*/
{
    // Status
    st.status    = m_action.getStatus();

    // Costs
    st.totalCost = m_action.costAction().getCost();
    st.available = m_action.costAction().getAvailableAmountAsCost();
    st.remaining = m_action.costAction().getRemainingAmountAsCost();
    st.missing   = m_action.costAction().getMissingAmountAsCost();

    // Part
    if (const game::spec::Component* p = m_shipList.getComponent(m_partArea, m_partId)) {
        st.partTech = p->getTechLevel();
        st.availableTech = m_planet.getBaseTechLevel(m_partArea).orElse(0);
        st.partCost = p->cost();
    }

    // Order
    st.order = m_action.getBuildOrder();
    st.order.describe(st.description, m_shipList, m_session.translator());

    // Engine limits
    if (const game::spec::Hull* h = m_shipList.hulls().get(m_action.getBuildOrder().getHullIndex())) {
        st.numEngines = h->getNumEngines();
        st.maxBeams = h->getMaxBeams();
        st.maxLaunchers = h->getMaxLaunchers();
    }

    // Flags
    st.isNew = m_planet.getBaseBuildOrderHullIndex().orElse(0) == 0;
    st.isUsePartsFromStorage = m_action.isUsePartsFromStorage();
    st.isChange = m_action.isChange();
}

void
game::proxy::BuildShipProxy::Trampoline::getCostSummary(game::spec::CostSummary& result)
{
    m_action.getCostSummary(result, m_session.translator());
}

inline void
game::proxy::BuildShipProxy::Trampoline::getQuery(ShipQuery& result)
{
    result = m_action.getQuery();
}

inline bool
game::proxy::BuildShipProxy::Trampoline::findShipCloningHere(Id_t& id, String_t& name)
{
    const game::map::Universe& univ = m_game.currentTurn().universe();
    if (Id_t shipId = univ.findShipCloningAt(m_planet.getId())) {
        const game::map::Ship* sh = univ.ships().get(shipId);
        id = shipId;
        name = (sh != 0 ? sh->getName() : String_t());
        return true;
    } else {
        return false;
    }
}

void
game::proxy::BuildShipProxy::Trampoline::cancelAllCloneOrders()
{
    game::map::cancelAllCloneOrders(m_game.currentTurn().universe(), m_planet, m_shipList.friendlyCodes(), m_session.rng());
    m_session.notifyListeners();
}

void
game::proxy::BuildShipProxy::Trampoline::sendStatus()
{
    class Task : public util::Request<BuildShipProxy> {
     public:
        Task(Trampoline& tpl)
            { tpl.packStatus(m_status); }

        virtual void handle(BuildShipProxy& proxy)
            { proxy.sig_change.raise(m_status); }

     private:
        Status m_status;
    };
    m_reply.postNewRequest(new Task(*this));
}



/*
 *  TrampolineFromSession
 */

class game::proxy::BuildShipProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(const util::RequestSender<BuildShipProxy>& reply, Id_t id)
        : m_reply(reply), m_id(id)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply, m_id); }
 private:
    util::RequestSender<BuildShipProxy> m_reply;
    Id_t m_id;
};

game::proxy::BuildShipProxy::BuildShipProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& receiver, Id_t planetId)
    : m_receiver(receiver, *this),
      m_sender(gameSender.makeTemporary(new TrampolineFromSession(m_receiver.getSender(), planetId)))
{ }

game::proxy::BuildShipProxy::~BuildShipProxy()
{ }

void
game::proxy::BuildShipProxy::getStatus(WaitIndicator& ind, Status& result)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(Status& result)
            : m_result(result)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.packStatus(m_result); }
     private:
        Status& m_result;
    };
    Task t(result);
    ind.call(m_sender, t);
}

void
game::proxy::BuildShipProxy::getCostSummary(WaitIndicator& ind, game::spec::CostSummary& result)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(game::spec::CostSummary& result)
            : m_result(result)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.getCostSummary(m_result); }
     private:
        game::spec::CostSummary& m_result;
    };
    Task t(result);
    ind.call(m_sender, t);
}

game::ShipQuery
game::proxy::BuildShipProxy::getQuery(WaitIndicator& ind)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(ShipQuery& result)
            : m_result(result)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.getQuery(m_result); }
     private:
        ShipQuery& m_result;
    };

    ShipQuery result;
    Task t(result);
    ind.call(m_sender, t);
    return result;
}

bool
game::proxy::BuildShipProxy::findShipCloningHere(WaitIndicator& ind, Id_t& id, String_t& name)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(Id_t& id, String_t& name)
            : m_id(id), m_name(name), m_result(false)
            { }
        virtual void handle(Trampoline& tpl)
            { m_result = tpl.findShipCloningHere(m_id, m_name); }
        bool getResult()
            { return m_result; }
     private:
        Id_t& m_id;
        String_t& m_name;
        bool m_result;
    };
    Task t(id, name);
    ind.call(m_sender, t);
    return t.getResult();
}

void
game::proxy::BuildShipProxy::cancelAllCloneOrders()
{
    m_sender.postRequest(&Trampoline::cancelAllCloneOrders);
}

void
game::proxy::BuildShipProxy::selectPart(TechLevel area, int id)
{
    m_sender.postRequest(&Trampoline::selectPart, area, id);
}

void
game::proxy::BuildShipProxy::setPart(TechLevel area, int id)
{
    m_sender.postRequest(&Trampoline::setPart, area, id);
}

void
game::proxy::BuildShipProxy::setNumParts(Weapon_t area, int amount)
{
    m_sender.postRequest(&Trampoline::setNumParts, area, amount);
}

void
game::proxy::BuildShipProxy::addParts(Weapon_t area, int delta)
{
    m_sender.postRequest(&Trampoline::addParts, area, delta);
}

void
game::proxy::BuildShipProxy::setUsePartsFromStorage(bool flag)
{
    m_sender.postRequest(&Trampoline::setUsePartsFromStorage, flag);
}

void
game::proxy::BuildShipProxy::commit()
{
    m_sender.postRequest(&Trampoline::commit);
}

void
game::proxy::BuildShipProxy::cancel()
{
    m_sender.postRequest(&Trampoline::cancel);
}
