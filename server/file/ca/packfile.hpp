/**
  *  \file server/file/ca/packfile.hpp
  *  \brief Class server::file::ca::PackFile
  */
#ifndef C2NG_SERVER_FILE_CA_PACKFILE_HPP
#define C2NG_SERVER_FILE_CA_PACKFILE_HPP

#include "afl/base/growablememory.hpp"
#include "afl/bits/uint32be.hpp"
#include "afl/bits/value.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/stream.hpp"
#include "server/file/ca/indexfile.hpp"
#include "server/file/ca/objectid.hpp"

namespace server { namespace file { namespace ca {

    /** Packfile object store.
        A packfile consists of the actual packfile (*.pack), and an index file (*.idx).
        This class represents such a pair and allows retrieval of objects from it.

        Because packfiles can be large, we cannot use the DirectoryHandler abstraction here
        which requires files to be loaded/mapped permanently.
        Instead, we rely on the Directory abstraction. */
    class PackFile {
     public:
        /*
         *  File Format Definitions
         *
         *  UInt32        magic       (='PACK')
         *  UInt32        version     (=2)
         *  UInt32        numObjects  (=number of objects)
         *
         *  Each object starts with
         *     VarInt     typeAndSize
         *  giving the type (OBJ_xxx) and the size of the decompressed object.
         *  Type is bits 4..6 of the value, size all the others.
         *  For OBJ_OFS_DELTA, this is followed by
         *     OfsInt     bytesToSeekBack (=offset to go back from typeAndSize field)
         *  For OBJ_REF_DELTA, this is followed by
         *     ObjectId   refObject (=SHA1, can be in this pack or another)
         *
         *  After that, the object content follows as a zlib stream.
         *  For non-delta objects, that's the immediate object content.
         *  For delta objects, the zlib stream is decoded using DeltaExpander;
         *  for format, see there.
         */

        typedef afl::bits::Value<afl::bits::UInt32BE> UInt32_t;

        struct Header {
            UInt32_t magic;
            UInt32_t version;
            UInt32_t numObjects;
        };

        static const uint32_t MAGIC = 0x5041434B;
        static const uint32_t VERSION = 2;

        static const uint8_t OBJ_COMMIT    = 1;
        static const uint8_t OBJ_TREE      = 2;
        static const uint8_t OBJ_BLOB      = 3;
        static const uint8_t OBJ_TAG       = 4;
        static const uint8_t OBJ_OFS_DELTA = 6;
        static const uint8_t OBJ_REF_DELTA = 7;

        /** Interface to request referenced objects. */
        class ObjectRequester {
         public:
            /** Virtual destructor. */
            virtual ~ObjectRequester()
                { }
            /** Get object.
                @param id        Object Id
                @param maxLevel  Maximum level of indirections to resolve.
                                 If the implementation calls PackFile::getObject(), it shall pass this maxLevel.
                @return Object
                @throw FileProblemException if object cannot be found */
            virtual afl::base::Ref<afl::io::FileMapping> getObject(const ObjectId& id, size_t maxLevel) = 0;
        };


        /*
         *  Operations
         */

        /** Constructor.
            @apram dir       Directory
            @param baseName  Base name (will be extended with ".pack" and ".idx") */
        PackFile(afl::io::Directory& dir, String_t baseName);

        /** Destructor. */
        ~PackFile();

        /** Get an object.
            @param id        Object Id
            @param req       ObjectRequester to resolve OBJ_REF_DELTA objects
            @param maxLevel  Maximum number of indirections to resolve. 0 means references are rejected.
            @return FileMapping if object was found; null otherwise */
        afl::base::Ptr<afl::io::FileMapping> getObject(const ObjectId& id, ObjectRequester& req, size_t maxLevel);

        /*
         *  File Format Utilities
         */

        struct VarInt;
        struct OfsInt;
        class DeltaExpander;

     private:
        IndexFile m_index;
        afl::base::Ref<afl::io::Stream> m_file;

        afl::base::Ref<afl::io::FileMapping> loadObject(afl::io::Stream::FileSize_t pos, ObjectRequester& req, size_t maxLevel);
    };

    /** Parser for "varint" value.
        This class is primarly exposed for testing purposes. */
    struct server::file::ca::PackFile::VarInt {
        uint64_t value;
        int shift;

        VarInt()
            : value(), shift()
            { }
        bool acceptByte(uint8_t byte);
    };

    /** Parser for "offset" value.
        This class is primarly exposed for testing purposes. */
    struct server::file::ca::PackFile::OfsInt {
        uint64_t value;

        OfsInt()
            : value()
            { }
        bool acceptByte(uint8_t byte);
    };

    /** Expander for "delta" encoding.
        Usage:
        - resolve reference object and create DeltaExpander
        - obtain data and push into this object using acceptBytes() until it returns null

        This class is primarly exposed for testing purposes.

        The stream consists of:
        - VarInt (reference object size)
        - VarInt (result object size)
        - Repeated instructions to build target object.

        Instructions are:
        - 0xxxxxxx(binary): followed by xxxxxxx bytes; add these bytes verbatim.
        - 1xxxxxxx(binary): followed by up to 7 parameter bytes where each set bit determines
          which parameters are present (LSB=first parameter, etc.).
          First four parameters give a little-endian 32-bit index into the reference object.
          Next three parameters give a little-endian 24-bit number of bytes to copy, 0 means copy 64k.
          Note that git only supports 16 bits here at some places. */
    class server::file::ca::PackFile::DeltaExpander {
     public:
        /** Constructor.
            @param fileName   File name for exceptions
            @param refObject  Reference object
            @param result     Room for result; must live as long as DeltaExpander object */
        DeltaExpander(const String_t& fileName, afl::base::Ref<afl::io::FileMapping> refObject, afl::base::GrowableBytes_t& result);

        /** Push data.
            @param mem Data
            @retval true  Result object has been completed
            @retval false Need more data */
        bool acceptBytes(afl::base::ConstBytes_t mem);

     private:
        enum State {
            ReadRefObjectSize,      ///< Complete m_refObjectSize.
            ReadResultObjectSize,   ///< Complete m_resultObjectSize.
            ReadOpcode,             ///< Read opcode and decide next state.
            CopyData,               ///< Read m_copyParameters; execute copying.
            AddData,                ///< Add m_toAdd bytes to output.
            End                     ///< Final state.
        };

        // Constructor parameters
        const String_t m_fileName;
        const afl::base::Ref<afl::io::FileMapping> m_refObject;
        afl::base::GrowableBytes_t& m_result;

        // State machine parameters
        VarInt m_refObjectSize;
        VarInt m_resultObjectSize;
        State m_state;
        uint8_t m_opcode;
        uint8_t m_copyParameters[7];
        int m_copyIndex;
        size_t m_toAdd;
    };

} } }

#endif
