/**
  *  \file u/t_server_host_spec_directory.cpp
  *  \brief Test for server::host::spec::Directory
  */

#include "server/host/spec/directory.hpp"

#include "t_server_host_spec.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/directoryentry.hpp"
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
void
TestServerHostSpecDirectory::testFileAccess()
{
    InternalFileServer fs;
    FileBaseClient fc(fs);

    // Prepare
    fc.createDirectoryTree("e/d");
    fc.putFile("e/d/file", "12345");
    Ref<Directory> dir = Directory::create(fc, "e/d");

    // Direct properties
    TS_ASSERT_EQUALS(dir->getDirectoryName(), "e/d");
    TS_ASSERT_EQUALS(dir->getTitle(), "d");
    TS_ASSERT(dir->getParentDirectory().get() == 0);

    // File properties
    Ref<DirectoryEntry> e = dir->getDirectoryEntryByName("file");
    TS_ASSERT_EQUALS(e->getFileSize(), 5U);
    TS_ASSERT_EQUALS(e->getFileType(), afl::io::DirectoryEntry::tFile);
    TS_ASSERT_EQUALS(e->getTitle(), "file");
    TS_ASSERT_EQUALS(e->getPathName(), "e/d/file");
    TS_ASSERT_EQUALS(&e->openContainingDirectory().get(), &dir.get());

    // File content
    Ref<Stream> s = dir->openFile("file", afl::io::FileSystem::OpenRead);
    uint8_t tmp[100];
    TS_ASSERT_EQUALS(s->read(tmp), 5U);
    TS_ASSERT_EQUALS(tmp[0], '1');
    TS_ASSERT_EQUALS(tmp[4], '5');

    // Attributes of nonexistant files
    TS_ASSERT_EQUALS(dir->getDirectoryEntryByName("other")->getFileType(), afl::io::DirectoryEntry::tUnknown);

    // Disallowed operations
    TS_ASSERT_THROWS(dir->erase("file"), afl::except::FileProblemException);
    TS_ASSERT_THROWS(e->renameTo("file2"), afl::except::FileProblemException);
    TS_ASSERT_THROWS(e->setFlag(afl::io::DirectoryEntry::Hidden, true), afl::except::FileProblemException);
    TS_ASSERT_THROWS(dir->getDirectoryEntryByName("other")->createAsDirectory(), afl::except::FileProblemException);
}

/** Test file access when access is disabled. */
void
TestServerHostSpecDirectory::testDisabledFileAccess()
{
    InternalFileServer fs;
    FileBaseClient fc(fs);

    // Prepare
    fc.createDirectoryTree("e/d");
    fc.putFile("e/d/file", "12345");
    Ref<Directory> dir = Directory::create(fc, "e/d");

    // File can be opened
    TS_ASSERT_THROWS_NOTHING(dir->openFile("file", afl::io::FileSystem::OpenRead));

    // Disable access, file access now throws
    dir->setEnabled(false);
    TS_ASSERT_THROWS(dir->openFile("file", afl::io::FileSystem::OpenRead), afl::except::FileProblemException);
}

/** Test directory access. */
void
TestServerHostSpecDirectory::testDirectoryAccess()
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
        TS_ASSERT(e.get() != 0);
        if (e->getTitle() == "file") {
            gotFile = true;
            TS_ASSERT_EQUALS(e->getFileSize(), 5U);
        }
    }
    TS_ASSERT(gotFile);

    // Disallowed operations
    // Although directory exists, we do not allow recursing into it
    TS_ASSERT_THROWS(dir->openDirectory("sub"), afl::except::FileProblemException);
}

/** Test file access redirection, positive case: if file does not exist, but .frag file exists, that one is used. */
void
TestServerHostSpecDirectory::testFragmentRedirect()
{
    InternalFileServer fs;
    FileBaseClient fc(fs);

    // Prepare
    fc.createDirectoryTree("e/d");
    fc.putFile("e/d/file.frag", "xyz");
    Ref<Directory> dir = Directory::create(fc, "e/d");

    // File properties
    Ref<DirectoryEntry> e = dir->getDirectoryEntryByName("file");
    TS_ASSERT_EQUALS(e->getFileSize(), 3U);
    TS_ASSERT_EQUALS(e->getFileType(), afl::io::DirectoryEntry::tFile);
    TS_ASSERT_EQUALS(e->getTitle(), "file");

    // File content
    Ref<Stream> s = dir->openFile("file", afl::io::FileSystem::OpenRead);
    uint8_t tmp[100];
    TS_ASSERT_EQUALS(s->read(tmp), 3U);
    TS_ASSERT_EQUALS(tmp[0], 'x');
    TS_ASSERT_EQUALS(tmp[2], 'z');
}

/** Test file access redirection, negative case. If file exists, .frag is not used. */
void
TestServerHostSpecDirectory::testNoFragmentRedirect()
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
    TS_ASSERT_EQUALS(e->getFileSize(), 5U);
    TS_ASSERT_EQUALS(dir->openFile("a.txt", afl::io::FileSystem::OpenRead)->getSize(), 5U);
}

