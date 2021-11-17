/**
  *  \file u/t_util_doc_fileblobstore.cpp
  *  \brief Test for util::doc::FileBlobStore
  */

#include "util/doc/fileblobstore.hpp"

#include "t_util_doc.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/string/format.hpp"

using afl::base::Enumerator;
using afl::base::Ptr;
using afl::base::Ref;
using afl::except::FileProblemException;
using afl::io::Directory;
using afl::io::DirectoryEntry;
using afl::io::FileSystem;
using afl::string::Format;
using util::doc::BlobStore;
using util::doc::FileBlobStore;

namespace {
    class TemporaryDirectory {
     public:
        TemporaryDirectory(FileSystem& fs)
            : m_dirEntry(createWorkDirectory(fs))
            { }

        ~TemporaryDirectory()
            {
                removeDirectoryContent(m_dirEntry);
                m_dirEntry->erase();
            }

        const Ref<DirectoryEntry>& get()
            { return m_dirEntry; }

     private:
        Ref<DirectoryEntry> m_dirEntry;

        static Ref<DirectoryEntry> createWorkDirectory(FileSystem& fs)
            {
                Ref<Directory> currentDirectory = fs.openDirectory(fs.getWorkingDirectoryName());
                int i = 0;
                while (1) {
                    try {
                        String_t name = Format("__test%d", ++i);
                        Ref<DirectoryEntry> entry = currentDirectory->getDirectoryEntryByName(name);
                        entry->createAsDirectory();
                        return entry;
                    }
                    catch (std::exception&) {
                        if (i > 1000) {
                            throw;
                        }
                    }
                }
            }

        static void removeDirectoryContent(Ref<DirectoryEntry> dir)
            {
                std::vector<Ptr<DirectoryEntry> > stuff;

                // Read everything so we don't delete and iterate at the same time
                Ref<Enumerator<Ptr<DirectoryEntry> > > it = dir->openDirectory()->getDirectoryEntries();
                Ptr<DirectoryEntry> e;
                while (it->getNextElement(e)) {
                    stuff.push_back(e);
                }

                // Remove everything
                for (size_t i = 0; i < stuff.size(); ++i) {
                    if (stuff[i]->getFileType() == DirectoryEntry::tDirectory) {
                        removeDirectoryContent(*stuff[i]);
                    }
                    stuff[i]->erase();
                }
            }
    };
}

/** Basic test case.
    A: create a FileBlobStore. Store data.
    E: storing the same data produces same object Id, different data produces different Id, retrieving nonexistant Id fails. */
void
TestUtilDocFileBlobStore::testIt()
{
    TemporaryDirectory dir(FileSystem::getInstance());
    FileBlobStore testee(dir.get()->openDirectory());

    // Store an object and retrieve it again
    BlobStore::ObjectId_t objId = testee.addObject(afl::string::toBytes("hello there"));
    String_t objContent = afl::string::fromBytes(testee.getObject(objId)->get());
    TS_ASSERT_EQUALS(objContent, "hello there");

    // Store the same object, must produce same Id
    BlobStore::ObjectId_t objId2 = testee.addObject(afl::string::toBytes("hello there"));
    TS_ASSERT_EQUALS(objId, objId2);

    // Store a different object, must produce different Id
    BlobStore::ObjectId_t objId3 = testee.addObject(afl::string::toBytes("1337"));
    TS_ASSERT_DIFFERS(objId, objId3);

    // Retrieving an invented Id must fail
    TS_ASSERT_THROWS(testee.getObject(objId + objId3), FileProblemException);
    TS_ASSERT_THROWS(testee.getObject(""), FileProblemException);
}

/** Test portability between instances.
    A: create a FileBlobStore and store data. Retrieve that data using a new instance.
    E: data retrieved correctly. */
void
TestUtilDocFileBlobStore::testPortability()
{
    TemporaryDirectory dir(FileSystem::getInstance());
    BlobStore::ObjectId_t objId;

    {
        FileBlobStore testee(dir.get()->openDirectory());
        objId = testee.addObject(afl::string::toBytes("hello there"));
    }
    {
        FileBlobStore testee(dir.get()->openDirectory());
        String_t objContent = afl::string::fromBytes(testee.getObject(objId)->get());
        TS_ASSERT_EQUALS(objContent, "hello there");
    }
}

