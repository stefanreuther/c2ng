/**
  *  \file game/proxy/fleetproxy.cpp
  *  \brief Class game::proxy::FleetProxy
  */

#include "game/proxy/fleetproxy.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/turn.hpp"

using afl::base::SignalConnection;
using game::ref::FleetMemberList;

/*
 *  Trampoline
 *
 *  This class could listen to currentShip().sig_indexChange and do the equivalent of selectFleetMember() in that case.
 *  This would mean that a script could control the fleet screen also by updating the ship iterator.
 *  On the downside, it would make a possible cyclic event chain (ship change modifies fleet, fleet change modifies ship).
 */

class game::proxy::FleetProxy::Trampoline {
 public:
    Trampoline(Session& session, util::RequestSender<FleetProxy> reply)
        : m_session(session),
          m_pGame(session.getGame()),     // to keep it alive
          m_game(game::actions::mustHaveGame(session)),
          m_reply(reply),
          conn_fleetChange(m_game.cursors().currentFleet().sig_indexChange.add(this, &Trampoline::onFleetChange)),
          conn_viewpointTurnChange(m_game.sig_viewpointTurnChange.add(this, &Trampoline::onViewpointTurnChange)),
          conn_universeChange(),
          conn_fleetSetChange()
        {
            onFleetChange();
            onViewpointTurnChange();
        }

    void selectFleetMember(Id_t shipId);

 private:
    void onViewpointTurnChange();
    void onFleetChange();
    void onUniverseChange();
    void onFleetSetChange();

    Id_t findSuggestedMember(Id_t currentShipId, Id_t fleetId, const FleetMemberList& newList) const;

    Session& m_session;
    afl::base::Ptr<Game> m_pGame;
    Game& m_game;
    util::RequestSender<FleetProxy> m_reply;
    FleetMemberList m_lastList;

    SignalConnection conn_fleetChange;
    SignalConnection conn_viewpointTurnChange;
    SignalConnection conn_universeChange;
    SignalConnection conn_fleetSetChange;
};

void
game::proxy::FleetProxy::Trampoline::selectFleetMember(Id_t shipId)
{
    if (const Turn* t = m_game.getViewpointTurn().get()) {
        if (const game::map::Ship* sh = t->universe().ships().get(shipId)) {
            const Id_t shipId = sh->getId();
            const Id_t fleetId = sh->getFleetNumber();
            if (fleetId != 0) {
                // Update ship (this is the actual selection)
                m_game.cursors().currentShip().setCurrentIndex(shipId);

                // Update fleet.
                // If we actually selected a ship from the current fleet, this is a no-op.
                // Otherwise, clean up by selecting the correct fleet; this will trigger onFleetChange().
                m_game.cursors().currentFleet().setCurrentIndex(fleetId);

                // Update UI.
                m_reply.postRequest(&FleetProxy::onFleetMemberSelected, shipId);
            }
        }
    }
}

void
game::proxy::FleetProxy::Trampoline::onViewpointTurnChange()
{
    if (Turn* t = m_game.getViewpointTurn().get()) {
        conn_universeChange = t->universe().sig_universeChange.add(this, &Trampoline::onUniverseChange);
        conn_fleetSetChange = t->universe().fleets().sig_setChange.add(this, &Trampoline::onFleetSetChange);
    } else {
        conn_universeChange.disconnect();
        conn_fleetSetChange.disconnect();
    }
}

void
game::proxy::FleetProxy::Trampoline::onFleetChange()
{
    Turn* t = m_game.getViewpointTurn().get();
    game::map::Universe* univ = t != 0 ? &t->universe() : 0;
    game::map::Object* ship = m_game.cursors().currentFleet().getCurrentObject();

    // Deflect intermediate state: we are sitting on the same event that ObjectCursor uses to recover from a deleted fleet.
    // ObjectCursor might therefore not yet have cleaned up.
    // However, we don't want under any circumstances report "0/empty" to the UI unless we truly have no more fleets.
    // Therefore, ignore a null report if we're certain that there still are fleets;
    // we will get an additional signal when ObjectCursor has decided.
    if (ship == 0 && univ != 0 && univ->fleets().findNextIndex(0) != 0) {
        return;
    }

    // Update
    std::auto_ptr<FleetMemberList> memList(new FleetMemberList());
    Id_t memId = 0;
    if (univ != 0 && ship != 0) {
        // Build new member list
        const Id_t fleetId = ship->getId();
        memList->setFleet(*univ, fleetId);

        // Determine current fleet member Id
        memId = findSuggestedMember(m_game.cursors().currentShip().getCurrentIndex(), fleetId, *memList);
    }

    // Inform UI side
    if (*memList != m_lastList) {
        m_lastList = *memList;
        m_reply.postRequest(&FleetProxy::onFleetChange, memList, memId);
    }

    // Select desired member (no-op if already selected)
    if (memId != 0) {
        m_game.cursors().currentShip().setCurrentIndex(memId);
    }
}

void
game::proxy::FleetProxy::Trampoline::onUniverseChange()
{
    // For now, just rebuild.
    // An alternative would have been to rebuild only if any of our ships changed,
    // by watching sig_preUpdate and checking each individually.
    onFleetChange();
}

void
game::proxy::FleetProxy::Trampoline::onFleetSetChange()
{
    // Change to fleets: could mean a ship entered, left or changed fleet.
    // Must rebuild from scratch to catch all cases.
    // If we have a fleet selected, its Id will be the first list element.
    onFleetChange();
}

game::Id_t
game::proxy::FleetProxy::Trampoline::findSuggestedMember(Id_t currentShipId, Id_t fleetId, const FleetMemberList& newList) const
{
    // If ship is part of this fleet, keep it
    size_t pos;
    if (newList.find(Reference(Reference::Ship, currentShipId)).get(pos)) {
        return currentShipId;
    }

    // If ship was part of the previous reported list, and the ship now in its place is part of the fleet, use that.
    // This is the common case of deleting a single ship.
    if (m_lastList.find(Reference(Reference::Ship, currentShipId)).get(pos)) {
        if (pos >= newList.size() && pos > 0) {
            --pos;
        }
        if (const FleetMemberList::Item* p = newList.get(pos)) {
            if (m_lastList.find(p->reference).isValid()) {
                return p->reference.getId();
            }
        }
    }

    // Otherwise, select leader
    return fleetId;
}

/*
 *  TrampolineFromSession
 */

class game::proxy::FleetProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(util::RequestSender<FleetProxy> reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply); }
 private:
    util::RequestSender<FleetProxy> m_reply;
};

game::proxy::FleetProxy::FleetProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply)
    : m_reply(reply, *this),
      m_request(gameSender.makeTemporary(new TrampolineFromSession(m_reply.getSender()))),
      m_fleetMemberList(),
      m_selectedFleetMember()
{ }

game::proxy::FleetProxy::~FleetProxy()
{ }

void
game::proxy::FleetProxy::selectFleetMember(Id_t shipId)
{
    // For now, do not update stored state (onFleetMemberSelected(shipId));
    // this would jump instead of just lag when game side lags.
    m_request.postRequest(&Trampoline::selectFleetMember, shipId);
}

const game::ref::FleetMemberList&
game::proxy::FleetProxy::getFleetMemberList() const
{
    return m_fleetMemberList;
}

game::Id_t
game::proxy::FleetProxy::getSelectedFleetMember() const
{
    return m_selectedFleetMember;
}

void
game::proxy::FleetProxy::onFleetChange(std::auto_ptr<game::ref::FleetMemberList> memList, Id_t memId)
{
    if (memList.get() != 0 && (*memList != m_fleetMemberList || memId != m_selectedFleetMember)) {
        m_fleetMemberList = *memList;
        m_selectedFleetMember = memId;
        sig_change.raise();
    }
}

void
game::proxy::FleetProxy::onFleetMemberSelected(Id_t memId)
{
    if (memId != m_selectedFleetMember) {
        m_selectedFleetMember = memId;
        sig_change.raise();
    }
}
