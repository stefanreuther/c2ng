/**
  *  \file interpreter/vmio/processloadcontext.cpp
  *  \brief Class interpreter::vmio::ProcessLoadContext
  */

#include "interpreter/vmio/processloadcontext.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/bits/uint32le.hpp"
#include "afl/bits/value.hpp"
#include "interpreter/mutexcontext.hpp"

namespace {
    /** Load undelimited, unencoded string. */
    String_t loadString(afl::io::Stream& s, size_t length)
    {
        afl::base::GrowableBytes_t buffer;
        buffer.resize(length);
        s.fullRead(buffer);
        return afl::string::fromBytes(buffer);
    }

    /** Load mutex. */
    interpreter::Context* loadMutex(afl::io::Stream& aux)
    {
        // ex IntMutexContext::load
        afl::bits::Value<afl::bits::UInt32LE> header[2];
        aux.fullRead(afl::base::fromObject(header));

        String_t name = loadString(aux, header[0]);
        String_t note = loadString(aux, header[1]);
        return new interpreter::MutexContext(name, note);
    }
}


interpreter::vmio::ProcessLoadContext::ProcessLoadContext(LoadContext& parent, Process& proc)
    : m_parent(parent),
      m_process(proc)
{ }

afl::data::Value*
interpreter::vmio::ProcessLoadContext::loadBCO(uint32_t id)
{
    return m_parent.loadBCO(id);
}

afl::data::Value*
interpreter::vmio::ProcessLoadContext::loadArray(uint32_t id)
{
    return m_parent.loadArray(id);
}

afl::data::Value*
interpreter::vmio::ProcessLoadContext::loadHash(uint32_t id)
{
    return m_parent.loadHash(id);
}

afl::data::Value*
interpreter::vmio::ProcessLoadContext::loadStructureValue(uint32_t id)
{
    return m_parent.loadStructureValue(id);
}

afl::data::Value*
interpreter::vmio::ProcessLoadContext::loadStructureType(uint32_t id)
{
    return m_parent.loadStructureType(id);
}

interpreter::Context*
interpreter::vmio::ProcessLoadContext::loadContext(const TagNode& tag, afl::io::Stream& aux)
{
    switch (tag.tag) {
     case TagNode::Tag_Mutex:
        return ::loadMutex(aux);

     case TagNode::Tag_Frame:
        // Frame. Depends on current process.
        return m_process.makeFrameContext(tag.value);

     default:
        return m_parent.loadContext(tag, aux);
    }
}

interpreter::Process*
interpreter::vmio::ProcessLoadContext::createProcess()
{
    // This function should not be called; nested processes are not allowed.
    // Otherwise, we should just be able to call our parent's version.
    return 0;
}

void
interpreter::vmio::ProcessLoadContext::finishProcess(Process& /*proc*/)
{ }
