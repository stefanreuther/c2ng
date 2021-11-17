/**
  *  \file util/doc/blobstore.hpp
  *  \brief Interface util::doc::BlobStore
  */
#ifndef C2NG_UTIL_DOC_BLOBSTORE_HPP
#define C2NG_UTIL_DOC_BLOBSTORE_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/ref.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/string/string.hpp"

namespace util { namespace doc {

    /** Interface for a blob store.

        Supported features:
        - add a blob with possible de-duplication
        - retrieve a blob by Id

        Deliberately unsupported:
        - modification of a blob
        - reference counting/removal of blobs

        When building a documentation set, pieces are added to the BlobStore.
        If documentation is modified, the BlobStore is regenerated from original input.
        Incremental updates are not required.

        @see server::file::ca::ObjectStore */
    class BlobStore : public afl::base::Deletable {
     public:
        /** Shortcut for an object Id.
            An object Id is a non-empty sequence of alphanumeric, case-sensitive characters.
            The actual meaning depends on the implementation. */
        typedef String_t ObjectId_t;

        /** Add an object.
            It is an error if the object already exists but has different content (hash collision).

            @param data Object data
            @return Object Id */
        virtual ObjectId_t addObject(afl::base::ConstBytes_t data) = 0;

        /** Get an object.
            It is an error if the object does not exist.

            @param id Object Id
            @return FileMapping representing object's content

            @throw afl::except::FileProblemException on errors */
        virtual afl::base::Ref<afl::io::FileMapping> getObject(const ObjectId_t& id) const = 0;
    };

} }

#endif
