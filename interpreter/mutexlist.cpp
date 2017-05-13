/**
  *  \file interpreter/mutexlist.cpp
  *  \brief Class interpreter::MutexList
  *
  *  PCC2 Comment:
  *
  *  Mutexes/locks are a convention to provide cooperation-based mutual
  *  exclusion for game features. It is intended to avoid that the user
  *  accidentally cancels an Auto Task action, or vice versa.
  *
  *  As of 1.1.18, PCC honors the following locks:
  *  - "Snnn.WAYPOINT" (ship #nnn's waypoint and intercept mission)
  *  - "Pnnn.TAX" (planet #nnn's taxation)
  *  - "Pnnn.BUILD" (planet #nnn's structure building)
  *
  *  The scripting language provides a feature
  *      With Lock("...") Do
  *  causing the content of the 'With' block to be executed with a lock
  *  being held. Only one process can be in such a block, another process
  *  (or the same process again) trying to enter the block will fail.
  *
  *  When this pattern is followed, lifetime of locks will be nicely
  *  managed by the interpreter. When a process goes away, all its
  *  data will go away, and so does the lock. However, it is possible
  *  to do things like
  *      sharedVar := Lock("...")
  *  which will associate the lock with this process, but then leave a
  *  reference in the shared variable. The lock will thus be still active
  *  but its process will be gone. This is called an orphaned / disowned
  *  lock.
  *
  *  Note that we use the terms 'Lock' and 'Mutex' interchangably when
  *  talking about these things. 'Lock' is the user-visible name. However,
  *  there is also the internal feature 'lock' meaning "locking a sensor
  *  beam onto an object".
  *
  *  This does not implement PCC 1.x's "CC$Lock" and "CC$Unlock" commands.
  *  When needed, they can be implemented as scripts using something like
  *  a hash-of-mutexes.
  */

#include <cassert>
#include "interpreter/mutexlist.hpp"
#include "interpreter/error.hpp"


// Destructor.
interpreter::MutexList::Mutex::~Mutex()
{
    // ex IntMutex::~IntMutex
}

// Constructor.
interpreter::MutexList::Mutex::Mutex(MutexList& container, Index_t slot, const String_t& note, const Process* owner)
    : m_container(&container),
      m_referenceCounter(1),
      m_slot(slot),
      m_note(note),
      m_owner(owner)
{
    // ex IntMutex::IntMutex
}

// Get user's note.
const String_t&
interpreter::MutexList::Mutex::getNote() const
{
    // ex IntMutex::getNote
    return m_note;
}

// Get mutex name.
String_t
interpreter::MutexList::Mutex::getName() const
{
    // ex IntMutex::getName
    if (m_container != 0) {
        return m_container->m_mutexNames.getNameByIndex(m_slot);
    } else {
        return "<dead>";
    }
}

// Get owning process.
const interpreter::Process*
interpreter::MutexList::Mutex::getOwner() const
{
    // ex IntMutex::getOwner
    return m_owner;
}

// Set owning process.
void
interpreter::MutexList::Mutex::setOwner(const Process* newOwner)
{
    // ex IntMutex::setOwner
    m_owner = newOwner;
}

// Increase reference count.
interpreter::MutexList::Mutex&
interpreter::MutexList::Mutex::addReference()
{
    // ex IntMutex::addReference
    ++m_referenceCounter;
    return *this;
}

// Remove a reference.
void
interpreter::MutexList::Mutex::removeReference()
{
    // ex IntMutex::removeReference
    if (--m_referenceCounter == 0) {
        if (m_container != 0 && m_slot < m_container->m_mutexObjects.size()) {
            m_container->m_mutexObjects[m_slot] = 0;
        }
        delete this;
    }
}

// Abandon mutex.
void
interpreter::MutexList::Mutex::abandon()
{
    // disconnect container
    m_container = 0;

    // The MutexList died, so the process probably also died or will die soon.
    // Even if it behaves nicely, it will have no way to free the mutex, so do that here.
    m_owner = 0;
}


/******************************* MutexList *******************************/

// Constructor.
interpreter::MutexList::MutexList()
    : m_mutexNames(),
      m_mutexObjects()
{ }

// Destructor.
interpreter::MutexList::~MutexList()
{
    for (size_t i = 0, n = m_mutexObjects.size(); i < n; ++i) {
        if (Mutex* p = m_mutexObjects[i]) {
            p->abandon();
        }
    }
}

// Create a new lock, creation semantics.
interpreter::MutexList::Mutex*
interpreter::MutexList::create(const String_t& name, const String_t& note, const Process* owner)
{
    // ex IntMutex::create
    // Get slot for this mutex. An existing slot will be recycled.
    afl::data::NameMap::Index_t slot = m_mutexNames.addMaybe(name);

    // Check existing mutex
    if (getMutexBySlot(slot) != 0) {
        throw interpreter::Error("Already locked");
    }

    // Create new mutex. This will place it in m_mutexObjects.
    return createMutex(slot, note, owner);
}

// Create a new lock, load semantics.
interpreter::MutexList::Mutex*
interpreter::MutexList::load(const String_t& name, const String_t& note, const Process* owner)
{
    // ex IntMutexContext::load (part)
    afl::data::NameMap::Index_t slot = m_mutexNames.addMaybe(name);
    if (Mutex* mtx = getMutexBySlot(slot)) {
        // This mutex already exists. Is it compatible?
        if (owner != 0) {
            if (mtx->getOwner() != 0 && mtx->getOwner() != owner) {
                throw interpreter::Error("Incompatible locks");
            }
            mtx->setOwner(owner);
        }
        return &mtx->addReference();
    } else {
        // Make new mutex
        return createMutex(slot, note, owner);
    }
}

// Query lock.
interpreter::MutexList::Mutex*
interpreter::MutexList::query(const String_t& name) const
{
    // ex IntMutex::query
    afl::data::NameMap::Index_t slot = m_mutexNames.getIndexByName(name);
    return getMutexBySlot(slot);
}

// Disown/orphan all locks owned by a process.
void
interpreter::MutexList::disownLocksByProcess(const Process* process)
{
    // ex IntMutex::disownLocksByProcess
    for (size_t i = 0, e = m_mutexObjects.size(); i < e; ++i) {
        if (m_mutexObjects[i] != 0 && m_mutexObjects[i]->getOwner() == process) {
            m_mutexObjects[i]->setOwner(0);
        }
    }
}

// Enumerate mutexes.
void
interpreter::MutexList::enumMutexes(std::vector<Mutex*>& data, const Process* process)
{
    // ex IntMutex::enumMutexes
    for (size_t i = 0, e = m_mutexObjects.size(); i < e; ++i) {
        if (m_mutexObjects[i] != 0
            && (process == 0 || m_mutexObjects[i]->getOwner() == process))
        {
            data.push_back(m_mutexObjects[i]);
        }
    }
}

// Create a mutex.
interpreter::MutexList::Mutex*
interpreter::MutexList::createMutex(Index_t slot, const String_t& note, const Process* owner)
{
    // ex IntMutex::createMutex
    // Make room
    while (m_mutexObjects.size() <= slot) {
        m_mutexObjects.push_back(0);
    }

    // Make a new mutex
    assert(m_mutexObjects[slot] == 0);
    Mutex* mtx = new Mutex(*this, slot, note, owner);
    m_mutexObjects[slot] = mtx;
    return mtx;
}

// Get mutex, given a slot.
interpreter::MutexList::Mutex*
interpreter::MutexList::getMutexBySlot(Index_t slot) const
{
    if (slot < m_mutexObjects.size()) {
        return m_mutexObjects[slot];
    } else {
        return 0;
    }
}
