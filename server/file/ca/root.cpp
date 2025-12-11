/**
  *  \file server/file/ca/root.cpp
  *  \brief Class server::file::ca::Root
  */

#include "server/file/ca/root.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/string/messages.hpp"
#include "server/file/ca/commit.hpp"
#include "server/file/ca/directoryhandler.hpp"
#include "server/file/ca/objectid.hpp"
#include "server/file/ca/objectstore.hpp"
#include "server/file/directoryhandler.hpp"

using server::file::DirectoryHandler;
using server::file::ca::ObjectId;
using server::file::ca::ObjectStore;

namespace {
    DirectoryHandler* getCreateDirectory(DirectoryHandler& parent, String_t name)
    {
        DirectoryHandler::Info info;
        if (!parent.findItem(name, info) || info.type != DirectoryHandler::IsDirectory) {
            info = parent.createDirectory(name);
        }
        return parent.getDirectory(info);
    }

    /* Read commit ID from a file.
       Returns commit ID on success, otherwise nothing. */
    afl::base::Optional<ObjectId> readCommitId(DirectoryHandler& dir, const String_t& fileName)
    {
        DirectoryHandler::Info info;
        afl::base::Optional<ObjectId> commitId;
        if (dir.findItem(fileName, info)) {
            afl::base::Ref<afl::io::FileMapping> p(dir.getFile(info));
            afl::base::ConstBytes_t bytes(p->get());
            bytes.trim(2*sizeof(ObjectId));

            String_t objName = afl::string::fromBytes(bytes);
            ObjectId id = ObjectId::fromHex(objName);
            if (id.toHex() == objName) {
                // Accept
                commitId = id;
            }
        }
        return commitId;
    }

    /* Write commit ID to a file. */
    void writeCommitId(DirectoryHandler& dir, const String_t& fileName, const ObjectId& objId)
    {
        String_t id = objId.toHex();
        id += '\n';
        dir.createFile(fileName, afl::string::toBytes(id));
    }

    void updateCommitId(DirectoryHandler& dir, const String_t& fileName, const ObjectId& objId, ObjectStore& objStore)
    {
        // Read old value
        afl::base::Optional<ObjectId> commitId = readCommitId(dir, fileName);

        // Update new reference
        objStore.linkObject(objId);

        // Update commit
        writeCommitId(dir, fileName, objId);

        // Update old reference
        if (const ObjectId* p = commitId.get()) {
            objStore.unlinkObject(ObjectStore::CommitObject, *p);
        }
    }
}

/*
 *  RootUpdater: create a commit and rewrite the `refs/heads/master` file
 */

class server::file::ca::Root::RootUpdater : public server::file::ca::ReferenceUpdater {
 public:
    RootUpdater(Root& parent, const ObjectId& commitId)
        : m_parent(parent),
          m_commitId(commitId)
        { }

    virtual void updateDirectoryReference(const String_t& /*name*/, const ObjectId& newId)
        {
            // Create a commit that points to this reference
            afl::base::GrowableMemory<uint8_t> out;
            server::file::ca::Commit(newId).store(out);
            ObjectId commitId = m_parent.m_store->addObject(ObjectStore::CommitObject, out);

            // Update master
            writeCommitId(*m_parent.m_refsHeads, "master", commitId);

            // Update link count
            m_parent.m_store->unlinkObject(ObjectStore::CommitObject, m_commitId);
            m_commitId = commitId;
        }
 private:
    Root& m_parent;
    ObjectId m_commitId;
};

/*
 *  SnapshotHandler
 */

class server::file::ca::Root::SnapshotHandler : public server::file::DirectoryHandler::SnapshotHandler {
 public:
    SnapshotHandler(Root& parent)
        : m_parent(parent)
        { }
    virtual void createSnapshot(String_t name)
        { m_parent.setSnapshotCommitId(name, m_parent.getMasterCommitId()); }
    virtual void copySnapshot(String_t oldName, String_t newName)
        {
            afl::base::Optional<ObjectId> commitId = m_parent.getSnapshotCommitId(oldName);
            if (!commitId.isValid()) {
                throw afl::except::FileProblemException(oldName, afl::string::Messages::fileNotFound());
            }
            m_parent.setSnapshotCommitId(newName, *commitId.get());
        }
    virtual void removeSnapshot(String_t name)
        { m_parent.removeSnapshot(name); }
    virtual void listSnapshots(afl::data::StringList_t& out)
        { m_parent.listSnapshots(out); }
 private:
    Root& m_parent;
};


/*
 *  Root
 */

// Constructor.
server::file::ca::Root::Root(server::file::DirectoryHandler& root)
    : m_root(root),
      m_refs(),
      m_refsHeads(),
      m_refsTags(),
      m_objects(),
      m_store(),
      m_snapshotHandler()
{
    init();
    m_snapshotHandler.reset(new SnapshotHandler(*this));
}

// Destructor.
server::file::ca::Root::~Root()
{ }

// Get ObjectId of the `master` commit.
server::file::ca::ObjectId
server::file::ca::Root::getMasterCommitId()
{
    return readCommitId(*m_refsHeads, "master").orElse(ObjectId::nil);
}

// Set ObjectId of the `master` commit.
void
server::file::ca::Root::setMasterCommitId(const ObjectId& objId)
{
    updateCommitId(*m_refsHeads, "master", objId, *m_store);
}

// Get ObjectId of a commit.
afl::base::Optional<server::file::ca::ObjectId>
server::file::ca::Root::getSnapshotCommitId(String_t snapshotName)
{
    return readCommitId(*m_refsTags, snapshotName);
}

// Set ObjectId of a snapshot.
void
server::file::ca::Root::setSnapshotCommitId(String_t snapshotName, const ObjectId& objId)
{
    updateCommitId(*m_refsTags, snapshotName, objId, *m_store);
}

// Remove a snapshot.
void
server::file::ca::Root::removeSnapshot(String_t snapshotName)
{
    afl::base::Optional<ObjectId> commitId = readCommitId(*m_refsTags, snapshotName);
    if (const ObjectId* p = commitId.get()) {
        m_refsTags->removeFile(snapshotName);
        m_store->unlinkObject(ObjectStore::CommitObject, *p);
    }
}

// Get list of snapshots.
void
server::file::ca::Root::listSnapshots(afl::data::StringList_t& list)
{
    class Callback : public DirectoryHandler::Callback {
     public:
        Callback(afl::data::StringList_t& list)
            : m_list(list)
            { }
        virtual void addItem(const DirectoryHandler::Info& info)
            {
                if (!info.name.empty() && info.name[0] != '.' && info.type == DirectoryHandler::IsFile) {
                    m_list.push_back(info.name);
                }
            }
     private:
        afl::data::StringList_t& m_list;
    };
    Callback cb(list);
    m_refsTags->readContent(cb);
}

// Get list of root objects.
void
server::file::ca::Root::listRoots(std::vector<ObjectId>& list)
{
    list.push_back(getMasterCommitId());

    afl::data::StringList_t snapshotNames;
    listSnapshots(snapshotNames);
    for (size_t i = 0; i < snapshotNames.size(); ++i) {
        afl::base::Optional<ObjectId> optId = getSnapshotCommitId(snapshotNames[i]);
        if (const ObjectId* p = optId.get()) {
            list.push_back(*p);
        }
    }
}

// Create DirectoryHandler for root directory.
server::file::DirectoryHandler*
server::file::ca::Root::createRootHandler()
{
    // Read Id of master
    ObjectId masterCommitId = getMasterCommitId();

    // Master is a commit. Read its tree.
    ObjectId masterTreeId = m_store->getCommit(masterCommitId);

    // Create root directory item
    return new DirectoryHandler(*m_store, masterTreeId, "(ca-root)", new RootUpdater(*this, masterCommitId), m_snapshotHandler.get());
}

// Create read-only DirectoryHandler for a snapshot.
server::file::DirectoryHandler*
server::file::ca::Root::createSnapshotHandler(ObjectId commitId)
{
    ObjectId treeId = m_store->getCommit(commitId);
    return new DirectoryHandler(*m_store, treeId, "(ca-snapshot)", 0, 0);
}

// Access the ObjectStore instance.
server::file::ca::ObjectStore&
server::file::ca::Root::objectStore()
{
    return *m_store;
}

// Initialize.
void
server::file::ca::Root::init()
{
    // Create directories
    m_refs.reset(getCreateDirectory(m_root, "refs"));
    m_refsHeads.reset(getCreateDirectory(*m_refs, "heads"));
    m_refsTags.reset(getCreateDirectory(*m_refs, "tags"));
    m_objects.reset(getCreateDirectory(m_root, "objects"));

    // Create HEAD file
    // (This is required for git to recognize the folder.)
    m_root.createFile("HEAD", afl::string::toBytes("ref: refs/heads/master\n"));

    // Create object store
    m_store.reset(new ObjectStore(*m_objects));
}
