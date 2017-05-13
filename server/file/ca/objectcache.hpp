/**
  *  \file server/file/ca/objectcache.hpp
  *  \brief Interface server::file::ca::ObjectCache
  */
#ifndef C2NG_SERVER_FILE_CA_OBJECTCACHE_HPP
#define C2NG_SERVER_FILE_CA_OBJECTCACHE_HPP

#include "server/file/ca/objectid.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/base/ref.hpp"
#include "server/file/ca/objectstore.hpp"
#include "afl/base/optional.hpp"

namespace server { namespace file { namespace ca {

    /** Object cache base class.
        Retrieving an object or object metadata may be expensive.
        This interface implements access to a cache.

        <b>Basic principles:</b>

        If the ObjectStore loads an object or otherwise obtains object properties,
        it calls addObject() etc. to place it in the cache.
        Given a sufficient cache implemenation, further calls may be satisfied from the cache.

        Because an ObjectId irrevocably refers to an object, metadata or content may be persisted.
        It is up to the concrete cache implementation to decide what data to persist or store.

        A minimum implementation just discards everything and answers requests negatively.
        A well-meaning, stupid implementation stores everything and therefore eventually replicates the whole ObjectStore.

        If any method is used with an (ObjectId, ObjectStore::Type) pair where the ObjectId is already
        in use for a different type, an ObjectCache is free to detect a hash collision by throwing an exception. */
    class ObjectCache : public afl::base::Deletable {
     public:
        /** Add object to the cache.
            \param id Object Id
            \param type Object type
            \param content Object content (in original, i.e. uncompressed, unprefixed form)

            Note: this function is not normally called with objects that already are fully-populated in the cache,
            but should handle that case gracefully.
            This function may be called for objects that are cached but have only a size. */
        virtual void addObject(const ObjectId& id, ObjectStore::Type type, afl::base::Ref<afl::io::FileMapping> content) = 0;

        /** Add object size to cache.
            \param id Object Id
            \param type Object type
            \param size Object size (in original, i.e. uncompressed, unprefixed form).
                        For a TreeObject, this is the size of the TreeObject, not the combined size of the files it contains!

            Note: this function is not normally called with objects that already are in the cache,
            but should handle that case gracefully. */
        virtual void addObjectSize(const ObjectId& id, ObjectStore::Type type, size_t size) = 0;

        /** Remove object from cache.
            This function is called when an object is destroyed.
            \param id Object Id */
        virtual void removeObject(const ObjectId& id) = 0;

        /** Get object content.
            \param id Object Id
            \param type Object type
            \return Object content if available in cache; null otherwise */
        virtual afl::base::Ptr<afl::io::FileMapping> getObject(const ObjectId& id, ObjectStore::Type type) = 0;

        /** Get object size.
            \param id Object Id
            \param type Object type
            \return Object size if available in cache */
        virtual afl::base::Optional<size_t> getObjectSize(const ObjectId& id, ObjectStore::Type type) = 0;
    };

} } }

#endif
