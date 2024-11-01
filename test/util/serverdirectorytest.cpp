/**
  *  \file test/util/serverdirectorytest.cpp
  *  \brief Test for util::ServerDirectory
  */

#include <map>
#include <set>
#include "util/serverdirectory.hpp"

#include "afl/except/fileproblemexception.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/test/testrunner.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::except::FileProblemException;
using afl::io::DirectoryEntry;
using afl::io::FileSystem;
using afl::io::InternalDirectory;
using afl::io::Stream;
using util::ServerDirectory;

namespace {
    /* Transport implementation that stores data in memory (and verifies requests) */
    class SimpleTransport : public ServerDirectory::Transport {
     public:
        SimpleTransport(afl::test::Assert a, bool writable)
            : m_assert(a),
              m_writable(writable),
              m_files(),
              m_nonFiles()
            { }

        bool hasFile(String_t name)
            {
                return m_files.find(name) != m_files.end();
            }

        bool hasNonFile(String_t name)
            {
                return m_nonFiles.find(name) != m_nonFiles.end();
            }

        String_t getFile(String_t name)
            {
                std::map<String_t, String_t>::iterator it = m_files.find(name);
                m_assert.check("getFile: exists", it != m_files.end());
                return it->second;
            }

        void createNonFile(String_t name)
            {
                m_assert.check("createNonFile: not a file", m_files.find(name) == m_files.end());
                m_nonFiles.insert(name);
            }

        // Interface methods
        virtual void getFile(String_t name, afl::base::GrowableBytes_t& data)
            {
                data.append(afl::string::toBytes(getFile(name)));
            }
        virtual void putFile(String_t name, afl::base::ConstBytes_t data)
            {
                m_assert.check("putFile: not a nonfile", m_nonFiles.find(name) == m_nonFiles.end());
                m_files[name] = afl::string::fromBytes(data);
            }
        virtual void eraseFile(String_t name)
            {
                std::map<String_t, String_t>::iterator it = m_files.find(name);
                m_assert.check("eraseFile: exists", it != m_files.end());
                m_files.erase(it);
            }
        virtual void getContent(std::vector<ServerDirectory::FileInfo>& result)
            {
                for (std::map<String_t, String_t>::iterator it = m_files.begin(); it != m_files.end(); ++it) {
                    result.push_back(ServerDirectory::FileInfo(it->first, it->second.size(), true));
                }
                for (std::set<String_t>::iterator it = m_nonFiles.begin(); it != m_nonFiles.end(); ++it) {
                    result.push_back(ServerDirectory::FileInfo(*it, 0, false));
                }
            }
        virtual bool isValidFileName(String_t name) const
            {
                return name.find('/') == String_t::npos;
            }
        virtual bool isWritable() const
            {
                return m_writable;
            }

     private:
        afl::test::Assert m_assert;
        bool m_writable;
        std::map<String_t, String_t> m_files;
        std::set<String_t> m_nonFiles;
    };

    /* Transport implementation that fails, for verification of flush() error handling */
    class FailingTransport : public ServerDirectory::Transport {
     public:
        FailingTransport(afl::test::Assert a)
            : m_assert(a),
              m_count(0)
            { }

        int getCount()
            { return m_count; }

        // Interface methods
        virtual void getFile(String_t /*name*/, afl::base::GrowableBytes_t& /*data*/)
            { m_assert.fail("getFile unexpected"); }
        virtual void putFile(String_t name, afl::base::ConstBytes_t /*data*/)
            { ++m_count; throw FileProblemException(name, "geht ned"); }
        virtual void eraseFile(String_t /*name*/)
            { m_assert.fail("eraseFile unexpected"); }
        virtual void getContent(std::vector<ServerDirectory::FileInfo>& /*result*/)
            { }
        virtual bool isValidFileName(String_t /*name*/) const
            { return true; }
        virtual bool isWritable() const
            { return true; }

     private:
        afl::test::Assert m_assert;
        int m_count;
    };
}

/** Basic test. Tests that files can be enumerated and retrieved. */
AFL_TEST("util.ServerDirectory:basics", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));
    trans->putFile("f", afl::string::toBytes("text"));
    trans->createNonFile("x");

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));

    // Properties
    a.checkEqual("01. title", testee->getTitle(), "d");
    a.checkEqual("02. dir",   testee->getDirectoryName(), "");
    a.check("03. parent",     testee->getParentDirectory().get() == 0);

    // Individual properties
    a.checkEqual("11. type",  testee->getDirectoryEntryByName("f")->getFileType(), DirectoryEntry::tFile);
    a.checkEqual("12. type",  testee->getDirectoryEntryByName("f")->getFileSize(), 4U);

    a.checkEqual("21. type",  testee->getDirectoryEntryByName("x")->getFileType(), DirectoryEntry::tDirectory);
    a.checkEqual("22. type",  testee->getDirectoryEntryByName("y")->getFileType(), DirectoryEntry::tUnknown);

    // Enumeration
    bool hasF = false, hasX = false;
    Ref<afl::base::Enumerator<Ptr<DirectoryEntry> > > e = testee->getDirectoryEntries();
    Ptr<DirectoryEntry> p;
    while (e->getNextElement(p)) {
        // Verify entry content
        a.checkNonNull("31. entry", p.get());
        if (p->getTitle() == "f") {
            a.check("32. unique f", !hasF);
            hasF = true;
        } else if (p->getTitle() == "x") {
            a.check("33. unique x", !hasX);
            hasX = true;
        } else {
            a.check("34. known name", false);
        }

        // Entry properties
        a.checkEqual("41. path", p->getPathName(), "");
        a.checkEqual("42. dir",  &*p->openContainingDirectory(), &*testee);
    }
    a.check("51. has f", hasF);
    a.check("52. has x", hasX);
}

/*
 *  Open File For Reading
 */

// Base case
AFL_TEST("util.ServerDirectory:OpenRead:success", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));
    trans->putFile("f", afl::string::toBytes("text"));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    Ref<Stream> f = testee->openFile("f", FileSystem::OpenRead);

    uint8_t buf[10];
    size_t got = f->read(buf);
    a.checkEqual("01. got", got, 4U);
    a.checkEqual("02. content", buf[0], 't');
    a.checkEqual("03. content", buf[1], 'e');
    a.checkEqual("04. content", buf[2], 'x');
    a.checkEqual("05. content", buf[3], 't');

    // Sync does not change file
    testee->flush();
    a.checkEqual("99. synced content", trans->getFile("f"), "text");
}

// Base case, open possible even if write disabled
AFL_TEST("util.ServerDirectory:OpenRead:write-disabled", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, false));
    trans->putFile("f", afl::string::toBytes("text"));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    Ref<Stream> f = testee->openFile("f", FileSystem::OpenRead);

    uint8_t buf[10];
    size_t got = f->read(buf);
    a.checkEqual("01. got", got, 4U);
    a.checkEqual("02. content", buf[0], 't');
    a.checkEqual("03. content", buf[1], 'e');
    a.checkEqual("04. content", buf[2], 'x');
    a.checkEqual("05. content", buf[3], 't');
}

// Error: opening a non-file
AFL_TEST("util.ServerDirectory:OpenRead:error:nonfile", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));
    trans->createNonFile("f");

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    AFL_CHECK_THROWS(a, testee->openFile("f", FileSystem::OpenRead), FileProblemException);
}

// Error: opening a missing file
AFL_TEST("util.ServerDirectory:OpenRead:error:missing", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    AFL_CHECK_THROWS(a, testee->openFile("f", FileSystem::OpenRead), FileProblemException);
}

// Error: opening a deleted file
AFL_TEST("util.ServerDirectory:OpenRead:error:deleted", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));
    trans->putFile("f", afl::string::toBytes("text"));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    testee->erase("f");
    AFL_CHECK_THROWS(a, testee->openFile("f", FileSystem::OpenRead), FileProblemException);
}

/*
 *  Open file for update
 */

// Base case (UnreadFile -> DirtyFile)
AFL_TEST("util.ServerDirectory:OpenWrite:success", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));
    trans->putFile("f", afl::string::toBytes("text"));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    Ref<Stream> f = testee->openFile("f", FileSystem::OpenWrite);

    uint8_t buf[10];
    size_t got = f->read(buf);
    a.checkEqual("01. got", got, 4U);
    a.checkEqual("02. content", buf[0], 't');
    a.checkEqual("03. content", buf[1], 'e');
    a.checkEqual("04. content", buf[2], 'x');
    a.checkEqual("05. content", buf[3], 't');

    // Append
    f->write(afl::string::toBytes("new"));
    a.checkEqual("11. size", f->getSize(), 7U);

    // Read through different file
    got = testee->openFile("f", FileSystem::OpenRead)->read(buf);
    a.checkEqual("21. got", got, 7U);
    a.checkEqual("22. content", buf[0], 't');
    a.checkEqual("23. content", buf[1], 'e');
    a.checkEqual("24. content", buf[2], 'x');
    a.checkEqual("25. content", buf[3], 't');
    a.checkEqual("26. content", buf[4], 'n');
    a.checkEqual("27. content", buf[5], 'e');
    a.checkEqual("28. content", buf[6], 'w');

    // Verify enumeration
    Ptr<DirectoryEntry> ptr;
    a.check("31. enum", testee->getDirectoryEntries()->getNextElement(ptr));
    a.checkNonNull("32. ptr", ptr.get());
    a.checkEqual("32. name", ptr->getTitle(), "f");
    a.checkEqual("33. path", ptr->getPathName(), "");
    a.checkEqual("34. size", ptr->getFileSize(), 7U);

    // Sync writes to transport
    testee->flush();
    a.checkEqual("99. synced content", trans->getFile("f"), "textnew");
}

// Write disabled
AFL_TEST("util.ServerDirectory:OpenWrite:error:disabled", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, false));
    trans->putFile("f", afl::string::toBytes("text"));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    AFL_CHECK_THROWS(a, testee->openFile("f", FileSystem::OpenWrite), FileProblemException);
}

// Open for writing after opening for reading (UnreadFile -> CleanFile -> DirtyFile)
AFL_TEST("util.ServerDirectory:OpenWrite:repeat", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));
    trans->putFile("f", afl::string::toBytes("text"));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    testee->openFile("f", FileSystem::OpenRead);
    testee->openFile("f", FileSystem::OpenWrite)->write(afl::string::toBytes("new"));

    uint8_t buf[10];
    size_t got = testee->openFile("f", FileSystem::OpenRead)->read(buf);
    a.checkEqual("01. got", got, 4U);
    a.checkEqual("02. content", buf[0], 'n');
    a.checkEqual("03. content", buf[1], 'e');
    a.checkEqual("04. content", buf[2], 'w');
    a.checkEqual("05. content", buf[3], 't');

    // Sync writes to transport
    testee->flush();
    a.checkEqual("99. synced content", trans->getFile("f"), "newt");
}

// Error: opening a non-file
AFL_TEST("util.ServerDirectory:OpenWrite:error:nonfile", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));
    trans->createNonFile("f");

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    AFL_CHECK_THROWS(a, testee->openFile("f", FileSystem::OpenWrite), FileProblemException);
}

// Error: opening a missing file
AFL_TEST("util.ServerDirectory:OpenWrite:error:missing", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    AFL_CHECK_THROWS(a, testee->openFile("f", FileSystem::OpenWrite), FileProblemException);
}

// Error: opening a deleted file
AFL_TEST("util.ServerDirectory:OpenWrite:error:deleted", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));
    trans->putFile("f", afl::string::toBytes("text"));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    testee->erase("f");
    AFL_CHECK_THROWS(a, testee->openFile("f", FileSystem::OpenWrite), FileProblemException);
}

/*
 *  Create
 */

// Base case (create anew)
AFL_TEST("util.ServerDirectory:Create:success", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    testee->openFile("f", FileSystem::Create)
        ->write(afl::string::toBytes("other"));

    // Read through different file
    uint8_t buf[10];
    size_t got = testee->openFile("f", FileSystem::OpenRead)->read(buf);
    a.checkEqual("01. got", got, 5U);
    a.checkEqual("02. content", buf[0], 'o');
    a.checkEqual("03. content", buf[1], 't');
    a.checkEqual("04. content", buf[2], 'h');
    a.checkEqual("05. content", buf[3], 'e');
    a.checkEqual("06. content", buf[4], 'r');

    // Sync writes to transport
    testee->flush();
    a.checkEqual("99. synced content", trans->getFile("f"), "other");
}

// Replace
AFL_TEST("util.ServerDirectory:Create:replace", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));
    trans->putFile("f", afl::string::toBytes("text"));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    testee->openFile("f", FileSystem::Create)
        ->write(afl::string::toBytes("other"));

    // Read through different file
    uint8_t buf[10];
    size_t got = testee->openFile("f", FileSystem::OpenRead)->read(buf);
    a.checkEqual("01. got", got, 5U);
    a.checkEqual("02. content", buf[0], 'o');
    a.checkEqual("03. content", buf[1], 't');
    a.checkEqual("04. content", buf[2], 'h');
    a.checkEqual("05. content", buf[3], 'e');
    a.checkEqual("06. content", buf[4], 'r');

    // Sync writes to transport
    testee->flush();
    a.checkEqual("99. synced content", trans->getFile("f"), "other");
}

// Replace repeatedly
AFL_TEST("util.ServerDirectory:Create:repeat", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    testee->openFile("f", FileSystem::Create)
        ->write(afl::string::toBytes("one"));
    testee->openFile("f", FileSystem::Create)
        ->write(afl::string::toBytes("two"));
    testee->openFile("f", FileSystem::Create)
        ->write(afl::string::toBytes("three"));

    // Read through different file
    uint8_t buf[10];
    size_t got = testee->openFile("f", FileSystem::OpenRead)->read(buf);
    a.checkEqual("01. got", got, 5U);
    a.checkEqual("02. content", buf[0], 't');
    a.checkEqual("03. content", buf[1], 'h');
    a.checkEqual("04. content", buf[2], 'r');
    a.checkEqual("05. content", buf[3], 'e');
    a.checkEqual("06. content", buf[4], 'e');

    // Sync writes to transport
    testee->flush();
    a.checkEqual("99. synced content", trans->getFile("f"), "three");
}

// Replace a deleted file
AFL_TEST("util.ServerDirectory:Create:replace-deleted", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));
    trans->putFile("f", afl::string::toBytes("text"));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    testee->erase("f");
    testee->openFile("f", FileSystem::Create)
        ->write(afl::string::toBytes("other"));

    // Read through different file
    uint8_t buf[10];
    size_t got = testee->openFile("f", FileSystem::OpenRead)->read(buf);
    a.checkEqual("01. got", got, 5U);
    a.checkEqual("02. content", buf[0], 'o');
    a.checkEqual("03. content", buf[1], 't');
    a.checkEqual("04. content", buf[2], 'h');
    a.checkEqual("05. content", buf[3], 'e');
    a.checkEqual("06. content", buf[4], 'r');

    // Sync writes to transport
    testee->flush();
    a.checkEqual("99. synced content", trans->getFile("f"), "other");
}

// Write disabled
AFL_TEST("util.ServerDirectory:Create:error:disabled", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, false));
    trans->putFile("f", afl::string::toBytes("text"));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    AFL_CHECK_THROWS(a, testee->openFile("f", FileSystem::Create), FileProblemException);
}

// Error: non-file
AFL_TEST("util.ServerDirectory:Create:error:non-file", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));
    trans->createNonFile("f");

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    AFL_CHECK_THROWS(a, testee->openFile("f", FileSystem::Create), FileProblemException);
}

/*
 *  CreateNew
 */

// Base case (create anew)
AFL_TEST("util.ServerDirectory:CreateNew:success", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    testee->openFile("f", FileSystem::CreateNew)
        ->write(afl::string::toBytes("other"));

    // Read through different file
    uint8_t buf[10];
    size_t got = testee->openFile("f", FileSystem::OpenRead)->read(buf);
    a.checkEqual("01. got", got, 5U);
    a.checkEqual("02. content", buf[0], 'o');
    a.checkEqual("03. content", buf[1], 't');
    a.checkEqual("04. content", buf[2], 'h');
    a.checkEqual("05. content", buf[3], 'e');
    a.checkEqual("06. content", buf[4], 'r');

    // Sync writes to transport
    testee->flush();
    a.checkEqual("99. synced content", trans->getFile("f"), "other");
}

// Replace
AFL_TEST("util.ServerDirectory:CreateNew:error:exists", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));
    trans->putFile("f", afl::string::toBytes("text"));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    AFL_CHECK_THROWS(a, testee->openFile("f", FileSystem::CreateNew), FileProblemException);
}

// Duplicate creation
AFL_TEST("util.ServerDirectory:CreateNew:error:dup", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    testee->openFile("f", FileSystem::CreateNew)->write(afl::string::toBytes("x"));
    AFL_CHECK_THROWS(a, testee->openFile("f", FileSystem::CreateNew), FileProblemException);

    // Synchronisation writes original data
    testee->flush();
    a.checkEqual("99. synced content", trans->getFile("f"), "x");
}

// Replace a deleted file
AFL_TEST("util.ServerDirectory:CreateNew:replace-deleted", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));
    trans->putFile("f", afl::string::toBytes("text"));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    testee->erase("f");
    testee->openFile("f", FileSystem::CreateNew)
        ->write(afl::string::toBytes("other"));

    // Read through different file
    uint8_t buf[10];
    size_t got = testee->openFile("f", FileSystem::OpenRead)->read(buf);
    a.checkEqual("01. got", got, 5U);
    a.checkEqual("02. content", buf[0], 'o');
    a.checkEqual("03. content", buf[1], 't');
    a.checkEqual("04. content", buf[2], 'h');
    a.checkEqual("05. content", buf[3], 'e');
    a.checkEqual("06. content", buf[4], 'r');

    // Sync writes to transport
    testee->flush();
    a.checkEqual("99. synced content", trans->getFile("f"), "other");
}

// Write disabled
AFL_TEST("util.ServerDirectory:CreateNew:error:disabled", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, false));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    AFL_CHECK_THROWS(a, testee->openFile("f", FileSystem::CreateNew), FileProblemException);
}

// Error: non-file
AFL_TEST("util.ServerDirectory:CreateNew:error:non-file", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));
    trans->createNonFile("f");

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    AFL_CHECK_THROWS(a, testee->openFile("f", FileSystem::CreateNew), FileProblemException);
}

/*
 *  Erase
 */

// Base case
AFL_TEST("util.ServerDirectory:erase:success", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));
    trans->putFile("f", afl::string::toBytes("text"));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    testee->erase("f");

    // Verify enumeration
    Ptr<DirectoryEntry> ptr;
    a.check("01. enum", !testee->getDirectoryEntries()->getNextElement(ptr));

    // Verify synchronisation
    a.check("10. before", trans->hasFile("f"));
    testee->flush();
    a.check("20. after", !trans->hasFile("f"));
}

// Writing disabled
AFL_TEST("util.ServerDirectory:erase:error:disabled", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, false));
    trans->putFile("f", afl::string::toBytes("text"));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    AFL_CHECK_THROWS(a, testee->erase("f"), FileProblemException);

    // Still there
    testee->flush();
    a.check("10. after", trans->hasFile("f"));
}

// Deleting a non-file
AFL_TEST("util.ServerDirectory:erase:error:non-file", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));
    trans->createNonFile("f");

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    AFL_CHECK_THROWS(a, testee->erase("f"), FileProblemException);

    // Flush must not cause any illegal operation
    testee->flush();
}

// Duplicate erase; second one fails
AFL_TEST("util.ServerDirectory:erase:error:dup", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));
    trans->putFile("f", afl::string::toBytes("text"));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    AFL_CHECK_SUCCEEDS(a, testee->erase("f"));
    AFL_CHECK_THROWS(a, testee->erase("f"), FileProblemException);

    testee->flush();
    a.check("20. after", !trans->hasFile("f"));
}

// Deleting an erased file
AFL_TEST("util.ServerDirectory:erase:error:missing", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    AFL_CHECK_THROWS(a, testee->erase("f"), FileProblemException);

    // Flush must not cause any illegal operation
    testee->flush();
}

// Deleting a created file
AFL_TEST("util.ServerDirectory:erase:created", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    testee->openFile("f", FileSystem::CreateNew);
    AFL_CHECK_SUCCEEDS(a, testee->erase("f"));

    // Directory must be empty
    Ptr<DirectoryEntry> ptr;
    a.check("11. list", !testee->getDirectoryEntries()->getNextElement(ptr));

    // Flush must not cause any illegal operation
    testee->flush();
}

// Deleting a created file
AFL_TEST("util.ServerDirectory:erase:recreate", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));

    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));
    testee->openFile("f", FileSystem::CreateNew)
        ->write(afl::string::toBytes("one"));
    AFL_CHECK_SUCCEEDS(a, testee->erase("f"));
    testee->openFile("f", FileSystem::CreateNew)
        ->write(afl::string::toBytes("two"));

    // Flush must not cause any illegal operation
    testee->flush();
    a.checkEqual("99. synced content", trans->getFile("f"), "two");
}

/*
 *  Dummy operations
 */

AFL_TEST("util.ServerDirectory:dummy", a)
{
    Ref<SimpleTransport> trans(*new SimpleTransport(a, true));
    trans->createNonFile("x");
    trans->putFile("f", afl::string::toBytes("x"));

    Ref<InternalDirectory> parent(InternalDirectory::create("p"));
    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", parent.asPtr()));

    // Can retrieve stored parent
    a.checkEqual("01. parent", testee->getParentDirectory().get(), &*parent);

    // Cannot open subdirectory
    AFL_CHECK_THROWS(a("11. subdir"), testee->openDirectory("x"), FileProblemException);

    // Cannot rename
    AFL_CHECK_THROWS(a("21. rename"), testee->getDirectoryEntryByName("x")->renameTo("y"), FileProblemException);

    // Cannot create directories
    AFL_CHECK_THROWS(a("31. create dir"), testee->getDirectoryEntryByName("y")->createAsDirectory(), FileProblemException);

    // Cannot set flags
    AFL_CHECK_THROWS(a("41. set flags"), testee->getDirectoryEntryByName("f")->setFlag(DirectoryEntry::Executable, true), FileProblemException);

    // Cannot create invalid file name
    AFL_CHECK_THROWS(a("51. create invalid"), testee->openFile("x/y", FileSystem::Create), FileProblemException);
}

/*
 *  Error during sync
 */

AFL_TEST("util.ServerDirectory:flush:error", a)
{
    Ref<FailingTransport> trans(*new FailingTransport(a));
    Ref<ServerDirectory> testee(ServerDirectory::create(trans, "d", 0));

    testee->openFile("a", FileSystem::Create)->write(afl::string::toBytes("one"));
    testee->openFile("b", FileSystem::Create)->write(afl::string::toBytes("two"));
    testee->openFile("c", FileSystem::Create)->write(afl::string::toBytes("three"));

    try {
        testee->flush();
        a.fail("01. expected FileProblemException, got none");
    }
    catch (FileProblemException& fpe) {
        // There is no contract which file will be uploaded first, but a is both first-created and first-in-alphabet,
        // so it's very likely that it'll be the one.
        a.checkEqual("11. name", fpe.getFileName(), "a");
    }
    catch (...) {
        a.fail("99. expected FileProblemException, got other");
    }
}
