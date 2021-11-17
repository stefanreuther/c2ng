/**
  *  \file util/doc/fileblobstore.hpp
  *  \brief Class util::doc::FileBlobStore
  */
#ifndef C2NG_UTIL_DOC_FILEBLOBSTORE_HPP
#define C2NG_UTIL_DOC_FILEBLOBSTORE_HPP

#include "afl/io/directory.hpp"
#include "util/doc/blobstore.hpp"

namespace util { namespace doc {

    /** Blob store using content-addressable files.
        Every blob is stored in a file whose name is derived from its content.

        A blob with SHA1 "da39a3ee5e6b4b0d3255bfef95601890afd80709" is stored
        in file "da/39a3ee5e6b4b0d3255bfef95601890afd80709".

        This format is similar, but not identical nor compatible with the one used in git
        and server::file::ca::ObjectStore; we do not compress and do not add metadata tags. */
    class FileBlobStore : public BlobStore {
     public:
        /** Constructor.
            @param dir Directory */
        explicit FileBlobStore(afl::base::Ref<afl::io::Directory> dir);
        ~FileBlobStore();

        virtual ObjectId_t addObject(afl::base::ConstBytes_t data);
        virtual afl::base::Ref<afl::io::FileMapping> getObject(const ObjectId_t& id) const;

     private:
        afl::base::Ref<afl::io::Directory> m_directory;
    };

} }

#endif
