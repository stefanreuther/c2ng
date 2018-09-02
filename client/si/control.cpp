/**
  *  \file client/si/control.cpp
  */

#include "client/si/control.hpp"
#include "client/si/userside.hpp"
#include "client/dialogs/consoledialog.hpp"
#include "afl/string/format.hpp"

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
    m_root.moveWidgetToEdge(m_blocker, 1, 2, 10);
    m_interface.addControl(*this);
}

client::si::Control::~Control()
{
    m_interface.mainLog().write(LogListener::Trace, LOG_NAME, Format("<%p> remove", this));
    m_interface.removeControl(*this);
}

void
client::si::Control::attachPreparedWait(uint32_t waitId)
{
    m_interface.mainLog().write(LogListener::Trace, LOG_NAME, Format("<%p> attachPreparedWait => %d", this, waitId));
    m_waiting = true;
    m_interacting = false;
    m_id = waitId;
    updateBlocker();
    m_loop.run();
}

void
client::si::Control::executeCommandWait(String_t command, bool verbose, String_t name)
{
    // replaces int/simple.h:runHook (using command "RunHook ...")
    // replaces int/simple.h:executeStatement (using command "C2$Eval atom, prefix" or similar)
    std::auto_ptr<ContextProvider> ctxp(createContextProvider());
    m_waiting = true;
    m_interacting = false;
    m_id = m_interface.allocateWaitId();
    m_interface.mainLog().write(LogListener::Trace, LOG_NAME, Format("<%p> executeCommandWait('%s') => %d", this, name, m_id));
    m_interface.executeCommandWait(m_id, command, verbose, name, ctxp);
    updateBlocker();
    m_loop.run();
}

void
client::si::Control::executeKeyCommandWait(String_t keymapName, util::Key_t key, int prefix)
{
    std::auto_ptr<ContextProvider> ctxp(createContextProvider());
    m_waiting = true;
    m_interacting = false;
    m_id = m_interface.allocateWaitId();
    m_interface.mainLog().write(LogListener::Trace, LOG_NAME, Format("<%p> executeKeyCommandWait('%s') => %d", this, util::formatKey(key), m_id));
    m_interface.executeKeyCommandWait(m_id, keymapName, key, prefix, ctxp);
    updateBlocker();
    m_loop.run();
}

void
client::si::Control::continueProcessWait(RequestLink2 link)
{
    if (link.isValid()) {
        m_waiting = true;
        m_interacting = false;
        m_id = m_interface.allocateWaitId();
        m_interface.mainLog().write(LogListener::Trace, LOG_NAME, Format("<%p> continueProcessWait => %d", this, m_id));
        m_interface.continueProcessWait(m_id, link);
        updateBlocker();
        m_loop.run();
    }
}

void
client::si::Control::handleWait(uint32_t id, interpreter::Process::State state, interpreter::Error error)
{
    // FIXME
    (void) state;
    (void) error;
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
client::si::Control::defaultHandlePopupConsole(UserSide& ui, RequestLink2 link)
{
    ui.detachProcess(link);

    InputState input;
    input.setProcess(link);

    OutputState output;
    client::dialogs::doConsoleDialog(ui, *this, input, output);
    handleStateChange(ui, output.getProcess(), output.getTarget());
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
