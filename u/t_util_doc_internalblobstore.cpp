/**
  *  \file u/t_util_doc_internalblobstore.cpp
  *  \brief Test for util::doc::InternalBlobStore
  */

#include "util/doc/internalblobstore.hpp"

#include "t_util_doc.hpp"
#include "afl/except/fileproblemexception.hpp"

using afl::except::FileProblemException;
using util::doc::BlobStore;
using util::doc::InternalBlobStore;

/** Basic test case.
    A: create an InternalBlobStore. Store data.
    E: storing the same data produces same object Id, different data produces different Id, retrieving nonexistant Id fails. */
void
TestUtilDocInternalBlobStore::testIt()
{
    InternalBlobStore testee;

    // Store an object and retrieve it again
    BlobStore::ObjectId_t objId = testee.addObject(afl::string::toBytes("hello there"));
    String_t objContent = afl::string::fromBytes(testee.getObject(objId)->get());
    TS_ASSERT_EQUALS(objContent, "hello there");

    // Store the same object, must produce same Id
    BlobStore::ObjectId_t objId2 = testee.addObject(afl::string::toBytes("hello there"));
    TS_ASSERT_EQUALS(objId, objId2);

    // Store a different object, must produce different Id
    BlobStore::ObjectId_t objId3 = testee.addObject(afl::string::toBytes("1337"));
    TS_ASSERT_DIFFERS(objId, objId3);

    // Retrieving an invented Id must fail
    TS_ASSERT_THROWS(testee.getObject(objId + objId3), FileProblemException);
    TS_ASSERT_THROWS(testee.getObject(""), FileProblemException);
}

