/**
  *  \file client/si/control.cpp
  *  \brief Class client::si::Control
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


client::si::Control::Control(UserSide& us)
    : m_interface(us),
      m_id(m_interface.allocateWaitId()),
      m_loop(us.root()),
      m_root(us.root()),
      m_translator(us.translator())
{
    m_interface.mainLog().write(LogListener::Trace, LOG_NAME, Format("<c%d> create", m_id));
    m_interface.addControl(*this);
}

client::si::Control::~Control()
{
    m_interface.mainLog().write(LogListener::Trace, LOG_NAME, Format("<c%d> remove", m_id));
    m_interface.removeControl(*this);
}

// Execute a script command.
void
client::si::Control::executeCommandWait(String_t command, bool verbose, String_t name)
{
    // replaces int/simple.h:executeStatement (using command "C2$Eval atom, prefix" or similar)
    std::auto_ptr<game::interface::ContextProvider> ctxp(createContextProvider());
    std::auto_ptr<ScriptTask> t(new CommandTask(command, verbose, name, ctxp));
    executeTaskInternal(t, Format("executeCommandWait('%s')", name));
}

void
client::si::Control::executeHookWait(String_t name)
{
    // replaces int/simple.h:runHook
    executeCommandWait(Format("RunHook %s", name), false, Format("(%s hook)", name));
}

// Execute a key command.
void
client::si::Control::executeKeyCommandWait(String_t keymapName, util::Key_t key, int prefix)
{
    class Task : public ScriptTask {
     public:
        Task(String_t keymapName, util::Key_t key, int prefix, std::auto_ptr<game::interface::ContextProvider> ctxp)
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
        std::auto_ptr<game::interface::ContextProvider> m_contextProvider;
    };
    std::auto_ptr<game::interface::ContextProvider> ctxp(createContextProvider());
    std::auto_ptr<ScriptTask> t(new Task(keymapName, key, prefix, ctxp));
    executeTaskInternal(t, Format("executeKeyCommandWait('%s')", util::formatKey(key)));
}

// Execute a "UI.GotoReference" command with the given game::Reference.
void
client::si::Control::executeGoToReferenceWait(String_t taskName, game::Reference ref)
{
    class ReferenceTask : public client::si::ScriptTask {
     public:
        ReferenceTask(String_t taskName, game::Reference ref)
            : m_taskName(taskName), m_ref(ref)
            { }
        virtual void execute(uint32_t pgid, game::Session& session)
            {
                // Create BCO
                interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
                game::interface::ReferenceContext ctx(m_ref, session);
                bco->setSubroutineName(m_taskName);
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
        executeTaskInternal(t, "executeGoToReferenceWait()");
    }
}

// Execute a script task.
void
client::si::Control::executeTaskWait(std::auto_ptr<ScriptTask> task)
{
    executeTaskInternal(task, "executeTaskWait()");
}

// Continue a detached process.
void
client::si::Control::continueProcessWait(RequestLink2 link)
{
    if (link.isValid()) {
        bool prev = m_interface.setWaiting(true);
        m_interface.mainLog().write(LogListener::Trace, LOG_NAME, Format("<c%d> continueProcessWait", m_id));
        m_interface.continueProcessWait(m_id, link);
        m_loop.run();
        m_interface.setWaiting(prev);
    }
}

// Handle successful wait.
void
client::si::Control::onTaskComplete(uint32_t waitId)
{
    if (waitId == m_id) {
        m_interface.mainLog().write(LogListener::Trace, LOG_NAME, Format("<c%d> onTaskComplete", m_id));
        m_loop.stop(0);
    }
}

/*
 *  User-Interface Callouts
 */

// Default implementation of handlePopupConsole().
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

// Default implementation of handleScanKeyboardMode().
void
client::si::Control::defaultHandleScanKeyboardMode(RequestLink2 link)
{
    // Default behaviour for UI.ScanKeyboardMode is to reject it
    interface().continueProcessWithFailure(link, "Context error");
}

// Default implementation of handleSetView().
void
client::si::Control::defaultHandleSetView(RequestLink2 link, String_t /*name*/, bool /*withKeymap*/)
{
    // Default behaviour for Chart.SetView is to reject it
    interface().continueProcessWithFailure(link, "Context error");
}

// Default implementation of handleUseKeymap().
void
client::si::Control::defaultHandleUseKeymap(RequestLink2 link, String_t name, int prefix)
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

// Default implementation of handleOverlayMessage().
void
client::si::Control::defaultHandleOverlayMessage(RequestLink2 link, String_t text)
{
    client::widgets::showDecayingMessage(root(), text);
    interface().continueProcess(link);
}

// Implementation of handleStateChange() for dialogs.
void
client::si::Control::dialogHandleStateChange(RequestLink2 link, OutputState::Target target, OutputState& out, ui::EventLoop& loop, int n)
{
    m_interface.mainLog().write(LogListener::Trace, LOG_NAME, Format("<c%d> dialogHandleStateChange %s", m_id, OutputState::toString(out.getTarget())));
    if (target == OutputState::NoChange) {
        m_interface.continueProcess(link);
    } else {
        m_interface.detachProcess(link);
        out.set(link, target);
        loop.stop(n);
    }
}

// Implementation of handleEndDialog() for dialogs.
void
client::si::Control::dialogHandleEndDialog(RequestLink2 link, int /*code*/, OutputState& out, ui::EventLoop& loop, int n)
{
    m_interface.detachProcess(link);
    out.set(link, OutputState::NoChange);
    loop.stop(n);
}

void
client::si::Control::executeTaskInternal(std::auto_ptr<ScriptTask> task, String_t name)
{
    bool prev = m_interface.setWaiting(true);
    m_interface.mainLog().write(LogListener::Trace, LOG_NAME, Format("<c%d> %s", m_id, name));
    m_interface.executeTaskWait(m_id, task);
    m_loop.run();
    m_interface.setWaiting(prev);
}
