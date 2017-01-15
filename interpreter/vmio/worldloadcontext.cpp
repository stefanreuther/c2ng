/**
  *  \file interpreter/vmio/worldloadcontext.cpp
  *  \brief Class interpreter::vmio::WorldLoadContext
  */

#include "interpreter/vmio/worldloadcontext.hpp"
#include "interpreter/mutexcontext.hpp"

interpreter::vmio::WorldLoadContext::WorldLoadContext(LoadContext& parent, World& world)
    : m_parent(parent),
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
interpreter::vmio::WorldLoadContext::loadMutex(const String_t& name, const String_t& note, Process* owner)
{
    if (MutexList::Mutex* mtx = m_world.mutexList().load(name, note, owner)) {
        return new MutexContext(mtx);
    } else {
        return 0;
    }
}

interpreter::Process*
interpreter::vmio::WorldLoadContext::createProcess()
{
    // FIXME: if the processes have priorities, we must somehow arrange to call handlePriorityChange() on them
    return &m_world.processList().create(m_world, "<Loaded Process>");
}
