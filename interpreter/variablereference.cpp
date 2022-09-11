/**
  *  \file interpreter/variablereference.cpp
  *  \brief Class interpreter::VariableReference
  */

#include "interpreter/variablereference.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/processlist.hpp"

namespace {
    interpreter::Process::Frame& makeTemporaryFrame(interpreter::Process& proc)
    {
        return proc.pushFrame(interpreter::BytecodeObject::create(true), false);
    }
}

// Constructor.
interpreter::VariableReference::Maker::Maker(Process& proc)
    : m_process(proc),
      m_frame(makeTemporaryFrame(proc))
{ }

// Create a variable reference.
interpreter::VariableReference
interpreter::VariableReference::Maker::make(const String_t& name, const afl::data::Value* value)
{
    // If a name is re-used, it is undefined what happens to the previous value;
    // however, the most-recent value must be retrievable.
    // For now, we store references by-name, so this will be a proper overwrite changing the previous reference's value.
    m_frame.localValues.set(m_frame.localNames.addMaybe(name), value);
    return fromProcess(m_process, name);
}

// Create null VariableReference.
interpreter::VariableReference::VariableReference()
    : m_name(),
      m_processId(0)       // 0 is a process Id guaranteed to not exist
{ }

// Create VariableReference from a process/name.
interpreter::VariableReference
interpreter::VariableReference::fromProcess(Process& proc, const String_t& name)
{
    return VariableReference(proc.getProcessId(), name);
}

// Resolve a VariableReference.
afl::data::Value*
interpreter::VariableReference::get(const ProcessList& list) const
{
    if (Process* p = list.getProcessById(m_processId)) {
        return p->getVariable(m_name);
    } else {
        return 0;
    }
}

inline
interpreter::VariableReference::VariableReference(uint32_t processId, const String_t& name)
    : m_name(name), m_processId(processId)
{ }
