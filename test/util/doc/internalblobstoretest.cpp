/**
  *  \file test/util/doc/internalblobstoretest.cpp
  *  \brief Test for util::doc::InternalBlobStore
  */

#include "util/doc/internalblobstore.hpp"

#include "afl/except/fileproblemexception.hpp"
#include "afl/test/testrunner.hpp"

using afl::except::FileProblemException;
using util::doc::BlobStore;
using util::doc::InternalBlobStore;

/** Basic test case.
    A: create an InternalBlobStore. Store data.
    E: storing the same data produces same object Id, different data produces different Id, retrieving nonexistant Id fails. */
AFL_TEST("util.doc.InternalBlobStore:basics", a)
{
    InternalBlobStore testee;

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
