/**
  *  \file server/file/ca/root.cpp
  *  \brief Class server::file::ca::Root
  */

#include "server/file/ca/root.hpp"
#include "server/file/ca/commit.hpp"
#include "server/file/ca/directoryhandler.hpp"
#include "server/file/ca/objectid.hpp"
#include "server/file/ca/objectstore.hpp"
#include "server/file/directoryhandler.hpp"

namespace {
    server::file::DirectoryHandler* getCreateDirectory(server::file::DirectoryHandler& parent, String_t name)
    {
        server::file::DirectoryHandler::Info info;
        if (!parent.findItem(name, info) || info.type != server::file::DirectoryHandler::IsDirectory) {
            info = parent.createDirectory(name);
        }
        return parent.getDirectory(info);
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
            String_t id = commitId.toHex();
            id += '\n';
            m_parent.m_refsHeads->createFile("master", afl::string::toBytes(id));

            // Update link count
            m_parent.m_store->unlinkObject(ObjectStore::CommitObject, m_commitId);
            m_commitId = commitId;
        }
 private:
    Root& m_parent;
    ObjectId m_commitId;
};

/*
 *  Root
 */

// Constructor.
server::file::ca::Root::Root(server::file::DirectoryHandler& root)
    : m_root(root),
      m_refs(),
      m_refsHeads(),
      m_objects(),
      m_store()
{
    init();
}

// Destructor.
server::file::ca::Root::~Root()
{ }

// Get ObjectId of the `master` commit.
server::file::ca::ObjectId
server::file::ca::Root::getMasterCommitId()
{
    DirectoryHandler::Info info;
    ObjectId masterCommitId = ObjectId::nil;
    if (m_refsHeads->findItem("master", info)) {
        afl::base::Ref<afl::io::FileMapping> p(m_refsHeads->getFile(info));
        afl::base::ConstBytes_t bytes(p->get());
        bytes.trim(2*sizeof(ObjectId));

        String_t name = afl::string::fromBytes(bytes);

        ObjectId id = ObjectId::fromHex(name);
        if (id.toHex() == name) {
            // Accept
            masterCommitId = id;
        }
    }
    return masterCommitId;
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
    return new DirectoryHandler(*m_store, masterTreeId, "(ca-root)", *new RootUpdater(*this, masterCommitId));
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
    m_objects.reset(getCreateDirectory(m_root, "objects"));

    // Create HEAD file
    // (This is required for git to recognize the folder.)
    m_root.createFile("HEAD", afl::string::toBytes("ref: refs/heads/master\n"));

    // Create object store
    m_store.reset(new ObjectStore(*m_objects));
}
