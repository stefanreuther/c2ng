/**
  *  \file u/t_server_file_clientdirectory.cpp
  *  \brief Test for server::file::ClientDirectory
  */

#include "server/file/clientdirectory.hpp"

#include <stdexcept>
#include "t_server_file.hpp"
#include "afl/data/errorvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/test/commandhandler.hpp"
#include "server/interface/filebase.hpp"
#include "server/interface/filebaseserver.hpp"
#include "server/types.hpp"

using server::makeStringValue;
using server::interface::FileBase;
using server::interface::FileBaseServer;

/** Test reading a single file.
    Should produce a single GET request. */
void
TestServerFileClientDirectory::testRead()
{
    afl::test::CommandHandler mock("testRead");
    afl::base::Ref<server::file::ClientDirectory> testee = server::file::ClientDirectory::create(mock, "t");

    // Set expectation
    mock.expectCall("GET, t/file");
    mock.provideNewResult(makeStringValue("content"));

    // Read file, verify content
    afl::base::Ref<afl::io::Stream> s = testee->openFile("file", afl::io::FileSystem::OpenRead);
    uint8_t buffer[20];
    TS_ASSERT_EQUALS(s->read(buffer), 7U);
    TS_ASSERT_SAME_DATA(buffer, "content", 7);

    mock.checkFinish();
}

/** Test reading a file's properties.
    Should produce a single STAT request. */
void
TestServerFileClientDirectory::testStat()
{
    afl::test::CommandHandler mock("testStat");
    afl::base::Ref<server::file::ClientDirectory> testee = server::file::ClientDirectory::create(mock, "t");

    // Set expectation
    mock.expectCall("STAT, t/other");
    FileBase::Info i;
    i.size = 78;
    i.type = FileBase::IsFile;
    mock.provideNewResult(FileBaseServer::packInfo(i));

    // Access directory entry
    afl::base::Ref<afl::io::DirectoryEntry> e = testee->getDirectoryEntryByName("other");
    TS_ASSERT_EQUALS(e->getFileSize(), 78U);
    TS_ASSERT_EQUALS(e->getFileType(), afl::io::DirectoryEntry::tFile);

    mock.checkFinish();
}

/** Test reading a directory's content.
    Should produce a single LS request; querying items should no longer access network. */
void
TestServerFileClientDirectory::testList()
{
    afl::test::CommandHandler mock("testList");
    afl::base::Ref<server::file::ClientDirectory> testee = server::file::ClientDirectory::create(mock, "t");
    TS_ASSERT_EQUALS(testee->getTitle(), "t");
    TS_ASSERT_EQUALS(testee->getDirectoryName(), "");

    // Set expectation
    mock.expectCall("LS, t");
    {
        FileBase::Info i;
        i.size = 92;
        i.type = FileBase::IsFile;
        FileBase::Info j;
        j.type = FileBase::IsDirectory;
        afl::data::Vector::Ref_t v = afl::data::Vector::create();
        v->pushBackNew(makeStringValue("i"));
        v->pushBackNew(FileBaseServer::packInfo(i));
        v->pushBackNew(makeStringValue("j"));
        v->pushBackNew(FileBaseServer::packInfo(j));
        mock.provideNewResult(new afl::data::VectorValue(v));
    }

    // Read content
    afl::base::Ref<afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > > iter = testee->getDirectoryEntries();
    afl::base::Ptr<afl::io::DirectoryEntry> e;

    // - verify i
    TS_ASSERT(iter->getNextElement(e));
    TS_ASSERT(e.get() != 0);
    TS_ASSERT_EQUALS(e->getTitle(), "i");
    TS_ASSERT_EQUALS(e->getPathName(), "");
    TS_ASSERT_EQUALS(e->getFileType(), afl::io::DirectoryEntry::tFile);
    TS_ASSERT_EQUALS(e->getFileSize(), 92U);

    // - verify j
    TS_ASSERT(iter->getNextElement(e));
    TS_ASSERT(e.get() != 0);
    TS_ASSERT_EQUALS(e->getTitle(), "j");
    TS_ASSERT_EQUALS(e->getPathName(), "");
    TS_ASSERT_EQUALS(e->getFileType(), afl::io::DirectoryEntry::tDirectory);
    TS_ASSERT_EQUALS(e->getFileSize(), 0U);   // default

    afl::base::Ref<afl::io::Directory> sub = e->openDirectory();
    TS_ASSERT_EQUALS(sub->getTitle(), "t/j");
    TS_ASSERT_EQUALS(sub->getDirectoryName(), "");
    TS_ASSERT_EQUALS(sub->getParentDirectory().get(), &testee.get());

    TS_ASSERT(!iter->getNextElement(e));

    mock.checkFinish();
}

/** Test remote-generated errors.
    These must be converted into FileProblemException. */
void
TestServerFileClientDirectory::testRemoteError()
{
    afl::test::CommandHandler mock("testRemoteError");
    afl::base::Ref<server::file::ClientDirectory> testee = server::file::ClientDirectory::create(mock, "t");

    // Open
    mock.expectCall("GET, t/f");
    mock.provideNewResult(new afl::data::ErrorValue("s", "text"));
    TS_ASSERT_THROWS(testee->openFile("f", afl::io::FileSystem::OpenRead), afl::except::FileProblemException);

    // OpenNT
    mock.expectCall("GET, t/g");
    mock.provideNewResult(new afl::data::ErrorValue("s", "text"));
    TS_ASSERT(testee->openFileNT("g", afl::io::FileSystem::OpenRead).get() == 0);

    // Stat
    mock.expectCall("STAT, t/q");
    mock.provideNewResult(new afl::data::ErrorValue("s", "text"));
    TS_ASSERT_THROWS(testee->getDirectoryEntryByName("q")->getFileSize(), afl::except::FileProblemException);

    // LS
    // Note that getDirectoryEntries() is allowed to not access the network until getNextElement() is called.
    mock.expectCall("LS, t");
    mock.provideNewResult(new afl::data::ErrorValue("s", "text"));
    afl::base::Ptr<afl::io::DirectoryEntry> e;
    TS_ASSERT_THROWS(testee->getDirectoryEntries()->getNextElement(e), afl::except::FileProblemException);

    mock.checkFinish();
}

/** Test locally-generated errors.
    These must not hit the network.
    Since we don't set an expectation, these calls will fail if they access the CommandHandler. */
void
TestServerFileClientDirectory::testLocalError()
{
    afl::test::CommandHandler mock("testLocalError");
    afl::base::Ref<server::file::ClientDirectory> testee = server::file::ClientDirectory::create(mock, "t");

    TS_ASSERT_THROWS(testee->getDirectoryEntryByName("x")->createAsDirectory(), afl::except::FileProblemException);
    TS_ASSERT_THROWS(testee->getDirectoryEntryByName("x")->erase(), afl::except::FileProblemException);
    TS_ASSERT_THROWS(testee->openFile("x", afl::io::FileSystem::OpenWrite), afl::except::FileProblemException);

    mock.checkFinish();
}

/** Test subdirectory behaviour.
    Merely creating subdirectory entries does not access the network, but accessing them does. */
void
TestServerFileClientDirectory::testSubdir()
{
    afl::test::CommandHandler mock("testSubdir");
    afl::base::Ref<server::file::ClientDirectory> testee = server::file::ClientDirectory::create(mock, "t");

    afl::base::Ref<afl::io::DirectoryEntry> e = testee->getDirectoryEntryByName("a")
        ->openDirectory()
        ->getDirectoryEntryByName("b")
        ->openDirectory()
        ->getDirectoryEntryByName("x");

    // Set expectation
    mock.expectCall("GET, t/a/b/x");
    mock.provideNewResult(makeStringValue("zzz"));

    // Read file, verify content
    afl::base::Ref<afl::io::Stream> s = e->openFile(afl::io::FileSystem::OpenRead);
    uint8_t buffer[20];
    TS_ASSERT_EQUALS(s->read(buffer), 3U);
    TS_ASSERT_SAME_DATA(buffer, "zzz", 3);
}

