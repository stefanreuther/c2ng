/**
  *  \file interpreter/mutexcontext.cpp
  *  \brief Class interpreter::MutexContext
  */

#include <cassert>
#include "interpreter/mutexcontext.hpp"
#include "afl/bits/uint32le.hpp"
#include "afl/bits/value.hpp"
#include "afl/string/format.hpp"
#include "interpreter/process.hpp"
#include "interpreter/savecontext.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"

namespace {
    uint32_t trimSize(size_t sz)
    {
        // This is used for string sizes, so we need not go all the way up to 4G.
        uint32_t result = 0x100000;
        if (sz < result) {
            result = static_cast<uint32_t>(sz);
        }
        return result;
    }
}

interpreter::MutexContext::MutexContext(const String_t& name, const String_t& note)
    : m_mutex(0),
      m_name(name),
      m_note(note)
{
    // ex IntMutexContext::IntMutexContext
}

/* Destructor. Removes the mutex when this was the last reference. */
interpreter::MutexContext::~MutexContext()
{
    // ex IntMutexContext::~IntMutexContext
    assert(m_mutex == 0);
}

// Context:

/* Lookup implementation. Mutex has no properties. */
interpreter::Context::PropertyAccessor*
interpreter::MutexContext::lookup(const afl::data::NameQuery& /*name*/, PropertyIndex_t& /*result*/)
{
    // ex IntMutexContext::lookup
    return 0;
}

/* Next implementation. Mutex is not iterable. */
bool
interpreter::MutexContext::next()
{
    // ex IntMutexContext::next
    return false;
}

/* Clone implementation. */
interpreter::MutexContext*
interpreter::MutexContext::clone() const
{
    // ex IntMutexContext::clone
    return new MutexContext(m_name, m_note);
}

/* getObject implementation. Mutex has no object. */
game::map::Object*
interpreter::MutexContext::getObject()
{
    // ex IntMutexContext::getObject
    return 0;
}

/* EnumProperties implementation. Mutex has no properties. */
void
interpreter::MutexContext::enumProperties(PropertyAcceptor& /*acceptor*/) const
{
    // ex IntMutexContext::enumProperties
}

void
interpreter::MutexContext::onContextEntered(Process& proc)
{
    assert(m_mutex == 0);
    m_mutex = proc.world().mutexList().create(m_name, m_note, &proc);
}

void
interpreter::MutexContext::onContextLeft()
{
    assert(m_mutex != 0);
    if (m_mutex != 0) {
        m_mutex->removeReference();
        m_mutex = 0;
    }
}

// BaseValue:

/* toString implementation. */
String_t
interpreter::MutexContext::toString(bool readable) const
{
    // ex IntMutexContext::toString
    if (readable) {
        return afl::string::Format(m_note.empty() ? "Lock(%s)" : "Lock(%s,%s)", quoteString(m_name), quoteString(m_note));
    } else {
        return "#<lock>";
    }
}

/* Store implementation. */
void
interpreter::MutexContext::store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const
{
    // Storage format is:
    //   tag is flag
    //     0=not this process, 1=this process
    //   aux is
    //     2 words for string lengths
    //     2 strings (name, info)
    //
    // Before 20220801 (and in PCC2 classic), we associated a MutexContext object with an owner at the time of creation/load.
    // This means we need to store an owner flag for compatibility with those although we do not need it ourselves.
    //
    // Storing just an owner flag avoids the need to name processes.
    // If one process contains a variable containing a lock owned by another one,
    // the other one will claim the lock if he still rightfully owns it;
    // if he doesn't, it's probably better to disown the lock.
    //
    // After 20220801, lock ownership is determined by the MutexContext object being on a process' context stack,
    // determined by onContextEntered/onContextLeft.
    // A value from the context stack cannot be transferred elsewhere.
    out.tag   = TagNode::Tag_Mutex;
    out.value = m_mutex != 0 && ctx.isCurrentProcess(m_mutex->getOwner());

    uint32_t nameSize = trimSize(m_name.size());
    uint32_t noteSize = trimSize(m_note.size());

    afl::bits::Value<afl::bits::UInt32LE> header[2];
    header[0] = nameSize;
    header[1] = noteSize;
    aux.handleFullData(afl::base::fromObject(header));
    aux.handleFullData(afl::string::toBytes(m_name).trim(static_cast<size_t>(nameSize)));
    aux.handleFullData(afl::string::toBytes(m_note).trim(static_cast<size_t>(noteSize)));
}
