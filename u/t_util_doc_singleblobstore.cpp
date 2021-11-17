/**
  *  \file u/t_util_doc_singleblobstore.cpp
  *  \brief Test for util::doc::SingleBlobStore
  */

#include "util/doc/singleblobstore.hpp"

#include "t_util_doc.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalstream.hpp"

using afl::base::Ref;
using afl::except::FileProblemException;
using afl::io::ConstMemoryStream;
using afl::io::InternalStream;
using util::doc::BlobStore;
using util::doc::SingleBlobStore;

/** Basic test case.
    A: create a SingleBlobStore in an InternalStream. Store data.
    E: storing the same data produces same object Id, different data produces different Id, retrieving nonexistant Id fails. */
void
TestUtilDocSingleBlobStore::testIt()
{
    Ref<InternalStream> str(*new InternalStream());
    SingleBlobStore testee(str);

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

    // Two objects -> 2 kB file
    TS_ASSERT_EQUALS(str->getSize(), 2048U);

    // Retrieving an invented Id must fail
    TS_ASSERT_THROWS(testee.getObject(objId + objId3), FileProblemException);
    TS_ASSERT_THROWS(testee.getObject(""), FileProblemException);
}

/** Test portability between instances.
    A: create an InternalBlobStore and store data. Retrieve that data using a new instance.
    E: data retrieved correctly. */
void
TestUtilDocSingleBlobStore::testPortability()
{
    Ref<InternalStream> str(*new InternalStream());
    BlobStore::ObjectId_t objId;
    {
        SingleBlobStore testee(str->createChild());
        objId = testee.addObject(afl::string::toBytes("hello there"));
    }
    {
        SingleBlobStore testee(str->createChild());
        String_t objContent = afl::string::fromBytes(testee.getObject(objId)->get());
        TS_ASSERT_EQUALS(objContent, "hello there");
    }
}

/** Test re-use of objects.
    A: create an InternalBlobStore and store an object.
       Access InternalBlobStore with a different, read-only instance.
       Store same object again.
    E: success; no new object written */
void
TestUtilDocSingleBlobStore::testReuse()
{
    Ref<InternalStream> str(*new InternalStream());
    BlobStore::ObjectId_t objId;
    {
        SingleBlobStore testee(str->createChild());
        objId = testee.addObject(afl::string::toBytes("hello there"));
    }

    {
        Ref<ConstMemoryStream> ms(*new ConstMemoryStream(str->getContent()));
        SingleBlobStore testee(ms);
        TS_ASSERT_EQUALS(objId, testee.addObject(afl::string::toBytes("hello there")));

        // Counter-check: attempt to write fails
        TS_ASSERT_THROWS(testee.addObject(afl::string::toBytes("hello")), FileProblemException);
    }
}

/** Test startup with invalid data.
    A: create a stream with invalid content. Start up.
    E: must throw. */
void
TestUtilDocSingleBlobStore::testFail()
{
    uint8_t data[4000];
    afl::base::Bytes_t mem(data);
    mem.fill('x');

    Ref<ConstMemoryStream> ms(*new ConstMemoryStream(mem));
    TS_ASSERT_THROWS((SingleBlobStore(ms)), FileProblemException);
}

/** Test startup with a null block.
    A: create stream containing nulls. Start up.
    E: must start up successfully and be able to store objects. */
void
TestUtilDocSingleBlobStore::testZero()
{
    // Fresh stream
    Ref<InternalStream> str(*new InternalStream());

    // Write some nulls
    uint8_t data[2000];
    afl::base::Bytes_t mem(data);
    mem.fill(0);
    str->fullWrite(data);
    str->setPos(0);

    // Start up an store stuff
    SingleBlobStore testee(str);
    BlobStore::ObjectId_t objId = testee.addObject(afl::string::toBytes("hello there"));
    BlobStore::ObjectId_t objId3 = testee.addObject(afl::string::toBytes("1337"));
    TS_ASSERT_DIFFERS(objId, objId3);

    // Two objects -> 2 kB file
    // Size would differ if we hadn't stopped at the null block above.
    TS_ASSERT_EQUALS(str->getSize(), 2048U);
}

