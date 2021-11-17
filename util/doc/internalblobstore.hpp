/**
  *  \file util/doc/internalblobstore.hpp
  *  \brief Class util::doc::InternalBlobStore
  */
#ifndef C2NG_UTIL_DOC_INTERNALBLOBSTORE_HPP
#define C2NG_UTIL_DOC_INTERNALBLOBSTORE_HPP

#include <map>
#include "util/doc/blobstore.hpp"
#include "afl/string/string.hpp"

namespace util { namespace doc {

    /** Internal blob store for testing.
        Stores blobs in memory. */
    class InternalBlobStore : public BlobStore {
     public:
        InternalBlobStore();
        ~InternalBlobStore();
        virtual ObjectId_t addObject(afl::base::ConstBytes_t data);
        virtual afl::base::Ref<afl::io::FileMapping> getObject(const ObjectId_t& id) const;

     private:
        std::map<String_t, String_t> m_content;
    };

} }

#endif
