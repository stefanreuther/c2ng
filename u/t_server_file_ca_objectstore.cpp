/**
  *  \file u/t_server_file_ca_objectstore.cpp
  *  \brief Test for server::file::ca::ObjectStore
  */

#include <memory>
#include <stdexcept>
#include "server/file/ca/objectstore.hpp"

#include "t_server_file_ca.hpp"
#include "server/file/internaldirectoryhandler.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/string/format.hpp"

using server::file::DirectoryHandler;

namespace {
    // A counting DirectoryHandler
    class CountingDirectoryHandler : public DirectoryHandler {
     public:
        CountingDirectoryHandler(size_t& count, std::auto_ptr<DirectoryHandler> impl)
            : m_count(count),
              m_impl(impl)
            { }
        virtual String_t getName()
            { return m_impl->getName(); }
        virtual afl::base::Ref<afl::io::FileMapping> getFile(const Info& info)
            {
                ++m_count;
                return m_impl->getFile(info);
            }
        virtual afl::base::Ref<afl::io::FileMapping> getFileByName(String_t name)
            {
                ++m_count;
                return m_impl->getFileByName(name);
            }
        virtual Info createFile(String_t name, afl::base::ConstBytes_t content)
            { return m_impl->createFile(name, content); }
        virtual void removeFile(String_t name)
            { m_impl->removeFile(name); }
        virtual afl::base::Optional<Info> copyFile(ReadOnlyDirectoryHandler& source, const Info& sourceInfo, String_t name)
            { return m_impl->copyFile(source, sourceInfo, name); }
        virtual void readContent(Callback& callback)
            { m_impl->readContent(callback); }
        virtual DirectoryHandler* getDirectory(const Info& info)
            {
                std::auto_ptr<DirectoryHandler> child(m_impl->getDirectory(info));
                return new CountingDirectoryHandler(m_count, child);
            }
        virtual Info createDirectory(String_t name)
            { return m_impl->createDirectory(name); }
        virtual void removeDirectory(String_t name)
            { m_impl->removeDirectory(name); }
     private:
        size_t& m_count;
        std::auto_ptr<DirectoryHandler> m_impl;
    };
}



/** Test getObject(), getObjectSize(). */
void
TestServerFileCaObjectStore::testGetObject()
{
    // A tree object
    static const uint8_t OBJ[] = {
        0x78, 0x01, 0x2b, 0x29, 0x4a, 0x4d, 0x55, 0x30, 0x36, 0x62, 0x30, 0x34,
        0x30, 0x30, 0x33, 0x31, 0x51, 0x48, 0xcb, 0xcc, 0x49, 0x65, 0x70, 0x10,
        0xd1, 0xe5, 0x3c, 0xae, 0xad, 0xa3, 0x1a, 0xce, 0x3d, 0xc3, 0x80, 0x47,
        0xfd, 0xc6, 0x9c, 0xf0, 0xb7, 0xc2, 0xba, 0x00, 0xd7, 0x51, 0x0b, 0x47
    };
    static const server::file::ca::ObjectId OBJID = {{0x39,0x7b,0xbf,0x05,0x97,0x39,0xcb,0xfa,0x73,0xaa,0xd2,0xf8,0xbf,0x40,0x4d,0x04,0xf4,0x78,0xb3,0x8a}};

    // Create test setup
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);
    server::file::DirectoryHandler::Info subInfo = rootHandler.createDirectory("39");

    std::auto_ptr<server::file::DirectoryHandler> subHandler(rootHandler.getDirectory(subInfo));
    subHandler->createFile("7bbf059739cbfa73aad2f8bf404d04f478b38a", OBJ);

    // Test existing object
    {
        server::file::ca::ObjectStore testee(rootHandler);
        afl::base::Ref<afl::io::FileMapping> result = testee.getObject(OBJID, testee.TreeObject);
        TS_ASSERT_EQUALS(result->get().size(), 32U);

        static const uint8_t EXPECTED[] = {
            0x31, 0x30, 0x30, 0x36, 0x34, 0x34, 0x20, 0x66, 0x69, 0x6c, 0x65, 0x00, 0x40, 0x14, 0x2d, 0x09,
            0xc7, 0x2b, 0x2c, 0x25, 0x57, 0x0b, 0x98, 0x30, 0x0c, 0x27, 0xd8, 0x9c, 0x57, 0xed, 0x13, 0x2d
        };
        TS_ASSERT_SAME_DATA(result->get().unsafeData(), EXPECTED, sizeof(EXPECTED));
    }
    {
        server::file::ca::ObjectStore testee(rootHandler);
        TS_ASSERT_EQUALS(testee.getObjectSize(OBJID, testee.TreeObject), 32U);
    }

    // Existing with wrong type
    {
        server::file::ca::ObjectStore testee(rootHandler);
        TS_ASSERT_THROWS(testee.getObjectSize(OBJID, testee.DataObject), std::runtime_error);
        TS_ASSERT_THROWS(testee.getObject(OBJID, testee.DataObject), std::runtime_error);
    }

    // Test null object
    {
        server::file::ca::ObjectStore testee(rootHandler);
        TS_ASSERT_EQUALS(testee.getObjectSize(server::file::ca::ObjectId::nil, testee.DataObject), 0U);
        TS_ASSERT_EQUALS(testee.getObjectSize(server::file::ca::ObjectId::nil, testee.TreeObject), 0U);
        TS_ASSERT_EQUALS(testee.getObjectSize(server::file::ca::ObjectId::nil, testee.CommitObject), 0U);

        TS_ASSERT_EQUALS(testee.getObject(server::file::ca::ObjectId::nil, testee.DataObject)->get().size(), 0U);
        TS_ASSERT_EQUALS(testee.getObject(server::file::ca::ObjectId::nil, testee.TreeObject)->get().size(), 0U);
        TS_ASSERT_EQUALS(testee.getObject(server::file::ca::ObjectId::nil, testee.CommitObject)->get().size(), 0U);
    }

    // Non-existant
    {
        static const server::file::ca::ObjectId OBJID1 = {{0x39,0x7b,0xbf,0x05,0x97,0x39,0xcb,0xfa,0x73,0xff,0xff,0xff,0xbf,0x40,0x4d,0x04,0xf4,0x78,0xb3,0x8a}};
        static const server::file::ca::ObjectId OBJID2 = {{0x38,0xff,0xff,0x05,0x97,0x39,0xcb,0xfa,0x73,0xff,0xff,0xff,0xbf,0x40,0x4d,0x04,0xf4,0x78,0xb3,0x8a}};
        server::file::ca::ObjectStore testee(rootHandler);
        TS_ASSERT_THROWS(testee.getObjectSize(OBJID1, testee.DataObject), afl::except::FileProblemException);
        TS_ASSERT_THROWS(testee.getObjectSize(OBJID2, testee.DataObject), afl::except::FileProblemException);
        TS_ASSERT_THROWS(testee.getObject(OBJID1, testee.DataObject), afl::except::FileProblemException);
        TS_ASSERT_THROWS(testee.getObject(OBJID2, testee.DataObject), afl::except::FileProblemException);
    }
}

/** Test addObject(). */
void
TestServerFileCaObjectStore::testAddObject()
{
    // Create test setup
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);

    // Add an object
    static const uint8_t CONTENT[] = {
        0x31, 0x30, 0x30, 0x36, 0x34, 0x34, 0x20, 0x66, 0x69, 0x6c, 0x65, 0x00, 0x40, 0x14, 0x2d, 0x09,
        0xc7, 0x2b, 0x2c, 0x25, 0x57, 0x0b, 0x98, 0x30, 0x0c, 0x27, 0xd8, 0x9c, 0x57, 0xed, 0x13, 0x2d
    };
    static const server::file::ca::ObjectId OBJID = {{0x39,0x7b,0xbf,0x05,0x97,0x39,0xcb,0xfa,0x73,0xaa,0xd2,0xf8,0xbf,0x40,0x4d,0x04,0xf4,0x78,0xb3,0x8a}};
    {
        // Add the object
        server::file::ca::ObjectStore testee(rootHandler);
        server::file::ca::ObjectId id = testee.addObject(testee.TreeObject, CONTENT);
        TS_ASSERT_EQUALS(id, OBJID);

        // Adding the same object is not an error
        TS_ASSERT_THROWS_NOTHING(testee.addObject(testee.TreeObject, CONTENT));
    }
    {
        // Retrieve the object again
        server::file::ca::ObjectStore testee(rootHandler);
        afl::base::Ref<afl::io::FileMapping> result = testee.getObject(OBJID, testee.TreeObject);
        TS_ASSERT_EQUALS(result->get().size(), 32U);

        static const uint8_t EXPECTED[] = {
            0x31, 0x30, 0x30, 0x36, 0x34, 0x34, 0x20, 0x66, 0x69, 0x6c, 0x65, 0x00, 0x40, 0x14, 0x2d, 0x09,
            0xc7, 0x2b, 0x2c, 0x25, 0x57, 0x0b, 0x98, 0x30, 0x0c, 0x27, 0xd8, 0x9c, 0x57, 0xed, 0x13, 0x2d
        };
        TS_ASSERT_SAME_DATA(result->get().unsafeData(), EXPECTED, sizeof(EXPECTED));
    }

    // Add some more objects
    // "000" -> fd594a59b16db3e1f6fec8f05f703765a000bdb7 (exercises "make new directory" path)
    // "170" -> 3968aef87f28b2029667d95cd6e22f31b0bd2e50 (exercises "use existing directory" path)
    static const uint8_t CONTENT_NEW[] = {'0','0','0'};
    static const uint8_t CONTENT_SAME[] = {'1','7','0'};
    static const server::file::ca::ObjectId OBJID_NEW =  {{0xfd,0x59,0x4a,0x59,0xb1,0x6d,0xb3,0xe1,0xf6,0xfe,0xc8,0xf0,0x5f,0x70,0x37,0x65,0xa0,0x00,0xbd,0xb7}};
    static const server::file::ca::ObjectId OBJID_SAME = {{0x39,0x68,0xae,0xf8,0x7f,0x28,0xb2,0x02,0x96,0x67,0xd9,0x5c,0xd6,0xe2,0x2f,0x31,0xb0,0xbd,0x2e,0x50}};
    {
        server::file::ca::ObjectStore testee(rootHandler);
        TS_ASSERT_EQUALS(testee.addObject(testee.DataObject, CONTENT_NEW), OBJID_NEW);
        TS_ASSERT_EQUALS(testee.addObject(testee.DataObject, CONTENT_SAME), OBJID_SAME);
    }

    // Adding same content with different type produces different ID
    {
        server::file::ca::ObjectStore testee(rootHandler);
        TS_ASSERT_DIFFERS(testee.addObject(testee.TreeObject, CONTENT_NEW), OBJID_NEW);
        TS_ASSERT_DIFFERS(testee.addObject(testee.CommitObject, CONTENT_NEW), OBJID_NEW);
    }

    // Can still retrieve original objects
    {
        server::file::ca::ObjectStore testee(rootHandler);
        afl::base::Ref<afl::io::FileMapping> result = testee.getObject(OBJID_NEW, testee.DataObject);
        TS_ASSERT(result->get().equalContent(CONTENT_NEW));
    }
}

/** Test storage/retrieval of large objects. */
void
TestServerFileCaObjectStore::testLarge()
{
    using server::file::ca::ObjectStore;

    // Create test setup
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);

    // Create a huge object
    // This is 80k that compress down to about 20k.
    afl::base::GrowableMemory<uint8_t> obj;
    for (int i = 0; i < 10000; ++i) {
        obj.append(afl::string::toBytes(afl::string::Format("%07d\n", i)));
    }
    TS_ASSERT_EQUALS(obj.size(), 80000U);

    // Store object
    server::file::ca::ObjectId id = ObjectStore(rootHandler).addObject(ObjectStore::DataObject, obj);

    // Retrieve object
    afl::base::Ref<afl::io::FileMapping> result = ObjectStore(rootHandler).getObject(id, ObjectStore::DataObject);
    TS_ASSERT_EQUALS(result->get().size(), obj.size());
    TS_ASSERT(result->get().equalContent(obj));
}

/** Test cache effectiveness. */
void
TestServerFileCaObjectStore::testCache()
{
    using server::file::ca::ObjectId;
    using server::file::ca::ObjectStore;

    // Create test setup
    ObjectId a, b, c;
    server::file::InternalDirectoryHandler::Directory rootDir("");
    {
        // Preload some files
        server::file::InternalDirectoryHandler rootHandler("root", rootDir);
        ObjectStore testee(rootHandler);
        a = testee.addObject(ObjectStore::DataObject, afl::string::toBytes("alpha"));
        b = testee.addObject(ObjectStore::DataObject, afl::string::toBytes("bravo"));
        c = testee.addObject(ObjectStore::DataObject, afl::string::toBytes("charlie"));
    }

    // Test sequence
    size_t count = 0;
    CountingDirectoryHandler rootCounter(count, std::auto_ptr<DirectoryHandler>(new server::file::InternalDirectoryHandler("root", rootDir)));
    ObjectStore testee(rootCounter);

    // Add some more files
    ObjectId d = testee.addObject(ObjectStore::DataObject, afl::string::toBytes("delta"));
    ObjectId e = testee.addObject(ObjectStore::DataObject, afl::string::toBytes("echo"));
    ObjectId f = testee.addObject(ObjectStore::DataObject, afl::string::toBytes("foxtrot"));

    // Retrieve these files repeatedly
    for (int i = 0; i < 100; ++i) {
        TS_ASSERT_THROWS_NOTHING(testee.getObject(a, ObjectStore::DataObject));
        TS_ASSERT_THROWS_NOTHING(testee.getObject(b, ObjectStore::DataObject));
        TS_ASSERT_THROWS_NOTHING(testee.getObject(c, ObjectStore::DataObject));
        TS_ASSERT_THROWS_NOTHING(testee.getObject(d, ObjectStore::DataObject));
        TS_ASSERT_THROWS_NOTHING(testee.getObject(e, ObjectStore::DataObject));
        TS_ASSERT_THROWS_NOTHING(testee.getObject(f, ObjectStore::DataObject));

        TS_ASSERT_EQUALS(testee.getObjectSize(a, ObjectStore::DataObject), 5U);
        TS_ASSERT_EQUALS(testee.getObjectSize(b, ObjectStore::DataObject), 5U);
        TS_ASSERT_EQUALS(testee.getObjectSize(c, ObjectStore::DataObject), 7U);
        TS_ASSERT_EQUALS(testee.getObjectSize(d, ObjectStore::DataObject), 5U);
        TS_ASSERT_EQUALS(testee.getObjectSize(e, ObjectStore::DataObject), 4U);
        TS_ASSERT_EQUALS(testee.getObjectSize(f, ObjectStore::DataObject), 7U);
    }

    // Check count
    TS_ASSERT(count > 0);
    TS_ASSERT(count < 10);
}

/** Test caching, size requests only.
    This is the same as above, but asks for sizes only. */
void
TestServerFileCaObjectStore::testCache2()
{
    using server::file::ca::ObjectId;
    using server::file::ca::ObjectStore;

    // Create test setup
    ObjectId a, b, c;
    server::file::InternalDirectoryHandler::Directory rootDir("");
    {
        // Preload some files
        server::file::InternalDirectoryHandler rootHandler("root", rootDir);
        ObjectStore testee(rootHandler);
        a = testee.addObject(ObjectStore::DataObject, afl::string::toBytes("alpha"));
        b = testee.addObject(ObjectStore::DataObject, afl::string::toBytes("bravo"));
        c = testee.addObject(ObjectStore::DataObject, afl::string::toBytes("charlie"));
    }

    // Test sequence
    size_t count = 0;
    CountingDirectoryHandler rootCounter(count, std::auto_ptr<DirectoryHandler>(new server::file::InternalDirectoryHandler("root", rootDir)));
    ObjectStore testee(rootCounter);

    // Add some more files
    ObjectId d = testee.addObject(ObjectStore::DataObject, afl::string::toBytes("delta"));
    ObjectId e = testee.addObject(ObjectStore::DataObject, afl::string::toBytes("echo"));
    ObjectId f = testee.addObject(ObjectStore::DataObject, afl::string::toBytes("foxtrot"));

    // Retrieve these files repeatedly
    for (int i = 0; i < 100; ++i) {
        TS_ASSERT_EQUALS(testee.getObjectSize(a, ObjectStore::DataObject), 5U);
        TS_ASSERT_EQUALS(testee.getObjectSize(b, ObjectStore::DataObject), 5U);
        TS_ASSERT_EQUALS(testee.getObjectSize(c, ObjectStore::DataObject), 7U);
        TS_ASSERT_EQUALS(testee.getObjectSize(d, ObjectStore::DataObject), 5U);
        TS_ASSERT_EQUALS(testee.getObjectSize(e, ObjectStore::DataObject), 4U);
        TS_ASSERT_EQUALS(testee.getObjectSize(f, ObjectStore::DataObject), 7U);
    }

    // Check count
    TS_ASSERT(count > 0);
    TS_ASSERT(count < 10);
}
