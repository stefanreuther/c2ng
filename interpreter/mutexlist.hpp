/**
  *  \file interpreter/mutexlist.hpp
  *  \brief Class interpreter::MutexList
  */
#ifndef C2NG_INTERPRETER_MUTEXLIST_HPP
#define C2NG_INTERPRETER_MUTEXLIST_HPP

#include "afl/data/namemap.hpp"
#include "afl/string/string.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/base/uncopyable.hpp"

namespace interpreter {

    class Process;

    /** Mutex list.
        Mutexes/locks are a convention to provide cooperation-based mutual exclusion for game features.
        It is intended to avoid that the user accidentally cancels an Auto Task action, or vice versa.

        MutexList contains all the mutexes from an interpreter World.

        Each mutex is referenced by a MutexContext, and counts as locked as long as a MutexContext exists (-> reference counting).
        If the reference count drops to zero, the mutex is destroyed.

        Normally, the MutexContext lives on a process' stack and will go away when the process goes away.
        If the value escapes the process, and the process dies, the mutex gets disowned,
        that is, it remains active but no longer associated with a process. */
    class MutexList : afl::base::Uncopyable {
     public:
        /** Shortcut for a mutex index. */
        typedef afl::data::NameMap::Index_t Index_t;

        /** Mutex data.
            This holds the data associated with a mutex.
            These objects are reference-counted and can only be created using the create() function.
            All objects are stored in the MutexList's m_mutexObjects registry. */
        class Mutex {
            friend class MutexList;
         public:
            /** Destructor. */
            ~Mutex();

            /** Get user's note.
                \return note */
            const String_t& getNote() const;

            /** Get mutex name.
                \return name */
            String_t getName() const;

            /** Get owning process.
                \return the process, 0 if disowned */
            const Process* getOwner() const;

            /** Set owning process.
                For use by MutexList only.
                \param newOwner New owner (0 to disown) */
            void setOwner(const Process* newOwner);

            /** Increase reference count.
                \return *this */
            Mutex& addReference();

            /** Remove a reference.
                If the reference count drops to 0, marks this mutex clean and deletes it. */
            void removeReference();

            /** Abandon mutex.
                Called when the MutexList dies, to remove the association. */
            void abandon();

         private:
            /** Constructor.
                \param container Container
                \param slot Slot in container.m_mutexObjects.
                \param note User's note
                \param owner Owning process */
            Mutex(MutexList& container, Index_t slot, const String_t& note, const Process* owner);

            /** Link to container. */
            MutexList* m_container;

            /** Reference counter.
                When this goes to 0, the mutex is freed. */
            size_t m_referenceCounter;

            /** Slot.
                Slot in m_container->m_mutexObjects and m_container->m_mutexNames.
                Corresponds to the name. */
            const Index_t m_slot;

            /** Note.
                User's note associated with this mutex. */
            const String_t m_note;

            /** Owner.
                If non-null, the process owning this mutex.
                Otherwise, the mutex is orphaned. */
            const Process* m_owner;
        };

        /** Constructor.
            Creates an empty list. */
        MutexList();

        /** Destructor.
            Abandons all remaining mutexes. */
        ~MutexList();

        /** Create a new lock, creation semantics.
            It is an error to create a new lock if one with the same name already exists, no matter who owns it,
            even if that is the same process or a disowned lock.
            \param name Name of lock, in upper case
            \param note User's note
            \param owner Owning process
            \return Mutex object
            \throw Error on locking conflict */
        Mutex* create(const String_t& name, const String_t& note, const Process* owner);

        /** Create a new lock, load semantics.
            If the lock already exists and is unowned or owned by the same process as requested, this just re-uses the existing lock.
            \param name Name of lock, in upper case
            \param note User's note
            \param owner Owning process
            \return Mutex object
            \throw Error on locking conflict */
        Mutex* load(const String_t& name, const String_t& note, const Process* owner);

        /** Query lock.
            \param name Name of lock, in upper-case
            \return Pointer to existing lock of that name, 0 if none */
        Mutex* query(const String_t& name) const;

        /** Disown/orphan all locks owned by a process.
            \param process Process to check for (must be non-null for this call to have any effect) */
        void disownLocksByProcess(const Process* process);

        /** Enumerate mutexes.
            This will enumerate all mutexes as defined by the \c owner filter, and add them to \c data.
            Note that this will not add references to the mutexes.

            \param [out] data     List will be produced here
            \param [in]  process  Filter. If non-null, only list mutexes owned by this process. If null, list all mutexes. */
        void enumMutexes(std::vector<Mutex*>& data, const Process* process) const;

     private:
        /** Create a mutex.
            This assumes that the mutex is actually free.
            \param slot Slot in m_mutexObjects.
            \param note User's note
            \param owner Owning process */
        Mutex* createMutex(Index_t slot, const String_t& note, const Process* owner);

        /** Get mutex, given a slot.
            \param slot Slot in m_mutexObjects
            \return Mutex, or null */
        Mutex* getMutexBySlot(Index_t slot) const;

        /** All mutex names. */
        afl::data::NameMap m_mutexNames;      // ex int/mutex.cc:mutex_names

        /** All mutex objects. */
        std::vector<Mutex*> m_mutexObjects;   // ex int/mutex.cc:mutex_objects
    };

}

#endif
