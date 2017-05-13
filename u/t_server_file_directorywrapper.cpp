/**
  *  \file u/t_server_file_directorywrapper.cpp
  *  \brief Test for server::file::DirectoryWrapper
  */

#include "server/file/directorywrapper.hpp"

#include "t_server_file.hpp"
#include "server/file/directoryitem.hpp"
#include "server/file/internaldirectoryhandler.hpp"
#include "afl/io/internaldirectory.hpp"
#include "server/file/root.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/directoryentry.hpp"

/** Test DirectoryWrapper. */
void
TestServerFileDirectoryWrapper::testIt()
{
    using server::file::InternalDirectoryHandler;

    // Test setup
    InternalDirectoryHandler::Directory content("");
    content.files.pushBackNew(new InternalDirectoryHandler::File("f1"))->content.append(afl::string::toBytes("content1"));
    content.files.pushBackNew(new InternalDirectoryHandler::File("f3"))->content.append(afl::string::toBytes("content3"));
    content.files.pushBackNew(new InternalDirectoryHandler::File("f2"))->content.append(afl::string::toBytes("content2"));
    content.subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("sub"));

    server::file::DirectoryItem item("itemName", 0, std::auto_ptr<server::file::DirectoryHandler>(new InternalDirectoryHandler("dirName", content)));
    server::file::Root root(item, afl::io::InternalDirectory::create("spec"));
    item.readContent(root);

    // Testee
    afl::base::Ref<server::file::DirectoryWrapper> testee(server::file::DirectoryWrapper::create(item));

    // Metadata
    TS_ASSERT_EQUALS(testee->getTitle(), "itemName");
    TS_ASSERT_EQUALS(testee->getDirectoryName(), "");
    TS_ASSERT(testee->getParentDirectory().get() == 0);

    // File access
    {
        // Open a file
        afl::base::Ref<afl::io::Stream> file = testee->openFile("f1", afl::io::FileSystem::OpenRead);
        TS_ASSERT_EQUALS(file->getSize(), 8U);
        uint8_t bytes[8];
        TS_ASSERT_THROWS_NOTHING(file->fullRead(bytes));
        TS_ASSERT_SAME_DATA(bytes, "content1", 8);

        // Open same file again. Check that we can read it again (=no false sharing)
        afl::base::Ref<afl::io::Stream> file2 = testee->openFile("f1", afl::io::FileSystem::OpenRead);
        TS_ASSERT_EQUALS(file2->getSize(), 8U);
        TS_ASSERT_THROWS_NOTHING(file2->fullRead(bytes));
    }

    // Some invalid file accesses
    TS_ASSERT_THROWS(testee->openFile("sub", afl::io::FileSystem::OpenRead),  afl::except::FileProblemException);
    TS_ASSERT_THROWS(testee->openFile("f4",  afl::io::FileSystem::OpenRead),  afl::except::FileProblemException);
    TS_ASSERT_THROWS(testee->openFile("f1",  afl::io::FileSystem::OpenWrite), afl::except::FileProblemException);
    TS_ASSERT_THROWS(testee->openFile("f1",  afl::io::FileSystem::Create),    afl::except::FileProblemException);
    TS_ASSERT_THROWS(testee->openFile("fn",  afl::io::FileSystem::Create),    afl::except::FileProblemException);

    // Subdirectory access (not allowed)
    TS_ASSERT_THROWS(testee->openDirectory("sub"), afl::except::FileProblemException);

    // Modification
    TS_ASSERT_THROWS(testee->erase("f1"), afl::except::FileProblemException);
}

/** Test operations on Enum. */
void
TestServerFileDirectoryWrapper::testEnum()
{
    using server::file::InternalDirectoryHandler;

    // Test setup
    InternalDirectoryHandler::Directory content("");
    content.files.pushBackNew(new InternalDirectoryHandler::File("f1"))->content.append(afl::string::toBytes("content1"));
    content.files.pushBackNew(new InternalDirectoryHandler::File("f3"))->content.append(afl::string::toBytes("content3"));
    content.files.pushBackNew(new InternalDirectoryHandler::File("f2"))->content.append(afl::string::toBytes("content2"));
    content.subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("sub"));

    server::file::DirectoryItem item("itemName", 0, std::auto_ptr<server::file::DirectoryHandler>(new InternalDirectoryHandler("dirName", content)));
    server::file::Root root(item, afl::io::InternalDirectory::create("spec"));
    item.readContent(root);

    // Testee
    afl::base::Ref<server::file::DirectoryWrapper> testee(server::file::DirectoryWrapper::create(item));

    // Iteration
    afl::base::Ref<afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > > e = testee->getDirectoryEntries();
    bool f1 = false, f2 = false, f3 = false;

    afl::base::Ptr<afl::io::DirectoryEntry> i;
    while (e->getNextElement(i)) {
        TS_ASSERT(i.get() != 0);
        if (i->getTitle() == "f1") {
            TS_ASSERT(!f1);
            TS_ASSERT_EQUALS(i->getFileType(), i->tFile);
            TS_ASSERT_EQUALS(i->getFileSize(), 8U);
            f1 = true;
        } else if (i->getTitle() == "f2") {
            TS_ASSERT(!f2);
            TS_ASSERT_EQUALS(i->getFileType(), i->tFile);
            TS_ASSERT_EQUALS(i->getFileSize(), 8U);
            f2 = true;
        } else if (i->getTitle() == "f3") {
            TS_ASSERT(!f3);
            TS_ASSERT_EQUALS(i->getFileType(), i->tFile);
            TS_ASSERT_EQUALS(i->getFileSize(), 8U);
            f3 = true;
        } else {
            TS_ASSERT(false);
        }
    }
}

/** Test operations on DirectoryEntry. */
void
TestServerFileDirectoryWrapper::testEntry()
{
    using server::file::InternalDirectoryHandler;

    // Test setup
    InternalDirectoryHandler::Directory content("");
    content.files.pushBackNew(new InternalDirectoryHandler::File("f1"))->content.append(afl::string::toBytes("content1"));
    content.files.pushBackNew(new InternalDirectoryHandler::File("f3"))->content.append(afl::string::toBytes("content3"));
    content.files.pushBackNew(new InternalDirectoryHandler::File("f2"))->content.append(afl::string::toBytes("content2"));
    content.subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("sub"));

    server::file::DirectoryItem item("itemName", 0, std::auto_ptr<server::file::DirectoryHandler>(new InternalDirectoryHandler("dirName", content)));
    server::file::Root root(item, afl::io::InternalDirectory::create("spec"));
    item.readContent(root);

    // Testee
    afl::base::Ref<server::file::DirectoryWrapper> testee(server::file::DirectoryWrapper::create(item));

    // Same things, using directory entry
    afl::base::Ref<afl::io::DirectoryEntry> e(testee->getDirectoryEntryByName("f1"));
    TS_ASSERT_EQUALS(e->getFileType(), e->tFile);
    TS_ASSERT_EQUALS(e->getFileSize(), 8U);
    TS_ASSERT(e->getFlags().empty());
    TS_ASSERT_EQUALS(e->getTitle(), "f1");
    TS_ASSERT_EQUALS(e->getPathName(), "");
    TS_ASSERT_EQUALS(&*e->openContainingDirectory(), &*testee);
    TS_ASSERT_THROWS(e->openDirectory(),          afl::except::FileProblemException);
    TS_ASSERT_THROWS(e->renameTo("f1new"),        afl::except::FileProblemException);
    TS_ASSERT_THROWS(e->erase(),                  afl::except::FileProblemException);
    TS_ASSERT_THROWS(e->createAsDirectory(),      afl::except::FileProblemException);
    TS_ASSERT_THROWS(e->setFlag(e->Hidden, true), afl::except::FileProblemException);
}

