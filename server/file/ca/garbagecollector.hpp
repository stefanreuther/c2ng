/**
  *  \file server/file/ca/garbagecollector.hpp
  *  \brief Class server::file::ca::GarbageCollector
  */
#ifndef C2NG_SERVER_FILE_CA_GARBAGECOLLECTOR_HPP
#define C2NG_SERVER_FILE_CA_GARBAGECOLLECTOR_HPP

#include <set>
#include "afl/sys/loglistener.hpp"
#include "server/file/ca/objectid.hpp"

namespace server { namespace file { namespace ca {

    class ObjectStore;

    /** Garbage collector.
        This class determines the set of live/reachable objects in order to be able to delete unreachable objects.
        It works by building the transitive closure of objects reachable from trees.

        Normally, we delete objects in the moment they become unreachable.
        However, because we do not persist reference counters for now,
        objects that were created in a previous lifecycle will not be deleted in a future lifecycle.
        The garbage collector is intended to clean this up.
        (An alternative could have been to rebuild the reference counters.)

        This class focuses on cleaning up, not on detecting and fixing inconsistencies.
        However, some warnings are generated.

        Basic operation:
        - use addCommit() to add root commits;
        - call checkObject() until it returns false;
        - call removeGarbageObjects() until it returns false.

        If the object store changes between calls of the sequence,
        the sequence can be safely restarted at any time by adding new root commits.
        Unless modifications arrive are faster than we can read them, this is guaranteed to be complete and correct,
        but may leave objects that became orphaned by the modifications; a future run from scratch will clean those up.

        Repeatedly restarting the sequence with the same unchanged commit is guaranteed to complete,
        i.e. checkObject() does not reset the position in the sequence if it has to.

        This logic is intended to allow garbage collecting a live instance by inserting GC slices between actual user operations.
        Parallel changes from other threads/processes are not safe. */
    class GarbageCollector {
     public:
        /** Constructor.
            @param objStore Object store
            @param log      Log listener (for warning messages) */
        GarbageCollector(ObjectStore& objStore, afl::sys::LogListener& log);

        /** Destructor. */
        ~GarbageCollector();

        /** Add a commit to the set of objects to keep.
            Will eventually add the commit and the referenced tree.
            @param id Object Id of a CommitObject. */
        void addCommit(const ObjectId& id);

        /** Add a tree to the set of objects to keep.
            Will eventually add the tree and all its children.
            @param id Object Id of a TreeObject. */
        void addTree(const ObjectId& id);

        /** Add a file to the set of objects to keep.
            @param id Object Id of a DataObject. */
        void addFile(const ObjectId& id);

        /** Main sequence: check one object.
            If there are still objects to check, pick one and check it.
            @retval true  Checked at least one object and updated the set of objects to keep
            @retval false No more objects to check */
        bool checkObject();

        /** Main sequence: remove garbage objects.
            If there are still objects to remove, pick some and remove them.
            @retval true  Made some progress
            @retval false No more objects to remove */
        bool removeGarbageObjects();

        /** Get number of objects to keep so far.
            @return number */
        size_t getNumObjectsToKeep() const;

        /** Get number of objects remaining to check.
            @return number */
        size_t getNumObjectsToCheck() const;

        /** Get number of objects removed.
            @return number */
        size_t getNumObjectsRemoved() const;

        /** Get number of errors.
            A nonzero value means the object store is guaranteed-broken (but a zero value doesn't guarantee it to be intact).
            @return number */
        size_t getNumErrors() const;

     private:
        /** Set of objects.
            Measured memory consumption per node for a 20-byte object Id:
            - 64 bytes (gcc, x64)
            - 40 bytes (gcc, x86) */
        typedef std::set<ObjectId> IdSet_t;

        ObjectStore& m_objectStore;
        afl::sys::LogListener& m_log;

        IdSet_t m_objectsToKeep;
        IdSet_t m_treesToCheck;

        size_t m_nextPrefixToCheck;
        size_t m_numObjectsRemoved;
        size_t m_numErrors;
    };

} } }

#endif
