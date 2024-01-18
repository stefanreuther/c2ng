/**
  *  \file test/server/file/directorywrappertest.cpp
  *  \brief Test for server::file::DirectoryWrapper
  */

#include "server/file/directorywrapper.hpp"

#include "afl/except/fileproblemexception.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/test/testrunner.hpp"
#include "server/file/directoryitem.hpp"
#include "server/file/internaldirectoryhandler.hpp"
#include "server/file/root.hpp"

/** Test DirectoryWrapper. */
AFL_TEST("server.file.DirectoryWrapper:basics", a)
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
    a.checkEqual("01. getTitle", testee->getTitle(), "itemName");
    a.checkEqual("02. getDirectoryName", testee->getDirectoryName(), "");
    a.checkNull("03. getParentDirectory", testee->getParentDirectory().get());

    // File access
    {
        // Open a file
        afl::base::Ref<afl::io::Stream> file = testee->openFile("f1", afl::io::FileSystem::OpenRead);
        a.checkEqual("11. getSize", file->getSize(), 8U);
        uint8_t bytes[8];
        AFL_CHECK_SUCCEEDS(a("12. fullRead"), file->fullRead(bytes));
        a.checkEqualContent<uint8_t>("13. content", bytes, afl::string::toBytes("content1"));

        // Open same file again. Check that we can read it again (=no false sharing)
        afl::base::Ref<afl::io::Stream> file2 = testee->openFile("f1", afl::io::FileSystem::OpenRead);
        a.checkEqual("21. getSize", file2->getSize(), 8U);
        AFL_CHECK_SUCCEEDS(a("22. fullRead"), file2->fullRead(bytes));
    }

    // Some invalid file accesses
    AFL_CHECK_THROWS(a("31. openFile"), testee->openFile("sub", afl::io::FileSystem::OpenRead),  afl::except::FileProblemException);
    AFL_CHECK_THROWS(a("32. openFile"), testee->openFile("f4",  afl::io::FileSystem::OpenRead),  afl::except::FileProblemException);
    AFL_CHECK_THROWS(a("33. openFile"), testee->openFile("f1",  afl::io::FileSystem::OpenWrite), afl::except::FileProblemException);
    AFL_CHECK_THROWS(a("34. openFile"), testee->openFile("f1",  afl::io::FileSystem::Create),    afl::except::FileProblemException);
    AFL_CHECK_THROWS(a("35. openFile"), testee->openFile("fn",  afl::io::FileSystem::Create),    afl::except::FileProblemException);

    // Subdirectory access (not allowed)
    AFL_CHECK_THROWS(a("41. openDirectory"), testee->openDirectory("sub"), afl::except::FileProblemException);

    // Modification
    AFL_CHECK_THROWS(a("51. erase"), testee->erase("f1"), afl::except::FileProblemException);
}

/** Test operations on Enum. */
AFL_TEST("server.file.DirectoryWrapper:getDirectoryEntries", a)
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
        a.checkNonNull("01. getNextElement", i.get());
        if (i->getTitle() == "f1") {
            a.check("02. f1", !f1);
            a.checkEqual("03. getFileType", i->getFileType(), i->tFile);
            a.checkEqual("04. getFileSize", i->getFileSize(), 8U);
            f1 = true;
        } else if (i->getTitle() == "f2") {
            a.check("05. f2", !f2);
            a.checkEqual("06. getFileType", i->getFileType(), i->tFile);
            a.checkEqual("07. getFileSize", i->getFileSize(), 8U);
            f2 = true;
        } else if (i->getTitle() == "f3") {
            a.check("08. f3", !f3);
            a.checkEqual("09. getFileType", i->getFileType(), i->tFile);
            a.checkEqual("10. getFileSize", i->getFileSize(), 8U);
            f3 = true;
        } else {
            a.fail("11. bad file name");
        }
    }
}

/** Test operations on DirectoryEntry. */
AFL_TEST("server.file.DirectoryWrapper:getDirectoryEntryByName", a)
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
    a.checkEqual("01. getFileType", e->getFileType(), e->tFile);
    a.checkEqual("02. getFileSize", e->getFileSize(), 8U);
    a.check     ("03. getFlags", e->getFlags().empty());
    a.checkEqual("04. getTitle", e->getTitle(), "f1");
    a.checkEqual("05. getPathName", e->getPathName(), "");
    a.checkEqual("06", &*e->openContainingDirectory(), &*testee);

    AFL_CHECK_THROWS(a("07. openDirectory"),     e->openDirectory(),          afl::except::FileProblemException);
    AFL_CHECK_THROWS(a("08. renameTo"),          e->renameTo("f1new"),        afl::except::FileProblemException);
    AFL_CHECK_THROWS(a("09. erase"),             e->erase(),                  afl::except::FileProblemException);
    AFL_CHECK_THROWS(a("10. createAsDirectory"), e->createAsDirectory(),      afl::except::FileProblemException);
    AFL_CHECK_THROWS(a("11. setFlag"),           e->setFlag(e->Hidden, true), afl::except::FileProblemException);
}
