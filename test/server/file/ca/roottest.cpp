/**
  *  \file test/server/file/ca/roottest.cpp
  *  \brief Test for server::file::ca::Root
  */

#include "server/file/ca/root.hpp"

#include "afl/io/internaldirectory.hpp"
#include "afl/test/testrunner.hpp"
#include "server/file/directoryitem.hpp"
#include "server/file/internaldirectoryhandler.hpp"
#include "server/file/root.hpp"

namespace {
    size_t countObjects(server::file::InternalDirectoryHandler& dir)
    {
        server::file::InternalDirectoryHandler::Directory* objDir = dir.findDirectory("objects");
        size_t count = 0;
        if (objDir != 0) {
            for (size_t i = 0, n = objDir->subdirectories.size(); i < n; ++i) {
                count += objDir->subdirectories[i]->files.size();
            }
        }
        return count;
    }
}


/** Test operation on initially-empty directory. */
AFL_TEST("server.file.ca.Root:empty", a)
{
    // Storage
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);

    // Store some files
    {
        server::file::ca::Root t(rootHandler);
        a.checkEqual("01. getMasterCommitId", t.getMasterCommitId(), server::file::ca::ObjectId::nil);

        server::file::DirectoryItem rootItem("(ca-root)", 0, std::auto_ptr<server::file::DirectoryHandler>(t.createRootHandler()));
        rootItem.createDirectory("d");
        rootItem.createFile("f", afl::string::toBytes("text"));
    }

    // Retrieve files
    {
        server::file::ca::Root t(rootHandler);
        server::file::DirectoryItem rootItem("(ca-root)", 0, std::auto_ptr<server::file::DirectoryHandler>(t.createRootHandler()));
        server::file::Root root(rootItem, afl::io::InternalDirectory::create("x"));
        rootItem.readContent(root);

        // Directory
        a.checkNonNull("11. findDirectory", rootItem.findDirectory("d"));

        // File
        server::file::FileItem* fi = rootItem.findFile("f");
        a.checkNonNull("21. findFile", fi);
        a.check("22. file size", rootItem.getFileContent(*fi)->get().equalContent(afl::string::toBytes("text")));
    }
}

/** Test operation with a preloaded image. */
AFL_TEST("server.file.ca.Root:preloaded", a)
{
    using server::file::DirectoryHandler;

    // Storage
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);

    // Preload
    {
        std::auto_ptr<DirectoryHandler> refs(rootHandler.getDirectory(rootHandler.createDirectory("refs")));
        std::auto_ptr<DirectoryHandler> heads(refs->getDirectory(refs->createDirectory("heads")));
        heads->createFile("master", afl::string::toBytes("1ec5873554c8cd604036b4b6c0221a5ded967637\n"));
    }
    {
        std::auto_ptr<DirectoryHandler> objs(rootHandler.getDirectory(rootHandler.createDirectory("objects")));

        static const uint8_t obj1e[] = {
            0x78, 0x01, 0x95, 0x8d, 0x51, 0x0a, 0x42, 0x21, 0x10, 0x45, 0xfb, 0x76,
            0x15, 0xf3, 0x1f, 0x84, 0x3e, 0xed, 0xa9, 0x10, 0xd1, 0x1a, 0x6a, 0x05,
            0xa3, 0x33, 0x96, 0x90, 0xef, 0x81, 0xcd, 0x83, 0x96, 0x9f, 0xd4, 0x0a,
            0xfa, 0xbc, 0x70, 0xce, 0xb9, 0x79, 0x6d, 0xad, 0x0a, 0x18, 0x6f, 0x77,
            0xd2, 0x99, 0x21, 0x22, 0xfa, 0xec, 0x22, 0x4e, 0x9e, 0x48, 0x6b, 0xa2,
            0x29, 0x51, 0x8a, 0x99, 0xed, 0xd1, 0x95, 0x88, 0x73, 0xc8, 0x45, 0x3b,
            0x1b, 0xe7, 0x14, 0x15, 0x6e, 0xf2, 0x58, 0x3b, 0xdc, 0x84, 0x0b, 0x2e,
            0x70, 0xe5, 0x31, 0xb9, 0xc3, 0xe9, 0x35, 0x2a, 0xdb, 0xe5, 0xde, 0xde,
            0x07, 0xe2, 0x33, 0x18, 0x17, 0x82, 0x35, 0xc6, 0x79, 0x0f, 0x7b, 0x6d,
            0xb4, 0x56, 0xf9, 0x7b, 0x27, 0x83, 0xfc, 0x53, 0x54, 0x75, 0xa9, 0x52,
            0xf1, 0x09, 0xbf, 0x82, 0xfa, 0x00, 0xb0, 0x30, 0x38, 0xdc
        };
        std::auto_ptr<DirectoryHandler> p(objs->getDirectory(objs->createDirectory("1e")));
        p->createFile("c5873554c8cd604036b4b6c0221a5ded967637", obj1e);

        static const uint8_t obj39[] = {
            0x78, 0x01, 0x2b, 0x29, 0x4a, 0x4d, 0x55, 0x30, 0x36, 0x62, 0x30, 0x34,
            0x30, 0x30, 0x33, 0x31, 0x51, 0x48, 0xcb, 0xcc, 0x49, 0x65, 0x70, 0x10,
            0xd1, 0xe5, 0x3c, 0xae, 0xad, 0xa3, 0x1a, 0xce, 0x3d, 0xc3, 0x80, 0x47,
            0xfd, 0xc6, 0x9c, 0xf0, 0xb7, 0xc2, 0xba, 0x00, 0xd7, 0x51, 0x0b, 0x47
        };
        p.reset(objs->getDirectory(objs->createDirectory("39")));
        p->createFile("7bbf059739cbfa73aad2f8bf404d04f478b38a", obj39);

        static const uint8_t obj40[] = {
            0x78, 0x01, 0x4b, 0xca, 0xc9, 0x4f, 0x52, 0x30, 0x65, 0x48, 0xca, 0x29,
            0x4d, 0xe2, 0x02, 0x00, 0x19, 0x4a, 0x03, 0xa4
        };
        p.reset(objs->getDirectory(objs->createDirectory("40")));
        p->createFile("142d09c72b2c25570b98300c27d89c57ed132d", obj40);

        static const uint8_t obj9a[] = {
            0x78, 0x01, 0x2b, 0x29, 0x4a, 0x4d, 0x55, 0x30, 0x33, 0x62, 0x30, 0x31,
            0x00, 0x02, 0x85, 0x94, 0xcc, 0x22, 0x06, 0xcb, 0xea, 0xfd, 0xac, 0xd3,
            0x2d, 0x4f, 0xff, 0x2a, 0x5e, 0x75, 0xe9, 0xc7, 0x7e, 0x07, 0x5f, 0x96,
            0x2f, 0x15, 0x9b, 0xbb, 0x0c, 0x0d, 0x0c, 0xcc, 0x4c, 0x4c, 0x14, 0xd2,
            0x32, 0x73, 0x52, 0x19, 0x96, 0xff, 0xb8, 0xf9, 0xf4, 0xce, 0xe7, 0x65,
            0xfd, 0x77, 0xb5, 0x7f, 0x17, 0xd5, 0xdc, 0x13, 0x62, 0x9a, 0x51, 0xaa,
            0xc6, 0x0d, 0x00, 0xc6, 0x68, 0x1d, 0xea
        };
        p.reset(objs->getDirectory(objs->createDirectory("9a")));
        p->createFile("a7c49a27dd00dd2bdb9ce354f9a68cf04396b9", obj9a);

        static const uint8_t obja7[] = {
            0x78, 0x01, 0x4b, 0xca, 0xc9, 0x4f, 0x52, 0x30, 0x61, 0x48, 0xca, 0x49,
            0xe4, 0x02, 0x00, 0x15, 0x20, 0x03, 0x2d
        };
        p.reset(objs->getDirectory(objs->createDirectory("a7")));
        p->createFile("f8d9e5dcf3a68fdd2bfb727cde12029875260b", obja7);
    }

    // Access it
    server::file::ca::Root t(rootHandler);
    server::file::DirectoryItem rootItem("(ca-root)", 0, std::auto_ptr<server::file::DirectoryHandler>(t.createRootHandler()));
    server::file::Root root(rootItem, afl::io::InternalDirectory::create("x"));
    rootItem.readContent(root);

    // File
    server::file::FileItem* fi = rootItem.findFile("file");
    a.checkNonNull("01. findFile", fi);
    a.check("02. file content", rootItem.getFileContent(*fi)->get().equalContent(afl::string::toBytes("bla\n")));

    // Directory
    server::file::DirectoryItem* di = rootItem.findDirectory("dir");
    a.checkNonNull("11. findDirectory", di);

    // Subdirectory
    di->readContent(root);
    fi = di->findFile("file");
    a.checkNonNull("21. findFile", fi);
    a.check("22. file content", rootItem.getFileContent(*fi)->get().equalContent(afl::string::toBytes("blub\n")));
}

/** Test garbage cleanup. */
AFL_TEST("server.file.ca.Root:garbage", a)
{
    using server::file::DirectoryHandler;

    // Storage
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);
    server::file::ca::Root testee(rootHandler);

    // Create stuff
    std::auto_ptr<DirectoryHandler> root(testee.createRootHandler());
    std::auto_ptr<DirectoryHandler> dir1(root->getDirectory(root->createDirectory("dir1")));
    std::auto_ptr<DirectoryHandler> dir2(root->getDirectory(root->createDirectory("dir2")));
    dir1->createFile("a", afl::string::toBytes("content"));
    dir2->createFile("a", afl::string::toBytes("content"));

    // Verify content.
    // 'objects' must have one DataObject, two TreeObject's, and one CommitObject.
    a.checkEqual("01", countObjects(rootHandler), 4U);

    // Update a file.
    // Must now have two DataObject's, three TreeObject's, and one CommitObject.
    dir2->createFile("a", afl::string::toBytes("newcontent"));
    a.checkEqual("11", countObjects(rootHandler), 6U);

    // Update other file. This will merge again.
    dir1->createFile("a", afl::string::toBytes("newcontent"));
    a.checkEqual("21", countObjects(rootHandler), 4U);
}
