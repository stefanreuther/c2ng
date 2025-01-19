/**
  *  \file test/server/host/spec/directorytest.cpp
  *  \brief Test for server::host::spec::Directory
  */

#include "server/host/spec/directory.hpp"

#include "afl/except/fileproblemexception.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/test/testrunner.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/interface/filebaseclient.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::io::DirectoryEntry;
using afl::io::Stream;
using server::file::InternalFileServer;
using server::host::spec::Directory;
using server::interface::FileBaseClient;

/** Test file access, normal cases. */
AFL_TEST("server.host.spec.Directory:file-access", a)
{
    InternalFileServer fs;
    FileBaseClient fc(fs);

    // Prepare
    fc.createDirectoryTree("e/d");
    fc.putFile("e/d/file", "12345");
    Ref<Directory> dir = Directory::create(fc, "e/d");

    // Direct properties
    a.checkEqual("01. getDirectoryName",  dir->getDirectoryName(), "e/d");
    a.checkEqual("02. getTitle",          dir->getTitle(), "d");
    a.checkNull("03. getParentDirectory", dir->getParentDirectory().get());

    // File properties
    Ref<DirectoryEntry> e = dir->getDirectoryEntryByName("file");
    a.checkEqual("11. getFileSize",              e->getFileSize(), 5U);
    a.checkEqual("12. getFileType",              e->getFileType(), afl::io::DirectoryEntry::tFile);
    a.checkEqual("13. getTitle",                 e->getTitle(), "file");
    a.checkEqual("14. getPathName",              e->getPathName(), "e/d/file");
    a.checkEqual("15. openContainingDirectory", &e->openContainingDirectory().get(), &dir.get());

    // File content
    Ref<Stream> s = dir->openFile("file", afl::io::FileSystem::OpenRead);
    uint8_t tmp[100];
    a.checkEqual("21. read", s->read(tmp), 5U);
    a.checkEqual("22. content", tmp[0], '1');
    a.checkEqual("23. content", tmp[4], '5');

    // Attributes of nonexistant files
    a.checkEqual("31. getDirectoryEntryByName", dir->getDirectoryEntryByName("other")->getFileType(), afl::io::DirectoryEntry::tUnknown);

    // Disallowed operations
    AFL_CHECK_THROWS(a("41. erase"), dir->erase("file"), afl::except::FileProblemException);
    AFL_CHECK_THROWS(a("42. renameTo"), e->renameTo("file2"), afl::except::FileProblemException);
    AFL_CHECK_THROWS(a("43. setFlag"), e->setFlag(afl::io::DirectoryEntry::Hidden, true), afl::except::FileProblemException);
    AFL_CHECK_THROWS(a("44. getDirectoryEntryByName"), dir->getDirectoryEntryByName("other")->createAsDirectory(), afl::except::FileProblemException);
    AFL_CHECK_THROWS(a("45. moveTo"), e->moveTo(*dir, "file3"), afl::except::FileProblemException);
}

/** Test file access when access is disabled. */
AFL_TEST("server.host.spec.Directory:file-access:disabled", a)
{
    InternalFileServer fs;
    FileBaseClient fc(fs);

    // Prepare
    fc.createDirectoryTree("e/d");
    fc.putFile("e/d/file", "12345");
    Ref<Directory> dir = Directory::create(fc, "e/d");

    // File can be opened
    AFL_CHECK_SUCCEEDS(a("01. openFile"), dir->openFile("file", afl::io::FileSystem::OpenRead));

    // Disable access, file access now throws
    dir->setEnabled(false);
    AFL_CHECK_THROWS(a("11. openFile when disabled"), dir->openFile("file", afl::io::FileSystem::OpenRead), afl::except::FileProblemException);
}

/** Test directory access. */
AFL_TEST("server.host.spec.Directory:directory-access", a)
{
    InternalFileServer fs;
    FileBaseClient fc(fs);

    // Prepare
    fc.createDirectoryTree("e/d");
    fc.createDirectoryTree("e/d/sub");
    fc.putFile("e/d/file", "12345");
    Ref<Directory> dir = Directory::create(fc, "e/d");

    // Iteration
    // Let's not make it contractual whether subdirectories are returned at all; at least, we cannot open them.
    Ref<afl::base::Enumerator<Ptr<DirectoryEntry> > > it = dir->getDirectoryEntries();
    bool gotFile = false;
    Ptr<DirectoryEntry> e;
    while (it->getNextElement(e)) {
        a.checkNonNull("01", e.get());
        if (e->getTitle() == "file") {
            gotFile = true;
            a.checkEqual("02. getFileSize", e->getFileSize(), 5U);
        }
    }
    a.check("03. found file", gotFile);

    // Disallowed operations
    // Although directory exists, we do not allow recursing into it
    AFL_CHECK_THROWS(a("11. openDirectory"), dir->openDirectory("sub"), afl::except::FileProblemException);
}

/** Test file access redirection, positive case: if file does not exist, but .frag file exists, that one is used. */
AFL_TEST("server.host.spec.Directory:fragment-redirect", a)
{
    InternalFileServer fs;
    FileBaseClient fc(fs);

    // Prepare
    fc.createDirectoryTree("e/d");
    fc.putFile("e/d/file.frag", "xyz");
    Ref<Directory> dir = Directory::create(fc, "e/d");

    // File properties
    Ref<DirectoryEntry> e = dir->getDirectoryEntryByName("file");
    a.checkEqual("01. getFileSize", e->getFileSize(), 3U);
    a.checkEqual("02. getFileType", e->getFileType(), afl::io::DirectoryEntry::tFile);
    a.checkEqual("03. getTitle",    e->getTitle(), "file");

    // File content
    Ref<Stream> s = dir->openFile("file", afl::io::FileSystem::OpenRead);
    uint8_t tmp[100];
    a.checkEqual("11. read", s->read(tmp), 3U);
    a.checkEqual("12. content", tmp[0], 'x');
    a.checkEqual("13. content", tmp[2], 'z');
}

/** Test file access redirection, negative case. If file exists, .frag is not used. */
AFL_TEST("server.host.spec.Directory:fragment-redirect:not-used", a)
{
    InternalFileServer fs;
    FileBaseClient fc(fs);

    // Prepare
    fc.createDirectoryTree("e/d");
    fc.putFile("e/d/a.txt.frag", "xyz");
    fc.putFile("e/d/a.txt", "12345");
    Ref<Directory> dir = Directory::create(fc, "e/d");

    // File properties
    Ref<DirectoryEntry> e = dir->getDirectoryEntryByName("a.txt");
    a.checkEqual("01. getFileSize", e->getFileSize(), 5U);
    a.checkEqual("02. openFile", dir->openFile("a.txt", afl::io::FileSystem::OpenRead)->getSize(), 5U);
}
