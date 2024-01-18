/**
  *  \file test/util/doc/singleblobstoretest.cpp
  *  \brief Test for util::doc::SingleBlobStore
  */

#include "util/doc/singleblobstore.hpp"

#include "afl/except/fileproblemexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/test/testrunner.hpp"

using afl::base::Ref;
using afl::except::FileProblemException;
using afl::io::ConstMemoryStream;
using afl::io::InternalStream;
using util::doc::BlobStore;
using util::doc::SingleBlobStore;

/** Basic test case.
    A: create a SingleBlobStore in an InternalStream. Store data.
    E: storing the same data produces same object Id, different data produces different Id, retrieving nonexistant Id fails. */
AFL_TEST("util.doc.SingleBlobStore:basics", a)
{
    Ref<InternalStream> str(*new InternalStream());
    SingleBlobStore testee(str);

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

    // Two objects -> 2 kB file
    a.checkEqual("31. getSize", str->getSize(), 2048U);

    // Retrieving an invented Id must fail
    AFL_CHECK_THROWS(a("41. invalid id"), testee.getObject(objId + objId3), FileProblemException);
    AFL_CHECK_THROWS(a("42. invalid id"), testee.getObject(""), FileProblemException);
}

/** Test portability between instances.
    A: create an InternalBlobStore and store data. Retrieve that data using a new instance.
    E: data retrieved correctly. */
AFL_TEST("util.doc.SingleBlobStore:portability", a)
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
        a.checkEqual("01. content", objContent, "hello there");
    }
}

/** Test re-use of objects.
    A: create an InternalBlobStore and store an object.
       Access InternalBlobStore with a different, read-only instance.
       Store same object again.
    E: success; no new object written */
AFL_TEST("util.doc.SingleBlobStore:reuse", a)
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
        a.checkEqual("01. objId", objId, testee.addObject(afl::string::toBytes("hello there")));

        // Counter-check: attempt to write fails
        AFL_CHECK_THROWS(a("11. addObject"), testee.addObject(afl::string::toBytes("hello")), FileProblemException);
    }
}

/** Test startup with invalid data.
    A: create a stream with invalid content. Start up.
    E: must throw. */
AFL_TEST("util.doc.SingleBlobStore:error:bad-data", a)
{
    uint8_t data[4000];
    afl::base::Bytes_t mem(data);
    mem.fill('x');

    Ref<ConstMemoryStream> ms(*new ConstMemoryStream(mem));
    AFL_CHECK_THROWS(a, (SingleBlobStore(ms)), FileProblemException);
}

/** Test startup with a null block.
    A: create stream containing nulls. Start up.
    E: must start up successfully and be able to store objects. */
AFL_TEST("util.doc.SingleBlobStore:null-block", a)
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
    a.checkDifferent("01. objId", objId, objId3);

    // Two objects -> 2 kB file
    // Size would differ if we hadn't stopped at the null block above.
    a.checkEqual("11. getSize", str->getSize(), 2048U);
}
