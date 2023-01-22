/**
  *  \file game/interface/processlisteditor.cpp
  *  \brief Class game::interface::ProcessListEditor
  */

#include "game/interface/processlisteditor.hpp"
#include "interpreter/process.hpp"
#include "game/map/ship.hpp"
#include "game/map/planet.hpp"
#include "game/interface/notificationstore.hpp"

using interpreter::ProcessList;
using interpreter::Process;

// Constructor.
game::interface::ProcessListEditor::ProcessListEditor(interpreter::ProcessList& list)
    : m_list(list),
      m_changes()
{ }

// Get number of processes.
size_t
game::interface::ProcessListEditor::getNumProcesses() const
{
    return m_list.getProcessList().size();
}

// Describe a process.
bool
game::interface::ProcessListEditor::describe(size_t slotNr, Info& info, const NotificationStore& notif, afl::string::Translator& tx) const
{
    const ProcessList::Vector_t& vec = m_list.getProcessList();
    if (slotNr < vec.size()) {
        Process& p = *vec[slotNr];

        info.processId = p.getProcessId();
        info.priority = p.getPriority();
        info.name = p.getName();

        std::map<uint32_t, State>::const_iterator it = m_changes.find(info.processId);
        if (it == m_changes.end() || it->second == Suspended) {
            info.status = toString(p.getState(), tx);
            info.isChanged = false;
        } else {
            info.status = (it->second == Runnable ? toString(Process::Runnable, tx) : toString(Process::Terminated, tx));
            info.isChanged = true;
        }

        const afl::base::Deletable* obj = p.getInvokingObject();
        if (const game::map::Ship* sh = dynamic_cast<const game::map::Ship*>(obj)) {
            info.invokingObject = Reference(Reference::Ship, sh->getId());
        } else if (const game::map::Planet* pl = dynamic_cast<const game::map::Planet*>(obj)) {
            info.invokingObject = Reference(p.getProcessKind() ==  Process::pkBaseTask ? Reference::Starbase : Reference::Planet, pl->getId());
        } else {
            info.invokingObject = Reference();
        }

        if (NotificationStore::Message* msg = notif.findMessageByProcessId(info.processId)) {
            if (notif.isMessageConfirmed(msg)) {
                info.notificationStatus = ConfirmedMessage;
            } else {
                info.notificationStatus = UnreadMessage;
            }
        } else {
            info.notificationStatus = NoMessage;
        }
        return true;
    } else {
        return false;
    }
}

// Prepare a state change.
void
game::interface::ProcessListEditor::setProcessState(uint32_t pid, State state)
{
    // client/dialogs/processmgr.cc:changeProcessState (sort-of)
    // WProcessManagerDialog::setProcessState (sort-of)
    Process* p = m_list.findProcessById(pid);
    if (p != 0 && p->getState() == Process::Suspended) {
        m_changes[pid] = state;
    } else {
        m_changes.erase(pid);
    }
}

// Prepare a state change for all processes.
void
game::interface::ProcessListEditor::setAllProcessState(State state)
{
    // WProcessManagerDialog::setProcessState (sort-of)
    m_changes.clear();
    if (state != Suspended) {
        const ProcessList::Vector_t& vec = m_list.getProcessList();
        for (size_t i = 0, n = vec.size(); i < n; ++i) {
            Process& p = *vec[i];
            if (p.getState() == Process::Suspended) {
                m_changes[p.getProcessId()] = state;
            }
        }
    }
}

// Set process priority.
void
game::interface::ProcessListEditor::setProcessPriority(uint32_t pid, int pri)
{
    if (Process* p = m_list.findProcessById(pid)) {
        p->setPriority(pri);
        m_list.handlePriorityChange(*p);
    }
}

// Perform all prepared state changes.
void
game::interface::ProcessListEditor::commit(uint32_t pgid)
{
    const ProcessList::Vector_t& vec = m_list.getProcessList();
    for (size_t i = 0, n = vec.size(); i < n; ++i) {
        Process& p = *vec[i];
        if (p.getState() == Process::Suspended) {
            std::map<uint32_t, State>::const_iterator it = m_changes.find(p.getProcessId());
            if (it != m_changes.end()) {
                switch (it->second) {
                 case Suspended:
                    break;
                 case Runnable:
                    m_list.resumeProcess(p, pgid);
                    break;
                 case Terminated:
                    m_list.terminateProcess(p);
                    break;
                }
            }
        }
    }
    m_changes.clear();
}
