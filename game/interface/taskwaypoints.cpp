/**
  *  \file game/interface/taskwaypoints.cpp
  *  \brief Class game::interface::TaskWaypoints
  */

#include "game/interface/taskwaypoints.hpp"

#include "game/game.hpp"
#include "game/interface/shiptaskpredictor.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"
#include "interpreter/basetaskeditor.hpp"

using game::map::Ship;
using game::spec::ShipList;
using interpreter::BaseTaskEditor;
using interpreter::Process;
using interpreter::ProcessList;

namespace {
    const game::ExtraIdentifier<game::Session, game::interface::TaskWaypoints> EXTRA_ID = {{ }};
}

game::interface::TaskWaypoints::TaskWaypoints(Session& session)
    : m_session(session),
      m_data(),
      conn_processStateChanged(session.processList().sig_processStateChange.add(this, &TaskWaypoints::updateProcess)),
      conn_connectionChange(session.sig_connectionChange.add(this, &TaskWaypoints::updateAll))
{
    updateAll();
}

game::interface::TaskWaypoints::~TaskWaypoints()
{ }

const game::interface::TaskWaypoints::Track*
game::interface::TaskWaypoints::getTrack(Id_t id) const
{
    if (id > 0 && id <= Id_t(m_data.size())) {
        return m_data[id-1];
    } else {
        return 0;
    }
}

void
game::interface::TaskWaypoints::updateAll()
{
    // Processes live in the Session.
    // Therefore, a change in Game or Root itself does not cause a Track to disappear;
    // thus, there is no need to track-and-remove unseen Tracks here.
    const ProcessList::Vector_t& vec = m_session.processList().getProcessList();
    for (size_t i = 0; i < vec.size(); ++i) {
        if (const Process* p = vec[i]) {
            updateProcess(*p, false);
        }
    }
}

void
game::interface::TaskWaypoints::updateProcess(const interpreter::Process& proc, bool willDelete)
{
    // Reject if not a ship task
    if (proc.getProcessKind() != Process::pkShipTask) {
        return;
    }

    // Reject if not a valid ship. Check ID to avoid unbounded allocation.
    Ship* sh = dynamic_cast<Ship*>(proc.getInvokingObject());
    if (sh == 0 || sh->getId() <= 0 || sh->getId() > MAX_NUMBER) {
        return;
    }

    // Parse task. If preconditions not valid, leave Track empty.
    Track track;
    Game* g = m_session.getGame().get();
    const Root* r = m_session.getRoot().get();
    const ShipList* sl = m_session.getShipList().get();
    if (!willDelete && g != 0 && r != 0 && sl != 0) {
        // Predictor
        ShipTaskPredictor pred(g->currentTurn().universe(), sh->getId(), g->shipScores(), *sl, g->mapConfiguration(), r->hostConfiguration(), r->hostVersion(), r->registrationKey());
        pred.setMovementMode(ShipTaskPredictor::SimpleMovement);

        // Re-use existing editor, if any.
        // Since this is normally triggered by a task that just executed, it will usually use a temporary editor.
        // We must not use Session::getAutoTaskEditor / Session::releaseAutoTaskEditor because that will run the auto task, causing recursion.
        const BaseTaskEditor* existingEditor = dynamic_cast<BaseTaskEditor*>(proc.getFreezer());
        if (existingEditor != 0) {
            pred.predictTask(*existingEditor);
        } else {
            BaseTaskEditor tempEditor;
            tempEditor.load(proc);
            pred.predictTask(tempEditor);
        }

        // Update waypoints
        for (size_t i = 0, n = pred.getNumPositions(); i < n; ++i) {
            track.waypoints.push_back(pred.getPosition(i));
        }
    }

    // Update
    Track* existingTrack = const_cast<Track*>(getTrack(sh->getId()));
    bool changed;
    if (track.waypoints.empty()) {
        if (existingTrack != 0) {
            // Have an existing track, but new track is empty
            m_data.replaceElementNew(sh->getId()-1, 0);
            changed = true;
        } else {
            // No track
            changed = false;
        }
    } else {
        if (existingTrack == 0) {
            // No existing track, but new one is nonempty
            if (m_data.size() < size_t(sh->getId())) {
                m_data.resize(sh->getId());
            }
            m_data.replaceElementNew(sh->getId()-1, new Track(track));
            changed = true;
        } else if (existingTrack->waypoints != track.waypoints) {
            // Existing track differs from new one
            existingTrack->waypoints.swap(track.waypoints);
            changed = true;
        } else {
            // No change
            changed = false;
        }
    }

    // Notify universe (trigger map redraw)
    if (changed && g != 0) {
        g->currentTurn().universe().markChanged();
    }
}

game::interface::TaskWaypoints&
game::interface::TaskWaypoints::create(Session& session)
{
    TaskWaypoints* p = session.extra().get(EXTRA_ID);
    if (p == 0) {
        p = session.extra().setNew(EXTRA_ID, new TaskWaypoints(session));
    }
    return *p;
}

game::interface::TaskWaypoints*
game::interface::TaskWaypoints::get(Session& session)
{
    return session.extra().get(EXTRA_ID);
}
