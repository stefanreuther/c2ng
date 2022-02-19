/**
  *  \file server/file/ca/root.hpp
  *  \brief Class server::file::ca::Root
  */
#ifndef C2NG_SERVER_FILE_CA_ROOT_HPP
#define C2NG_SERVER_FILE_CA_ROOT_HPP

#include <memory>
#include "afl/base/deletable.hpp"
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
            @param root DirectoryHandler to work on */
        explicit Root(server::file::DirectoryHandler& root);

        /** Destructor. */
        ~Root();

        /** Get ObjectId of the `master` commit.
            This commit represents the current status.
            @return commit Id
            @see ObjectStore::getCommit */
        ObjectId getMasterCommitId();

        /** Create DirectoryHandler for root directory.
            This DirectoryHandler supports all operations.
            @return newly-allocated DirectoryHandler */
        server::file::DirectoryHandler* createRootHandler();

        /** Access the ObjectStore instance.
            @return ObjectStore instance */
        ObjectStore& objectStore();

     private:
        /** Implementation of ReferenceUpdater for root directory */
        class RootUpdater;

        /** Initialize. */
        void init();

        /** DirectoryHandler as given to constructor. */
        server::file::DirectoryHandler& m_root;

        /** Directory m_root/refs. */
        std::auto_ptr<server::file::DirectoryHandler> m_refs;

        /** Directory m_root/refs/heads. */
        std::auto_ptr<server::file::DirectoryHandler> m_refsHeads;

        /** Directory m_root/objects. */
        std::auto_ptr<server::file::DirectoryHandler> m_objects;

        /** ObjectStore instance. Never null during lifetime of this object. */
        std::auto_ptr<ObjectStore> m_store;
    };

} } }

#endif
