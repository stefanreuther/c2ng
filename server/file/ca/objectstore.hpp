/**
  *  \file server/file/ca/objectstore.hpp
  *  \brief Class server::file::ca::ObjectStore
  */
#ifndef C2NG_SERVER_FILE_CA_OBJECTSTORE_HPP
#define C2NG_SERVER_FILE_CA_OBJECTSTORE_HPP

#include <memory>
#include "afl/base/growablememory.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/ref.hpp"
#include "afl/base/types.hpp"
#include "afl/base/uncopyable.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/io/filemapping.hpp"
#include "server/file/ca/objectid.hpp"
#include "server/file/ca/packfile.hpp"
#include "server/file/directoryhandler.hpp"

namespace server { namespace file { namespace ca {

    class ReferenceCounter;
    class ObjectCache;

    /** Object storage.
        This is the central component of the content-addressable storage backend.
        At allows storage and retrieval of typed objects.
        - storing an object produces an ObjectId
        - using that ObjectId (and type) allows retrieving the object

        This class also aggregates optional features:
        - data and metadata caching
        - pack files (read-only objects)
        - reference counting

        Reference counting enables removal of objects that become unused.

        We do not try to combine or cancel writes.
        Updating 3 files in a directory will write out the individual versions of that directory several times.
        With reference counting enabled, the superseded versions will immediately be deleted again and, with Linux, never hit the disk I/O. */
    class ObjectStore : private afl::base::Uncopyable {
     public:
        /** Object type. */
        enum Type {
            DataObject,         ///< Data object ("blob"). Contains uninterpreted data.
            TreeObject,         ///< Directory object ("tree"). Points to TreeObject's or DataObject's. See class server::file::ca::DirectoryEntry.
            CommitObject        ///< Commit object ("commit"). Points to a TreeObject. See class server::file::ca::Commit.
        };

        /** Constructor.
            \param dir directory to store objects in ("objects" directory; children will be "hex-byte" directory containing the objects). */
        explicit ObjectStore(server::file::DirectoryHandler& dir);

        /** Destructor. */
        ~ObjectStore();

        /** Add a new pack file.
            This file is used to resolve object references.
            \param p Newly-allocated PackFile. Must not be null. Ownership is taken by ObjectStore. */
        void addNewPackFile(PackFile* p);

        /** Get object content.
            \param id Object Id
            \param expectedType Expected type
            \return FileMapping representing the object's content
            \throw afl::except::FileProblemException on errors:
            - object does not exist
            - object has wrong format/bad content
            - object has wrong type */
        afl::base::Ref<afl::io::FileMapping> getObject(const ObjectId& id, Type expectedType);

        /** Read object as commit.
            \param id Commit Id
            \return tree Id associated with commit
            \throw afl::except::FileProblemException on errors:
            - object does not exist
            - object is not a commit
            - object has wrong format */
        ObjectId getCommit(const ObjectId& id);

        /** Get object size.
            This function returns the same as getObject(...)->get().size(), but possibly more efficient.
            \param id Object Id
            \param expectedType Expected type
            \return object payload size
            \throw afl::except::FileProblemException on errors:
            - object does not exist
            - object has wrong format/bad content
            - object has wrong type */
        size_t getObjectSize(const ObjectId& id, Type expectedType);

        /** Add an object.
            If an object already exists with the same content, its reference counter is increased.
            If the object does not exist, it is created with reference counter 1.
            It is an error if the object already exists but has different content (hash collision).
            If the object contains links to other objects (TreeObject, CommitObject),
            you are expected to have accounted for one link for each referenced object.
            If this call does not actually create a new object, it will adjust accordingly.
            \param type Object type
            \param data Payload
            \return Object Id */
        ObjectId addObject(Type type, afl::base::ConstBytes_t data);

        /** Link an object.
            Adds one to the object's reference counter.
            \param id Object Id */
        void linkObject(const ObjectId& id);

        /** Unlink an object.
            Removes one from the object's reference counter.
            If the reference counter reaches 0, the object can be removed from the underlying storage;
            if it references other objects, their reference count is reduced as well, recursively.
            \param type Object type
            \param id Object Id */
        void unlinkObject(Type type, const ObjectId& id);

        /** Get directory for a first-byte directory.
            \param prefix Number of directory (0..255)
            \return DirectoryHandler if one exists, null if this directory does not exist (=has no objects) */
        DirectoryHandler* getObjectDirectory(size_t prefix);

     private:
        bool loadObject(const ObjectId& id, Type expectedType, size_t maxLevel, size_t* pSize, afl::base::Ptr<afl::io::FileMapping>* pContent);
        bool loadObjectFromPackFile(const ObjectId& id, Type expectedType, size_t maxLevel, size_t* pSize, afl::base::Ptr<afl::io::FileMapping>* pContent);
        void readDirectory();
        void unlinkContent(Type type, afl::base::ConstBytes_t data);

        // DirectoryHandler for the "objects" directory.
        DirectoryHandler& m_directory;

        // DirectoryHandler's for the 256 first-byte directories.
        afl::container::PtrVector<DirectoryHandler> m_subdirectories;

        // Pack files
        afl::container::PtrVector<PackFile> m_packFiles;

        // ReferenceCounter
        std::auto_ptr<ReferenceCounter> m_refCounter;

        // Cache
        std::auto_ptr<ObjectCache> m_cache;
    };

} } }

#endif
