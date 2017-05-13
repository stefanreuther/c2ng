/**
  *  \file server/file/ca/internalobjectcache.hpp
  *  \brief Class server::file::ca::InternalObjectCache
  */
#ifndef C2NG_SERVER_FILE_CA_INTERNALOBJECTCACHE_HPP
#define C2NG_SERVER_FILE_CA_INTERNALOBJECTCACHE_HPP

#include "server/file/ca/objectcache.hpp"
#include "afl/container/ptrmap.hpp"

namespace server { namespace file { namespace ca {

    /** Internal (in-memory) object cache.
        This tracks object metadata and content in memory, up to a configured upper limit.
        It makes no attempt at persisting metadata.

        <b>Usage statistic 20170314:</b>

        Test case "import hostfile data" ("c2fileclient cp -r INPUT ca:OUTPUT"):
        - 7367168k user data (du -sk INPUT)
        - 27 minutes conversion time on 'rocket' (~6:30 CPU)
        - 113772 objects (55% savings)
        - 925968k object data (du -sk OUTPUT)
        - 615114k effective object data (file size)
        - 9725501440 bytes written, 7566229504 cancelled (=short-lived objects)
        - <b>Object cache gets CPU usage down to approx. 5:30</b> but no difference in other metrics

        Test case "compute total size" ("c2console file usage games" on the above data set):
        - 4.5 seconds normally
        - <b>2.4 seconds with size cache</b> (addObjectSize)
        - assuming hot OS file system cache */
    class InternalObjectCache : public ObjectCache {
     public:
        /** Constructor. */
        InternalObjectCache();

        /** Destructor. */
        ~InternalObjectCache();

        /** Set cache limits.
            \param maxObjects Maximum number of objects (metadata) cached
            \param maxBytes Maximum size of object data cached */
        void setLimits(size_t maxObjects, size_t maxBytes);

        // ObjectCache:
        virtual void addObject(const ObjectId& id, ObjectStore::Type type, afl::base::Ref<afl::io::FileMapping> content);
        virtual void addObjectSize(const ObjectId& id, ObjectStore::Type type, size_t size);
        virtual void removeObject(const ObjectId& id);
        virtual afl::base::Ptr<afl::io::FileMapping> getObject(const ObjectId& id, ObjectStore::Type type);
        virtual afl::base::Optional<size_t> getObjectSize(const ObjectId& id, ObjectStore::Type type);

     private:
        void trimCache();

        struct Node {
            const ObjectId m_id;
            ObjectStore::Type m_type;
            afl::base::Ptr<afl::io::FileMapping> m_content;
            size_t m_size;

            Node* m_next;
            Node** m_pThis;

            Node(const ObjectId& id, ObjectStore::Type type, afl::base::Ref<afl::io::FileMapping> content);
            Node(const ObjectId& id, ObjectStore::Type type, size_t size);
            ~Node();
            void unlink();
            void link(Node*& first);
            size_t releaseMemory();
            void checkType(ObjectStore::Type type);
        };

        typedef afl::container::PtrMap<ObjectId, Node> Map_t;
        Map_t m_data;
        Node* m_newest;

        size_t m_numObjects;
        size_t m_numBytes;
        size_t m_maxObjects;
        size_t m_maxBytes;
    };

} } }

#endif
