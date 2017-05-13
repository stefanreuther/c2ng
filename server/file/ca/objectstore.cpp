/**
  *  \file server/file/ca/objectstore.cpp
  */

#include <stdexcept>
#include "server/file/ca/objectstore.hpp"
#include "afl/checksums/sha1.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/deflatetransform.hpp"
#include "afl/io/inflatetransform.hpp"
#include "afl/io/internalfilemapping.hpp"
#include "afl/string/format.hpp"
#include "afl/string/hex.hpp"
#include "server/errors.hpp"
#include "server/file/ca/commit.hpp"
#include "server/file/ca/directoryentry.hpp"
#include "server/file/ca/internalreferencecounter.hpp"
#include "server/file/ca/referencecounter.hpp"
#include "server/file/ca/objectcache.hpp"
#include "server/file/ca/internalobjectcache.hpp"

namespace {
    const char*const BAD_HASH = "500 Bad hash";
    const char*const BAD_OBJECT_TYPE = "500 Bad object type";
    const char*const BAD_OBJECT_SIZE = "500 Bad object size";
    const char*const BAD_OBJECT_CONTENT = "500 Bad object content";
    const char*const MISSING_OBJECT = "500 Missing object";
    const char*const HASH_COLLISION = "500 Hash collision";

    const char KEYWORDS[][8] = { "blob ", "tree ", "commit " };

    int hexValue(char c)
    {
        if (c >= '0' && c <= '9') {
            return c - '0';
        } else if (c >= 'a' && c <= 'f') {
            return c - 'a' + 10;
        } else {
            return -1;
        }
    }

    String_t getTailName(const server::file::ca::ObjectId& id)
    {
        String_t name;
        for (size_t i = 1; i < sizeof(server::file::ca::ObjectId); ++i) {
            afl::string::putHexByte(name, id.m_bytes[i], afl::string::HEX_DIGITS_LOWER);
        }
        return name;
    }

    size_t verifyHeader(afl::base::Bytes_t& data, const char* keyword)
    {
        // Verify keyword
        while (const char p = *keyword++) {
            uint8_t* d = data.eat();
            if (!d || *d != uint8_t(p)) {
                throw std::runtime_error(BAD_OBJECT_TYPE);
            }
        }

        // Verify size
        size_t result = 0;
        while (uint8_t* d = data.eat()) {
            if (*d == '\0') {
                // ok
                break;
            } else if (*d >= '0' && *d <= '9') {
                // size
                // Limit to about 2G. Actual limit will be enforced by front-end / c2file and be much lower.
                if (result >= (0x7FFFFFFF/10)) {
                    throw std::runtime_error(BAD_OBJECT_SIZE);
                }
                result = 10*result + (*d - '0');
            } else {
                throw std::runtime_error(BAD_OBJECT_SIZE);
            }
        }

        return result;
    }

    void transformAdd(afl::base::GrowableMemory<uint8_t>& out, afl::io::Transform& tx, afl::base::ConstBytes_t in)
    {
        while (!in.empty()) {
            uint8_t outBuffer[4096];
            afl::base::Bytes_t outContent(outBuffer);
            tx.transform(in, outContent);
            out.append(outContent);
        }
    }

    void transformFinish(afl::base::GrowableMemory<uint8_t>& out, afl::io::Transform& tx)
    {
        afl::base::ConstBytes_t in;
        tx.flush();
        while (1) {
            uint8_t outBuffer[4096];
            afl::base::Bytes_t outContent(outBuffer);
            tx.transform(in, outContent);
            out.append(outContent);
            if (outContent.empty()) {
                break;
            }
        }
    }
}


// Constructor.
server::file::ca::ObjectStore::ObjectStore(DirectoryHandler& dir)
    : m_directory(dir),
      m_subdirectories(),
      m_refCounter(new InternalReferenceCounter()),
      m_cache(new InternalObjectCache())
{
    readDirectory();
}

// Destructor.
server::file::ca::ObjectStore::~ObjectStore()
{ }

// Get object content.
afl::base::Ref<afl::io::FileMapping>
server::file::ca::ObjectStore::getObject(const ObjectId& id, Type expectedType)
{
    afl::base::Ptr<afl::io::FileMapping> result;
    if (!loadObject(id, expectedType, 0, &result) || result.get() == 0) {
        throw afl::except::FileProblemException(id.toHex(), MISSING_OBJECT);
    }
    return *result;
}

// Get object size.
size_t
server::file::ca::ObjectStore::getObjectSize(const ObjectId& id, Type expectedType)
{
    size_t n = 0;
    if (!loadObject(id, expectedType, &n, 0)) {
        throw afl::except::FileProblemException(id.toHex(), MISSING_OBJECT);
    }
    return n;
}

// Add an object.
server::file::ca::ObjectId
server::file::ca::ObjectStore::addObject(Type type, afl::base::ConstBytes_t data)
{
    // Although we accept ObjectId::nil to refer to a zero-size object,
    // we do not optimize creation of zero-size objects this way
    // (it doesn't save much, and git does not like it).

    // Compute object Id
    const String_t prefix = afl::string::Format("%s%d", KEYWORDS[type], data.size());
    static const uint8_t ZERO[1] = {0};
    afl::checksums::SHA1 checksummer;
    checksummer.add(afl::string::toBytes(prefix));
    checksummer.add(ZERO);
    checksummer.add(data);

    const ObjectId id = ObjectId::fromHash(checksummer);

    // Verify object content
    afl::base::Ptr<afl::io::FileMapping> existingContent;
    if (loadObject(id, type, 0, &existingContent) && existingContent.get() != 0) {
        // Check hash collision
        if (!existingContent->get().equalContent(data)) {
            throw afl::except::FileProblemException(id.toHex(), HASH_COLLISION);
        }

        // User assumed this is a new object and allocated reference counts.
        // Undo that as we're not actually creating an object...
        unlinkContent(type, data);

        // ...but increase our reference counter
        int32_t tmp;
        m_refCounter->modify(id, +1, tmp);
    } else {
        // Object does not exist, create it
        const uint8_t firstChar = id.m_bytes[0];
        if (!m_subdirectories[firstChar]) {
            String_t name;
            afl::string::putHexByte(name, firstChar, afl::string::HEX_DIGITS_LOWER);
            DirectoryHandler::Info info = m_directory.createDirectory(name);
            m_subdirectories.replaceElementNew(firstChar, m_directory.getDirectory(info));
        }

        // Compression
        afl::base::GrowableMemory<uint8_t> newContent;
        afl::io::DeflateTransform tx(afl::io::DeflateTransform::Zlib);

        transformAdd(newContent, tx, afl::string::toBytes(prefix));
        transformAdd(newContent, tx, ZERO);
        transformAdd(newContent, tx, data);
        transformFinish(newContent, tx);

        // Create file
        m_subdirectories[firstChar]->createFile(getTailName(id), newContent);

        // Set initial reference counter
        m_refCounter->set(id, 1);

        // Cache it (cache the original data!)
        afl::base::GrowableMemory<uint8_t> originalContent;
        originalContent.append(data);
        m_cache->addObject(id, type, *new afl::io::InternalFileMapping(originalContent));
    }

    return id;
}

// Link an object.
void
server::file::ca::ObjectStore::linkObject(const ObjectId& id)
{
    if (id != ObjectId::nil) {
        int32_t tmp;
        m_refCounter->modify(id, +1, tmp);
    }
}

// Unlink an object.
void
server::file::ca::ObjectStore::unlinkObject(Type type, const ObjectId& id)
{
    if (id != ObjectId::nil) {
        int32_t result;
        if (m_refCounter->modify(id, -1, result) && result == 0) {
            // Reference count is zero; we can delete this object.
            // First, remove embedded references.
            if (type != DataObject) {
                unlinkContent(type, getObject(id, type)->get());
            }

            // Remove the file
            const uint8_t firstChar = id.m_bytes[0];
            if (firstChar < m_subdirectories.size() && m_subdirectories[firstChar] != 0) {
                m_subdirectories[firstChar]->removeFile(getTailName(id));
            }

            // Remove from cache
            m_cache->removeObject(id);
        }
    }
}

/** Load an object, internal.
    \param id Object Id
    \param expectedType Expected type
    \param pSize [optional,out] Object size
    \param pContent [optional,out] Object content
    \retval true Object loaded successfully
    \retval false Object does not exist
    \throw afl::except::FileProblemException Object is damaged */
bool
server::file::ca::ObjectStore::loadObject(const ObjectId& id, Type expectedType, size_t* pSize, afl::base::Ptr<afl::io::FileMapping>* pContent)
{
    const uint8_t firstChar = id.m_bytes[0];
    if (id == ObjectId::nil) {
        // Null matches anything
        if (pSize != 0) {
            *pSize = 0;
        }
        if (pContent != 0) {
            // This will allocate a new pseudo file mapping for each instance, which is ok since it happens very rarely.
            // Newly-created objects will not have the nil identifier.
            afl::base::GrowableMemory<uint8_t> mem;
            *pContent = new afl::io::InternalFileMapping(mem);
        }
        return true;
    } else if (pSize != 0 && pContent == 0 && m_cache->getObjectSize(id, expectedType).get(*pSize)) {
        // Satisified size request from cache
        return true;
    } else if (pContent != 0 && (*pContent = m_cache->getObject(id, expectedType)).get() != 0) {
        // Satisfied data request from cache
        if (pSize != 0) {
            *pSize = (*pContent)->get().size();
        }
        return true;
    } else if (firstChar >= m_subdirectories.size() || m_subdirectories[firstChar] == 0) {
        // Directory does not exist
        return false;
    } else {
        // Directory does exist. Open as file.
        afl::base::Ptr<afl::io::FileMapping> compressedMapping;
        try {
            compressedMapping = m_subdirectories[firstChar]->getFileByName(getTailName(id)).asPtr();
        }
        catch (std::exception&) {
            // File open failed, assume item not present
            return false;
        }
        afl::base::ConstBytes_t compressedContent(compressedMapping->get());

        // Decompress header.
        // Start by decompressing the first page. However, if content is not requested, it's enough to decode a few bytes just to see type + size.
        uint8_t uncompressedBuffer[4096];
        afl::base::Bytes_t uncompressedContent(uncompressedBuffer);
        if (pContent == 0) {
            uncompressedContent.trim(100);
        }

        afl::io::InflateTransform tx(afl::io::InflateTransform::Zlib);
        tx.transform(compressedContent, uncompressedContent);

        // Check header
        size_t size = verifyHeader(uncompressedContent, KEYWORDS[expectedType]);
        if (pSize != 0) {
            *pSize = size;
        }

        // Read content if requested
        if (pContent != 0) {
            afl::base::GrowableBytes_t mem;
            mem.reserve(size);
            mem.append(uncompressedContent);
            while (!compressedContent.empty()) {
                uncompressedContent = uncompressedBuffer;
                tx.transform(compressedContent, uncompressedContent);
                mem.append(uncompressedContent);
            }

            tx.flush();
            while (1) {
                uncompressedContent = uncompressedBuffer;
                tx.transform(compressedContent, uncompressedContent);
                mem.append(uncompressedContent);
                if (compressedContent.empty()) {
                    break;
                }
            }

            if (size != mem.size()) {
                throw afl::except::FileProblemException(id.toHex(), BAD_OBJECT_CONTENT);
            }

            // Cache result
            afl::base::Ref<afl::io::FileMapping> map(*new afl::io::InternalFileMapping(mem));
            m_cache->addObject(id, expectedType, map);

            // Build result
            *pContent = map.asPtr();
        } else {
            // Cache the size
            m_cache->addObjectSize(id, expectedType, size);
        }

        // Success
        return true;
    }
}

/** Read directory.
    Initially populates the m_subdirectories member. */
void
server::file::ca::ObjectStore::readDirectory()
{
    class Callback : public DirectoryHandler::Callback {
     public:
        Callback(ObjectStore& parent)
            : m_parent(parent)
            { }
        virtual void addItem(const DirectoryHandler::Info& info)
            {
                if (info.name.size() == 2 && info.type == DirectoryHandler::IsDirectory) {
                    int a = hexValue(info.name[0]);
                    int b = hexValue(info.name[1]);
                    if (a >= 0 && b >= 0) {
                        m_parent.m_subdirectories.replaceElementNew(16*a+b, m_parent.m_directory.getDirectory(info));
                    }
                }
            }
     private:
        ObjectStore& m_parent;
    };
    Callback cb(*this);
    m_subdirectories.resize(256);
    m_directory.readContent(cb);
}

/** Unlink an object's content.
    Call before removing the object.
    \param type Object type
    \param data Payload */
void
server::file::ca::ObjectStore::unlinkContent(Type type, afl::base::ConstBytes_t data)
{
    switch (type) {
     case DataObject:
        // No embedded links; nothing to do.
        break;

     case TreeObject: {
        // Unlink all referenced sub-objects.
        DirectoryEntry e;
        while (e.parse(data)) {
            switch (e.getType()) {
             case DirectoryHandler::IsFile:
             case DirectoryHandler::IsUnknown:
                unlinkObject(DataObject, e.getId());
                break;

             case DirectoryHandler::IsDirectory:
                unlinkObject(TreeObject, e.getId());
                break;
            }
        }
        break;
     }

     case CommitObject: {
        // Unlink contained tree.
        Commit c;
        if (c.parse(data)) {
            unlinkObject(TreeObject, c.getTreeId());
        }
        break;
     }
    }
}
