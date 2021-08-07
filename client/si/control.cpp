/**
  *  \file client/si/control.cpp
  */

#include "client/si/control.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/consoledialog.hpp"
#include "client/si/commandtask.hpp"
#include "client/si/keymaphandler.hpp"
#include "client/si/userside.hpp"
#include "client/widgets/decayingmessage.hpp"
#include "game/interface/referencecontext.hpp"
#include "interpreter/values.hpp"

using afl::sys::LogListener;
using afl::string::Format;

namespace {
    const char*const LOG_NAME = "client.si";
}


client::si::Control::Control(UserSide& iface, ui::Root& root, afl::string::Translator& tx)
    : m_waiting(false),
      m_interacting(false),
      m_active(false),
      m_interface(iface),
      m_id(0),
      m_blocker(root, tx.translateString("Script working...")),
      m_loop(root),
      m_root(root),
      m_translator(tx)
{
    m_interface.mainLog().write(LogListener::Trace, LOG_NAME, Format("<%p> create", this));
    m_blocker.setExtent(gfx::Rectangle(gfx::Point(), m_blocker.getLayoutInfo().getPreferredSize()));
    m_root.moveWidgetToEdge(m_blocker, gfx::CenterAlign, gfx::BottomAlign, 10);
    m_interface.addControl(*this);
}

client::si::Control::~Control()
{
    m_interface.mainLog().write(LogListener::Trace, LOG_NAME, Format("<%p> remove", this));
    m_interface.removeControl(*this);
}

void
client::si::Control::executeCommandWait(String_t command, bool verbose, String_t name)
{
    // replaces int/simple.h:runHook (using command "RunHook ...")
    // replaces int/simple.h:executeStatement (using command "C2$Eval atom, prefix" or similar)
    std::auto_ptr<ContextProvider> ctxp(m_interface.createContextProvider());
    std::auto_ptr<ScriptTask> t(new CommandTask(command, verbose, name, ctxp));
    executeTaskInternal(t, Format("executeCommandWait('%s')", name));
}

void
client::si::Control::executeKeyCommandWait(String_t keymapName, util::Key_t key, int prefix)
{
    class Task : public ScriptTask {
     public:
        Task(String_t keymapName, util::Key_t key, int prefix, std::auto_ptr<ContextProvider> ctxp)
            : m_keymapName(keymapName), m_key(key), m_prefix(prefix), m_contextProvider(ctxp)
            { }
        virtual void execute(uint32_t pgid, game::Session& session)
            {
                util::KeymapRef_t k = session.world().keymaps().getKeymapByName(m_keymapName);
                util::Atom_t a = (k != 0 ? k->lookupCommand(m_key) : 0);
                if (a != 0) {
                    CommandTask(afl::string::Format("C2$Eval %d, %d, %s", a, m_prefix, interpreter::quoteString(util::formatKey(m_key))),
                                false,
                                afl::string::Format(session.translator()("Key '%s' in '%s'").c_str(), util::formatKey(m_key), m_keymapName),
                                m_contextProvider).execute(pgid, session);
                }
            }
     private:
        uint32_t m_id;
        String_t m_keymapName;
        util::Key_t m_key;
        int m_prefix;
        std::auto_ptr<ContextProvider> m_contextProvider;
    };
    std::auto_ptr<ContextProvider> ctxp(m_interface.createContextProvider());
    std::auto_ptr<ScriptTask> t(new Task(keymapName, key, prefix, ctxp));
    executeTaskInternal(t, Format("executeKeyCommandWait('%s')", util::formatKey(key)));
}

void
client::si::Control::executeGoToReference(String_t taskName, game::Reference ref)
{
    class ReferenceTask : public client::si::ScriptTask {
     public:
        ReferenceTask(String_t taskName, game::Reference ref)
            : m_taskName(taskName), m_ref(ref)
            { }
        virtual void execute(uint32_t pgid, game::Session& session)
            {
                // Create BCO
                interpreter::BCORef_t bco = *new interpreter::BytecodeObject();
                game::interface::ReferenceContext ctx(m_ref, session);
                bco->setSubroutineName(m_taskName);
                bco->setIsProcedure(true);
                bco->addPushLiteral(&ctx);
                bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sNamedShared, bco->addName("UI.GOTOREFERENCE"));
                bco->addInstruction(interpreter::Opcode::maIndirect, interpreter::Opcode::miIMCall, 1);

                // Create process
                interpreter::ProcessList& processList = session.processList();
                interpreter::Process& proc = processList.create(session.world(), m_taskName);
                proc.pushFrame(bco, false);
                processList.resumeProcess(proc, pgid);
            }
     private:
        String_t m_taskName;
        game::Reference m_ref;
    };

    if (ref.isSet()) {
        std::auto_ptr<client::si::ScriptTask> t(new ReferenceTask(taskName, ref));
        executeTaskWait(t);
    }
}


void
client::si::Control::executeTaskWait(std::auto_ptr<ScriptTask> task)
{
    executeTaskInternal(task, "executeTask()");
}

void
client::si::Control::continueProcessWait(RequestLink2 link)
{
    if (link.isValid()) {
        m_interacting = false;
        if (m_waiting) {
            // If I am already waiting, re-use the wait Id, and do NOT re-enter m_loop.run().
            // This happens when someone does continueProcessWait from a Control virtual,
            // e.g. KeymapHandler::handleUseKeymapRequest when a process does two UseKeymap in a row.
            // Failure to do this will cause KeymapHandler::run() to hang.
            // This means continueProcessWait will not actually wait for recursive waits.
            // (quick fix, not 100% sure about it)
            m_interface.mainLog().write(LogListener::Trace, LOG_NAME, Format("<%p> continueProcessWait => %d (nested)", this, m_id));
            m_interface.continueProcessWait(m_id, link);
            updateBlocker();
        } else {
            m_waiting = true;
            m_id = m_interface.allocateWaitId();
            m_interface.mainLog().write(LogListener::Trace, LOG_NAME, Format("<%p> continueProcessWait => %d", this, m_id));
            m_interface.continueProcessWait(m_id, link);
            updateBlocker();
            m_loop.run();
        }
    }
}

void
client::si::Control::handleWait(uint32_t id)
{
    if (id == m_id) {
        m_interface.mainLog().write(LogListener::Trace, LOG_NAME, Format("<%p> handleWait <= %d", this, m_id));
        m_waiting = false;
        updateBlocker();
        m_loop.stop(0);
    }
}

void
client::si::Control::setInteracting(bool state)
{
    m_interface.mainLog().write(LogListener::Trace, LOG_NAME, Format("<%p> setInteracting(%d)", this, int(state)));
    m_interacting = state;
    updateBlocker();
}

void
client::si::Control::defaultHandlePopupConsole(RequestLink2 link)
{
    m_interface.detachProcess(link);

    InputState input;
    input.setProcess(link);

    OutputState output;
    client::dialogs::doConsoleDialog(m_interface, *this, input, output);
    handleStateChange(output.getProcess(), output.getTarget());
}

void
client::si::Control::defaultHandleScanKeyboardMode(RequestLink2 link)
{
    // Default behaviour for UI.ScanKeyboardMode is to reject it
    interface().continueProcessWithFailure(link, "Context error");
}

void
client::si::Control::defaultHandleSetViewRequest(RequestLink2 link, String_t /*name*/, bool /*withKeymap*/)
{
    // Default behaviour for Chart.SetView is to reject it
    interface().continueProcessWithFailure(link, "Context error");
}

void
client::si::Control::defaultHandleUseKeymapRequest(RequestLink2 link, String_t name, int prefix)
{
    KeymapHandler::Result r = KeymapHandler(*this, name, prefix).run(link);
    switch (r.action) {
     case KeymapHandler::NoAction:
        break;
     case KeymapHandler::KeyCommand:
        executeKeyCommandWait(r.keymapName, r.key, prefix);
        break;
     case KeymapHandler::StateChange:
        handleStateChange(r.link, r.target);
        break;
     case KeymapHandler::EndDialog:
        handleEndDialog(r.link, r.code);
        break;
     case KeymapHandler::PopupConsole:
        handlePopupConsole(r.link);
        break;
     case KeymapHandler::ScanKeyboardMode:
        handleScanKeyboardMode(r.link);
        break;
    }
}

void
client::si::Control::defaultHandleOverlayMessageRequest(RequestLink2 link, String_t text)
{
    client::widgets::showDecayingMessage(root(), text);
    interface().continueProcess(link);
}

void
client::si::Control::dialogHandleStateChange(RequestLink2 link, OutputState::Target target, OutputState& out, ui::EventLoop& loop, int n)
{
    if (target == OutputState::NoChange) {
        m_interface.continueProcess(link);
    } else {
        m_interface.detachProcess(link);
        out.set(link, target);
        loop.stop(n);
    }
}

void
client::si::Control::dialogHandleEndDialog(RequestLink2 link, int /*code*/, OutputState& out, ui::EventLoop& loop, int n)
{
    m_interface.detachProcess(link);
    out.set(link, OutputState::NoChange);
    loop.stop(n);
}

void
client::si::Control::updateBlocker()
{
    bool newState = m_waiting && !m_interacting;
    if (newState != m_active) {
        m_active = newState;
        if (newState) {
            m_root.add(m_blocker);
        } else {
            m_root.remove(m_blocker);
            m_blocker.replayEvents();
        }
    }
}

void
client::si::Control::executeTaskInternal(std::auto_ptr<ScriptTask> task, String_t name)
{
    m_waiting = true;
    m_interacting = false;
    m_id = m_interface.allocateWaitId();
    m_interface.mainLog().write(LogListener::Trace, LOG_NAME, Format("<%p> %s => %d", this, name, m_id));
    m_interface.executeTaskWait(m_id, task);
    updateBlocker();
    m_loop.run();
}
