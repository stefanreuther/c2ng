/**
  *  \file test/server/file/ca/internalobjectcachetest.cpp
  *  \brief Test for server::file::ca::InternalObjectCache
  */

#include "server/file/ca/internalobjectcache.hpp"

#include "afl/except/fileproblemexception.hpp"
#include "afl/io/internalfilemapping.hpp"
#include "afl/test/testrunner.hpp"

using server::file::ca::ObjectId;
using server::file::ca::ObjectStore;

/** Simple test. This plays just a simple add/get/remove cycle. */
AFL_TEST("server.file.ca.InternalObjectCache:basics", a)
{
    const ObjectId id = ObjectId::fromHex("78d16fb0b0c1dede94861a7a328d8c4d16b5d7ff");
    size_t tmp = 0;

    // Test subject
    server::file::ca::InternalObjectCache testee;

    // Cache is empty and answers with negative response
    a.checkNull("01. getObject",      testee.getObject(id, ObjectStore::TreeObject).get());
    a.check    ("02. getObjectSize", !testee.getObjectSize(id, ObjectStore::TreeObject).isValid());

    // Add size
    testee.addObjectSize(id, ObjectStore::TreeObject, 5);
    a.checkNull ("11. getObject",     testee.getObject(id, ObjectStore::TreeObject).get());
    a.check     ("12. getObjectSize", testee.getObjectSize(id, ObjectStore::TreeObject).get(tmp));
    a.checkEqual("13. result", tmp, 5U);

    // Add content
    afl::base::GrowableMemory<uint8_t> mem;
    mem.append(afl::string::toBytes("abcde"));
    testee.addObject(id, ObjectStore::TreeObject, *new afl::io::InternalFileMapping(mem));
    a.checkNonNull("21. getObject", testee.getObject(id, ObjectStore::TreeObject).get());
    a.check       ("22. getObject", testee.getObject(id, ObjectStore::TreeObject)->get().equalContent(afl::string::toBytes("abcde")));
    tmp = 0;
    a.check     ("23. getObjectSize", testee.getObjectSize(id, ObjectStore::TreeObject).get(tmp));
    a.checkEqual("24. result", tmp, 5U);

    // Remove
    testee.removeObject(id);
    a.checkNull("31. getObject",      testee.getObject(id, ObjectStore::TreeObject).get());
    a.check    ("32. getObjectSize", !testee.getObjectSize(id, ObjectStore::TreeObject).isValid());
}

/** Test expiry. */
AFL_TEST("server.file.ca.InternalObjectCache:expire", a)
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
    a.check("01. getObjectSize", testee.getObjectSize(ObjectId::fromHex("21"), ObjectStore::TreeObject).get(tmp));
    a.checkEqual("02. result", tmp, 21U);
    a.check("03. getObjectSize", testee.getObjectSize(ObjectId::fromHex("22"), ObjectStore::TreeObject).get(tmp));
    a.checkEqual("04. result", tmp, 22U);
    a.check("05. getObjectSize", testee.getObjectSize(ObjectId::fromHex("23"), ObjectStore::TreeObject).get(tmp));
    a.checkEqual("06. result", tmp, 23U);

    // Retrieval with a wrong type is an error
    AFL_CHECK_THROWS(a("11. getObjectSize"), testee.getObjectSize(ObjectId::fromHex("23"), ObjectStore::DataObject), afl::except::FileProblemException);

    // Add 21 again; can still retrieve all 3
    testee.addObjectSize(ObjectId::fromHex("21"), ObjectStore::TreeObject, 21);

    a.check("21. getObjectSize", testee.getObjectSize(ObjectId::fromHex("21"), ObjectStore::TreeObject).get(tmp));
    a.checkEqual("22. result", tmp, 21U);
    a.check("23. getObjectSize", testee.getObjectSize(ObjectId::fromHex("22"), ObjectStore::TreeObject).get(tmp));
    a.checkEqual("24. result", tmp, 22U);
    a.check("25. getObjectSize", testee.getObjectSize(ObjectId::fromHex("23"), ObjectStore::TreeObject).get(tmp));
    a.checkEqual("26. result", tmp, 23U);

    // Add 24; this will expire 21+22.
    testee.addObjectSize(ObjectId::fromHex("24"), ObjectStore::TreeObject, 24);

    a.check("31. getObjectSize", !testee.getObjectSize(ObjectId::fromHex("21"), ObjectStore::TreeObject).get(tmp));
    a.check("32. getObjectSize", !testee.getObjectSize(ObjectId::fromHex("22"), ObjectStore::TreeObject).get(tmp));
    a.check("33. getObjectSize", testee.getObjectSize(ObjectId::fromHex("23"), ObjectStore::TreeObject).get(tmp));
    a.checkEqual("34. result", tmp, 23U);
    a.check("35. getObjectSize", testee.getObjectSize(ObjectId::fromHex("24"), ObjectStore::TreeObject).get(tmp));
    a.checkEqual("36. result", tmp, 24U);
}
