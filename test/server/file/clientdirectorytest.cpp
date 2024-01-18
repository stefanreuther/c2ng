/**
  *  \file test/server/file/clientdirectorytest.cpp
  *  \brief Test for server::file::ClientDirectory
  */

#include "server/file/clientdirectory.hpp"

#include "afl/data/errorvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/filebase.hpp"
#include "server/interface/filebaseserver.hpp"
#include "server/types.hpp"
#include <stdexcept>

using afl::base::ConstBytes_t;
using server::makeStringValue;
using server::interface::FileBase;
using server::interface::FileBaseServer;

/** Test reading a single file.
    Should produce a single GET request. */
AFL_TEST("server.file.ClientDirectory:read", a)
{
    afl::test::CommandHandler mock(a);
    afl::base::Ref<server::file::ClientDirectory> testee = server::file::ClientDirectory::create(mock, "t");

    // Set expectation
    mock.expectCall("GET, t/file");
    mock.provideNewResult(makeStringValue("content"));

    // Read file, verify content
    afl::base::Ref<afl::io::Stream> s = testee->openFile("file", afl::io::FileSystem::OpenRead);
    uint8_t buffer[20];
    a.checkEqual("01. read", s->read(buffer), 7U);
    a.checkEqualContent<uint8_t>("02. content", ConstBytes_t(buffer).trim(7), afl::string::toBytes("content"));

    mock.checkFinish();
}

/** Test reading a file's properties.
    Should produce a single STAT request. */
AFL_TEST("server.file.ClientDirectory:stat", a)
{
    afl::test::CommandHandler mock(a);
    afl::base::Ref<server::file::ClientDirectory> testee = server::file::ClientDirectory::create(mock, "t");

    // Set expectation
    mock.expectCall("STAT, t/other");
    FileBase::Info i;
    i.size = 78;
    i.type = FileBase::IsFile;
    mock.provideNewResult(FileBaseServer::packInfo(i));

    // Access directory entry
    afl::base::Ref<afl::io::DirectoryEntry> e = testee->getDirectoryEntryByName("other");
    a.checkEqual("01. getFileSize", e->getFileSize(), 78U);
    a.checkEqual("02. getFileType", e->getFileType(), afl::io::DirectoryEntry::tFile);

    mock.checkFinish();
}

/** Test reading a directory's content.
    Should produce a single LS request; querying items should no longer access network. */
AFL_TEST("server.file.ClientDirectory:list", a)
{
    afl::test::CommandHandler mock(a);
    afl::base::Ref<server::file::ClientDirectory> testee = server::file::ClientDirectory::create(mock, "t");
    a.checkEqual("01. getTitle", testee->getTitle(), "t");
    a.checkEqual("02. getDirectoryName", testee->getDirectoryName(), "");

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
    a.check("11. getNextElement", iter->getNextElement(e));
    a.checkNonNull("12. element", e.get());
    a.checkEqual("13. getTitle", e->getTitle(), "i");
    a.checkEqual("14. getPathName", e->getPathName(), "");
    a.checkEqual("15. getFileType", e->getFileType(), afl::io::DirectoryEntry::tFile);
    a.checkEqual("16. getFileSize", e->getFileSize(), 92U);

    // - verify j
    a.check("21. getNextElement", iter->getNextElement(e));
    a.checkNonNull("22. element", e.get());
    a.checkEqual("23. getTitle", e->getTitle(), "j");
    a.checkEqual("24. getPathName", e->getPathName(), "");
    a.checkEqual("25. getFileType", e->getFileType(), afl::io::DirectoryEntry::tDirectory);
    a.checkEqual("26. getFileSize", e->getFileSize(), 0U);   // default

    afl::base::Ref<afl::io::Directory> sub = e->openDirectory();
    a.checkEqual("31. getTitle", sub->getTitle(), "t/j");
    a.checkEqual("32. getDirectoryName", sub->getDirectoryName(), "");
    a.checkEqual("33. getParentDirectory", sub->getParentDirectory().get(), &testee.get());

    a.check("41. getNextElement", !iter->getNextElement(e));

    mock.checkFinish();
}

/** Test remote-generated errors.
    These must be converted into FileProblemException. */
AFL_TEST("server.file.ClientDirectory:remote-error", a)
{
    afl::test::CommandHandler mock(a);
    afl::base::Ref<server::file::ClientDirectory> testee = server::file::ClientDirectory::create(mock, "t");

    // Open
    mock.expectCall("GET, t/f");
    mock.provideNewResult(new afl::data::ErrorValue("s", "text"));
    AFL_CHECK_THROWS(a("01. openFile"), testee->openFile("f", afl::io::FileSystem::OpenRead), afl::except::FileProblemException);

    // OpenNT
    mock.expectCall("GET, t/g");
    mock.provideNewResult(new afl::data::ErrorValue("s", "text"));
    a.checkNull("11. openFileNT", testee->openFileNT("g", afl::io::FileSystem::OpenRead).get());

    // Stat
    mock.expectCall("STAT, t/q");
    mock.provideNewResult(new afl::data::ErrorValue("s", "text"));
    AFL_CHECK_THROWS(a("21. getDirectoryEntryByName"), testee->getDirectoryEntryByName("q")->getFileSize(), afl::except::FileProblemException);

    // LS
    // Note that getDirectoryEntries() is allowed to not access the network until getNextElement() is called.
    mock.expectCall("LS, t");
    mock.provideNewResult(new afl::data::ErrorValue("s", "text"));
    afl::base::Ptr<afl::io::DirectoryEntry> e;
    AFL_CHECK_THROWS(a("31. getDirectoryEntries"), testee->getDirectoryEntries()->getNextElement(e), afl::except::FileProblemException);

    mock.checkFinish();
}

/** Test locally-generated errors.
    These must not hit the network.
    Since we don't set an expectation, these calls will fail if they access the CommandHandler. */
AFL_TEST("server.file.ClientDirectory:local-errors", a)
{
    afl::test::CommandHandler mock(a);
    afl::base::Ref<server::file::ClientDirectory> testee = server::file::ClientDirectory::create(mock, "t");

    AFL_CHECK_THROWS(a("01. createAsDirectory"), testee->getDirectoryEntryByName("x")->createAsDirectory(), afl::except::FileProblemException);
    AFL_CHECK_THROWS(a("02. erase"),             testee->getDirectoryEntryByName("x")->erase(), afl::except::FileProblemException);
    AFL_CHECK_THROWS(a("03. OpenWrite"),         testee->openFile("x", afl::io::FileSystem::OpenWrite), afl::except::FileProblemException);

    mock.checkFinish();
}

/** Test subdirectory behaviour.
    Merely creating subdirectory entries does not access the network, but accessing them does. */
AFL_TEST("server.file.ClientDirectory:subdir", a)
{
    afl::test::CommandHandler mock(a);
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
    a.checkEqual("01. read", s->read(buffer), 3U);
    a.checkEqualContent<uint8_t>("02. content", ConstBytes_t(buffer).trim(3), afl::string::toBytes("zzz"));
}
