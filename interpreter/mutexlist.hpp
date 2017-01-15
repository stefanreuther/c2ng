/**
  *  \file interpreter/mutexlist.hpp
  */
#ifndef C2NG_INTERPRETER_MUTEXLIST_HPP
#define C2NG_INTERPRETER_MUTEXLIST_HPP

#include "afl/data/namemap.hpp"
#include "afl/string/string.hpp"
#include "afl/container/ptrvector.hpp"

namespace interpreter {

    class Process;

    class MutexList {
     public:
        typedef afl::data::NameMap::Index_t Index_t;

        // /** Mutex data. This holds the data associated with a mutex. These objects
        //     are reference-counted and can only be created using the create() function.
        //     All objects are stored in the global mutex_objects registry. */
        class Mutex {
            friend class MutexList;
         public:
            ~Mutex();

            const String_t& getNote() const;
            String_t getName() const;
            Process* getOwner() const;
            void setOwner(Process* newOwner);

            Mutex& addReference();
            void removeReference();

            void abandon();

         private:
            Mutex(MutexList& container, Index_t slot, const String_t& note, Process* owner);

            // Container
            MutexList* m_container;

            /** Reference counter. When this goes to 0, the mutex is freed. */
            uint32_t reference_counter;
            /** Slot. Slot in mutex_objects registry. Corresponds to the name. */
            const Index_t slot;
            /** Note. User's not associated with this mutex. */
            const String_t note;
            /** Owner. If non-null, the process owning this mutex. Otherwise, the
                mutex is orphaned. */
            Process* owner;
        };

        MutexList();
        ~MutexList();

        Mutex* create(const String_t& name, const String_t& note, Process* owner);
        Mutex* load(const String_t& name, const String_t& note, Process* owner);
        Mutex* query(const String_t& name) const;

        Mutex* createMutex(Index_t slot, const String_t& note, Process* owner);

        void disownLocksByProcess(Process* process);
        void enumMutexes(std::vector<Mutex*>& data, Process* process);

     private:
        /** All mutex names. */
        afl::data::NameMap m_mutexNames;      // ex int/mutex.cc:mutex_names

        /** All mutex objects. */
        // afl::container::PtrVector<Mutex> m_mutexObjects;
        std::vector<Mutex*> m_mutexObjects;   // ex int/mutex.cc:mutex_objects
    };

}

#endif
