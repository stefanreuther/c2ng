/**
  *  \file game/proxy/buildammoproxy.cpp
  *  \brief Class game::proxy::BuildAmmoProxy
  */

#include "game/proxy/buildammoproxy.hpp"
#include "afl/base/deleter.hpp"
#include "game/actions/buildammo.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/map/planetstorage.hpp"
#include "game/map/shipstorage.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/registrationkey.hpp"
#include "game/turn.hpp"

using game::actions::mustExist;

/*
 *  Trampoline
 */

class game::proxy::BuildAmmoProxy::Trampoline {
 public:
    Trampoline(util::RequestSender<BuildAmmoProxy> reply, Session& session, Id_t planetId);

    void setPlanet();
    void setShip(Id_t shipId);
    void addLimitCash(Element::Type type, int count);
    void commit();

    void packStatus(Status& out) const;

 private:
    util::RequestSender<BuildAmmoProxy> m_reply;        // Messages back to BuildAmmoProxy
    Session& m_session;                                 // Session
    afl::base::Deleter m_deleter;                       // Deleter; stored objects related to BuildAmmo action
    std::auto_ptr<game::actions::BuildAmmo> m_pAction;  // Action (can be null)
    afl::base::Ptr<Turn> m_pTurn;                       // Turn (to keep it alive)
    game::map::Planet& m_planet;                        // Planet
    String_t m_targetName;                              // Name of target planet/ship
    afl::base::SignalConnection conn_targetChange;      // Signal for change of target container
    afl::base::SignalConnection conn_sourceChange;      // Signal for change of source container

    bool isValidReceiver(const game::map::Ship& ship) const;
    void finishAction(String_t targetName, game::CargoContainer& target, game::CargoContainer& source);
    void reset();
    void addPart(Status& out, Part& pt) const;
    void sendStatus();
};

game::proxy::BuildAmmoProxy::Trampoline::Trampoline(util::RequestSender<BuildAmmoProxy> reply, Session& session, Id_t planetId)
    : m_reply(reply),
      m_session(session),
      m_deleter(),
      m_pAction(),
      // Obtain turn and keep it alive
      m_pTurn(game::actions::mustHaveGame(session).getViewpointTurn()),
      // Obtain planet
      m_planet(mustExist(mustExist(m_pTurn.get()).universe().planets().get(planetId))),
      m_targetName(),
      conn_targetChange(),
      conn_sourceChange()
{
    game::actions::mustHavePlayedBase(m_planet);
}

void
game::proxy::BuildAmmoProxy::Trampoline::setPlanet()
{
    reset();

    Root& r = game::actions::mustHaveRoot(m_session);
    game::spec::ShipList& sl = game::actions::mustHaveShipList(m_session);

    game::map::PlanetStorage& ps = m_deleter.addNew(new game::map::PlanetStorage(m_planet, r.hostConfiguration()));
    m_pAction.reset(new game::actions::BuildAmmo(m_planet, ps, ps, sl, r));
    finishAction(m_planet.getName(m_session.translator()), ps, ps);
}

void
game::proxy::BuildAmmoProxy::Trampoline::setShip(Id_t shipId)
{
    reset();

    Root& r = game::actions::mustHaveRoot(m_session);
    game::spec::ShipList& sl = game::actions::mustHaveShipList(m_session);

    game::map::Ship& ship = mustExist(mustExist(m_pTurn.get()).universe().ships().get(shipId));
    if (isValidReceiver(ship)) {
        game::map::ShipStorage& ss = m_deleter.addNew(new game::map::ShipStorage(ship, sl));
        game::map::PlanetStorage& ps = m_deleter.addNew(new game::map::PlanetStorage(m_planet, r.hostConfiguration()));
        m_pAction.reset(new game::actions::BuildAmmo(m_planet, ps, ss, sl, r));
        finishAction(ship.getName(), ss, ps);
    }
}

void
game::proxy::BuildAmmoProxy::Trampoline::addLimitCash(Element::Type type, int count)
{
    if (m_pAction.get() != 0) {
        m_pAction->addLimitCash(type, count);
        sendStatus();
    }
}

void
game::proxy::BuildAmmoProxy::Trampoline::commit()
{
    if (m_pAction.get() != 0) {
        m_pAction->commit();
        reset();
        sendStatus();
    }
}

void
game::proxy::BuildAmmoProxy::Trampoline::packStatus(Status& out) const
{
    // Clear
    out = Status();

    // Action status
    if (m_pAction.get() != 0) {
        out.cost      = m_pAction->costAction().getCost();
        out.available = m_pAction->costAction().getAvailableAmountAsCost();
        out.remaining = m_pAction->costAction().getRemainingAmountAsCost();
        out.missing   = m_pAction->costAction().getMissingAmountAsCost();
    }
    out.targetName = m_targetName;
    out.availableTech = m_planet.getBaseTechLevel(TorpedoTech).orElse(0);

    // Torpedoes
    const game::spec::ShipList& sl = game::actions::mustHaveShipList(m_session);
    const game::spec::TorpedoVector_t& torps = sl.launchers();
    for (const game::spec::TorpedoLauncher* p = torps.findNext(0); p != 0; p = torps.findNext(p->getId())) {
        Part pt;
        pt.type         = Element::fromTorpedoType(p->getId());
        pt.page         = game::spec::info::TorpedoPage;
        pt.id           = p->getId();
        pt.name         = p->getName(sl.componentNamer());
        pt.cost         = p->torpedoCost();
        pt.techLevel    = p->getTechLevel();
        addPart(out, pt);
    }

    // Fighter
    const Root& root = game::actions::mustHaveRoot(m_session);
    Part ft;
    const int owner = m_planet.getOwner().orElse(0);
    ft.type       = Element::Fighters;
    ft.page       = game::spec::info::FighterPage;
    ft.id         = owner;
    ft.name       = m_session.translator()("Fighter");
    ft.cost       = root.hostConfiguration()[game::config::HostConfiguration::BaseFighterCost](owner);
    ft.techLevel  = 1;
    addPart(out, ft);
}

bool
game::proxy::BuildAmmoProxy::Trampoline::isValidReceiver(const game::map::Ship& ship) const
{
    Exception ex("");
    return game::actions::BuildAmmo::isValidCombination(m_planet, ship, ex)
        && (ship.getNumBays().orElse(0) > 0
            || ship.getNumLaunchers().orElse(0) > 0);
}

void
game::proxy::BuildAmmoProxy::Trampoline::finishAction(String_t targetName, game::CargoContainer& target, game::CargoContainer& source)
{
    m_pAction->setUndoInformation(m_pTurn->universe());
    m_targetName = targetName;
    conn_targetChange = target.sig_change.add(this, &Trampoline::sendStatus);
    if (&target != &source) {
        conn_sourceChange = source.sig_change.add(this, &Trampoline::sendStatus);
    }
    sendStatus();
}

void
game::proxy::BuildAmmoProxy::Trampoline::reset()
{
    conn_targetChange.disconnect();
    conn_sourceChange.disconnect();
    m_pAction.reset();
    m_deleter.clear();
}

void
game::proxy::BuildAmmoProxy::Trampoline::addPart(Status& out, Part& pt) const
{
    int availTech = m_planet.getBaseTechLevel(TorpedoTech).orElse(1);
    pt.techStatus   = (availTech >= pt.techLevel
                       ? AvailableTech
                       : pt.techLevel > game::actions::mustHaveRoot(m_session).registrationKey().getMaxTechLevel(TorpedoTech)
                       ? LockedTech
                       : BuyableTech);
    pt.isAccessible = m_pAction.get() != 0 && m_pAction->receiver().canHaveElement(pt.type);
    pt.amount       = m_pAction.get() != 0 ?  m_pAction->getAmount(pt.type) : 0;
    pt.maxAmount    = m_pAction.get() != 0 ?  m_pAction->getMaxAmount(pt.type) : 0;
    out.parts.push_back(pt);
}

void
game::proxy::BuildAmmoProxy::Trampoline::sendStatus()
{
    class Task : public util::Request<BuildAmmoProxy> {
     public:
        Task(const Trampoline& tpl)
            : m_status()
            { tpl.packStatus(m_status); }
        virtual void handle(BuildAmmoProxy& proxy)
            { proxy.sig_update.raise(m_status); }
     private:
        Status m_status;
    };
    m_reply.postNewRequest(new Task(*this));
}


/*
 *  TrampolineFromSession
 */

class game::proxy::BuildAmmoProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(util::RequestSender<BuildAmmoProxy> reply, Id_t planetId)
        : m_reply(reply), m_planetId(planetId)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(m_reply, session, m_planetId); }
 private:
    util::RequestSender<BuildAmmoProxy> m_reply;
    Id_t m_planetId;
};


/*
 *  BuildAmmoProxy
 */

game::proxy::BuildAmmoProxy::BuildAmmoProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply, Id_t planetId)
    : m_receiver(reply, *this),
      m_sender(gameSender.makeTemporary(new TrampolineFromSession(m_receiver.getSender(), planetId)))
{ }

game::proxy::BuildAmmoProxy::~BuildAmmoProxy()
{ }

void
game::proxy::BuildAmmoProxy::getStatus(WaitIndicator& ind, Status& result)
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
game::proxy::BuildAmmoProxy::setPlanet()
{
    m_sender.postRequest(&Trampoline::setPlanet);
}

void
game::proxy::BuildAmmoProxy::setShip(Id_t shipId)
{
    m_sender.postRequest(&Trampoline::setShip, shipId);
}

void
game::proxy::BuildAmmoProxy::addLimitCash(Element::Type type, int count)
{
    m_sender.postRequest(&Trampoline::addLimitCash, type, count);
}

void
game::proxy::BuildAmmoProxy::commit()
{
    m_sender.postRequest(&Trampoline::commit);
}
