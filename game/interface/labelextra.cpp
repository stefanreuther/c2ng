/**
  *  \file game/interface/labelextra.cpp
  *  \brief Class game::interface::LabelExtra
  */

#include "game/interface/labelextra.hpp"
#include "afl/string/format.hpp"
#include "game/game.hpp"
#include "game/interface/planetfunction.hpp"
#include "game/interface/shipfunction.hpp"
#include "game/map/anyplanettype.hpp"
#include "game/map/anyshiptype.hpp"
#include "game/map/universe.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/simpleprocedure.hpp"
#include "interpreter/values.hpp"

using afl::string::Format;
using afl::sys::LogListener;
using game::interface::LabelExtra;
using game::interface::LabelVector;
using game::map::Universe;
using interpreter::Process;
using interpreter::ProcessList;
using interpreter::SimpleProcedure;

namespace {
    // Logger name
    const char*const LOG_NAME = "game.labels";

    // Extra Identifier
    const game::ExtraIdentifier<game::Session, game::interface::LabelExtra> LABEL_ID = {{}};

    /*
     *  Loop Avoidance
     *
     *  We are allowing arbitrary expressions for labels, which can change the underlying objects.
     *  This can mean that a recomputation triggers a change, which triggers another recomputation.
     *
     *  The simple case would be an object triggering its own change, e.g. using
     *       Label.Ship = FCode:=RandomFCode()
     *  To solve this,
     *  - ignore changes that arrive while the object is updating (see LabelVector::checkObjects())
     *  - collect dirty bits in sig_preUpdate, but start actions in sig_universeChange, where they are already reset
     *  - call Session::notifyObservers() before resetting the "is-updating" status,
     *    so changes during the update are collected in "is-updating" state
     *
     *  A more complex case is an object updating another one, e.g.
     *       Label.Ship = Ship(Iterator(1).PreviousIndex(Id,"w")).FCode:=RandomFCode()
     *  This would infinitely trigger recomputation.
     *  We therefore stop updating labels after this many re-triggers.
     */
    const int LOOP_LIMIT = 20;

    // Process priority. High value to have it happen after UI actions.
    const int PRIORITY = 90;

    // Configuration options
    const game::config::StringOptionDescriptor label_ship   = { "Label.Ship" };
    const game::config::StringOptionDescriptor label_planet = { "Label.Planet" };

    /* Shortcut to retrieve the viewpoint-universe from a session */
    Universe* getUniverse(game::Session& session)
    {
        if (game::Game* g = session.getGame().get()) {
            if (game::Turn* t = g->getViewpointTurn().get()) {
                return &t->universe();
            }
        }
        return 0;
    }

    /*
     *  Script-to-LabelExtra interface
     *
     *  For now, we do not expose these as named functions to the user.
     */

    /* Common code for ships and planets */
    void updateLabel(LabelVector& vec, interpreter::Arguments& args)
    {
        args.checkArgumentCount(3);

        int32_t id = 0;
        if (!interpreter::checkIntegerArg(id, args.getNext())) {
            return;
        }
        String_t value = afl::string::strTrim(interpreter::toString(args.getNext(), false));
        bool success = interpreter::getBooleanValue(args.getNext()) > 0;

        vec.updateLabel(id, success, value);
    }

    /* updateFunction for ships */
    void IFCCSetShipLabel(game::Session& session, Process& /*proc*/, interpreter::Arguments& args)
    {
        LabelExtra* x = LabelExtra::get(session);
        if (x == 0) {
            throw interpreter::Error::contextError();
        }
        updateLabel(x->shipLabels(), args);
    }

    /* updateFunction for planets */
    void IFCCSetPlanetLabel(game::Session& session, Process& /*proc*/, interpreter::Arguments& args)
    {
        LabelExtra* x = LabelExtra::get(session);
        if (x == 0) {
            throw interpreter::Error::contextError();
        }
        updateLabel(x->planetLabels(), args);
    }
}

/** Finalizer for label updater process.
    Signals completion back to the LabelExtra. */
class game::interface::LabelExtra::Finalizer : public Process::Finalizer {
 public:
    explicit Finalizer(Session& session)
        : m_session(session)
        { }

    virtual void finalizeProcess(Process& p)
        {
            // Process should not end in any state other than Ended.
            // (This can happen, for example, if the user code calls 'End' or 'Stop'.)
            if (p.getState() != Process::Ended) {
                // Log error
                String_t msg = Format(m_session.translator()("Label update failed: %s"), interpreter::toString(p.getState(), m_session.translator()));
                if (p.getState() == Process::Failed) {
                    msg += ", ";
                    msg += p.getError().what();
                }
                m_session.log().write(LogListener::Error, LOG_NAME, msg);

                // Forcibly terminate it so we don't get tons of those processes to pile up
                p.setState(Process::Ended);
            }

            // Signal to LabelExtra
            if (LabelExtra* x = LabelExtra::get(m_session)) {
                x->onUpdateComplete();
            }
        }

 private:
    Session& m_session;
};

/*
 *  LabelExtra
 */

game::interface::LabelExtra::LabelExtra(Session& session)
    : m_session(session),
      m_shipLabels(),
      m_planetLabels(),
      m_running(false),
      m_paranoiaCounter(0),
      conn_connectionChange(session.sig_connectionChange.add(this, &LabelExtra::onConnectionChange)),
      conn_viewpointTurnChange(),
      conn_preUpdate(),
      conn_universeChange(),
      conn_configChange()
{ }

game::interface::LabelExtra::~LabelExtra()
{ }

game::interface::LabelExtra&
game::interface::LabelExtra::create(Session& session)
{
    LabelExtra* p = session.extra().get(LABEL_ID);
    if (p == 0) {
        p = session.extra().setNew(LABEL_ID, new LabelExtra(session));

        // Initial signalisation.
        // These cannot be done in the constructor as they may run a process,
        // which wants to see the Session->LabelExtra link intact (IFCCSetShipLabel, IFCCSetPlanetLabel).
        p->onConnectionChange();
        p->onConfigChange();
    }
    return *p;
}

game::interface::LabelExtra*
game::interface::LabelExtra::get(Session& session)
{
    return session.extra().get(LABEL_ID);
}

game::interface::LabelVector&
game::interface::LabelExtra::shipLabels()
{
    return m_shipLabels;
}

const game::interface::LabelVector&
game::interface::LabelExtra::shipLabels() const
{
    return m_shipLabels;
}

game::interface::LabelVector&
game::interface::LabelExtra::planetLabels()
{
    return m_planetLabels;
}

const game::interface::LabelVector&
game::interface::LabelExtra::planetLabels() const
{
    return m_planetLabels;
}

void
game::interface::LabelExtra::setConfiguration(afl::base::Optional<String_t> shipExpr,
                                              afl::base::Optional<String_t> planetExpr)
{
    if (Root* r = m_session.getRoot().get()) {
        game::config::UserConfiguration& config = r->userConfiguration();

        // Update public and local config; onConfigChange() will therefore not see a change.
        if (const String_t* s = shipExpr.get()) {
            config[label_ship].set(*s);
            m_shipLabels.setExpression(*s, m_session.world());
        }

        if (const String_t* p = planetExpr.get()) {
            config[label_planet].set(*p);
            m_planetLabels.setExpression(*p, m_session.world());
        }

        // Mark everything for update
        markObjects();

        // Notify listeners; this will update the configuration.
        // (No change to objects, these are already marked.)
        m_session.notifyListeners();

        // Clear error state to get clean error reports
        m_shipLabels.clearErrorStatus();
        m_planetLabels.clearErrorStatus();

        // Perform updates.
        // If this does not generate an update, force one.
        // (Checking m_running is not sufficient here; it might already have gotten reset.)
        if (!runUpdater()) {
            notifyCompletion();
        }
    } else {
        // Force notification although there's nothing to change.
        notifyCompletion();
    }
}

/*
 *  Events
 */

/** Session: connection change.
    If Game or Root become available, hook these.
    Next will be onConfigChange() and/or onViewpointTurnChange(). */
void
game::interface::LabelExtra::onConnectionChange()
{
    m_session.log().write(LogListener::Error, LOG_NAME, "-> onConnectionChange");

    // Connect game/viewpoint turn
    Game* g = m_session.getGame().get();
    if (g != 0) {
        conn_viewpointTurnChange = g->sig_viewpointTurnChange.add(this, &LabelExtra::onViewpointTurnChange);
    } else {
        conn_viewpointTurnChange.disconnect();
        m_shipLabels.clear();
        m_planetLabels.clear();
    }
    onViewpointTurnChange();

    // Connect root/user configuration
    Root* r = m_session.getRoot().get();
    if (r != 0) {
        conn_configChange = r->userConfiguration().sig_change.add(this, &LabelExtra::onConfigChange);
    } else {
        conn_configChange.disconnect();
    }
    onConfigChange();
}

/** Game: viewpoint turn change.
    Hook the correct universe and recompute everything. */
void
game::interface::LabelExtra::onViewpointTurnChange()
{
    // ex onTurnChanged()
    m_session.log().write(LogListener::Trace, LOG_NAME, "-> onViewpointTurnChange");
    if (Universe* u = getUniverse(m_session)) {
        conn_preUpdate = u->sig_preUpdate.add(this, &LabelExtra::onPreUpdate);
        conn_universeChange = u->sig_universeChange.add(this, &LabelExtra::onUniverseChanged);
        markObjects();
        runUpdater();
    } else {
        conn_preUpdate.disconnect();
        conn_universeChange.disconnect();
    }
}

/** Universe: before update.
    Collect dirty bits.
    Next will be onUniverseChanged(), called by Universe, if there are actual changes. */
void
game::interface::LabelExtra::onPreUpdate()
{
    // ex onUniverseChanged()
    // m_session.log().write(LogListener::Trace, LOG_NAME, "-> onPreUpdate");
    checkObjects();
}

/** Universe: changes detected.
    This runs the update process, if any.
    Next will be onUpdateComplete(). */
void
game::interface::LabelExtra::onUniverseChanged()
{
    runUpdater();
}

/** Root: configuration changed.
    Update expressions and, if needed, run the update process.
    Next will be onUpdateComplete(). */
void
game::interface::LabelExtra::onConfigChange()
{
    m_session.log().write(LogListener::Trace, LOG_NAME, "-> onConfigChange");
    if (const Root* r = m_session.getRoot().get()) {
        const game::config::UserConfiguration& config = r->userConfiguration();

        bool change = false;
        String_t shipExpr = config[label_ship]();
        if (shipExpr != m_shipLabels.getExpression()) {
            m_shipLabels.setExpression(shipExpr, m_session.world());
            change = true;
        }

        String_t planetExpr = config[label_planet]();
        if (planetExpr != m_planetLabels.getExpression()) {
            m_planetLabels.setExpression(planetExpr, m_session.world());
            change = true;
        }

        if (change) {
            markObjects();
            runUpdater();
            notifyCompletion();
        }
    }
}

/** Finalizer: completion.
    Finish the update and try to start another one. */
void
game::interface::LabelExtra::onUpdateComplete()
{
    m_session.log().write(LogListener::Trace, LOG_NAME, "-> onUpdateComplete");
    m_running = false;

    // Collect changes accumulated until here before exiting "updating" state (loop avoidance)
    m_session.notifyListeners();

    // Mark labels done updating
    m_shipLabels.finishUpdate();
    m_planetLabels.finishUpdate();

    // Try another round or notify completion
    runUpdater();
    notifyCompletion();
}

/*
 *  Actions
 */

/** Notify listener.
    Signal is emitted only when the next update isn't yet running. */
void
game::interface::LabelExtra::notifyCompletion()
{
    if (!m_running) {
        // Check/reset change markers
        bool change = m_shipLabels.hasChangedLabels() || m_planetLabels.hasChangedLabels();
        m_shipLabels.markLabelsUnchanged();
        m_planetLabels.markLabelsUnchanged();

        // Emit signal
        m_session.log().write(LogListener::Trace, LOG_NAME, Format("<- sig_change(%d)", int(change)));
        sig_change.raise(change);
    }
}

/** Check objects to update, after universe change.
    @see LabelVector::checkObjects() */
void
game::interface::LabelExtra::checkObjects()
{
    if (Universe* u = getUniverse(m_session)) {
        game::map::AnyShipType ships(u->ships());
        m_shipLabels.checkObjects(ships);

        game::map::AnyPlanetType planets(u->planets());
        m_planetLabels.checkObjects(planets);
    }
}

/** Mark objects to update, after configuration change.
    @see LabelVector::markObjects() */
void
game::interface::LabelExtra::markObjects()
{
    if (Universe* u = getUniverse(m_session)) {
        game::map::AnyShipType ships(u->ships());
        m_shipLabels.markObjects(ships);

        game::map::AnyPlanetType planets(u->planets());
        m_planetLabels.markObjects(planets);

        m_paranoiaCounter = 0;
    }
}

/** Run update process, if needed.
    Returns true if it ran a process. */
bool
game::interface::LabelExtra::runUpdater()
{
    // ex recomputeLabels (sort-of)
    // ex chartusr.pas:UpdateAfterCommand, chartusr.pas:RecalcShip, chartusr.pas:RecalcPlanet (sort-of)
    if (!m_running) {
        if (m_shipLabels.hasDirtyLabels() || m_planetLabels.hasDirtyLabels()) {
            if (m_paranoiaCounter >= LOOP_LIMIT) {
                // Exceeded the paranoia limit: discard updates
                m_shipLabels.markClean();
                m_planetLabels.markClean();
                if (m_paranoiaCounter == LOOP_LIMIT) {
                    m_session.log().write(LogListener::Warn, LOG_NAME, m_session.translator()("Too many label updates; ignoring some. Check your \"Label.Planet\" and/or \"Label.Ship\" expression."));
                    ++m_paranoiaCounter;
                }
                return false;
            } else {
                // Mark status
                m_running = true;
                ++m_paranoiaCounter;

                // Build code
                interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
                int n = m_shipLabels.compileUpdater(*bco, ShipFunction(m_session),  SimpleProcedure<Session&>(m_session, IFCCSetShipLabel));
                n += m_planetLabels.compileUpdater(*bco, PlanetFunction(m_session), SimpleProcedure<Session&>(m_session, IFCCSetPlanetLabel));
                m_session.log().write(LogListener::Debug, LOG_NAME, Format("updating %d objects", n));
                assert(!m_shipLabels.hasDirtyLabels());
                assert(!m_planetLabels.hasDirtyLabels());

                // Build process
                ProcessList& processList = m_session.processList();
                Process& proc = processList.create(m_session.world(), "(Label Updater)");
                proc.pushFrame(bco, false);
                proc.setNewFinalizer(new Finalizer(m_session));
                proc.setPriority(PRIORITY);
                processList.handlePriorityChange(proc);

                // Run process
                uint32_t pgid = processList.allocateProcessGroup();
                processList.resumeProcess(proc, pgid);
                processList.startProcessGroup(pgid);
                m_session.sig_runRequest.raise();
                return true;
            }
        } else {
            m_paranoiaCounter = 0;
            return false;
        }
    } else {
        return true;
    }
}
