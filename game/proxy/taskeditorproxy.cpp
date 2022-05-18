/**
  *  \file game/proxy/taskeditorproxy.cpp
  *  \brief Class game::proxy::TaskEditorProxy
  */

#include "game/proxy/taskeditorproxy.hpp"
#include "afl/base/signalconnection.hpp"
#include "game/actions/buildship.hpp"
#include "game/actions/cargocostaction.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/game.hpp"
#include "game/interface/basetaskbuildcommandparser.hpp"
#include "game/interface/notificationstore.hpp"
#include "game/interface/shiptaskpredictor.hpp"
#include "game/map/planetstorage.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"

using game::config::UserConfiguration;
using game::interface::NotificationStore;
using game::interface::ShipTaskPredictor;
using interpreter::Process;
using interpreter::TaskEditor;

namespace {
    void addMissingCargo(String_t& out, const char* lbl, int32_t miss, const util::NumberFormatter& fmt)
    {
        if (miss > 0) {
            if (!out.empty()) {
                out += ' ';
            }
            out += fmt.formatNumber(miss);
            out += lbl;
        }
    }
}


/*
 *  Trampoline
 */

class game::proxy::TaskEditorProxy::Trampoline {
 public:
    Trampoline(Session& session, util::RequestSender<TaskEditorProxy> reply)
        : m_session(session),
          m_reply(reply),
          m_editor(),
          m_id(),
          m_kind(Process::pkDefault)
        { }

    ~Trampoline()
        {
            // Explicitly deselect the auto-task.
            // This causes it to be scheduled to run.
            selectTask(0, Process::pkDefault, false);
        }

    void selectTask(Id_t id, Process::ProcessKind kind, bool create);
    void setCursor(size_t newCursor);
    void addAsCurrent(String_t cmd);
    void addAtEnd(String_t cmd);
    void describe(Status& out) const;
    void describeShip(ShipStatus& out) const;
    void describeBase(BaseStatus& out) const;
    void describeMessage(MessageStatus& out) const;
    void sendStatus();

 private:
    Session& m_session;
    util::RequestSender<TaskEditorProxy> m_reply;
    afl::base::Ptr<TaskEditor> m_editor;
    afl::base::SignalConnection conn_change;
    afl::base::SignalConnection conn_objectChange;
    afl::base::SignalConnection conn_prefChange;
    Id_t m_id;
    Process::ProcessKind m_kind;
};

void
game::proxy::TaskEditorProxy::Trampoline::selectTask(Id_t id, Process::ProcessKind kind, bool create)
{
    // Remember the old editor
    // This means the old one will die no earlier than releaseAutoTaskEditor() below.
    // In particular, when this function is called with the same parameters again, it'll re-use the same instance.
    afl::base::Ptr<TaskEditor> old = m_editor;

    // Disconnect the signal. Anything that happens during the change will be ignored,
    // we explicitly send a status at the end.
    conn_change.disconnect();
    conn_objectChange.disconnect();
    conn_prefChange.disconnect();

    // Set up new one
    m_editor = m_session.getAutoTaskEditor(id, kind, create);
    m_id = id;
    m_kind = kind;

    // Destroy old one
    m_session.releaseAutoTaskEditor(old);

    // Connect the signal and inform user
    if (m_editor.get() != 0) {
        conn_change = m_editor->sig_change.add(this, &Trampoline::sendStatus);
        if (game::map::Object* obj = m_editor->process().getInvokingObject()) {
            conn_objectChange = obj->sig_change.add(this, &Trampoline::sendStatus);
        }

        if (Root* r = m_session.getRoot().get()) {
            conn_prefChange = r->userConfiguration().sig_change.add(this, &Trampoline::sendStatus);
        }
    }

    sendStatus();
}

void
game::proxy::TaskEditorProxy::Trampoline::setCursor(size_t newCursor)
{
    if (m_editor.get() != 0) {
        m_editor->setCursor(newCursor);
    }
}

void
game::proxy::TaskEditorProxy::Trampoline::addAsCurrent(String_t cmd)
{
    if (m_editor.get() != 0) {
        m_editor->addAsCurrent(TaskEditor::Commands_t::fromSingleObject(cmd));
    }
}

void
game::proxy::TaskEditorProxy::Trampoline::addAtEnd(String_t cmd)
{
    if (m_editor.get() != 0) {
        m_editor->addAtEnd(TaskEditor::Commands_t::fromSingleObject(cmd));
    }
}

void
game::proxy::TaskEditorProxy::Trampoline::describe(Status& out) const
{
    out.commands.clear();
    if (m_editor.get() != 0) {
        m_editor->getAll(out.commands);
        out.pc                 = m_editor->getPC();
        out.cursor             = m_editor->getCursor();
        out.isInSubroutineCall = m_editor->isInSubroutineCall();
        out.valid              = true;
    } else {
        out.pc                 = 0;
        out.cursor             = 0;
        out.isInSubroutineCall = false;
        out.valid              = false;
    }
}

void
game::proxy::TaskEditorProxy::Trampoline::describeShip(ShipStatus& out) const
{
    // ex WShipAutoTaskSelection::onTaskChange, sort-of
    out = ShipStatus();

    const Game* g = m_session.getGame().get();
    const Root* r = m_session.getRoot().get();
    const game::spec::ShipList* sl = m_session.getShipList().get();
    if (m_editor.get() != 0 && m_kind == Process::pkShipTask && g != 0 && r != 0 && sl != 0) {
        // Configuration
        bool isPredictToEnd = r->userConfiguration()[UserConfiguration::Task_PredictToEnd]();
        bool isShowDistances = r->userConfiguration()[UserConfiguration::Task_ShowDistances]();

        // Predict
        const game::map::Universe& univ = g->currentTurn().universe();
        ShipTaskPredictor pred(univ, m_id, g->shipScores(), *sl, g->mapConfiguration(), r->hostConfiguration(), r->hostVersion(), r->registrationKey());
        const game::map::Point startPosition = pred.getPosition();
        const int startFuel = pred.getRemainingFuel();
        if (isPredictToEnd || m_editor->getCursor() < m_editor->getPC()) {
            pred.predictTask(*m_editor);
        } else {
            pred.predictTask(*m_editor, m_editor->getCursor());
        }

        // Send status
        out.startPosition    = startPosition;
        game::map::Point pt = startPosition;
        for (size_t i = 0, n = pred.getNumPositions(); i < n; ++i) {
            game::map::Point npt = g->mapConfiguration().getSimpleNearestAlias(pred.getPosition(i), startPosition);
            out.positions.push_back(npt);
            if (isShowDistances) {
                out.distances2.push_back(pt.getSquaredRawDistance(npt));
            }
            pt = npt;
        }
        out.numFuelPositions = pred.getNumFuelPositions();
        out.currentTurn      = g->currentTurn().getTurnNumber();
        out.numTurns         = pred.getNumTurns();
        out.numFuelTurns     = pred.getNumFuelTurns();
        out.startingFuel     = startFuel;
        out.movementFuel     = pred.getMovementFuel();
        out.cloakFuel        = pred.getCloakFuel();;
        out.remainingFuel    = pred.getRemainingFuel();
        out.numberFormatter  = r->userConfiguration().getNumberFormatter();
        out.isHyperdriving   = pred.isHyperdriving();
        out.valid            = true;
    }
}

void
game::proxy::TaskEditorProxy::Trampoline::describeBase(BaseStatus& out) const
{
    // ex WBaseAutoTaskCommandTile::drawData (part)
    out = BaseStatus();

    Game*const g = m_session.getGame().get();
    Root*const r = m_session.getRoot().get();
    game::spec::ShipList*const sl = m_session.getShipList().get();
    if (m_editor.get() != 0 && m_kind == Process::pkBaseTask && g != 0 && r != 0 && sl != 0) {
        // Parse current command
        game::interface::BaseTaskBuildCommandParser parser(*sl);
        parser.predictStatement(*m_editor, m_editor->getCursor());

        if (parser.getOrder().getHullIndex() != 0) {
            // It's a valid build order, report it
            parser.getOrder().describe(out.buildOrder, *sl, m_session.translator());

            // Validate cost
            game::map::Universe& univ = g->currentTurn().universe();
            try {
                if (game::map::Planet* pl = univ.planets().get(m_id)) {
                    game::map::PlanetStorage storage(*pl, r->hostConfiguration());
                    game::actions::BuildShip a(*pl, storage, *sl, *r);
                    a.setUsePartsFromStorage(false);
                    a.setBuildOrder(parser.getOrder());

                    const game::actions::CargoCostAction& cca = a.costAction();
                    util::NumberFormatter fmt = r->userConfiguration().getNumberFormatter();
                    addMissingCargo(out.missingMinerals, "T",   cca.getMissingAmount(Element::Tritanium),  fmt);
                    addMissingCargo(out.missingMinerals, "D",   cca.getMissingAmount(Element::Duranium),   fmt);
                    addMissingCargo(out.missingMinerals, "M",   cca.getMissingAmount(Element::Molybdenum), fmt);
                    addMissingCargo(out.missingMinerals, "mc",  cca.getMissingAmount(Element::Money),      fmt);
                    addMissingCargo(out.missingMinerals, "sup", cca.getMissingAmount(Element::Supplies),   fmt);
                }
            }
            catch (std::exception& e) {
                // Ignore if planet is not played
            }
        }
    }
}

void
game::proxy::TaskEditorProxy::Trampoline::describeMessage(MessageStatus& out) const
{
    out = MessageStatus();
    if (m_editor.get() != 0) {
        const NotificationStore& notif = m_session.notifications();
        const NotificationStore::Message* msg = notif.findMessageByProcessId(m_editor->process().getProcessId());
        if (msg != 0 && !notif.isMessageConfirmed(msg)) {
            out.hasUnconfirmedMessage = true;
            out.text = notif.getMessageBody(msg);
        }
    }
}

void
game::proxy::TaskEditorProxy::Trampoline::sendStatus()
{
    // General information
    class Task : public util::Request<TaskEditorProxy> {
     public:
        Task(const Trampoline& self)
            : m_status()
            { self.describe(m_status); }
        virtual void handle(TaskEditorProxy& proxy)
            { proxy.sig_change.raise(m_status); }
     private:
        Status m_status;
    };
    m_reply.postNewRequest(new Task(*this));

    // Ship information
    class ShipTask : public util::Request<TaskEditorProxy> {
     public:
        ShipTask(const Trampoline& self)
            : m_status()
            { self.describeShip(m_status); }
        virtual void handle(TaskEditorProxy& proxy)
            { proxy.sig_shipChange.raise(m_status); }
     private:
        ShipStatus m_status;
    };
    m_reply.postNewRequest(new ShipTask(*this));

    // Starbase information
    class BaseTask : public util::Request<TaskEditorProxy> {
     public:
        BaseTask(const Trampoline& self)
            : m_status()
            { self.describeBase(m_status); }
        virtual void handle(TaskEditorProxy& proxy)
            { proxy.sig_baseChange.raise(m_status); }
     private:
        BaseStatus m_status;
    };
    m_reply.postNewRequest(new BaseTask(*this));

    // Message information
    class MessageTask : public util::Request<TaskEditorProxy> {
     public:
        MessageTask(const Trampoline& self)
            : m_status()
            { self.describeMessage(m_status); }
        virtual void handle(TaskEditorProxy& proxy)
            { proxy.sig_messageChange.raise(m_status); }
     private:
        MessageStatus m_status;
    };
    m_reply.postNewRequest(new MessageTask(*this));
}



class game::proxy::TaskEditorProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(util::RequestSender<TaskEditorProxy> reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply); }
 private:
    util::RequestSender<TaskEditorProxy> m_reply;
};


/*
 *  TaskEditorProxy
 */

game::proxy::TaskEditorProxy::TaskEditorProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply)
    : m_reply(reply, *this),
      m_trampoline(gameSender.makeTemporary(new TrampolineFromSession(m_reply.getSender())))
{ }

game::proxy::TaskEditorProxy::~TaskEditorProxy()
{ }

void
game::proxy::TaskEditorProxy::selectTask(Id_t id, interpreter::Process::ProcessKind kind, bool create)
{
    m_trampoline.postRequest(&Trampoline::selectTask, id, kind, create);
}

void
game::proxy::TaskEditorProxy::getStatus(WaitIndicator& ind, Status& out)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(Status& out)
            : m_out(out)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.describe(m_out); }
     private:
        Status& m_out;
    };
    Task t(out);
    ind.call(m_trampoline, t);
}

void
game::proxy::TaskEditorProxy::setCursor(size_t newCursor)
{
    m_trampoline.postRequest(&Trampoline::setCursor, newCursor);
}

void
game::proxy::TaskEditorProxy::addAsCurrent(const String_t& cmd)
{
    m_trampoline.postRequest(&Trampoline::addAsCurrent, cmd);
}

void
game::proxy::TaskEditorProxy::addAtEnd(const String_t& cmd)
{
    m_trampoline.postRequest(&Trampoline::addAtEnd, cmd);
}
