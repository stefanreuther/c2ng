/**
  *  \file interpreter/mutexcontext.cpp
  */

#include "interpreter/mutexcontext.hpp"
#include "afl/bits/value.hpp"
#include "afl/bits/uint32le.hpp"
#include "interpreter/savecontext.hpp"

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

// /** Constructor.
//     \param mtx Mutex. Must have one reference allocated to this object. */
interpreter::MutexContext::MutexContext(MutexList::Mutex* mtx)
    : m_mutex(mtx)
{
    // ex IntMutexContext::IntMutexContext
}

// /** Destructor. Removes the mutex when this was the last reference. */
interpreter::MutexContext::~MutexContext()
{
    // ex IntMutexContext::~IntMutexContext
    m_mutex->removeReference();
}

// Context:

// /** Lookup implementation. Mutex has no properties. */
interpreter::Context*
interpreter::MutexContext::lookup(const afl::data::NameQuery& /*name*/, PropertyIndex_t& /*result*/)
{
    // ex IntMutexContext::lookup
    return 0;
}

// /** Set implementation. Mutex has no properties. */
void
interpreter::MutexContext::set(PropertyIndex_t /*index*/, const afl::data::Value* /*value*/)
{
    // ex IntMutexContext::set
}

// /** Get implementation. Mutex has no properties. */
afl::data::Value*
interpreter::MutexContext::get(PropertyIndex_t /*index*/)
{
    // ex IntMutexContext::get
    return 0;
}

/** Next implementation. Mutex is not iterable. */
bool
interpreter::MutexContext::next()
{
    // ex IntMutexContext::next
    return false;
}

// /** Clone implementation. */
interpreter::MutexContext*
interpreter::MutexContext::clone() const
{
    // ex IntMutexContext::clone
    return new MutexContext(&m_mutex->addReference());
}

// /** GetObject implementation. Mutex has no object. */
game::map::Object*
interpreter::MutexContext::getObject()
{
    // ex IntMutexContext::getObject
    return 0;
}

// /** EnumProperties implementation. Mutex has no properties. */
void
interpreter::MutexContext::enumProperties(PropertyAcceptor& /*acceptor*/)
{
    // ex IntMutexContext::enumProperties
}

// BaseValue:

// /** toString implementation. */
String_t
interpreter::MutexContext::toString(bool /*readable*/) const
{
    // ex IntMutexContext::toString
    // FIXME: we can do better for readable=true
    return "#<lock>";
}

// /** Store implementation.
//     \param sv [out] Tag node to store to
//     \param aux [out] Aux data stream */
void
interpreter::MutexContext::store(TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& /*cs*/, SaveContext& ctx) const
{
    // Storage format is:
    //   tag is flag
    //     0=not this process, 1=this process
    //   aux is
    //     2 words for string lengths
    //     2 strings (name, info)
    // Storing just an owner flag avoids the need to name processes.
    // If one process contains a variable containing a lock owned by
    // another one, the other one will claim the lock if he still
    // rightfully owns it; if he doesn't, it's probably better to
    // disown the lock.
    out.tag   = TagNode::Tag_Mutex;
    out.value = ctx.isCurrentProcess(m_mutex->getOwner());

    uint32_t nameSize = trimSize(m_mutex->getName().size());
    uint32_t noteSize = trimSize(m_mutex->getNote().size());

    afl::bits::Value<afl::bits::UInt32LE> header[2];
    header[0] = nameSize;
    header[1] = noteSize;
    aux.handleFullData(afl::base::fromObject(header));
    aux.handleFullData(afl::string::toBytes(m_mutex->getName()).trim(static_cast<size_t>(nameSize)));
    aux.handleFullData(afl::string::toBytes(m_mutex->getNote()).trim(static_cast<size_t>(noteSize)));
}
