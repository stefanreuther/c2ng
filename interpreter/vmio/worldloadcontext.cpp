/**
  *  \file interpreter/vmio/worldloadcontext.cpp
  *  \brief Class interpreter::vmio::WorldLoadContext
  */

#include "interpreter/vmio/worldloadcontext.hpp"
#include "interpreter/mutexcontext.hpp"

interpreter::vmio::WorldLoadContext::WorldLoadContext(LoadContext& parent, ProcessList& processList, World& world)
    : m_parent(parent),
      m_processList(processList),
      m_world(world)
{ }

afl::data::Value*
interpreter::vmio::WorldLoadContext::loadBCO(uint32_t id)
{
    return m_parent.loadBCO(id);
}

afl::data::Value*
interpreter::vmio::WorldLoadContext::loadArray(uint32_t id)
{
    return m_parent.loadArray(id);
}

afl::data::Value*
interpreter::vmio::WorldLoadContext::loadHash(uint32_t id)
{
    return m_parent.loadHash(id);
}

afl::data::Value*
interpreter::vmio::WorldLoadContext::loadStructureValue(uint32_t id)
{
    return m_parent.loadStructureValue(id);
}

afl::data::Value*
interpreter::vmio::WorldLoadContext::loadStructureType(uint32_t id)
{
    return m_parent.loadStructureType(id);
}

interpreter::Context*
interpreter::vmio::WorldLoadContext::loadContext(const TagNode& tag, afl::io::Stream& aux)
{
    return m_parent.loadContext(tag, aux);
}

interpreter::Context*
interpreter::vmio::WorldLoadContext::loadMutex(const String_t& name, const String_t& note)
{
    return new MutexContext(name, note);
}

interpreter::Process*
interpreter::vmio::WorldLoadContext::createProcess()
{
    return &m_processList.create(m_world, "<Loaded Process>");
}

void
interpreter::vmio::WorldLoadContext::finishProcess(Process& proc)
{
    m_processList.handlePriorityChange(proc);
}
