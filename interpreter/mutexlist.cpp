/**
  *  \file interpreter/mutexlist.cpp
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
  *
  *  FIXME c2ng: replace the adventurous reference counting with Ref<>
  */

#include <cassert>
#include "interpreter/mutexlist.hpp"
#include "interpreter/error.hpp"



interpreter::MutexList::Mutex::~Mutex()
{
    // ex IntMutex::~IntMutex
}

// /** Constructor.
//     \param slot Slot in mutex_objects.
//     \param note User's note
//     \param owner Owning process */
interpreter::MutexList::Mutex::Mutex(MutexList& container, Index_t slot, const String_t& note, Process* owner)
    : m_container(&container),
      reference_counter(1),
      slot(slot),
      note(note),
      owner(owner)
{
    // ex IntMutex::IntMutex
}

// /** Get user's note. */
const String_t&
interpreter::MutexList::Mutex::getNote() const
{
    // ex IntMutex::getNote
    return note;
}

// /** Get mutex name. */
String_t
interpreter::MutexList::Mutex::getName() const
{
    // ex IntMutex::getName
    if (m_container != 0) {
        return m_container->m_mutexNames.getNameByIndex(slot);
    } else {
        return "<dead>";
    }
}

// /** Get owning process.
//     \return the process, or 0 if orphaned */
interpreter::Process*
interpreter::MutexList::Mutex::getOwner() const
{
    // ex IntMutex::getOwner
    return owner;
}

// /** Set owning process. Use with care. */
void
interpreter::MutexList::Mutex::setOwner(Process* newOwner)
{
    // ex IntMutex::setOwner
    owner = newOwner;
}

// /** Increase reference counter.
//     \return this */
interpreter::MutexList::Mutex&
interpreter::MutexList::Mutex::addReference()
{
    // ex IntMutex::addReference
    ++reference_counter;
    return *this;
}

// /** Remove reference. Unregisters the mutex and deletes it when this
//     was the last reference. */
void
interpreter::MutexList::Mutex::removeReference()
{
    // ex IntMutex::removeReference
    if (--reference_counter == 0) {
        if (m_container != 0) {
            m_container->m_mutexObjects[slot] = 0;
        }
        delete this;
    }
}

void
interpreter::MutexList::Mutex::abandon()
{
    // disconnect container
    m_container = 0;

    // The MutexList died, so the process probably also died or will die soon.
    // Even if it behaves nicely, it will have no way to free the mutex, so do that here.
    owner = 0;
}


/******************************* MutexList *******************************/

interpreter::MutexList::MutexList()
    : m_mutexNames(),
      m_mutexObjects()
{ }

interpreter::MutexList::~MutexList()
{
    for (size_t i = 0, n = m_mutexObjects.size(); i < n; ++i) {
        if (Mutex* p = m_mutexObjects[i]) {
            p->abandon();
        }
    }
}

// /** Create new lock.
//     \param name Name of lock, in upper-case
//     \param note User's comment
//     \param owner Owning process
//     \return newly-created IntMutex object
//     \throw IntError if there is a locking conflict */
interpreter::MutexList::Mutex*
interpreter::MutexList::create(const String_t& name, const String_t& note, Process* owner)
{
    // ex IntMutex::create
    // Get slot for this mutex. An existing slot will be recycled.
    afl::data::NameMap::Index_t slot = m_mutexNames.addMaybe(name);

    // Check existing mutex
    if (slot < m_mutexObjects.size() && m_mutexObjects[slot] != 0) {
        throw interpreter::Error("Already locked");
    }

    // Create new mutex. This will place it in mutex_objects.
    return createMutex(slot, note, owner);
}

interpreter::MutexList::Mutex*
interpreter::MutexList::load(const String_t& name, const String_t& note, Process* owner)
{
    // ex IntMutexContext::load (part)
    afl::data::NameMap::Index_t slot = m_mutexNames.addMaybe(name);
    if (slot < m_mutexObjects.size() && m_mutexObjects[slot] != 0) {
        // This mutex already exists. Is it compatible?
        if (owner != 0) {
            if (m_mutexObjects[slot]->getOwner() != 0 && m_mutexObjects[slot]->getOwner() != owner) {
                throw interpreter::Error("Incompatible locks");
            }
            m_mutexObjects[slot]->setOwner(owner);
        }
        return &m_mutexObjects[slot]->addReference();
    } else {
        // Make new mutex
        return createMutex(slot, note, owner);
    }
}

// /** Query lock.
//     \param name Name of lock, in upper-case
//     \return Pointer to existing lock of that name, 0 if none */
interpreter::MutexList::Mutex*
interpreter::MutexList::query(const String_t& name) const
{
    // ex IntMutex::query
    afl::data::NameMap::Index_t slot = m_mutexNames.getIndexByName(name);
    if (slot == m_mutexNames.nil || slot >= m_mutexNames.getNumNames()) {
        return 0;
    } else {
        return m_mutexObjects[slot];
    }
}

// /** Create a mutex. This assumes that the mutex is actually free.
//     \param slot Slot in mutex_objects.
//     \param note User's note
//     \param owner Owning process */
interpreter::MutexList::Mutex*
interpreter::MutexList::createMutex(Index_t slot, const String_t& note, Process* owner)
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

// /** Disown/orphan all locks owned by a process.
//     \param process Process to check for */
void
interpreter::MutexList::disownLocksByProcess(Process* process)
{
    // ex IntMutex::disownLocksByProcess
    for (size_t i = 0, e = m_mutexObjects.size(); i < e; ++i) {
        if (m_mutexObjects[i] != 0 && m_mutexObjects[i]->getOwner() == process) {
            m_mutexObjects[i]->setOwner(0);
        }
    }
}

// /** Enumerate mutexes. This will enumerate all mutexes as defined by
//     the owner filter, and add them to data. Note that this will not
//     add references to the mutexes.

//     \param data [out] List will be produced here
//     \param owner [in] Filter. If non-null, only list mutexes owned by
//     this process. If null, list all mutexes. */
void
interpreter::MutexList::enumMutexes(std::vector<Mutex*>& data, Process* process)
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
