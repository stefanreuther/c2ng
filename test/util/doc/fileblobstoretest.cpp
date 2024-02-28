/**
  *  \file test/util/doc/fileblobstoretest.cpp
  *  \brief Test for util::doc::FileBlobStore
  */

#include "util/doc/fileblobstore.hpp"

#include "afl/except/fileproblemexception.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/string/format.hpp"
#include "afl/test/testrunner.hpp"

using afl::base::Enumerator;
using afl::base::Ptr;
using afl::base::Ref;
using afl::except::FileProblemException;
using afl::io::Directory;
using afl::io::DirectoryEntry;
using afl::io::FileSystem;
using afl::io::InternalFileSystem;
using afl::string::Format;
using util::doc::BlobStore;
using util::doc::FileBlobStore;

/** Basic test case.
    A: create a FileBlobStore. Store data.
    E: storing the same data produces same object Id, different data produces different Id, retrieving nonexistant Id fails. */
AFL_TEST("util.doc.FileBlobStore:basics", a)
{
    InternalFileSystem fs;
    fs.createDirectory("/dir");
    FileBlobStore testee(fs.openDirectory("/dir"));

    // Store an object and retrieve it again
    BlobStore::ObjectId_t objId = testee.addObject(afl::string::toBytes("hello there"));
    String_t objContent = afl::string::fromBytes(testee.getObject(objId)->get());
    a.checkEqual("01. content", objContent, "hello there");

    // Store the same object, must produce same Id
    BlobStore::ObjectId_t objId2 = testee.addObject(afl::string::toBytes("hello there"));
    a.checkEqual("11. same id", objId, objId2);

    // Store a different object, must produce different Id
    BlobStore::ObjectId_t objId3 = testee.addObject(afl::string::toBytes("1337"));
    a.checkDifferent("21. different id", objId, objId3);

    // Retrieving an invented Id must fail
    AFL_CHECK_THROWS(a("31. invalid id"), testee.getObject(objId + objId3), FileProblemException);
    AFL_CHECK_THROWS(a("32. invalid id"), testee.getObject(""), FileProblemException);
}

/** Test portability between instances.
    A: create a FileBlobStore and store data. Retrieve that data using a new instance.
    E: data retrieved correctly. */
AFL_TEST("util.doc.FileBlobStore:portability", a)
{
    InternalFileSystem fs;
    fs.createDirectory("/dir");
    BlobStore::ObjectId_t objId;

    {
        FileBlobStore testee(fs.openDirectory("/dir"));
        objId = testee.addObject(afl::string::toBytes("hello there"));
    }
    {
        FileBlobStore testee(fs.openDirectory("/dir"));
        String_t objContent = afl::string::fromBytes(testee.getObject(objId)->get());
        a.checkEqual("objContent", objContent, "hello there");
    }
}
