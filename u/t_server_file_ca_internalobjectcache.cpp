/**
  *  \file u/t_server_file_ca_internalobjectcache.cpp
  *  \brief Test for server::file::ca::InternalObjectCache
  */

#include "server/file/ca/internalobjectcache.hpp"

#include "t_server_file_ca.hpp"
#include "afl/io/internalfilemapping.hpp"
#include "afl/except/fileproblemexception.hpp"

using server::file::ca::ObjectId;
using server::file::ca::ObjectStore;

/** Simple test. This plays just a simple add/get/remove cycle. */
void
TestServerFileCaInternalObjectCache::testIt()
{
    const ObjectId id = ObjectId::fromHex("78d16fb0b0c1dede94861a7a328d8c4d16b5d7ff");
    size_t tmp = 0;

    // Test subject
    server::file::ca::InternalObjectCache testee;

    // Cache is empty and answers with negative response
    TS_ASSERT(testee.getObject(id, ObjectStore::TreeObject).get() == 0);
    TS_ASSERT(!testee.getObjectSize(id, ObjectStore::TreeObject).isValid());

    // Add size
    testee.addObjectSize(id, ObjectStore::TreeObject, 5);
    TS_ASSERT(testee.getObject(id, ObjectStore::TreeObject).get() == 0);
    TS_ASSERT(testee.getObjectSize(id, ObjectStore::TreeObject).get(tmp));
    TS_ASSERT_EQUALS(tmp, 5U);

    // Add content
    afl::base::GrowableMemory<uint8_t> mem;
    mem.append(afl::string::toBytes("abcde"));
    testee.addObject(id, ObjectStore::TreeObject, *new afl::io::InternalFileMapping(mem));
    TS_ASSERT(testee.getObject(id, ObjectStore::TreeObject).get() != 0);
    TS_ASSERT(testee.getObject(id, ObjectStore::TreeObject)->get().equalContent(afl::string::toBytes("abcde")));
    tmp = 0;
    TS_ASSERT(testee.getObjectSize(id, ObjectStore::TreeObject).get(tmp));
    TS_ASSERT_EQUALS(tmp, 5U);

    // Remove
    testee.removeObject(id);
    TS_ASSERT(testee.getObject(id, ObjectStore::TreeObject).get() == 0);
    TS_ASSERT(!testee.getObjectSize(id, ObjectStore::TreeObject).isValid());
}

/** Test expiry. */
void
TestServerFileCaInternalObjectCache::testExpire()
{
    // Test subject
    server::file::ca::InternalObjectCache testee;

    // Limit 3 objects, 30 bytes
    // This means we will expire down to 2 objects, 22 bytes
    testee.setLimits(3, 30);

    // Add 3 objects
    testee.addObjectSize(ObjectId::fromHex("21"), ObjectStore::TreeObject, 21);
    testee.addObjectSize(ObjectId::fromHex("22"), ObjectStore::TreeObject, 22);
    testee.addObjectSize(ObjectId::fromHex("23"), ObjectStore::TreeObject, 23);

    // All three can be retrieved
    size_t tmp = 0;
    TS_ASSERT(testee.getObjectSize(ObjectId::fromHex("21"), ObjectStore::TreeObject).get(tmp));
    TS_ASSERT_EQUALS(tmp, 21U);
    TS_ASSERT(testee.getObjectSize(ObjectId::fromHex("22"), ObjectStore::TreeObject).get(tmp));
    TS_ASSERT_EQUALS(tmp, 22U);
    TS_ASSERT(testee.getObjectSize(ObjectId::fromHex("23"), ObjectStore::TreeObject).get(tmp));
    TS_ASSERT_EQUALS(tmp, 23U);

    // Retrieval with a wrong type is an error
    TS_ASSERT_THROWS(testee.getObjectSize(ObjectId::fromHex("23"), ObjectStore::DataObject), afl::except::FileProblemException);

    // Add 21 again; can still retrieve all 3
    testee.addObjectSize(ObjectId::fromHex("21"), ObjectStore::TreeObject, 21);

    TS_ASSERT(testee.getObjectSize(ObjectId::fromHex("21"), ObjectStore::TreeObject).get(tmp));
    TS_ASSERT_EQUALS(tmp, 21U);
    TS_ASSERT(testee.getObjectSize(ObjectId::fromHex("22"), ObjectStore::TreeObject).get(tmp));
    TS_ASSERT_EQUALS(tmp, 22U);
    TS_ASSERT(testee.getObjectSize(ObjectId::fromHex("23"), ObjectStore::TreeObject).get(tmp));
    TS_ASSERT_EQUALS(tmp, 23U);

    // Add 24; this will expire 21+22.
    testee.addObjectSize(ObjectId::fromHex("24"), ObjectStore::TreeObject, 24);

    TS_ASSERT(!testee.getObjectSize(ObjectId::fromHex("21"), ObjectStore::TreeObject).get(tmp));
    TS_ASSERT(!testee.getObjectSize(ObjectId::fromHex("22"), ObjectStore::TreeObject).get(tmp));
    TS_ASSERT(testee.getObjectSize(ObjectId::fromHex("23"), ObjectStore::TreeObject).get(tmp));
    TS_ASSERT_EQUALS(tmp, 23U);
    TS_ASSERT(testee.getObjectSize(ObjectId::fromHex("24"), ObjectStore::TreeObject).get(tmp));
    TS_ASSERT_EQUALS(tmp, 24U);
}
