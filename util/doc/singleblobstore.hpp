/**
  *  \file util/doc/singleblobstore.hpp
  *  \brief Class util::doc::SingleBlobStore
  */
#ifndef C2NG_UTIL_DOC_SINGLEBLOBSTORE_HPP
#define C2NG_UTIL_DOC_SINGLEBLOBSTORE_HPP

#include <map>
#include "afl/io/stream.hpp"
#include "util/doc/blobstore.hpp"

namespace util { namespace doc {

    /** Blob store using a single archive file.
        All blobs are stored in a single TAR file.
        Every blob is stored in a member whose name is derived from its content.

        A blob with SHA1 "da39a3ee5e6b4b0d3255bfef95601890afd80709" is stored
        in a member "da/39a3ee5e6b4b0d3255bfef95601890afd80709".
        When unpacked, this produces the same format as FileBlobStore produces.

        Rationale:

        Using a single file conserves resources and simplifies deployment
        (need to deploy just the structure XML file and the content TAR file).
        TAR has been chosen because it is reasonably simple and offers enough space for file names
        (AR only has 16-character file names which is not enough for a SHA1).

        Making it pack/unpack compatible with FileBlobStore is a plus in debuggability:
        you can un-tar a file written by SingleBlobStore, or tar a directory written by FileBlobStore,
        to use it with the respective other.

        On the downside, we need to read the entire file on startup.

        An alternative would have been to make a custom file format, possibly with a separate index file.
        This could have been a little more efficient, especially when constructing the file
        using repeated c2docmanager calls, but that is not an important usecase, performance-wise. */
    class SingleBlobStore : public BlobStore {
     public:
        /** Constructor.
            @param file File */
        explicit SingleBlobStore(afl::base::Ref<afl::io::Stream> file);
        ~SingleBlobStore();

        virtual ObjectId_t addObject(afl::base::ConstBytes_t data);
        virtual afl::base::Ref<afl::io::FileMapping> getObject(const ObjectId_t& id) const;

     private:
        /*
         *  File
         */
        afl::base::Ref<afl::io::Stream> m_file;

        /*
         *  File Structure
         */
        struct UstarHeader;

        /*
         *  Index
         *
         *  Given 20 bytes SHA-1 + 4 bytes length + 8 bytes FileSize_t, a node is exactly 32 bytes.
         */
        static const size_t KEY_SIZE = 20;
        struct Key {
            uint8_t data[KEY_SIZE];
            bool operator<(const Key& other) const;
            bool operator==(const Key& other) const;
        };
        struct Address {
            uint32_t length;
            afl::io::Stream::FileSize_t pos;
            Address(afl::io::Stream::FileSize_t pos, uint32_t length)
                : length(length), pos(pos)
                { }
        };
        std::map<Key, Address> m_index;

        afl::io::Stream::FileSize_t m_endPos;


        void readFile();
        static bool parseObjectId(const ObjectId_t& objId, Key& key);
        static bool parseMemberName(String_t name, Key& key);
        static ObjectId_t formatObjectId(const Key& k);
    };

} }


#endif
