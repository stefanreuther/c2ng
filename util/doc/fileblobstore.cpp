/**
  *  \file util/doc/fileblobstore.cpp
  *  \brief Class util::doc::FileBlobStore
  */

#include <cassert>
#include "util/doc/fileblobstore.hpp"
#include "afl/checksums/sha1.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/string/hex.hpp"
#include "afl/string/messages.hpp"

namespace {
    typedef afl::checksums::SHA1 Hash_t;

    using afl::base::Ptr;
    using afl::base::Ref;
    using afl::except::FileProblemException;
    using afl::io::Directory;
    using afl::io::DirectoryEntry;
    using afl::io::FileSystem;
    using afl::io::Stream;
    using afl::string::Messages;

    bool isValidHash(const String_t& id)
    {
        // Check size
        if (id.size() != Hash_t::HASH_SIZE*2) {
            return false;
        }

        // Check characters. Must be HEX_DIGITS_LOWER.
        for (size_t i = 0; i < id.size(); ++i) {
            char ch = id[i];
            int val = afl::string::getHexDigitValue(ch);
            if (val < 0 || ch != afl::string::HEX_DIGITS_LOWER[val]) {
                return false;
            }
        }
        return true;
    }

    String_t getDirectoryNamePart(const String_t& id)
    {
        return id.substr(0, 2);
    }

    String_t getFileNamePart(const String_t& id)
    {
        return id.substr(2);
    }
}

util::doc::FileBlobStore::FileBlobStore(afl::base::Ref<afl::io::Directory> dir)
    : m_directory(dir)
{ }

util::doc::FileBlobStore::~FileBlobStore()
{ }

util::doc::BlobStore::ObjectId_t
util::doc::FileBlobStore::addObject(afl::base::ConstBytes_t data)
{
    // Compute object Id.
    Hash_t hasher;
    hasher.add(data);
    const ObjectId_t id = hasher.getHashAsHexString();
    assert(isValidHash(id));

    // Create containing directory if needed
    Ref<DirectoryEntry> e = m_directory->getDirectoryEntryByName(getDirectoryNamePart(id));
    if (e->getFileType() != DirectoryEntry::tDirectory) {
        e->createAsDirectory();
    }
    Ref<Directory> dir = e->openDirectory();

    // Check existing blob
    Ptr<Stream> existingBlob = dir->openFileNT(getFileNamePart(id), FileSystem::OpenRead);
    if (existingBlob.get() == 0) {
        // It does not exist; create it
        dir->openFile(getFileNamePart(id), FileSystem::Create)
            ->fullWrite(data);
    } else {
        // It already exists; check for collision
        if (!data.equalContent(existingBlob->createVirtualMapping()->get())) {
            throw FileProblemException(id, "Hashing collision or corrupted data");
        }
    }
    return id;
}

afl::base::Ref<afl::io::FileMapping>
util::doc::FileBlobStore::getObject(const ObjectId_t& id) const
{
    // Check syntax
    if (!isValidHash(id)) {
        throw FileProblemException(id, Messages::fileNotFound());
    }

    // Produce mapping. Any step in this sequence may throw which is what we want.
    return m_directory
        ->openDirectory(getDirectoryNamePart(id))
        ->openFile(getFileNamePart(id), FileSystem::OpenRead)
        ->createVirtualMapping();
}
