/**
  *  \file server/doc/root.hpp
  *  \brief Class server::doc::Root
  */
#ifndef C2NG_SERVER_DOC_ROOT_HPP
#define C2NG_SERVER_DOC_ROOT_HPP

#include "util/doc/blobstore.hpp"
#include "util/doc/index.hpp"

namespace server { namespace doc {

    /** Documentation server global state.
        Global state includes:
        - a BlobStore
        - an Index */
    class Root {
     public:
        /** Constructor.
            @param blobStore BlobStore to serve */
        explicit Root(util::doc::BlobStore& blobStore)
            : m_blobStore(blobStore),
              m_index()
            { }

        /** Access index.
            @return index */
        util::doc::Index& index()
            { return m_index; }

        /** Access index (const version).
            @return index */
        const util::doc::Index& index() const
            { return m_index; }

        /** Access BlobStore.
            @return BlobStore */
        const util::doc::BlobStore& blobStore() const
            { return m_blobStore; }

     private:
        util::doc::BlobStore& m_blobStore;
        util::doc::Index m_index;
    };

} }

#endif
