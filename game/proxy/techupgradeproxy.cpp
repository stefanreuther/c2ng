/**
  *  \file game/proxy/techupgradeproxy.cpp
  *  \brief Class game::proxy::TechUpgradeProxy
  */

#include "game/proxy/techupgradeproxy.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "game/actions/preconditions.hpp"
#include "game/actions/techupgrade.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/planetstorage.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/turn.hpp"

using game::spec::ShipList;
using game::spec::Cost;
using afl::base::Ptr;

namespace {
    // FIXME: elsewhere?
    template<typename T>
    T& mustExist(T* p, const char* what)
    {
        afl::except::checkAssertion(p != 0, what);
        return *p;
    }
}


/*
 *  Trampoline
 */

class game::proxy::TechUpgradeProxy::Trampoline {
 public:
    Trampoline(Session& session, util::RequestSender<TechUpgradeProxy> reply, Id_t planetId);

    void packStatus(Status& st);

    void setAll(Order order);
    void setTechLevel(TechLevel area, int value);
    void upgradeTechLevel(TechLevel area, int value);
    void commit();

    void onChange();

 private:
    Session& m_session;
    util::RequestSender<TechUpgradeProxy> m_reply;

    Ptr<Turn> m_pTurn;
    Ptr<ShipList> m_pShipList;
    Ptr<Root> m_pRoot;

    Turn& m_turn;
    Root& m_root;

    game::map::Planet& m_planet;
    game::map::PlanetStorage m_container;
    game::actions::TechUpgrade m_action;
};

game::proxy::TechUpgradeProxy::Trampoline::Trampoline(Session& session, util::RequestSender<TechUpgradeProxy> reply, Id_t planetId)
    : m_session(session),
      m_reply(reply),

      // Keep objects alive
      m_pTurn(game::actions::mustHaveGame(session).getViewpointTurn()),
      m_pShipList(session.getShipList()),
      m_pRoot(session.getRoot()),

      // Readymade objects
      m_turn(mustExist(m_pTurn.get(), "<Trampoline:Turn>")),
      m_root(game::actions::mustHaveRoot(session)),
      m_planet(mustExist(m_turn.universe().planets().get(planetId), "<Trampoline:Planet>")),
      m_container(m_planet, m_root.hostConfiguration(), session.translator()),
      m_action(m_planet, m_container, game::actions::mustHaveShipList(session), m_root, session.translator())
{
    m_action.setUndoInformation(m_pTurn->universe());
    m_action.sig_change.add(this, &Trampoline::onChange);
}

void
game::proxy::TechUpgradeProxy::Trampoline::packStatus(Status& st)
{
    st.cost = m_action.costAction().getCost();
    st.available.set(Cost::Money, m_container.getAmount(game::Element::Money));
    st.available.set(Cost::Supplies, m_container.getAmount(game::Element::Supplies));
    st.remaining = m_action.costAction().getRemainingAmountAsCost();
    st.missing = m_action.costAction().getMissingAmountAsCost();
    for (size_t i = 0; i < NUM_TECH_AREAS; ++i) {
        st.min[i] = m_action.getMinTechLevel(TechLevel(i));
        st.max[i] = m_action.getMaxTechLevel(TechLevel(i));
        st.current[i] = m_action.getTechLevel(TechLevel(i));
    }
    st.status = m_action.getStatus();
}

void
game::proxy::TechUpgradeProxy::Trampoline::setAll(Order order)
{
    for (size_t i = 0; i < NUM_TECH_AREAS; ++i) {
        setTechLevel(TechLevel(i), order.values[i]);
    }
}

void
game::proxy::TechUpgradeProxy::Trampoline::setTechLevel(TechLevel area, int value)
{
    m_action.setTechLevel(area, value);
}

void
game::proxy::TechUpgradeProxy::Trampoline::upgradeTechLevel(TechLevel area, int value)
{
    m_action.upgradeTechLevel(area, value);
}

void
game::proxy::TechUpgradeProxy::Trampoline::commit()
{
    m_action.commit();
    m_session.notifyListeners();
}

void
game::proxy::TechUpgradeProxy::Trampoline::onChange()
{
    class Task : public util::Request<TechUpgradeProxy> {
     public:
        Task(Trampoline& tpl)
            { tpl.packStatus(m_status); }
        virtual void handle(TechUpgradeProxy& proxy)
            { proxy.sig_change.raise(m_status); }
     private:
        Status m_status;
    };
    m_reply.postNewRequest(new Task(*this));
}



/*
 *  TrampolineFromSession
 */

class game::proxy::TechUpgradeProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(util::RequestSender<TechUpgradeProxy> reply, Id_t planetId)
        : m_reply(reply), m_planetId(planetId)
        { }

    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply, m_planetId); }
 private:
    util::RequestSender<TechUpgradeProxy> m_reply;
    Id_t m_planetId;
};


/*
 *  TechUpgradeProxy
 */

game::proxy::TechUpgradeProxy::TechUpgradeProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& receiver, Id_t planetId)
    : m_receiver(receiver, *this),
      m_sender(gameSender.makeTemporary(new TrampolineFromSession(m_receiver.getSender(), planetId)))
{ }

game::proxy::TechUpgradeProxy::~TechUpgradeProxy()
{ }

void
game::proxy::TechUpgradeProxy::getStatus(WaitIndicator& ind, Status& result)
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
game::proxy::TechUpgradeProxy::setAll(const Order& order)
{
    m_sender.postRequest(&Trampoline::setAll, order);
}

void
game::proxy::TechUpgradeProxy::setTechLevel(TechLevel area, int value)
{
    m_sender.postRequest(&Trampoline::setTechLevel, area, value);
}

void
game::proxy::TechUpgradeProxy::upgradeTechLevel(TechLevel area, int value)
{
    m_sender.postRequest(&Trampoline::upgradeTechLevel, area, value);
}

void
game::proxy::TechUpgradeProxy::commit()
{
    m_sender.postRequest(&Trampoline::commit);
}
