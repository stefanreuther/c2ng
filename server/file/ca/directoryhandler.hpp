/**
  *  \file server/file/ca/directoryhandler.hpp
  *  \brief Class server::file::ca::DirectoryHandler
  */
#ifndef C2NG_SERVER_FILE_CA_DIRECTORYHANDLER_HPP
#define C2NG_SERVER_FILE_CA_DIRECTORYHANDLER_HPP

#include "afl/base/ptr.hpp"
#include "afl/base/ref.hpp"
#include "server/file/ca/objectid.hpp"
#include "server/file/ca/referenceupdater.hpp"
#include "server/file/directoryhandler.hpp"

namespace server { namespace file { namespace ca {

    class ObjectStore;

    /** Implementation of DirectoryHandler for content-addressable back-end.
        This uses an ObjectStore to store directory content and files.

        A directory is identified by an ObjectId.
        Every change to the directory changes the ObjectId.
        Each DirectoryHandler therefore has a ReferenceUpdater that can update the reference pointing to it.
        For the root directory, that would be the commit pointing to it;
        for subdirectories, that would be the parent directory.

        FIXME: creating multiple DirectoryHandler instances for the same directory will cause inconsistencies
        because the instances are not synchronized against each other.
        It probably makes sense to protect against that somehow. */
    class DirectoryHandler : public server::file::DirectoryHandler {
     public:
        /** Constructor.
            \param store Object store
            \param id Initial object Id
            \param name Directory name
            \param updater ReferenceUpdater instance. Null to make this a read-only DirectoryHandler.
            \param sh SnapshotHandler; can be null */
        DirectoryHandler(ObjectStore& store, const ObjectId& id, const String_t& name, afl::base::Ptr<ReferenceUpdater> updater, SnapshotHandler* sh);

        /** Destructor. */
        ~DirectoryHandler();

        // DirectoryHandler:
        virtual String_t getName();
        virtual afl::base::Ref<afl::io::FileMapping> getFile(const Info& info);
        virtual afl::base::Ref<afl::io::FileMapping> getFileByName(String_t name);
        virtual Info createFile(String_t name, afl::base::ConstBytes_t content);
        virtual void removeFile(String_t name);
        virtual afl::base::Optional<Info> copyFile(ReadOnlyDirectoryHandler& source, const Info& sourceInfo, String_t name);

        virtual void readContent(Callback& callback);
        virtual DirectoryHandler* getDirectory(const Info& info);
        virtual Info createDirectory(String_t name);
        virtual void removeDirectory(String_t name);

        virtual SnapshotHandler* getSnapshotHandler();
        virtual afl::base::Ptr<afl::io::Directory> getDirectory();

     private:
        /** Local ReferenceUpdater descendant.
            Because a child DirectoryHandler may live longer than this one,
            we store all our state in a reference-counted object that can survive us. */
        class ContentUpdater;

        afl::base::Ref<ContentUpdater> m_content;
        SnapshotHandler* m_snapshotHandler;
    };

} } }

#endif
