/**
  *  \file game/proxy/buildpartsproxy.cpp
  *  \brief Class game::proxy::BuildPartsProxy
  */

#include "game/proxy/buildpartsproxy.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/planetstorage.hpp"
#include "game/map/universe.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/turn.hpp"

/*
 *  Trampoline
 */

class game::proxy::BuildPartsProxy::Trampoline {
 public:
    Trampoline(Session& session, util::RequestSender<BuildPartsProxy> reply, Id_t id);
    ~Trampoline();

    int getPlanetOwner() const;

    void selectPart(TechLevel area, int id);
    void add(int amount);
    void commit();

    void packStatus(Status& st);

    void onChange();

 private:
    Session& m_session;
    util::RequestSender<BuildPartsProxy> m_reply;
    Id_t m_id;

    // We'll be making dumb pointers to these objects, so make smart ones to keep them alive:
    afl::base::Ptr<Game> m_pGame;
    afl::base::Ptr<Root> m_pRoot;
    afl::base::Ptr<game::spec::ShipList> m_pShipList;

    // Working objects
    Game& m_game;
    Root& m_root;
    game::spec::ShipList& m_shipList;
    game::map::Planet& m_planet;

    game::map::PlanetStorage m_storage;
    game::actions::BuildParts m_action;

    TechLevel m_currentArea;
    Id_t m_currentId;
    const game::spec::Component* m_currentPart;
};

game::proxy::BuildPartsProxy::Trampoline::Trampoline(Session& session, util::RequestSender<BuildPartsProxy> reply, Id_t id)
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
      m_storage(m_planet, m_root.hostConfiguration()),
      m_action(m_planet, m_storage, m_shipList, m_root),
      m_currentArea(HullTech),
      m_currentId(0),
      m_currentPart(0)
{
    m_action.setUndoInformation(m_game.currentTurn().universe());
    m_action.sig_change.add(this, &Trampoline::onChange);
}

game::proxy::BuildPartsProxy::Trampoline::~Trampoline()
{ }

inline int
game::proxy::BuildPartsProxy::Trampoline::getPlanetOwner() const
{
    int playerNr = 0;
    m_planet.getOwner(playerNr);
    return playerNr;
}

void
game::proxy::BuildPartsProxy::Trampoline::selectPart(TechLevel area, int id)
{
    m_currentArea = area;
    m_currentPart = m_shipList.getComponent(area, id);
    if (area == HullTech) {
        m_currentId = m_shipList.hullAssignments().getIndexFromHull(m_root.hostConfiguration(), getPlanetOwner(), id);
    } else {
        m_currentId = id;
    }
    onChange();
}

void
game::proxy::BuildPartsProxy::Trampoline::add(int amount)
{
    if (m_currentId != 0) {
        m_action.add(m_currentArea, m_currentId, amount, true);
    }
}

void
game::proxy::BuildPartsProxy::Trampoline::commit()
{
    m_action.commit();
    m_session.notifyListeners();
}

void
game::proxy::BuildPartsProxy::Trampoline::packStatus(Status& st)
{
    // Action status
    st.status           = m_action.getStatus();

    // Current part
    st.name             = (m_currentPart != 0 ? m_currentPart->getName(m_shipList.componentNamer()) : String_t());
    st.numExistingParts = m_action.getNumExistingParts(m_currentArea, m_currentId);
    st.numParts         = m_action.getNumParts(m_currentArea, m_currentId);

    // Total cost
    st.cost             = m_action.costAction().getCost();
    st.available        = m_action.costAction().getAvailableAmountAsCost();
    st.remaining        = m_action.costAction().getRemainingAmountAsCost();
    st.missing          = m_action.costAction().getMissingAmountAsCost();
}

void
game::proxy::BuildPartsProxy::Trampoline::onChange()
{
    class Updater : public util::Request<BuildPartsProxy> {
     public:
        Updater(Trampoline& tpl)
            : m_status()
            { tpl.packStatus(m_status); }
        virtual void handle(BuildPartsProxy& proxy)
            { proxy.sig_change.raise(m_status); }
     private:
        Status m_status;
    };
    m_reply.postNewRequest(new Updater(*this));
}


/*
 *  TrampolineFromSession
 */

class game::proxy::BuildPartsProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(const util::RequestSender<BuildPartsProxy>& reply, Id_t id)
        : m_reply(reply), m_id(id)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply, m_id); }
 private:
    util::RequestSender<BuildPartsProxy> m_reply;
    Id_t m_id;
};


/*
 *  BuildPartsProxy
 */

game::proxy::BuildPartsProxy::BuildPartsProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& receiver, Id_t planetId)
    : m_receiver(receiver, *this),
      m_sender(gameSender.makeTemporary(new TrampolineFromSession(m_receiver.getSender(), planetId)))
{ }

game::proxy::BuildPartsProxy::~BuildPartsProxy()
{ }

void
game::proxy::BuildPartsProxy::getStatus(WaitIndicator& ind, Status& st)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(Status& st)
            : m_status(st)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.packStatus(m_status); }
     private:
        Status& m_status;
    };
    Task t(st);
    ind.call(m_sender, t);
}

void
game::proxy::BuildPartsProxy::selectPart(TechLevel area, int id)
{
    m_sender.postRequest(&Trampoline::selectPart, area, id);
}

void
game::proxy::BuildPartsProxy::add(int amount)
{
    m_sender.postRequest(&Trampoline::add, amount);
}

void
game::proxy::BuildPartsProxy::commit()
{
    m_sender.postRequest(&Trampoline::commit);
}
