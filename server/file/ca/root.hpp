/**
  *  \file server/file/ca/root.hpp
  *  \brief Class server::file::ca::Root
  */
#ifndef C2NG_SERVER_FILE_CA_ROOT_HPP
#define C2NG_SERVER_FILE_CA_ROOT_HPP

#include <memory>
#include "afl/base/deletable.hpp"
#include "afl/base/optional.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/sys/loglistener.hpp"
#include "server/file/ca/objectid.hpp"
#include "server/file/directoryhandler.hpp"

namespace server { namespace file { namespace ca {

    class ObjectStore;

    /** Root of a content-addressable file store.
        Implements bootstrapping of a file store in a git-compatible way:
        - create the ObjectStore on directory `objects`
        - create the `HEAD` and `refs/heads/master` metadata files */
    class Root : public afl::base::Deletable {
     public:
        /** Constructor.
            @param root DirectoryHandler to work on
            @param log  Logger (must live for duration of initialisation; logs initialisation) */
        Root(server::file::DirectoryHandler& root, afl::sys::LogListener& log);

        /** Destructor. */
        ~Root();

        /** Get ObjectId of the `master` commit.
            This commit represents the current status.
            @return commit Id
            @see ObjectStore::getCommit */
        ObjectId getMasterCommitId();

        /** Set ObjectId of the `master` commit.
            Note that this operation must not be used on a Root that also has a live root handler (createRootHandler()).
            The root handler and objects created by it will cache information that would be invalidated by this call.
            @param objId Object Id */
        void setMasterCommitId(const ObjectId& objId);

        /** Get ObjectId of a snapshot.
            @param snapshotName Snapshot name (lower-case alphanumeric)
            @return commit Id referring to a commit object, if any */
        afl::base::Optional<ObjectId> getSnapshotCommitId(String_t snapshotName);

        /** Set ObjectId of a snapshot.
            @param snapshotName Snapshot name (lower-case alphanumeric)
            @param objId   Object Id, referring to a commit object (e.g. from getMasterCommitId(), getSnapshotCommitId()) */
        void setSnapshotCommitId(String_t snapshotName, const ObjectId& objId);

        /** Remove a snapshot.
            This will potentially delete objects.
            @param snapshotName Snapshot name (lower-case alphanumeric) */
        void removeSnapshot(String_t snapshotName);

        /** Get list of snapshots.
            @param list [out]  Snapshots */
        void listSnapshots(afl::data::StringList_t& list);

        /** Get list of root objects.
            Enumerates all commits that must be treated as roots, and not be deleted.
            This function makes no attempt to remove duplicates.
            @param list [out] List of ObjectId's pointing at commit objects */
        void listRoots(std::vector<ObjectId>& list);

        /** Create DirectoryHandler for root directory.
            This DirectoryHandler supports all operations.
            @return newly-allocated DirectoryHandler */
        server::file::DirectoryHandler* createRootHandler();

        /** Create read-only DirectoryHandler for a commit (snapshot, master).
            @param commitId Object Id
            @return newly-allocated DirectoryHandler */
        server::file::DirectoryHandler* createSnapshotHandler(ObjectId commitId);

        /** Access the ObjectStore instance.
            @return ObjectStore instance */
        ObjectStore& objectStore();

     private:
        /** Implementation of ReferenceUpdater for root directory */
        class RootUpdater;
        class SnapshotHandler;

        /** Initialize. */
        void init(afl::sys::LogListener& log);

        /** Load pack files.
            Iterate the objects/pack directory (if any) and add all found packs.
            @param log Logger */
        void loadPackFiles(afl::sys::LogListener& log);

        /** Unpack packed-refs file.
            When using "git gc" for packing a repository, it will combine all branches and tags into a packed-refs file.
            Although an unpacked ref always has precedence over packed-refs, this would interfere with deleting snapshots.
            We therefore unpack and delete this file.
            @param log Logger */
        void unpackPackedRefs(afl::sys::LogListener& log);

        /** DirectoryHandler as given to constructor. */
        server::file::DirectoryHandler& m_root;

        /** Directory m_root/refs. */
        std::auto_ptr<server::file::DirectoryHandler> m_refs;

        /** Directory m_root/refs/heads. */
        std::auto_ptr<server::file::DirectoryHandler> m_refsHeads;

        /** Directory m_root/refs/tags. */
        std::auto_ptr<server::file::DirectoryHandler> m_refsTags;

        /** Directory m_root/objects. */
        std::auto_ptr<server::file::DirectoryHandler> m_objects;

        /** ObjectStore instance. Never null during lifetime of this object. */
        std::auto_ptr<ObjectStore> m_store;

        /** Snapshot Handler. */
        std::auto_ptr<SnapshotHandler> m_snapshotHandler;
    };

} } }

#endif
